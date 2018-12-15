	.file	"hail64asm.s"
	.text
	.globl	hail64an
	.type	hail64an, @function
# Check for peak in steps given a 64-bit input number
#	
#	hail64an(n, steps, maxsteps, maxvalue_size, *maxvalue, *peak_found)
#		%rdi - n, input value
#		%rsi - steps, current steps
#		%rdx - maxsteps, previous peak in steps
#		%rcx - maxvalue_size, previous peak in values
#		%r8  - maxvalue, previous peak in values
#		%r9  - pointer set to 1 if peak is found
#
#		%r11 - struct poly *p
#
# TODO:
# 1. move register saves to overflow region rather than every time.
#if CHECK_MAXVALUE
		.globl	hail64am
	.type	hail64am, @function
hail64am:	
#else
	.globl	hail64an
	.type	hail64an, @function	
hail64an:
#endif
	pushq	%r15			# we use %rcx and %rdx so move these arguments to caller save registers %r15, %r14
	pushq	%r14
	movl	%edx, %r14d		# maxsteps
	movq	%rcx, %r15		# maxvalue_size

	cmpq	$1048575, %rdi		# while (n > 0)
	jbe	.L1_end_routine		#
.L2_while_loop:
	movzwl	%di, %r11d
	salq	$4, %r11
#if CHECK_MAXVALUE
	addq	$mpoly16, %r11		# p (%r11) = &fpoly[n&0xffff]
#else
	addq	$fpoly16, %r11		# p (%r11) = &fpoly[n&0xffff]	
#endif
	cmpb	$0, 12(%r11)		# if (!p->smaller){
	jne	.L3_else_smaller
	movq	%rdi, %r10		# save copy of n - in case we overflow
#if CHECK_MAXVALUE
	movw	$0, %di			# clear bottom 16-bits if polynomial < 16
#endif
	movzwl	4(%r11),%ecx
	shrq	%cl, %rdi		# n = n >> p->div
	movl	(%r11), %eax
	mulq	%rdi			# n = n * p->mul
	jc	.L5_overflow
	movq	%rax,%rdi
	movl	8(%r11), %eax
	addq	%rax, %rdi		# n = n + p->add
	movzwl	6(%r11), %eax
	addl	%eax, %esi		# steps = steps + p->steps
	jmp	.L2_while_loop
.L5_overflow:				# multiply has overflowed >64 bits
	pushq	%r8			# save maxvalue, peak_found since their arg registers get re-used
	pushq	%r9
	subq	$424, %rsp		# create a new frame - for local num, and pointers to variables
	movl	%esi, 12(%rsp)		# steps lives @ 12(%rsp)
	movl	%r14d, 8(%rsp)		# maxsteps lives @ 8(%rsp)
	leaq	16(%rsp), %rdi		# zero the num[] buffer
	movl	$50, %ecx
	movl	$0, %eax
	rep	stosq
	movl	%r10d, 16(%rsp)		# num[0] = n & 0xffffffff
	shrq	$32, %r10
	movl	%r10d, 20(%rsp)
	movq	%r9, (%rsp)		# peak_found
	movl	%r15d, %r9d		# maxvalue_size
					# maxvalue stays in %r8
	leaq	8(%rsp), %rcx		# maxsteps
	leaq	12(%rsp), %rdx		# steps
	movl	$2, %esi		# nsize
	leaq	16(%rsp), %rdi		# num
#if CHECK_MAXVALUE
	call	hailxmf
#else
	call	hailxnf
#endif
	movl	12(%rsp), %esi		# restore steps
	movl	20(%rsp), %edi		# restore n
	salq	$32, %rdi
	movl	16(%rsp), %eax
	orq	%rax, %rdi
	addq	$424, %rsp
	popq	%r9			# restore maxvalue, peak_found
	popq	%r8
	jmp	.L2_while_loop
.L3_else_smaller:
#if CHECK_MAXVALUE
	movw	$0, %di			# clear bottom 16-bits if polynomial < 16
#endif	
	movzwl	4(%r11),%ecx
	shrq	%cl, %rdi		# n = n >> p->div
	movl	(%r11), %eax
	imulq	%rax, %rdi		# n = n * p->mul
	movl	8(%r11), %eax
	addq	%rax, %rdi		# n = n + p->add
	movzwl	6(%r11), %eax
	addl	%eax, %esi		# steps = steps + p->steps
	movl	%esi, %ecx		
	lzcntq	%rdi, %rax
	addl	clz64(,%rax,4),%ecx	# clz64[clz] + steps < maxsteps?
	cmpl	%r14d, %ecx
	jb	.L0_epilogue
.L4_endif_smaller:
	cmpq	$1048575, %rdi
	ja	.L2_while_loop
.L1_end_routine:
	addl	steps20(,%rdi,4), %esi
	cmpl	%r14d, %esi
	jbe	.L0_epilogue
	movl	$1,(%r9)
.L0_epilogue:
	popq	%r14
	popq	%r15
	ret
#if CHECK_MAXVALUE
	.size	hail64am, .-hail64am
#else
	.size	hail64an, .-hail64an
#endif
