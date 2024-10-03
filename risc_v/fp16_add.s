.data
x: .4byte 0x3c00
y: .4byte 0x3800
str1: .string "Input FP16 is:"
str2: .string " and "
str3: .string "\nResult is "

.text
main:
    lw a0,x
    lw a1,y #load data
    
    mv t0,a0
    mv t1,a1
    
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
    
    mv a0,t0
    mv a1,t1

    jal ra,fp16_add
    
    mv t1,a0
    la a0,str3
    li a7,4
    ecall #print "\nResult is "

    mv a0,t1
    li a7,34
    ecall #print result
    
    li a7,10
    ecall
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
    
    