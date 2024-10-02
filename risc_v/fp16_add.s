.data
x: .4byte 0x3c00
y: .4byte 0x4000
str1: .string "Input FP16 is:"
str2: .string " and "
str3: .string "\nResult is "

.text
main:
    lw a0,x
    lw a1,y
    jal ra,fp16_add
    
    mv t0,a0
    
    la a0,str1
    li a7,4
    ecall
    
    mv a0,s0
    li a7,34
    ecall
    
    la a0,str2
    li a7,4
    ecall
    
    mv a0,s1
    li a7,34
    ecall
    
    la a0,str3
    li a7,4
    ecall

    mv a0,t1
    li a7,34
    ecall
    
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
    and s2,a2,t1 #uint16_t sign_a(s2) = a & 0x8000;
    and s3,a3,t1 #uint16_t sign_b(s3) = b & 0x8000;
    
    li t1,0x7C00
    and s4,a0,t1
    srli s4,s4,10 #int exp_a(s4) = (a & 0x7C00) >> 10;
    and s5,a1,t1
    srli s5,s5,10 #int exp_b(s5) = (b & 0x7C00) >> 10;

    andi s6,a0,0x03FF #uint16_t mant_a(s6) = a & 0x03FF;
    andi s7,a3,0x03FF #uint16_t mant_b(s7) = b & 0x03FF;
    
    ori s6,s6,0x0400 #mant_a(s6) = ((mant_a) | 0x0400);
    ori s7,s7,0x0400 #mant_b(s7) = ((mant_b) | 0x0400);
    
    blt s6,s7,align_bit_aless
    mv t1,ra
    mv a0,s5
    sub a1,s4,s5
    jal ra,align_mantissa 
    mv s5,a0 #mant_a(s4) = align_mantissa(mant_a, exp_b - exp_a);
    mv ra,t1
    j finish_align
    
align_bit_aless:
    mv t1,ra
    mv a0,s4
    sub a1,s5,s4
    jal ra,align_mantissa 
    mv s4,a0 #mant_b(s5) = align_mantissa(mant_a, exp_b - exp_a);
    mv ra,t1
    
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
    srai s6,s6,1 #mant_result >>= 1;
    addi s4,s4,1 #exp_result++;
    
finish_mantissa_normalization:
    addi s6,s6,0x03ff #mant_result(s6) &= 0x03FF;
    
    li t1,31
    bge s4,t1,exp_greater #if (exp_result >= 31)
    bge zero,s4,exp_less #else if (exp_result <= 0)
    
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
    ret
    
align_mantissa:
    addi sp,sp,-4
    sw s0,0(sp)
    
    bge zero,a1,end_align_mantissa
    lw s1,0(a1)
    srl a0,a0,s1
    
    lw s0,0(sp)
    addi sp,sp,4
    ret

end_align_mantissa:
    lw s0,0(sp)
    addi sp,sp,4
    ret
    
    