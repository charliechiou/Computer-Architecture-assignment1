#include <stdint.h>
#include <stdio.h>

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

int main()
{
    uint16_t a = 0x3C00; // FP16: 1.0
    uint16_t b = 0x4000; // FP16: 2.0

    uint16_t result = fp16_bitwise_add(a, b); // fp16 addition

    printf("Result (in FP16): 0x%04X\n", result); // expcted 0x4200
    return 0;
}
