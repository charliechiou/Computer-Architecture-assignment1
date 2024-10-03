.data
x: .4byte 0x3c00
y: .4byte 0x4200
test_cases:
    .4byte 0x3c00, 0x4000     # 1.0 * 2.0, -> 0x4000 (2.0)
    .4byte 0x3800, 0x3e00     # 0.5 * 1.5, -> 0x3a00 (0.75)
    .4byte 0x3a00, 0x4000     # 0.75 * 2.0, -> 0x3e00 (1.5)
    .4byte 0x3c00, 0x3e00     # 1.0 * 1.5, -> 0x3e00 (1.5)
    .4byte 0x4000, 0x4000     # 2.0 * 2.0, -> 0x4400 (4.0)
    
str1: .string "Input FP16 is:"
str2: .string " and "
str3: .string "\nResult is "
str4: .string "\n"

.text
main:
    li s2,0 #set index = 0
    li s3,5 #total data = 5

test_loop:
    beq s2,s3,end_tests
    la t0,test_cases
    slli t1,s2,3
    add t0,t0,t1
    
    lw s0,0(t0)
    lw s1,4(t0)
    
    mv a0,s0
    mv a1,s1
    jal ra,fp16_mul
    
    mv t0,a0
    mv a0,s0
    mv a1,s1
    mv a2,t0
    jal ra,printResult
    
    addi s2,s2,1
    j test_loop

end_tests:
    li a7,10
    ecall
    
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
    
    mul t1,s4,s5
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


printResult:
    mv t0,a0
    
    la a0,str1
    li a7,4
    ecall #print "Input FP16 is:"
    
    mv a0,t0
    li a7,34
    ecall #print x
    
    la a0,str2
    li a7,4
    ecall #print "and"
    
    mv a0,a1
    li a7,34
    ecall #print y
        
    la a0,str3
    li a7,4
    ecall #print "\nResult is "

    mv a0,a2
    li a7,34
    ecall #print result
    
    la a0,str4
    li a7,4
    ecall #print "\n"
    
    ret