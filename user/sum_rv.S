.data

msg_err: .asciz "Invalid input\n"
msg_enter: .asciz "Enter 2 numbers separated by a space: "
msg_res: .asciz "Result: %d\n"

.text
.globl main
main:
	addi sp, sp, -256
	sd ra, 248(sp)
	sd s0, 240(sp)
	sd s1, 232(sp)
	sd s2, 224(sp)
	sd s3, 216(sp)
	sd s4, 208(sp)
	sd s5, 200(sp)

	mv s0, sp
	mv s1, sp

	la a0, msg_enter
	call printf

	mv a0, s0
	li a1, 128
	call gets

	mv s2, s0
 
parse_first_start:
	lbu t0, 0(s2)
	li t1, 48
	blt t0, t1, invalid_input
	li t1, 57
	bgt t0, t1, invalid_input

parse_first:
	lbu t0, 0(s2)
	li t1, 48
	blt t0, t1, add_first_token_end
	li t1, 57
	bgt t0, t1, add_first_token_end
	addi s2, s2, 1
	j parse_first

add_first_token_end:
	lbu t0, 0(s2)
	li t1, 32
	bne t0, t1, invalid_input
	sb zero, 0(s2)
	addi s2, s2, 1
	mv s3, s2

parse_second_start:
	lbu t0, 0(s2)
	li t1, 48
	blt t0, t1, invalid_input
	li t1, 57
	bgt t0, t1, invalid_input

parse_second:
	lbu t0, 0(s2)
	li t1, 48
	blt t0, t1, add_second_token_end
	li t1, 57
	bgt t0, t1, add_second_token_end
	addi s2, s2, 1
	j parse_second

add_second_token_end:
	lbu t0, 0(s2)
	li t1, 10
	beq t0, t1, terminate_second_token
	bnez t0, invalid_input
	j calc

terminate_second_token:
	sb zero, 0(s2)
	addi s2, s2, 1 
	lbu t0, 0(s2)
	bnez t0, invalid_input

calc:
	mv a0, s0
	call atoi
	mv s4, a0

	mv a0, s3
	call atoi
	mv s5, a0

	mv a0, s4
	mv a1, s5
	call add
	mv s4, a0

	la a0, msg_res
	mv a1, s4
	call printf

	li a0, 0
	call exit

invalid_input:
	li a0, 2
	la a1, msg_err
	call fprintf
	li a0, 1
	call exit