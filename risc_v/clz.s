.data
x: .word 102
str1: .string "Result is "

.text
my_clz:
    li t0, 0 #set count
    li t1, 31 #set i
    lw a0, x #set x
    
clz_loop:
    bltz t1, clz_done #if i<0, escape the loop
    li t2, 1 
    sll t3, t2, t1
    and t4, t3, a0
    bnez t4, clz_done
    addi t0, t0, 1
    addi t1, t1, -1

    j clz_loop
    
clz_done:    
    mv a0, t0
    jal ra, printResult
    li a7,10
    ecall
    
printResult:
    mv t1, a0
    la a0, str1
    li a7, 4
    ecall
    
    mv a0, t1
    li a7, 1
    ecall
    ret