// tlsprobe: TLS 1.3 client (mTLS tuỳ chọn) in thông tin handshake.
// Build: gcc -Wall -O2 tlsprobe.c -o tlsprobe -lssl -lcrypto
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
}

static void die(const char *msg) {
    fprintf(stderr, "✘ %s\n", msg);
    ERR_print_errors_fp(stderr);                    // lý do chi tiết từ OpenSSL (alert, verify…)
    exit(1);
}

static int tcp_connect(const char *addr, const char *port) {
    struct addrinfo hints = {0}, *res, *p;
    hints.ai_socktype = SOCK_STREAM;
    int rc = getaddrinfo(addr, port, &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return -1;
    }
    int fd = -1;
    for (p = res; p; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);
    return fd;
}

static void print_san(X509 *cert) {
    GENERAL_NAMES *sans = X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
    if (!sans) return;
    printf("  SAN        :");
    for (int i = 0; i < sk_GENERAL_NAME_num(sans); i++) {
        const GENERAL_NAME *gn = sk_GENERAL_NAME_value(sans, i);
        if (gn->type == GEN_DNS || gn->type == GEN_URI) {
            const ASN1_STRING *s = gn->type == GEN_DNS ? gn->d.dNSName : gn->d.uniformResourceIdentifier;
            printf(" %.*s", ASN1_STRING_length(s), (const char *)ASN1_STRING_get0_data(s));
        }
    }
    printf("\n");
    GENERAL_NAMES_free(sans);
}

static void usage(const char *prog) {
    fprintf(stderr, "Dùng: %s [-c cafile] [-C cert_chain -K key] [-a connect_addr] host port\n", prog);
    exit(2);
}

int main(int argc, char **argv) {
    const char *cafile = NULL, *cert = NULL, *key = NULL, *addr = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "c:C:K:a:")) != -1) {
        if (opt == 'c') cafile = optarg;
        else if (opt == 'C') cert = optarg;
        else if (opt == 'K') key = optarg;
        else if (opt == 'a') addr = optarg;    // IP để connect; host vẫn dùng cho SNI + hostname check
        else usage(argv[0]);
    }
    if (argc - optind != 2) usage(argv[0]);
    const char *host = argv[optind], *port = argv[optind + 1];

    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) die("SSL_CTX_new");
    SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);          // chỉ nhận TLS 1.3
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);               // bắt buộc verify server
    if (cafile ? !SSL_CTX_load_verify_locations(ctx, cafile, NULL)
               : !SSL_CTX_set_default_verify_paths(ctx))
        die("không nạp được trust store");
    if (cert) {                                                   // mTLS: gửi cert + chain
        if (SSL_CTX_use_certificate_chain_file(ctx, cert) != 1 ||
            SSL_CTX_use_PrivateKey_file(ctx, key ? key : cert, SSL_FILETYPE_PEM) != 1 ||
            SSL_CTX_check_private_key(ctx) != 1)
            die("client cert/key không hợp lệ hoặc không khớp nhau");
    }

    int fd = tcp_connect(addr ? addr : host, port);
    if (fd < 0) die("không kết nối TCP được");
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, fd);
    SSL_set_tlsext_host_name(ssl, host);                          // SNI
    SSL_set1_host(ssl, host);                                     // so host với SAN khi verify
    static const unsigned char alpn[] = {8, 'h', 't', 't', 'p', '/', '1', '.', '1'};
    SSL_set_alpn_protos(ssl, alpn, sizeof alpn);

    double t0 = now_ms();
    if (SSL_connect(ssl) != 1) {
        long v = SSL_get_verify_result(ssl);
        if (v != X509_V_OK) fprintf(stderr, "verify: %s\n", X509_verify_cert_error_string(v));
        die("handshake thất bại (SSL_connect)");
    }
    double t1 = now_ms();

    const unsigned char *proto = NULL;
    unsigned int plen = 0;
    SSL_get0_alpn_selected(ssl, &proto, &plen);
    printf("✔ handshake %.1f ms\n", t1 - t0);
    printf("  version    : %s\n", SSL_get_version(ssl));
    printf("  cipher     : %s\n", SSL_get_cipher_name(ssl));
    printf("  group      : %s\n", SSL_group_to_name(ssl, SSL_get_negotiated_group(ssl)));
    printf("  ALPN       : %.*s\n", plen ? (int)plen : 6, plen ? (const char *)proto : "(none)");
    X509 *peer = SSL_get1_peer_certificate(ssl);
    if (peer) {
        printf("  subject    : ");
        X509_NAME_print_ex_fp(stdout, X509_get_subject_name(peer), 0, XN_FLAG_ONELINE);
        printf("\n  issuer     : ");
        X509_NAME_print_ex_fp(stdout, X509_get_issuer_name(peer), 0, XN_FLAG_ONELINE);
        printf("\n");
        print_san(peer);
        X509_free(peer);
    }
    printf("  client cert: %s\n", cert ? cert : "(không gửi)");

    char req[512];
    snprintf(req, sizeof req, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", host);
    if (SSL_write(ssl, req, (int)strlen(req)) <= 0) die("SSL_write");
    char buf[256];
    int n = SSL_read(ssl, buf, sizeof buf - 1);                   // TLS 1.3: lỗi mTLS hiện ra ở đây
    if (n <= 0) die("SSL_read thất bại: server từ chối sau handshake?");
    buf[n] = '\0';
    buf[strcspn(buf, "\r\n")] = '\0';
    printf("  response   : %s\n", buf);

    SSL_shutdown(ssl);                                            // gửi close_notify
    SSL_free(ssl);
    close(fd);
    SSL_CTX_free(ctx);
    return 0;
}
