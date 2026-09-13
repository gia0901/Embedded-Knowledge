/* =============================================================================
 * COD-005 — Tu cai memcpy + strlen           Phien B2 · 2026-09-13 · DIEM 3/4
 * Log: ../../sessions/2026-09-13--B2--coding.md
 *
 * ✅ DUOC
 *   [D1] Cast void* -> unsigned char* : dung ban chat, memcpy lam viec tren BYTE
 *   [D2] Giu con tro goc de tra ve dst (khong dung d da bi tang)
 *   [D3] strlen tra (p - s), khong cong 1 : DUNG
 *   [D4] Nhip (1) hoi 4 cau spec that su tot (n vuot src? tra gi khi loi?
 *        co copy NUL? string khong co NUL?)
 *   [D5] Nhip (4) sau khi duoc nhac: bang dry-run 3 vong CHINH XAC
 *   [D6] Du doan 1d "AAAAAAAAA" cho ham CUA MINH la DUNG (da do, xem duoi)
 *
 * ❌ MAT DIEM
 *   [L1] Nhip (1) KHONG hoi ve CHONG LAN (overlap) — cau spec quan trong
 *        NHAT cua memcpy, vi do la dieu kien duy nhat trong hop dong chuan
 *        ma caller phai bao dam (tham so la `restrict`).
 *   [L2] Chu ky thieu `const` o src.
 *   [L3] Nhip (2) noi "copy du n PHAN TU" — phai la BYTE. Chinh vi vay moi
 *        phai cast void* -> unsigned char*.
 *   [L4] Nhip (4) lan dau nop main() co test case = CHAY MAY, khong phai
 *        dry-run bang tay. Bai nay lam TREN GIAY.
 *   [L5] Nhip (5) dat cau hoi thay vi neu lo hong tu biet.
 *
 * 🔬 DO THAT tren may nay (gcc 11.4.0, x86-64), vung CHONG LAN 8 byte:
 *        my_memcpy : AAAAAAAAA   <- vong lap byte TIEN, tu boi len chinh no
 *        memcpy    : AABCDEFGH   <- glibc copy nguoc/theo khoi, "may ma" dung
 *        memmove   : AABCDEFGH   <- DAM BAO dung theo hop dong
 *    => Du doan cua ung vien dung cho ham cua minh. File test cua ban goc
 *       goi memcpy CUA THU VIEN (khong phai my_memcpy) nen thay ket qua khac.
 *    => Bai hoc: glibc "may ma dung" o kich thuoc/alignment nay. Doi kich
 *       thuoc la doi ket qua — xem COD-016: -O0 sach, -O2 lech tu byte 17.
 * ========================================================================== */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ---------------------------------------------------------------------------
 * PHAN 1 — BAN UNG VIEN NOP, GIU NGUYEN TUNG DONG
 * ------------------------------------------------------------------------- */
void *sub_memcpy(void *dst, void *src, size_t n) {   /* [L2] src thieu const */
    unsigned char *d = (unsigned char *)dst;         /* [D1] dung */
    const unsigned char *s = (const unsigned char *)src;

    while (n--) {                                     /* [D5] dry-run dung */
        *d++ = *s++;
    }
    return dst;                                       /* [D2] dung */
}
/* [L1] Khong o dau — trong code lan trong nhip (1) — nhac toi CHONG LAN.
 *      Ham nay copy TIEN, nen dst > src va hai vung giao nhau => boi (smear). */

size_t sub_strlen(const char *s) {
    const char *p = s;

    while (*p) {
        p++;
    }
    return (size_t)(p - s);   /* [D3] dung. Luc dung, p tro vao '\0' */
}

/* ---------------------------------------------------------------------------
 * PHAN 2 — BAN SUA
 * Giu nguyen moi quyet dinh hop ly cua ung vien (vong lap byte, con tro
 * chay, tra ve dst). Chi va dung 2 loi da danh nhan: [L1] va [L2].
 * ------------------------------------------------------------------------- */
void *fix_memcpy(void *restrict dst, const void *restrict src, size_t n)
{
    /* [L2] da sua: src la `const void*`, dung chu ky chuan.
     * `restrict` o ca hai tham so la cach CHUAN dat hop dong "khong chong lan"
     * vao chinh chu ky ham — trinh bien dich duoc phep gia dinh dieu do. */
    unsigned char       *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

/* [L1] da sua: vung CHONG LAN thi phai dung ham nay, khong dung fix_memcpy. */
void *fix_memmove(void *dst, const void *src, size_t n)
{
    unsigned char       *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    if (d == s || n == 0) return dst;

    if (d < s) {                    /* dst truoc src -> copy TIEN an toan */
        while (n--) *d++ = *s++;
    } else {                        /* dst sau src   -> phai copy NGUOC */
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

size_t fix_strlen(const char *s)
{
    const char *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

/* ---------------------------------------------------------------------------
 * PHAN 3 — TEST cac ca bien da ban trong phien
 * ------------------------------------------------------------------------- */
static void show(const char *tag, const char *b) { printf("%-22s %s\n", tag, b); }

int main(void)
{
    /* --- ca thuong --- */
    char dst[16] = {0};
    fix_memcpy(dst, "hello", 5);
    assert(fix_strlen(dst) == 5);
    show("copy thuong:", dst);

    /* --- ca bien strlen --- */
    assert(fix_strlen("")      == 0);   /* chuoi rong        */
    assert(fix_strlen("H")     == 1);   /* mot ky tu         */
    assert(fix_strlen("Hello") == 5);
    puts("strlen ca bien:       OK (0, 1, 5)");

    /* --- ca bien memcpy: n == 0 phai KHONG dung toi con tro --- */
    char keep[8] = "ABCDEFG";
    fix_memcpy(keep, "zzz", 0);
    assert(fix_strlen(keep) == 7);
    show("n == 0 giu nguyen:", keep);

    /* --- [L1] CHONG LAN: cho thay hai ham khac nhau the nao --- */
    char naive[16] = "ABCDEFGH";
    char safe [16] = "ABCDEFGH";

    /* Goi ban UNG VIEN (copy tien) tren vung chong lan -> boi len */
    sub_memcpy(naive + 1, naive, 8);
    /* Goi ban DUNG */
    fix_memmove(safe + 1, safe, 8);

    show("sub_memcpy (sai):", naive);   /* AAAAAAAAA */
    show("fix_memmove (dung):", safe);  /* AABCDEFGH */

    /* Chieu nguoc lai: dst < src, ca hai deu dung */
    char back1[16] = "ABCDEFGH", back2[16] = "ABCDEFGH";
    sub_memcpy (back1, back1 + 1, 7);
    fix_memmove(back2, back2 + 1, 7);
    show("dst < src (sub):", back1);
    show("dst < src (fix):", back2);

    puts("\n=> memcpy KHONG dam bao gi khi hai vung chong lan (tham so restrict).");
    puts("=> Chong lan thi dung memmove. Chi phi gan bang nhau.");
    return 0;
}
