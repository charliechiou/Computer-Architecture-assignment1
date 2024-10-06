#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
    mant_a = (mant_a | 0x0400);
    mant_b = (mant_b | 0x0400);

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

    // 組合符號、指數和尾數為最終的 FP16 數值
    return sign_a | (exp_result << 10) | mant_result;
}

// 16 位浮點數乘法
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
    if ((res_mant >> 21) & 1)
    {                    // 如果最高位在第 21 位
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
    // 固定的 FP16 輸入值
    uint16_t x[] = {0x3C00, 0x4000, 0x0000}; // FP16: 1.0, 2.0, 0.0
    uint16_t h[] = {0x4200, 0x4000, 0x3c00}; // FP16: 3.0, 3.0, 1.0
    uint16_t y[20] = {0};                    // 假設卷積結果的最大長度
    int i, j, m = 3, n = 3;

    // 計算卷積，使用 FP16 相乘和相加
    for (i = 0; i < m + n - 1; i++)
    {
        y[i] = 0;
        for (j = 0; j <= i; j++)
        {
            if (j < m && (i - j) < n)
            {
                // 使用 FP16 進行乘法
                uint16_t fp16_result = float_mul(x[j], h[i - j]);
                // 使用 FP16 直接相加
                y[i] = fp16_bitwise_add(y[i], fp16_result);
            }
        }
    }

    // 輸出卷積結果
    printf("Convoluted sequence is:\n");
    for (i = 0; i < m + n - 1; i++)
    {
        printf("y[%d] = 0x%04X\n", i, y[i]);
    }

    return 0;
}
