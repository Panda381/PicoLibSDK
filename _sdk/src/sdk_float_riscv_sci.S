
// ****************************************************************************
//
//                       Float RP2350 - RISC-V science functions
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040

// This code contains fast RISC-V functions (* recommended):
// (all-libc means libc without fast fmul/fadd support, no speed-up at all)

//  sqrtf (all-libc 3600ns, libc 2900ns, *fast 359ns)
//  expf (all-libc 13700ns, libc 6600ns, *Chebyshev 468ns, Taylor 533ns, Cordic 3270ns)
//  exp2f  (all-libc 47100ns, libc 6730ns, *Chebyshev 433ns, Taylor 501ns, Cordic 3580ns)
//  exp10f (all-libc 61200ns, libc 7000ns, *Chebyshev 530ns, Taylor 584ns, Cordic 3580ns)
//  logf (all-libc 14400ns, libc 6700ns, *Chebyshev 616ns, Mercator 822ns, Cordic 4770ns)
//  log2f (all-libc 15800ns, libc 7060ns, *Chebyshev 650ns, Mercator 876ns, Cordic 5090ns)
//  log10f (all-libc 16700ns, libc 6900ns, *Chebyshev 676ns, Mercator 882ns, Cordic 5090ns)
//  sinf (all-libc 11800ns, libc 5680ns, Chebyshev 867ns, *Taylor 730ns, Cordic 3440ns)
//  sinf_deg (Chebyshev 808ns, *Taylor 687ns, Cordic 3750ns)
//  cosf (all-libc 13600ns, libc -ns, Chebyshev 880ns, *Taylor 766ns, Cordic 3500ns)
//  cosf_deg (Chebyshev 814ns, *Taylor 719ns, Cordic 3810ns)
//  sincosf (all-libc 25700ns, libc -ns, Chebyshev 1370ns, *Taylor 1120ns, Cordic 3530ns)
//  sincosf_deg (Chebyshev 1290ns, *Taylor 1070ns, Cordic 3870ns)
//  tanf (all-libc 24100ns, libc -ns, Chebyshev 1840ns, *Taylor 1610ns, Cordic 3970ns)
//  tanf_deg (Chebyshev 1780ns, *Taylor 1560ns, 4280ns)
//  cotanf (all-libc 25800ns, libc -ns, Chebyshev 1850ns, *Taylor 1610ns, Cordic 4000ns)
//  cotanf_deg (Chebyshev 1770ns, *Taylor 1560ns, 4310ns)
//  asinf (all-libc 18400ns, *Taylor 556ns, Cordic 5510ns)
//  asinf_deg (all-libc 19200ns, *Taylor 576ns, Cordic 5820ns)
//  acosf (all-libc 18800ns, *Taylor 601ns, Cordic 5540ns)
//  acosf_deg (all-libc 19500ns, *Taylor 616ns, Cordic 5850ns)
//  atanf (*Chebyshev 1010ns)
//  atanf_deg (*Chebyshev 1030ns)
//  acotanf (*Chebyshev 1030ns)
//  acotanf_deg (*Chebyshev 1070ns)

// Functions were tested with the FPUTEST application from the PicoLibSDK  
// https://github.com/Panda381/PicoLibSDK/tree/main/PicoPad/TEST/FPUTEST

// .p2align 2,,
// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - adds incorrect alignment
// after the end of the section. Use commas ,, to work a little better.

#include "../asm_include.inc"

// SETUP: Select functions
// - If selecting libc variant, comment out "wrap" rows in Makefile.inc, RP2350RISCV group.
// - To use libc variant of all functions, set USE_FLOATLIBC to 1 in Makefile.inc.

#ifndef USE_RISCV_SQRTF
#define USE_RISCV_SQRTF		1		// RISC-V sqrtf() function: 0=libc, 1=fast (recommended)
#endif

#ifndef USE_RISCV_EXPF
#define USE_RISCV_EXPF		1		// RISC-V expf(), exp2f() and exp10f() function: 0=libc, 1=Chebyshev (recommended), 2=Taylor, 3=Cordic
#endif

#ifndef USE_RISCV_LOGF
#define USE_RISCV_LOGF		1		// RISC-V logf(), log2f() and log10f() function: 0=libc, 1=Chebyshev (recommended), 2=Mercator, 3=Cordic
#endif

#ifndef USE_RISCV_SINF
#define USE_RISCV_SINF		2		// RISC-V sinf(), cosf(), sincosf(), tanf(), cotanf() functions: 0=libc, 1=Chebyshev, 2=Taylor (recommended), 3=Cordic
#endif

#ifndef USE_RISCV_ASINF
#define USE_RISCV_ASINF		2		// RISC-V asinf(), acosf() functions: 0=libc, 2=Taylor (recommended), 3=Cordic
#endif

#ifndef USE_RISCV_ATANF
#define USE_RISCV_ATANF		1		// RISC-V atanf(), acotanf(), atan2f() functions: 0=libc, 1=Chebyshev (recommended), 3=Cordic
#endif

#if USE_FLOAT		// use float support 1=in RAM, 2=in Flash

#if USE_FLOAT == 1
#define float_section section_noflash
#else
#define float_section section_text
#endif

#define FLOAT_EXP1	0x7f		// exponent of '1' (= 127)

.macro _asm_rad2degf	// convert radians to degrees (do not call inline rad2degf directly, a slow function in Flash would be used)
	li	a1,0x42652ee1		// A1 <- 180/PI = 57.29578f
	call	fmul			// convert radians to degrees
.endm

.macro _asm_frec	// reciprocal 1/x (do not call inline frec directly, a slow function in Flash would be used)
	mv	a1,a0			// A1 <- argument x
	li	a0,0x3F800000		// A0 <- 1.0
	call	fdiv			// calculate 1/x
.endm

.macro _asm_fsqr	// calculate x^2 (do not call inline fsqr directly, a slow function in Flash would be used)
	mv	a1,a0
	call	fmul
.endm

#if !USE_FLOATLIBC

// ----------------------------------------------------------------------------
//                            Square root
// ----------------------------------------------------------------------------
// float sqrtf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result sqrtf(x)
// TRASHED: A1, A2, A3, A4
// 357ns, err avg=0.089, err max=1

#if USE_RISCV_SQRTF == 1		// RISC-V sqrtf() function: 0=libc, 1=fast (recommended)

float_section _sqrtf
.p2align 2,,

.global __wrap_sqrtf
.global sqrtf
__wrap_sqrtf:
sqrtf:

// >>> This code is an adaptation of the fsqrt() function from Pico1 M0+ (c) Raspberry Pi <<<

// The square root routine uses an initial approximation to the reciprocal of the square root of the argument based
// on the top four bits of the mantissa (possibly shifted one place to make the exponent even). It then performs two
// Newton-Raphson iterations, resulting in about 14 bits of accuracy. This reciprocal is then multiplied by
// the original argument to produce an approximation to the result, again with about 14 bits of accuracy.
// Then a remainder is calculated, and multiplied by the reciprocal estiamte to generate a correction term
// giving a final answer to about 28 bits of accuracy. A final remainder calculation rounds to the correct
// result if necessary.
// Again, the fixed-point calculation is carefully implemented to preserve accuracy, and similar comments to those
// made above on the fast division routine apply.
// The reciprocal square root calculation has been tested for all possible (possibly shifted) input mantissa values.

	// check if argument is negative
	bltz	a0,sq_0		// argument is negative

	// get mantissa (23+1 bits) without exponent and add implied hidden bit '1' -> A1
	slli	a1,a0,8		// A1 <- mantissa << 8
	bseti	a1,a1,31	// set implied hidden bit '1'
	srli	a1,a1,8		// A1 <- mantissa

	// get exponent (8 bits, sign bit is not set here) -> A2
	srli	a2,a0,23	// extract exponent -> A2

	// check if number is zero
	beqz	a2,sq_2		// number is zero

	// check if number is infinity
	li	a4,255		// exponent of infinity
	beq	a2,a4,sq_1	// number is infinite

	// pre-correction for packing - add 125 to exponent
	add	a2,a2,125	// correction for packing

	// exponent/2 (sqrtf halves the exponent) - if odd, double mantissa (mantissa is 25 bits now)
	slli	a4,a2,31	// A4 <- save lowest bit of the exponent
	srai	a2,a2,1	// exponent/2
	beqz	a4,1f		// exponent is even
	slli	a1,a1,1		// exponent was odd - double mantissa; mantissa y now 1..4 Q23

	// get first approximation from the table
	// - first four table entries are never accessed because of the mantissa's leading 1
1:	lui	a4,%hi(rsqrtapp-4) // A4 <- table addres HIGH
	srli	a3,a1,21	// y Q2 .. get highest 4 bits of the mantissa (implied bit 2 or 3 is always set)
	add	a4,a4,a3	// A4 <- table address HIGH + mantissa index
	lbu	a4,%lo(rsqrtapp-4)(a4) // load initial approximation to reciprocal square root m0 Q8

	// first Newton-Raphson iteration
	srli	a0,a1,7		// y Q16: first Newton-Raphson iteration
	mul	a0,a0,a4	// m0*y Q24
	mul	a0,a0,a4	// a0=p0=m0*y*y Q32
	srai	a0,a0,12	// a0 Q20

	mul	a0,a0,a4	// dy0=m0*a0 Q28
	srai	a0,a0,13	// dy0 Q15
	slli	a4,a4,8		// m0 Q16
	sub	a4,a4,a0	// m1=m0-dy0/2 Q16-Q15/2 -> Q16
	addi	a4,a4,170	// mostly remove systematic error in this approximation: gains approximately 1 bit

	// second Newton-Raphson iteration
	mv	a0,a4		// second Newton-Raphson iteration
	mul	a0,a0,a0	// m1*m1 Q32
	srli	a0,a0,15	// m1*m1 Q17
	srli	a3,a1,8		// y Q15

	mul	a0,a0,a3	// a1=p1=m1*m1*y Q32
	srai	a0,a0,12	// a1 Q20
	mul	a0,a0,a4	// dy1=m1*a1 Q36
	srai	a0,a0,21	// dy1 Q15
	sub	a4,a4,a0	// m2=m1-dy1/2 Q16-Q15/2 -> Q16

	mul	a3,a3,a4	// m3=y*m2 Q31
	srli	a3,a3,15	// m3 Q16

// here m2 is an approximation to the reciprocal square root
// and m3 is an approximation to the square root

	mv	a0,a3
	mul	a0,a0,a0	// m3*m3 Q32
	slli	a1,a1,9		// y Q32
	sub	a0,a1,a0	// a2=y-m3*m3 Q32 remainder
	srai	a0,a0,5		// a2 Q27

	mul	a4,a4,a0	// a2*m2 Q43
	slli	a3,a3,7		// m3 Q23
	srai	a0,a4,15	// a2*m2 Q28
	addi	a0,a0,16	// rounding to Q24
	srai	a0,a0,6		// a2*m2 Q22

	// rounding
	add	a3,a3,a0	// m4 Q23: candidate final result
	sltu	a4,a3,a0	// set A4 to 1 if A3+A0 overflow
	beqz	a4,sq_3		// near rounding boundary? skip if no rounding needed

	add	a4,a4,a3	// m4+0.5ulp Q24
	mul	a4,a4,a4	// Q48
	slli	a1,a1,16	// y Q48
	sub	a1,a1,a4	// remainder Q48
	bltz	a1,sq_3		// negative
	add	a3,a3,1		// round up

	// pack number
sq_3:	slli	a2,a2,23	// pack exponent
	add	a0,a2,a3
sq_6:	ret

	// argument is negative
sq_0:	slli	a1,a0,1		// destroy sign bit
	srli	a1,a1,24	// check if exponent is 0
	beqz	a1,sq_2		// -0: return it

	// here negative and not -0: return -Inf
	srai	a0,a0,31
sq_5:	slli	a0,a0,23
	ret

	// return +inf
sq_1:	srli	a0,a0,23
	j	sq_5

	// return +-zero result
sq_2:	srli	a0,a0,31
	slli	a0,a0,31
	ret

// round(sqrt(2^22./[72:16:248]))
rsqrtapp:
	.byte	0xf1,0xda,0xc9,0xbb, 0xb0,0xa6,0x9e,0x97, 0x91,0x8b,0x86,0x82

#endif // USE_RISCV_SQRTF == 1

// ----------------------------------------------------------------------------
//                              Float Cordic
// ----------------------------------------------------------------------------
// All the scientific functions are implemented using the CORDIC algorithm. For notation,
// details not explained in the comments below, and a good overall survey see
// "50 Years of CORDIC: Algorithms, Architectures, and Applications" by Meher et al.,
// IEEE Transactions on Circuits and Systems Part I, Volume 56 Issue 9.

// >>> This code is an adaptation of the cordic function from Pico1 M0+ (c) Raspberry Pi <<<

float_section _asm_cordic
.p2align 2,,

// CORDIC initialize
// INPUT: A0 = x
//	  A1 = y
//	  A2 = angle
//	  A3 = table pointer
//	  A4 = m
// OUTPUT: A0 = next x
//	   A1 = next y
//	   A2 = next angle
//	   A3 = next table pointer
//	   A4 = new m
//	   A5 = index
//	   A6, A7 = step
//	   T0 = m (copy of A4)
//	   T1 = 1 if table finished, 0 if continue with next step

	// initialisation
cordic_start:
	li	a5,0		// initial shift=0
	mv	t0,a4
	j	5f

	// one step of algorithm in vector mode
cordic_vstep:
	bgtz	a1,4f		// y is positive
	j	1f		// y is negative or zero

	// one step of algorithm in rotation mode
cordic_rstep:
	bgez	a2,1f		// angle is positive

4:	sub	a1,a1,a6	// negative rotation: y=y-(x>>i)
	neg	a7,a7
	add	a2,a2,a4	// accumulate angle
	j	2f

1:	add	a1,a1,a6	// positive rotation: y=y+(x>>i)
	sub	a2,a2,a4	// accumulate angle

2:	mv	a4,t0
	mul	a7,a7,a4	// apply sign from m
	sub	a0,a0,a7	// finish rotation: x=x{+/-}(y>>i)

5:	lw	a4,(a3)		// fetch next angle from table
	addi	a3,a3,4		// bump pointer

	andi	t1,a4,1		// repeated angle?
	bnez	t1,7f		// repeated angle
	addi	a5,a5,1		// adjust shift if not repeated angle
7:	srli	a4,a4,1		// shift

3:	mv	a6,a0
	sra	a6,a6,a5	// x>>i
	mv	a7,a1
	sra	a7,a7,a5	// y>>i
	andi	t1,a4,1		// end flag
	srli	a4,a4,1		// shift
	ret

// CORDIC rotation mode
// INPUT: A0 = x
//	  A1 = y
//	  A2 = angle
//	  A3 = table pointer
//	  A4 = m
// OUTPUT: A0 = next x
//	   A1 = next y
//	   A2 = next angle
//	   A3 = next table pointer
//	   A4 = new m
//	   A5 = index
//	   A6, A7 = step
//	   T0 = m (copy of A4)
// TRASHED: T1, T2
cordic_rot:

	// initialize
// INPUT: A0 = x
//	  A1 = y
//	  A2 = angle
//	  A3 = table pointer
//	  A4 = m
// OUTPUT: A0 = next x
//	   A1 = next y
//	   A2 = next angle
//	   A3 = next table pointer
//	   A4 = new m
//	   A5 = index
//	   A6, A7 = step
//	   T0 = m (copy of A4)
//	   T1 = 1 if table finished, 0 if continue with next step
	mv	t2,ra		// save return address
	call	cordic_start	// initialise (use A0-A7, T1, T2)

	// one step of rotation mode
1:	call	cordic_rstep	// use A0-A7, T1, T2
	beqz	t1,1b		// step until table finished
	mv	ra,t2		// restore return address

	// remaining small rotations can be linearised: see IV.B of paper referenced above
	srai	a6,a0,14
	srai	a7,a1,14
	srai	a2,a2,3
	mul	a6,a6,a2	// all remaining CORDIC steps in a multiplication
	mul	a7,a7,a2
	mv	a4,t0

	mul	a7,a7,a4
	srai	a6,a6,12
	srai	a7,a7,12
	sub	a0,a0,a7	// x=x{+/-}(yz>>k)
	add	a1,a1,a6	// y=y+(xz>>k)
	ret

// CORDIC vector mode
// INPUT: A0 = x
//	  A1 = y
//	  A2 = angle
//	  A3 = table pointer
//	  A4 = m
// OUTPUT: A0 = next x
//	   A1 = next y
//	   A2 = next angle
//	   A3 = next table pointer
//	   A4 = new m
//	   A5 = index
//	   A6, A7 = step
//	   T0 = m (copy of A4)
// TRASHED: T1, T2
cordic_vec:

	// initialize
// INPUT: A0 = x
//	  A1 = y
//	  A2 = angle
//	  A3 = table pointer
//	  A4 = m
// OUTPUT: A0 = next x
//	   A1 = next y
//	   A2 = next angle
//	   A3 = next table pointer
//	   A4 = new m
//	   A5 = index
//	   A6, A7 = step
//	   T0 = m (copy of A4)
//	   T1 = 1 if table finished, 0 if continue with next step
	mv	t2,ra		// save return address
	call	cordic_start	// initialise (use A0-A7, T1, T2)

	// one step in vector mode
1:	call	cordic_vstep	// use A0-A7, T1, T2
	beqz	t1,1b		// step until table finished
	mv	ra,t2		// restore return address

	// continue as in cordic_vstep but without using table; x is not affected as y is small
4:	bgtz	a1,2f		// check sign of y
	add	a1,a1,a6	// positive rotation: y=y+(x>>i)
	sub	a2,a2,a4	// accumulate angle
	j	3f

2:	sub	a1,a1,a6	// negative rotation: y=y-(x>>i)
	add	a2,a2,a4	// accumulate angle

3:	srai	a6,a6,1
	srai	a4,a4,1		// next "table entry"
	bnez	a4,4b
	ret

// force a0 to lie in range [-1,1] Q29
// INPUT: A0
// OUTPUT: A0
// TRASHED: A4

clampx:	li	a4,1
	slli	a4,a4,29
	bgt	a0,a4,1f
	neg	a4,a4
	ble	a0,a4,1f
	ret

1:	mv	a0,a4
	ret

float_section _asm_cordic_tab_cc
.p2align 2,,

// first entry in following table is pi Q29
pi_q29:
// circular CORDIC coefficients: atan(2^-i), b0=flag for preventing shift, b1=flag for end of table
tab_cc:
	.word	0x1921fb54*4 + 1	// no shift before first iteration
	.word	0x0ed63383*4 + 0
	.word	0x07d6dd7e*4 + 0
	.word	0x03fab753*4 + 0
	.word	0x01ff55bb*4 + 0
	.word	0x00ffeaae*4 + 0
	.word	0x007ffd55*4 + 0
	.word	0x003fffab*4 + 0
	.word	0x001ffff5*4 + 0
	.word	0x000fffff*4 + 0
	.word	0x0007ffff*4 + 0
	.word	0x00040000*4 + 0
	.word	0x00020000*4 + 0 + 2	// +2 marks end

float_section _asm_cordic_tab_ch
.p2align 2,,

// hyperbolic CORDIC coefficients: atanh(2^-i)
// flags: b0=flag for preventing shift index, b1=flag for end of table
tab_ch:
	.word	0x1193ea7b*4 + 0
	.word	0x1193ea7b*4 + 1	// repeat i=1
	.word	0x082c577d*4 + 0
	.word	0x04056247*4 + 0
	.word	0x0200ab11*4 + 0
	.word	0x0200ab11*4 + 1	// repeat i=4
	.word	0x01001559*4 + 0
	.word	0x008002ab*4 + 0
	.word	0x00400055*4 + 0
	.word	0x0020000b*4 + 0
	.word	0x00100001*4 + 0
	.word	0x00080001*4 + 0
	.word	0x00040000*4 + 0
	.word	0x00020000*4 + 0
	.word	0x00020000*4 + 1 + 2	// repeat i=12

// ----------------------------------------------------------------------------
//                      Natural exponent - Cordic
// ----------------------------------------------------------------------------
// float expf(float x);
// INPUT: A0 = x
// OUTPUT: A0 = result
// TRASHED: A1-A7, T0-T4
// 3270ns, err avg=0.202, err max=2

#if USE_RISCV_EXPF == 3		// RISC-V expf(), exp2f() and exp10f() function: 0=libc, 1=Chebyshev (recommended), 2=Taylor, 3=Cordic

float_section _expf
.p2align 2,,

.global __wrap_expf
.global expf
__wrap_expf:
expf:
	// convert to fixed point 8.24
	li	a1,24		// A1 <- 24 number of shifts
	mv	t3,ra		// T3 <- save return address
// input: A0 = float number
//	  A1 = size of fractional part
// output: A0 = integer number
// trashes: A1, A2, A3, A4
	call	_asm_float2fix	// Q24: covers entire valid input range

	srai	a1,a0,16	// Q8
	li	a2,5909		// log_2(e) Q12
	mul	a1,a1,a2	// estimate exponent of result Q20
	srai	a1,a1,19	// Q1
	addi	a1,a1,1		// rounding
	srai	a1,a1,1		// rounded estimate of exponent of result

	mv	t4,a1		// save for later
	slli	a2,a0,5		// Q29
	li	a0,0x162e42ff	// ln(2) Q29
	mul	a1,a1,a0	// accurate contribution of estimated exponent
	sub	a2,a2,a1	// residual to be exponentiated, approximately -.5..+.5 Q29
	li	a0,0x2c9e15ca	// initialise CORDIC x,y with scaling
	li	a1,0
	la	a3,tab_ch	// hyperbolic coefficients
	not	a4,a1		// m=-1

// INPUT: A0 = x
//	  A1 = y
//	  A2 = angle
//	  A3 = table pointer
//	  A4 = m
// OUTPUT: A0 = next x
//	   A1 = next y
//	   A2 = next angle
//	   A3 = next table pointer
//	   A4 = new m
//	   A5 = index
//	   A6, A7 = step
//	   T0 = m (copy of A4)
// TRASHED: T1, T2
	call	cordic_rot	// calculate cosh and sinh

	add	a0,a0,a1	// exp=cosh+sinh
	mv	a2,t4		// recover exponent
	mv	ra,t3		// RA <- return address

	// pack number
	li	a5,0		// pack and return, sticky bits=0
// input: A0 = signed mantissa (24 significant bits)
//        A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
//        A5 = sticky bits (used for rounding)
// output: A0 = float number
// trashes: A2, A4, A5, T0
	j	_asm_packx

#endif // USE_RISCV_EXPF == 3

// ----------------------------------------------------------------------------
//                    Natural exponent - Chebyshev approximation
// ----------------------------------------------------------------------------
// float expf(float x);
// INPUT: A0 = x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4, T0, T1
// 467ns, err avg=0.088, err max=1

// Another alternative method is the Taylor series (see below),
// but this one is a little faster and a little more accurate.

#if USE_RISCV_EXPF == 1		// RISC-V expf(), exp2f() and exp10f() function: 0=libc, 1=Chebyshev (recommended), 2=Taylor, 3=Cordic

float_section _expf
.p2align 2,,

.global __wrap_expf
.global expf
__wrap_expf:
expf:

// Here is: A0 = input number

	// check if number A0 is negative
	bltz	a0,12f			// number is negative

	// number A0 is positive - check maximal positive number
	li	t0,0x42B17217		// T0 max. valid value: 88.722832 = 0x42B17217, infinity: 88.722839 = 0x42B17218
	bgtu	a0,t0,13f		// number is not valid - return infinity

	// get signed exponent -> A1
1:	slli	a1,a0,1			// shift 1 left to delete sign bit
	srli	a1,a1,24		// shift exponent to base position
	addi	a1,a1,-FLOAT_EXP1	// subtract exopnent bias

	// check minimal exponent A1 - return 1.0
	// - value with result 1.0000001 = 0x3F800001: 5.9604652e-8 = 0x33800001, exponent FLOAT_EXP1-24
	// - value with result 1.0000000 = 0x3F800000: 5.9604641e-8 = 0x337FFFFF, exponent FLOAT_EXP1-25
	li	t0,-24			// minimal valid exponent
	blt	a1,t0,14f		// exponent is too small - return +1.0

	// get sign -> A2
	srli	a2,a0,31		// A2 <- sign 0 or 1

// Here is:
//  A0 = input number x
//  A1 = signed exponent 
//  A2 = sign

	// normalize mantissa A0 - shift to max position and restore hidden bit '1'
	// - mantissa will be 0x80000000..0xFFFFFF00 (or with limit 0xB1721700 or 0xAEAC4F00)
	slli	a0,a0,8			// destroy exponent and sign
	bset	a0,a0,31		// set hidden bit '1'

	// prepare pointer to table of constants -> A3
	la	a3,expf_chebtab		// A3 <- table pointer

expf_exp10f_jump:

	// shift mantissa A0 right by exponent bits (shift >>= 7 - exp)
	// - get x as signed fixed number in format 8.24
	// - mantissa will be 0x00000001..0x58B90B80 or 0x57562780
	// - exp is in range -24..+6, shift is in range 1..31
	li	t1,7			// T1 <- number of shifts
	sub	t1,t1,a1		// T1 <- 7 - exp
	lw	t0,0(a3)		// T0 <- 3098164010 (1/ln(2)) (= 0xB8AA3B2A)
	srl	a0,a0,t1		// mantissa >>= 31 - exp - 24

	// convert mantissa to 2-base and get integer part of exponent -> A1
	// multiply by 1/ln(2) and shift >> 24, round to zero
	// constant: 1/ln(2) << 31 = 3098164010 round up (= 0xB8AA3B2A)
	// - exponent will be 0..+127
	mulhu	a1,a0,t0		// A1 (exponent) <- mantissa * 3098164010
	lw	t1,4(a3)		// T1 <- 2977044472 (ln(2)) (= 0xB17217F8)
	srli	a1,a1,24+31 - 32	// shift right = 23

	// convert exponent back to e-base: multiply by ln(2)
	//   ln(2) << 32 = 2977044472 round up
	// Later we will discard the upper 8 bits of the result,
	// so we will not need the HIGH result of the multiplication.
	mul	t1,a1,t1		// T1 <- exp * 2977044472

	// subtract exponent correction from the number
	slli	a0,a0,8			// mantissa << 8
	sub	a0,a0,t1		// mantissa = (mantissa << 8) - exponent part

	// convert to range 0..1.0: multiply by 1/ln(2) ... number will be in 0.32 format
	//   1/ln(2) << 31 = 3098164010 round up
	mulhu	a0,a0,t0		// A0 <- mantissa * 3098164010

// overflow (no need, but be aware)
// if ((s32)xx < 0) xx = (u32)-1;

	slli	a4,a0,1			// A4 <- mantissa << 1

expf_exp2f_jump:
	// coeff[9]
	li	a0,0x000000DD		// accumulator <- coeff[9] = 221 = 1.02897824066917e-007 << 31
	lw	t0,8(a3)		// T0 <- coeff[8] = 2876 = 1.3392906481613e-006 << 31
	mulhu	a0,a0,a4		// accumulator *= X

// Registers:
//  A0 = accumulator of the result 'acc' = x + x^2/2!
//  A1 = unsigned exponent 'exp'
//  A2 = sign (0 or 1)
//  A3 = table pointer
//  A4 = unsigned mantissa in format 0.32, without implied bit 1 'x'

	// check sign
	bnez	a2,2f			// skip if negative

// ---- positive

	// coeff[8]
	add	a0,t0,a0		// acc = coeff[8] + acc
	lw	t0,12(a3)		// T0 <- coeff[7] = 32753 = 1.52517549025788e-005 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[7]
	add	a0,t0,a0		// acc = coeff[7] + acc
	lw	t0,16(a3)		// T0 <- coeff[6] = 330755 = 0.000154019750482348 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[6]
	add	a0,t0,a0		// acc = coeff[6] + acc
	lw	t0,20(a3)		// T0 <- coeff[5] = 2863361 = 0.00133335616465047 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[5]
	add	a0,t0,a0		// acc = coeff[5] + acc
	lw	t0,24(a3)		// T0 <- coeff[4] = 20654787 = 0.00961813466710737 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[4]
	add	a0,t0,a0		// acc = coeff[4] + acc
	lw	t0,28(a3)		// T0 <- coeff[3] = 119194166 = 0.0555041086190613 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[3]
	add	a0,t0,a0		// acc = coeff[3] + acc
	lw	t0,32(a3)		// T0 <- coeff[2] = 515882494 = 0.240226506255567 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[2]
	add	a0,t0,a0		// acc = coeff[2] + acc
	lw	t0,36(a3)		// T0 <- coeff[1] = 1488522236 = 0.693147180601954 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[1]
	add	a0,t0,a0		// acc = coeff[1] + acc
	lui	t0,%hi(0x80000000)	// T0 <- coeff[0] = 2147483648 = 1.0f << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[0]
	add	a0,t0,a0		// acc = coeff[0] + acc

	// - result mantissa is acc = 0x80000000 .. 0xFFFFFFFF = 1.0 .. 1.9999
	j	4f

// ---- negative

	// negate exponent
2:	neg	a1,a1

	// coeff[8]
	sub	a0,t0,a0		// acc = coeff[8] - acc
	lw	t0,12(a3)		// T0 <- coeff[7] = 32753 = 1.52517549025788e-005 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[7]
	sub	a0,t0,a0		// acc = coeff[7] - acc
	lw	t0,16(a3)		// T0 <- coeff[6] = 330755 = 0.000154019750482348 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[6]
	sub	a0,t0,a0		// acc = coeff[6] - acc
	lw	t0,20(a3)		// T0 <- coeff[5] = 2863361 = 0.00133335616465047 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[5]
	sub	a0,t0,a0		// acc = coeff[5] - acc
	lw	t0,24(a3)		// T0 <- coeff[4] = 20654787 = 0.00961813466710737 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[4]
	sub	a0,t0,a0		// acc = coeff[4] - acc
	lw	t0,28(a3)		// T0 <- coeff[3] = 119194166 = 0.0555041086190613 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[3]
	sub	a0,t0,a0		// acc = coeff[3] - acc
	lw	t0,32(a3)		// T0 <- coeff[2] = 515882494 = 0.240226506255567 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[2]
	sub	a0,t0,a0		// acc = coeff[2] - acc
	lw	t0,36(a3)		// T0 <- coeff[1] = 1488522236 = 0.693147180601954 << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[1]
	sub	a0,t0,a0		// acc = coeff[1] - acc
	lui	t0,%hi(0x80000000)	// T0 <- coeff[0] = 2147483648 = 1.0f << 31
	mulhu	a0,a0,a4		// accumulator *= X

	// coeff[0]
	sub	a0,t0,a0		// acc = coeff[0] - acc

	// mantissa correction
	// - result mantissa is acc = 0x40000000 .. 0x7FFFFFFF = 0.5 .. 0.9999
	slli	a0,a0,1			// acc <<= 1
	add	a1,a1,-1		// exp--

	// rounding
4:	addi	a0,a0,0x80+1		// add rounding correction
	bltz	a0,5f			// no overflow
	li	a0,-1			// limit mantissa on overflow to 0xFFFFFFFF (do not do exponent correction - could overflow to inf)

	// Normalize mantissa
5:	srli	a0,a0,8			// shift mantissa to position

	// compose number
	addi	a1,a1,FLOAT_EXP1 - 1	// add bias to exponent (use "exponent - 1" to clear hidden bit '1' of the mantissa)
	slli	a1,a1,23		// shift exponent to position
	add	a0,a0,a1		// compose number
	ret

	// number is negative - check minimal negative number
12:	li	t0,0xC2AEAC4F		// min. valid value: -87.336540 = 0xC2AEAC4F, zero: -87.336548 = 0xC2AEAC50
	bleu	a0,t0,1b		// number is valid

	// negative overflow - return zero
	li	a0,0			// +0.0
	ret

expf_13:
	// return positive infinity
13:	li	a0,0x7f800000		// +inf
	ret

expf_14:
	// exponent is too small - return 1.0
14:	li	a0,0x3F800000		// +1.0
	ret

// ---- continue to exp2f

// ----------------------------------------------------------------------------
//                    Binary exponent - Chebyshev approximation
// ----------------------------------------------------------------------------
// float exp2f(float x);
// INPUT: A0 = x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4, T0, T1
// 433ns, err avg=0.109, err max=1

.global __wrap_exp2f
.global exp2f
__wrap_exp2f:
exp2f:

	// check if number A0 is negative
	bltz	a0,12f			// number is negative

	// number A0 is positive - check maximal positive number
	li	t0,0x42FFFFFF		// T0 max. valid value: 127.99999 = 0x42FFFFFF, infinity: 128.00000 = 0x43000000
	bgtu	a0,t0,expf_13		// number is not valid - return infinity

	// get signed exponent -> A1
1:	slli	a1,a0,1			// shift 1 left to delete sign bit
	srli	a1,a1,24		// shift exponent to base position
	addi	a1,a1,-FLOAT_EXP1	// subtract exopnent bias

	// check minimal exponent A1 - return 1.0
	// - value with result 1.0000001 = 0x3F800001: 8.5991324e-8 = 0x33B8AA3B, exponent FLOAT_EXP1-24
	// - value with result 1.0000000 = 0x3F800000: 8.5991317e-8 = 0x33B8AA3A, exponent FLOAT_EXP1-24
	li	t0,-24			// minimal valid exponent
	blt	a1,t0,expf_14		// exponent is too small - return +1.0

	// get sign -> A2
	srli	a2,a0,31		// A2 <- sign 0 or 1

// Here is:
//  A0 = input number x
//  A1 = signed exponent 
//  A2 = sign

	// normalize mantissa A0 - shift to max position and restore hidden bit '1'
	// - mantissa will be 0x80000000..0xFFFFFF00
	slli	a0,a0,8			// destroy exponent and sign
	bset	a0,a0,31		// set hidden bit '1'

	// prepare pointer to table of constants -> A3
	la	a3,expf_chebtab		// A3 <- table pointer

	// shift mantissa A0 right by exponent bits (shift >>= 7 - exp)
	// - get x as signed fixed number in format 8.24
	// - mantissa will be 0x00000001..0x58B90B80 or 0x57562780
	// - exp is in range -24..+6, shift is in range 1..31
	li	t0,7			// T0 <- number of shifts
	sub	t0,t0,a1		// T0 <- 7 - exp
	srl	a0,a0,t0		// mantissa >>= 31 - exp - 24

	// get integer part of exponent
	// - exponent will be 0..+127
	srli	a1,a0,24		// A1 <- exponent = mantissa >> 24

	// convert mantissa to range 0..1.0 ... number will be in 0.32 format
	slli	a4,a0,8			// A4 <- mantissa << 8
	j	expf_exp2f_jump

	// number is negative - check minimal negative number
12:	li	t0,0xC2FC0000		// min. valid value: -126.00000 = 0xC2FC0000, zero: -126.00001 = 0xC2FC0001
	bleu	a0,t0,1b		// number is valid

	// negative overflow - return zero
	li	a0,0			// +0.0
	ret

// ---- continue to exp10f

// ----------------------------------------------------------------------------
//                    Decimal exponent - Chebyshev approximation
// ----------------------------------------------------------------------------
// float exp2f(float x);
// INPUT: A0 = x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4, T0, T1
// 530ns, err avg=0.329, err max=1

.global __wrap_exp10f
.global exp10f
__wrap_exp10f:
exp10f:

	// check if number A0 is negative
	bltz	a0,12f			// number is negative

	// number A0 is positive - check maximal positive number
	li	t0,0x421A209A		// T0 max. valid value: 38.531837 = 0x421A209A, infinity: 38.531841 = 0x421A209B
	bgtu	a0,t0,expf_13		// number is not valid - return infinity

	// get signed exponent -> A1
1:	slli	a1,a0,1			// shift 1 left to delete sign bit
	srli	a1,a1,24		// shift exponent to base position
	addi	a1,a1,-FLOAT_EXP1	// subtract exopnent bias

	// check minimal exponent A1 - return 1.0
	// - value with result 1.0000001 = 0x3F800001: 2.5885969e-8 = 0x32DE5BD9, exponent FLOAT_EXP1-26
	// - value with result 1.0000000 = 0x3F800000: 2.5885967e-8 = 0x32DE5BD8, exponent FLOAT_EXP1-26
	li	t0,-26			// minimal valid exponent
	ble	a1,t0,14f		// exponent is too small - return +1.0 (or maybe not?)

	// get sign -> A2
2:	srli	a2,a0,31		// A2 <- sign 0 or 1

// Here is:
//  A0 = input number x
//  A1 = signed exponent 
//  A2 = sign

	// prepare pointer to table of constants -> A3
	la	a3,expf_chebtab		// A3 <- table pointer

	// normalize mantissa A0 - shift to max position and restore hidden bit '1'
	// - mantissa will be 0x80000000..0xFFFFFF00
	slli	a0,a0,8			// destroy exponent and sign
	lw	t0,40(a3)		// T0 <- ln(10) << 30 = 2472381918 round up
	bset	a0,a0,31		// set hidden bit '1'

	// multiply mantissa by ln(10) << 30 (= 2472381918 round up) = mantissa/4
	mulhu	a0,a0,t0		// A0 <- multiply mantissa * ln(10)/4
	addi	a1,a1,2			// shift exponent + 2

	// normalize mantissa
	bltz	a0,expf_exp10f_jump	// number is already normalized to >= 0x80000000
	slli	a0,a0,1			// mantissa <<= 1
	addi	a1,a1,-1		// shift exponent - 1
	j	expf_exp10f_jump

	// number is negative - check minimal negative number
12:	li	t0,0xC217B818		// min. valid value: -37.929779 = 0xC217B818, zero: -37.929783 = 0xC217B819
	bleu	a0,t0,1b		// number is valid

	// negative overflow - return zero
	li	a0,0			// +0.0
	ret

	// exponent is too small - return 1.0
14:	blt	a1,t0,15f		// exponent is really too small - return +1.0
	li	t0,0x32DE5BD9*2		// minimal valid value*2
	slli	a4,a0,1			// A4 <- number without sign
	bgeu	a4,t0,2b		// number is not so small, continue

15:	li	a0,0x3F800000		// +1.0
	ret

float_section _expf_cheb_tab

// expf table of constants
.p2align 2,,
expf_chebtab:
	.word	3098164010+1		// 0: 1/ln(2) << 31 = 3098164010 round up (= 0xB8AA3B2A)
	.word	2977044472		// 4: ln(2) << 32 = 2977044472 round up (= 0xB17217F8)
	.word	0x00000B3C		// 8: coeff[8] = 2876 = 1.3392906481613e-006 << 31
	.word	0x00007FF1		// 12: coeff[7] = 32753 = 1.52517549025788e-005 << 31
	.word	0x00050C03		// 16: coeff[6] = 330755 = 0.000154019750482348 << 31
	.word	0x002BB101		// 20: coeff[5] = 2863361 = 0.00133335616465047 << 31
	.word	0x013B2AC3		// 24: coeff[4] = 20654787 = 0.00961813466710737 << 31
	.word	0x071AC236		// 28: coeff[3] = 119194166 = 0.0555041086190613 << 31
	.word	0x1EBFBDFE		// 32: coeff[2] = 515882494 = 0.240226506255567 << 31
	.word	0x58B90BFC		// 36: coeff[1] = 1488522236 = 0.693147180601954 << 31
	.word	2472381918+1		// 40: ln(10) << 30 = 2472381918 round up

#endif // USE_RISCV_EXPF == 1

// ----------------------------------------------------------------------------
//                      Natural exponent - Taylor serie
// ----------------------------------------------------------------------------
// float expf(float x);
// INPUT: A0 = x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4, T0, T1
// 533ns, err avg=0.159, err max=1

// It is better to use the Chebyshev approximation method (see above),
// which is a little faster and a little more accurate.

#if USE_RISCV_EXPF == 2		// RISC-V expf(), exp2f() and exp10f() function: 0=libc, 1=Chebyshev (recommended), 2=Taylor, 3=Cordic

float_section _expf
.p2align 2,,

.global __wrap_expf
.global expf
__wrap_expf:
expf:

// Here is: A0 = input number

	// check if number A0 is negative
	bltz	a0,12f			// number is negative

	// number A0 is positive - check maximal positive number
	li	t0,0x42B17217		// T0 max. valid value: 88.722832 = 0x42B17217, infinity: 88.722839 = 0x42B17218
	bgtu	a0,t0,13f		// number is not valid - return infinity

	// get signed exponent -> A1
1:	slli	a1,a0,1			// shift 1 left to delete sign bit
	srli	a1,a1,24		// shift exponent to base position
	addi	a1,a1,-FLOAT_EXP1	// subtract exopnent bias

	// check minimal exponent A1 - return 1.0
	// - value with result 1.0000001 = 0x3F800001: 5.9604652e-8 = 0x33800001, exponent FLOAT_EXP1-24
	// - value with result 1.0000000 = 0x3F800000: 5.9604641e-8 = 0x337FFFFF, exponent FLOAT_EXP1-25
	li	t0,-24			// minimal valid exponent
	blt	a1,t0,14f		// exponent is too small - return +1.0

	// get sign -> A2
	srli	a2,a0,31		// A2 <- sign 0 or 1

// Here is:
//  A0 = input number x
//  A1 = signed exponent 
//  A2 = sign

	// normalize mantissa A0 - shift to max position and restore hidden bit '1'
	// - mantissa will be 0x80000000..0xFFFFFF00 (or with limit 0xB1721700 or 0xAEAC4F00)
	slli	a0,a0,8			// destroy exponent and sign
	bset	a0,a0,31		// set hidden bit '1'

	// prepare pointer to table of constants -> A3
	la	a3,expf_tab		// A3 <- table pointer

expf_exp10f_jump:

	// shift mantissa A0 right by exponent bits (shift >>= 7 - exp)
	// - get x as signed fixed number in format 8.24
	// - mantissa will be 0x00000001..0x58B90B80 or 0x57562780
	// - exp is in range -24..+6, shift is in range 1..31
	li	t0,7			// T0 <- number of shifts
	sub	t0,t0,a1		// T0 <- 7 - exp
	srl	a0,a0,t0		// mantissa >>= 31 - exp - 24

	// prepare constant 1/ln(2) << 30 -> T0
	lw	t0,0(a3)		// T0 <- 1549082005 (1/ln(2)) (= 0x5C551D95)

	// signed mantissa A0
	// - mantissa will be -0x57562780..0x58B90B80
	beqz	a2,2f			// sign is positive
	neg	a0,a0			// negate mantissa

	// convert mantissa to 2-base and get integer part of exponent -> A1
	// multiply by 1/ln(2) and shift >> 24, round to zero
	// constant: 1/ln(2) << 30 = 1549082005 round up (= 0x5C551D95)
	// - exponent will be -126..+127
2:	mulh	a1,a0,t0		// A1 (exponent) <- mantissa * 1549082005 (signed)
	lw	t0,4(a3)		// T0 <- 1488522236 (ln(2)) (= 0x58B90BFC)
	srai	a1,a1,24+30 - 32	// signed shift right = 22

	// convert exponent back to e-base: multiply by ln(2) and convert to 8.24 format, round to zero
	//   ln(2) << 31 = 1488522236 round up
	// - exponent correction will be -0x002BAB13E608..+0x002C03CCF204 >> 7 -> -0x575627CC..+0x580799E4
	//  Later we will discard the upper 8 bits of the result,
	//  so we will not need the HIGH result of the multiplication.
	mul	t0,a1,t0		// T0 <- exp * 1488522236
	srai	t0,t0,7			// result >> 7

	// subtract exponent correction from the number
	// - number will now be in range 0x00000001..0x00B17217 (=0.00000006..0.69314712)
	sub	a0,a0,t0		// mantissa -= exponent part

expf_exp2f_jump:
	// use mantissa in format 0.32 -> A2
	lw	t0,36(a3)		// T0 <- 0xB1721700 max. value of the mantissa
	slli	a2,a0,8			// A2 <- mantissa << 8

	// inconsistency correction (due to rounding it can overflow a litle above 0x00B17217)
	bleu	a2,t0,3f		// compare woth 0xB1721700, skip if no overflow
	mv	a2,t0			// limit mantissa on overflow to 0xB1721700

	// + x^2/2!
3:	mulhu	a4,a2,a2		// A4 <- serie member = xn*xn
	srli	a4,a4,1			// A4 <- x^2/2
	add	a0,a2,a4		// add member to accumulator A0 <- x + x^2/2!

// Here is:
//  A0 = accumulator of the result 'acc' = x + x^2/2!
//  A1 = signed exponent 'exp'
//  A2 = signed mantissa in format 0.32, without implied bit 1 'x'
//  A3 = table pointer
//  A4 = serie member 'm' = x*x

	// + x^3/3!
	lw	t1,8(a3)		// T1 <- 1/3 << 32 = 1431655766 round up (= 0x55555556)
	mulhu	a4,a4,a2		// A4 <- serie member = m*xn
	mulhu	a4,a4,t1		// A4 <- m * 1431655766
	add	a0,a0,a4		// add member to accumulator

	// + x^4/4!
	mulhu	a4,a4,a2		// A4 <- m = m*xn
	srli	a4,a4,2			// A4 <- m/4
	add	a0,a0,a4		// add member to accumulator

	// + x^5/5!
	li	t0,5
	lw	t1,12(a3)		// T1 <- 1/5 << 32 = 858993460 round up (= 0x33333334)
	mulhu	a4,a4,a2		// A4 <- m = m*xn
	bltu	a4,t0,4f		// end of iteration
	mulhu	a4,a4,t1		// A4 <- m * 858993460
	add	a0,a0,a4		// add member to accumulator

	// + x^6/6!
	lw	t1,16(a3)		// T1 <- 1/6 << 32 = 715827883 round up (= 0x2AAAAAAB)
	mulhu	a4,a4,a2		// A4 <- m = m*xn
	bleu	a4,t0,4f		// end of iteration
	mulhu	a4,a4,t1		// A4 <- m * 715827883
	add	a0,a0,a4		// add member to accumulator

	// + x^7/7!
	lw	t1,20(a3)		// T1 <- 1/7 << 32 = 613566757 round up (= 0x24924925)
	mulhu	a4,a4,a2		// A4 <- m = m*xn
	mulhu	a4,a4,t1		// A4 <- m * 613566757
	add	a0,a0,a4		// add member to accumulator

	// + x^8/8!
	mulhu	a4,a4,a2		// A4 <- m = m*xn
	srli	a4,a4,3			// A4 <- m/8
	add	a0,a0,a4		// add member to accumulator

	// + x^9/9!
	li	t0,9
	lw	t1,24(a3)		// T1 <- 1/9 << 32 = 477218589 round up (= 0x1C71C71D)
	mulhu	a4,a4,a2		// A4 <- m = m*xn
	bltu	a4,t0,4f		// end of iteration
	mulhu	a4,a4,t1		// A4 <- m * 477218589
	add	a0,a0,a4		// add member to accumulator

	// + x^10/10!
	lw	t1,28(a3)		// T1 <- 1/10 << 32 = 429496730 round up (= 0x1999999A)
	mulhu	a4,a4,a2		// A4 <- m = m*xn
	bleu	a4,t0,4f		// end of iteration
	mulhu	a4,a4,t1		// A4 <- m * 429496730
	add	a0,a0,a4		// add member to accumulator

	// + x^11/11!
	lw	t1,32(a3)		// T1 <- 1/11 << 32 = 390451573 round up (= 0x1745D175)
	mulhu	a4,a4,a2		// A4 <- m = m*xn
	mulhu	a4,a4,t1		// A4 <- m * 390451573
	add	a0,a0,a4		// add member to accumulator

	// Rounding
4:	addi	a0,a0,0x100		// add rounding correction

// acc before rounding is in range 0x00000100..0xFFFFFE0B, overflow will never occur, but beware
//	li	t0,0x100
//	bgeu	a0,t0,5f		// no overflow
//	li	a0,-1			// limit to 0xFFFFFFFF

	// Normalize mantissa
5:	srli	a0,a0,8+1		// acc >> 9

	// compose number
	addi	a1,a1,FLOAT_EXP1	// add bias to exponent
	slli	a1,a1,23		// shift exponent to position
	or	a0,a0,a1		// compose number
	ret

	// number is negative - check minimal negative number
12:	li	t0,0xC2AEAC4F		// min. valid value: -87.336540 = 0xC2AEAC4F, zero: -87.336548 = 0xC2AEAC50
	bleu	a0,t0,1b		// number is valid

	// negative overflow - return zero
	li	a0,0			// +0.0
	ret

expf_13:
	// return positive infinity
13:	li	a0,0x7f800000		// +inf
	ret

expf_14:
	// exponent is too small - return 1.0
14:	li	a0,0x3F800000		// +1.0
	ret

// ---- continue to exp2f

// ----------------------------------------------------------------------------
//                    Binary exponent - Taylor serie
// ----------------------------------------------------------------------------
// float exp2f(float x);
// INPUT: A0 = x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4, T0, T1
// 501ns, err avg=0.259, err max=1

.global __wrap_exp2f
.global exp2f
__wrap_exp2f:
exp2f:

	// check if number A0 is negative
	bltz	a0,12f			// number is negative

	// number A0 is positive - check maximal positive number
	li	t0,0x42FFFFFF		// T0 max. valid value: 127.99999 = 0x42FFFFFF, infinity: 128.00000 = 0x43000000
	bgtu	a0,t0,expf_13		// number is not valid - return infinity

	// get signed exponent -> A1
1:	slli	a1,a0,1			// shift 1 left to delete sign bit
	srli	a1,a1,24		// shift exponent to base position
	addi	a1,a1,-FLOAT_EXP1	// subtract exopnent bias

	// check minimal exponent A1 - return 1.0
	// - value with result 1.0000001 = 0x3F800001: 8.5991324e-8 = 0x33B8AA3B, exponent FLOAT_EXP1-24
	// - value with result 1.0000000 = 0x3F800000: 8.5991317e-8 = 0x33B8AA3A, exponent FLOAT_EXP1-24
	li	t0,-24			// minimal valid exponent
	blt	a1,t0,expf_14		// exponent is too small - return +1.0

	// get sign -> A2
	srli	a2,a0,31		// A2 <- sign 0 or 1

// Here is:
//  A0 = input number x
//  A1 = signed exponent 
//  A2 = sign

	// normalize mantissa A0 - shift to max position and restore hidden bit '1'
	// - mantissa will be 0x80000000..0xFFFFFF00
	slli	a0,a0,8			// destroy exponent and sign
	bset	a0,a0,31		// set hidden bit '1'

	// prepare pointer to table of constants -> A3
	la	a3,expf_tab		// A3 <- table pointer

	// shift mantissa A0 right by exponent bits (shift >>= 7 - exp)
	// - get x as signed fixed number in format 8.24
	// - mantissa will be 0x00000001..0x58B90B80 or 0x57562780
	// - exp is in range -24..+6, shift is in range 1..31
	li	t0,7			// T0 <- number of shifts
	sub	t0,t0,a1		// T0 <- 7 - exp
	srl	a0,a0,t0		// mantissa >>= 31 - exp - 24

	// signed mantissa A0
	// - mantissa will be -0x57562780..0x58B90B80
	beqz	a2,2f			// sign is positive
	neg	a0,a0			// negate mantissa

	// get integer part of exponent
	// - exponent will be -126..+127
2:	srai	a1,a0,24		// A1 <- exponent = mantissa >> 24

	// convert exponent back to base
	slli	a4,a1,24		// A4 <- exp << 24

	// subtract exponent correction from the number
	lw	t0,40(a3)		// T0 <- ln(2) << 32 = 2977044472 round up
	sub	a0,a0,a4		// mantissa -= exponent part

	// convert to range ln(2)
	//   ln(2) << 32 = 2977044472 round up
	mulhu	a0,a0,t0		// mantissa * ln(2)
	j	expf_exp2f_jump

	// number is negative - check minimal negative number
12:	li	t0,0xC2FC0000		// min. valid value: -126.00000 = 0xC2FC0000, zero: -126.00001 = 0xC2FC0001
	bleu	a0,t0,1b		// number is valid

	// negative overflow - return zero
	li	a0,0			// +0.0
	ret

// ---- continue to exp10f

// ----------------------------------------------------------------------------
//                    Decimal exponent - Taylor serie
// ----------------------------------------------------------------------------
// float exp2f(float x);
// INPUT: A0 = x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4, T0, T1
// 584ns, err avg=0.324, err max=2

.global __wrap_exp10f
.global exp10f
__wrap_exp10f:
exp10f:

	// check if number A0 is negative
	bltz	a0,12f			// number is negative

	// number A0 is positive - check maximal positive number
	li	t0,0x421A209A		// T0 max. valid value: 38.531837 = 0x421A209A, infinity: 38.531841 = 0x421A209B
	bgtu	a0,t0,expf_13		// number is not valid - return infinity

	// get signed exponent -> A1
1:	slli	a1,a0,1			// shift 1 left to delete sign bit
	srli	a1,a1,24		// shift exponent to base position
	addi	a1,a1,-FLOAT_EXP1	// subtract exopnent bias

	// check minimal exponent A1 - return 1.0
	// - value with result 1.0000001 = 0x3F800001: 2.5885969e-8 = 0x32DE5BD9, exponent FLOAT_EXP1-26
	// - value with result 1.0000000 = 0x3F800000: 2.5885967e-8 = 0x32DE5BD8, exponent FLOAT_EXP1-26
	li	t0,-26			// minimal valid exponent
	ble	a1,t0,14f		// exponent is too small - return +1.0 (or maybe not?)

	// get sign -> A2
2:	srli	a2,a0,31		// A2 <- sign 0 or 1

// Here is:
//  A0 = input number x
//  A1 = signed exponent 
//  A2 = sign

	// prepare pointer to table of constants -> A3
	la	a3,expf_tab		// A3 <- table pointer

	// normalize mantissa A0 - shift to max position and restore hidden bit '1'
	// - mantissa will be 0x80000000..0xFFFFFF00
	slli	a0,a0,8			// destroy exponent and sign
	lw	t0,44(a3)		// T0 <- ln(10) << 30 = 2472381918 round up
	bset	a0,a0,31		// set hidden bit '1'

	// multiply mantissa by ln(10) << 30 (= 2472381918 round up) = mantissa/4
	mulhu	a0,a0,t0		// A0 <- multiply mantissa * ln(10)/4
	addi	a1,a1,2			// shift exponent + 2

	// normalize mantissa
	bltz	a0,expf_exp10f_jump	// number is already normalized to >= 0x80000000
	slli	a0,a0,1			// mantissa <<= 1
	addi	a1,a1,-1		// shift exponent - 1
	j	expf_exp10f_jump

	// number is negative - check minimal negative number
12:	li	t0,0xC217B818		// min. valid value: -37.929779 = 0xC217B818, zero: -37.929783 = 0xC217B819
	bleu	a0,t0,1b		// number is valid

	// negative overflow - return zero
	li	a0,0			// +0.0
	ret

	// exponent is too small - return 1.0
14:	blt	a1,t0,15f		// exponent is really too small - return +1.0
	li	t0,0x32DE5BD9*2		// minimal valid value*2
	slli	a4,a0,1			// A4 <- number without sign
	bgeu	a4,t0,2b		// number is not so small, continue

15:	li	a0,0x3F800000		// +1.0
	ret

float_section _expf_taylor_tab

// expf table of constants
.p2align 2,,
expf_tab:
	.word	1549082005		// 0: 1/ln(2) << 30 = 1549082005 round up (= 0x5C551D95)
	.word	1488522236		// 4: ln(2) << 31 = 1488522236 round up (= 0x58B90BFC)
	.word	1431655766		// 8: 1/3 << 32 = 1431655766 round up (= 0x55555556)
	.word	858993460		// 12: 1/5 << 32 = 858993460 round up (= 0x33333334)
	.word	715827883		// 16: 1/6 << 32 = 715827883 round up (= 0x2AAAAAAB)
	.word	613566757		// 20: 1/7 << 32 = 613566757 round up (= 0x24924925)
	.word	477218589		// 24: 1/9 << 32 = 477218589 round up (= 0x1C71C71D)
	.word	429496730		// 28: 1/10 << 32 = 429496730 round up (= 0x1999999A)
	.word	390451573		// 32: 1/11 << 32 = 390451573 round up (= 0x1745D175)
	.word	0xB1721700		// 36: max. value of the mantissa
	.word	2977044472		// 40: ln(2) << 32 = 2977044472 round up
	.word	2472381918		// 44: ln(10) << 30 = 2472381918 round up

#endif // USE_RISCV_EXPF == 2

// ----------------------------------------------------------------------------
//                         Natural logarithm - Cordic
// ----------------------------------------------------------------------------
// float logf(float x);
// INPUT: A0 = x
// OUTPUT: A0 = result
// TRASHED: A1-A7, T0-T5
// 4770ns, err avg=0.028, err max=6

#if USE_RISCV_LOGF == 3		// RISC-V logf(), log2f() and log10f() function: 0=libc, 1=Chebyshev (recommended), 2=Mercator, 3=Cordic

float_section _logf
.p2align 2,,

.global __wrap_logf
.global logf
__wrap_logf:
logf:

mufp_fln:
	slli	a1,a0,1
	srli	a1,a1,24
	beqz	a1,11f		// zero

	bltz	a0,1f		// negative number

	mv	t5,ra		// T5 <- save return address
// USED: A0-A7, T0-T4
	call	mufp_fsqrt_ln	// get unpacked ln in a1/a3; exponent has been halved
	mv	ra,t5		// RA <- restore return address

	li	t4,70		// ln(Inf)?
	bgt	a3,t4,2f	// return Inf
	neg	a3,a3
	bgt	a3,t4,1f	// ln(0)? return -Inf

3:	li	a0,0x0162e430	// ln(4) Q24
	mul	a0,a0,a3	// contribution from negated, halved exponent
	addi	a1,a1,8		// round result of ln
	srai	a1,a1,4		// Q24
	sub	a0,a1,a0	// add in contribution from (negated) exponent

	// pack number
	li	a2,5		// pack expects Q29
	li	a5,0		// pack and return, sticky bits=0
// input: A0 = signed mantissa (24 significant bits)
//        A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
//        A5 = sticky bits (used for rounding)
// output: A0 = float number
// trashes: A2, A4, A5, T0
	j	_asm_packx

11:	li	a0,0

1:	not	a0,a0		// make result -Inf

	// return Inf
2:	li	a2,255

	// pack number
	li	a5,0		// pack and return, sticky bits=0
// input: A0 = signed mantissa (24 significant bits)
//        A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
//        A5 = sticky bits (used for rounding)
// output: A0 = float number
// trashes: A2, A4, A5, T0
	j	_asm_packx


// calculate sqrt and ln using vector method
// USED: A0-A7, T0-T4
mufp_fsqrt_ln:

	mv	t4,ra		// T4 <- save return address
// input: A0 = float number
// output: A0 = signed mantissa 9Q23 (24 significant bits)
//         A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
// trashes: A4
	call	_asm_unpackx

	mv	a1,a0		// -ve argument?
	bltz	a1,3f		// return -Inf, -Inf

	li	a1,0x0593C2B9	// scale factor for CORDIC
// signed multiply a0 1Q23 by a1 4Q23, result in a0 7Q25, sticky bits in a5
// INPUT: A0, A1
// OUTPUT: A0, A5
// TRASHED: A1, A3, A4
// @TODO: optimise with MULH
	call	mul0		// Q29

	bexti	t3,a2,0		// T3 <- extract bit 0
	srai	a1,a2,1		// halve exponent
	beqz	t3,1f		// no carry
	addi	a1,a1,1		// was odd: add 1 and shift mantissa
	srai	a0,a0,1

1:	mv	t3,a1		// save exponent/2 for later
	mv	a1,a0
	li	a3,0x0593C2B9	// re-use constant
	slli	a3,a3,2
	add	a0,a0,a3	// "a+1"
	sub	a1,a1,a3	// "a-1"
	li	a2,0
	la	a3,tab_ch	// hyperbolic coefficients
	not	a4,a2		// m=-1

// INPUT: A0 = x
//	  A1 = y
//	  A2 = angle
//	  A3 = table pointer
//	  A4 = m
// OUTPUT: A0 = next x
//	   A1 = next y
//	   A2 = next angle
//	   A3 = next table pointer
//	   A4 = new m
//	   A5 = index
//	   A6, A7 = step
//	   T0 = m (copy of A4)
// TRASHED: T1, T2
	call	cordic_vec
	mv	a1,a2		// keep ln result
	mv	a2,t3		// retrieve exponent/2

	// pack number
2:	mv	a3,a2
	mv	ra,t4		// restore return address
	li	a5,0		// pack and return, sticky bits=0
// input: A0 = signed mantissa (24 significant bits)
//        A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
//        A5 = sticky bits (used for rounding)
// output: A0 = float number
// trashes: A2, A4, A5, T0
	j	_asm_packx

3:	li	a2,255
	j	2b

// signed multiply a0 1Q23 by a1 4Q23, result in a0 7Q25, sticky bits in a5
// INPUT: A0, A1
// OUTPUT: A0, A5
// TRASHED: A1, A3, A4
//   @TODO: optimise with MULH
mul0:
	zext.h	a3,a0		// Q23
	srai	a4,a1,16	// Q7
	mul	a3,a3,a4	// L*H, Q30 signed

	srai	a4,a0,16	// Q7
	zext.h	a5,a1		// Q23
	mul	a4,a4,a5	// H*L, Q30 signed

	add	a3,a3,a4	// sum of middle partial products
	zext.h	a4,a0
	mul	a4,a4,a5	// L*L, Q46 unsigned

	slli	a5,a4,16	// initialise sticky bits from low half of low partial product
	srli	a4,a4,16	// Q25
	add	a3,a3,a4	// add high half of low partial product to sum of middle partial products
				// (cannot generate carry by limits on input arguments)
	srai	a0,a0,16	// Q7
	srai	a1,a1,16	// Q7
	mul	a0,a0,a1	// H*H, Q14 signed

	slli	a0,a0,11	// high partial product Q25
	slli	a1,a3,27	// sticky
	or	a5,a5,a1	// collect further sticky bits
	srai	a1,a3,5		// middle partial products Q25
	add	a0,a0,a1	// final result
	ret

#endif // USE_RISCV_LOGF == 3

// ----------------------------------------------------------------------------
//                   Natural logarithm - Chebyshev approximation
// ----------------------------------------------------------------------------
// float logf(float x);
// float log2f(float x);
// float log10f(float x);
// INPUT: A0 = x
// OUTPUT: A0 = result
// TRASHED: A1,A2,A3,A4,A5,T0
// logf(): 616ns, err avg=0.045, err max=2
// log2f(): 650ns, err avg=0.005, err max=1
// log10f(): 676ns, err avg=0.283, err max=2

#if USE_RISCV_LOGF == 1		// RISC-V logf(), log2f() and log10f() function: 0=libc, 1=Chebyshev (recommended), 2=Mercator, 3=Cordic

float_section _logf
.p2align 2,,

.global __wrap_log2f
.global log2f
__wrap_log2f:
log2f:
	li	a5,1			// A5 <- 1, log2f() mode
	j	logf_start

.global __wrap_log10f
.global log10f
__wrap_log10f:
log10f:
	li	a5,-1			// A5 <- -1, log10f() mode
	j	logf_start

.global __wrap_logf
.global logf
__wrap_logf:
logf:
	li	a5,0			// A5 <- 0, logf() mode

logf_start:

	// result is 0 if number is 1
	li	a1,0x3F800000		// value 1.0f
	beq	a0,a1,10f		// result is 0

	// get unsigned exponent -> A1
	slli	a1,a0,1			// destroy sign bit
	srli	a1,a1,24		// isolate unsigned exponent -> A1

	// zero returns -inf (both positive and negative zero)
	beqz	a1,11f			// number is zero, return -inf

	// negative number returns +inf
	bltz	a0,12f			// negative number returns +inf

	// positive infinity returns +inf
	li	t0,0xff			// T0 <- infinity exponent
	beq	a1,t0,12f		// positive infinity returns +inf

	// get signed exponent -> A1
	addi	a1,a1,-FLOAT_EXP1	// signed exponent -> A1

	// prepare table address -> A3
	la	a3,logf_tab		// A3 <- table address

	// get mantissa in 1.31 format signed, convert from range 1.0000..1.9999 to range -1.0000..0.9999 -> A2
	//  s32 m = ((xn<<8)|B31);	// convert mantissa to 1.31 format, range 1.0000..1.9999
	//  m -= B31;			// subtract 1, convert to range 0.0000..0.9999
	//  m <<= 1;			// mantissa * 2, convert to range 0.0000..1.9999
	//  m -= B31;			// subtract 1, convert to range -1.0000..0.9999
	slli	a0,a0,9			// convert to 1.31 format * 2
	li	t0,0x80000000		// bit 31
	sub	a2,a0,t0		// subtract 1, convert to range -1.0000..0.9999

// Here is:
//  A0 = result accumulator
//  A1 = signed exponent
//  A2 = mantissa
//  A3 = table address

	// coeff[12]
	li	a0,-930			// A0 <- coeff[12] = -930 = -2,16502815508779e-007 << 32
	mulh	a0,a0,a2		// A0 <- accumulator * mantissa

	// coeff[11]
	li	t0,1515			// T0 <- coeff[11] = 1515 = 7.05661007405212e-007 << 31
	add	a0,a0,t0		// A0 <- add coeff
	lw	t0,40(a3)		// T0 <- coeff[10] = -3466 = -1.61398543996327e-006 << 31
	mulh	a0,a0,a2		// A0 <- accumulator * mantissa

	// coeff[10]
	sh1add	a0,a0,t0		// A0 <- add coeff
	lw	t0,36(a3)		// T0 <- coeff[ 9] =  11572 = 5.38877348255085e-006 << 31
	mulh	a0,a0,a2		// A0 <- accumulator * mantissa

	// coeff[9]
	sh1add	a0,a0,t0		// A0 <- add coeff
	lw	t0,32(a3)		// T0 <- coeff[ 8] = -41019 = -1.91011454901968e-005 << 31
	mulh	a0,a0,a2		// A0 <- accumulator * mantissa

	// coeff[8]
	sh1add	a0,a0,t0		// A0 <- add coeff
	lw	t0,28(a3)		// T0 <- coeff[ 7] =  140616 = 6.54794611136822e-005 << 31
	mulh	a0,a0,a2		// A0 <- accumulator * mantissa

	// coeff[7]
	sh1add	a0,a0,t0		// A0 <- add coeff
	lw	t0,24(a3)		// T0 <- coeff[ 6] = -490934 = -0.000228609107864486 << 31
	mulh	a0,a0,a2		// A0 <- accumulator * mantissa

	// coeff[6]
	sh1add	a0,a0,t0		// A0 <- add coeff
	lw	t0,20(a3)		// T0 <- coeff[ 5] =  1767375 = 0.000822998312287382 << 31
	mulh	a0,a0,a2		// A0 <- accumulator * mantissa

	// coeff[5]
	sh1add	a0,a0,t0		// A0 <- add coeff
	lw	t0,16(a3)		// T0 <- coeff[ 4] = -6628040 = -0.00308642159143346 << 31
	mulh	a0,a0,a2		// A0 <- accumulator * mantissa

	// coeff[4]
	sh1add	a0,a0,t0		// A0 <- add coeff
	lw	t0,12(a3)		// T0 <- coeff[ 3] = 26512157 = 0.0123456849323702 << 31
	mulh	a0,a0,a2		// A0 <- accumulator * mantissa

	// coeff[3]
	sh1add	a0,a0,t0		// A0 <- add coeff
	lw	t0,8(a3)		// T0 <- coeff[ 2] = -119304647 = -0.0555555554892635 << 31
	mulh	a0,a0,a2		// A0 <- accumulator * mantissa

	// coeff[2]
	sh1add	a0,a0,t0		// A0 <- add coeff
	lw	t0,4(a3)		// T0 <- coeff[ 1] = 715827882 = 0.333333333139308 << 31
	mulh	a0,a0,a2		// A0 <- accumulator * mantissa

	// coeff[1]
	sh1add	a0,a0,t0		// A0 <- add coeff
	lw	t0,0(a3)		// T0 <- coeff[ 0] = 870729689 = 0.405465108109638 << 31
	mulh	a0,a0,a2		// A0 <- accumulator * mantissa

	// coeff[0]
	sh1add	a0,a0,t0		// A0 <- add coeff

// Here is:
//  A0 = result unsigned mantissa in 1.31 format, value 0..0.6931 (log(2)) = 0x00000001..0x58B90BFC
//  A1 = signed exponent
//  A3 = table address

	// go log2f() or log10f() path
	bgtz	a5,log2f_path		// go log2f() path
	bltz	a5,log10f_path		// go log10f() path

// logf() path

	// round mantissa up
	addi	a0,a0,0x40

	// convert mantissa from format 1.31 to format 8.24
	lw	t0,44(a3)		// T0 <- log(2) << 24 = 11629080 round up
	srai	a0,a0,7			// mantissa >> 7

	// multiply exponent * log(2) and convert to format 8.24
	mul	a1,a1,t0		// A1 <- exponent * log(2)
	add	a0,a0,a1		// add exponent to mantissa

// return from log2f() path
log2f_return:

	// prepare result exponent base -> A1
	li	a1,FLOAT_EXP1 - 1	// A1 <- exponent base

// return from log10f() path
log10f_return:

// Here is:
//  A0 = result signed mantissa in 8.24 or 7.25 format
//  A1 = result exponent base
//  A2 = result sign (will be prepared later)
//  A3 = table address, no longer needed - will be used for number of shifts

	// zero result
	beqz	a0,13f			// mantissa is zero

	// negative mantissa A0, sign -> A2
	li	a2,0			// A2 <- positive sign flag
	bgez	a0,3f			// result is not negative
	neg	a0,a0			// absolute value of the mantissa
	li	a2,0x80000000		// A2 <- negative sign flag

	// normalize mantissa A0, get shifts A3
3:	clz	t0,a0			// T0 <- count leading zeroes
	li	a3,8			// A3 <- 8
	sub	a3,a3,t0		// A3 <- number of remaining bits to normalize = 8 - clz(mantissa)

	// prepare result exponent -> A1
	add	a1,a1,a3		// A1 <- add number of shifts to result exponent

	// shift mantissa A0 up (A3 <= 0)
	bgtz	a3,4f			// shift up (A3 > 0)
	neg	a3,a3			// A3 <- - shifts
	sll	a0,a0,a3		// shift mantissa left
	j	5f

	// shift mantissa A0 right (A3 > 0)
4:	addi	a3,a3,-1		// number of shifts - 1
	srl	a0,a0,a3		// shift mantissa right - 1
	addi	a0,a0,1			// rounding mantissa up
	srli	a0,a0,1			// finish last 1 shift

	// carry on rounding
	srli	t0,a0,24		// check mantissa overflow
	beqz	t0,5f			// no overflow
	srli	a0,a0,1			// shift mantissa 1 bit right
	addi	a1,a1,1			// increment exponent

	// clear hidden bit '1' from the mantissa A0
5:	slli	a0,a0,9
	srli	a0,a0,9

	// compose result A0
	slli	a1,a1,23		// shift exponent to position
	or	a0,a0,a1		// add exponent to the result
	or	a0,a0,a2		// add sign to the result
	ret

	// result is 0
10:	li	a0,0			// value 0
	ret

	// result is -inf
11:	li	a0,0xff800000		// value -inf
	ret

	// result is +inf
12:	li	a0,0x7f800000		// value +inf
13:	ret

// log2f() path
log2f_path:

	// round mantissa up
	addi	a0,a0,0x2c		// = 0x40 * log(2) = 0x2C round down

	// multiply 1.31 mantissa * 1/log(2) and convert to format 8.24
	// 1/log(2) << 25 = 48408813 round up
	lw	t0,48(a3)		// T0 <- 1/log(2) << 25 = 48408813 round up
	slli	a1,a1,24		// A1 <- shift exponent to position
	mulhu	a0,a0,t0		// A0 <- mantissa * 1/log(2)

	// add exponent in format 8.24
	add	a0,a0,a1		// add exponent to mantissa
	j	log2f_return

// log10f() path
log10f_path:

	// round mantissa up
	lw	t0,52(a3)		// T0 <- 1/log(10) << 26 = 29145009 round down
	addi	a0,a0,0x49		// = 0x20 * log(10) = 0x49 round down

	// multiply 1.31 mantissa * 1/log(10) and convert to format 7.25
	// 1/log(10) << 26 = 29145009 round down
	mulhu	a0,a0,t0		// A0 <- mantissa * 1/log(10)

	// multiply exponent * log(10) and convert to format 7.25
	// log(2)/log(10) << 25 = 10100890 round down
	li	t0,10100890		// T0 <- log(2)/log(10) << 25 = 10100890 round down
	mul	a1,a1,t0		// A1 <- exponent * log(2)/log(10)
	add	a0,a0,a1		// add exponent to mantissa

	// prepare result exponent base -> A1
	li	a1,FLOAT_EXP1 - 2	// A1 <- exponent base
	j	log10f_return

float_section _logf_cheb_tab

// logf table of constants
.p2align 2,,
logf_tab:
	.word	870729689		//  0: coeff[ 0] =  870729689 = 0.405465108109638 << 31
	.word	715827882		//  4: coeff[ 1] =  715827882 = 0.333333333139308 << 31
	.word	-119304647		//  8: coeff[ 2] = -119304647 = -0.0555555554892635 << 31
	.word	26512157		// 12: coeff[ 3] =  26512157 = 0.0123456849323702 << 31
	.word	-6628040		// 16: coeff[ 4] = -6628040 = -0.00308642159143346 << 31
	.word	1767375			// 20: coeff[ 5] =  1767375 = 0.000822998312287382 << 31
	.word	-490934			// 24: coeff[ 6] = -490934 = -0.000228609107864486 << 31
	.word	140616			// 28: coeff[ 7] =  140616 = 6.54794611136822e-005 << 31
	.word	-41019			// 32: coeff[ 8] = -41019 = -1.91011454901968e-005 << 31
	.word	11572			// 36: coeff[ 9] =  11572 = 5.38877348255085e-006 << 31
	.word	-3466			// 40: coeff[10] = -3466 = -1.61398543996327e-006 << 31
	.word	11629080		// 44: log(2) << 24 = 11629080 round up
	.word	48408813		// 48: 1/log(2) << 25 = 48408813 round up
	.word	29145009		// 52: 1/log(10) << 26 = 29145009 round down

#endif // USE_RISCV_LOGF == 1

// ----------------------------------------------------------------------------
//                      Natural logarithm - Mercator serie
// ----------------------------------------------------------------------------
// float logf(float x);
// float log2f(float x);
// float log10f(float x);
// INPUT: A0 = x
// OUTPUT: A0 = result
// TRASHED: A1,A2,A3,A4,A5,T0,T1
// logf(): 822ns, err avg=0.034, err max=6
// log2f(): 876ns, err avg=0.011, err max=6
// log10f(): 882ns, err avg=0.289, err max=5

#if USE_RISCV_LOGF == 2		// RISC-V logf(), log2f() and log10f() function: 0=libc, 1=Chebyshev (recommended), 2=Mercator, 3=Cordic

float_section _logf
.p2align 2,,

.global __wrap_log2f
.global log2f
__wrap_log2f:
log2f:
	li	a5,1			// A5 <- 1, log2f() mode
	j	logf_start

.global __wrap_log10f
.global log10f
__wrap_log10f:
log10f:
	li	a5,-1			// A5 <- -1, log10f() mode
	j	logf_start

.global __wrap_logf
.global logf
__wrap_logf:
logf:
	li	a5,0			// A5 <- 0, logf() mode

logf_start:

	// result is 0 if number is 1
	li	a1,0x3F800000		// value 1.0f
	beq	a0,a1,10f		// result is 0

	// get unsigned exponent -> A1
	slli	a1,a0,1			// destroy sign bit
	srli	a1,a1,24		// isolate unsigned exponent -> A1

	// zero returns -inf (both positive and negative zero)
	beqz	a1,11f			// number is zero, return -inf

	// negative number returns +inf
	bltz	a0,12f			// negative number returns +inf

	// positive infinity returns +inf
	li	t0,0xff			// T0 <- infinity exponent
	beq	a1,t0,12f		// positive infinity returns +inf

	// get signed exponent, to have mantissa in range 0.5..0.9999 -> A1
	addi	a1,a1,-FLOAT_EXP1 + 1	// signed exponent -> A1

	// prepare table address -> A3
	la	a3,logf_tab		// A3 <- table address

	// get signed mantissa in 1.31 format, range 0.5..0.9999 = 0x40000000..0x7FFFFF80
	slli	a0,a0,9			// discard exponent and sign
	srli	a0,a0,2			// shift  to position of value 0.5..0.9999 in 1.31 format
	bseti	a0,a0,30		// set hidden implied bit '1' on position 0.5

	// Prepare number D = x - 1 (input interval is 0.5..0.9999)
	// For simplicity, we will work with the negative value of D, negating the result.
	//   D = 1 - x,  -ln(x) = + D + D^2/2 + D^3/3 + D^4/4 +
	li	a4,0x80000000		// A4 <- value "1"
	sub	a2,a4,a0		// A2 <- B31 - m = "D" coefficient

	// prepare result accumulator = D
	mv	a0,a2			// A0 <- result accumulator = D

	// pre-set D * 2, because mul-high uses >> 32 and we need >> 31
	slli	a4,a2,1			// D coefficient * 2

// Here is:
//  A0 = result accumulator 'acc'
//  A1 = signed exponent 'exp'
//  A2 = serie member 'm'
//  A3 = table address
//  A4 = D coefficient * 2 'd'

	// D^2/2
	mulhu	a2,a2,a4		// m = m*d
	srli	t0,a2,1			// T0 <- m/2
	lw	t1,0(a3)		// T1 <- 1/3 << 32, 1431655766
	add	a0,a0,t0		// add member/n to accumulator

	// D^3/3
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	add	a0,a0,t0		// add member/n to accumulator

	// D^4/4
	mulhu	a2,a2,a4		// m = m*d
	srli	t0,a2,2			// T0 <- m/4
	lw	t1,4(a3)		// T0 <- 1/5 << 32, 858993460
	add	a0,a0,t0		// add member/n to accumulator

	// D^5/5
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	lw	t1,8(a3)		// T0 <- 1/6 << 32, 715827883
	add	a0,a0,t0		// add member/n to accumulator

	// D^6/6
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	lw	t1,12(a3)		// T0 <- 1/7 << 32, 613566757
	add	a0,a0,t0		// add member/n to accumulator

	// D^7/7
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	add	a0,a0,t0		// add member/n to accumulator

	// D^8/8
	mulhu	a2,a2,a4		// m = m*d
	srli	t0,a2,3			// T0 <- m/8
	lw	t1,16(a3)		// T0 <- 1/9 << 32, 477218589
	add	a0,a0,t0		// add member/n to accumulator

	// D^9/9
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	lw	t1,20(a3)		// T0 <- 1/10 << 32, 429496730
	add	a0,a0,t0		// add member/n to accumulator

	// D^10/10
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	lw	t1,24(a3)		// T0 <- 1/11 << 32, 390451573
	add	a0,a0,t0		// add member/n to accumulator

	// D^11/11
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	lw	t1,28(a3)		// T0 <- 1/12 << 32, 357913942
	add	a0,a0,t0		// add member/n to accumulator

	// D^12/12
	mulhu	a2,a2,a4		// m = m*d
	li	t0,12
	blt	a2,t0,4f		// stop iteration
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	lw	t1,32(a3)		// T0 <- 1/13 << 32, 330382100
	add	a0,a0,t0		// add member/n to accumulator

	// D^13/13
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	lw	t1,36(a3)		// T0 <- 1/14 << 32, 306783379
	add	a0,a0,t0		// add member/n to accumulator

	// D^14/14
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	lw	t1,40(a3)		// T0 <- 1/15 << 32, 286331153
	add	a0,a0,t0		// add member/n to accumulator

	// D^15/15
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	add	a0,a0,t0		// add member/n to accumulator

	// D^16/16
	mulhu	a2,a2,a4		// m = m*d
	li	t0,16
	blt	a2,t0,4f		// stop iteration
	srli	t0,a2,4			// T0 <- m/16
	lw	t1,44(a3)		// T0 <- 1/17 << 32, 252645135
	add	a0,a0,t0		// add member/n to accumulator

	// D^17/17
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	lw	t1,48(a3)		// T0 <- 1/18 << 32, 238609295
	add	a0,a0,t0		// add member/n to accumulator

	// D^18/18
	mulhu	a2,a2,a4		// m = m*d
	li	t0,18
	blt	a2,t0,4f		// stop iteration
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	lw	t1,52(a3)		// T0 <- 1/19 << 32, 226050911
	add	a0,a0,t0		// add member/n to accumulator

	// D^19/19
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	lw	t1,56(a3)		// T0 <- 1/20 << 32, 214748365
	add	a0,a0,t0		// add member/n to accumulator

	// D^20/20
	mulhu	a2,a2,a4		// m = m*d
	mulhu	t0,a2,t1		// T0 <- m *= coeff
	add	a0,a0,t0		// add member/n to accumulator

// Here is:
//  A0 = result unsigned mantissa in 1.31 format, value 0..0.6931 (log(2)) = 0x00000001..0x58B90BFC
//  A1 = signed exponent
//  A3 = table address

	// go log2f() or log10f() path
4:	bgtz	a5,log2f_path		// go log2f() path
	bltz	a5,log10f_path		// go log10f() path

// logf() path

	// round mantissa up
	addi	a0,a0,0x40

	// convert mantissa from format 1.31 to format 8.24
	lw	t0,60(a3)		// T0 <- log(2) << 24 = 11629080 round up
	srai	a0,a0,7			// mantissa >> 7

	// negate result
	neg	a0,a0

	// multiply exponent * log(2) and convert to format 8.24
	mul	a1,a1,t0		// A1 <- exponent * log(2)
	add	a0,a0,a1		// add exponent to mantissa

// return from log2f() path
log2f_return:

	// prepare result exponent base -> A1
	li	a1,FLOAT_EXP1 - 1	// A1 <- exponent base

// return from log10f() path
log10f_return:

// Here is:
//  A0 = result signed mantissa in 8.24 or 7.25 format
//  A1 = result exponent base
//  A2 = result sign (will be prepared later)
//  A3 = table address, no longer needed - will be used for number of shifts

	// zero result
	beqz	a0,13f			// mantissa is zero

	// negative mantissa A0, sign -> A2
	li	a2,0			// A2 <- positive sign flag
	bgez	a0,3f			// result is not negative
	neg	a0,a0			// absolute value of the mantissa
	li	a2,0x80000000		// A2 <- negative sign flag

	// normalize mantissa A0, get shifts A3
3:	clz	t0,a0			// T0 <- count leading zeroes
	li	a3,8			// A3 <- 8
	sub	a3,a3,t0		// A3 <- number of remaining bits to normalize = 8 - clz(mantissa)

	// prepare result exponent -> A1
	add	a1,a1,a3		// A1 <- add number of shifts to result exponent

	// shift mantissa A0 up (A3 <= 0)
	bgtz	a3,4f			// shift up (A3 > 0)
	neg	a3,a3			// A3 <- - shifts
	sll	a0,a0,a3		// shift mantissa left
	j	5f

	// shift mantissa A0 right (A3 > 0)
4:	addi	a3,a3,-1		// number of shifts - 1
	srl	a0,a0,a3		// shift mantissa right - 1
	addi	a0,a0,1			// rounding mantissa up
	srli	a0,a0,1			// finish last 1 shift

	// carry on rounding
	srli	t0,a0,24		// check mantissa overflow
	beqz	t0,5f			// no overflow
	srli	a0,a0,1			// shift mantissa 1 bit right
	addi	a1,a1,1			// increment exponent

	// clear hidden bit '1' from the mantissa A0
5:	slli	a0,a0,9
	srli	a0,a0,9

	// compose result A0
	slli	a1,a1,23		// shift exponent to position
	or	a0,a0,a1		// add exponent to the result
	or	a0,a0,a2		// add sign to the result
	ret

	// result is 0
10:	li	a0,0			// value 0
	ret

	// result is -inf
11:	li	a0,0xff800000		// value -inf
	ret

	// result is +inf
12:	li	a0,0x7f800000		// value +inf
13:	ret

// log2f() path
log2f_path:

	// round mantissa up
	addi	a0,a0,0x2c		// = 0x40 * log(2) = 0x2C round down

	// multiply 1.31 mantissa * 1/log(2) and convert to format 8.24
	// 1/log(2) << 25 = 48408813 round up
	lw	t0,64(a3)		// T0 <- 1/log(2) << 25 = 48408813 round up
	slli	a1,a1,24		// A1 <- shift exponent to position
	mulhu	a0,a0,t0		// A0 <- mantissa * 1/log(2)

	// negate result
	neg	a0,a0

	// add exponent in format 8.24
	add	a0,a0,a1		// add exponent to mantissa
	j	log2f_return

// log10f() path
log10f_path:

	// round mantissa up
	lw	t0,68(a3)		// T0 <- 1/log(10) << 26 = 29145009 round down
	addi	a0,a0,0x49		// = 0x20 * log(10) = 0x49 round down

	// multiply 1.31 mantissa * 1/log(10) and convert to format 7.25
	// 1/log(10) << 26 = 29145009 round down
	mulhu	a0,a0,t0		// A0 <- mantissa * 1/log(10)

	// negate result
	neg	a0,a0

	// multiply exponent * log(10) and convert to format 7.25
	// log(2)/log(10) << 25 = 10100890 round down
	li	t0,10100890		// T0 <- log(2)/log(10) << 25 = 10100890 round down
	mul	a1,a1,t0		// A1 <- exponent * log(2)/log(10)
	add	a0,a0,a1		// add exponent to mantissa

	// prepare result exponent base -> A1
	li	a1,FLOAT_EXP1 - 2	// A1 <- exponent base
	j	log10f_return

float_section _logf_merc_tab

// logf table of constants
.p2align 2,,
logf_tab:
	.word	1431655766		//  0: 1/3 << 32
	.word	858993460		//  4: 1/5 << 32
	.word	715827883		//  8: 1/6 << 32
	.word	613566757		// 12: 1/7 << 32
	.word	477218589		// 16: 1/9 << 32
	.word	429496730		// 20: 1/10 << 32
	.word	390451573		// 24: 1/11 << 32
	.word	357913942		// 28: 1/12 << 32
	.word	330382100		// 32: 1/13 << 32
	.word	306783379		// 36: 1/14 << 32
	.word	286331153		// 40: 1/15 << 32
	.word	252645135		// 44: 1/17 << 32
	.word	238609295		// 48: 1/18 << 32
	.word	226050911		// 52: 1/19 << 32
	.word	214748365		// 56: 1/20 << 32
	.word	11629080		// 60: log(2) << 24 = 11629080 round up
	.word	48408813		// 64: 1/log(2) << 25 = 48408813 round up
	.word	29145009		// 68: 1/log(10) << 26 = 29145009 round down

#endif // USE_RISCV_LOGF == 2

// ----------------------------------------------------------------------------
//                         sinf(x) - Cordic
// ----------------------------------------------------------------------------
// float sinf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4, T0 (+ registers from remainderf function)

#if USE_RISCV_SINF == 3		// RISC-V sinf(), cosf(), sincosf(), tanf(), cotanf() functions: 0=libc, 1=Chebyshev, 2=Taylor (recommended), 3=Cordic

// >>> This code is an adaptation of the mufp_fsincos() function from Pico1 M0+ (c) Raspberry Pi <<<

float_section _sinf
.p2align 2,,

.global __wrap_sinf
.global sinf
__wrap_sinf:
sinf:

// ==== sincos base
_asm_sincos:

	// push return address
	add	sp,sp,-8		// space in the stack
	sw	ra,4(sp)		// save return address

	// check angle range - code requires range -128 < angle < 128
	slli	a1,a0,1			// delete sign bit
	srli	a1,a1,24		// delete mantissa, get only exponent -> A1
	li	t0,127 + 7		// T0 <- max. exponent = 2^7 = angle 128
	blt	a1,t0,2f		// exponent is ok

	// angle is too big, modulo angle range
	li	a1,0x40c90fdb		// modulo = angle 2*pi (= 6.2831853f)
	call	remainderf		// get remainder of division angle/(2*pi) -> A0

	// convert angle to fixed point 8.24
2:	li	a1,24			// A1 <- size of fractional part
	call	float2fix

	// A4 <- 2*PI
	li	a4,0x06487ED5		// A4 <- 2*PI in 8.24 format

	// limit max. angle to modulo 2*PI
1:	sub	a0,a0,a4		// angle -= 2*PI
	bgez	a0,1b			// limit positive angles

	// limit min. angle to modulo 2*PI (result angle in A0 is in range 0 .. 0x06487ED4)
1:	add	a0,a0,a4		// angle += 2*PI
	bltz	a0,1b			// result angle will be in range 0 (include) .. 2*PI (exclude)

	// convert to format 6.26
	slli	a2,a0,2			// A2 <- angle in format 6.26
	slli	a5,a4,1			// A5 <- PI in format 6.26, A4 = PI/2 in format 6.26

	// initialise CORDIC x,y with scaling
	li	a0,0x136e9db4
	li	a1,0

	// reduce range to -PI/2 .. +PI/2
1:	blt	a2,a4,2f		// angle is < PI/2
	sub	a2,a2,a5		// subtract PI from angle
	neg	a0,a0			// negate
	j	1b

	// convert to format 3.29
2:	slli	a2,a2,3			// A2 <- angle << 3
	la	a3,tab_cc		// A3 <- table of circular coefficients
	li	a4,1			// m = 1

	// process CORDIC in rotation mode
// INPUT: A0 = x
//	  A1 = y
//	  A2 = angle
//	  A3 = table pointer
//	  A4 = m
// OUTPUT: A0 = next x
//	   A1 = next y
//	   A2 = next angle
//	   A3 = next table pointer
//	   A4 = new m
//	   A5 = index
//	   A6, A7 = step
//	   T0 = m (copy of A4)
// TRASHED: T1, T2
	call	cordic_rot

	// proces result
	addi	a1,a1,9			// fiddle factor to make sin(0)==0
	li	a2,0			// exponents to zero
	li	a3,0

	// clamp and pack cosine
	call	_asm_clampx		// clamp cos
	li	a5,0			// no sticky bits
	call	_asm_packx		// pack cos

	// clamp and pack sine
	call	_asm_xchxy		// exchange operands
	call	_asm_clampx		// clamp sin
	li	a5,0			// no sticky bits
	call	_asm_packx		// pack sin

	// exit
	lw	ra,4(sp)		// restore return address
	add	sp,sp,8			// destroy space in the stack
	ret

float_section _clampx
.p2align 2,,

// ==== force a0 to lie in range [-1,1] Q29

_asm_clampx:
	li	a4,0x20000000		// 1 << 29
	bgt	a0,a4,1f	
	neg	a4,a4
	ble	a0,a4,1f
	ret

1:	mv	a0,a4
	ret

// ----------------------------------------------------------------------------
//                         cosf(x) - Cordic
// ----------------------------------------------------------------------------
// float cosf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4, T0 (+ registers from remainderf function)

float_section _cosf
.p2align 2,,

.global __wrap_cosf
.global cosf
__wrap_cosf:
cosf:
	// push return address
	add	sp,sp,-8		// space in the stack
	sw	ra,4(sp)		// save return address

	call	_asm_sincos
	mv	a0,a1			// A0 <- cos

	// exit
	lw	ra,4(sp)		// restore return address
	add	sp,sp,8			// destroy space in the stack
	ret

// ----------------------------------------------------------------------------
//                         sincosf(x) - Cordic
// ----------------------------------------------------------------------------
// void sincosf(float x, float* psin, float* pcos);
// INPUT: A0 = argument x
//	  A1 = pointer to psin
//	  A2 = pointer to pcos
// OUTPUT: -
// TRASHED: A0, A1, A2, A3, A4, T0 (+ registers from remainderf function)

float_section _sincosf
.p2align 2,,

.global __wrap_sincosf
.global sincosf
__wrap_sincosf:
sincosf:
	// push return address
	add	sp,sp,-16		// space in the stack
	sw	ra,4(sp)		// save return address
	sw	a1,8(sp)		// save pointer to psin
	sw	a2,12(sp)		// save pointer to pcos

	call	_asm_sincos

	// store result and exit
	lw	a2,12(sp)		// get pointer to pcos
	sw	a1,(a2)			// store cos
	lw	a2,8(sp)		// get pointer to psic
	sw	a0,(a2)			// store sin

	lw	ra,4(sp)		// restore return address
	add	sp,sp,16		// destroy space in the stack
	ret

// ----------------------------------------------------------------------------
//                         tanf(x) - Cordic
// ----------------------------------------------------------------------------
// float tanf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4, T0 (+ registers from remainderf function)

float_section _tanf
.p2align 2,,

.global __wrap_tanf
.global tanf
__wrap_tanf:
tanf:
	// push return address
	add	sp,sp,-8		// space in the stack
	sw	ra,4(sp)		// save return address

	call	_asm_sincos

	// divide sin/cos
	lw	ra,4(sp)		// restore return address
	add	sp,sp,8			// destroy space in the stack
	j	fdiv

// ----------------------------------------------------------------------------
//                         cotanf(x) - Cordic
// ----------------------------------------------------------------------------
// float tanf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4, T0 (+ registers from remainderf function)
// , err avg=2.527, err max=10

float_section _cotanf
.p2align 2,,

.global __wrap_cotanf
.global cotanf
__wrap_cotanf:
cotanf:
	// push return address
	add	sp,sp,-8		// space in the stack
	sw	ra,4(sp)		// save return address

	call	_asm_sincos
	mv	a2,a0
	mv	a0,a1
	mv	a1,a2

	// divide cos/sin
	lw	ra,4(sp)		// restore return address
	add	sp,sp,8			// destroy space in the stack
	j	fdiv

#endif // USE_RISCV_SINF == 3

// ----------------------------------------------------------------------------
//                 sinf(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// float sinf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

#if USE_RISCV_SINF == 1		// RISC-V sinf(), cosf(), sincosf(), tanf(), cotanf() functions: 0=libc, 1=Chebyshev, 2=Taylor (recommended), 3=Cordic

// local stack:
#define SINF_STACK_SIZE	32	// stack size
// SP+0: original return address
// SP+4: internal return address
// SP+8: saved S2 (used as sin sign)
// SP+12: saved S3 (used as cos sign)
// SP+16: saved S4 (used as table pointer)
// SP+20: saved sin() result
// SP+24: saved pointer to psin
// SP+28: saved pointer to pcos

float_section _sinf
.p2align 2,,

.global __wrap_sinf
.global sinf
__wrap_sinf:
sinf:
	// small angle or zero
	slli	a1,a0,1			// A1 <- delete sign bit
	srli	a1,a1,24		// A1 <- get unsigned exponent
	li	t0,FLOAT_EXP1-7		// minimal exponent
	blt	a1,t0,9f		// small angle - return x

	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..pi/2 and convert to fixed point format 3.29, mantissa range 0..0x3243F6A8
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 3.29, range = 0 .. 0x3243F6A8 = 843314856 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_norm

	// convert range 0..pi/2 in unsigned fixed point format 3.29 to range -1..+1 in signed fixed point format 2.30
	//  k = (0x80000000 / 0x3243F6A8 << 32) >> 2 = (2147483648 / 843314856 << 32) >> 2 = 2734261104
	li	t0,2734261104+1
	mulhu	a0,a0,t0
	slli	a0,a0,2			// max. value 2147483648 = 0x80000000
	li	t0,0x40000000
	sub	a0,a0,t0		// subtract 1 to get range -1..+1 in format 2.30, range -0x40000000..+0x40000000

	// Chebyshev approximation
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sinf_cheb

sinf_return:
	// pop registers
	lw	s4,16(sp)		// restore S4
	lw	s3,12(sp)		// restore S3
	lw	s2,8(sp)		// restore S2
	lw	ra,0(sp)		// restore original return address
	add	sp,sp,SINF_STACK_SIZE	// destroy space in the stack
9:	ret

// ----------------------------------------------------------------------------
//                 cosf(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// float cosf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _cosf
.p2align 2,,

.global __wrap_cosf
.global cosf
__wrap_cosf:
cosf:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..pi/2 and convert to fixed point format 3.29, mantissa range 0..0x3243F6A8
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 3.29, range = 0 .. 0x3243F6A8 = 843314856 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_norm

	// convert range 0..pi/2 in unsigned fixed point format 3.29 to range -1..+1 in signed fixed point format 2.30
	//  k = (0x80000000 / 0x3243F6A8 << 32) >> 2 = (2147483648 / 843314856 << 32) >> 2 = 2734261104
	li	t0,2734261104+1
	mulhu	a0,a0,t0
	slli	a0,a0,2			// max. value 2147483648 = 0x80000000
	li	t0,0x40000000
	sub	a0,t0,a0		// subtract 1 to get range -1..+1 in format 2.30, range -0x40000000..+0x40000000, negate

	// Chebyshev approximation
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sinf_cheb
	j	sinf_return		// return

// ----------------------------------------------------------------------------
//                 sincosf(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// void sincosf(float x, float* psin, float* pcos);
// INPUT: A0 = argument x
// 	  A1 = pointer to sin() result
// 	  A2 = pointer to cos() result
// OUTPUT: -
// TRASHED: A0, A1, A2, A3, A4 (+ registers from remainderf function)

float_section _sincosf
.p2align 2,,

.global __wrap_sincosf
.global sincosf
__wrap_sincosf:
sincosf:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address
	sw	a1,24(sp)		// save pointer to psin
	sw	a2,28(sp)		// save pointer to pcos

	// normalize angle to range 0..pi/2 and convert to fixed point format 3.29, mantissa range 0..0x3243F6A8
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 3.29, range = 0 .. 0x3243F6A8 = 843314856 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_norm

	// convert range 0..pi/2 in unsigned fixed point format 3.29 to range -1..+1 in signed fixed point format 2.30
	//  k = (0x80000000 / 0x3243F6A8 << 32) >> 2 = (2147483648 / 843314856 << 32) >> 2 = 2734261104
	li	t0,2734261104+1
	mulhu	a0,a0,t0
	slli	a0,a0,2			// max. value 2147483648 = 0x80000000
	li	t0,0x40000000
	sub	a0,a0,t0		// subtract 1 to get range -1..+1 in format 2.30, range -0x40000000..+0x40000000
	sw	a0,20(sp)		// save normalized angle

	// Chebyshev approximation - sin()
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sinf_cheb
	lw	a1,24(sp)		// A1 <- load pointer to psin
	sw	a0,(a1)			// save sin result

	// Chebyshev approximation - cos()
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	lw	a0,20(sp)		// A0 <- load normalized angle
	neg	a0,a0			// negate angle
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sinf_cheb
	lw	a1,28(sp)		// A1 <- load pointer to pcos
	sw	a0,(a1)			// save cos result
	j	sinf_return		// return

// ----------------------------------------------------------------------------
//                 tanf(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// float cosf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _tanf
.p2align 2,,

.global __wrap_tanf
.global tanf
__wrap_tanf:
tanf:
	// small angle
	slli	a1,a0,1			// A1 <- delete sign bit
	srli	a1,a1,24		// A1 <- get unsigned exponent
	li	t0,FLOAT_EXP1-8		// minimal exponent
	blt	a1,t0,9f		// small angle - return x

	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..pi/2 and convert to fixed point format 3.29, mantissa range 0..0x3243F6A8
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 3.29, range = 0 .. 0x3243F6A8 = 843314856 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_norm

	// convert range 0..pi/2 in unsigned fixed point format 3.29 to range -1..+1 in signed fixed point format 2.30
	//  k = (0x80000000 / 0x3243F6A8 << 32) >> 2 = (2147483648 / 843314856 << 32) >> 2 = 2734261104
	li	t0,2734261104+1
	mulhu	a0,a0,t0
	slli	a0,a0,2			// max. value 2147483648 = 0x80000000
	li	t0,0x40000000
	sub	a0,a0,t0		// subtract 1 to get range -1..+1 in format 2.30, range -0x40000000..+0x40000000
	sw	a0,20(sp)		// save normalized angle

	// Chebyshev approximation - sin()
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sinf_cheb
	lw	a1,20(sp)		// A1 <- load normalized angle
	sw	a0,20(sp)		// save sin result
	neg	a0,a1			// negate angle

	// Chebyshev approximation - cos()
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sinf_cheb

	// divide sin/cos
	mv	a1,a0			// A1 <- cos
	lw	a0,20(sp)		// A0 <- sin
	call	fdiv			// divide sin/cos
	j	sinf_return		// return

9:	ret

// ----------------------------------------------------------------------------
//                 cotanf(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// float cotanf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _cotanf
.p2align 2,,

.global __wrap_cotanf
.global cotanf
__wrap_cotanf:
cotanf:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..pi/2 and convert to fixed point format 3.29, mantissa range 0..0x3243F6A8
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 3.29, range = 0 .. 0x3243F6A8 = 843314856 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_norm

	// convert range 0..pi/2 in unsigned fixed point format 3.29 to range -1..+1 in signed fixed point format 2.30
	//  k = (0x80000000 / 0x3243F6A8 << 32) >> 2 = (2147483648 / 843314856 << 32) >> 2 = 2734261104
	li	t0,2734261104+1
	mulhu	a0,a0,t0
	slli	a0,a0,2			// max. value 2147483648 = 0x80000000
	li	t0,0x40000000
	sub	a0,a0,t0		// subtract 1 to get range -1..+1 in format 2.30, range -0x40000000..+0x40000000
	sw	a0,20(sp)		// save normalized angle

	// Chebyshev approximation - sin()
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sinf_cheb
	lw	a1,20(sp)		// A1 <- load normalized angle
	sw	a0,20(sp)		// save sin result
	neg	a0,a1			// negate angle

	// Chebyshev approximation - cos()
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sinf_cheb

	// divide sin/cos
	lw	a1,20(sp)		// A1 <- sin
	call	fdiv			// divide cos/sin
	j	sinf_return		// return

// ----------------------------------------------------------------------------
//                 sinf_deg(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// float sinf_deg(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _sinf_deg
.p2align 2,,

.global sinf_deg
sinf_deg:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..90 deg and convert to fixed point format 7.25, mantissa range 0..0xB4000000 = 3019898880
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 7.25, range = 0 .. 0xB4000000 = 3019898880 (= 0..90)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_deg_norm

	// convert range 0..90 in unsigned fixed point format 7.25 to range -1..+1 in signed fixed point format 2.30
	//  k = (0x80000000 / 0xB4000000 << 32) = (2147483648 / 3019898880 << 32) = 3054198967 round up
	li	t0,3054198967
	mulhu	a0,a0,t0       		// max. value 2147483648 = 0x80000000
	li	t0,0x40000000
	sub	a0,a0,t0		// subtract 1 to get range -1..+1 in format 2.30, range -0x40000000..+0x40000000

	// Chebyshev approximation
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sinf_cheb

	j	sinf_return

// ----------------------------------------------------------------------------
//                 cosf_deg(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// float cosf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _cosf_deg
.p2align 2,,

.global cosf_deg
cosf_deg:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..90 deg and convert to fixed point format 7.25, mantissa range 0..0xB4000000 = 3019898880
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 7.25, range = 0 .. 0xB4000000 = 3019898880 (= 0..90)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_deg_norm

	// convert range 0..90 in unsigned fixed point format 7.25 to range -1..+1 in signed fixed point format 2.30
	//  k = (0x80000000 / 0xB4000000 << 32) = (2147483648 / 3019898880 << 32) = 3054198967 round up
	li	t0,3054198967
	mulhu	a0,a0,t0        	// max. value 2147483648 = 0x80000000
	li	t0,0x40000000
	sub	a0,t0,a0		// subtract 1 to get range -1..+1 in format 2.30, range -0x40000000..+0x40000000, negate

	// Chebyshev approximation
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sinf_cheb
	j	sinf_return		// return

// ----------------------------------------------------------------------------
//                 sincosf_deg(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// void sincosf_deg(float x, float* psin, float* pcos);
// INPUT: A0 = argument x
// 	  A1 = pointer to sin() result
// 	  A2 = pointer to cos() result
// OUTPUT: -
// TRASHED: A0, A1, A2, A3, A4 (+ registers from remainderf function)

float_section _sincosf_deg
.p2align 2,,

.global sincosf_deg
sincosf_deg:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address
	sw	a1,24(sp)		// save pointer to psin
	sw	a2,28(sp)		// save pointer to pcos

	// normalize angle to range 0..90 deg and convert to fixed point format 7.25, mantissa range 0..0xB4000000 = 3019898880
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 7.25, range = 0 .. 0xB4000000 = 3019898880 (= 0..90)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_deg_norm

	// convert range 0..90 in unsigned fixed point format 7.25 to range -1..+1 in signed fixed point format 2.30
	//  k = (0x80000000 / 0xB4000000 << 32) = (2147483648 / 3019898880 << 32) = 3054198967 round up
	li	t0,3054198967
	mulhu	a0,a0,t0      		// max. value 2147483648 = 0x80000000
	li	t0,0x40000000
	sub	a0,a0,t0		// subtract 1 to get range -1..+1 in format 2.30, range -0x40000000..+0x40000000
	sw	a0,20(sp)		// save normalized angle

	// Chebyshev approximation - sin()
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sinf_cheb
	lw	a1,24(sp)		// A1 <- load pointer to psin
	sw	a0,(a1)			// save sin result

	// Chebyshev approximation - cos()
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	lw	a0,20(sp)		// A0 <- load normalized angle
	neg	a0,a0			// negate angle
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sinf_cheb
	lw	a1,28(sp)		// A1 <- load pointer to pcos
	sw	a0,(a1)			// save cos result
	j	sinf_return		// return

// ----------------------------------------------------------------------------
//                 tanf_deg(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// float tanf_deg(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _tanf_deg
.p2align 2,,

.global tanf_deg
tanf_deg:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..90 deg and convert to fixed point format 7.25, mantissa range 0..0xB4000000 = 3019898880
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 7.25, range = 0 .. 0xB4000000 = 3019898880 (= 0..90)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_deg_norm

	// convert range 0..90 in unsigned fixed point format 7.25 to range -1..+1 in signed fixed point format 2.30
	//  k = (0x80000000 / 0xB4000000 << 32) = (2147483648 / 3019898880 << 32) = 3054198967 round up
	li	t0,3054198967
	mulhu	a0,a0,t0      		// max. value 2147483648 = 0x80000000
	li	t0,0x40000000
	sub	a0,a0,t0		// subtract 1 to get range -1..+1 in format 2.30, range -0x40000000..+0x40000000
	sw	a0,20(sp)		// save normalized angle

	// Chebyshev approximation - sin()
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sinf_cheb
	lw	a1,20(sp)		// A1 <- load normalized angle
	sw	a0,20(sp)		// save sin result
	neg	a0,a1			// negate angle

	// Chebyshev approximation - cos()
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sinf_cheb

	// divide sin/cos
	mv	a1,a0			// A1 <- cos
	lw	a0,20(sp)		// A0 <- sin
	call	fdiv			// divide sin/cos
	j	sinf_return		// return

// ----------------------------------------------------------------------------
//                 cotanf_deg(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// float cotanf_deg(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _cotanf_deg
.p2align 2,,

.global cotanf_deg
cotanf_deg:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..90 deg and convert to fixed point format 7.25, mantissa range 0..0xB4000000 = 3019898880
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 7.25, range = 0 .. 0xB4000000 = 3019898880 (= 0..90)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_deg_norm

	// convert range 0..90 in unsigned fixed point format 7.25 to range -1..+1 in signed fixed point format 2.30
	//  k = (0x80000000 / 0xB4000000 << 32) = (2147483648 / 3019898880 << 32) = 3054198967 round up
	li	t0,3054198967
	mulhu	a0,a0,t0      		// max. value 2147483648 = 0x80000000
	li	t0,0x40000000
	sub	a0,a0,t0		// subtract 1 to get range -1..+1 in format 2.30, range -0x40000000..+0x40000000
	sw	a0,20(sp)		// save normalized angle

	// Chebyshev approximation - sin()
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sinf_cheb
	lw	a1,20(sp)		// A1 <- load normalized angle
	sw	a0,20(sp)		// save sin result
	neg	a0,a1			// negate angle

	// Chebyshev approximation - cos()
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sinf_cheb

	// divide sin/cos
	lw	a1,20(sp)		// A1 <- sin
	call	fdiv			// divide cos/sin
	j	sinf_return		// return

// ==== normalize angle to range 0..pi/2 and convert to fixed point format 3.29, mantissa range 0..0x3243F6A8
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 3.29, range = 0 .. 0x3243F6A8 = 843314856 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)

float_section _asm_sinf_norm
.p2align 2,,

_asm_sinf_norm:
	// push registers
	sw	ra,4(sp)		// save internal return address
	sw	s2,8(sp)		// save S2
	sw	s3,12(sp)		// save S3
	sw	s4,16(sp)		// save S4

	// get unsigned exponent -> A1
	slli	a1,a0,1			// destroy sign bit
	srli	a1,a1,24		// shift exponent down

	// normalize big angle >= 128.0
	li	t0,FLOAT_EXP1+7		// angle limit 2^7 = minimum 128.0 (= 20*(2*pi))
	bltu	a1,t0,1f		// not big angle

	// normalize using remainderf, modulo 2*pi (remainderf returns angle in range -pi .. +pi)
	li	a1,0x40C90FDB		// A1 <- 2*pi
	call	remainderf		// normalize angle to range -pi..+pi

	// get exponent with sign -> T0
1:	srli	t0,a0,23		// get exponent with sign -> T0

	// unpack mantissa to format 9.23 -> A1
	slli	a1,a0,9			// destroy exponent with sign
	srli	a1,a1,9			// return mantissa to base position
	bset	a1,a1,23		// set hidden bit '1'

	// negate mantissa
	bgez	a0,1f			// mantissa is not negative
	zext.b	t0,t0			// mask exponent (clear sign bit)
	neg	a1,a1			// negate mantissa

	// prepare number of shifts to format 8.24 (rounding down) -> T0
1:	addi	t0,t0,- FLOAT_EXP1 + (24-23) // exps <= +6, number of shifts <= +7

	// convert to signed fixed point 8.24 - right shift by -T0, mantissa -> A0
	bgtz	t0,2f			// number of shifts > 0
	neg	t0,t0			// negate number of shifts
	li	a0,31			// max. number of shifts
	blt	t0,a0,1f		// number of shifts is OK < 31
	mv	t0,a0			// limit number of shifts to 31
1:	sra	a0,a1,t0		// shift mantissa right -> A0
	j	3f

	// convert to signed fixed point 8.24 - left shift by T0, mantissa -> A0
2:	sll	a0,a1,t0		// shift mantissa left -> A0

	// 2*pi in 8.24 fixed point -> A1
3:	li	a1,0x06487ED5		// A1 <- 2*pi << 24

	// limit max. angle to modulo 2*pi
	// - mantissa will be in range 0 .. 0x06487ED4 = 105414356
1:	sub	a0,a0,a1		// subtract 2*pi
	bgez	a0,1b			// limit positive number
1:	add	a0,a0,a1		// add 2*pi
	bltz	a0,1b			// limit negative number, result angle will be in range 0 (include) .. 2*PI (exclude)

	// convert angle to unsigned format 3.29
	// - mantissa will be in range 0 .. 0xC90FDA80 = 3373259392 = 2*pi
	slli	a0,a0,5			// shift mantissa << 5

	// prepare pointer to table of constants -> S4
	la	s4,sinf_tab		// S4 <- table of constants

	// clear signs
	li	s2,0			// sin() sign = 0
	li	s3,0			// cos() sign = 0

	// convert to range 0..pi
	// - mantissa will be in range = 0 .. 0x6487ED4F (= 0..pi)
	slli	a1,a1,4			// A1 <- pi in 3.29 format, value 0x6487ED50 = 1686629712
	bltu	a0,a1,1f		// x < pi
	sub	a0,a0,a1		// x = x - pi
	not	s2,s2			// invert sin sign
	not	s3,s3			// invert cos sign

	// convert to range 0..pi/2
	//  mantissa will be in range = 0 .. 0x3243F6A8 = 843314856 (= 0..pi/2)
1:	srli	t0,a1,1			// T0 <- pi/2 = 0x3243F6A8 = 843314856
	bltu	a0,t0,1f		// x < pi/2
	sub	a0,a1,a0		// A0 <- pi - x
	not	s3,s3			// invert cos sign

	// pop registers
1:	lw	ra,4(sp)		// restore internal return address
	ret

// ==== normalize angle to range 0..90 deg and convert to fixed point format 7.25, mantissa range 0..0xB4000000 = 3019898880
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 7.25, range = 0 .. 0xB4000000 = 3019898880 (= 0..90)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)

float_section _asm_sinf_deg_norm
.p2align 2,,

_asm_sinf_deg_norm:
	// push registers
	sw	ra,4(sp)		// save internal return address
	sw	s2,8(sp)		// save S2
	sw	s3,12(sp)		// save S3
	sw	s4,16(sp)		// save S4

	// normalize big angle >= 360
	slli	a1,a0,1			// destroy sign bit
	li	t0,0x87680000		// float angle 360 << 1
	bltu	a1,t0,1f		// not big angle

	// normalize using remainderf, modulo 2*pi (remainderf returns angle in range -180 .. +180)
	li	a1,0x43B40000		// A1 <- 360
	call	remainderf		// normalize angle to range -180..+180

	// absolute value of the angle
	// - angle will be in range 0..360
1:	li	s2,0			// sin() sign = 0
	bgez	a0,1f			// angle is not negative
	not	s2,s2			// invert sin() sign
	bclri	a0,a0,31		// clear sign bit

	// get unsigned exponent -> T0
	// - signed exponent here is max. +8
1:	srli	t0,a0,23		// get unsigned exponent -> T0

	// unpack mantissa to unsigned format 9.23, set hidden bit '1' -> A0
	slli	a0,a0,9			// destroy exponent
	srli	a0,a0,9			// return mantissa to base position
	bset	a0,a0,23		// set hidden bit '1'

	// prepare number of shifts to format 9.23 (rounding down) -> T0
	addi	t0,t0,- FLOAT_EXP1 + (23-23) // exps <= +8, number of shifts <= +8

	// convert mantissa to unsigned fixed point format 9.23 (rounding down)
	// - mantissa will be in range = 0 .. 0xB4000000
	// right shift
	bgtz	t0,2f			// number of shifts > 0
	neg	t0,t0			// negate number of shifts
	li	a1,31			// max. number of shifts
	blt	t0,a1,1f		// number of shifts is OK < 31
	mv	t0,a1			// limit number of shifts to 31
1:	srl	a0,a0,t0		// shift mantissa right -> A0
	j	3f

	// convert to signed fixed point 8.24 - left shift by T0, mantissa -> A0
2:	sll	a0,a0,t0		// shift mantissa left -> A0

	// 180 in 9.23 fixed point -> A1
3:	li	a1,0x5A000000		// A1 <- 180 << 23

	// prepare pointer to table of constants -> S4
	la	s4,sinf_tab		// S4 <- table of constants

	// convert to range 0..180
	// - mantissa will be in range = 0 .. 0x5A000000 (= 0..180)
	li	s3,0			// cos() sign = 0
	bltu	a0,a1,1f		// x < 180
	sub	a0,a0,a1		// x = x - 180
	not	s2,s2			// invert sin sign
	not	s3,s3			// invert cos sign

	// convert to range 0..90
	//  mantissa will be in range = 0 .. 0x2D000000 = 754974720 (= 0..90)
1:	srli	t0,a1,1			// T0 <- 90deg = 0x2D000000 = 754974720
	bltu	a0,t0,1f		// x < 90
	sub	a0,a1,a0		// A0 <- 180 - x
	not	s3,s3			// invert cos sign

	// convert to format 7.25
	// mantissa = 0 .. 0xB4000000 = 3019898880
1:	slli	a0,a0,2			// mantissa << 2

	// pop registers
	lw	ra,4(sp)		// restore internal return address
	ret

// ==== Chebyshev approximation
// INPUT: A0 = mantissa in signed fixed point format 2.30, range = -1..+1
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0

float_section _asm_sinf_cheb
.p2align 2,,

_asm_sinf_cheb:

	// coeff[10]
	li	a2,-19			// A2 <- add coeff[10] = -19 = -1.74017541973437e-008 << 30
	mulh	a2,a2,a0		// A2 <- accumulator * mantissa
	slli	a2,a2,2			// A2 << 2

	// coeff[9]
	addi	a2,a2,238		// A2 <- add coeff[ 9] = 238 = 2.21560369717544e-007 << 30
	mulh	a2,a2,a0		// A2 <- accumulator * mantissa
	lw	t0,8(s4)		// T0 <- coeff[ 8] = 2726 = 2.53912110714793e-006 << 30
	slli	a2,a2,2			// A2 << 2

	// coeff[8]
	add	a2,a2,t0		// A2 <- add coeff
	mulh	a2,a2,a0		// A2 <- accumulator * mantissa
	lw	t0,12(s4)		// T0 <- coeff[ 7] = -27770  = -2.58632701317651e-005 << 30
	slli	a2,a2,2			// A2 << 2

	// coeff[7]
	add	a2,a2,t0		// A2 <- add coeff
	mulh	a2,a2,a0		// A2 <- accumulator * mantissa
	lw	t0,16(s4)		// T0 <- coeff[ 6] = -247509 = -0.000230511073652906 << 30
	slli	a2,a2,2			// A2 << 2

	// coeff[6]
	add	a2,a2,t0		// A2 <- add coeff
	mulh	a2,a2,a0		// A2 <- accumulator * mantissa
	lw	t0,20(s4)		// T0 <- coeff[ 5] = 1890832  = 0.00176097488474625 << 30
	slli	a2,a2,2			// A2 << 2

	// coeff[5]
	add	a2,a2,t0		// A2 <- add coeff
	mulh	a2,a2,a0		// A2 <- accumulator * mantissa
	lw	t0,24(s4)		// T0 <- coeff[ 4] = 12037413 = 0.0112107143249887 << 30
	slli	a2,a2,2			// A2 << 2

	// coeff[4]
	add	a2,a2,t0		// A2 <- add coeff
	mulh	a2,a2,a0		// A2 <- accumulator * mantissa
	lw	t0,28(s4)		// T0 <- coeff[ 3] = -61306040 = -0.0570956992160063 << 30
	slli	a2,a2,2			// A2 << 2

	// coeff[3]
	add	a2,a2,t0		// A2 <- add coeff
	mulh	a2,a2,a0		// A2 <- accumulator * mantissa
	lw	t0,32(s4)		// T0 <- coeff[ 2] = -234171824 = -0.218089506262913 << 30
	slli	a2,a2,2			// A2 << 2

	// coeff[2]
	add	a2,a2,t0		// A2 <- add coeff
	mulh	a2,a2,a0		// A2 <- accumulator * mantissa
	lw	t0,36(s4)		// T0 <- coeff[ 1] = 596313654 = 0.555360367288813 << 30
	slli	a2,a2,2			// A2 << 2

	// coeff[1]
	add	a2,a2,t0		// A2 <- add coeff
	mulh	a2,a2,a0		// A2 <- accumulator * mantissa
	lw	t0,40(s4)		// T0 <- coeff[ 0] = 759250125 = 0.707106781192124 << 30
	slli	a2,a2,2			// A2 << 2

	// coeff[0]
	add	a2,a2,t0		// A2 <- add coeff + rounding correction

// result mantissa is in 2.30 format, value -1..+1

	// convert mantissa from format 2.30 to format 8.24 -> A0
	srai	a0,a2,6			// A0 <- mantissa >> 6

	// zero result (including small underflow below 0)
	blez	a0,9f			// result is zero

	// limit to max. value 1.0
	li	t0,0x01000000		// value 1.0 in 8.24 format
	ble	a0,t0,1f		// value is OK
	mv	a0,t0			// limit to 1.0

	// get number of shifts -> T0
1:	clz	a2,a0			// count leading zeroes -> A2
	li	t0,8
	sub	t0,t0,a2		// T0 <- number of shifts

	// prepare result exponent -> A2
	add	a2,t0,FLOAT_EXP1-1	// exp = d + FLOAT_EXP1 - 1

	// normalize mantissa
	bgtz	t0,1f			// skip if number of shits > 0 (not = 0!)

	// shifts are negative or zero, shift mantissa up
	neg	t0,t0			// T0 <- shifts positive
	sll	a0,a0,t0		// shift result left
	j	2f

	// shifts are positive, shift mantissa down with rounding
1:	add	t0,t0,-1		// T0 <- number of shifts - 1
	srl	a0,a0,t0		// shift result right - 1
	addi	a0,a0,1			// round result up
	srli	a0,a0,1			// last 1 shift result right

	// carry on rounding
	srli	t0,a0,24		// get carry
	beqz	t0,2f			// no carry
	srli	a0,a0,1			// shift result right
	addi	a2,a2,1			// increase exponent	

	// clear hidden bit '1'
2:	bclri	a0,a0,23

	// compose result
	slli	a1,a1,31		// shift sign to position
	or	a0,a0,a1		// add sign to result
	slli	a2,a2,23		// shift exponent to position
	or	a0,a0,a2		// add exponent to result
	ret

	// zero result
9:	li	a0,0
	ret

float_section _sinf_cheb_tab
.p2align 2,,

// sinf table of constants
sinf_tab:
	.word	0x40C90FDB		//  0: 2*pi as float (abs. exponent = +2)
	.word	0x6487ED51		//  4: 8*(2*pi) in fixed point format 7.25
	.word	2726			//  8: coeff[ 8] = 2726 = 2.53912110714793e-006 << 30
	.word	-27770			// 12: coeff[ 7] = -27770  = -2.58632701317651e-005 << 30
	.word	-247509			// 16: coeff[ 6] = -247509 = -0.000230511073652906 << 30
	.word	1890832			// 20: coeff[ 5] = 1890832  = 0.00176097488474625 << 30
	.word	12037413		// 24: coeff[ 4] = 12037413 = 0.0112107143249887 << 30
	.word	-61306040		// 28: coeff[ 3] = -61306040 = -0.0570956992160063 << 30
	.word	-234171824+7		// 32: coeff[ 2] = -234171824 = -0.218089506262913 << 30
	.word	596313654+5		// 36: coeff[ 1] = 596313654 = 0.555360367288813 << 30
	.word	759250125 + 0x20	// 40: coeff[ 0] = 759250125 = 0.707106781192124 << 30 + rounding correction

#endif // USE_RISCV_SINF == 1

// ----------------------------------------------------------------------------
//                         sinf(x) - Taylor serie
// ----------------------------------------------------------------------------
// float sinf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result sin(x)
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)
// Taylor serie: sin(x) = x/1! - x^3/3! + x^5/5! - x^7/7! + x^9/9! - ...

#if USE_RISCV_SINF == 2		// RISC-V sinf(), cosf(), sincosf(), tanf(), cotanf() functions: 0=libc, 1=Chebyshev, 2=Taylor (recommended), 3=Cordic

// local stack:
#define SINF_STACK_SIZE	32	// stack size
// SP+0: original return address
// SP+4: internal return address
// SP+8: saved S2 (used as sin sign)
// SP+12: saved S3 (used as cos sign)
// SP+16: saved S4 (used as table pointer)
// SP+20: saved sin() result
// SP+24: saved pointer to psin
// SP+28: saved pointer to pcos

float_section _sinf
.p2align 2,,

.global __wrap_sinf
.global sinf
__wrap_sinf:
sinf:
	// small angle or zero
	slli	a1,a0,1			// A1 <- delete sign bit
	srli	a1,a1,24		// A1 <- get unsigned exponent
	li	t0,FLOAT_EXP1-7		// minimal exponent
	blt	a1,t0,9f		// small angle - return x

	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..pi/2 and convert to fixed point format 2.30, mantissa range 0..0x6487ED50
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 2.30, range = 0 .. 0x6487ED50 = 1686629712 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_norm

	// Taylor serie
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sin_taylor

sinf_return:
	// pop registers
	lw	s4,16(sp)		// restore S4
	lw	s3,12(sp)		// restore S3
	lw	s2,8(sp)		// restore S2
	lw	ra,0(sp)		// restore original return address
	add	sp,sp,SINF_STACK_SIZE	// destroy space in the stack
9:	ret

// ----------------------------------------------------------------------------
//                      cosf(x) - Taylor serie
// ----------------------------------------------------------------------------
// float cosf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _cosf
.p2align 2,,

.global __wrap_cosf
.global cosf
__wrap_cosf:
cosf:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..pi/2 and convert to fixed point format 2.30, mantissa range 0..0x6487ED50
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 2.30, range = 0 .. 0x6487ED50 = 1686629712 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_norm

	li	a1,1686629712		// A1 <- pi/2 in 2.30 format
	sub	a0,a1,a0		// x = pi - x

	// Taylor serie
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sin_taylor
	j	sinf_return		// return

// ----------------------------------------------------------------------------
//                     sincosf(x) - Taylor serie
// ----------------------------------------------------------------------------
// void sincosf(float x, float* psin, float* pcos);
// INPUT: A0 = argument x
// 	  A1 = pointer to sin() result
// 	  A2 = pointer to cos() result
// OUTPUT: -
// TRASHED: A0, A1, A2, A3, A4 (+ registers from remainderf function)

float_section _sincosf
.p2align 2,,

.global __wrap_sincosf
.global sincosf
__wrap_sincosf:
sincosf:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address
	sw	a1,24(sp)		// save pointer to psin
	sw	a2,28(sp)		// save pointer to pcos

	// normalize angle to range 0..pi/2 and convert to fixed point format 2.30, mantissa range 0..0x6487ED50
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 2.30, range = 0 .. 0x6487ED50 = 1686629712 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_norm
	sw	a0,20(sp)		// save normalized angle

	// Taylor serie - sin()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sin_taylor
	lw	a1,24(sp)		// A1 <- load pointer to psin
	sw	a0,(a1)			// save sin result

	// Taylor serie - cos()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	lw	a0,20(sp)		// A0 <- load normalized angle
	li	a1,1686629712		// A1 <- pi/2 in 2.30 format
	sub	a0,a1,a0		// x = pi - x
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sin_taylor
	lw	a1,28(sp)		// A1 <- load pointer to pcos
	sw	a0,(a1)			// save cos result
	j	sinf_return		// return

// ----------------------------------------------------------------------------
//                        tanf(x) - Taylor serie
// ----------------------------------------------------------------------------
// float cosf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _tanf
.p2align 2,,

.global __wrap_tanf
.global tanf
__wrap_tanf:
tanf:
	// small angle
	slli	a1,a0,1			// A1 <- delete sign bit
	srli	a1,a1,24		// A1 <- get unsigned exponent
	li	t0,FLOAT_EXP1-8		// minimal exponent
	blt	a1,t0,9f		// small angle - return x

	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..pi/2 and convert to fixed point format 2.30, mantissa range 0..0x6487ED50
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 2.30, range = 0 .. 0x6487ED50 = 1686629712 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_norm
	sw	a0,20(sp)		// save normalized angle

	// Taylor serie - sin()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sin_taylor
	lw	a1,20(sp)		// A1 <- load normalized angle
	sw	a0,20(sp)		// save sin result
	li	a0,1686629712		// A0 <- pi/2 in 2.30 format
	sub	a0,a0,a1		// x = pi - x

	// Taylor serie - cos()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sin_taylor

	// divide sin/cos
	mv	a1,a0			// A1 <- cos
	lw	a0,20(sp)		// A0 <- sin
	call	fdiv			// divide sin/cos
	j	sinf_return		// return

9:	ret

// ----------------------------------------------------------------------------
//                      cotanf(x) - Taylor serie
// ----------------------------------------------------------------------------
// float cotanf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _cotanf
.p2align 2,,

.global __wrap_cotanf
.global cotanf
__wrap_cotanf:
cotanf:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..pi/2 and convert to fixed point format 2.30, mantissa range 0..0x6487ED50
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 2.30, range = 0 .. 0x6487ED50 = 1686629712 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_norm
	sw	a0,20(sp)		// save normalized angle

	// Taylor serie - sin()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sin_taylor
	lw	a1,20(sp)		// A1 <- load normalized angle
	sw	a0,20(sp)		// save sin result
	li	a0,1686629712		// A0 <- pi/2 in 2.30 format
	sub	a0,a0,a1		// x = pi - x

	// Taylor serie - cos()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sin_taylor

	// divide sin/cos
	lw	a1,20(sp)		// A1 <- sin
	call	fdiv			// divide cos/sin
	j	sinf_return		// return

// ----------------------------------------------------------------------------
//                      sinf_deg(x) - Taylor serie
// ----------------------------------------------------------------------------
// float sinf_deg(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _sinf_deg
.p2align 2,,

.global sinf_deg
sinf_deg:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..90 and convert to fixed point format 2.30, mantissa range 0..0x6487ED50
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 2.30, range = 0 .. 0x6487ED50 = 1686629712 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_deg_norm

	// Taylor serie - sin()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sin_taylor

	j	sinf_return

// ----------------------------------------------------------------------------
//                        cosf_deg(x) - Taylor serie
// ----------------------------------------------------------------------------
// float cosf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _cosf_deg
.p2align 2,,

.global cosf_deg
cosf_deg:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..90 and convert to fixed point format 2.30, mantissa range 0..0x6487ED50
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 2.30, range = 0 .. 0x6487ED50 = 1686629712 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_deg_norm

	li	a1,1686629712		// A1 <- pi/2 in 2.30 format
	sub	a0,a1,a0		// x = pi - x

	// Taylor serie - cos()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sin_taylor
	j	sinf_return		// return

// ----------------------------------------------------------------------------
//                    sincosf_deg(x) - Taylor serie
// ----------------------------------------------------------------------------
// void sincosf(float x, float* psin, float* pcos);
// INPUT: A0 = argument x
// 	  A1 = pointer to sin() result
// 	  A2 = pointer to cos() result
// OUTPUT: -
// TRASHED: A0, A1, A2, A3, A4 (+ registers from remainderf function)

float_section _sincosf_deg
.p2align 2,,

.global sincosf_deg
sincosf_deg:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address
	sw	a1,24(sp)		// save pointer to psin
	sw	a2,28(sp)		// save pointer to pcos

	// normalize angle to range 0..90 and convert to fixed point format 2.30, mantissa range 0..0x6487ED50
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 2.30, range = 0 .. 0x6487ED50 = 1686629712 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_deg_norm
	sw	a0,20(sp)		// save normalized angle

	// Taylor serie - sin()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sin_taylor
	lw	a1,24(sp)		// A1 <- load pointer to psin
	sw	a0,(a1)			// save sin result

	// Taylor serie - cos()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	lw	a0,20(sp)		// A0 <- load normalized angle
	li	a1,1686629712		// A1 <- pi/2 in 2.30 format
	sub	a0,a1,a0		// x = pi - x
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sin_taylor
	lw	a1,28(sp)		// A1 <- load pointer to pcos
	sw	a0,(a1)			// save cos result
	j	sinf_return		// return

// ----------------------------------------------------------------------------
//                    tanf_deg(x) - Taylor serie
// ----------------------------------------------------------------------------
// float cosf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _tanf_deg
.p2align 2,,

.global tanf_deg
tanf_deg:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..90 and convert to fixed point format 2.30, mantissa range 0..0x6487ED50
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 2.30, range = 0 .. 0x6487ED50 = 1686629712 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_deg_norm
	sw	a0,20(sp)		// save normalized angle

	// Taylor serie - sin()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sin_taylor
	lw	a1,20(sp)		// A1 <- load normalized angle
	sw	a0,20(sp)		// save sin result
	li	a0,1686629712		// A0 <- pi/2 in 2.30 format
	sub	a0,a0,a1		// x = pi - x

	// Taylor serie - cos()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sin_taylor

	// divide sin/cos
	mv	a1,a0			// A1 <- cos
	lw	a0,20(sp)		// A0 <- sin
	call	fdiv			// divide sin/cos
	j	sinf_return		// return

// ----------------------------------------------------------------------------
//                     cotanf_deg(x) - Taylor serie
// ----------------------------------------------------------------------------
// float cotanf_deg(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)

float_section _cotanf_deg
.p2align 2,,

.global cotanf_deg
cotanf_deg:
	// push registers
	addi	sp,sp,-SINF_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..90 and convert to fixed point format 2.30, mantissa range 0..0x6487ED50
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 2.30, range = 0 .. 0x6487ED50 = 1686629712 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)
	call	_asm_sinf_deg_norm
	sw	a0,20(sp)		// save normalized angle

	// Taylor serie - sin()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s2			// A1 <- sin sign
	call	_asm_sin_taylor
	lw	a1,20(sp)		// A1 <- load normalized angle
	sw	a0,20(sp)		// save sin result
	li	a0,1686629712		// A0 <- pi/2 in 2.30 format
	sub	a0,a0,a1		// x = pi - x

	// Taylor serie - cos()
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, T0
	mv	a1,s3			// A1 <- cos sign
	call	_asm_sin_taylor

	// divide sin/cos
	lw	a1,20(sp)		// A1 <- sin
	call	fdiv			// divide cos/sin
	j	sinf_return		// return

// ==== normalize angle to range 0..pi/2 and convert to fixed point format 2.30, mantissa range 0..0x6487ED50
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 2.30, range = 0 .. 0x6487ED50 = 1686629712 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)

float_section _asm_sinf_norm
.p2align 2,,

_asm_sinf_norm:
	// push registers
	sw	ra,4(sp)		// save internal return address
	sw	s2,8(sp)		// save S2
	sw	s3,12(sp)		// save S3
	sw	s4,16(sp)		// save S4

	// get unsigned exponent -> A1
	slli	a1,a0,1			// destroy sign bit
	srli	a1,a1,24		// shift exponent down

	// normalize big angle >= 128.0
	li	t0,FLOAT_EXP1+7		// angle limit 2^7 = minimum 128.0 (= 20*(2*pi))
	bltu	a1,t0,1f		// not big angle

	// normalize using remainderf, modulo 2*pi (remainderf returns angle in range -pi .. +pi)
	li	a1,0x40C90FDB		// A1 <- 2*pi
	call	remainderf		// normalize angle to range -pi..+pi

	// get exponent with sign -> T0
1:	srli	t0,a0,23		// get exponent with sign -> T0

	// unpack mantissa to format 9.23 -> A1
	slli	a1,a0,9			// destroy exponent with sign
	srli	a1,a1,9			// return mantissa to base position
	bset	a1,a1,23		// set hidden bit '1'

	// negate mantissa
	bgez	a0,1f			// mantissa is not negative
	zext.b	t0,t0			// mask exponent (clear sign bit)
	neg	a1,a1			// negate mantissa

	// prepare number of shifts to format 8.24 (rounding down) -> T0
1:	addi	t0,t0,- FLOAT_EXP1 + (24-23) // exps <= +6, number of shifts <= +7

	// convert to signed fixed point 8.24 - right shift by -T0, mantissa -> A0
	bgtz	t0,2f			// number of shifts > 0
	neg	t0,t0			// negate number of shifts
	li	a0,31			// max. number of shifts
	blt	t0,a0,1f		// number of shifts is OK < 31
	mv	t0,a0			// limit number of shifts to 31
1:	sra	a0,a1,t0		// shift mantissa right -> A0
	j	3f

	// convert to signed fixed point 8.24 - left shift by T0, mantissa -> A0
2:	sll	a0,a1,t0		// shift mantissa left -> A0

	// 2*pi in 8.24 fixed point -> A1
3:	li	a1,0x06487ED5		// A1 <- 2*pi << 24

	// limit max. angle to modulo 2*pi
	// - mantissa will be in range 0 .. 0x06487ED4 = 105414356
1:	sub	a0,a0,a1		// subtract 2*pi
	bgez	a0,1b			// limit positive number
1:	add	a0,a0,a1		// add 2*pi
	bltz	a0,1b			// limit negative number, result angle will be in range 0 (include) .. 2*PI (exclude)

	// convert angle to unsigned format 3.29
	// - mantissa will be in range 0 .. 0xC90FDA80 = 3373259392 = 2*pi
	slli	a0,a0,5			// shift mantissa << 5

	// prepare pointer to table of constants -> S4
	la	s4,sinf_tab		// S4 <- table of constants

	// clear signs
	li	s2,0			// sin() sign = 0
	li	s3,0			// cos() sign = 0

	// convert to range 0..pi
	// - mantissa will be in range = 0 .. 0x6487ED4F (= 0..pi)
	slli	a1,a1,4			// A1 <- pi in 3.29 format, value 0x6487ED50 = 1686629712
	bltu	a0,a1,1f		// x < pi
	sub	a0,a0,a1		// x = x - pi
	not	s2,s2			// invert sin sign
	not	s3,s3			// invert cos sign

	// convert to range 0..pi/2
	//  mantissa will be in range = 0 .. 0x3243F6A8 = 843314856 (= 0..pi/2)
1:	srli	t0,a1,1			// T0 <- pi/2 = 0x3243F6A8 = 843314856
	bltu	a0,t0,1f		// x < pi/2
	sub	a0,a1,a0		// A0 <- pi - x
	not	s3,s3			// invert cos sign

	// convert to format 2.30
	//  xn will be in range 0..0x6487ED50 = 1686629712 (= 0..pi/2), unsigned fixed point format 2.30
1:	slli	a0,a0,1			// xn <<= 1

	// pop registers
	lw	ra,4(sp)		// restore internal return address
	ret

// ==== normalize angle to range 0..90 and convert to fixed point format 2.30, mantissa range 0..0x6487ED50
// INPUT: A0 = argument x
// OUTPUT: A0 = mantissa in fixed point format 2.30, range = 0 .. 0x6487ED50 = 1686629712 (= 0..pi/2)
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
//	   S4 = pointer to table of constants
// TRASHED: A1, T0 (+ registers from remainderf function)

float_section _asm_sinf_deg_norm
.p2align 2,,

_asm_sinf_deg_norm:
	// push registers
	sw	ra,4(sp)		// save internal return address
	sw	s2,8(sp)		// save S2
	sw	s3,12(sp)		// save S3
	sw	s4,16(sp)		// save S4

	// normalize big angle >= 360
	slli	a1,a0,1			// destroy sign bit
	li	t0,0x87680000		// float angle 360 << 1
	bltu	a1,t0,1f		// not big angle

	// normalize using remainderf, modulo 2*pi (remainderf returns angle in range -180 .. +180)
	li	a1,0x43B40000		// A1 <- 360
	call	remainderf		// normalize angle to range -180..+180

	// absolute value of the angle
	// - angle will be in range 0..360
1:	li	s2,0			// sin() sign = 0
	bgez	a0,1f			// angle is not negative
	not	s2,s2			// invert sin() sign
	bclri	a0,a0,31		// clear sign bit

	// get unsigned exponent -> T0
	// - signed exponent here is max. +8
1:	srli	t0,a0,23		// get unsigned exponent -> T0

	// unpack mantissa to unsigned format 9.23, set hidden bit '1' -> A0
	slli	a0,a0,9			// destroy exponent
	srli	a0,a0,9			// return mantissa to base position
	bset	a0,a0,23		// set hidden bit '1'

	// prepare number of shifts to format 9.23 (rounding down) -> T0
	addi	t0,t0,- FLOAT_EXP1 + (23-23) // exps <= +8, number of shifts <= +8

	// convert mantissa to unsigned fixed point format 9.23 (rounding down)
	// - mantissa will be in range = 0 .. 0xB4000000
	// right shift
	bgtz	t0,2f			// number of shifts > 0
	neg	t0,t0			// negate number of shifts
	li	a1,31			// max. number of shifts
	blt	t0,a1,1f		// number of shifts is OK < 31
	mv	t0,a1			// limit number of shifts to 31
1:	srl	a0,a0,t0		// shift mantissa right -> A0
	j	3f

	// convert to signed fixed point 8.24 - left shift by T0, mantissa -> A0
2:	sll	a0,a0,t0		// shift mantissa left -> A0

	// 180 in 9.23 fixed point -> A1
3:	li	a1,0x5A000000		// A1 <- 180 << 23

	// prepare pointer to table of constants -> S4
	la	s4,sinf_tab		// S4 <- table of constants

	// convert to range 0..180
	// - mantissa will be in range = 0 .. 0x5A000000 (= 0..180)
	li	s3,0			// cos() sign = 0
	bltu	a0,a1,1f		// x < 180
	sub	a0,a0,a1		// x = x - 180
	not	s2,s2			// invert sin sign
	not	s3,s3			// invert cos sign

	// convert to range 0..90
	//  mantissa will be in range = 0 .. 0x2D000000 = 754974720 (= 0..90)
1:	srli	t0,a1,1			// T0 <- 90deg = 0x2D000000 = 754974720
	bltu	a0,t0,1f		// x < 90
	sub	a0,a1,a0		// A0 <- 180 - x
	not	s3,s3			// invert cos sign

	// convert range 0..90 in unsigned fixed point format 9.23 to range 0..pi/2 in format 2.30
	// - input range: 0..90 in 9.23 = 0..0x2D000000 = 754974720
	// - output range: 0..pi/2 in 2.30 = 0..0x6487ED51 = 1686629713
	// k = (1686629713 / 754974720 << 32) >> 2 = 2398762259 round up
1:	li	t0,2398762259
	mulhu	a0,a0,t0
	slli	a0,a0,2			// max. value = 421657428 << 2 = 1686629712

	// pop registers
	lw	ra,4(sp)		// restore internal return address
	ret

// ==== Taylor serie
//  sin(x) = x/1! - x^3/3! + x^5/5! - x^7/7! + x^9/9! - ...
// INPUT: A0 = mantissa in fixed point format 2.30, range 0..0x6487ED50 = 1686629712 (= 0..pi/2)
//	  A1 = sign 0 or -1
//	  S4 = pointer to table of constants
// OUTPUT: A0 = result
// TRASHED: A1, A2, A3, T0

float_section _asm_sin_taylor
.p2align 2,,

_asm_sin_taylor:

	// prepare x^2 -> A2
	mulhu	a2,a0,a0		// A2 <- x*x
	slli	a2,a2,2			// << 2
	
	// A0 <- x^1/1!

	// - x^3/3!
	lw	t0,0(s4)		// T0 <- 2^34/(2*3) = 2863311531 round up
	mulhu	a3,a0,a2		// x <- x * x^2
	mulhu	a3,a3,t0		// x <- x * const
	sub	a0,a0,a3		// acc -= x

	// + x^5/5!
	lw	t0,4(s4)		// T0 <- 2^34/(4*5) = 858993460 round up
	mulhu	a3,a3,a2		// x <- x * x^2
	mulhu	a3,a3,t0		// x <- x * const	
	add	a0,a0,a3

	// - x^7/7!
	lw	t0,8(s4)		// T0 <- 2^34/(6*7) = 409044505 round up
	mulhu	a3,a3,a2		// x <- x * x^2
	mulhu	a3,a3,t0		// x <- x * const	
	sub	a0,a0,a3

	// + x^9/9!
	lw	t0,12(s4)		// T0 <- 2^34/(8*9) = 238609295 round up
	mulhu	a3,a3,a2		// x <- x * x^2
	mulhu	a3,a3,t0		// x <- x * const	
	add	a0,a0,a3

	// - x^11/11!
	lw	t0,16(s4)		// T0 <- 2^34/(10*11) = 156180630 round up
	mulhu	a3,a3,a2		// x <- x * x^2
	mulhu	a3,a3,t0		// x <- x * const	
	sub	a0,a0,a3

	// + x^13/13!
	lw	t0,20(s4)		// T0 <- 2^34/(12*13) = 110127367 round up
	mulhu	a3,a3,a2		// x <- x * x^2
	mulhu	a3,a3,t0		// x <- x * const	
	add	a0,a0,a3

	// zero result (including small underflow below 0)
	blez	a0,9f			// result is zero

	// limit to max. value 1.0
	li	t0,0x40000000		// value 1.0 in 2.30 format
	bleu	a0,t0,1f		// value is OK
	mv	a0,t0			// limit to 1.0

	// normalize mantissa to max. position 1.31
1:	clz	a2,a0			// A1 <- get number of leading zeroes
	sll	a0,a0,a2		// shift mantissa to max. position

	// prepare exponent -> A2
	li	t0,FLOAT_EXP1 + 2
	sub	a2,t0,a2		// exp = FLOAT_EXP1 - d + 2

	// rounding mantissa
	addi	a0,a0,0x80-2		// "-2" is an empirical correction to increase accuracy
	bgez	a0,1f			// skip if result >= 0
	slli	a0,a0,1			// remove leading bit '1'
	addi	a2,a2,-1		// compensate exponent

	// compose result
1:	slli	a2,a2,23		// shift exponent to final position
	srli	a0,a0,9			// shift mantissa to position
	slli	a1,a1,31		// shift sign to bit 31
	or	a0,a0,a1		// add sign to result
	or	a0,a0,a2		// add exponent to result
	ret

	// zero result
9:	li	a0,0
	ret

float_section _sinf_taylor_tab
.p2align 2,,

// sinf table of constants
sinf_tab:
	.word	0xAAAAAAAB		//  0: 2^34/(2*3) = 2863311531 round up
	.word	0x33333334		//  4: 2^34/(4*5) = 858993460 round up
	.word	0x18618619		//  8: 2^34/(6*7) = 409044505 round up
	.word	0x0E38E38F		// 12: 2^34/(8*9) = 238609295 round up
	.word	0x094F2096		// 16: 2^34/(10*11) = 156180630 round up
	.word	0x06906907		// 20: 2^34/(12*13) = 110127367 round up

#endif // USE_RISCV_SINF == 2

// ----------------------------------------------------------------------------
//                           atan2f(x) - Cordic
// ----------------------------------------------------------------------------
// float atan2f(float y, float x);
// INPUT: A0 = argument y
//	  A1 = argument x
// OUTPUT: A0 = result
// TRASHED: A1-A7, T0-T3

#if (USE_RISCV_ASINF == 3) || (USE_RISCV_ATANF == 3)	// RISC-V asinf(), acosf() functions: 0=libc, 2=Taylor (recommended), 3=Cordic

// >>> This code is an adaptation of the mufp_fsincos() function from Pico1 M0+ (c) Raspberry Pi <<<

float_section _atan2f
.p2align 2,,

.global __wrap_atan2f
.global atan2f
__wrap_atan2f:
atan2f:
	mv	t3,ra		// T3 <- save return address

	// unpack argument y
// input: A0 = float number
// output: A0 = signed mantissa 9Q23 (24 significant bits)
//         A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
// trashes: A4
	call	_asm_unpackx	// unpack number y (trashes A4)
	mv	a4,a0		// A4 <- signed mantissa y
	mv	a0,a1		// A0 <- argument x
	mv	a1,a4		// A1 <- signed mantissa y
	mv	a3,a2		// A3 <- signed exponent y

	// unpack argument x
// input: A0 = float number
// output: A0 = signed mantissa 9Q23 (24 significant bits)
//         A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
// trashes: A4
	call	_asm_unpackx

// Here is:
//  A0 = mantissa x
//  A1 = mantissa y
//  A2 = exponent x
//  A3 = exponent y
//  T3 = return address

	// shift mantissas left
	slli	a0,a0,5		// shift mantissa x to bits 5..27
	slli	a1,a1,5		// shift mantissa y to bits 5..27

	// sum exponents
	add	a4,a2,a3	// A4 <- exponent x + y, -760 if both are 0
	srai	a4,a4,9		// sum / 512
	addi	a4,a4,1		// sum + 1
	bltz	a4,2f		// force y to 0 proper, so result will be zero

	// prepare shifts
	li	t0,28		// max. shift
	sub	a4,a2,a3	// A4 (dif) <- difference of exponents
	bgez	a4,1f		// ex >= ey
	neg	a4,a4		// make shift positive
	sra	a0,a0,a4	// ex < ey, so shift x >> dif
	bltu	a4,t0,3f	// dif < 28
	srai	a0,a0,31	// x >> 31
	j	3f

1:	sra	a1,a1,a4	// ex >= ey, so shift y >> dif
	bltu	a4,t0,3f	// dif < 28

	// here abs(x) >> abs(y), or both x and y are +-0
2:	bgez	a0,4f		// x positive, return signed 0
	li	a0,0x1921fb54*4+1 // x negative, return +/- pi Q29
	srai	a1,a1,31	// A1 <- get y sign
	xor	a0,a0,a1	// negate result by sign
	j	7f

4:	srai	a0,a1,31
	j	7f

3: 	li	a2,0		// initial angle
	bgez	a0,5f		// skip on x positive
	neg	a0,a0		// rotate to 1st/4th quadrants
	neg	a1,a1
	li	a2,0x1921fb54*4+1 // pi Q29

5:	la	a3,tab_cc	// A3 <- circular coefficients
	li	a4,1		// m = 1

// INPUT: A0 = x
//	  A1 = y
//	  A2 = angle
//	  A3 = table pointer
//	  A4 = m
// OUTPUT: A0 = next x
//	   A1 = next y
//	   A2 = next angle
//	   A3 = next table pointer
//	   A4 = new m
//	   A5 = index
//	   A6, A7 = step
//	   T0 = m (copy of A4)
// TRASHED: T1, T2
	call	cordic_vec	// also produces magnitude (with scaling factor 1.646760119), which is discarded

	mv	a0,a2		// result here is -pi/2..3pi/2 Q29
	// srai a2,29
	// sub a0,a0,a2
	li	a2,0x1921fb54*4+1 // pi Q29
	add	a4,a0,a2	// attempt to fix -3pi/2..-pi case
	bgez	a4,6f		// -pi/2..0? leave result as is
	sub	a4,a0,a2	// <pi? leave as is
	bltz	a4,6f
	sub	a0,a4,a2	// >pi: take off 2pi

6:	addi	a0,a0,-1	// fiddle factor so atan2(0,1)==0
7:	li	a2,0		// exponent for pack

	// pack result
	li	a5,0		// no sticky bits
// input: A0 = signed mantissa (24 significant bits)
//        A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
//        A5 = sticky bits (used for rounding)
// output: A0 = float number
// trashes: A2, A4, A5, T0
	call	_asm_packx	// pack number

	mv	ra,t3		// restore return address
	ret

#endif // USE_RISCV_ASINF == 3

// ----------------------------------------------------------------------------
//                           asinf(x)... - Taylor serie
// ----------------------------------------------------------------------------
// float asinf(float x);
// float asinf_deg(float x);
// float acosf(float x);
// float acosf_deg(float x);
// INPUT: A0 = argument X
// OUTPUT: A0 = result
// TRASHED: A1-A4, T0-T3
//  arcsin(x) = x + x^3/2/3 + 3*x^5/2/4/5 + 3*5*x^7/2/4/6/7 +

#if USE_RISCV_ASINF == 2	// RISC-V asinf(), acosf() functions: 0=libc, 2=Taylor (recommended), 3=Cordic

float_section _asinf
.p2align 2,,

// arc cosine in degrees - Taylor serie
//  616ns avg=0.118 max=9
.global acosf_deg
acosf_deg:
	li	t3,1		// T3 <- flag 0=radians, 1=degrees
	li	t2,1		// T2 <- flag 0=asin(), 1=acos()
	j	_asm_asincosf

// arc cosine in radians - Taylor serie
//  601ns avg=0.023 max=9
.global __wrap_acosf
.global acosf
__wrap_acosf:
acosf:
	li	t3,0		// T3 <- flag 0=radians, 1=degrees
	li	t2,1		// T2 <- flag 0=asin(), 1=acos()
	j	_asm_asincosf

// arc sine in degrees - Taylor serie (result is -90 .. +90)
//  576ns avg=0.193 max=3
.global asinf_deg
asinf_deg:
	li	t3,1		// T3 <- flag 0=radians, 1=degrees
	j	asinf2

// arc sine in radians - Taylor serie (result is -pi/2 .. +pi/2)
//  556ns avg=0.037 max=3
.global __wrap_asinf
.global asinf
__wrap_asinf:
asinf:
	li	t3,0		// T3 <- flag 0=radians, 1=degrees
asinf2:	li	t2,0		// T2 <- flag 0=asin(), 1=acos()

// arc sine/cosine - Taylor serie
// INPUT: A0 = argument X
//	  T2 = flag 0=asin(), 1=acos()
//	  T3 = flag 0=radians, 1=degrees
// OUTPUT: A0 = result
// TRASHED: A1-A4, T0-T3
_asm_asincosf:

	// get sign -> A1
	srai	a1,a0,31		// A1 <- sign 0=positive, -1=negative

	// absolute value of the number -> A2
	bclri	a2,a0,31		// A2 <- clear sign bit of the number

	// check argument "1"
	// acos() +1 returns 0
	// acos() -1 returns pi or 180
	// asin() +-1 returns +-pi/2 or +-90
	li	t0,0x3F800000		// T0 <- "+1" as float number
	beq	a2,t0,_asm_asincosf_1	// skip if argument is "+1"

	// get signed exponent -> A3
	srli	a3,a2,23		// A3 <- unsigned exponent
	addi	a3,a3,-FLOAT_EXP1	// subtract exponent bias

	// zero or small value
	li	t0,-10			// T0 <- minimal exponent = -10
	blt	a3,t0,_asm_asincosf_0	// skip if signed exponent is < -10

	// invalid input value > 1.0
	bgez	a3,_asm_asincosf_nan	// value > 1.0 is invalid (value = 1.0 was served above)
_asm_asincosf_0_back:

// Here is:
//  A0 = input argument x
//  A1 = sign 0=positive, -1=negative
//  A2 = absolute value of the number x (without sign flag)
//  A3 = signed exponent in range -20..-1
//  T1 = 1=use alternative method sqrt(1 - x^2)
//  T2 = flag 0=asin(), 1=acos()
//  T3 = flag 0=radians, 1=degrees

	// if > 0.707107 (= sqrt(0.5)), use alternate method sqrt(1 - x^2)
	li	t1,0			// T1 <- 0 no alternative method
	li	t0,0x3F3504F3 + 400000	// T0 <- sqrt(0.5), plus small improvement correction
	ble	a2,t0,3f		// do not use alternative method

	add	sp,sp,-16		// space in the stack
	sw	ra,0(sp)		// save return address
	sw	t2,4(sp)		// save flag asin/acos
	sw	t3,8(sp)		// save flag radians/degrees
	sw	a1,12(sp)		// save sign

	_asm_fsqr			// x^2
	binvi	a0,a0,31		// invert sign bit (negate)
	li	a1,0x3F800000		// A1 <- 1.0f float
	call	fadd			// 1 - fsqr(x)
	call	sqrtf			// sqrtf(1 - fsqr(x))

	lw	a1,12(sp)		// restore sign
	lw	t3,8(sp)		// restore flag radians/degrees
	lw	t2,4(sp)		// restore flag asin/acos
	lw	ra,0(sp)		// restore return address
	add	sp,sp,16		// destroy space in the stack

	li	t1,1			// T1 <- 1 use alternative method
	bclri	a2,a0,31		// A2 <- clear sign bit of the number
	srli	a3,a2,23		// A3 <- unsigned exponent
	addi	a3,a3,-FLOAT_EXP1	// subtract exponent bias

	// unpack mantissa to fixed point format 1.31 -> A2
3:	slli	a0,a0,8			// A0 <- mantissa
	bset	a0,a0,31		// set hidden bit '1'
	neg	a3,a3			// negate exponent
	srl	a2,a0,a3		// A2 <- mantissa in fixed point format 1.31

	// table of constants -> A4
	la	a4,asinf_tab

// Here is:
//  A0 = ... result accumulator 'acc' (setup later)
//  A1 = sign 0=positive, -1=negative
//  A2 = mantissa in fixed point format 1.31, used as serie member 'xn'
//  A3 = ... square of number x2 = x^2 (setup later)
//  A4 = table of constants
//  T1 = 1=use alternative method sqrt(1 - x^2)
//  T2 = flag 0=asin(), 1=acos()
//  T3 = flag 0=radians, 1=degrees

	// prepare square x^2 -> A3
	mulhu	a3,a2,a2		// A3 <- x^2
	slli	a3,a3,2			// correct shift

	// prepare result accumulator
	mv	a0,a2			// A0 <- x

	// (1) + x^3/(2*3)
	lw	t0,0(a4)		// T0 <- coeff[1]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (2) + 3*x^5/(2*4*5)
	lw	t0,4(a4)		// T0 <- coeff[2]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (3) + 3*5*x^7/(2*4*6*7)
	mulhu	a2,a2,a3		// xn = xn*x2
	li	t0,22			// T0 <- 4294967296/191739612
	bltu	a2,t0,4f		// end of iteration
	lw	t0,8(a4)		// T0 <- coeff[3]
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (4) + 3*5*7*x^9/(2*4*6*8*9)
	lw	t0,12(a4)		// T0 <- coeff[4]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (5) + 3*5*7*9*x^11/(2*4*6*8*10*11)
	lw	t0,16(a4)		// T0 <- coeff[5]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (6) + 3*5*7*9*11*x^13/(2*4*6*8*10*12*13)
	mulhu	a2,a2,a3		// xn = xn*x2
	li	t0,57			// T0 <- 4294967296/74529556
	bltu	a2,t0,4f		// end of iteration
	lw	t0,20(a4)		// T0 <- coeff[6]
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (7) + 3*5*7*9*11*13*x^15/(2*4*6*8*10*12*14*15)
	lw	t0,24(a4)		// T0 <- coeff[7]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator
	
	// (8) + 3*5*7*9*11*13*15*x^17/(2*4*6*8*10*12*14*16*17)
	lw	t0,28(a4)		// T0 <- coeff[8]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (9) + 3*5*7*9*11*13*15*17*x^19/(2*4*6*8*10*12*14*16*18*19)
	mulhu	a2,a2,a3		// xn = xn*x2
	li	t0,102			// T0 <- 4294967296/41925794
	bltu	a2,t0,4f		// end of iteration
	lw	t0,32(a4)		// T0 <- coeff[9]
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (10) + 3*5*7*9*11*13*15*17*19*x^21/(2*4*6*8*10*12*14*16*18*20*21)
	lw	t0,36(a4)		// T0 <- coeff[10]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (11) + 3*5*7*9*11*13*15*17*19*21*x^23/(2*4*6*8*10*12*14*16*18*20*22*23)
	lw	t0,40(a4)		// T0 <- coeff[11]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (12) + 3*5*7*9*11*13*15*17*19*21*23*x^25/(2*4*6*8*10*12*14*16*18*20*22*24*25)
	mulhu	a2,a2,a3		// xn = xn*x2
	li	t0,155			// T0 <- 4294967296/27690558
	bltu	a2,t0,4f		// end of iteration
	lw	t0,44(a4)		// T0 <- coeff[12]
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (13) + 3*5*7*9*11*13*15*17*19*21*23*25*x^27/(2*4*6*8*10*12*14*16*18*20*22*24*26*27)
	lw	t0,48(a4)		// T0 <- coeff[13]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (14) + 3*5*7*9*11*13*15*17*19*21*23*25*27*x^29/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*29)
	lw	t0,52(a4)		// T0 <- coeff[14]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (15) + 3*5*7*9*11*13*15*17*19*21*23*25*27*29*x^31/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*31)
	mulhu	a2,a2,a3		// xn = xn*x2
	li	t0,214			// T0 <- 4294967296/20015164
	bltu	a2,t0,4f		// end of iteration
	lw	t0,56(a4)		// T0 <- coeff[15]
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (16) + 3*5*7*9*11*13*15*17*19*21*23*25*27*29*31*x^33/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*32*33)
	lw	t0,60(a4)		// T0 <- coeff[16]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (17) + 3*5*7*9*11*13*15*17*19*21*23*25*27*29*31*33*x^35/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*32*34*35)
	lw	t0,64(a4)		// T0 <- coeff[17]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

	// (18) + 3*5*7*9*11*13*15*17*19*21*23*25*27*29*31*33*35*x^37/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*32*34*36*37)
	lw	t0,68(a4)		// T0 <- coeff[18]
	mulhu	a2,a2,a3		// xn = xn*x2
	mulhu	t0,a2,t0		// T0 <- xn * coeff
	add	a0,a0,t0		// add to accumulator

// Here is:
//  A0 = result accumulator 'acc'
//  A1 = sign 0=positive, -1=negative
//  A3 = ... exponent (setup later)
//  A4 = table of constants
//  T1 = 1=use alternative method sqrt(1 - x^2)
//  T2 = flag 0=asin(), 1=acos()
//  T3 = flag 0=radians, 1=degrees

// A0: acc value here is in range = 0..3373259426 (=0..0xC90FDAA2)

// ---- recalc to degrees in 7.25 format

4:	beqz	t3,6f			// skip if radians (T3 = flag 0=radians, 1=degrees)

	// (180/pi << 32) >> 6 = 3845054676 round up, max. result value
	li	t0,3845054676+30	// T0 <- 180/pi plus small correction to increase precision
	mulhu	a0,a0,t0		// recalc to degrees

	// prepare exponent base -> A3
	li	t0,3019898880-21	// T0 <- 90 in 7.25, plus small correction to increase precision
	li	a3,FLOAT_EXP1 + 1 + 6	// A3 <- prepare exponent base

	// limit angle to 90 deg
	bleu	a0,t0,1f		// skip if result <= 90 deg
	mv	a0,t0			// result <- limit to 90 deg on overflow

	// use alternative method
1:	beqz	t1,1f			// skip if not using alternative method (T1 = 1=use alternative method sqrt(1 - x^2))
	sub	a0,t0,a0		// acc = 90 - acc

	// convert asin() to acos()
1:	beqz	t2,8f			// skip if not using acos() (T2 = flag 0=asin(), 1=acos())

	// acos_deg(x) = 90 - asin_deg(x)

	// asin_deg(x) is positive
	bnez	a1,1f			// skip if input argument is negative (A1 = sign 0=positive, -1=negative)
	sub	a0,t0,a0		// acc = 90 - acc
	j	8f

	// asin_deg(x) is negative, halve precision to 8.24 format (result can be up to 180)
1:	srli	a0,a0,1			// halve result precision
	li	t0,1509949435		// T0 <- 90 in 8.24 format
	add	a0,a0,t0		// add 90 to result
	addi	a3,a3,1			// increase exponent
	li	a1,0			// clear sign flag
	j	8f

// ---- process result in radians

	// pi/2 = 3373259426 = 0xC90FDAA2
6:	li	t0,3373259426-6		// T0 <- pi/2 in 1.31, plus small correction to increase precision

	// prepare exponent base -> A3
	li	a3,FLOAT_EXP1 + 1	// A3 <- prepare exponent base

	// limit angle to pi/2
	bleu	a0,t0,1f		// skip if result <= pi/2
	mv	a0,t0			// result <- limit to pi/2 on overflow

	// use alternative method
1:	beqz	t1,1f			// skip if not using alternative method (T1 = 1=use alternative method sqrt(1 - x^2))
	sub	a0,t0,a0		// acc = pi/2 - acc

	// convert asin() to acos()
1:	beqz	t2,8f			// skip if not using acos() (T2 = flag 0=asin(), 1=acos())

	// acos(x) = pi/2 - asin(x)

	// asin(x) is positive
	bnez	a1,1f			// skip if input argument is negative (A1 = sign 0=positive, -1=negative)
	sub	a0,t0,a0		// acc = pi/2 - acc
	j	8f

	// asin(x) is negative, halve precision to 2.30 format (result can be up to pi)
1:	srli	a0,a0,1			// halve result precision
	li	t0,1686629712		// T0 <- pi/2 in 2.30 format
	add	a0,a0,t0		// add pi/2 to result
	addi	a3,a3,1			// increase exponent
	li	a1,0			// clear sign flag

// Here is:
//  A0 = result accumulator 'acc'
//  A1 = sign 0=positive, -1=negative
//  A3 = exponent base
//  A4 = table of constants

	// zero result
8:	beqz	a0,9f

	// normalize mantissa to max. position 1.31
	clz	t0,a0			// T0 <- count leading zeroes
	sll	a0,a0,t0		// shift to max. position

	// prepare exponent
	sub	a3,a3,t0

	// rounding mantissa
	li	t0,0x80
	add	a0,a0,t0
	bltu	a0,t0,1f		// skip if overflow
	slli	a0,a0,1			// remove leading bit '1'
	addi	a3,a3,-1		// compensate exponent

	// compose result
1:	srli	a0,a0,9			// shift mantissa >> 9
	slli	a3,a3,23		// shift exponent << 23
	slli	a1,a1,31		// shift sign << 31
	or	a0,a0,a3		// add exponent to result
	or	a0,a0,a1		// add sign to result
9:	ret

// ---- argument is "1"
_asm_asincosf_1:
	// - acos()
	beqz	t2,4f			// skip if asin() (T2 = 0=asin(), 1=acos())

	// - negative
	beqz	a1,3f			// skip if positive (A1 = 0=positive, -1=negative)

	// - degrees, return 180 deg
	beqz	t3,2f			// skip if radians (T3 = 0=radians, 1=degrees)
	li	a0,0x43340000		// A0 <- 180 degrees as float
	ret

	// - radians, return pi radians
2:	li	a0,0x40490FDB		// A0 <- pi as float
	ret

	// - positive, return 0
3:	li	a0,0			// A0 <- 0 (= acos(1))
	ret

	// - asin(), return +-90 or +-pi/2
4:	slli	a1,a1,31		// A1 <- sign
	li	a0,0x3FC90FDB		// A0 <- pi/2
	beqz	t3,5f			// skip if radians (T3 = 0=radians, 1=degrees)
	li	a0,0x42B40000		// A0 <- 90
5:	or	a0,a0,a1		// add sign
	ret

// ---- argument is 0 or small value
_asm_asincosf_0:
	// asin(x) returns x
	bnez	t2,2f			// skip if acos() (T2 = 0=asin(), 1=acos())

	// degrees - recalc x to degrees
	beqz	t3,1f			// return x if radians (T3 = 0=radians, 1=degrees)
	add	sp,sp,-8		// space in the stack
	sw	ra,4(sp)		// save return address
	_asm_rad2degf			// convert radians to degrees
	lw	ra,4(sp)		// restore return address
	add	sp,sp,8			// destroy space in the stack
1:	ret

	// acos(0) returns pi/2 or 90
2:	li	t0,-20			// T0 <- minimal exponent = -20
	bge	a3,t0,_asm_asincosf_0_back // continue back with exponent = -20..-11

	// - acos(0), return +90 or +pi/2
	li	a0,0x3FC90FDB		// A0 <- pi/2
	beqz	t3,3f			// skip if radians (T3 = 0=radians, 1=degrees)
	li	a0,0x42B40000		// A0 <- 90
3:	ret

// ---- invalid input value
_asm_asincosf_nan:
	li	a0,0x7fc00000		// value +1.#QNAN
	ret

float_section _asinf_tab
.p2align 2,,

// asinf table of constants
asinf_tab:
	.word	715827883+99		//  0: (1) 1/(2*3) << 32 = 715827883 round up (plus small correction to increase precision)
	.word	322122548		//  4: (2) 3/(2*4*5) << 32 = 322122548 round up
	.word	191739612		//  8: (3) (3*5)/(2*4*6*7) << 32 = 191739612 round up
	.word	130489458		// 12: (4) (3*5*7)/(2*4*6*8*9) << 32 = 130489458 round up
	.word	96087692		// 16: (5) (3*5*7*9)/(2*4*6*8*10*11) << 32 = 96087692 round up
	.word	74529556		// 20: (6) (3*5*7*9*11)/(2*4*6*8*10*12*13) << 32 = 74529556 round up
	.word	59978548		// 24: (7) (3*5*7*9*11*13)/(2*4*6*8*10*12*14*15) << 32 = 59978548 round up
	.word	49614608		// 28: (8) (3*5*7*9*11*13*15)/(2*4*6*8*10*12*14*16*17) << 32 = 49614608 round up
	.word	41925794		// 32: (9) (3*5*7*9*11*13*15*17)/(2*4*6*8*10*12*14*16*18*19) << 32 = 41925794 round up
	.word	36036218		// 36: (10) (3*5*7*9*11*13*15*17*19)/(2*4*6*8*10*12*14*16*18*20*21) << 32 = 36036218 round up
	.word	31407060		// 40: (11) (3*5*7*9*11*13*15*17*19*21)/(2*4*6*8*10*12*14*16*18*20*22*23) << 32 = 31407060 round up
	.word	27690558		// 44: (12) (3*5*7*9*11*13*15*17*19*21*23)/(2*4*6*8*10*12*14*16*18*20*22*24*25) << 32 = 27690558 round up
	.word	24653275		// 48: (13) (3*5*7*9*11*13*15*17*19*21*23*25)/(2*4*6*8*10*12*14*16*18*20*22*24*26*27) << 32 = 24653275 round up
	.word	22133297		// 52: (14) (3*5*7*9*11*13*15*17*19*21*23*25*27)/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*29) << 32 = 22133297 round up
	.word	20015164		// 56: (15) (3*5*7*9*11*13*15*17*19*21*23*25*27*29)/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*31) << 32 = 20015164 round up
	.word	18214558		// 60: (16) (3*5*7*9*11*13*15*17*19*21*23*25*27*29*31)/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*32*33) << 32 = 18214558 round up
	.word	16668616		// 64: (17) (3*5*7*9*11*13*15*17*19*21*23*25*27*29*31*33)/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*32*34*35) << 32 = 16668616 round up
	.word	15329621		// 68: (18) (3*5*7*9*11*13*15*17*19*21*23*25*27*29*31*33*35)/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*32*34*36*37) << 32 = 15329621 round up

#endif // USE_RISCV_ASINF == 2

// ----------------------------------------------------------------------------
//                      atanf(x)... - Chebyshev approximation
// ----------------------------------------------------------------------------
// float atanf(float x);
// float atanf_deg(float x);
// float acotanf(float x);
// float acotanf_deg(float x);
// INPUT: A0 = argument X
// OUTPUT: A0 = result
// TRASHED: A1-A4, T0-T3

#if USE_RISCV_ATANF == 1	// RISC-V atanf(), acotanf(), atan2f() functions: 0=libc, 1=Chebyshev (recommended), 3=Cordic

float_section _atanf
.p2align 2,,

// arc cotangent in degrees (result is 0 .. 180)
//  1070ns avg=0.172 max=4
.global acotanf_deg
acotanf_deg:
	li	t3,1		// T3 <- flag 0=radians, 1=degrees
	li	t2,1		// T2 <- flag 0=atan(), 1=acotan()
	j	_asm_atanf

// arc cotangent in radians (result is 0 .. pi)
//  1030ns avg=0.156 max=3
.global __wrap_acotanf
.global acotanf
__wrap_acotanf:
acotanf:
	li	t3,0		// T3 <- flag 0=radians, 1=degrees
	li	t2,1		// T2 <- flag 0=atan(), 1=acotan()
	j	_asm_atanf

// arc tangent in degrees (result is -90 .. +90)
//  1030ns avg=0.304 max=4
.global atanf_deg
atanf_deg:
	li	t3,1		// T3 <- flag 0=radians, 1=degrees
	j	_atanf2

// arc tangent in radians (result is -pi/2 .. +pi/2)
//  1010ns avg=0.225 max=3
.global __wrap_atanf
.global atanf
__wrap_atanf:
atanf:
	li	t3,0		// T3 <- flag 0=radians, 1=degrees
_atanf2:li	t2,0		// T2 <- flag 0=atan(), 1=acotan()

// arc tangent - Chebyshev approximation
// INPUT: A0 = argument X
//	  T2 = flag 0=atan(), 1=acotan()
//	  T3 = flag 0=radians, 1=degrees
// OUTPUT: A0 = result
// TRASHED: A1-A4, T0-T3
_asm_atanf:

	// get sign -> A1
	srai	a1,a0,31		// A1 <- sign 0=positive, -1=negative

	// absolute value of the number -> A2
	bclri	a2,a0,31		// A2 <- clear sign bit of the number

	// get unsigned exponent -> A3
	srli	a3,a2,23		// A3 <- unsigned exponent

	// zero or small angle
	li	t0,FLOAT_EXP1-9		// T0 <- minimal exponent
	blt	a3,t0,_asm_atanf_0	// go to small value

	// convert to signed exponent
	addi	a3,a3,-FLOAT_EXP1	// subtract exponent bias

	// infinity or big value
	li	t0,9			// T0 <- maximal exponent
	bgt	a3,t0,_asm_atanf_inf	// go to big value

	// result addition = 0 -> T1
	li	t1,0			// T1 <- 0, no result addition

// Here is:
//  A0 = input argument x
//  A1 = sign 0=positive, -1=negative
//  A2 = absolute value of the number x (clear sign bit)
//  A3 = signed exponent
//  T1 = result addition (on alternative method)
//  T2 = flag 0=atan(), 1=acotan()
//  T3 = flag 0=radians, 1=degrees

// ---- if abs(x) > 1, use alternative method with acotan()

	bltz	a3,4f			// skip if exp < 0, number is < 1.0

	// use -1/x
	add	sp,sp,-16		// space in the stack
	sw	ra,0(sp)		// save return address
	sw	t2,4(sp)		// save T2 (flag 0=atan(), 1=acotan())
	sw	t3,8(sp)		// save T3 (flag 0=radians, 1=degrees)
	_asm_frec			// calculate 1/x
	binvi	a0,a0,31		// A0 <- result negate
	lw	t3,8(sp)		// restore T3 (flag 0=radians, 1=degrees)
	lw	t2,4(sp)		// restore T2 (flag 0=atan(), 1=acotan())
	lw	ra,0(sp)		// restore return address
	add	sp,sp,16		// destroy space in the stack

	// get sign -> A1
	srai	a1,a0,31		// A1 <- sign 0=positive, -1=negative

	// absolute value of the number -> A2
	bclri	a2,a0,31		// A2 <- clear sign bit of the number

	// get signed exponent -> A3
	srli	a3,a2,23		// A3 <- unsigned exponent
	addi	a3,a3,-FLOAT_EXP1	// subtract exponent bias

	// result addition = pi/2 -> T1
	li	t1,1686629713-4		// T1 <- pi/2 in format 2.30, result addition

	// is x is positive, use -pi/2
	bnez	a1,4f			// skip if x is negative (A1 = sign 0=positive, -1=negative)
	neg	t1,t1			// negate result addition

// argument x (= A0) is in range 0..+1 here

// Here is:
//  A0 = ... accumulator (used later below)
//  A1 = sign 0=positive, -1=negative
//  A2 = absolute value of the number x (clear sign bit)
//  A3 = signed exponent in range -20..0 (... or x^2 later below)
//  A4 = ... pointer to table of constants (setup later below)
//  T1 = result addition (on alternative method)
//  T2 = flag 0=atan(), 1=acotan()
//  T3 = flag 0=radians, 1=degrees

	// normalize mantissa - shift to max position 1.31 and restore hidden bit '1'
4:	slli	a2,a2,8			// A2 <- shift mantissa left << 8
	bseti	a2,a2,31		// set hidden bit '1'

	// shift mantissa right by exponent bits, to get signed fixed number in format 2.30
	neg	a3,a3			// negate exponent (exponent was in range -20..0)
	addi	a3,a3,1			// increase exponent (exponent is now in range 1..21)
	srl	a2,a2,a3		// shift mantissa right -> A2 (save x for later)

	// calculate 2*x^2 - 1, signed fixed point format 2.30 -> A3 (= m)
	//   y = x*fnc(2*x^2 - 1)
	mulhu	a3,a2,a2		// A3 <- xn*xn >> 32
	slli	a3,a3,3			// convert to format 2.30 and multiply * 2
	li	t0,0x40000000		// T0 <- 1 in format 2.30
	sub	a3,a3,t0		// A3 <- mantissa will be in range -1..+1 in format 2.30

// argument m (A3) is in range -1..+1 here, signed fixed point format 2.30

	// table of constants -> A4
	la	a4,atanf_tab

	// coeff[13] = -35 = -3.2544185651906e-008 << 30
	li	a0,-35-2		// setup accumulator with coeff[13]
	mulh	a0,a0,a3		// acc *= m
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[12] = 113 = 1.04951570961687e-007 << 30
	addi	a0,a0,113+1		// add coeff[12]
	mulh	a0,a0,a3		// acc *= m
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[11] = -243 = -2.26499863253959e-007 << 30
	addi	a0,a0,-243-2		// add coeff[11]
	mulh	a0,a0,a3		// acc *= m
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[10] = 799 = 7.44219783577194e-007 << 30
	addi	a0,a0,799+20		// add coeff[10]
	mulh	a0,a0,a3		// acc *= m
	lw	t0,0(a4)		// T0 <- coeff[9]
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[9] = -2818 = -2.624138732088e-006 << 30
	add	a0,a0,t0		// add coeff[9]
	mulh	a0,a0,a3		// acc *= m
	lw	t0,4(a4)		// T0 <- coeff[8]
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[8] = 9424 = 8.77708897562002e-006 << 30
	add	a0,a0,t0		// add coeff[8]
	mulh	a0,a0,a3		// acc *= m
	lw	t0,8(a4)		// T0 <- coeff[7]
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[7] = -31833 = -2.9646454208887e-005 << 30
	add	a0,a0,t0		// add coeff[7]
	mulh	a0,a0,a3		// acc *= m
	lw	t0,12(a4)		// T0 <- coeff[6]
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[6] = 109806 = 0.000102265103919308 << 30
	add	a0,a0,t0		// add coeff[6]
	mulh	a0,a0,a3		// acc *= m
	lw	t0,16(a4)		// T0 <- coeff[5]
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[5] = -387602 = -0.000360982690949641 << 30
	add	a0,a0,t0		// add coeff[5]
	mulh	a0,a0,a3		// acc *= m
	lw	t0,20(a4)		// T0 <- coeff[4]
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[4] = 1412602 = 0.00131558787870745 << 30
	add	a0,a0,t0		// add coeff[4]
	mulh	a0,a0,a3		// acc *= m
	lw	t0,24(a4)		// T0 <- coeff[3]
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[3] = -5401849 = -0.00503086362186878 << 30
	add	a0,a0,t0		// add coeff[3]
	mulh	a0,a0,a3		// acc *= m
	lw	t0,28(a4)		// T0 <- coeff[2]
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[2] = 22389505 = 0.0208518510844442 << 30
	add	a0,a0,t0		// add coeff[2]
	mulh	a0,a0,a3		// acc *= m
	lw	t0,32(a4)		// T0 <- coeff[1]
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[1] = -109389104 = -0,10187654235051 << 30
	add	a0,a0,t0		// add coeff[1]
	mulh	a0,a0,a3		// acc *= m
	lw	t0,36(a4)		// T0 <- coeff[0]
	slli	a0,a0,2			// compensate '>>32' of mulh

	// coeff[0] = 934606091 = 0,870419751387089 << 30
	add	a0,a0,t0		// add coeff[0]

	// result will be -pi/2..+pi/2 in 2.30 format
	mulh	a0,a0,a2		// multiply result with original x
	slli	a0,a0,2			// compensate '>>32' of mulh
	beqz	a1,1f			// skip if result is positive (A1 = sign 0=positive, -1=negative)
	neg	a0,a0			// negate result

	// add alternative correction
1:	add	a0,a0,t1		// add result addition

	// prepare exponent base -> A3
	li	a3,FLOAT_EXP1 + 2

	// acotan() correction: acotan(x) = pi/2 - atan(x)
	beqz	t2,3f			// skip if atan() (T2 = flag 0=atan(), 1=acotan())

	// if result is positive, use "acc = pi/2 - acc" in 2.30 format
	li	t0,1686629713-3		// T0 <- pi/2 in format 2.30
	bgez	a0,1f			// skip if result is positive

	// if result is negative, use "acc = pi/2 - acc" in 3.29 format (result would overflow otherwise)
	li	t0,843314857		// T0 <- pi/2 in format 3.29
	addi	a3,a3,1			// compensate exponent
	srai	a0,a0,1			// A0 <- halve result
1:	sub	a0,t0,a0		// A0 <- pi/2 - acc

	// zero result
3:	beqz	a0,9f			// zero result

	// absolute value
	srai	a1,a0,31		// A1 <- sign 0=positive, -1=negative
	bgez	a0,1f			// skip if result is positive
	neg	a0,a0			// negate result

	// convert to degrees: x = x * (180/PI)
1:	beqz	t3,2f			// skip if radians (T3 = flag 0=radians, 1=degrees)

	// x = x * (180/PI) ... (180/PI << 32) >> 6 = 3845054675
	li	t0,3845054675+15	// T0 <- multiply with correction for atan()
	beqz	t2,1f			// skip if atan() (T2 = flag 0=atan(), 1=acotan())
	addi	t0,t0,(3845054675+3) - (3845054675+15) // T0 <- multiply with correction for acotan()
1:	mulhu	a0,a0,t0		// convert to degrees
	addi	a3,a3,6			// compensate exponent

	// normalize mantissa to max. position 1.31
2:	clz	t0,a0			// T0 <- count leading zeroes
	sll	a0,a0,t0		// A0 <- shift to max. position

	// prepare exponent -> A3
	sub	a3,a3,t0

	// rounding mantissa
	li	t0,0x80
	add	a0,a0,t0
	bltu	a0,t0,1f		// skip if overflow
	slli	a0,a0,1			// remove leading bit '1'
	addi	a3,a3,-1		// compensate exponent

	// compose result
1:	srli	a0,a0,9			// shift mantissa >> 9
	slli	a3,a3,23		// shift exponent << 23
	slli	a1,a1,31		// shift sign << 31
	or	a0,a0,a3		// add exponent to result
	or	a0,a0,a1		// add sign to result
9:	ret

// ---- zero or small angle
// Here is:
//  A0 = input argument x
//  A1 = sign 0=positive, -1=negative
//  A2 = absolute value of the number x (clear sign bit)
//  A3 = unsigned exponent
//  T2 = flag 0=atan(), 1=acotan()
//  T3 = flag 0=radians, 1=degrees

_asm_atanf_0:

	// push return address
	add	sp,sp,-16		// space in the stack
	sw	ra,0(sp)		// save return address

	// convert to degrees
	beqz	a3,1f			// skip if argument is zero (unsigned exponent = 0)		
	beqz	t3,1f			// skip if radians (T3 = flag 0=radians, 1=degrees)
	sw	t2,4(sp)		// save T2 (flag 0=atan(), 1=acotan())
	sw	t3,8(sp)		// save T3 (flag 0=radians, 1=degrees)
	sw	a3,12(sp)		// save A3 (unsigned exponent)
	_asm_rad2degf			// convert radians to degrees
	lw	a3,12(sp)		// restore A3 (unsigned exponent)
	lw	t3,8(sp)		// restore T3 (flag 0=radians, 1=degrees)
	lw	t2,4(sp)		// restore T2 (flag 0=atan(), 1=acotan())

	// atan() - return x
1:	beqz	t2,9f			// skip if atan() (T2 = flag 0=atan(), 1=acotan())

	binvi	a1,a0,31		// A1 <- x negate
	li	a0,0x42B40000		// A0 <- +90 deg
	bnez	t3,2f			// skip if degrees (T3 = flag 0=radians, 1=degrees)
	li	a0,0x3FC90FDB		// A0 <- +pi/2 radians

2:	beqz	a3,9f			// skip if argument is zero (unsigned exponent = 0)		
	call	fadd			// subtract x from constant

	// exit
9:	lw	ra,0(sp)		// restore return address
	add	sp,sp,16		// destroy space in the stack
	ret

// ---- infinity or big value
// acotan(x) = pi/2 - atan(x)

// Here is:
//  A0 = input argument x
//  A1 = sign 0=positive, -1=negative
//  A2 = absolute value of the number x (clear sign bit)
//  A3 = signed exponent
//  T2 = flag 0=atan(), 1=acotan()
//  T3 = flag 0=radians, 1=degrees

_asm_atanf_inf:

	// push return address
	add	sp,sp,-8		// space in the stack
	sw	ra,4(sp)		// save return address

	// acotan()
	beqz	t2,5f			// skip if atan() (T2 = flag 0=atan(), 1=acotan())

	// acotan() positive value
	bnez	a1,3f			// skip if negative (A1 = sign 0=positive, -1=negative)

	// positive infinity - return x
	beqz	t3,1f			// skip if radians (T3 = flag 0=radians, 1=degrees)
	_asm_frec			// calculate 1/x
	_asm_rad2degf			// convert radians to degrees
	j	9f
1:	_asm_frec			// calculate 1/x
	j	9f

	// acotan() negative value
3:	li	t0,0x43340000		// T0 <- 180 degrees
	bnez	t3,1f			// skip if degrees (T3 = flag 0=radians, 1=degrees)
	li	t0,0x40490FDB		// T0 <- pi radians
1:	li	t1,23			// minimal exponent
	bge	a3,t1,8f		// skip if big exponent - result is only 180 or pi

	sw	t0,0(sp)		// save T0
	beqz	t3,1f			// skip if radians (T3 = flag 0=radians, 1=degrees)

	_asm_frec			// calculate 1/x
	_asm_rad2degf			// convert radians to degrees
	j	2f

1:	_asm_frec			// calculate 1/x
2:	lw	a1,0(sp)		// get constant
	call	fadd			// add constant
	j	9f

	// atan()
5:	li	t0,0x42B40000		// T0 <- 90 degrees
	bnez	t3,1f			// skip if degrees (T3 = flag 0=radians, 1=degrees)
	li	t0,0x3FC90FDB		// T0 <- pi/2 radians
1:	slli	a1,a1,31		// shift sign to bit 31
	or	t0,t0,a1		// add sign

	li	t1,23			// minimal exponent
	bge	a3,t1,8f		// skip if big exponent - result is only 90 or pi/2

	sw	t0,0(sp)		// save T0
	beqz	t3,1f			// skip if radians (T3 = flag 0=radians, 1=degrees)

	_asm_frec			// calculate 1/x
	_asm_rad2degf			// convert radians to degrees
	j	2f

1:	_asm_frec			// calculate 1/x
2:	binvi	a1,a0,31		// A1 <- result negate
	lw	a0,0(sp)		// get constant
	call	fadd			// add constant
	j	9f

	// result is in T0
8:	mv	a0,t0			// A0 <- result

	// exit
9:	lw	ra,4(sp)		// restore return address
	add	sp,sp,8			// destroy space in the stack
	ret


float_section _atanf_tab
.p2align 2,,

// atanf table of constants
atanf_tab:
	.word	-2818+1			//  0: coeff[9] = -2818 = -2.624138732088e-006 << 30
	.word	9424+3			//  4: coeff[8] = 9424 = 8.77708897562002e-006 << 30
	.word	-31833+2		//  8: coeff[7] = -31833 = -2.9646454208887e-005 << 30
	.word	109806+3		// 12: coeff[6] = 109806 = 0.000102265103919308 << 30
	.word	-387602+3		// 16: coeff[5] = -387602 = -0.000360982690949641 << 30
	.word	1412602			// 20: coeff[4] = 1412602 = 0.00131558787870745 << 30
	.word	-5401849		// 24: coeff[3] = -5401849 = -0.00503086362186878 << 30
	.word	22389505+1		// 28: coeff[2] = 22389505 = 0.0208518510844442 << 30
	.word	-109389104-1		// 32: coeff[1] = -109389104 = -0,10187654235051 << 30
	.word	934606091		// 36: coeff[0] = 934606091 = 0,870419751387089 << 30

#endif // USE_RISCV_ATANF == 1

#endif // !USE_FLOATLIBC
#endif // USE_FLOAT
