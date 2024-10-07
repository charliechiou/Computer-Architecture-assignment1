#include <stdio.h>
#include <stdint.h>
#include <string.h>

// customize clz
static inline int my_clz(uint32_t x)
{
    int count = 0;
    for (int i = 31; i >= 0; --i)
    {
        if (x & (1U << i))
            break;
        count++;
    }
    return count;
}

// bits to fp32
static inline float bits_to_fp32(uint32_t w)
{
    union
    {
        uint32_t as_bits;
        float as_value;
    } fp32 = {.as_bits = w};
    return fp32.as_value;
}

// fp16 to fp32
static inline float fp16_to_fp32(uint16_t h)
{
    const uint32_t w = (uint32_t)h << 16;              // Extends the fp16 to fp32 bits
    const uint32_t sign = w & UINT32_C(0x80000000);    // Isolates the sign bit
    const uint32_t nonsign = w & UINT32_C(0x7FFFFFFF); // Extracts the mantissa and exponent

    uint32_t renorm_shift = my_clz(nonsign); // Indicate howmany bits the mantissa needs to be shifted
    // for renorm number the shift will be 0,shifting a denormalized number will move the mantissa into the exponent
    renorm_shift = renorm_shift > 5 ? renorm_shift - 5 : 0;
    /*
     *  inf_nan_mask ==
     *                   0x7F800000 if the half-precision number is
     *                   NaN or infinity (exponent of 15)
     *                   0x00000000 otherwise
     */
    const int32_t inf_nan_mask = ((int32_t)(nonsign + 0x04000000) >> 8) & INT32_C(0x7F800000);
    const int32_t zero_mask = (int32_t)(nonsign - 1) >> 31;
    int32_t result = sign | ((((nonsign << renorm_shift >> 3) + ((0x70 - renorm_shift) << 23)) | inf_nan_mask) & ~zero_mask);

    return bits_to_fp32(result);
}

int highest_bit_pos(uint32_t value)
{
    if (value == 0)
        return -1; 

    return 31 - my_clz(value);
}

// fp16 multiplication
uint16_t float_mul(uint16_t ia, uint16_t ib)
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
    // fp16 input
    uint16_t x[] = {0x3C00, 0x4000, 0x0000}; // FP16: 1.0, 2.0, 0.0
    uint16_t h[] = {0x4200, 0x4400, 0x0000}; // FP16: 3.0, 4.0, 0.0
    float y[20] = {0};
    int i, j, m = 3, n = 3;

    // Perform convolution by multiplying using FP16, then convert to FP32 for addition.
    for (i = 0; i < m + n - 1; i++)
    {
        y[i] = 0.0f;
        for (j = 0; j <= i; j++)
        {
            if (j < m && (i - j) < n)
            {
                // multiplying using FP16
                uint16_t fp16_result = float_mul(x[j], h[i - j]);
                // convert to FP32 for addition
                y[i] += fp16_to_fp32(fp16_result);
            }
        }
    }

    // output
    printf("Convoluted sequence is:\n");
    for (i = 0; i < m + n - 1; i++)
    {
        printf("y[%d] = %f\n", i, y[i]);
    }

    return 0;
}
