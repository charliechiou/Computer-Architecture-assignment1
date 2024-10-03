#include <stdio.h>
#include <stdint.h>
static inline int64_t getbit(int64_t value, int n)
{
    return (value >> n) & 1;
}

int64_t imul16(int64_t a, int64_t b)
{
    int64_t r = 0, a64 = (int64_t)a, b64 = (int64_t)b;
    for (int i = 0; i < 16; i++)
    {
        if (getbit(b64, i))
        {
            r += a64 << i;
        }
    }
    return r;
}

uint16_t fmul16(uint16_t a, uint16_t b)
{
    /* sign */
    int sign_a = a >> 15;
    int sign_b = b >> 15;

    /* mantissa */
    int32_t mantissa_a = (a & 0x3FF) | 0x400; // FP16: 10-bit mantissa + hidden bit
    int32_t mantissa_b = (b & 0x3FF) | 0x400;

    /* exponent */
    int32_t exp_a = ((a >> 10) & 0x1F); // FP16: 5-bit exponent
    int32_t exp_b = ((b >> 10) & 0x1F);

    /* 'r' = result */
    int64_t mantissa_result_tmp = imul16(mantissa_a, mantissa_b) >> 10; // Shift by FP16 mantissa bits (10 bits)
    int mshift = mantissa_result_tmp >> 24;                             // Adjust shift if necessary (simplified example for mshift calculation)

    int64_t mantissa_result = mantissa_result_tmp >> mshift;
    int32_t exp_result_tmp = exp_a + exp_b - 15; // Adjust exponent bias for FP16 (bias is 15)
    int32_t exp_resultr = mshift ? exp_result_tmp + 1 : exp_result_tmp;

    int sign_result = sign_a ^ sign_b;
    uint16_t r = (sign_result << 15) | ((exp_resultr & 0x1F) << 10) | (mantissa_result & 0x3FF); // Reconstruct FP16
    return r;
}

int main()
{
    // 測試浮點數乘法
    uint16_t fp16_a = 0x3a00; // 1.0
    uint16_t fp16_b = 0x4000; // 32.0
    int fp16_result = fmul16(fp16_a, fp16_b);
    printf("FP16 Multiply Result: 0x%04X\n", fp16_result);

    return 0;
}
