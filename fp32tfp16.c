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
    float test_values[] = {0.0f, -0.0f, 1.0f, -1.0f, 65504.0f, 1.5f, 3.14159f, 0.00006103515625f, INFINITY, -INFINITY, NAN};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < num_tests; i++)
    {
        float value = test_values[i];
        uint16_t fp16 = fp32_to_fp16(value);
        printf("Float32: %f -> Float16 (hex): 0x%04X\n", value, fp16);
    }

    return 0;
}