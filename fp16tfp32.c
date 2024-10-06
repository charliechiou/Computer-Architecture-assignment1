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

    return result;
}
void main()
{
    uint16_t x = 0x4800;
    int32_t y = fp16_to_fp32(x);
    printf("0x%08X \n", x);
    printf("0x%08X \n", y);
}