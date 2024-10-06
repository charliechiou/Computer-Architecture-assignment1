.data
testcase1_x:
    .4byte 0x3C00 0x4000 0x0000 #input x
testcase1_h:
    .4byte 0x4200 0x4000 0x3c00 #input h
# expected output -> 0x4200 0x4800 0x4500 0x4000 0x0000

testcase2_x:
    .4byte 0x3c00 0x4000 0x3c00 #input x
testcase2_h:
    .4byte 0x4200 0x4000 0x3c00 #input h
# expected output -> 0x4200 0x4800 0x4800 0x4400 0x3c00
    
testcase3_x:
    .4byte 0x4500 0x4400 0x3c00 #input x
testcase3_h:
    .4byte 0x4000 0x3c00 0x3c00 #input h
# expected output ->  0x4900 0x4a80 0x4980 0x4500 0x3c00

testcase4_x:
    .4byte 0x3c00 0x4000 0x3c00 0x4400 #input x
testcase4_h:
    .4byte 0x4200 0x4000 0x3c00 #input h
# expected output ->  0x4200 0x4800 0x4800 0x4c00 0x4880 0x4400

y:
    .zero 20 #for testcase 1~3
    #.zero 24 #for testcase 4

str1:.string "Input x is : "
str2:.string "Input h is : "
str3:.string " "
str4:.string "\n"
str5:.string "Output y is : "

.text
printInputInit:
    la s0,testcase1_x
    la s1,testcase1_h
    mv a0,s0
    mv a1,s1
    jal printInput

main:
    li s2,3 # m(s2) = 3 for testcase 1~3
    #li s2,4 # m(s2) = 4 for testcase 4
    li s3,3 # n(s3) = 3
    li s4,0 # i(s4) = 0
    add s5,s2,s3
    addi s5,s5,-1 # s5 = m + n - 1   
    la s6,y #y array address
    
outer_loop:
    bge s4, s5, end_outer_loop
    li s7, 0        
    
inner_loop:
    blt s4, s7, end_inner_loop
    
check_j_less_m:
    blt s7, s2, check_ij_less_n
    j skip_inner_loop
    
check_ij_less_n:
    sub t1, s4, s7 
    blt t1, s3, calculate
    j skip_inner_loop

calculate:
    slli t0,s7,2
    add t0,t0,s0
    lw a0,0(t0)
    
    slli t1,t1,2
    add t1,t1,s1
    lw a1,0(t1)
    jal fp16_mul #uint16_t fp16_result = float_mul(x[j], h[i - j]);
    
    slli t0,s4,2
    add t0,t0,s6
    lw a1,0(t0)
    jal fp16_add #y[i] = fp16_bitwise_add(y[i], fp16_result);
    
    sw a0,0(t0)
    
    
skip_inner_loop:
    addi s7, s7, 1     # j++
    j inner_loop

end_inner_loop:
    addi s4,s4,1
    j outer_loop
    
end_outer_loop:
    mv a0,s6
    j printOutput
    
#################################################################
    
fp16_mul:
    #prologue
    addi sp,sp,-32
    sw s0,0(sp)
    sw s1,4(sp)
    sw s2,8(sp)
    sw s3,12(sp)
    sw s4,16(sp)
    sw s5,20(sp)
    sw s6,24(sp)
    sw s7,28(sp)
    sw ra,32(sp)
    
    mv s0,a0
    mv s1,a1
    
    srli s2,s0,15 #int sign_a(s2) = a(S0) >> 15;
    srli s3,s1,15 #int sign_b(S3) = b(s1) >> 15;
    
    andi t1,s0,0x3ff 
    ori s4,t1,0x400 #int32_t mantissa_a(s4) = (a & 0x3FF) | 0x400;
    
    andi t1,s1,0x3ff 
    ori s5,t1,0x400 #int32_t mantissa_b(s5) = (b & 0x3FF) | 0x400;
    
    srli t1,s0,10
    andi s6,t1,0x1f #int32_t exp_a(s6) = ((a >> 10) & 0x1F);
    
    srli t1,s1,10
    andi s7,t1,0x1f #int32_t exp_b(s7) = ((b >> 10) & 0x1F);
    
    #mul t1,s4,s5
    mv t0,ra
    mv a0,s4
    mv a1,s5
    jal ra,imul16
    mv t1,a0
    mv ra,t0
    
    srli t1,t1,10 #int64_t mantissa_result_tmp(t1) = ((int64_t)mantissa_a * mantissa_b) >> 10;
    
    srli t2,t1,24 #int mshift(t2) = mantissa_result_tmp >> 24;
    
    srl s4,t1,t2 #int64_t mantissa_result(s4) = mantissa_result_tmp >> mshift;
    
    add s6,s6,s7
    addi s6,s6,-15 #int32_t exp_result_tmp(s6) = exp_a + exp_b - 15;
    
    #int32_t exp_resultr(s6) = mshift ? exp_result_tmp + 1 : exp_result_tmp;
    beqz t2,mshift_zero
    addi s6,s6,1 
    
mshift_zero:
    xor s2,s2,s3 #int sign_result(S2) = sign_a ^ sign_b;
    
    slli s2,s2,15
    
    andi s6,s6,0x1f
    slli s6,s6,10
    
    andi s4,s4,0x3ff
    
    or t1,s2,s6
    or a0,t1,s4 

    #epilogue
    lw s0,0(sp)
    lw s1,4(sp)
    lw s2,8(sp)
    lw s3,12(sp)
    lw s4,16(sp)
    lw s5,20(sp)
    lw s6,24(sp)
    lw s7,28(sp)
    lw ra,32(sp)
    addi sp,sp,32
    
    ret
imul16:
    #a0 -> a, a1 -> b
    li a3,0
    li t1,0 #set i
    li t2,16 #set max i
loop:
    beq t1,t2,end_loop
    srl t3,a1,t1
    andi t3,t3,1 #getbit(b64, i)
    beqz t3,skip_loop
    sll t4,a0,t1
    add a3,a3,t4 #r += a64 << i;
    
skip_loop:
    addi t1,t1,1
    j loop
    
end_loop:
    mv a0,a3
    ret
    
#################################################################
    
fp16_add:
    #prologue
    addi sp,sp,-32
    sw s0,0(sp)
    sw s1,4(sp)
    sw s2,8(sp)
    sw s3,12(sp)
    sw s4,16(sp)
    sw s5,20(sp)
    sw s6,24(sp)
    sw s7,28(sp)
    sw ra,32(sp)
    
    mv s0,a0
    mv s1,a1
    
    li t1,0x8000
    and s2,s0,t1 #uint16_t sign_a(s2) = a(s0) & 0x8000;
    and s3,s1,t1 #uint16_t sign_b(s3) = b(S1) & 0x8000;
    
    li t1,0x7C00
    and s4,s0,t1
    srli s4,s4,10 #int exp_a(s4) = (a & 0x7C00) >> 10;
    and s5,s1,t1
    srli s5,s5,10 #int exp_b(s5) = (b & 0x7C00) >> 10;

    andi s6,s0,0x03FF #uint16_t mant_a(s6) = a & 0x03FF;
    andi s7,s1,0x03FF #uint16_t mant_b(s7) = b & 0x03FF;
    
    ori s6,s6,0x0400 #mant_a(s6) = ((mant_a) | 0x0400);
    ori s7,s7,0x0400 #mant_b(s7) = ((mant_b) | 0x0400);
    
    beq s4,s5,finish_align
    blt s4,s5,align_bit_aless
    mv t1,ra
    mv a0,s7 #a0 = mant_b(s7)
    sub a1,s4,s5 #a1 = exp_a(s4) - exp_b(s5)
    jal ra,align_mantissa 
    mv s7,a0 #mant_b(s7) = align_mantissa(mant_b, exp_a - exp_b);
    mv ra,t1
    mv s5,s4 #exp_b(s5) = exp_a(s4);

    j finish_align
    
align_bit_aless:
    mv t1,ra
    mv a0,s6 #a0 = mant_a(s6)
    sub a1,s5,s4 #a1 = exp_b(s5) - exp_a(s4)
    jal ra,align_mantissa 
    mv s6,a0 #mant_a(s6) = align_mantissa(mant_a, exp_b - exp_a);
    mv ra,t1
    mv s4,s5 #exp_a(s4) = exp_b(s5);
    
finish_align:
    # int exp_result = exp_a; -> exp_result(s4)
    beq s2,s3,sign_equal
    bge s6,s7,a_greater_b
    sub s6,s7,s6 #mant_result(s6) = mant_b(s7) - mant_a(s6);
    mv s2,s3 #sign_a(s2) = sign_b(s3); 
    j finish_mantissa
    
a_greater_b:
    sub s6,s6,s7 #mant_result(s6) = mant_a(s6) - mant_b(s7);
    j finish_mantissa
    
sign_equal:
    add s6,s6,s7 #mant_result(s6) = mant_a(s6) + mant_b(s7);
    
finish_mantissa:
    li t1,0x0800
    and t1,s6,t1
    beqz t1,finish_mantissa_normalization #if (mant_result & 0x0800)
    srli s6,s6,1 #mant_result >>= 1;
    addi s4,s4,1 #exp_result++;
    
finish_mantissa_normalization:
    andi s6,s6,0x03ff #mant_result(s6) &= 0x03FF;
    
    li t1,31
    bge s4,t1,exp_greater #if (exp_result >= 31)
    bge zero,s4,exp_less #else if (exp_result <= 0)
    j combine_result
    
exp_greater:
    li s4,31 #exp_result = 31;
    li s6,0 #mant_result = 0;
    j combine_result
    
exp_less:
    li s4,0 #exp_result = 0;
    li s6,0 #mant_result = 0;
    j combine_result
    
combine_result:
    slli t1,s4,10
    or t1,s2,t1
    or a0,t1,s6 #return sign_a | (exp_result << 10) | mant_result;
    
    #epilogue
    lw s0,0(sp)
    lw s1,4(sp)
    lw s2,8(sp)
    lw s3,12(sp)
    lw s4,16(sp)
    lw s5,20(sp)
    lw s6,24(sp)
    lw s7,28(sp)
    lw ra,32(sp)
    addi sp,sp,32

    
    ret
    
align_mantissa:
    
    blez a1,end_align_mantissa
    srl a0,a0,a1
end_align_mantissa:
    ret
    
#################################################################

printInput:
    mv t1,a0
    mv t2,a1
    la a0,str1
    li a7,4
    ecall #print "Input x is : "
    
    lw a0,0(t1)
    li a7,34
    ecall #print x[0]
    
    la a0,str3
    li a7,4
    ecall #print space
    
    lw a0,4(t1)
    li a7,34
    ecall #print x[1]
    
    la a0,str3
    li a7,4
    ecall #print space
    
    lw a0,8(t1)
    li a7,34
    ecall #print x[2]
    
    la a0,str3
    li a7,4
    ecall #print space
    
    ##### below is for testcase 4 #####
    
    #lw a0,12(t1)
    #li a7,34
    #ecall #print x[3]
    
    ###################################
    
    la a0,str4
    li a7,4
    ecall #next line
    
    la a0,str2
    li a7,4
    ecall #print "Input h is : "

    lw a0,0(t2)
    li a7,34
    ecall #print h[0]
    
    la a0,str3
    li a7,4
    ecall #print space
    
    lw a0,4(t2)
    li a7,34
    ecall #print h[1]
    
    la a0,str3
    li a7,4
    ecall #print space
    
    lw a0,8(t2)
    li a7,34
    ecall #print h[2]

    la a0,str4
    li a7,4
    ecall #next line
    
    ret
    
#################################################################
    
printOutput:
    mv a3,a0
    la a0,str5
    li a7,4
    ecall #print "Output x is : "
    
    lw a0,0(a3)
    jal fp16_to_fp32
    li a7,2
    ecall #print y[1]
    
    la a0,str3
    li a7,4
    ecall #print space
    
    lw a0,4(a3)
    jal fp16_to_fp32
    li a7,2    
    ecall #print y[2]
    
    la a0,str3
    li a7,4
    ecall #print space
    
    lw a0,8(a3)
    jal fp16_to_fp32
    li a7,2
    ecall #print y[3]
    
    la a0,str3
    li a7,4
    ecall #print space
    
    lw a0,12(a3)
    jal fp16_to_fp32
    li a7,2
    ecall #print y[4]
    
    la a0,str3
    li a7,4
    ecall #print space
    
    lw a0,16(a3)
    jal fp16_to_fp32
    li a7,2
    ecall #print y[5]
    
    la a0,str3
    li a7,4
    ecall #print space
    
    ##### below is for testcase 4 #####
    #lw a0,20(t6)
    #li a7,34
    #ecall #print y[6]
    
    #la a0,str3
    #li a7,4
    #ecall #print space
    ###################################
    
    li a7,10
    ecall
    
#################################################################

fp16_to_fp32:
    #prologue
    addi sp,sp,-20
    sw s0,0(sp)
    sw s1,4(sp)
    sw s2,8(sp)
    sw s3,12(sp)
    sw s4,20(sp)
    
    #start
    mv s0,a0
    slli s1, s0, 16 # w(s1) = (uint32_t)h << 16;
    li s2, 0x80000000
    and s2, s1, s2 # sign(s2) = w & UINT32_C(0x80000000);
    li s3,0x7FFFFFFF
    and s3, s1, s3 # nonsign(s3) = w & UINT32_C(0x7FFFFFFF);
    
    mv a0,s3
    mv t6,ra
    jal my_clz #renorm_shift = my_clz(nonsign);
    jal renorm_if
    mv s4, a0  #renorm_shift(s4) = renorm_shift > 5 ? renorm_shift - 5 : 0;
    mv ra,t6
    
    li t0,0x7F800000 #INT32_C(0x7F800000)
    li t1,0x04000000 
    add t1,s3,t1 # nonsign + 0x04000000
    srli t1,t1,8  # (nonsign + 0x04000000) >> 8
    and t0,t0,t1 # inf_nan_mask(t0) = (nonsign + 0x04000000) >> 8 & 0x7F800000
    
    addi t1,s3,-1
    srli t1,t1,31 #zero_mask(t1) = (int32_t)(nonsign - 1) >> 31;
    
    sll t2,s3,s4
    srli t2,t2,3
    li t3,0x70
    sub t3,t3,s4
    slli t3,t3,23
    add t2,t2,t3 #((nonsign << renorm_shift >> 3) + ((0x70 - renorm_shift) << 23))
    or t2,t2,t0 #(((nonsign << renorm_shift >> 3) + ((0x70 - renorm_shift) << 23)) | inf_nan_mask)
    
    li t4,0xFFFFFFFF
    xor t1,t1,t4 # ~zero_mask(~t1)
    
    and t2,t2,t1 #((((nonsign << renorm_shift >> 3) + ((0x70 - renorm_shift) << 23)) | inf_nan_mask) & ~zero_mask)
    
    or a0,s2,t2
    
    #epilogue
    lw s4,20(sp)
    lw s3,12(sp)
    lw s2,8(sp)
    lw s1,4(sp)
    lw s0,0(sp)
    addi sp,sp,20
    
    ret
    
my_clz:
    addi sp,sp,-20
    sw s0,0(sp)
    sw s1,4(sp)
    sw s2,8(sp)
    sw s3,12(sp)
    sw s4,20(sp)
    
    li s0, 0 #set count
    li s1, 31 #set i
    
clz_loop:
    bltz s1, clz_done #if i<0, escape the loop
    li s2, 1 
    sll s3, s2, s1
    and s4, s3, a0
    bnez s4, clz_done
    addi s0, s0, 1
    addi s1, s1, -1
    j clz_loop
    
clz_done:    
    mv a0, s0
    lw s4,20(sp)
    lw s3,12(sp)
    lw s2,8(sp)
    lw s1,4(sp)
    lw s0,0(sp)
    addi sp,sp,20
    ret
    
renorm_if:
    li t0, 5
    blt t0,a0,renorm_if_true
    li a0,0
    ret
    
renorm_if_true:
    addi a0,a0,-5
    ret