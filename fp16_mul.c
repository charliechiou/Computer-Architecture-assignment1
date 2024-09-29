#include <stdio.h>
#include <stdint.h>

uint16_t fmul16(uint16_t ia, uint16_t ib)
{
    /* TODO: Special values like NaN and INF */

    /* sign */
    int sa = ia >> 15;
    int sb = ib >> 15;

    /* mantissa */
    int32_t ma = (ia & 0x3FF) | 0x400; // FP16: 10-bit mantissa + hidden bit
    int32_t mb = (ib & 0x3FF) | 0x400;

    /* exponent */
    int32_t ea = ((ia >> 10) & 0x1F); // FP16: 5-bit exponent
    int32_t eb = ((ib >> 10) & 0x1F);

    /* 'r' = result */
    int64_t mrtmp = ((int64_t)ma * mb) >> 10; // Shift by FP16 mantissa bits (10 bits)
    int mshift = mrtmp >> 24;                 // Adjust shift if necessary (simplified example for mshift calculation)

    int64_t mr = mrtmp >> mshift;
    int32_t ertmp = ea + eb - 15; // Adjust exponent bias for FP16 (bias is 15)
    int32_t er = mshift ? ertmp + 1 : ertmp;
    /* TODO: Handle overflow */
    int sr = sa ^ sb;
    uint16_t r = (sr << 15) | ((er & 0x1F) << 10) | (mr & 0x3FF); // Reconstruct FP16
    return r;
}

int main()
{
    // 測試浮點數乘法
    uint16_t fp16_a = 0x3C00; // 1.0
    uint16_t fp16_b = 0x4200; // 32.0
    int fp16_result = fmul16(fp16_a, fp16_b);
    printf("FP16 Multiply Result: 0x%04X\n", fp16_result);

    return 0;
}
