#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint16_t align_mantissa(uint16_t mant, int shift)
{
    if (shift > 0)
    {
        return mant >> shift;
    }
    return mant;
}

// bitwise addition
uint16_t fp16_bitwise_add(uint16_t a, uint16_t b)
{
    // extract the sign, exponential, mantissa
    uint16_t sign_a = a & 0x8000;
    uint16_t sign_b = b & 0x8000;
    int exp_a = (a & 0x7C00) >> 10;
    int exp_b = (b & 0x7C00) >> 10;
    uint16_t mant_a = a & 0x03FF;
    uint16_t mant_b = b & 0x03FF;

    // adding mantissa hidding 1
    mant_a = (mant_a | 0x0400);
    mant_b = (mant_b | 0x0400);

    // move smaller mantissa to the right
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

    // compare the nnumbers and do the addition
    uint16_t mant_result;
    int exp_result = exp_a;
    if (sign_a == sign_b)
    {
        mant_result = mant_a + mant_b; // both + or -
    }
    else
    {
        if (mant_a >= mant_b)
        {
            mant_result = mant_a - mant_b; // compare and minus
        }
        else
        {
            mant_result = mant_b - mant_a;
            sign_a = sign_b; // change the sign bit
        }
    }

    // if there is a carry-over, increase the exponent by 1
    if (mant_result & 0x0800)
    {
        mant_result >>= 1;
        exp_result++;
    }

    // remove hidding 1
    mant_result &= 0x03FF;

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

    // construct together
    return sign_a | (exp_result << 10) | mant_result;
}

int float_mul(int f1, int f2)
{
    int res_exp = 0;
    int res_frac = 0;

    int exp1 = (f1 & (((1 << 5) - 1) << 10)) >> 10;
    int exp2 = (f2 & (((1 << 5) - 1) << 10)) >> 10;
    int frac1 = (f1 & ((1 << 10) - 1)) | (1 << 10);
    int frac2 = (f2 & ((1 << 10) - 1)) | (1 << 10);

    res_exp = exp1 + exp2 - 15;

    int64_t res_mant = (int64_t)frac1 * (int64_t)frac2;

    if ((res_mant >> 21) & 1)
    {
        res_mant >>= 11;
        res_exp += 1;
    }
    else
    {
        res_mant >>= 10;
    }
    res_frac = res_mant & ((1 << 10) - 1);

    if (res_exp <= 0)
        res_exp = 0;
    else if (res_exp >= (1 << 5) - 1)
        res_exp = (1 << 5) - 1, res_frac = 0;

    int result = (res_exp << 10) | res_frac;
    return result;
}

int main()
{
    uint16_t x[] = {0x3C00, 0x4000, 0x0000}; // FP16: 1.0, 2.0, 0.0
    uint16_t h[] = {0x4200, 0x4000, 0x3c00}; // FP16: 3.0, 3.0, 1.0
    uint16_t y[20] = {0};
    int i, j, m = 3, n = 3;

    for (i = 0; i < m + n - 1; i++)
    {
        y[i] = 0;
        for (j = 0; j <= i; j++)
        {
            if (j < m && (i - j) < n)
            {
                uint16_t fp16_result = float_mul(x[j], h[i - j]);
                y[i] = fp16_bitwise_add(y[i], fp16_result);
            }
        }
    }

    printf("Convoluted sequence is:\n");
    for (i = 0; i < m + n - 1; i++)
    {
        printf("y[%d] = 0x%04X\n", i, y[i]);
    }

    return 0;
}
