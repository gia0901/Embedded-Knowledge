# sum_values.S: long sum_values(const uint8_t* buf, size_t n)
# Cộng trường value (i32 big-endian ở offset 12) của n frame 16 byte. Viết tay theo System V ABI x86-64:
# tham số 1 trong rdi, tham số 2 trong rsi, kết quả trả trong rax. Chỉ dùng thanh ghi caller-saved.
# Bản BROKEN dùng rbx (callee-saved) mà không cất lại: vi phạm ABI. Lời giải Lab 1.4.
    .text
    .globl  sum_values
    .type   sum_values, @function
sum_values:
#ifdef BROKEN
    xor     %ebx, %ebx              # SAI: rbx thuộc về hàm gọi, phải push/pop nếu dùng
#define ACC %rbx
#else
#define ACC %rax
#endif
    xor     %eax, %eax              # rax = 0 (xor 32 bit tự xóa cả 64 bit)
    test    %rsi, %rsi
    jz      2f                      # n == 0: trả 0
1:  movl    12(%rdi), %edx          # nạp 4 byte value
    bswap   %edx                    # big-endian -> little-endian
    movslq  %edx, %rdx              # mở rộng có dấu lên 64 bit
    add     %rdx, ACC
    add     $16, %rdi               # sang frame kế
    dec     %rsi
    jnz     1b
2:
#ifdef BROKEN
    mov     %rbx, %rax
#endif
    ret
    .size   sum_values, .-sum_values
    .section .note.GNU-stack, "", @progbits
