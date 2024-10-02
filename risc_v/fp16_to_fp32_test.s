.data
x: .4byte 0x3555
str1: .string "Input FP16 is: "
next_line: .string "\n"
str2: .string "Output fp32 is: "

.text

main:
    lw a0,x
    jal ra,fp16_to_fp32
    li a7,10
    ecall   
    
fp16_to_fp32:
    #prologue
    addi sp,sp,-20
    sw s0,0(sp)
    sw s1,4(sp)
    sw s2,8(sp)
    sw s3,12(sp)
    sw s4,20(sp)
    
    #start
    lw a0,x # load uint16 word(s0)
    slli s1, a0, 16 # w(s1) = (uint32_t)h << 16;
    li s2, 0x80000000
    and s2, s1, s2 # sign(s2) = w & UINT32_C(0x80000000);
    li s3,0x7FFFFFFF
    and s3, s1, s3 # nonsign(s3) = w & UINT32_C(0x7FFFFFFF);
    
    mv t1,a0
    mv t2,ra
    mv a0,s3
    jal my_clz #renorm_shift = my_clz(nonsign);
    jal renorm_if
    mv s4, a0  #renorm_shift(s4) = renorm_shift > 5 ? renorm_shift - 5 : 0;
    mv a0,t1
    mv ra,t2
    
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
    
    mv t1,ra
    jal printResult
    mv ra,t1
    
    #epilogue
    lw s4,20(sp)
    lw s3,12(sp)
    lw s2,8(sp)
    lw s1,4(sp)
    lw s0,0(sp)
    addi sp,sp,20
    
    ret  
    
my_clz:
    #prologue
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

    #epilogue
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
    
printResult:
    mv t0,a0
    
    la a0,str1
    li a7, 4
    ecall
    
    mv a0,s0
    li a7,34
    ecall

    la a0,next_line
    li a7,4
    ecall
    
    la a0,str2
    li a7, 4
    ecall

    mv a0,t0
    li a7,34
    ecall
    
    ret