#include <stdint.h>
#include <stdio.h>

// 加法位移並對齊指數
uint16_t align_mantissa(uint16_t mant, int shift)
{
    if (shift > 0)
    {
        return mant >> shift; // 若指數不一致，將尾數右移
    }
    return mant;
}

// 位運算方式實現 FP16 加法
uint16_t fp16_bitwise_add(uint16_t a, uint16_t b)
{
    // 提取符號、指數和尾數
    uint16_t sign_a = a & 0x8000;
    uint16_t sign_b = b & 0x8000;
    int exp_a = (a & 0x7C00) >> 10;
    int exp_b = (b & 0x7C00) >> 10;
    uint16_t mant_a = a & 0x03FF;
    uint16_t mant_b = b & 0x03FF;

    // 正規化尾數（加上隱含的 1）
    mant_a = ((mant_a) | 0x0400);
    mant_b = ((mant_b) | 0x0400);

    // 對齊指數，將較小的尾數右移
    if (exp_a > exp_b)
    {
        mant_b = align_mantissa(mant_b, exp_a - exp_b);
        exp_b = exp_a;
    }
    else if (exp_b > exp_a)
    {
        mant_a = align_mantissa(mant_a, exp_b - exp_a);
        exp_a = exp_b;
    }

    // 根據符號進行加減
    uint16_t mant_result;
    int exp_result = exp_a;
    if (sign_a == sign_b)
    {
        mant_result = mant_a + mant_b; // 同號相加
    }
    else
    {
        if (mant_a >= mant_b)
        {
            mant_result = mant_a - mant_b; // 相異號，大的減小的
        }
        else
        {
            mant_result = mant_b - mant_a;
            sign_a = sign_b; // 改變結果的符號
        }
    }

    // 規則化結果，如果有進位，指數加 1
    if (mant_result & 0x0800)
    { // 進位導致尾數溢出

        mant_result >>= 1;
        exp_result++;
    }

    // 移除隱含的 1 並處理結果
    mant_result &= 0x03FF;

    // 防止指數溢出
    if (exp_result >= 31)
    {
        exp_result = 31;
        mant_result = 0;
    }
    else if (exp_result <= 0)
    {
        exp_result = 0;
        mant_result = 0;
    }
    printf("sign (in FP16): 0x%04X\n", sign_a);          // 輸出結果 expcted 0x4200
    printf("exp (in FP16): 0x%04X\n", exp_result);       // 輸出結果 expcted 0x4200
    printf("mantissa (in FP16): 0x%04X\n", mant_result); // 輸出結果 expcted 0x4200

    // 組合符號、指數和尾數為最終的 FP16 數值
    return sign_a | (exp_result << 10) | mant_result;
}

int main()
{
    uint16_t a = 0x3C00; // FP16: 1.0
    uint16_t b = 0x4000; // FP16: 2.0

    uint16_t result = fp16_bitwise_add(a, b); // 使用位運算的 FP16 加法

    printf("Result (in FP16): 0x%04X\n", result); // 輸出結果 expcted 0x4200
    return 0;
}
