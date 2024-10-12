#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*Addition */
// Aligning the mantissa based on the shift value
uint16_t align_mantissa(uint16_t mant, int shift)
{
    if (shift > 0)
    {
        return mant >> shift; // Right-shift the mantissa if shift > 0
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
            mant_result = mant_a - mant_b; // compare and subtraction
        }
        else
        {
            mant_result = mant_b - mant_a; // subtraction
            sign_a = sign_b;               // change the sign bit
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

/*Multiplication*/
// Get the nth bit of a value
static inline int64_t getbit(int64_t value, int n)
{
    return (value >> n) & 1;
}

// FP16 integer multiplication
int64_t imul16(int64_t a, int64_t b)
{
    int64_t r = 0, a64 = (int64_t)a, b64 = (int64_t)b;
    for (int i = 0; i < 16; i++)
    {
        if (getbit(b64, i))
        {
            r += a64 << i;
        }
    }
    return r;
}

// FP16 multiplication
uint16_t fmul16(uint16_t a, uint16_t b)
{
    /* sign */
    int sign_a = a >> 15;
    int sign_b = b >> 15;

    /* mantissa */
    int32_t mantissa_a = (a & 0x3FF) | 0x400; // FP16: 10-bit mantissa + hidden bit
    int32_t mantissa_b = (b & 0x3FF) | 0x400;

    /* exponent */
    int32_t exp_a = ((a >> 10) & 0x1F); // FP16: 5-bit exponent
    int32_t exp_b = ((b >> 10) & 0x1F);

    /* Perform the mantissa multiplication using the imul16 function */
    int64_t mantissa_result_tmp = imul16(mantissa_a, mantissa_b) >> 10; // Shift by FP16 mantissa bits (10 bits)
    int mshift = mantissa_result_tmp >> 24;                             // Adjust shift if necessary (simplified example for mshift calculation)

    /* Normalize the mantissa and adjust the exponent */
    int64_t mantissa_result = mantissa_result_tmp >> mshift;
    int32_t exp_result_tmp = exp_a + exp_b - 15; // Adjust exponent bias for FP16 (bias is 15)
    int32_t exp_resultr = mshift ? exp_result_tmp + 1 : exp_result_tmp;

    /* Determine the sign of the result */
    int sign_result = sign_a ^ sign_b;

    /* Reconstruct the final 16-bit floating-point number */
    uint16_t r = (sign_result << 15) | ((exp_resultr & 0x1F) << 10) | (mantissa_result & 0x3FF); // Reconstruct FP16
    return r;
}

int main()
{
    uint16_t x[] = {0x3C00, 0x4000, 0x0000}; // FP16: 1.0, 2.0, 0.0
    uint16_t h[] = {0x4200, 0x4000, 0x3c00}; // FP16: 3.0, 2.0, 1.0
    uint16_t y[20] = {0};
    int i, j, m = 3, n = 3;

    for (i = 0; i < m + n - 1; i++)
    {
        y[i] = 0;
        for (j = 0; j <= i; j++)
        {
            if (j < m && (i - j) < n)
            {
                uint16_t fp16_result = fmul16(x[j], h[i - j]);
                y[i] = fp16_bitwise_add(y[i], fp16_result);
            }
        }
    }

    printf("Convoluted sequence is:\n");
    for (i = 0; i < m + n - 1; i++)
    {
        printf("y[%d] = 0x%04X\n", i, y[i]);
    }
    /*
    Expectied:
    0x4200 -> 3.0
    0x4800 -> 8.0
    0x4500 -> 5.0
    0x4000 -> 2.0
    0x0000 -> 0.0
    */

    return 0;
}
