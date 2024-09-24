#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

static inline uint32_t fp32_to_bits(float f)
{
    uint32_t bits;
    memcpy(&bits, &f, sizeof(bits));
    return bits;
}

static inline float bits_to_fp32(uint32_t bits)
{
    float f;
    memcpy(&f, &bits, sizeof(f));
    return f;
}

// 將 FP32 轉換為 FP16 的函數
static inline uint16_t fp32_to_fp16(float f)
{
    const float scale_to_inf = 0x1.0p+112f;
    const float scale_to_zero = 0x1.0p-110f;
    float base = (fabsf(f) * scale_to_inf) * scale_to_zero;

    const uint32_t w = fp32_to_bits(f);
    const uint32_t shl1_w = w + w;
    const uint32_t sign = w & UINT32_C(0x80000000);
    uint32_t bias = shl1_w & UINT32_C(0xFF000000);
    if (bias < UINT32_C(0x71000000))
        bias = UINT32_C(0x71000000);

    base = bits_to_fp32((bias >> 1) + UINT32_C(0x07800000)) + base;
    const uint32_t bits = fp32_to_bits(base);
    const uint32_t exp_bits = (bits >> 13) & UINT32_C(0x00007C00);
    const uint32_t mantissa_bits = bits & UINT32_C(0x00000FFF);
    const uint32_t nonsign = exp_bits + mantissa_bits;
    return (sign >> 16) | (shl1_w > UINT32_C(0xFF000000) ? UINT16_C(0x7E00) : nonsign);
}

int main()
{
    // 增加陣列大小以確保足夠空間，並考慮卷積結果的最大長度
    uint16_t x_fp16[30] = {0}, h_fp16[30] = {0}, y_fp16[60] = {0}; // 假設序列長度不超過 30
    float x_fp32[30] = {0}, h_fp32[30] = {0}, y_fp32[60] = {0};    // 假設序列長度不超過 30

    int i, j, m, n;

    // 輸入兩個序列的長度
    printf("Enter the length of the first sequence, m = ");
    scanf("%d", &m);
    printf("Enter the length of the second sequence, n = ");
    scanf("%d", &n);

    // 輸入第一個序列
    printf("Enter the first sequence x[n]: ");
    for (i = 0; i < m; i++)
    {
        scanf("%f", &x_fp32[i]);
        x_fp16[i] = fp32_to_fp16(x_fp32[i]);
    }

    // 輸入第二個序列
    printf("Enter the second sequence h[n]: ");
    for (i = 0; i < n; i++)
    {
        scanf("%f", &h_fp32[i]);
        h_fp16[i] = fp32_to_fp16(h_fp32[i]);
    }

    // 計算卷積
    for (i = 0; i < m + n - 1; i++)
    {
        y_fp16[i] = 0;
        for (j = 0; j <= i; j++)
        {
            if (j < m && (i - j) < n)
            { // 確保不超出 x 和 h 的範圍
                y_fp16[i] += x_fp16[j] * h_fp16[i - j];
            }
        }
    }

    // 輸出卷積結果
    printf("Convoluted sequence is :\n");
    for (i = 0; i < m + n - 1; i++)
    {
        printf("y[%d] = %f\n", i, y_fp16[i]);
    }

    return 0;
}
