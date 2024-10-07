I developed the program step by step, function by function, and eventually combined them into the final result. 
Below is the usage of each function listed in the order in which I created them:

1. **convolution.m** - MATLAB code to simulate the result of convolution and display the results in a chart.
2. **convolution_origin.c** - The original convolution code for floating-point numbers.
3. **fp16tfp32.c** - For understanding the transformation from FP16 to FP32.
4. **fp32tfp16.c** - For understanding the transformation from FP32 to FP16.
5. **fp32_mul.c** - Referenced from the Quiz 1 of last year.
6. **fp16_mul.c** - Adapted from fp32_mul.c to handle FP16.
7. **convolution_fp16_v1.c** - Before successfully implementing fp16_add, I converted the multiplication result to FP32 and then added them together.
8. **fp16_add.c** - Handcrafted bitwise addition for FP16 convolution.
9. **convolution_fp16_v2.c** - Combined all the functions to produce the final result for FP16 convolution."
