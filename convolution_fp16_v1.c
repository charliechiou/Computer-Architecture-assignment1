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

// 計算最高有效位的位置，使用 `my_clz` 函數
int highest_bit_pos(uint32_t value)
{
    if (value == 0)
        return -1; // 處理零的情況

    return 31 - my_clz(value); // 使用 `my_clz` 計算最高位位置
}

// fp16 multiplication
int float_mul(int f1, int f2)
{
    int res_exp = 0;
    int res_frac = 0;

    // 取得指數和尾數
    int exp1 = (f1 & (((1 << 5) - 1) << 10)) >> 10;
    int exp2 = (f2 & (((1 << 5) - 1) << 10)) >> 10;
    int frac1 = (f1 & ((1 << 10) - 1)) | (1 << 10);
    int frac2 = (f2 & ((1 << 10) - 1)) | (1 << 10);

    // 加上指數，並去除雙重偏置
    res_exp = exp1 + exp2 - 15;

    // 相乘尾數
    int64_t res_mant = (int64_t)frac1 * (int64_t)frac2; // 11 位 * 11 位 → 22 位

    // 根據最高位來調整尾數
    if (highest_bit_pos(res_mant) == 21) // 如果最高位在第 21 位
    {
        res_mant >>= 11; // 向右移 11 位
        res_exp += 1;    // 調整指數
    }
    else
    {
        res_mant >>= 10; // 向右移 10 位
    }
    res_frac = res_mant & ((1 << 10) - 1); // 移除隱含位元

    // 如果指數超過範圍，則設為無窮大或 NaN
    if (res_exp <= 0)
        res_exp = 0; // 設為次正常數或零
    else if (res_exp >= (1 << 5) - 1)
        res_exp = (1 << 5) - 1, res_frac = 0; // 無窮大或 NaN

    // 構建最終的浮點數結果
    int result = (res_exp << 10) | res_frac;
    return result;
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
