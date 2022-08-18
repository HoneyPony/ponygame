	.file	"pony_math.c"
	.text
	.p2align 4
	.globl	abs_i
	.def	abs_i;	.scl	2;	.type	32;	.endef
	.seh_proc	abs_i
abs_i:
	.seh_endprologue
	movl	%ecx, %eax
	negl	%eax
	cmovs	%ecx, %eax
	ret
	.seh_endproc
	.p2align 4
	.globl	abs_f
	.def	abs_f;	.scl	2;	.type	32;	.endef
	.seh_proc	abs_f
abs_f:
	.seh_endprologue
	andps	.LC0(%rip), %xmm0
	ret
	.seh_endproc
	.p2align 4
	.globl	abs_d
	.def	abs_d;	.scl	2;	.type	32;	.endef
	.seh_proc	abs_d
abs_d:
	.seh_endprologue
	andpd	.LC1(%rip), %xmm0
	ret
	.seh_endproc
	.p2align 4
	.globl	atan2_f
	.def	atan2_f;	.scl	2;	.type	32;	.endef
	.seh_proc	atan2_f
atan2_f:
	.seh_endprologue
	jmp	atan2f
	.seh_endproc
	.p2align 4
	.globl	atan2_d
	.def	atan2_d;	.scl	2;	.type	32;	.endef
	.seh_proc	atan2_d
atan2_d:
	.seh_endprologue
	jmp	atan2
	.seh_endproc
	.p2align 4
	.globl	clamp_i
	.def	clamp_i;	.scl	2;	.type	32;	.endef
	.seh_proc	clamp_i
clamp_i:
	.seh_endprologue
	cmpl	%r8d, %ecx
	movl	%edx, %eax
	cmovle	%ecx, %r8d
	cmpl	%edx, %ecx
	cmovge	%r8d, %eax
	ret
	.seh_endproc
	.p2align 4
	.globl	clamp_f
	.def	clamp_f;	.scl	2;	.type	32;	.endef
	.seh_proc	clamp_f
clamp_f:
	.seh_endprologue
	comiss	%xmm0, %xmm1
	ja	.L10
	minss	%xmm0, %xmm2
	movaps	%xmm2, %xmm1
.L10:
	movaps	%xmm1, %xmm0
	ret
	.seh_endproc
	.p2align 4
	.globl	clamp_d
	.def	clamp_d;	.scl	2;	.type	32;	.endef
	.seh_proc	clamp_d
clamp_d:
	.seh_endprologue
	comisd	%xmm0, %xmm1
	ja	.L14
	minsd	%xmm0, %xmm2
	movapd	%xmm2, %xmm1
.L14:
	movapd	%xmm1, %xmm0
	ret
	.seh_endproc
	.p2align 4
	.globl	sin_f
	.def	sin_f;	.scl	2;	.type	32;	.endef
	.seh_proc	sin_f
sin_f:
	.seh_endprologue
	jmp	sinf
	.seh_endproc
	.p2align 4
	.globl	sin_d
	.def	sin_d;	.scl	2;	.type	32;	.endef
	.seh_proc	sin_d
sin_d:
	.seh_endprologue
	jmp	sin
	.seh_endproc
	.p2align 4
	.globl	cos_f
	.def	cos_f;	.scl	2;	.type	32;	.endef
	.seh_proc	cos_f
cos_f:
	.seh_endprologue
	jmp	cosf
	.seh_endproc
	.p2align 4
	.globl	cos_d
	.def	cos_d;	.scl	2;	.type	32;	.endef
	.seh_proc	cos_d
cos_d:
	.seh_endprologue
	jmp	cos
	.seh_endproc
	.p2align 4
	.globl	rad2deg_i
	.def	rad2deg_i;	.scl	2;	.type	32;	.endef
	.seh_proc	rad2deg_i
rad2deg_i:
	.seh_endprologue
	pxor	%xmm0, %xmm0
	cvtsi2sdl	%ecx, %xmm0
	mulsd	.LC2(%rip), %xmm0
	cvttsd2sil	%xmm0, %eax
	ret
	.seh_endproc
	.p2align 4
	.globl	rad2deg_f
	.def	rad2deg_f;	.scl	2;	.type	32;	.endef
	.seh_proc	rad2deg_f
rad2deg_f:
	.seh_endprologue
	cvtss2sd	%xmm0, %xmm0
	mulsd	.LC2(%rip), %xmm0
	cvtsd2ss	%xmm0, %xmm0
	ret
	.seh_endproc
	.p2align 4
	.globl	rad2deg_d
	.def	rad2deg_d;	.scl	2;	.type	32;	.endef
	.seh_proc	rad2deg_d
rad2deg_d:
	.seh_endprologue
	mulsd	.LC2(%rip), %xmm0
	ret
	.seh_endproc
	.p2align 4
	.globl	deg2rad_i
	.def	deg2rad_i;	.scl	2;	.type	32;	.endef
	.seh_proc	deg2rad_i
deg2rad_i:
	.seh_endprologue
	pxor	%xmm0, %xmm0
	cvtsi2sdl	%ecx, %xmm0
	mulsd	.LC3(%rip), %xmm0
	cvttsd2sil	%xmm0, %eax
	ret
	.seh_endproc
	.p2align 4
	.globl	deg2rad_f
	.def	deg2rad_f;	.scl	2;	.type	32;	.endef
	.seh_proc	deg2rad_f
deg2rad_f:
	.seh_endprologue
	cvtss2sd	%xmm0, %xmm0
	mulsd	.LC3(%rip), %xmm0
	cvtsd2ss	%xmm0, %xmm0
	ret
	.seh_endproc
	.p2align 4
	.globl	deg2rad_d
	.def	deg2rad_d;	.scl	2;	.type	32;	.endef
	.seh_proc	deg2rad_d
deg2rad_d:
	.seh_endprologue
	mulsd	.LC3(%rip), %xmm0
	ret
	.seh_endproc
	.section .rdata,"dr"
	.align 16
.LC0:
	.long	2147483647
	.long	0
	.long	0
	.long	0
	.align 16
.LC1:
	.long	-1
	.long	2147483647
	.long	0
	.long	0
	.align 8
.LC2:
	.long	442745336
	.long	1078765020
	.align 8
.LC3:
	.long	-1571644103
	.long	1066524486
	.ident	"GCC: (Rev2, Built by MSYS2 project) 12.1.0"
	.def	atan2f;	.scl	2;	.type	32;	.endef
	.def	atan2;	.scl	2;	.type	32;	.endef
	.def	sinf;	.scl	2;	.type	32;	.endef
	.def	sin;	.scl	2;	.type	32;	.endef
	.def	cosf;	.scl	2;	.type	32;	.endef
	.def	cos;	.scl	2;	.type	32;	.endef
