#include <stdio.h>
#include <stdint.h>

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

// 計算最高有效位的位置，使用 my_clz 函數
int highest_bit_pos(uint32_t value)
{
    if (value == 0)
        return -1; // 處理零的情況

    return 31 - my_clz(value); // 使用 my_clz 計算最高位位置
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
    // 測試浮點數乘法
    int fp16_a = 0x3C00;
    int fp16_b = 0x4200;
    int fp16_result = float_mul(fp16_a, fp16_b);
    printf("FP16 Multiply Result: 0x%04X\n", fp16_result);

    int fp16_a = 0x4400;
    int fp16_b = 0x3C00;
    int fp16_result = float_mul(fp16_a, fp16_b);
    printf("FP16 Multiply Result: 0x%04X\n", fp16_result);

    int fp16_a = 0x3C00;
    int fp16_b = 0x4200;
    int fp16_result = float_mul(fp16_a, fp16_b);
    printf("FP16 Multiply Result: 0x%04X\n", fp16_result);

    int fp16_a = 0x3C00;
    int fp16_b = 0x4200;
    int fp16_result = float_mul(fp16_a, fp16_b);
    printf("FP16 Multiply Result: 0x%04X\n", fp16_result);

    return 0;
}
