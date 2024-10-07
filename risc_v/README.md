Every assembly program was converted from C code, and there is still room for improvement, such as optimizing the choice of registers.
Below is the usage of each program:

1. **clz.s** - Counts leading zeros in `fp16_to_fp32.s`.
2. **convolution.s** - Performs convolution and outputs the result in FP16 format.
3. **convolution_v2.s** - Performs convolution and outputs the result in FP32 format.
4. **fp16_add.s** - Handles FP16 addition.
5. **fp16_mul.s** - Uses the M extension for mantissa multiplication.
6. **fp16_mul_cus.s** - Performs mantissa multiplication using bitwise operations.
7. **fp16_to_fp32.s** - Converts the result back into FP32 format."
