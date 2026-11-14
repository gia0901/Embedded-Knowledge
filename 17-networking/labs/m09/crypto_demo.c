// crypto_demo.c — SHA-256, HMAC-SHA256, AES-256-GCM bằng OpenSSL EVP API
// Build: gcc -O2 -Wall -Wextra -o crypto_demo crypto_demo.c -lcrypto
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <string.h>

static void hex(const char *label, const unsigned char *b, size_t n) {
    printf("%-10s", label);
    for (size_t i = 0; i < n; i++) printf("%02x", b[i]);
    printf("\n");
}

/* Trả về độ dài ciphertext, hoặc -1 nếu lỗi. IV mặc định của GCM là 12 byte. */
static int gcm_encrypt(const unsigned char *key, const unsigned char *iv,
                       const unsigned char *aad, int aad_len,
                       const unsigned char *pt, int pt_len,
                       unsigned char *ct, unsigned char tag[16]) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len = 0, total = -1;
    if (ctx &&
        EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) == 1 &&
        EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len) == 1 &&       /* AAD: xác thực, không mã hoá */
        EVP_EncryptUpdate(ctx, ct, &len, pt, pt_len) == 1) {
        total = len;
        if (EVP_EncryptFinal_ex(ctx, ct + total, &len) == 1 &&
            EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) == 1)
            total += len;
        else
            total = -1;
    }
    EVP_CIPHER_CTX_free(ctx);
    return total;
}

/* Trả về độ dài plaintext, hoặc -1 nếu tag không khớp (dữ liệu bị sửa). */
static int gcm_decrypt(const unsigned char *key, const unsigned char *iv,
                       const unsigned char *aad, int aad_len,
                       const unsigned char *ct, int ct_len,
                       const unsigned char tag[16], unsigned char *pt) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len = 0, total = -1;
    if (ctx &&
        EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) == 1 &&
        EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len) == 1 &&
        EVP_DecryptUpdate(ctx, pt, &len, ct, ct_len) == 1) {
        total = len;
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void *)tag);
        if (EVP_DecryptFinal_ex(ctx, pt + total, &len) > 0)              /* kiểm tra tag ở đây */
            total += len;
        else
            total = -1;
    }
    EVP_CIPHER_CTX_free(ctx);
    return total;
}

int main(void) {
    const char *msg = "{\"user\":\"alice\",\"role\":\"admin\"}";

    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;
    EVP_Digest(msg, strlen(msg), md, &md_len, EVP_sha256(), NULL);
    hex("SHA-256", md, md_len);

    const char *mac_key = "secret-key";
    unsigned char mac[EVP_MAX_MD_SIZE];
    unsigned int mac_len = 0;
    HMAC(EVP_sha256(), mac_key, (int)strlen(mac_key),
         (const unsigned char *)msg, strlen(msg), mac, &mac_len);
    hex("HMAC", mac, mac_len);

    unsigned char key[32], iv[12], tag[16], ct[256], out[256];
    if (RAND_bytes(key, sizeof key) != 1 || RAND_bytes(iv, sizeof iv) != 1) return 1;  /* CSPRNG */
    const unsigned char aad[] = "kid=k1";
    int n = gcm_encrypt(key, iv, aad, (int)sizeof aad - 1,
                        (const unsigned char *)msg, (int)strlen(msg), ct, tag);
    if (n < 0) return 1;
    hex("cipher", ct, (size_t)n);
    hex("tag", tag, sizeof tag);

    int m = gcm_decrypt(key, iv, aad, (int)sizeof aad - 1, ct, n, tag, out);
    if (m >= 0) { out[m] = '\0'; printf("decrypt   %s\n", (char *)out); }

    ct[0] ^= 0x01;                                                        /* sửa 1 bit */
    m = gcm_decrypt(key, iv, aad, (int)sizeof aad - 1, ct, n, tag, out);
    printf("tampered  %s\n", m < 0 ? "tag không khớp → từ chối" : "?! không phát hiện");
    return 0;
}
