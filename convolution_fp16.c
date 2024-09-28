#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// 用於將 FP32 轉換為位元表示
static inline uint32_t fp32_to_bits(float f)
{
    uint32_t bits;
    memcpy(&bits, &f, sizeof(bits));
    return bits;
}

// 用於將位元表示轉換為 FP32
static inline float bits_to_fp32(uint32_t bits)
{
    float f;
    memcpy(&f, &bits, sizeof(f));
    return f;
}

// 將 FP16 轉換為 FP32 的函數
static inline float fp16_to_fp32(uint16_t h)
{
    const uint32_t w = (uint32_t)h << 16;
    const uint32_t sign = w & UINT32_C(0x80000000);
    const uint32_t two_w = w + w;

    const uint32_t exp_offset = UINT32_C(0xE0) << 23;
    const float exp_scale = 0x1.0p-112f;
    const float normalized_value =
        bits_to_fp32((two_w >> 4) + exp_offset) * exp_scale;

    const uint32_t mask = UINT32_C(126) << 23;
    const float magic_bias = 0.5f;
    const float denormalized_value =
        bits_to_fp32((two_w >> 17) | mask) - magic_bias;

    const uint32_t denormalized_cutoff = UINT32_C(1) << 27;
    const uint32_t result =
        sign | (two_w < denormalized_cutoff ? fp32_to_bits(denormalized_value)
                                            : fp32_to_bits(normalized_value));
    return bits_to_fp32(result);
}

// 自定義 `my_clz` 用於計算最高有效位
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

// 計算最高有效位的位置，使用 `my_clz` 函數
int highest_bit_pos(uint32_t value)
{
    if (value == 0)
        return -1; // 處理零的情況

    return 31 - my_clz(value); // 使用 `my_clz` 計算最高位位置
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
    clock_t start, end;   // 定義 clock_t 型別的變數來存放時間
    double cpu_time_used; // 用於儲存計算出來的運行時間

    start = clock(); // 記錄開始時間

    // 固定的 FP16 輸入值
    uint16_t x[] = {0x3C00, 0x4000, 0x0000}; // FP16: 1.0, 2.0, 0.0
    uint16_t h[] = {0x4200, 0x4400, 0x0000}; // FP16: 3.0, 4.0, 0.0
    float y[60] = {0};                       // 假設卷積結果的最大長度
    int i, j, m = 3, n = 3;

    // 計算卷積，使用 FP16 相乘，然後轉換為 FP32 再加法
    for (i = 0; i < m + n - 1; i++)
    {
        y[i] = 0.0f;
        for (j = 0; j <= i; j++)
        {
            if (j < m && (i - j) < n)
            {
                // 使用 FP16 進行乘法
                uint16_t fp16_result = float_mul(x[j], h[i - j]);
                // 將結果轉換為 FP32，進行累加
                y[i] += fp16_to_fp32(fp16_result);
            }
        }
    }

    end = clock(); // 記錄結束時間

    // 輸出卷積結果
    printf("Convoluted sequence is:\n");
    for (i = 0; i < m + n - 1; i++)
    {
        printf("y[%d] = %f\n", i, y[i]);
    }

    return 0;
}
