
// ****************************************************************************
//
//                    Double RP2350 - RISC-V science functions
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040

// This code contains fast RISC-V functions:
//  sqrt (711ns)
//  exp (2080ns)
//  exp2 (1810ns)
//  exp10 (2190ns)
//  log (3500ns)
//  log2 (3420ns)
//  log10 (3530ns)
//  sin (2580ns)
//  sin_deg (2780ns)
//  cos (3010ns)
//  cos_deg (2790ns)
//  sincos (5080ns)
//  sincos_deg (5050ns)
//  tan (6060ns)
//  tan_deg (6040ns)
//  cotan (6120ns)
//  cotan_deg (6150ns)
//  asin (6390ns)
//  asin_deg (6600ns)
//  acos (6800ns)
//  acos_deg (6910ns)
//  atan (4200ns)
//  atan_deg (4340ns)
//  acotan (4330ns)
//  acotan_deg (4410ns)
//  ...atan2 (5460ns)
//  ...atan2_deg (6080ns)

// Functions were tested with the FPUTEST application from the PicoLibSDK  
// https://github.com/Panda381/PicoLibSDK/tree/main/PicoPad/TEST/FPUTEST

// .p2align 2,,
// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - adds incorrect alignment
// after the end of the section. Use commas ,, to work a little better.

#include "../asm_include.inc"

#if USE_DOUBLE		// use double support 1=in RAM, 2=in Flash

#if USE_DOUBLE == 1
#define double_section section_noflash
#else
#define double_section section_text
#endif

#define DOUBLE_EXP1		0x3ff			// double - exponent of '1' (= 1023)

#if !USE_DOUBLELIBC

// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulhu	\tmp1,\aL,\bH		// tmp1 <- aL*bH
	mulhu	\tmp2,\aH,\bL		// tmp2 <- aH*bL
	mul	\aL,\aH,\bH		// aL <- aH*bH low
	mulhu	\aH,\aH,\bH		// aH <- aH*bH high

	add	\aL,\aL,\tmp1		// add aL*bH
	sltu	\tmp1,\aL,\tmp1		// carry
	add	\aH,\aH,\tmp1		// add carry

	add	\aL,\aL,\tmp2		// add aH*bL
	sltu	\tmp2,\aL,\tmp2		// carry
	add	\aH,\aH,\tmp2		// add carry
.endm

// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// resL . result LOW (can be equal to aL)
// resH . result HIGH (can be equal to aH)
// aL ... 1st operand LOW
// aH ... 1st operand HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
.macro mulu64highto resL, resH, aL, aH, bL, bH, tmp1, tmp2
	mulhu	\tmp1,\aL,\bH		// tmp1 <- aL*bH
	mulhu	\tmp2,\aH,\bL		// tmp2 <- aH*bL
	mul	\resL,\aH,\bH		// resL <- aH*bH low
	mulhu	\resH,\aH,\bH		// resH <- aH*bH high

	add	\resL,\resL,\tmp1	// add aL*bH
	sltu	\tmp1,\resL,\tmp1	// carry
	add	\resH,\resH,\tmp1	// add carry

	add	\resL,\resL,\tmp2	// add aH*bL
	sltu	\tmp2,\resL,\tmp2	// carry
	add	\resH,\resH,\tmp2	// add carry
.endm

// square (u64*u64->u128)>>64 simplified (no carry from LOW)
// resL . result LOW (can be equal to aL)
// resH . result HIGH (can be equal to aH)
// aL ... operand LOW
// aH ... operand HIGH
// tmp1, tmp2 ... temporary, trashed
.macro sqru64high resL, resH, aL, aH, tmp1, tmp2
	mulhu	\tmp1,\aL,\aH		// tmp1 <- aL*aH
	mul	\resL,\aH,\aH		// resL <- aH*aH low
	mulhu	\resH,\aH,\aH		// resH <- aH*aH high

	add	\resL,\resL,\tmp1	// add aL*aH
	sltu	\tmp2,\resL,\tmp1	// carry
	add	\resH,\resH,\tmp2	// add carry

	add	\resL,\resL,\tmp1	// add aL*aH
	sltu	\tmp2,\resL,\tmp1	// carry
	add	\resH,\resH,\tmp2	// add carry
.endm

// negate s64
// aL ... operand LOW
// aH ... operand HIGH
// tmp ... temporary
.macro negs64 aL, aH, tmp
	snez	\tmp,\aL
	neg	\aH,\aH
	neg	\aL,\aL
	sub	\aH,\aH,\tmp
.endm

// multiply s64 * s32 -> s64
// aL ... 1st operand s64 LOW
// aH ... 1st operand s64 HIGH
// bL ... 2nd operand s32
// resL ... result s64 LOW (can be same as aL, aH or bL)
// resH ... result s64 HIGH (can be same as aL, aH, bL, tmp1 or tmp2)
// tmp1, tmp2 ... temporary, trashed
.macro muls64s32 aL, aH, bL, resL, resH, tmp1, tmp2
	srai	\tmp1,\bL,31			// tmp1 <- extend bL to bH
	mul	\tmp2,\bL,\aH			// tmp2 <- bL*aH
	mul	\tmp1,\tmp1,\aL			// tmp1 <- bH*aL
	add	\tmp1,\tmp1,\tmp2		// tmp1 <- bH*aL + bL*aH
	mulhu	\tmp2,\bL,\aL			// tmp2 <- (bL*aL)HIGH
	mul	\resL,\bL,\aL			// resL <- bL*aL
	add	\resH,\tmp1,\tmp2		// resH <- bH*aL + bL*aH + (bL*aL)HIGH
.endm

.macro _asm_rad2deg	// convert radians to degrees (do not call inline rad2deg directly, a slow function in Flash would be used)
	li	a3,0x404CA5DC
	li	a2,0x1A63C1F8		// A3:A2 <- 180/PI = 57.295779513082320877
	call	dmul			// convert radians to degrees
.endm

.macro _asm_drec	// reciprocal 1/x (do not call inline drec directly, a slow function in Flash would be used)
	mv	a2,a0			// A3:A2 <- argument x
	mv	a3,a1
	li	a1,0x3ff00000		// A1:A0 <- 1.0
	li	a0,0x00000000
	call	ddiv			// calculate 1/x
.endm

.macro _asm_drecneg	// reciprocal -1/x
	mv	a2,a0			// A3:A2 <- argument x
	mv	a3,a1
	li	a1,0xbff00000		// A1:A0 <- -1.0
	li	a0,0x00000000
	call	ddiv			// calculate 1/x
.endm

// ----------------------------------------------------------------------------
//                            Square root
// ----------------------------------------------------------------------------
// double sqrt(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result sqrt(x)
// TRASHED: A2..A7, T0..T6

double_section __sqrt
.p2align 2,,

// >>> This code is an adaptation of the fsqrt() function from Pico1 M0+ (c) Raspberry Pi <<<

// Copyright (c) 2020 Mark Owen https://www.quinapalus.com - GPLv2 licence

/*
Approach to square root x=sqrt(y) is as follows.

First generate a3, an approximation to 1/sqrt(y) to about 30 bits. Multiply this by y
to give a4~sqrt(y) to about 28 bits and a remainder r4=y-a4^2. Then, because
d sqrt(y) / dy = 1 / (2 sqrt(y)) let d4=r4*a3/2 and then the value a5=a4+d4 is
a better approximation to sqrt(y). If this is near a rounding boundary we
compute an exact remainder y-a5*a5 to decide whether to round up or down.

The calculation of a3 and a4 is as given in dsqrttest.c. That code verifies exhaustively
that | 1 - a3a4 | < 10*2^-32, | r4 | < 40*2^-32 and | r4/y | < 20*2^-32.

More precisely, with "y" representing y truncated to 30 binary places:

u=(q3)y;                          // 24-entry table
a0=(q8~)"1/sqrtq(x+x_ulp/2)"(u);  // first approximation from table
p0=(q16)(a0*a0) * (q16)y;
r0=(q20)(p0-1);
dy0=(q15)(r0*a0);                 // Newton-Raphson correction term
a1=(q16)a0-dy0/2;                 // good to ~9 bits

p1=(q19)(a1*a1)*(q19)y;
r1=(q23)(p1-1);
dy1=(q15~)(r1*a1);                // second Newton-Raphson correction
a2x=(q16)a1-dy1/2;                // good to ~16 bits
a2=a2x-a2x/1t16;                  // prevent overflow of a2*a2 in 32 bits

p2=(a2*a2)*(q30)y;                // Q62
r2=(q36)(p2-1+1t-31);
dy2=(q30)(r2*a2);                 // Q52->Q30
a3=(q31)a2-dy2/2;                 // good to about 30 bits
a4=(q30)(a3*(q30)y+1t-31);        // good to about 28 bits

Error analysis

          r4 = y - a4^2
          d4 = 1/2 a3*r4
          a5 = a4 + d4
          r5 = y - a5^2
             = y - ( a4 + d4 )^2
             = y - a4^2 - a3a4r4 - 1/4 a3^2*r4^2
             = r4 - a3a4r4 - 1/4 a3^2*r4^2

      | r5 | < | r4 | | 1 - a3*a4 | + 1/4 r4^2

          a5 = sqrt(y) sqrt( 1 - r5/y )
             = sqrt(y) ( 1 - 1/2 r5/y + ... )

So to first order (second order being very tiny)

     sqrt(y) - a5 = 1/2 r5/y

and

 | sqrt(y) - a5 | < 1/2 ( | r4/y | | 1 - a3*a4 | + 1/4 r4^2/y )

From dsqrttest.c (conservatively):

             < 1/2 ( 20*2^-32 * 10*2^-32 + 1/4 * 40*2^-32*20*2^-32 )
             = 1/2 ( 200 + 200 ) * 2^-64
             < 2^-56

Empirically we see about 1ulp worst-case error including rounding at Q57.

To determine correctly whether the exact remainder calculation can be skipped we need a result
accurate to < 0.25ulp at Q52, or 2^-54.
*/

	// X is zero or +Inf
dq_2:	bge	a2,a3,dq_3	// +Inf?
	li	a1,0		// return 0
	j	dq_4

	// X is negative
dq_0:	srli	a1,a1,31	// A1 <- sign flaf
	slli	a1,a1,31	// preserve sign bit
	srli	a2,a2,21	// A2 <- extract exponent
	beqz	a2,dq_4		// -0? return it
	srai	a1,a1,11	// make -Inf
	j	dq_4

dq_3:	li	a1,0x7ff<<20	// return +Inf
dq_4:	li	a0,0		// clear mantissa LOW
dq_1:	ret

.global __wrap_sqrt
.global sqrt
__wrap_sqrt:
sqrt:

	// check negative
	srli	t0,a1,31	// T0 <- sign
	slli	a2,a1,1		// A2 <- clear sign flag
	bnez	t0,dq_0		// negative?

	// check Inf, get exponent - 1 -> A2
	srli	a2,a2,21	// extract exponent
	addi	a2,a2,-1	// exponent - 1
	li	a3,0x7fe
	bgeu	a2,a3,dq_2	// catches 0 and +Inf

	// prepare mantissa with implied bit "1" -> A1:A0
	slli	a4,a2,20	// shift exponent - 1 back
	sub	a1,a1,a4	// insert implied 1

	// check even exponent
	slli	t0,a2,31	// T0 <- get bit 0 from exponent
	srli	a2,a2,1		// exponent / 2
	beqz	t0,1f		// even exponent? skip

	srli	t0,a0,31	// T0 <- carry bit
	slli	a0,a0,1		// odd exponent: shift up mantissa
	slli	a1,a1,1
	add	a1,a1,t0	// add carry

1:	srli	a3,a3,2		// exponent / 2
	add	a2,a2,a3
	slli	t1,a2,20	// T1 <- result exponent

// Here is:
//  A0:A1 = y mantissa Q52 [1,4)
//  T1 = result exponent

.equ drsqrtapp_minus_8, (drsqrtapp-8)

	la	a4,drsqrtapp_minus_8 // first eight table entries are never accessed because of the mantissa's leading 1
	srli	a2,a1,17	// y Q3
	add	t0,a2,a4
	lbu	a2,(t0)		// initial approximation to reciprocal square root m0 Q8
	srli	a3,a1,4		// first Newton-Raphson iteration
	mul	a3,a3,a2
	mul	a3,a3,a2	// i32 p0=m0*m0*(y>>14);          // Q32
	srai	a3,a3,12	// i32 a0=p0>>12;                 // Q20
	mul	a3,a3,a2
	srai	a3,a3,13	// i32 dy0=(a0*m0)>>13;           // Q15
	slli	a2,a2,8
	sub	a2,a2,a3	// i32 m1=(m0<<8)-dy0;         // Q16

	mv	a3,a2
	mul	a3,a3,a3
	srli	a3,a3,13
	srli	a4,a1,1
	mul	a3,a3,a4	// i32 p1=((m1*m1)>>11)*(y>>11);  // Q19*Q19=Q38
	srai	a3,a3,15	// i32 a1=p1>>15;                 // Q23
	mul	a3,a3,a2
	srai	a3,a3,23
	addi	a3,a3,1
	srai	a3,a3,1		// i32 dy1=(a1*m1+(1<<23))>>24;   // Q23*Q16=Q39; Q15
	sub	a2,a2,a3	// i32 m2=m1-dy1;                 // Q16
	srli	a3,a2,16
	sub	a2,a2,a3	// if(m2>=0x10000) m2=0xffff; to prevent overflow of m2*m2

// Here is:
//  A0:A1 = y mantissa
//  A2 = m2 ~ 1/sqrt(y) Q16
//  T1 = result exponent

	mv	a3,a2
	mul	a3,a3,a3
	slli	a1,a1,10
	srli	a4,a0,22
	or	a1,a1,a4	// y Q30

	mul	a4,a1,a3
	mulhu	a3,a1,a3

	slli	a5,a3,6
	srli	a4,a4,26
	or	a4,a4,a5
	addi	a4,a4,0x20	// i32 a2=(p2>>26)+0x20;          // Q36 a4
	zext.h	a5,a4
	mul	a5,a5,a2
	srai	a4,a4,16
	mul	a4,a4,a2
	srli	a5,a5,16
	add	a4,a4,a5
	srai	a4,a4,6		// i32 dy2=((i64)a2*(i64)m2)>>22; // Q36*Q16=Q52; Q30
	slli	a2,a2,15
	sub	a2,a2,a4

// Here is:
//  A0 = y low bits
//  A1 = y Q30
//  A2 = m3 ~ 1/sqrt(y) Q31
//  T1 = result exponent

	mul	a3,a2,a1
	mulhu	a4,a2,a1

	srli	t0,a3,31	// T0 <- carry
	slli	a3,a3,1
	slli	a4,a4,1
	add	a4,a4,t0

	srli	t0,a3,31	// T0 <- carry
	add	a3,a4,t0	// ui32 m4=((ui64)m3*(ui64)y+(1U<<31))>>31; // Q30

// Here is:
//  A0 = y low bits
//  A1 = y Q30
//  A2 = m3 Q31 ~ 1/sqrt(y)
//  A3 = m4 Q30 ~ sqrt(y)
//  T1 = result exponent

	mul	a4,a3,a3
	mulhu	a5,a3,a3

	slli	t0,a0,8
	srli	a7,a1,2

	sub	a6,t0,a4
	sltu	t0,t0,a6
	sub	a7,a7,a5	// a4=(q60)y-m4*m4
	sub	a7,a7,t0

// by exhaustive testing, a4 = fffffffc0e134fdc .. 00000003c2bf539c Q60

	slli	a5,a7,29
	bexti	t0,a6,2
	srli	a6,a6,3
	add	a6,a6,a5	// a4 Q57 with rounding
	add	t0,a6,t0	// rounding

	mul	a6,t0,a2
	mulhsu	a2,t0,a2	// d4=m3*a4/2 Q89

// a4+d4 is correct to 1ULP at Q57, tested on ~9bn cases including all extreme values of a4 for each possible y Q30

	addi	a2,a2,8
	bexti	t0,a2,4
	srai	a2,a2,5		// d4 Q52, rounded to Q53 with spare bit in carry

// Here is:
//  A0 = y low bits
//  A1 = y Q30
//  A2 = d4 Q52, rounded to Q53
//  T0 = contains d4_b53
//  A3 = m4 Q30

	bnez	t0,dq_5

	srli	a5,a3,10	// m4 Q52
	slli	a4,a3,22

	srai	a1,a2,31

	add	a0,a2,a4
	sltu	t0,a0,a4
	add	a1,a1,a5	// m4+d4
	add	a1,a1,t0

	add	a1,a1,t1	// pack exponent
	ret

dq_5:
// here we are near a rounding boundary
	add	a2,a2,a2	// d4 Q53+1ulp
	addi	a2,a2,1
	srli	a5,a3,9
	slli	a4,a3,23	// a4:a5 m4 Q53
	srai	a1,a2,31

	add	a4,a2,a4
	sltu	t0,a4,a2
	add	a5,a5,a1	// a4:a5 m5=m4+d4 Q53+1ulp
	add	a5,a5,t0

	mv	a3,a5
	mul	a3,a3,a4

	mul	a1,a4,a4
	mulhu	a2,a4,a4

	add	a2,a2,a3
	add	a2,a2,a3		// a1:a2 m5^2 Q106
	slli	a0,a0,22	// y Q84

	neg	a1,a1
	sub	a0,a0,a2	// remainder y-m5^2
	beqz	a1,5f
	addi	a0,a0,-1

5:	bltz	a0,1f		// y<m5^2: no need to increment m5

	addi	a4,a4,1
	bnez	a4,1f
	addi	a5,a5,1		// bump m5 if over rounding boundary

1:	srli	a0,a4,1
	srli	a1,a5,1
	slli	a5,a5,31
	or	a0,a0,a5
	add	a1,a1,t1
	ret

double_section __drsqrtapp
.p2align 2,,

// round(sqrt(2^22./[68:8:252]))
drsqrtapp:
	.byte	0xf8,0xeb,0xdf,0xd6,0xcd,0xc5,0xbe,0xb8
	.byte	0xb2,0xad,0xa8,0xa4,0xa0,0x9c,0x99,0x95
	.byte	0x92,0x8f,0x8d,0x8a,0x88,0x85,0x83,0x81

// ----------------------------------------------------------------------------
//                    Natural exponent - Chebyshev approximation
// ----------------------------------------------------------------------------
// double exp(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6, T0..T2
// 2080ns avg=0.102 max=1

double_section __exp
.p2align 2,,

.global __wrap_exp
.global exp
__wrap_exp:
exp:

// Here is: A1:A0 = input number

	// check if number A1:A0 is negative
	bltz	a1,2f			// number is negative

	// number A1:A0 is positive - check maximal positive number (exp = 9)
	// exp max. value: 709.78271289338401 = 0x40862E42FEFA39EF, infinity: 709.78271289338419 = 0x40862E42FEFA39F1
	li	t0,0x40862E42		// T0 <- max. positive value HIGH
	bltu	a1,t0,4f		// x < max, continue
	bne	a1,t0,1f		// x > max, return +inf
	li	t0,0xFEFA39EF		// T0 <- max. positive value LOW
	bleu	a0,t0,4f		// x <= max, continue

	// max. value, return +inf
1:	li	a1,0x7ff00000		// +inf HIGH
	li	a0,0			// +inf LOW
	ret

	// number A1:A0 is negative - check maximal negative number (exp = 9)
	// exp min. value: -708.39641853226407 = 0xC086232BDD7ABCD2, zero: -708.39641853226424 = 0xC086232BDD7ABCD3
2:	li	t0,0xC086232B		// T0 <- max. negative value HIGH
	bltu	a1,t0,4f		// x < max, continue
	bne	a1,t0,3f		// x > max, return +0.0
	li	t0,0xDD7ABCD2		// T0 <- max. negative value LOW
	bleu	a0,t0,4f		// x <= max, continue

	// max. value, return +0.0
3:	li	a1,0			// +0.0 HIGH
	li	a0,0			// +0.0 LOW
	ret

	// get signed exponent -> A4
4:	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position
	addi	a4,a4,-DOUBLE_EXP1	// subtract exponent bias

	// check minimal exponent - return 1.0
	li	t0,-53			// T0 <- minimal valid exponent
	blt	a4,t0,exp_14		// exponent is too small - return +1.0

	// get sign -> A5
	srli	a5,a1,31		// A5 <- sign 0 or 1

// Here is:
//  A1:A0 = input number x
//  A4 = signed exponent in range -53..+9
//  A5 = sign 0 or 1

	// normalize mantissa A1:A0 - shift to max position and restore hidden bit '1', mantissa format 1.63
	// - mantissa will be 0x8000000000000000..0xFFFFFFFFFFFFF800
	//    max exp: (+)0xB17217F7D1CF7800 exp=9, (-)0xB1195EEBD5E69000 exp=9
	slli	a1,a1,11		// shift mantissa HIGH << 11 to destroy exponent
	bset	a1,a1,31		// set hidden bit "1"
	srli	t0,a0,21		// T0 <- carry from mantissa LOW
	or	a1,a1,t0		// add carry from mantissa LOW to mantissa HIGH
	slli	a0,a0,11		// shift mantissa LOW << 11

	// prepare pointer to data table -> A6
	la	a6,_exptab		// A6 <- pointer to data table

// continue from exp & exp10
exp_exp10_cont:

// Here is:
//  A1:A0 = input number x
//  A4 = signed exponent in range -53..+10
//  A5 = sign 0 or 1
//  A6 = pointer to data table
// max exp10: (+)0x58B90BFBE8E7B876 exp=10, min exp10: (-)0x588CAF75EAF34495 exp=10
// max exp: (+)0xB17217F7D1CF7800 exp=9, (-)0xB1195EEBD5E69000 exp=9
// exp is in range -53..+9 for exp, -53..+10 for exp10

	// shift mantissa A1:A0 right by exponent bits (shift >>= 10 - exp)
	// - get x as fixed number in format 11.53
	// - exp is in range -53..+10, shift is in range 0..63
	li	t0,10			// T0 <- 10
	sub	t0,t0,a4		// T0 <- number of shifts = 10 - exp (= 1..63)
	add	t1,t0,-32		// check number of shift - 32 (= -31..+31)
	bgez	t1,2f			// number of shifts is >= 32
	beqz	t0,4f			// shift = 0

	// number of shifts = 1..31 (exp = -21..+9)
	addi	t1,a4,22		// T1 <- shift inversion = 32 - (10 - exp) = 22 + exp (= 1..31)
	srl	a0,a0,t0		// shift mantissa LOW
	sll	t1,a1,t1		// T1 <- carry from mantissa HIGH
	or	a0,a0,t1		// add carry from mantissa HIGH to mantissa LOW
	srl	a1,a1,t0		// shift mantissa HIGH
	j	4f

	// number of shifts = 32..63 (exp = -53..-22), T1 = 0..31
2:	srl	a0,a1,t1		// A0 <- shift mantissa HIGH
	li	a1,0			// A1 <- mantissa HIGH = 0

// Here is:
//  A1:A0 = mantissa x in fixed point format 11.53
//  A5 = sign 0 or 1
//  A6 = pointer to data table
// exp10: max 0x58B90BFBE8E7B876 = 6393154322601326710
// exp: max 0x58B90BFBE8E7BC00 = 6393154322601327616

	// convert number to 2-base (multiply by 1/ln(2)), format 12.52
	// 1/ln(2) << 63 = 13306513097844322492 round up
4:	lw	t0,0x64(a6)		// T0 <- low(13306513097844322492)
	lw	t1,0x68(a6)		// T1 <- high(13306513097844322492)

// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high t0,t1,a0,a1,a2,a3

// exp10: 0 .. +0x3FFFFFFFFFFFFCD8 (= 4611686018427387096)
// exp: 0 .. +0x3FFFFFFFFFFFFF65 (= 4611686018427387749)

	// get integer part of exponent: shift >> 52, round to zero
	// - exponent will be 0..+1023
	srli	a4,t1,20		// A4 <- exponent 0..1023

	// convert exponent A4 back to e-base: multiply by ln(2) -> A3:A2
	//   ln(2) << 64 = 12786308645202655660 round up
	lw	t0,0x6c(a6)		// T0 <- constant LOW
	lw	t1,0x70(a6)		// T1 <- constant HIGH
	mul	a2,t0,a4
	mulhu	a3,t0,a4
	mul	t1,t1,a4
	add	a3,a3,t1		// A3:A2 = exp*const

	// shift mantissa A1:A0 << 11 to format 0.64 -> A1:T0
	slli	a1,a1,11
	srli	t1,a0,21
	slli	t0,a0,11
	or	a1,a1,t1

	// subtract exponent correction A3:A2 from the number A1:T0 -> A1:A0
	sub	a0,t0,a2
	sltu	t0,t0,a0
	sub	a1,a1,a3
	sub	a1,a1,t0

	// convert to range 0..1.0: multiply by 1/ln(2) ... number will be in 0.64 format
	//   1/ln(2) << 63 = 13306513097844322492 round up
	lw	a2,0x64(a6)		// A2 <- constant LOW
	lw	a3,0x68(a6)		// A3 <- constant HIGH
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high a0, a1, a2, a3, t0, t1

	// overflow
	bgez	a1,4f			// number is OK
	li	a0,-1			// overflow, use value -1
	li	a1,-1

	// convert mantissa <<= 1 from format 1.63 to format 0.64 -> A3:A2
4:	slli	a1,a1,1
	srli	t0,a0,31
	slli	a2,a0,1
	or	a3,a1,t0

// continue from exp2
exp2_cont:

// Here is:
//  A1:A0 = accumulator acc
//  A3:A2 = number xx
//  A4 = exponent 0..1023
//  A5 = sign 0 or 1
//  A6 = pointer to data table

	// coeff[16]
	li	a0,1260			// coeff[16] = 1.3661088453272162965e-16 << 63
	mulhu	a0,a0,a3		// acc <- xxH * acc

	// number is positive
	lw	t0,(a6)			// T0 <- coeff[15] = 29109 = 3.1560349252155140648e-15 << 63
	la	t2,_exptabend2-0x14	// T2 <- end of the table 2
	bnez	a5,8f			// skip if number is negative

	// coeff[15]
	add	a0,a0,t0		// add coeff[15]
	lw	t0,4(a6)		// T0 <- coeff[14] = 625212 = 6.7785672188660199117e-14 << 63
	mulhu	a0,a0,a3		// acc <- xxH * acc

	// coeff[14]
	add	a0,a0,t0		// add coeff[14]
	lw	t0,8(a6)		// T0 <- coeff[13] = 12627788 = 1.3691075408642918388e-12 << 63
	mulhu	a0,a0,a3		// acc <- xxH * acc

	// coeff[13]
	add	a0,a0,t0		// add coeff[13]
	lw	t0,0x0c(a6)		// T0 <- coeff[12] = 236841782 = 2.5678437471925562869e-11 << 63
	mulhu	a0,a0,a3		// acc <- xxH * acc

	// coeff[12]
	add	a0,a0,t0		// add coeff[12]
	lw	t0,0x10(a6)		// T0 <- coeff[11] = 4100285693 = 4.4455386559996910448e-10 << 63
	mulhu	a0,a0,a3		// acc <- xxH * acc

	// coef[11]
	add	a0,a0,t0		// add coeff[11]
	sltu	a1,a0,t0		// carry

	// multiply accumulator (A1:A0 = aH:aL) by the number x (A3:A2 = bH:bL) -> A1:A0
// multiply (u64*u64->u128)>>64 simplified
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high a0, a1, a2, a3, t0, t1

	// add constant to accumulator -> A1:A0
5:	lw	t0,0x14(a6)		// T0 <- load constant LOW
	lw	t1,0x18(a6)		// T1 <- load constant HIGH
	addi	a6,a6,8			// shift table pointer
	add	a0,t0,a0		// add constant LOW
	add	a1,t1,a1		// add constant HIGH
	sltu	t0,a0,t0		// T0 <- carry
	add	a1,a1,t0		// add carry

	// multiply accumulator (A1:A0 = aH:aL) by the number x (A3:A2 = bH:bL) -> A1:A0
// multiply (u64*u64->u128)>>64 simplified
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high a0, a1, a2, a3, t0, t1

	// loop
	bne	a6,t2,5b		// loop

	// add coeff[0] = 9223372036854775808+6 = 1.0 << 63 = 0x8000000000000000+6
	li	t1,0x80000000
	li	t0,0x00000000+6
	add	a0,t0,a0		// add constant LOW
	add	a1,t1,a1		// add constant HIGH
	sltu	t0,a0,t0		// T0 <- carry
	add	a1,a1,t0		// add carry

	// overflow (result should be 0x800... to 0xfff...)
	bltz	a1,9f			// result is OK
	li	a0,-1
	li	a1,-1
	j	9f

// Here is:
//  A1:A0 = accumulator acc
//  A3:A2 = number xx
//  A4 = exponent 0..1023
//  A5 = sign 0 or 1
//  A6 = pointer to data table

	// number is negative
8:	neg	a4,a4			// negate exponent

	// coeff[15]
	sub	a0,t0,a0		// sub coeff[15]
	lw	t0,4(a6)		// T0 <- coeff[14] = 625212 = 6.7785672188660199117e-14 << 63
	mulhu	a0,a0,a3		// acc <- xxH * acc

	// coeff[14]
	sub	a0,t0,a0		// sub coeff[14]
	lw	t0,8(a6)		// T0 <- coeff[13] = 12627788 = 1.3691075408642918388e-12 << 63
	mulhu	a0,a0,a3		// acc <- xxH * acc

	// coeff[13]
	sub	a0,t0,a0		// sub coeff[13]
	lw	t0,0x0c(a6)		// T0 <- coeff[12] = 236841782 = 2.5678437471925562869e-11 << 63
	mulhu	a0,a0,a3		// acc <- xxH * acc

	// coeff[12]
	sub	a0,t0,a0		// sub coeff[12]
	lw	t0,0x10(a6)		// T0 <- coeff[11] = 4100285693 = 4.4455386559996910448e-10 << 63
	mulhu	a0,a0,a3		// acc <- xxH * acc

	// coeff[11]
	sub	a0,t0,a0		// sub coeff[11]
	mulhu	a0,a0,a3		// acc <- xxH * acc
	li	a1,0

	// subtract accumulator from constant
5:	lw	t0,0x14(a6)		// T0 <- load constant LOW
	lw	t1,0x18(a6)		// T1 <- load constant HIGH
	addi	a6,a6,8			// shift table pointer
	sub	a0,t0,a0		// subtract constant LOW
	sub	a1,t1,a1		// subtract constant HIGH
	sltu	t0,t0,a0		// T0 <- carry
	sub	a1,a1,t0		// sub carry

	// multiply accumulator (A1:A0 = aH:aL) by the number x (A3:A2 = bH:bL) -> A1:A0
// multiply (u64*u64->u128)>>64 simplified
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high a0, a1, a2, a3, t0, t1

// max exp10: (+)0x58B90BFBE8E7B876 exp=10, min exp10: (-)0x588CAF75EAF34495 exp=10

	// loop
	bne	a6,t2,5b		// loop

	// subtract coeff[0] = 9223372036854775808-5 = 1.0 << 63 = 0x8000000000000000-5
	li	t1,0x7fffffff
	li	t0,0x00000000-5
	sub	a0,t0,a0		// subtract constant LOW
	sub	a1,t1,a1		// subtract constant HIGH
	sltu	t0,t0,a0		// T0 <- carry
	sub	a1,a1,t0		// sub carry

	// mantissa correcction <<= 1
	slli	a1,a1,1
	srli	t0,a0,31
	slli	a0,a0,1
	or	a1,a1,t0

	// exponent - 1
	addi	a4,a4,-1

	// rounding
9:	li	t0,0x400+3
	add	a0,a0,t0		// add correction
	sltu	t0,a0,t0		// T0 <- carry
	add	a1,a1,t0		// add carry

	// overflow (result should be 0x800... to 0xfff...)
	bltz	a1,1f			// result is OK
	li	a0,0
	li	a1,0x80000000		// overflow to 1.0
	addi	a4,a4,1			// exponent + 1 (it can become inf)

	// normalize mantissa to 12.52 (shift >> 11)
1:	srli	a0,a0,11
	slli	t0,a1,21
	srli	a1,a1,11
	or	a0,a0,t0

	// return number (use "exponent - 1" to clear hidden bit '1' of the mantissa)
	addi	a4,a4,DOUBLE_EXP1-1	// exponent + bias - 1
	slli	a4,a4,20		// shift exponent to position
	add	a1,a1,a4		// add exponent to result
	ret

exp_14:
	// exponent is too small - return 1.0
	li	a1,0x3ff00000		// +1.0 HIGH
	li	a0,0			// +1.0 LOW
	ret

// ---- continue to exp2

// ----------------------------------------------------------------------------
//                    Binary exponent - Chebyshev approximation
// ----------------------------------------------------------------------------
// double exp2(double x);
// INPUT: A1:A0 = argument x
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6, T0..T2
// 1810ns avg=0.093 max=1

.global __wrap_exp2
.global exp2
__wrap_exp2:
exp2:

// Here is: A1:A0 = input number

	// check if number A1:A0 is negative
	bltz	a1,2f			// number is negative

	// number A1:A0 is positive - check maximal positive number (exp = 9)
	// exp2 max. value: 1023.9999999999997 = 0x408FFFFFFFFFFFFF, infinity: 1024.0000000000000 = 0x4090000000000000
	li	t0,0x408FFFFF		// T0 <- max. positive value HIGH
	bltu	a1,t0,4f		// x < max, continue
	bgtu	a1,t0,1f		// x > max, return +inf
	li	t0,0xFFFFFFFF		// T0 <- max. positive value LOW
	bleu	a0,t0,4f		// x <= max, continue

	// max. value, return +inf
1:	li	a1,0x7ff00000		// +inf HIGH
	li	a0,0			// +inf LOW
	ret

	// number A1:A0 is negative - check maximal negative number (exp = 9)
	// exp2 min. value: -1022.0000000000000 = 0xC08FF00000000000, zero: -1022.0000000000001 = 0xC08FF00000000001
2:	li	t0,0xC08FF000		// T0 <- max. negative value HIGH
	bltu	a1,t0,4f		// x < max, continue
	bgtu	a1,t0,3f		// x > max, return +0.0
	li	t0,0x00000000		// T0 <- max. negative value LOW
	bleu	a0,t0,4f		// x <= max, continue

	// max. value, return +0.0
3:	li	a1,0			// +0.0 HIGH
	li	a0,0			// +0.0 LOW
	ret

	// get signed exponent -> A4
4:	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position
	addi	a4,a4,-DOUBLE_EXP1	// subtract exponent bias

	// check minimal exponent - return 1.0
	li	t0,-53			// T0 <- minimal valid exponent
	blt	a4,t0,exp_14		// exponent is too small - return +1.0

	// get sign -> A5
	srli	a5,a1,31		// A5 <- sign 0 or 1

// Here is:
//  A1:A0 = input number x
//  A4 = signed exponent in range -53..+9
//  A5 = sign 0 or 1

	// normalize mantissa A1:A0 - shift to max position and restore hidden bit '1', mantissa format 1.63
	// - mantissa will be 0x8000000000000000..0xFFFFFFFFFFFFF800
	//    max exp2: (+)0xFFFFFFFFFFFFF800 exp=9, (-)0xFF80000000000000 exp=9
	slli	a1,a1,11		// shift mantissa HIGH << 11 to destroy exponent
	bset	a1,a1,31		// set hidden bit "1"
	srli	t0,a0,21		// T0 <- carry from mantissa LOW
	or	a1,a1,t0		// add carry from mantissa LOW to mantissa HIGH
	slli	a0,a0,11		// shift mantissa LOW << 11

	// prepare pointer to data table -> A6
	la	a6,_exptab		// A6 <- pointer to data table

// Here is:
//  A1:A0 = input number x
//  A4 = signed exponent in range -53..+9
//  A5 = sign 0 or 1
//  A6 = pointer to data table
// max exp2: (+)0xFFFFFFFFFFFFF800 exp=9, (-)0xFF80000000000000 exp=9
// exp is in range -53..+9

	// shift mantissa A1:A0 right by exponent bits (shift >>= 10 - exp)
	// - get x as fixed number in format 11.53
	// - exp is in range -53..+9, shift is in range 1..63
	li	t0,10			// T0 <- 10
	sub	t0,t0,a4		// T0 <- number of shifts = 10 - exp (= 1..63)
	add	t1,t0,-32		// check number of shift - 32 (= -31..+31)
	bgez	t1,2f			// number of shifts is >= 32

	// number of shifts = 1..31 (exp = -21..+9)
	addi	t1,a4,22		// T1 <- shift inversion = 32 - (10 - exp) = 22 + exp (= 1..31)
	srl	a0,a0,t0		// shift mantissa LOW
	sll	t1,a1,t1		// T1 <- carry from mantissa HIGH
	or	a0,a0,t1		// add carry from mantissa HIGH to mantissa LOW
	srl	a1,a1,t0		// shift mantissa HIGH
	j	4f

	// number of shifts = 32..63 (exp = -53..-22), T1 = 0..31
2:	srl	a0,a1,t1		// A0 <- shift mantissa HIGH
	li	a1,0			// A1 <- mantissa HIGH = 0

	// get integer part of exponent: shift >> 52, round to zero
	// - exponent will be 0..+1023
4:	srli	a4,a1,21		// A4 <- exponent 0..1023

	// shift mantissa A1:A0 << 11 to format 0.64 -> A3:A2
	slli	a1,a1,11
	srli	t1,a0,21
	slli	a2,a0,11
	or	a3,a1,t1

//  A3:A2 = number xx
//  A4 = exponent 0..1023
//  A5 = sign 0 or 1
//  A6 = pointer to data table

	j	exp2_cont

// ---- continue to exp10

// ----------------------------------------------------------------------------
//                    Decimal exponent - Chebyshev approximation
// ----------------------------------------------------------------------------
// double exp10(double x);
// INPUT: A1:A0 = argument x
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6, T0..T2
// 2190ns avg=0.288 max=1

.global __wrap_exp10
.global exp10
__wrap_exp10:
exp10:

// Here is: A1:A0 = input number

	// check if number A1:A0 is negative
	bltz	a1,2f			// number is negative

	// number A1:A0 is positive - check maximal positive number (exp = 8)
	// exp10 max. value: 308.25471555991668 = 0x40734413509F79FE, infinity: 308.25471555991676 = 0x40734413509F79FF
	li	t0,0x40734413		// T0 <- max. positive value HIGH
	bltu	a1,t0,4f		// x < max, continue
	bgtu	a1,t0,1f		// x > max, return +inf
	li	t0,0x509F79FE		// T0 <- max. positive value LOW
	bleu	a0,t0,4f		// x <= max, continue

	// max. value, return +inf
1:	li	a1,0x7ff00000		// +inf HIGH
	li	a0,0			// +inf LOW
	ret

	// number A1:A0 is negative - check maximal negative number (exp = 8)
	// exp10 min. value: -307.65265556858873 = 0xC0733A7146F72A41, zero: -307.65265556858878 = 0xC0733A7146F72A42
2:	li	t0,0xC0733A71		// T0 <- max. negative value HIGH
	bltu	a1,t0,4f		// x < max, continue
	bgtu	a1,t0,3f		// x > max, return +0.0
	li	t0,0x46F72A41		// T0 <- max. negative value LOW
	bleu	a0,t0,4f		// x <= max, continue

	// max. value, return +0.0
3:	li	a1,0			// +0.0 HIGH
	li	a0,0			// +0.0 LOW
	ret

	// get signed exponent -> A4
4:	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position
	addi	a4,a4,-DOUBLE_EXP1	// subtract exponent bias

	// check minimal exponent - return 1.0
	li	t0,-55			// T0 <- minimal valid exponent
	blt	a4,t0,exp_14		// exponent is too small - return +1.0

	// get sign -> A5
	srli	a5,a1,31		// A5 <- sign 0 or 1

// Here is:
//  A1:A0 = input number x
//  A4 = signed exponent in range -55..+8
//  A5 = sign 0 or 1

	// normalize mantissa A1:A0 - shift to max position and restore hidden bit '1', mantissa format 1.63
	// - mantissa will be 0x8000000000000000..0xFFFFFFFFFFFFF800
	//    max exp10: (+)0x9A209A84FBCFF000 exp=8, (-)0x99D38A37B9520800 exp=8
	slli	a1,a1,11		// shift mantissa HIGH << 11 to destroy exponent
	bset	a1,a1,31		// set hidden bit "1"
	srli	t0,a0,21		// T0 <- carry from mantissa LOW
	or	a1,a1,t0		// add carry from mantissa LOW to mantissa HIGH
	slli	a0,a0,11		// shift mantissa LOW << 11

	// prepare pointer to data table -> A6
	la	a6,_exptab		// A6 <- pointer to data table

	// multiply mantissa by ln(10) << 62 (= 10618799479599967255 round up) = mantissa/4
	lw	a2,0x74(a6)		// A2 <- ln(10) low
	lw	a3,0x78(a6)		// A3 <- ln(10) high
// multiply (u64*u64->u128)>>64 simplified
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high a0, a1, a2, a3, t0, t1

	// increase exponent + 2
	addi	a4,a4,2
	j	exp_exp10_cont

double_section __exptab
.p2align 2,,

_exptab:
	.word	0x000071B5 		// 0x00: coeff[15] = 29109 = 3.1560349252155140648e-15 << 63
	.word	0x00098A3C 		// 0x04: coeff[14] = 625212 = 6.7785672188660199117e-14 << 63
	.word	0x00C0AF4C 		// 0x08: coeff[13] = 12627788 = 1.3691075408642918388e-12 << 63
	.word	0x0E1DEB36 		// 0x0C: coeff[12] = 236841782 = 2.5678437471925562869e-11 << 63
	.word	0xF46564FD 		// 0x10: coeff[11] = 4100285693 = 4.4455386559996910448e-10 << 63
	.word	0x267A8ABF, 0x0000000F	// 0x14: coeff[10] = 65070074559 = 7.0549116200190313717e-9 << 63
	.word	0x929E9BF4, 0x000000DA	// 0x1C: coeff[9] = 938762738676 = 1.0178086007207596430e-7 << 63
	.word	0x0111D2E6, 0x00000B16	// 0x24: coeff[8] = 12189135131366 = 1.3215486790146656182e-6 << 63
	.word	0xFF1622FB, 0x00007FF2	// 0x2C: coeff[7] = 140681638454011 = 1.5252733804065937795e-5 << 63
	.word	0x4BE1B1E2, 0x00050C24	// 0x34: coeff[6] = 1420724914991586 = 1.5403530393381606220e-4 << 63
	.word	0xCF14CE59, 0x002BB0FF	// 0x3C: coeff[5] = 12298036735954521 = 1.3333558146428433721e-3 << 63
	.word	0xFBA4E773, 0x013B2AB6	// 0x44: coeff[4] = 88711583058159475 = 9.6181291076284771650e-3 << 63
	.word	0xC1282FE3, 0x071AC235	// 0x4C: coeff[3] = 511935043789664227 = 5.5504108664821580022e-2 << 63
	.word	0x82C58EA8, 0x1EBFBDFF	// 0x54: coeff[2] = 2215698446797868712 = 2.4022650695910071233e-1 << 63
	.word	0xE8E7BCD6, 0x58B90BFB	// 0x5C: coeff[1] = 6393154322601327830 = 6.9314718055994530943e-1 << 63
_exptabend2:
	.word	0x5C17F0BC+1, 0xB8AA3B29 // 0x64: 1/ln(2) << 63 = 13306513097844322492 + 1
	.word	0xD1CF79AC, 0xB17217F7	// 0x6C: ln(2) << 64 = 12786308645202655660
	.word	0xAAA8AC17+2, 0x935D8DDD // 0x74: ln(10) << 62 = 10618799479599967255

// ----------------------------------------------------------------------------
//                   Natural logarithm - Chebyshev approximation
// ----------------------------------------------------------------------------
// double log(double x);
// double log2(double x);
// double log10(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6,T0..T2
// log(): 3500ns, err avg=0.071, err max=1
// log2(): 3420ns, err avg=0.002, err max=2
// log10(): 3530ns, err avg=0.036, err max=3

double_section _log
.p2align 2,,

.global __wrap_log2
.global log2
__wrap_log2:
log2:
	li	a6,1			// A6 <- 1, log2() mode
	j	log_start

.global __wrap_log10
.global log10
__wrap_log10:
log10:
	li	a6,-1			// A6 <- -1, log10() mode
	j	log_start

.global __wrap_log
.global log
__wrap_log:
log:
	li	a6,0			// A6 <- 0, log() mode

log_start:

// ---- prepare

	// result is 0.0 if number is 1.0
	beqz	a0,log_res0		// number is 1.0 low ?
log_not1:

	// get unsigned exponent -> A4
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position

	// zero returns -inf (both positive and negative zero)
	beqz	a4,log_minf		// number is zero, return -inf

	// negative number returns +inf
	bltz	a1,log_pinf		// negative number returns +inf

	// positive infinity returns +inf
	li	t0,0x7ff		// T0 <- infinity exponent
	beq	a4,t0,log_pinf		// positive infinity returns +inf

	// get signed exponent -> A4
	addi	a4,a4,-DOUBLE_EXP1	// subtract exponent bias

	// prepare pointer to data table -> A5
	la	a5,_logtab		// A5 <- pointer to data table

	// get mantissa in 1.63 format signed, convert from range 1.0000..1.9999 to range -1.0000..0.9999 -> A3:A2
	//  s64 m = ((xn<<11)|B63);	// convert mantissa to 1.63 format, range 1.0000..1.9999
	//  m -= B63;			// subtract 1, convert to range 0.0000..0.9999
	//  m <<= 1;			// mantissa * 2, convert to range 0.0000..1.9999
	//  m -= B63;			// subtract 1, convert to range -1.0000..0.9999
	// or simplified: s64 m = (xn << 12) ^ (1ull << 63);
	slli	a1,a1,12		// shift mantissa high << 12
	srli	t0,a0,20		// T0 <- carry from mantissa low
	or	a1,a1,t0		// add carry to mantissa high
	slli	a2,a0,12		// A2 <- shift mantissa low << 12
	binvi	a3,a1,31		// A3 <- mantissa - bit63 (= subtract 1)

// Here is:
//  A1:A0 = result accumulator
//  A3:A2 = input mantissa x
//  A4 = signed exponent
//  A5 = pointer to data table
//  A6 = mode 0=log, 1=log2, -1=log10

// ---- process coefficients 26 to 18

	// coeff[26] = -3.1292075768665710843e-014 << 64
	lw	a0,0x00(a5)			// A0 <- coeff[26]
	lw	t0,0x04(a5)			// T0 <- coeff[25]
	mulh	a0,a0,a3			// A0 <- accumulator * mantissa

	// coeff[25] = 9.7431876039391340245e-014 << 63
	add	a0,a0,t0			// A0 <- add coeff
	lw	t0,0x08(a5)			// T0 <- coeff[24]
	mulh	a0,a0,a3			// A0 <- accumulator * mantissa

	// coeff[24] = -9.2584594371947089050e-014 << 63
	sh1add	a0,a0,t0			// A0 <- add coeff
	lw	t0,0x0c(a5)			// T0 <- coeff[23]
	mulh	a0,a0,a3			// A0 <- accumulator * mantissa

	// coeff[23] = 2.9112908581834786416e-013 << 63
	sh1add	a0,a0,t0			// A0 <- add coeff
	lw	t0,0x10(a5)			// T0 <- coeff[22]
	mulh	a0,a0,a3			// A0 <- accumulator * mantissa

	// coeff[22] = -1.5511222530986678301e-012 << 63
	sh1add	a0,a0,t0			// A0 <- add coeff
	lw	t0,0x14(a5)			// T0 <- coeff[21]
	mulh	a0,a0,a3			// A0 <- accumulator * mantissa

	// coeff[21] = 4.8712103978938373001e-012 << 63
	sh1add	a0,a0,t0			// A0 <- add coeff
	lw	t0,0x18(a5)			// T0 <- coeff[20]
	mulh	a0,a0,a3			// A0 <- accumulator * mantissa

	// coeff[20] = -1.4218342495132474418e-011 << 63
	sh1add	a0,a0,t0			// A0 <- add coeff
	lw	t0,0x1c(a5)			// T0 <- coeff[19]
	mulh	a0,a0,a3			// A0 <- accumulator * mantissa

	// coeff[19] = 4.4906320218786305240e-011 << 63
	sh1add	a0,a0,t0			// A0 <- add coeff
	lw	t0,0x20(a5)			// T0 <- coeff[18]
	mulh	a0,a0,a3			// A0 <- accumulator * mantissa

	// coeff[18] = -1.4349535043305586936e-010 << 63
	sh1add	a0,a0,t0			// A0 <- add coeff
	mulh	a0,a0,a3			// A0 <- accumulator * mantissa
	srai	a1,a0,31			// A1 <- extend mantissa to s64
	slli	a0,a0,1				// A0 << 1

// Here is:
//  A1:A0 = result accumulator
//  A3:A2 = input mantissa x
//  A4 = signed exponent
//  A5 = pointer to data table
//  A6 = mode 0=log, 1=log2, -1=log10

// ---- add coefficients in case of positive mantissa

	// positive mantissa A3:A2
	lw	t0,0x24(a5)			// T0 <- coeff[17]
	li	t1,0
	la	t3,_logtab_end - 0x28		// T3 <- end of data table
	bltz	a3,4f				// skip if mantissa is negative

	// mantissa A3:A2 << 1
	slli	a3,a3,1
	srli	t2,a2,31
	or	a3,a3,t2
	slli	a2,a2,1

	// add coeff T1:T0 to acc A1:A0 -> A1:A0
2:	add	a0,a0,t0
	sltu	t0,a0,t0
	add	a1,a1,t1
	add	a1,a1,t0

	// absolute value of the accumulator -> A1:A0
	mv	t2,a1				// T2 <- save acc HIGH, for later check sign
	bgez	t2,1f

//	snez	t0,a0
//	neg	a1,a1
//	neg	a0,a0
//	sub	a1,a1,t0

	not	a0,a0				// use NOT instead of NEG, faster and better results
	not	a1,a1

	// multiply
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
1:	mulu64high a0,a1,a2,a3,t0,t1

	// negate result
	// - no carry from LOW, so we use more common case NOT instead of NEG
	bgez	t2,1f
	not	a0,a0
	not	a1,a1

	// load next constant
1:	lw	t0,0x28(a5)			// T0 <- coeff low
	lw	t1,0x2C(a5)			// T1 <- coeff high
	addi	a5,a5,8				// shift table pointer

	// loop
	bne	a5,t3,2b			// next coeff
	j	8f

// ---- add coefficients in case of maximum negative mantissa 0x8000000000000000

	// maximum negative
4:	bnez	a2,6f				// check value 0x8000000000000000
	li	t2,0x80000000
	bne	a3,t2,6f

	// add coeff T1:T0 to acc A1:A0 -> A1:A0
5:	add	a0,a0,t0
	sltu	t0,a0,t0
	add	a1,a1,t1
	add	a1,a1,t0

	// multiply 0x80.. = negate
	snez	t0,a0
	neg	a1,a1
	neg	a0,a0
	sub	a1,a1,t0

	// load next constant
1:	lw	t0,0x28(a5)			// T0 <- coeff low
	lw	t1,0x2c(a5)			// T1 <- coeff high
	addi	a5,a5,8				// shift table pointer

	// loop
	bne	a5,t3,5b			// next coeff
	j	8f

// ---- add coefficients in case of negative mantissa

	// negate mantissa A3:A2
6:
//	snez	t2,a2
//	neg	a3,a3
//	neg	a2,a2
//	sub	a3,a3,t2

	not	a2,a2
	not	a3,a3				// use faster NOT instead of NEG

	// mantissa A3:A2 << 1
	slli	a3,a3,1
	srli	t2,a2,31
	or	a3,a3,t2
	slli	a2,a2,1

	// add coeff T1:T0 to acc A1:A0 -> A1:A0
2:	add	a0,a0,t0
	sltu	t0,a0,t0
	add	a1,a1,t1
	add	a1,a1,t0

	// absolute value of the accumulator -> A1:A0
	mv	t2,a1				// T2 <- save acc HIGH, for later check sign
	bgez	t2,1f

//	snez	t0,a0
//	neg	a1,a1
//	neg	a0,a0
//	sub	a1,a1,t0

	not	a0,a0
	not	a1,a1				// use NOT instead of NEG, faster and better results

	// multiply
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
1:	mulu64high a0,a1,a2,a3,t0,t1

	// negate result
	// - no carry from LOW, so we use more common case NOT instead of NEG
	bltz	t2,1f
	not	a0,a0
	not	a1,a1

	// load next constant
1:	lw	t0,0x28(a5)			// T0 <- coeff low
	lw	t1,0x2c(a5)			// T1 <- coeff high
	addi	a5,a5,8				// shift table pointer

	// loop
	bne	a5,t3,2b			// next coeff

	// add last coeff T1:T0 to acc A1:A0 -> A1:A0
8:	add	a0,a0,t0
	sltu	t0,a0,t0
	add	a1,a1,t1
	add	a1,a1,t0

// result mantissa is in 1.63 format, value 0..0.6931 (log(2)) = 0x0000000000000002..0x58B90BFBE8E7B8C2
// should be max. 0x58B90BFBE8E7BCD5 = 6393154322601327829

	// It doesn't happen, but be careful
//	if (acc >= 0x58B90BFBE8E7BCD5) acc = 0x58B90BFBE8E7BCD5-1;
//	if (acc < 0) acc = 0;

// Here is:
//  A1:A0 = result mantissa
//  A4 = signed exponent
//  A5 = pointer to data table
//  A6 = mode 0=log, 1=log2, -1=log10

// ---- log()

	bnez	a6,log_mode2			// skip if not mode log()

	// prepare absolute value of the exponent -> A2
	mv	a2,a4
	bgez	a2,1f
	neg	a2,a2

	// exp2 < 90 (max. 91)
1:	li	t0,90
	bge	a2,t0,2f			// skip if >= 90

	// round mantissa up
	li	t0,0x20+30
	add	a0,a0,t0
	sltu	t0,a0,t0
	add	a1,a1,t0

	// convert mantissa from format 1.63 to format 7.57
	srli	a0,a0,6
	slli	t0,a1,32-6
	or	a0,a0,t0
	srli	a1,a1,6

	// multiply exponent A4 * log(2) and convert to format 7.57 -> A3:A2
	// log(2) << 57 = 99893036290645747
// multiply s64 * s32 -> s64
// aL ... 1st operand s64 LOW
// aH ... 1st operand s64 HIGH
// bL ... 2nd operand s32
// resL ... result s64 LOW (can be same as aL, aH or bL)
// resH ... result s64 HIGH (can be same as aL, aH, bL, tmp1 or tmp2)
// tmp1, tmp2 ... temporary, trashed
//.macro muls64s32 aL, aH, bL, resL, resH, tmp1, tmp2
	li	a2,0xEFA39EF3			// const LOW
	li	a3,0x0162E42F 			// const HIGH
	muls64s32 a2,a3,a4,a2,a3,t0,t1

	// add exponent A3:A2 to mantissa A1:A0
	add	a0,a0,a2
	sltu	t0,a0,a2
	add	a1,a1,a3
	add	a1,a1,t0

	// prepare result exponent base (-1 to clear hidden bit '1') -> A4
	li	a4,DOUBLE_EXP1 - 5 - 1
	j	log_mode_ok

	// exp2 < 368 (max. 369)
2:	li	t0,368
	bge	a2,t0,2f			// skip if >= 368

	// round mantissa up
	li	t0,0x80+400
	add	a0,a0,t0
	sltu	t0,a0,t0
	add	a1,a1,t0

	// convert mantissa from format 1.63 to format 9.55
	srli	a0,a0,8
	slli	t0,a1,32-8
	or	a0,a0,t0
	srli	a1,a1,8

	// multiply exponent * log(2) and convert to format 9.55 -> A3:A2
	// log(2) << 55 = 24973259072661437
//.macro muls64s32 aL, aH, bL, resL, resH, tmp1, tmp2
	li	a2,0xFBE8E7BD			// const LOW
	li	a3,0x0058B90B 			// const HIGH
	muls64s32 a2,a3,a4,a2,a3,t0,t1

	// add exponent A3:A2 to mantissa A1:A0
	add	a0,a0,a2
	sltu	t0,a0,a2
	add	a1,a1,a3
	add	a1,a1,t0

	// prepare result exponent base (-1 to clear hidden bit '1') -> A4
	li	a4,DOUBLE_EXP1 - 3 - 1
	j	log_mode_ok

	// exp2 >= 368
	// round mantissa up
2:	li	t0,0x200
	add	a0,a0,t0
	sltu	t0,a0,t0
	add	a1,a1,t0

	// convert mantissa from format 1.63 to format 11.53
	srli	a0,a0,10
	slli	t0,a1,32-10
	or	a0,a0,t0
	srli	a1,a1,10

	// multiply exponent * log(2) and convert to format 11.53 -> A3:A2
	// log(2) << 53 = 6243314768165359
//.macro muls64s32 aL, aH, bL, resL, resH, tmp1, tmp2
	li	a2,0xFEFA39EF			// const LOW
	li	a3,0x00162E42 			// const HIGH
	muls64s32 a2,a3,a4,a2,a3,t0,t1

	// add exponent A3:A2 to mantissa A1:A0
	add	a0,a0,a2
	sltu	t0,a0,a2
	add	a1,a1,a3
	add	a1,a1,t0

	// prepare result exponent base (-1 to clear hidden bit '1') -> A4
	li	a4,DOUBLE_EXP1 - 1 - 1
	j	log_mode_ok

// ---- log2()

log_mode2:
	bltz	a6,log_mode10			// skip if not mode log2()

	// round mantissa up
	li	t0,354 - 6			// = 0x200 * log(2) = 354 round down
	add	a0,a0,t0
	sltu	t0,a0,t0			// check carry
	add	a1,a1,t0			// add carry

	// multiply 1.63 mantissa * 1/log(2) and convert to format 11.53
	// 1/log(2) << 54 = 25989283394227192
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	li	a2,0x94AE0BF8			// low
	li	a3,0x005C551D			// high
	mulu64high a0,a1,a2,a3,t0,t1

	// add exponent in format 11.53
	slli	a4,a4,21
	add	a1,a1,a4

	// prepare result exponent base (-1 to clear hidden bit '1') -> A4
	li	a4,DOUBLE_EXP1 - 1 - 1
	j	log_mode_ok

// ---- log10()

log_mode10:

	// round mantissa up
	li	t0,589 + 10			// = 0x100 * log(10) = 589 round down
	add	a0,a0,t0
	sltu	t0,a0,t0			// check carry
	add	a1,a1,t0			// add carry

	// multiply 1.63 mantissa * 1/log(10) and convert to format 10.54
	// 1/log(10) << 55 = 15647107734948380 round down
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	li	a2,0x2A4DCA1C			// low
	li	a3,0x003796F6			// high
	mulu64high a0,a1,a2,a3,t0,t1

	// multiply exponent A4 * log(10) and convert to format 10.54 -> A3:A2
	// log(2)/log(10) << 54 = 5422874305198591
// multiply s64 * s32 -> s64
// aL ... 1st operand s64 LOW
// aH ... 1st operand s64 HIGH
// bL ... 2nd operand s32
// resL ... result s64 LOW (can be same as aL, aH or bL)
// resH ... result s64 HIGH (can be same as aL, aH, bL, tmp1 or tmp2)
// tmp1, tmp2 ... temporary, trashed
//.macro muls64s32 aL, aH, bL, resL, resH, tmp1, tmp2
	li	a2,0x509F79FF			// const LOW
	li	a3,0x00134413			// const HIGH
	muls64s32 a2,a3,a4,a2,a3,t0,t1

	// add exponent A3:A2 to mantissa A1:A0
	add	a0,a0,a2
	sltu	t0,a0,a2
	add	a1,a1,a3
	add	a1,a1,t0

	// prepare result exponent base (-1 to clear hidden bit '1') -> A4
	li	a4,DOUBLE_EXP1 - 2 - 1

log_mode_ok:

// Here is:
//  A1:A0 = result mantissa
//  A4 = result exponent base

// ---- resulting

	// zero result
	or	t0,a0,a1		// zero result?
	beqz	t0,9f			// zero result

	// negate mantissa, result sign -> A6
	srli	a6,a1,31		// A6 <- 1 sign
	beqz	a6,1f			// skip if not negative
	snez	t0,a0
	neg	a1,a1
	neg	a0,a0
	sub	a1,a1,t0

	// prepare number of shifts -> T1 -52 (up,left) .. +11 (down,right)
1:	bnez	a1,1f			// skip if mantissa HIGH is not 0
	li	t1,11 - 32		// T1 <- 11 - 32 = -21
	clz	t0,a0			// T0 <- count leading zeroes LOW
	j	2f
1:	clz	t0,a1			// T0 <- count leading zeroes HIGH
	li	t1,11			// T1 <- 11
2:	sub	t1,t1,t0		// T1 <- number of shifts (11-(clz(a0)+32) or 11-clz(a1))

	// prepare result exponent -> A4
	add	a4,a4,t1

	// normalize mantissa (T1 contains number of shifts: -52 (up,left) .. +11 (down,right))
	li	t2,32
	bgtz	t1,3f			// skip if number of shifts > 0 (shift down,right)

	// shift mantissa up,left 0..52
	neg	t1,t1			// T1 <- absolute value of number of shifts left (= 0..52)
	sub	t0,t1,t2		// T0 <- number of shifts - 32 (= -32..+20)
	bgez	t0,2f			// number of shifts is >= 32 (= 32..52, T0=0..+20)
	beqz	t1,8f			// skip if number of shifts = 0

	sub	t0,t2,t1		// T0 <- shift inversion = 32 - shifts = 1..31 (T1 = 1..31)
	sll	a1,a1,t1		// shift mantissa HIGH left by 1..31
	srl	t0,a0,t0		// T0 <- carry from mantissa LOW
	or	a1,a1,t0		// add carry from mantissa LOW to mantissa HIGH
	sll	a0,a0,t1		// shift mantissa LOW left by 1..31
	j	8f

2:	sll	a1,a0,t0		// A1 <- shift mantissa left by 0..20 shifts
	li	a0,0			// A0 <- mantissa LOW = 0
	j	8f

	// shift mantissa down with rounding 1..11 (here is T1 = number of shifts right 1..11)
3:	add	t1,t1,-1		// number of shifts = 0..10
	beqz	t1,1f			// skip number of shifts 0 (number of shifts 32 is not supported)
	sub	t0,t2,t1		// T0 <- shift inversion 32-(1..10) = 22..31
	srl	a0,a0,t1		// shift mantissa LOW right by 1..10
	sll	t0,a1,t0		// T0 <- carry from mantissa HIGH, shift left by 22..31
	or	a0,a0,t0		// add carry from mantissa HIGH to mantissa LOW
	srl	a1,a1,t1		// shift mantissa HIGH right by 1..10

1:	addi	t0,a0,1			// increment mantissa, to round up
	sltu	t1,t0,a0		// carry from LOW
	add	a1,a1,t1		// add carry to HIGH

	srli	a0,t0,1			// shift mantissa >> 1 LOW
	slli	t0,a1,31		// carry from high
	or	a0,a0,t0		// add carry from HIGH to LOW
	srli	a1,a1,1			// shift mantissa >> 1 HIGH

	// carry on rounding
	srli	t0,a1,21		// check carry on rounding
	beqz	t0,8f			// no carry

	srli	a0,a0,1			// shift mantissa >> 1 LOW
	slli	t0,a1,31		// carry from high
	or	a0,a0,t0		// add carry from HIGH to LOW
	srli	a1,a1,1			// shift mantissa >> 1 HIGH
	addi	a4,a4,1			// shift exponent

	// compose result
8:	slli	a4,a4,20		// shift exponent to position
	add	a1,a1,a4		// add exponent to result, clear hidden bit '1'
	slli	a6,a6,31		// A6 <- shift sign to position
	or	a1,a1,a6		// add sign to result
9:	ret

// ---- result is 0 if number is 1 (here is A0 = 0)

log_res0:
	li	t0,0x3ff00000		// number 1.0 high
	bne	a1,t0,log_not1		// jump back if not 1.0
	li	a1,0			// result is 0
	ret

// ---- zero returns -inf

log_minf:
	li	a0,0			// -inf low
	li	a1,0xfff00000		// -inf high
	ret

// ---- negative number and positive infinity returns +inf

log_pinf:
	li	a0,0			// +inf low
	li	a1,0x7ff00000		// +inf high
	ret

double_section __logtab
.p2align 2,,

_logtab:
	.word	-577237				// 0x00: coeff[26] = -3.1292075768665710843e-014 << 64
	.word	898650				// 0x04: coeff[25] =  9.7431876039391340245e-014 << 63
	.word	-853942				// 0x08: coeff[24] = -9.2584594371947089050e-014 << 63
	.word	2685192				// 0x0C: coeff[23] =  2.9112908581834786416e-013 << 63
	.word	-14306578			// 0x10: coeff[22] = -1.5511222530986678301e-012 << 63
	.word	44928986			// 0x14: coeff[21] =  4.8712103978938373001e-012 << 63
	.word	-131141063			// 0x18: coeff[20] = -1.4218342495132474418e-011 << 63
	.word	414187698			// 0x1C: coeff[19] =  4.4906320218786305240e-011 << 63
	.word	-1323511003			// 0x20: coeff[18] = -1.4349535043305586936e-010 << 63
	.word	4204030849			// 0x24: coeff[17] =  4.5580193793557624035e-010 << 63

	.word	0xE1D530DB, 0xFFFFFFFC		// 0x28: coeff[16] = -1.4518577213667552372e-009 << 63
	.word	0xFA234AFE, 0x00000009		// 0x30: coeff[15] =  4.6459497081175274052e-009 << 63
	.word	0xEDF7F9AF, 0xFFFFFFDF		// 0x38: coeff[14] = -1.4933960021895990116e-008 << 63
	.word	0x9CB71C18, 0x00000067		// 0x40: coeff[13] =  4.8248176012585914163e-008 << 63
	.word	0x42CA428D, 0xFFFFFEAF		// 0x48: coeff[12] = -1.5680636337856674805e-007 << 63
	.word	0x0E245CDE, 0x0000044E		// 0x50: coeff[11] =  5.1318446283779635237e-007 << 63
	.word	0x37B38D0C, 0xFFFFF1CB		// 0x58: coeff[10] = -1.6935087817346793673e-006 << 63
	.word	0x9BA977DC, 0x00002F5A		// 0x60: coeff[9]  =  5.6450292722459159508e-006 << 63
	.word	0x32A55F9E, 0xFFFF602E		// 0x68: coeff[8]  = -1.9051973784389208243e-005 << 63
	.word	0xE4A47020, 0x000223F3		// 0x70: coeff[7]  =  6.5321052975079351008e-005 << 63
	.word	0x5FC05234, 0xFFF8822A		// 0x78: coeff[6]  = -2.2862368541381454891e-004 << 63
	.word	0x40E53E5F, 0x001AF834		// 0x80: coeff[5]  =  8.2304526748972956870e-004 << 63
	.word	0x0CA45880, 0xFF9ADD3C		// 0x88: coeff[4]  = -3.0864197530864195951e-003 << 63
	.word	0xCD6E9E02, 0x01948B0F		// 0x90: coeff[3]  =  1.2345679012345678522e-002 << 63
	.word	0xE38E38E4, 0xF8E38E38		// 0x98: coeff[2]  = -5.5555555555555555558e-002 << 63
	.word	0xAAAAAAAB, 0x2AAAAAAA		// 0xA0: coeff[1]  =  3.3333333333333333334e-001 << 63
	.word	0x7F3097DB, 0x33E647D9		// 0xA8: coeff[0]  =  4.0546510810816438199e-001 << 63
_logtab_end:

// ----------------------------------------------------------------------------
//                      sin(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// double sin(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A7,T0..T4 (+ registers from remainder function)

// local stack:
#define SIN_STACK_SIZE	32	// stack size
// SP+0: (4) original return address
// SP+4: (4) internal return address
// SP+8: (4) saved S2 (used as sin sign)
// SP+12: (4) saved S3 (used as cos sign)
// SP+16: (4) saved pointer to psin
// SP+20: (4) saved pointer to pcos
// SP+24: (8) saved sin() result

double_section _sin
.p2align 2,,

.global __wrap_sin
.global sin
__wrap_sin:
sin:
	// get unsigned exponent -> A4
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position

	// hack small angles (if exp < -6)
	li	t0,DOUBLE_EXP1-6
	blt	a4,t0,_asm_small_sin	// go to sin() of small angle

	// push registers
	addi	sp,sp,-SIN_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..pi/2 and convert to fixed point format 1.63, range = -1..+1 = 0..pi/2
// INPUT: A1:A0 = argument x
// OUTPUT: A3:A2 = mantissa in fixed point format 1.63, range = -1..+1 = 0..pi/2
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
// TRASHED: A0,A1,A4, T0-T2 (+ registers from remainder function)
	call	_asm_sin_norm

	// Chebyshev approximation
// INPUT: A3:A2 = mantissa in signed fixed point format 1.63, range = -1..+1 = 0..pi/2
//	  A4 = sign 0 or -1
// OUTPUT: A1:A0 = result
// TRASHED: A2-A5, T0-T3
	mv	a4,s2			// A4 <- sin sign
	call	_asm_sin_cheb

sin_return:
	// pop registers
	lw	s3,12(sp)		// restore S3
	lw	s2,8(sp)		// restore S2
	lw	ra,0(sp)		// restore original return address
	addi	sp,sp,SIN_STACK_SIZE	// destroy space in the stack
	ret

// ----------------------------------------------------------------------------
//                      sin_deg(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// double sin_deg(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6,T0..T2 (+ registers from remainder function)

.global sin_deg
sin_deg:
	// get unsigned exponent -> A4
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position

	// hack small angles (if exp < -2)
	li	t0,DOUBLE_EXP1-2
	blt	a4,t0,_asm_small_sin_deg // go to sin() of small angle

	// push registers
	addi	sp,sp,-SIN_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..90 and convert to fixed point format 1.63, range = -1..+1 = 0..pi/2
// INPUT: A1:A0 = argument x
// OUTPUT: A3:A2 = mantissa in fixed point format 1.63, range = -1..+1 = 0..pi/2
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
// TRASHED: A0,A1,A4, T0-T2 (+ registers from remainder function)
	call	_asm_sin_deg_norm

	// Chebyshev approximation
// INPUT: A3:A2 = mantissa in signed fixed point format 1.63, range = -1..+1 = 0..pi/2
//	  A4 = sign 0 or -1
// OUTPUT: A1:A0 = result
// TRASHED: A2-A5, T0-T3
	mv	a4,s2			// A4 <- sin sign
	call	_asm_sin_cheb
	j	sin_return


// ==== sin() of small angle (exps < -6)
// - Simplified Taylor serie is used: sin(x) = x/1! - x^3/3! + x^5/5! - ...

// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A7,T0..T4 (+ undefined registers from functions)

_asm_small_sin_deg:

	// push registers
	addi	sp,sp,-8		// create space in the stack
	sw	ra,0(sp)		// save original return address

	// convert deg to rad (x * pi/180 = x * 0.01745329251994329577)
	li	a3,0x3F91DF46
	li	a2,0xA2529D39
	call	dmul

	// pop registers
	lw	ra,0(sp)		// restore original return address
	addi	sp,sp,8			// destroy space in the stack

_asm_small_sin:

// ----- small sin() - faster version (takes 428 bytes)

#if 1			// set 1=use faster version 428 bytes, 0=use slower version 120 bytes

	// get signed exponent -> A4 (exp < -6)
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position
	addi	a4,a4,-DOUBLE_EXP1

	// use 1 member of the serie = 'x' (if exp < -26)
	//  exp will be = -26 .. -7
	li	t0,-26
	blt	a4,t0,9f		// use 1 member of the serie (= x) if exps < -26

	// get sign -> A5
	srli	a5,a1,31		// A5 <- sign 0 or 1

	// unpack mantissa to format 2.62, add hidden bit '1' -> A1:A0
	// - mantissa will be in range 0x4000000000000000 .. 0x7FFFFFFFFFFFFFFF
	slli	a1,a1,11		// shift to clear exponent
	bseti	a1,a1,31		// set hidden bit '1'
	srli	a1,a1,1			// shift mantissa HIGH to position 2.62
	srli	t0,a0,32 - 10		// T0 <- carry from mantissa LOW
	or	a1,a1,t0		// add carry from LOW to HIGH
	slli	a0,a0,10		// shift mantissa LOW to position 2.62

	// multiply x*x = x^2 -> A7:A6
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// resL . result LOW (can be equal to aL)
// resH . result HIGH (can be equal to aH)
// aL ... 1st operand LOW
// aH ... 1st operand HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64highto resL, resH, aL, aH, bL, bH, tmp1, tmp2
	mulu64highto a6,a7,a0,a1,a0,a1,t0,t1

	// multiply x*x*x = x^3 -> T3:T2
	mulu64highto t2,t3,a6,a7,a0,a1,t0,t1

	// multiply member x^3 by 1/3! -> T3:T2
	// 1/3! = 0.1666666666666666667 << 64 = 3074457345618258603 = 0x2AAAAAAAAAAAAAAB
	li	a3,0x2AAAAAAA
	li	a2,0xAAAAAAAB
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high t2,t3,a2,a3,t0,t1

	// prepare number of shifts = exp*2 - 2*2 -> T4, shift will be 10..48 (exp is 7..26)
	neg	t4,a4			// negate exponent
	add	t4,t4,t4		// T4 <- exponent*2
	addi	t4,t4,-2*2		// T4 <- exponent*2 - 2*2

	// shift member T3:T2 right by T4 = 10..48 -> A3:A2
	addi	t0,t4,-32		// T0 <- number of shifts - 32
	bgez	t0,2f			// number of shifts is >= 32
	neg	t0,t0			// T0 <- 32 - number of shifts
	sll	t0,t3,t0		// T0 <- carry from HIGH
	srl	a2,t2,t4		// A2 <- shifted member LOW
	or	a2,a2,t0		// add carry
	srl	a3,t3,t4		// A3 <- shifted member HIGH
	j	4f

2:	srl	a2,t3,t0		// A2 <- shift from HIGH to LOW
	li	a3,0

	// subtract shifted member A3:A2 (= x^3/3!) from accumulator A1:A0
4:	mv	t0,a0			// T0 <- save accumulator LOW
	sub	a0,a0,a2
	sltu	t0,t0,a0
	sub	a1,a1,a3
	sub	a1,a1,t0

	// use 3rd member x^5/5! if exp >= -13
	//  exp will be = 7..13
	li	t0,-13
	blt	a4,t0,7f		// skip 3rd member if exp < -13

	// multiply x^2 * x^3 = x^5 -> T3:T2
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high t2,t3,a6,a7,t0,t1

	// multiply member x^5 by 1/(4*5) -> T3:T2
	// 1/(4*5) = 0.05 << 64 = 922337203685477581 = 0x0CCCCCCCCCCCCCCD
	li	a3,0x0CCCCCCC
	li	a2,0xCCCCCCCD
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high t2,t3,a2,a3,t0,t1

	// prepare number of shifts = exp*4 - 4*2 -> T4, shift will be 20..44 (exp is 7..13)
	slli	t4,t4,1			// T4 <- 2*(exp*2 - 2*2) = exp*4 - 4*2

	// shift member T3:T2 right by T4 = 20..44 -> A3:A2
	addi	t0,t4,-32		// T0 <- number of shifts - 32
	bgez	t0,2f			// number of shifts is >= 32
	neg	t0,t0			// T0 <- 32 - number of shifts
	sll	t0,t3,t0		// T0 <- carry from HIGH
	srl	a2,t2,t4		// A2 <- shifted member LOW
	srl	a3,t3,t4		// A3 <- shifted member HIGH
	or	a2,a2,t0		// add carry
	j	4f

2:	srl	a2,t3,t0		// A2 <- shift from HIGH to LOW
	li	a3,0

	// add shifted member A3:A2 (= x^5/5!) to accumulator A1:A0
4:	add	a0,a0,a2
	sltu	t0,a0,a2
	add	a1,a1,a3
	add	a1,a1,t0

	// normalize mantissa A1:A0 >> 8, if mantissa is < 0x4000000000000000, or A1:A0 >> 9 otherwise
7:	srli	t0,a1,30		// check if mantissa is less
	bnez	t0,8f			// mantissa is >= 0x4000000000000000
	slli	t0,a1,32 - 8		// T0 <- carry from HIGH
	srli	a1,a1,8
	srli	a0,a0,8
	or	a0,a0,t0
	add	a4,a4,-1		// exp - 1
	j	4f

8:	slli	t0,a1,32 - 9		// T0 <- carry from HIGH
	srli	a1,a1,9
	srli	a0,a0,9
	or	a0,a0,t0

	// add 1 for rounding
4:	addi	a0,a0,1
	bnez	a0,5f
	addi	a1,a1,1

	// >> 1
5:	slli	t0,a1,32 - 1		// T0 <- carry from HIGH
	srli	a1,a1,1
	srli	a0,a0,1
	or	a0,a0,t0

	// carry on rounding
	srli	t0,a1,21		// check overflow
	beqz	t0,6f			// no overflow

	// >> 1
	slli	t0,a1,32 - 1		// T0 <- carry from HIGH
	srli	a1,a1,1
	srli	a0,a0,1
	or	a0,a0,t0
	addi	a4,a4,1			// exp + 1

	// compose result ('exponent - 1' to compensate hidden bit '1')
6:	addi	a4,a4,DOUBLE_EXP1-1
	slli	a4,a4,20
	add	a1,a1,a4
	slli	a5,a5,31
	or	a1,a1,a5
9:	ret

#else

// ----- small sin() - slower version (takes 120 bytes)
// - Simplified Taylor serie is used: sin(x) = x/1! - x^3/3! + x^5/5! - ...
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A7,T0..T4

// SP+0: (4) original return address
// SP+4: (4) saved exponent
// SP+8: (8) saved sin() result
// SP+16: (8) saved x member of Taylor serie
// SP+24: (8) saved original x
// SP+32: (8) saved x*x

	// get unsigned exponent -> A4 (exp < -6)
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position

	// exps < -26 requires 1 Taylor member
	li	t0,DOUBLE_EXP1-26
	blt	a4,t0,9f		// small number exps < -26

	// push registers
	addi	sp,sp,-40		// create space in the stack
	sw	ra,0(sp)		// save original return address

	// exps < -12 requires 2 Taylor members
	// - SP+4: (4) saved exponent
	sw	a4,4(sp)		// save exponent

	// x/1!
	// - SP+24: (8) saved original x
	sw	a0,24(sp)		// save x LOW
	sw	a1,28(sp)		// save x HIGH

	// x*x
	mv	a2,a0
	mv	a3,a1
	call	dmul			// x*x
	// - SP+32: (8) saved x*x
	sw	a0,32(sp)		// save x*x LOW	
	sw	a1,36(sp)		// save x*x HIGH

	// x*x*x
	// - SP+24: (8) saved original x
	lw	a2,24(sp)		// x LOW
	lw	a3,28(sp)		// x HIGH
	call	dmul			// x*x*x

	// -x*x*x/3!
	li	a3,0xbfc55555		// -1/3! = -1/6 = -0.16666666666666665
	li	a2,0x55555555
	call	dmul			// -x*x*x/3!
	// - SP+16: (8) saved x member of Taylor serie
	sw	a0,16(sp)		// save x member LOW
	sw	a1,20(sp)		// save x member HIGH

	// x/1! - x^3/3!
	// - SP+24: (8) saved original x
	lw	a2,24(sp)		// get x LOW
	lw	a3,28(sp)		// get x HIGH
	call	dadd			// result x/1! - x^3/3!

	// check exps < -12
	li	t0,DOUBLE_EXP1-12
	// - SP+4: (4) saved exponent
	lw	a4,4(sp)		// restore exponent
	blt	a4,t0,8f		// small number exps < -12

	// exps < -7 requires 3 Taylor members
	// - SP+8: (8) saved sin() result
	sw	a0,8(sp)		// save result LOW
	sw	a1,12(sp)		// save result HIGH

	// -x^5/3!
	// - SP+16: (8) saved x member of Taylor serie
	// - SP+32: (8) saved x*x
	lw	a0,16(sp)		// restore x member LOW
	lw	a1,20(sp)		// restore x member HIGH
	lw	a2,32(sp)		// restore x*x LOW	
	lw	a3,36(sp)		// restore x*x HIGH
	call	dmul			// -x^5/3!

	// +x^5/5!
	li	a3,0xbfa99999		// -1/(4*5) = -1/20 = -0.05
	li	a2,0x9999999a
	call	dmul			// +x^5/5!

	// x/1! - x^3/3! + x^5/5!
	// - SP+8: (8) saved sin() result
	lw	a2,8(sp)		// get result LOW
	lw	a3,12(sp)		// get result HIGH
	call	dadd			// result x/1! - x^3/3! + x^5/5!

8:	lw	ra,0(sp)		// restore original return address
	addi	sp,sp,40		// destroy space in the stack
9:	ret

#endif

// ----------------------------------------------------------------------------
//                      cos(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// double cos(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6,T0..T2 (+ registers from remainder function)

double_section _cos
.p2align 2,,

.global __wrap_cos
.global cos
__wrap_cos:
cos:
	// get unsigned exponent -> A4
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position

	// hack small angles (if exp < -14)
	li	t0,DOUBLE_EXP1-14
	blt	a4,t0,_asm_small_cos	// go to cos() of small angle

	// push registers
	addi	sp,sp,-SIN_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..pi/2 and convert to fixed point format 1.63, range = -1..+1 = 0..pi/2
// INPUT: A1:A0 = argument x
// OUTPUT: A3:A2 = mantissa in fixed point format 1.63, range = -1..+1 = 0..pi/2
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
// TRASHED: A0,A1,A4, T0-T2 (+ registers from remainder function)
	call	_asm_sin_norm

	// negate mantissa
	snez	t0,a2
	neg	a3,a3
	neg	a2,a2
	sub	a3,a3,t0

	// Chebyshev approximation
// INPUT: A3:A2 = mantissa in signed fixed point format 1.63, range = -1..+1 = 0..pi/2
//	  A4 = sign 0 or -1
// OUTPUT: A1:A0 = result
// TRASHED: A2-A5, T0-T3
	mv	a4,s3			// A4 <- cos sign
	call	_asm_sin_cheb
	j	sin_return

// ==== cos() of small angle (exps < -14)
// - Simplified Taylor serie is used: cos(x) = 1/1! - x^2/2! + ...

// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: undefined

_asm_small_cos:

// SP+0: (4) original return address

	// get unsigned exponent -> A4 (exp < -6)
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position

	// exps < -27 requires 1 Taylor member
	li	t0,DOUBLE_EXP1-27
	blt	a4,t0,9f		// small number exps < -27, retur 1.0

	// push registers
	addi	sp,sp,-8		// create space in the stack
	sw	ra,0(sp)		// save original return address

	// x*x
	mv	a2,a0
	mv	a3,a1
	call	dmul			// x*x

	// x^2/2 ... subtract 1 from the exponent (cannot underflow here)
	li	t0,1<<20
	sub	a1,a1,t0

	// negate
	binvi	a1,a1,31		// invert sign

	// add 1.0
	li	a3,0x3ff00000
	li	a2,0
	call	dadd			// add 1.0 - x^2/2

	// restore registers
	lw	ra,0(sp)		// restore original return address
	addi	sp,sp,8			// destroy space in the stack
	ret

9:	li	a1,0x3ff00000		// return 1.0
	li	a0,0
	ret

// ----------------------------------------------------------------------------
//                      sincos(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// void sincos(double x, double* psin, double* pcos);
// INPUT: A1:A0 = argument X
// 	  A2 = pointer to sin() result
// 	  A3 = pointer to cos() result
// OUTPUT: -
// TRASHED: A0..A6,T0..T2 (+ registers from remainder function)

double_section _sincos
.p2align 2,,

.global __wrap_sincos
.global sincos
__wrap_sincos:
sincos:
	// push registers
	addi	sp,sp,-SIN_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address
	sw	a2,16(sp)		// save pointer to sin() result
	sw	a3,20(sp)		// save pointer to cos() result

	// get unsigned exponent -> A4
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position

	// hack small angles (if exp < -6)
	li	t0,DOUBLE_EXP1-6
	bge	a4,t0,5f		// not small angle

	// save original argument X
	sw	a0,24(sp)		// save original argument x LOW
	sw	a1,28(sp)		// save original argument x HIGH

	// calculate sin()
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A7,T0..T4 (+ undefined registers from functions)
	call	_asm_small_sin

	// save result sin()
	lw	a2,16(sp)		// A2 <- pointer to sin() result
	sw	a0,0(a2)		// save result sin() LOW
	sw	a1,4(a2)		// save result sin() HIGH

	// load original argument X
	lw	a0,24(sp)		// load original argument x LOW
	lw	a1,28(sp)		// load original argument x HIGH

	// calculate cos()
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6,T0..T2 (+ registers from remainder function)
	call	cos

	// save result cos()
	lw	a2,20(sp)		// A2 <- pointer to cos() result
	sw	a0,0(a2)		// save result cos() LOW
	sw	a1,4(a2)		// save result cos() HIGH

	// restore registers
	lw	ra,0(sp)		// restore original return address
	addi	sp,sp,SIN_STACK_SIZE	// destroy space in the stack
	ret

	// normalize angle to range 0..pi/2 and convert to fixed point format 1.63, range = -1..+1 = 0..pi/2
// INPUT: A1:A0 = argument x
// OUTPUT: A3:A2 = mantissa in fixed point format 1.63, range = -1..+1 = 0..pi/2
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
// TRASHED: A0,A1,A4, T0-T2 (+ registers from remainder function)
5:	call	_asm_sin_norm

	// save normalized value
	sw	a2,24(sp)		// save argument x LOW
	sw	a3,28(sp)		// save argument x HIGH

	// Chebyshev approximation - sin()
// INPUT: A3:A2 = mantissa in signed fixed point format 1.63, range = -1..+1 = 0..pi/2
//	  A4 = sign 0 or -1
// OUTPUT: A1:A0 = result
// TRASHED: A2-A5, T0-T3
	mv	a4,s2			// A4 <- sin sign
	call	_asm_sin_cheb

	// save result sin()
	lw	a2,16(sp)		// A2 <- pointer to sin() result
	sw	a0,0(a2)		// save result sin() LOW
	sw	a1,4(a2)		// save result sin() HIGH

	// negate argument
	lw	a2,24(sp)		// load argument x LOW
	lw	a3,28(sp)		// load argument x HIGH
	snez	t0,a2
	neg	a3,a3
	neg	a2,a2
	sub	a3,a3,t0

	// Chebyshev approximation - cos()
// INPUT: A3:A2 = mantissa in signed fixed point format 1.63, range = -1..+1 = 0..pi/2
//	  A4 = sign 0 or -1
// OUTPUT: A1:A0 = result
// TRASHED: A2-A5, T0-T3
	mv	a4,s3			// A4 <- cos sign
	call	_asm_sin_cheb

	// save result cos()
	lw	a2,20(sp)		// A2 <- pointer to cos() result
	sw	a0,0(a2)		// save result cos() LOW
	sw	a1,4(a2)		// save result cos() HIGH
	j	sin_return

// ----------------------------------------------------------------------------
//                      tan(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// double tan(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6,T0..T2 (+ registers from remainder function)

double_section _tan
.p2align 2,,

.global __wrap_tan
.global tan
__wrap_tan:
tan:
	// push registers
	addi	sp,sp,-24		// create space in the stack
	sw	ra,16(sp)		// save original return address

	// calculate sin(), cos()
// void sincos(double x, double* psin, double* pcos);
// INPUT: A1:A0 = argument X
// 	  A2 = pointer to sin() result
// 	  A3 = pointer to cos() result
// OUTPUT: -
// TRASHED: A0..A6,T0..T2 (+ registers from remainder function)
	mv	a2,sp			// A2 <- pointer to sin() result
	addi	a3,a2,8			// A3 <- pointer to cos() result
_tan_cotan_do:
	call	sincos

	// divide sin/cos, pop registers
	lw	a0,0(sp)		// A0 <- sin() LOW
	lw	a1,4(sp)		// A1 <- sin() HIGH
	lw	a2,8(sp)		// A2 <- cos() LOW
	lw	a3,12(sp)		// A3 <- cos() HIGH
	lw	ra,16(sp)		// restore original return address
	add	sp,sp,24		// destroy space in the stack
	j	ddiv

// ----------------------------------------------------------------------------
//                      cotan(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// double cotan(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6,T0..T2 (+ registers from remainder function)

double_section _cotan
.p2align 2,,

.global __wrap_cotan
.global cotan
__wrap_cotan:
cotan:
	// push registers
	addi	sp,sp,-24		// create space in the stack
	sw	ra,16(sp)		// save original return address

	// calculate sin(), cos()
	mv	a3,sp			// A3 <- pointer to cos() result
	addi	a2,a3,8			// A2 <- pointer to sin() result
	j	_tan_cotan_do

// ----------------------------------------------------------------------------
//                      cos_deg(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// double cos_deg(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6,T0..T2 (+ registers from remainder function)

double_section _cos_deg
.p2align 2,,

.global cos_deg
cos_deg:
	// get unsigned exponent -> A4
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position

	// hack small angles (if exp < -10)
	li	t0,DOUBLE_EXP1-10
	blt	a4,t0,_asm_small_cos_deg // go to cos_deg() of small angle

	// push registers
	addi	sp,sp,-SIN_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address

	// normalize angle to range 0..90 and convert to fixed point format 1.63, range = -1..+1 = 0..pi/2
// INPUT: A1:A0 = argument x
// OUTPUT: A3:A2 = mantissa in fixed point format 1.63, range = -1..+1 = 0..pi/2
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
// TRASHED: A0,A1,A4, T0-T2 (+ registers from remainder function)
	call	_asm_sin_deg_norm

	// negate mantissa
	snez	t0,a2
	neg	a3,a3
	neg	a2,a2
	sub	a3,a3,t0

	// Chebyshev approximation
// INPUT: A3:A2 = mantissa in signed fixed point format 1.63, range = -1..+1 = 0..pi/2
//	  A4 = sign 0 or -1
// OUTPUT: A1:A0 = result
// TRASHED: A2-A5, T0-T3
	mv	a4,s3			// A4 <- cos sign
	call	_asm_sin_cheb
	j	sin_return

// ==== cos() of small angle (exps < -10)
// - Simplified Taylor serie is used: cos(x) = 1/1! - x^2/2! + ...

// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: undefined

_asm_small_cos_deg:

// SP+0: (4) original return address

	// get unsigned exponent -> A4 (exp < -10)
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position

	// exps < -21 requires 1 Taylor member
	li	t0,DOUBLE_EXP1-21
	blt	a4,t0,9f		// small number exps < -27, retur 1.0

	// push registers
	addi	sp,sp,-8		// create space in the stack
	sw	ra,0(sp)		// save original return address

	// x*x
	mv	a2,a0
	mv	a3,a1
	call	dmul			// x*x

	// divide /2! and convert deg to rad = -(pi/180)^2/2 = -1.523087098933542997e-4
	li	a3,0xbf23f6a1
	li	a2,0xdb141fba
	call	dmul			// -x*x/2! in rad

	// add 1.0
	li	a3,0x3ff00000
	li	a2,0
	call	dadd			// add 1.0 - x^2/2

	// restore registers
	lw	ra,0(sp)		// restore original return address
	addi	sp,sp,8			// destroy space in the stack
	ret

9:	li	a1,0x3ff00000		// return 1.0
	li	a0,0
	ret

// ----------------------------------------------------------------------------
//                      sincos_deg(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// void sincos_deg(double x, double* psin, double* pcos);
// INPUT: A1:A0 = argument X
// 	  A1 = pointer to sin() result
// 	  A2 = pointer to cos() result
// OUTPUT: -
// TRASHED: A0..A6,T0..T2 (+ registers from remainder function)

double_section _sincos_deg
.p2align 2,,

.global sincos_deg
sincos_deg:
	// push registers
	addi	sp,sp,-SIN_STACK_SIZE	// create space in the stack
	sw	ra,0(sp)		// save original return address
	sw	a2,16(sp)		// save pointer to sin() result
	sw	a3,20(sp)		// save pointer to cos() result

	// get unsigned exponent -> A4
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position

	// hack small angles (if exp < -2)
	li	t0,DOUBLE_EXP1-2
	bge	a4,t0,5f		// not small angle

	// save original argument X
	sw	a0,24(sp)		// save original argument x LOW
	sw	a1,28(sp)		// save original argument x HIGH

	// calculate sin_deg()
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A7,T0..T4 (+ undefined registers from functions)
	call	_asm_small_sin_deg

	// save result sin()
	lw	a2,16(sp)		// A2 <- pointer to sin() result
	sw	a0,0(a2)		// save result sin() LOW
	sw	a1,4(a2)		// save result sin() HIGH

	// load original argument X
	lw	a0,24(sp)		// load original argument x LOW
	lw	a1,28(sp)		// load original argument x HIGH

	// calculate cos()
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6,T0..T2 (+ registers from remainder function)
	call	cos_deg

	// save result cos()
	lw	a2,20(sp)		// A2 <- pointer to cos() result
	sw	a0,0(a2)		// save result cos() LOW
	sw	a1,4(a2)		// save result cos() HIGH

	// restore registers
	lw	ra,0(sp)		// restore original return address
	addi	sp,sp,SIN_STACK_SIZE	// destroy space in the stack
	ret

	// normalize angle to range 0..90 and convert to fixed point format 1.63, range = -1..+1 = 0..pi/2
// INPUT: A1:A0 = argument x
// OUTPUT: A3:A2 = mantissa in fixed point format 1.63, range = -1..+1 = 0..pi/2
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
// TRASHED: A0,A1,A4, T0-T2 (+ registers from remainder function)
5:	call	_asm_sin_deg_norm

	// save normalized value
	sw	a2,24(sp)		// save argument x LOW
	sw	a3,28(sp)		// save argument x HIGH

	// Chebyshev approximation - sin()
// INPUT: A3:A2 = mantissa in signed fixed point format 1.63, range = -1..+1 = 0..pi/2
//	  A4 = sign 0 or -1
// OUTPUT: A1:A0 = result
// TRASHED: A2-A5, T0-T3
	mv	a4,s2			// A4 <- sin sign
	call	_asm_sin_cheb

	// save result sin()
	lw	a2,16(sp)		// A2 <- pointer to sin() result
	sw	a0,0(a2)		// save result sin() LOW
	sw	a1,4(a2)		// save result sin() HIGH

	// negate argument
	lw	a2,24(sp)		// load argument x LOW
	lw	a3,28(sp)		// load argument x HIGH
	snez	t0,a2
	neg	a3,a3
	neg	a2,a2
	sub	a3,a3,t0

	// Chebyshev approximation - cos()
// INPUT: A3:A2 = mantissa in signed fixed point format 1.63, range = -1..+1 = 0..pi/2
//	  A4 = sign 0 or -1
// OUTPUT: A1:A0 = result
// TRASHED: A2-A5, T0-T3
	mv	a4,s3			// A4 <- cos sign
	call	_asm_sin_cheb

	// save result cos()
	lw	a2,20(sp)		// A2 <- pointer to cos() result
	sw	a0,0(a2)		// save result cos() LOW
	sw	a1,4(a2)		// save result cos() HIGH
	j	sin_return

// ----------------------------------------------------------------------------
//                      tan_deg(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// double tan_deg(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6,T0..T2 (+ registers from remainder function)

double_section _tan_deg
.p2align 2,,

.global tan_deg
tan_deg:
	// push registers
	addi	sp,sp,-24		// create space in the stack
	sw	ra,16(sp)		// save original return address

	// calculate sin(), cos()
// void sincos(double x, double* psin, double* pcos);
// INPUT: A1:A0 = argument X
// 	  A2 = pointer to sin() result
// 	  A3 = pointer to cos() result
// OUTPUT: -
// TRASHED: A0..A6,T0..T2 (+ registers from remainder function)
	mv	a2,sp			// A2 <- pointer to sin() result
	addi	a3,a2,8			// A3 <- pointer to cos() result
_tan_cotan_deg_do:
	call	sincos_deg

	// divide sin/cos, pop registers
	lw	a0,0(sp)		// A0 <- sin() LOW
	lw	a1,4(sp)		// A1 <- sin() HIGH
	lw	a2,8(sp)		// A2 <- cos() LOW
	lw	a3,12(sp)		// A3 <- cos() HIGH
	lw	ra,16(sp)		// restore original return address
	add	sp,sp,24		// destroy space in the stack
	j	ddiv

// ----------------------------------------------------------------------------
//                      cotan_deg(x) - Chebyshev approximation
// ----------------------------------------------------------------------------
// double cotan_deg(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A6,T0..T2 (+ registers from remainder function)

double_section _cotan_deg
.p2align 2,,

.global cotan_deg
cotan_deg:
	// push registers
	addi	sp,sp,-24		// create space in the stack
	sw	ra,16(sp)		// save original return address

	// calculate sin(), cos()
	mv	a3,sp			// A3 <- pointer to cos() result
	addi	a2,a3,8			// A2 <- pointer to sin() result
	j	_tan_cotan_deg_do

// ==== normalize angle to range 0..pi/2 and convert to fixed point format 1.63, range = -1..+1 = 0..pi/2
// INPUT: A1:A0 = argument x
// OUTPUT: A3:A2 = mantissa in fixed point format 1.63, range = -1..+1 = 0..pi/2
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
// TRASHED: A0,A1,A4, T0-T2 (+ registers from remainder function)

double_section _asm_sin_norm
.p2align 2,,

_asm_sin_norm:
	// push registers
	sw	s2,8(sp)		// save S2
	sw	s3,12(sp)		// save S3

	// get unsigned exponent -> A4
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position

	// normalize big angle >= 128.0 (= 20*(2*pi))
	li	t0,DOUBLE_EXP1+7	// angle limit 2^7 = minimum 128.0 (= 20*(2*pi))
	bltu	a4,t0,1f		// not big angle

	// normalize using remainder, modulo 2*pi (remainder returns angle in range -pi .. +pi)
	sw	ra,4(sp)		// save internal return address
	li	a2,0x54442D18		// A2 <- 2*pi LOW
	li	a3,0x401921FB		// A3 <- 2*pi HIGH
	call	remainder		// normalize angle to range -pi..+pi
	lw	ra,4(sp)		// restore internal return address

	// get unsigned exponent -> A4
	slli	a4,a1,1			// shift 1 left to delete sign bit
	srli	a4,a4,21		// shift exponent to base position

	// get signed exponent -> A4
	// - signed exponent here is max. +6
1:	addi	a4,a4,-DOUBLE_EXP1

	// unpack mantissa to format 12.52 -> A3:A0
	slli	a3,a1,12		// destroy exponent with sign
	srli	a3,a3,12		// return mantissa to base position
	bset	a3,a3,20		// set hidden bit '1'

	// negate mantissa A3:A0
	bgez	a1,1f			// mantissa is not negative
	snez	t0,a0
	neg	a3,a3
	neg	a0,a0
	sub	a3,a3,t0

	// prepare number of shifts, to convert mantissa to signed format 8.56 -> T0
1:	addi	t0,a4,56-52		// exps <= +6, number of shifts <= +10

	// convert to signed fixed point 8.56 -> A3:A0
	li	t2,32
	bgez	t0,2f			// number of shifts >= 0

	// - right shift A3:A0 by -T0
	neg	t0,t0			// T0 <- negate number of shifts
	sub	t1,t0,t2		// T1 <- number of shifts - 32
	bgez	t1,1f			// number of shifts is >= 32

	sub	t1,t2,t0		// T1 <- 32 - shifts
	sll	t1,a3,t1		// T1 <- carry from HIGH
	srl	a0,a0,t0		// low >>
	or	a0,a0,t1		// add carry from high
	sra	a3,a3,t0		// high >>
	j	3f

1:	bge	t1,t2,11f		// number of shifts >= 64
	sra	a0,a3,t1		// A0 <- shift mantissa right
	srai	a3,a3,31		// A3 <- mantissa HIGH sign
	j	3f

11:	li	a3,0			// A3 <- mantissa HIGH = 0
	li	a0,0			// A0 <- mantissa LOW = 0
	j	3f

	// - left shift A3:A0 by T0, number of shifts = 0..10
2:	beqz	t0,3f			// no shifts
	sub	t1,t2,t0		// T1 <- 32 - shifts
	srl	t1,a0,t1		// T1 <- carry from LOW
	sll	a3,a3,t0		// high <<
	or	a3,a3,t1		// add carry from low
	sll	a0,a0,t0		// low <<

	// 2*pi in 8.56 fixed point -> A2:A1 (= 2*pi << 56 = 0x06487ED5110B4612 = 452751216129820178)
3:	li	a2,0x06487ED5		// A2 <- 2*pi << 56 HIGH
	li	a1,0x110B4612		// A1 <- 2*pi << 56 LOW

	// limit max. angle to modulo 2*PI
	// - mantissa A3:A0 will be in range 0 .. 0x06487ED5110B4611 = 452751216129820177
1:	mv	t0,a0
	sub	a0,a0,a1		// subtract 2*pi
	sltu	t0,t0,a0
	sub	a3,a3,a2
	sub	a3,a3,t0
	bgez	a3,1b			// limit positive number

1:	add	a0,a0,a1		// add 2*pi
	sltu	t0,a0,a1
	add	a3,a3,a2
	add	a3,a3,t0
	bltz	a3,1b			// limit negative number, result angle will be in range 0 (include) .. 2*PI (exclude)

	// convert angle A3:A0 to unsigned format 3.61 -> A3:A2 (shift mantissa left << 5)
	// - mantissa will be in range 0 .. 0xC90FDAA22168C220 = 14488038916154245664
	srli	t0,a0,32-5		// T0 <- carry from low
	slli	a3,a3,5			// A3 <- shift high << 5
	or	a3,a3,t0		// add carry from low
	slli	a2,a0,5			// A2 <- shift low << 5

// ---- mantissa A3:A2 here is in unsigned fixed point format 3.61, range 0..0xC90FDAA22168C220 = 14488038916154245664 (= 0..2*pi)

	// clear signs
	li	s2,0			// sin() sign = 0
	li	s3,0			// cos() sign = 0

	// pi in 3.61 fixed point -> A1:A0 (= pi << 61 = 0x6487ED5110B46120 = 7244019458077122848)
	li	a1,0x6487ED51		// A1 <- pi << 61 HIGH
	li	a0,0x10B46120		// A0 <- pi << 61 LOW

	// convert A3:A2 to range 0..pi
	// - mantissa will be in range = 0 .. 0x6487ED5110B4611F (= 0..pi)
	bltu	a3,a1,2f		// skip if xn < pi
	bne	a3,a1,1f		// process if xn > pi
	bltu	a2,a0,2f		// skip if xn < pi

1:	mv	t0,a2
	sub	a2,a2,a0		// subtract xn - pi
	sltu	t0,t0,a2
	sub	a3,a3,a1
	sub	a3,a3,t0
	not	s2,s2			// invert sin sign
	not	s3,s3			// invert cos sign

	// convert A3:A2 to range 0..pi/2
	//  mantissa will be in range = 0 .. 0x3243F6A8885A3090 = 3622009729038561424 (= 0..pi/2)
	// pi/2 in 3.61 fixed point = 0x3243F6A8885A3090 = 3622009729038561424
2:	li	t0,0x3243F6A8		// T0 <- pi/2 << 61 HIGH
	bltu	a3,t0,2f		// skip if xn < pi/2
	bne	a3,t0,1f		// process if xn > pi/2
	li	t0,0x885A3090		// T0 <- pi/2 << 61 LOW
	bltu	a2,t0,2f		// skip if xn < pi/2

1:	mv	t0,a0
	sub	a2,a0,a2		// subtract pi - xn
	sltu	t0,t0,a2
	sub	a3,a1,a3
	sub	a3,a3,t0
	not	s3,s3			// invert cos sign

	// convert A3:A2 in range 0..pi/2 in unsigned fixed point format 3.61 to range -1..+1 in signed fixed point format 1.63
	//  k = (0x8000000000000000 / 0x3243F6A8885A3090 << 64) >> 2 = (9223372036854775808 / 3622009729038561424 << 64) >> 2 = 11743562013128004897
2:	li	a0,0x4E441523		// = 11743562013128004897+2 = 0xA2F9836E4E441523
	li	a1,0xA2F9836E 
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high a2,a3,a0,a1,t0,t1

	// limit range x < 0x2000000000000000
	srli	t0,a3,29		// check high 3 bits
	beqz	t0,3f			// no overflow
	li	a3,0x1fffffff		// limit value to 0x1fffffffffffffff
	li	a2,0xffffffff

	// A3:A2 << 3
3:	srli	t0,a2,32-3		// T0 <- carry from LOW
	slli	a2,a2,3
	slli	a3,a3,3
	or	a3,a3,t0

	// subtract 1 to get range -1..+1 in signed format 1.63, range -0x7FFFFFFFFFFFFFFC..+0x7FFFFFFFFFFFFFFC
	li	a0,0xFFFFFFFC		// middle of the range = 0x7FFFFFFFFFFFFFFC
	li	a1,0x7FFFFFFF
	mv	t0,a2
	sub	a2,a2,a0
	sltu	t0,t0,a2
	sub	a3,a3,a1
	sub	a3,a3,t0
	ret

// ==== normalize angle to range 0..90 deg and convert to fixed point format 1.63
// INPUT: A1:A0 = argument x
// OUTPUT: A3:A2 = mantissa in fixed point format 1.63, range = -1..+1 = 0..pi/2
//	   S2 = sin sign 0 or -1
//	   S3 = cos sign 0 or -1
// TRASHED: A0,A1,A4, T0-T2 (+ registers from remainder function)

double_section _asm_sin_deg_norm
.p2align 2,,

_asm_sin_deg_norm:

	// push registers
	sw	s2,8(sp)		// save S2
	sw	s3,12(sp)		// save S3

	// normalize big angle >= 360
	bclr	a2,a1,31		// clear sign bit
	li	a3,0x40768000		// 360 deg HIGH
	bltu	a2,a3,1f		// not big angle

	// normalize using remainder, modulo 360 (remainder returns angle in range -180 .. +180)
	sw	ra,4(sp)		// save internal return address
	li	a2,0			// A2 <- 360 LOW
	call	remainder		// normalize angle to range -pi..+pi
	lw	ra,4(sp)		// restore internal return address

	// absolute value of the angle
	// - angle will be in range 0..360
1:	srai	s2,a1,31		// S2 <- sign 0 or -1
	bclr	a1,a1,31		// clear sign bit

	// get signed exponent -> A4
	// - signed exponent here is max. +8
	srli	a4,a1,20		// shift exponent to base position
	addi	a4,a4,-DOUBLE_EXP1

	// unpack mantissa to unsigned format 12.52, set hidden bit '1' -> A3:A0
	slli	a1,a1,11		// clear exponent
	bset	a1,a1,31		// set hidden bit "1"
	srli	a3,a1,11		// return to position		

	// prepare number of shifts to convert mantissa to format 9.55 -> T0
	addi	t0,a4,55-52		// exps <= +8, number of shifts <= +11

	// convert mantissa to unsigned fixed point format 9.55 (rounding down) -> A3:A0
	// - mantissa will be in range = 0 .. 0xB400000000000000
	li	t2,32
	bgez	t0,2f			// number of shifts >= 0

	// - right shift A3:A0 by -T0
	neg	t0,t0			// T0 <- negate number of shifts
	sub	t1,t0,t2		// T1 <- number of shifts - 32
	bgez	t1,1f			// number of shifts is >= 32

	sub	t1,t2,t0		// T1 <- 32 - shifts
	sll	t1,a3,t1		// T1 <- carry from HIGH
	srl	a0,a0,t0		// low >>
	or	a0,a0,t1		// add carry from high
	srl	a3,a3,t0		// high >>
	j	3f

1:	bge	t1,t2,11f		// number of shifts >= 64
	srl	a0,a3,t1		// A0 <- shift mantissa right
	li	a3,0			// A3 <- mantissa HIGH = 0
	j	3f

11:	li	a3,0			// A3 <- mantissa HIGH = 0
	li	a0,0			// A0 <- mantissa LOW = 0
	j	3f

	// - left shift A3:A0 by T0, number of shifts = 0..11
2:	beqz	t0,3f			// no shifts
	sub	t1,t2,t0		// T1 <- 32 - shifts
	srl	t1,a0,t1		// T1 <- carry from LOW
	sll	a3,a3,t0		// high <<
	or	a3,a3,t1		// add carry from low
	sll	a0,a0,t0		// low <<

	// 180 in 9.55 fixed point = 180 << 55 = 6485183463413514240 = 0x5A00000000000000 -> A2:A1
3:	li	a2,0x5A000000		// A2 <- 180 HIGH (low = 0)

	// convert A3:A0 to range 0..180
	// - mantissa will be in range = 0 .. 0x5A00000000000000 (= 0..180)
	li	s3,0			// cos sign = 0
	bltu	a3,a2,2f		// skip if xn < 180
	sub	a3,a3,a2		// xn - 180
	not	s2,s2			// invert sin sign
	not	s3,s3			// invert cos sign

	// convert A3:A0 to range 0..90
	//  mantissa will be in range = 0 .. 0x2D00000000000000 = 3242591731706757120 (= 0..90)
	//  90deg = 0x2D00000000000000 = 3242591731706757120
2:	li	t0,0x2D000000		// T0 <- 90 HIGH (low = 0)
	bltu	a3,t0,2f		// skip if xn < 90
	
	snez	t0,a0			// T0 <- 1 of angle LOW != 0 (there will be carry)
	sub	a0,x0,a0		// subtract 180 - xn LOW
	sub	a3,a2,a3		// subtract 180 - xn HIGH
	sub	a3,a3,t0		// carry
	not	s3,s3			// invert cos sign

	// convert A3:A0 to format 7.57 -> A3:A2 (shift mantissa left << 2)
	// mantissa will be in range 0 .. 0xB400000000000000 = 12970366926827028480
2:	srli	t0,a0,32-2		// T0 <- carry from low
	slli	a3,a3,2			// A3 <- shift high << 2
	or	a3,a3,t0		// add carry from low
	slli	a2,a0,2			// A2 <- shift low << 2

	// convert A3:A2 in range 0..90 in unsigned fixed point format 7.57 to range -1..+1 in signed fixed point format 1.63
	//  k = (0x8000000000000000 / 0xB400000000000000 << 64) = (9223372036854775808 / 12970366926827028480 << 64) = 13117684674637903372 round up
	li	a0,0x0B60B60A		// = 13117684674637903372-2 = 0xB60B60B60B60B60A
	li	a1,0xB60B60B6
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high a2,a3,a0,a1,t0,t1

	// limit range x < 0x8000000000000000
	bgez	a3,3f			// no overflow
	li	a3,0x7fffffff		// limit value to 0x7fffffffffffffff
	li	a2,0xffffffff

	// A3:A2 << 1
3:	srli	t0,a2,32-1		// T0 <- carry from LOW
	slli	a2,a2,1
	slli	a3,a3,1
	or	a3,a3,t0

	// subtract 1 to get range -1..+1 in signed format 1.63, range -0x7FFFFFFFFFFFFFFF..+0x7FFFFFFFFFFFFFFF -> A3:A2
	li	a0,0xFFFFFFFF		// middle of the range = 0x7FFFFFFFFFFFFFFF
	li	a1,0x7FFFFFFF
	mv	t0,a2
	sub	a2,a2,a0
	sltu	t0,t0,a2
	sub	a3,a3,a1
	sub	a3,a3,t0
	ret

// ==== Chebyshev approximation
// INPUT: A3:A2 = mantissa in signed fixed point format 1.63, range = -1..+1 = 0..pi/2
//	  A4 = sign 0 or -1
// OUTPUT: A1:A0 = result
// TRASHED: A2-A5, T0-T3

double_section _asm_sin_cheb
.p2align 2,,

_asm_sin_cheb:
	la	a5,_sin_tab			// A5 <- table of constants

// Here is:
//  A1:A0 = result accumulator
//  A3:A2 = input mantissa x in 1.63 format
//  A4 = sign 0 or -1
//  A5 = pointer to table of constants

// ---- process coefficients 15 to 12

	// coeff[15]
	lw	a0,0x60(a5)			// A0 <- coeff[15]
	lw	t0,0x64(a5)			// T0 <- coeff[14]
	mulh	a0,a3,a0			// A0 <- mantissaH * accumulatorL

	// coeff[14]
	add	a0,a0,t0			// A0 <- add coeff[14]
	lw	t0,0x68(a5)			// T0 <- coeff[13]
	mulh	a0,a3,a0			// A0 <- mantissaH * accumulatorL

	// coeff[13]
	sh1add	a0,a0,t0			// A0 <- accL << 1, add coeff[13]
	lw	t0,0x6c(a5)			// T0 <- coeff[12]
	mulh	a0,a3,a0			// A0 <- mantissaH * accumulatorL

	// coeff[12]
	sh1add	a0,a0,t0			// A0 <- accL << 1, add coeff[12]
	lw	t0,0x00(a5)			// T0 <- load coeff[11] LOW
	lw	t1,0x04(a5)			// T1 <- load coeff[11] HIGH
	mulh	a0,a3,a0			// A0 <- mantissaH * accumulatorL
	srai	a1,a0,31			// A1 <- extend accumulator to s64

	// add coeff[11] T1:T0 to acc A1:A0
	sh1add	a0,a0,t0			// A0 <- accL << 1, add coeff LOW
	sltu	t0,a0,t0			// T0 <- carry
	add	a1,a1,t1			// add coeff HIGH
	add	a1,a1,t0			// add carry

// ---- add coefficients in case of positive mantissa

	// check positive mantissa
	la	t3,_sin_tab_end - 8		// T3 <- end of data table
	srli	t0,a2,31			// T0 <- carry from mantissa LOW
	slli	a2,a2,1				// A2 <- mantissa << 1 LOW
	bltz	a3,4f				// skip if mantissa is negative

	// mantissa A3:A2 << 1 (continue)
	sh1add	a3,a3,t0			// A3 <- mantissa << 1 HIGH, add carry from mantissa LOW

	// absolute value of the accumulator A1:A0
2:	mv	t2,a1				// T2 <- save acc HIGH, for later check sign
	bgez	t2,1f				// skip if mantissa is not negative
	not	a0,a0				// use NOT instead of NEG
	not	a1,a1

	// multiply
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
1:	mulu64high a0,a1,a2,a3,t0,t1

	// negate result
	// - no carry from LOW, so we use more common case NOT instead of NEG
	bgez	t2,1f				// skip if mantissa was not negative
	not	a0,a0
	not	a1,a1

	// load next constant
1:	addi	a5,a5,8				// shift table pointer
	lw	t0,0x00(a5)			// T0 <- coeff low
	lw	t1,0x04(a5)			// T1 <- coeff high

	// add coeff T1:T0 to acc A1:A0
	add	a0,a0,t0			// add coeff LOW
	sltu	t0,a0,t0			// T0 <- carry
	add	a1,a1,t1			// add coeff HIGH
	add	a1,a1,t0			// add carry

	// loop
	bne	a5,t3,2b			// next coeff
	j	8f

// ---- add coefficients in case of negative mantissa

	// mantissa A3:A2 << 1 (continue)
4:	sh1add	a3,a3,t0			// A3 <- mantissa << 1 HIGH, add carry from mantissa LOW

	// negate mantissa A3:A2 - use faster NOT instead of NEG
	not	a2,a2
	not	a3,a3

	// absolute value of the accumulator A1:A0
2:	mv	t2,a1				// T2 <- save acc HIGH, for later check sign
	bgez	t2,1f				// skip if mantissa is not negative
	not	a0,a0				// use NOT instead of NEG
	not	a1,a1

	// multiply
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
1:	mulu64high a0,a1,a2,a3,t0,t1

	// negate result
	// - no carry from LOW, so we use more common case NOT instead of NEG
	bltz	t2,1f				// skip if mantissa was negative, but x was negative too
	not	a0,a0
	not	a1,a1

	// load next constant
1:	addi	a5,a5,8				// shift table pointer
	lw	t0,0x00(a5)			// T0 <- coeff low
	lw	t1,0x04(a5)			// T1 <- coeff high

	// add coeff T1:T0 to acc A1:A0
	add	a0,a0,t0			// add coeff LOW
	sltu	t0,a0,t0			// T0 <- carry
	add	a1,a1,t1			// add coeff HIGH
	add	a1,a1,t0			// add carry

	// loop
	bne	a5,t3,2b			// next coeff

// result mantissa A1:A0 is in 2.62 format, value 0..+1

	// zero result or underflow
8:	or	t0,a0,a1			// check zero result
	beqz	t0,_asm_sin_cheb_zero		// zero result
	bltz	a1,_asm_sin_cheb_zero		// negative result

	// overflow, limit to max. value 1.0
	li	t0,0x40000000			// check value 0x4000000000000000
	blt	a1,t0,1f
	li	a1,0x3FFFFFFF			// max. value = 0x3FFFFFFFFFFFFFFF
	li	a0,0xFFFFFFFF

	// prepare number of shifts -> T1 -52 (up,left) .. +11 (down,right)
1:	bnez	a1,1f			// skip if mantissa HIGH is not 0
	li	t1,11 - 32		// T1 <- 11 - 32 = -21
	clz	t0,a0			// T0 <- count leading zeroes LOW
	j	2f
1:	clz	t0,a1			// T0 <- count leading zeroes HIGH
	li	t1,11			// T1 <- 11
2:	sub	t1,t1,t0		// T1 <- number of shifts (11-(clz(a0)+32) or 11-clz(a1))

	// prepare result exponent -> T3 (subtract 1 to clear hidden bit '1')
	addi	t3,t1,DOUBLE_EXP1-10 - 1

	// normalize mantissa (T1 contains number of shifts: -52 (up,left) .. +11 (down,right))
	li	t2,32
	bgtz	t1,3f			// skip if number of shifts > 0 (shift down,right)

	// shift mantissa up,left 0..52
	neg	t1,t1			// T1 <- absolute value of number of shifts left (= 0..52)
	sub	t0,t1,t2		// T0 <- number of shifts - 32 (= -32..+20)
	bgez	t0,2f			// number of shifts is >= 32 (= 32..52, T0=0..+20)
	beqz	t1,8f			// skip if number of shifts = 0

	sub	t0,t2,t1		// T0 <- shift inversion = 32 - shifts = 1..31 (T1 = 1..31)
	sll	a1,a1,t1		// shift mantissa HIGH left by 1..31
	srl	t0,a0,t0		// T0 <- carry from mantissa LOW
	or	a1,a1,t0		// add carry from mantissa LOW to mantissa HIGH
	sll	a0,a0,t1		// shift mantissa LOW left by 1..31
	j	8f

2:	sll	a1,a0,t0		// A1 <- shift mantissa left by 0..20 shifts
	li	a0,0			// A0 <- mantissa LOW = 0
	j	8f

	// shift mantissa down with rounding 1..11 (here is T1 = number of shifts right 1..11)
3:	add	t1,t1,-1		// number of shifts = 0..10
	beqz	t1,1f			// skip number of shifts 0 (number of shifts 32 is not supported)
	sub	t0,t2,t1		// T0 <- shift inversion 32-(1..10) = 22..31
	srl	a0,a0,t1		// shift mantissa LOW right by 1..10
	sll	t0,a1,t0		// T0 <- carry from mantissa HIGH, shift left by 22..31
	or	a0,a0,t0		// add carry from mantissa HIGH to mantissa LOW
	srl	a1,a1,t1		// shift mantissa HIGH right by 1..10

	// increment acc + 1 for rounding -> A1:T0
1:	addi	t0,a0,1			// increment mantissa, to round up -> T0
	sltu	t1,t0,a0		// carry from LOW
	add	a1,a1,t1		// add carry to HIGH

	// shift mantissa >> 1
	srli	a0,t0,1			// shift mantissa >> 1 LOW
	slli	t0,a1,31		// carry from high
	or	a0,a0,t0		// add carry from HIGH to LOW
	srli	a1,a1,1			// shift mantissa >> 1 HIGH

	// check carry on rounding
	srli	t0,a1,21		// check carry on rounding
	beqz	t0,8f			// no carry

	// carry correction
	srli	a0,a0,1			// shift mantissa >> 1 LOW
	slli	t0,a1,31		// carry from high
	or	a0,a0,t0		// add carry from HIGH to LOW
	srli	a1,a1,1			// shift mantissa >> 1 HIGH
	addi	t3,t3,1			// shift exponent

	// compose result
8:	slli	t3,t3,20		// shift exponent to position
	add	a1,a1,t3		// add exponent to result, clear hidden bit '1'
	slli	a4,a4,31		// A4 <- shift sign to position
	or	a1,a1,a4		// add sign to result
9:	ret

// ---- zero result

_asm_sin_cheb_zero:
	li	a1,0
	li	a0,0
	ret

double_section _sin_cheb_tab
.p2align 2,,

// sin table of constants
_sin_tab:
	.word	0xAA7280C1, 0xFFFFFFFE	// 0x00: coeff[11] = -5730303807 = -1.2425615672446040571e-9 << 62
	.word	0x50566D7A, 0xFFFFFFED	// 0x08: coeff[10] = -80256537222 = -1.7402862402446451959e-8 << 62
	.word	0xEB7EA9D5, 0x000000ED	// 0x10: coeff[9]  =  1021858195925 = 2.2158017519889359529e-7 << 62
	.word	0x5C7A6329, 0x00000AA6	// 0x18: coeff[8]  =  11709632373545 = 2.5391217716809155258e-6 << 62
	.word	0x832CD61B, 0xFFFF9385	// 0x20: coeff[7]  = -119273336023525 = -2.5863282007259964339e-5 << 62
	.word	0x9E7D96DC, 0xFFFC392A	// 0x28: coeff[6]  = -1063044696402212 = -2.3051107385769441652e-4 << 62
	.word	0x63820E0C, 0x001CDA10	// 0x30: coeff[5]  =  8121063271697932 = 1.7609748884134271780e-3 << 62
	.word	0xD94BEFE7, 0x00B7AD24	// 0x38: coeff[4]  =  51700294514110439 = 1.1210714326067788982e-2 << 62
	.word	0xC7417045, 0xFC588B47	// 0x40: coeff[3]  = -263307437799280571 = -5.7095699218714364755e-2 << 62
	.word	0xC6A6CEF7, 0xF20AD24F	// 0x48: coeff[2]  = -1005760326686814473 = -2.1808950623871498270e-1 << 62
	.word	0xBAC5C854, 0x238B0635	// 0x50: coeff[1]  =  2561147640926816340 = 5.5536036726979578088e-1 << 62
	.word	0xFE779921, 0x2D413CCC	// 0x58: coeff[0]  =  3260954456333195553 = 7.0710678118654752438e-1 << 62
_sin_tab_end:
	.word	-131912			// 0x60: coeff[15] = -1.4301875419685838441e-14 << 63
	.word	-1258138		// 0x64: coeff[14] = -2.7281516101906876443e-13 << 62
	.word	22657614		// 0x68: coeff[13] =  4.9130868909982775974e-12 << 62
	.word	375026459		// 0x6C: coeff[12] =  8.1320900422699374499e-11 << 62

// ----------------------------------------------------------------------------
//                      atan(x)... - Chebyshev approximation
// ----------------------------------------------------------------------------
// double atanf(double x);
// double atanf_deg(double x);
// double acotanf(double x);
// double acotanf_deg(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A7, T0..T6

double_section _atan
.p2align 2,,

// arc cotangent in degrees (result is 0 .. 180)
.global acotan_deg
acotan_deg:
	li	t5,1		// T5 <- flag 0=radians, 1=degrees
	li	t4,1		// T4 <- flag 0=atan(), 1=acotan()
	j	_asm_atan

// arc cotangent in radians (result is 0 .. pi)
.global __wrap_acotan
.global acotan
__wrap_acotan:
acotan:
	li	t5,0		// T5 <- flag 0=radians, 1=degrees
	li	t4,1		// T4 <- flag 0=atan(), 1=acotan()
	j	_asm_atan

// arc tangent in degrees (result is -90 .. +90)
.global atan_deg
atan_deg:
	li	t5,1		// T5 <- flag 0=radians, 1=degrees
	j	_atan22

// arc tangent in radians (result is -pi/2 .. +pi/2)
.global __wrap_atan
.global atan
__wrap_atan:
atan:
	li	t5,0		// T5 <- flag 0=radians, 1=degrees
_atan22:li	t4,0		// T4 <- flag 0=atan(), 1=acotan()

// arc tangent - Chebyshev approximation
// INPUT: A1:A0 = argument X
//	  T4 = flag 0=atan(), 1=acotan()
//	  T5 = flag 0=radians, 1=degrees
// OUTPUT: A1:A0 = result
// TRASHED: A2..A7, T0..T6
_asm_atan:
	// get sign -> A4
	srai	a4,a1,31		// A4 <- sign 0=positive, -1=negative

	// absolute value of the number -> A2:(A0)
	bclri	a2,a1,31		// A2 <- clear sign bit of the number

	// get unsigned exponent -> A3
	srli	a3,a2,20		// A3 <- unsigned exponent

	// zero or small angle
	li	t0,DOUBLE_EXP1-20	// T0 <- minimal exponent
	blt	a3,t0,_asm_atan_0	// go to small value

	// convert to signed exponent -> A3
	addi	a3,a3,-DOUBLE_EXP1	// subtract exponent bias

	// infinity or big value
	li	t0,23			// T0 <- maximal exponent
	bgt	a3,t0,_asm_atan_inf	// go to big value

	// result addition = 0 -> T3:T2
	li	t2,0			// T2 <- 0, no result addition
	li	t3,0			// T3 <- 0

// Here is:
//  A1:A0 = input argument x
//  A2:(A0) = absolute value of the number x (clear sign bit)
//  A3 = signed exponent
//  A4 = sign 0=positive, -1=negative
//  T3:T2 = result addition (on alternative method)
//  T4 = flag 0=atan(), 1=acotan()
//  T5 = flag 0=radians, 1=degrees

// ---- if abs(x) > 1, use alternative method with acotan()

	bltz	a3,4f			// skip if exp < 0, number is < 1.0

	// use -1/x
	add	sp,sp,-16		// space in the stack
	sw	ra,0(sp)		// save return address
	sw	t4,4(sp)		// save T4 (flag 0=atan(), 1=acotan())
	sw	t5,8(sp)		// save T5 (flag 0=radians, 1=degrees)
	_asm_drecneg			// calculate -1/x
	lw	t5,8(sp)		// restore T5 (flag 0=radians, 1=degrees)
	lw	t4,4(sp)		// restore T4 (flag 0=atan(), 1=acotan())
	lw	ra,0(sp)		// restore return address
	add	sp,sp,16		// destroy space in the stack

	// get sign -> A4
	srai	a4,a1,31		// A4 <- sign 0=positive, -1=negative

	// absolute value of the number -> A2:(A0)
	bclri	a2,a1,31		// A2 <- clear sign bit of the number

	// get signed exponent -> A3
	srli	a3,a2,20		// A3 <- unsigned exponent
	addi	a3,a3,-DOUBLE_EXP1	// subtract exponent bias

	// result addition = pi/2 -> T3:T2
	li	t3,0x6487ED51		// T3:T2 <- pi/2 in format 2.62, result addition = 7244019458077122842-3
	li	t2,0x10B46117

	// is x is positive, use -pi/2
	bnez	a4,4f			// skip if x is negative (A4 = sign 0=positive, -1=negative)
	li	t3,0x9B7812AE 		// T3:T2 <- -pi/2 in format 2.62, result addition = -7244019458077122842+60
	li	t2,0xEF4B9F22

// argument x (= A2:A0) is in range 0..+1 here, exponent is -23..0

// Here is:
//  A1:A0 = input argument x
//  A2:(A0) = absolute value of the number x (clear sign bit)
//  A3 = signed exponent
//  A4 = sign 0=positive, -1=negative
//  T3:T2 = result addition (on alternative method)
//  T4 = flag 0=atan(), 1=acotan()
//  T5 = flag 0=radians, 1=degrees

	// normalize mantissa A2:A0 - shift to max position 1.63 and restore hidden bit '1' -> A7:A6
4:	neg	t0,a3			// T0 <- number of shifts (= -exp) = 0..23
	slli	a7,a2,11		// A7 <- shift mantissa HIGH << 11 to destroy exponent
	bset	a7,a7,31		// set hidden bit "1"
	srli	t1,a0,21		// T1 <- carry from mantissa LOW
	or	a7,a7,t1		// add carry from mantissa LOW to mantissa HIGH
	slli	a6,a0,11		// A6 <- shift mantissa LOW << 11

	// shift mantissa A7:A6 right by exponent bits, to get signed fixed number in format 2.62
	blez	t0,1f			// number of shifts = 0; continue if number of shifts = 1..23
	li	t1,32
	sub	t1,t1,t0		// T1 <- 32 - number of shifts
	srl	a6,a6,t0		// shift mantissa LOW
	sll	t1,a7,t1		// T1 <- carry from mantissa HIGH
	or	a6,a6,t1		// add carry from mantissa HIGH to mantissa LOW
	srl	a7,a7,t0		// shift mantissa HIGH

	// calculate 2*x^2 - 1, signed fixed point format 2.62 -> A3:A2
	//   y = x*fnc(2*x^2 - 1)
// square (u64*u64->u128)>>64 simplified (no carry from LOW)
// resL . result LOW (can be equal to aL)
// resH . result HIGH (can be equal to aH)
// aL ... operand LOW
// aH ... operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro sqru64high resL, resH, aL, aH, tmp1, tmp2
1:	sqru64high a2,a3,a6,a7,t0,t1

	// mantissa << 1
	srli	t0,a2,31		// T0 <- carry from LOW
	slli	a3,a3,1
	slli	a2,a2,1
	or	a3,a3,t0

	// subtract 1
	li	t0,0x40000000
	sub	a3,a3,t0

// argument x (A3:A2) is in range -1..+1 here, signed fixed point format 2.62

	// table of constants -> A5
	la	a5,_atan_tab

// Here is:
//  A1:A0 = result accumulator
//  A3:A2 = mantissa m
//  A4 = sign 0=positive, -1=negative
//  A5 = talbe of constants
//  A7:A6 = original mantissa x
//  T3:T2 = result addition (on alternate method)
//  T4 = flag 0=atan(), 1=acotan()
//  T5 = flag 0=radians, 1=degrees

// ---- process coefficients 23 to 17

	// coeff[23]
	lw	a0,0x80(a5)			// A0 <- coeff[23]
	lw	t0,0x84(a5)			// T0 <- coeff[22]
	mulh	a0,a3,a0			// A0 <- mantissaH * accumulatorL

	// coeff[22]
	sh1add	a0,a0,t0			// A0 <- accL << 1, add coeff[22]
	lw	t0,0x88(a5)			// T0 <- coeff[21]
	mulh	a0,a3,a0			// A0 <- mantissaH * accumulatorL

	// coeff[21]
	sh2add	a0,a0,t0			// A0 <- accL << 2, add coeff[21]
	lw	t0,0x8c(a5)			// T0 <- coeff[20]
	mulh	a0,a3,a0			// A0 <- mantissaH * accumulatorL

	// coeff[20]
	sh2add	a0,a0,t0			// A0 <- accL << 2, add coeff[20]
	lw	t0,0x90(a5)			// T0 <- coeff[19]
	mulh	a0,a3,a0			// A0 <- mantissaH * accumulatorL

	// coeff[19]
	sh2add	a0,a0,t0			// A0 <- accL << 2, add coeff[19]
	lw	t0,0x94(a5)			// T0 <- coeff[18]
	mulh	a0,a3,a0			// A0 <- mantissaH * accumulatorL

	// coeff[18]
	sh2add	a0,a0,t0			// A0 <- accL << 2, add coeff[18]
	lw	t0,0x98(a5)			// T0 <- coeff[17]
	mulh	a0,a3,a0			// A0 <- mantissaH * accumulatorL

	// coeff[17]
	sh2add	a0,a0,t0			// A0 <- accL << 2, add coeff[17]
	lw	t0,0x9c(a5)			// T0 <- load coeff[11] LOW
	mulh	a0,a3,a0			// A0 <- mantissaH * accumulatorL
	srai	a1,a0,31			// A1 <- extend accumulator to s64

	// add coeff[16] T0 to acc A1:A0
	sh2add	a0,a0,t0			// A0 <- accL << 2, add coeff[16]
	sltu	t0,a0,t0			// T0 <- carry
	add	a1,a1,t0			// add carry

// ---- add coefficients in case of positive mantissa

	// check positive mantissa
	bltz	a3,4f				// skip if mantissa is negative

	// absolute value of the accumulator A1:A0
2:	mv	t6,a1				// T6 <- save acc HIGH, for later check sign
	bgez	t6,1f				// skip if mantissa is not negative
	not	a0,a0				// use NOT instead of NEG
	not	a1,a1

	// multiply
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
1:	mulu64high a0,a1,a2,a3,t0,t1

	// shift << 2
	srli	t0,a0,30
	sh2add	a1,a1,t0
	slli	a0,a0,2

	// negate result
	// - no carry from LOW, so we use more common case NOT instead of NEG
	bgez	t6,1f				// skip if mantissa was not negative
	not	a0,a0
	not	a1,a1

	// load next constant -> T1:T0
1:	lw	t0,0x00(a5)			// T0 <- coeff low
	lw	t1,0x04(a5)			// T1 <- coeff high
	addi	a5,a5,8				// shift table pointer

	// add coeff T1:T0 to acc A1:A0
	add	a0,a0,t0			// add coeff LOW
	sltu	t0,a0,t0			// T0 <- carry
	add	a1,a1,t1			// add coeff HIGH
	add	a1,a1,t0			// add carry

	// loop
	la	t0,_atan_tab_end		// T6 <- end of data table
	bne	a5,t0,2b			// next coeff
	j	8f

// ---- add coefficients in case of negative mantissa

	// mantissa A3:A2 << 2
4:	srli	t0,a2,30
	sh2add	a3,a3,t0
	slli	a2,a2,2

	// negate mantissa A3:A2 - use faster NOT instead of NEG
	not	a2,a2
	not	a3,a3

	// absolute value of the accumulator A1:A0
2:	mv	t6,a1				// T6 <- save acc HIGH, for later check sign
	bgez	t6,1f				// skip if mantissa is not negative
	not	a0,a0				// use NOT instead of NEG
	not	a1,a1

	// multiply
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
1:	mulu64high a0,a1,a2,a3,t0,t1

	// negate result
	// - no carry from LOW, so we use more common case NOT instead of NEG
	bltz	t6,1f				// skip if mantissa was negative, but x was negative too
	not	a0,a0
	not	a1,a1

	// load next constant
1:	lw	t0,0x00(a5)			// T0 <- coeff low
	lw	t1,0x04(a5)			// T1 <- coeff high
	addi	a5,a5,8				// shift table pointer

	// add coeff T1:T0 to acc A1:A0
	add	a0,a0,t0			// add coeff LOW
	sltu	t0,a0,t0			// T0 <- carry
	add	a1,a1,t1			// add coeff HIGH
	add	a1,a1,t0			// add carry

	// loop
	la	t0,_atan_tab_end		// T6 <- end of data table
	bne	a5,t0,2b			// next coeff

// Here is:
//  A1:A0 = result accumulator
//  A4 = sign 0=positive, -1=negative
//  A7:A6 = original mantissa x
//  T3:T2 = result addition (on alternate method)
//  T4 = flag 0=atan(), 1=acotan()
//  T5 = flag 0=radians, 1=degrees

// result A1:A0 is -pi/2..+pi/2 in 2.62 format

	// multiply by original x
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
8:	mulu64high a0,a1,a6,a7,t0,t1

	// shift << 1
	srli	t0,a0,31
	sh1add	a1,a1,t0
	slli	a0,a0,1

	// negate
	beqz	a4,1f				// skip if was not negative
	negs64	a0,a1,t0

	// add alternative correction T3:T2
1:	add	a0,a0,t2
	add	a1,a1,t3
	sltu	t0,a0,t2
	add	a1,a1,t0

	// prepare exponent base -> A3
	li	a3,DOUBLE_EXP1 + 2

	// acotan() correction: acotan(x) = pi/2 - atan(x)
	beqz	t4,3f			// skip if atan() (T4 = flag 0=atan(), 1=acotan())

	// if result is positive, use "acc = pi/2 - acc" in 2.62 format
	li	t1,0x6487ED51		// T1:T0 <- pi/2 in format 2.62 = 7244019458077122842
	li	t0,0x10B4611A
	bgez	a1,1f			// skip if result is positive

	// if result is negative, use "acc = pi/2 - acc" in 3.61 format (result would overflow otherwise)
	slli	t0,a1,31		// A1:A0 <- halve result
	srai	a1,a1,1
	srli	a0,a0,1
	or	a0,a0,t0
	li	t1,0x3243F6A8		// T1:T0 <- pi/2 in format 3.61 = 3622009729038561421 - 35
	li	t0,0x885A306A
	addi	a3,a3,1			// compensate exponent

1:	sub	a0,t0,a0		// A1:A0 <- pi/2 - acc
	sltu	t0,t0,a0
	sub	a1,t1,a1
	sub	a1,a1,t0

	// zero result
3:	or	t0,a1,a0
	beqz	t0,9f			// zero result

	// absolute value
	srai	a4,a1,31		// A4 <- sign 0=positive, -1=negative
	beqz	a4,1f			// not negative
	negs64	a0,a1,t0		// negate

	// convert to degrees: x = x * (180/PI)
1:	beqz	t5,2f			// skip if radians (T5 = flag 0=radians, 1=degrees)

	// x = x * (180/PI) ... (180/PI << 64) >> 6 = 16514384080961256899 - 30
	li	t1,0xE52EE0D3
	li	t0,0x1E0FBDA5
	mulu64high a0,a1,t0,t1,t6,a2
	or	t0,a1,a0
	beqz	t0,9f			// zero result
	addi	a3,a3,6			// compensate exponent

	// prepare number of shifts to normalize mantissa to max. position 1.63 -> T1
2:	bnez	a1,1f			// skip if mantissa HIGH is not 0
	clz	t1,a0			// T1 <- count leading zeroes LOW
	addi	t1,t1,32
	j	2f
1:	clz	t1,a1			// T1 <- count leading zeroes HIGH
2:	sub	a3,a3,t1		// compensate exponent

	// normalize mantissa to max. position 1.63 (shift A1:A0 << T1)
	li	t2,32
	sub	t0,t1,t2		// T0 <- number of shifts - 32
	bgez	t0,2f			// number of shifts is >= 32
	beqz	t1,3f			// skip if number of shifts = 0

	sub	t0,t2,t1		// T0 <- shift inversion = 32 - shifts = 1..31 (T1 = 1..31)
	sll	a1,a1,t1		// shift mantissa HIGH left by 1..31
	srl	t0,a0,t0		// T0 <- carry from mantissa LOW
	or	a1,a1,t0		// add carry from mantissa LOW to mantissa HIGH
	sll	a0,a0,t1		// shift mantissa LOW left by 1..31
	j	3f

2:	sll	a1,a0,t0		// A1 <- shift mantissa left
	li	a0,0			// A0 <- mantissa LOW = 0

	// rounding mantissa
3:	li	t0,0x400 + 125		// rounding correction
	add	a0,a0,t0
	sltu	t1,a0,t0
	add	a1,a1,t1

	// check overflow
	bnez	a1,1f			// no overflow
	blt	a0,t0,2f		// overflow

	// no overflow - remove leading bit '1'
1:	srli	t0,a0,31
	sh1add	a1,a1,t0
	slli	a0,a0,1
	addi	a3,a3,-1		// compensate exponent

	// shift mantissa A1:A0 to position >> 12
2:	srli	a0,a0,12
	slli	t0,a1,32-12
	srli	a1,a1,12
	or	a0,a0,t0

	// compose result
	slli	a3,a3,20		// shift exponent to position
	or	a1,a1,a3		// add exponent to mantissa
	slli	a4,a4,31		// shift sign to position
	or	a1,a1,a4		// add sign to mantissa
9:	ret

// ---- zero or small angle
// Here is:
//  A1:A0 = input argument x
//  A2:(A0) = absolute value of the number x (clear sign bit)
//  A3 = unsigned exponent
//  A4 = sign 0=positive, -1=negative
//  T4 = flag 0=atan(), 1=acotan()
//  T5 = flag 0=radians, 1=degrees

_asm_atan_0:
	// push return address
	add	sp,sp,-16		// space in the stack
	sw	ra,0(sp)		// save return address

	// convert to degrees
	beqz	a3,1f			// skip if argument is zero (unsigned exponent = 0)		
	beqz	t5,1f			// skip if radians (T5 = flag 0=radians, 1=degrees)

	sw	t4,4(sp)		// save T4 (flag 0=atan(), 1=acotan())
	sw	t5,8(sp)		// save T5 (flag 0=radians, 1=degrees)
	sw	a3,12(sp)		// save A3 (unsigned exponent)
	_asm_rad2deg			// convert radians to degrees
	lw	a3,12(sp)		// restore A3 (unsigned exponent)
	lw	t5,8(sp)		// restore T5 (flag 0=radians, 1=degrees)
	lw	t4,4(sp)		// restore T4 (flag 0=atan(), 1=acotan())

	// atan() - return x
1:	beqz	t4,9f			// skip if atan() (T4 = flag 0=atan(), 1=acotan())

	binvi	a1,a1,31		// negate x

	li	a3,0x40568000		// A3:A2 <- +90 deg
	li	a2,0x00000000
	bnez	t5,2f			// skip if degrees (T5 = flag 0=radians, 1=degrees)
	li	a3,0x3FF921FB		// A3:A2 <- +pi/2 radians
	li	a2,0x54442D18

2:	beqz	a3,9f			// skip if argument is zero (unsigned exponent = 0)		
	call	dadd			// subtract x from constant

	// exit
9:	lw	ra,0(sp)		// restore return address
	add	sp,sp,16		// destroy space in the stack
	ret

// ---- infinity or big value
// acotan(x) = pi/2 - atan(x)

// Here is:
//  A1:A0 = input argument x
//  A2:(A0) = absolute value of the number x (clear sign bit)
//  A3 = signed exponent
//  A4 = sign 0=positive, -1=negative
//  T4 = flag 0=atan(), 1=acotan()
//  T5 = flag 0=radians, 1=degrees

_asm_atan_inf:

	// push return address
	add	sp,sp,-16		// space in the stack
	sw	ra,0(sp)		// save return address

	// acotan()
	beqz	t4,5f			// skip if atan() (T4 = flag 0=atan(), 1=acotan())

	// acotan() positive value
	bnez	a4,3f			// skip if negative (A4 = sign 0=positive, -1=negative)

	// positive infinity - return x
	beqz	t5,1f			// skip if radians (T5 = flag 0=radians, 1=degrees)
	_asm_drec			// calculate 1/x
	_asm_rad2deg			// convert radians to degrees
	j	9f
1:	_asm_drec			// calculate 1/x
	j	9f

	// acotan() negative value
3:	li	t1,0x40668000		// T1:T0 <- 180 degrees
	li	t0,0x00000000
	bnez	t5,1f			// skip if degrees (T5 = flag 0=radians, 1=degrees)
	li	t1,0x400921FB		// T1:T0 <- pi radians
	li	t0,0x54442D18

1:	li	t2,52			// minimal exponent
	bge	a3,t2,8f		// skip if big exponent - result is only 180 or pi

	sw	t0,4(sp)		// save T0
	sw	t1,8(sp)		// save T1
	beqz	t5,1f			// skip if radians (T5 = flag 0=radians, 1=degrees)
	_asm_drec			// calculate 1/x
	_asm_rad2deg			// convert radians to degrees
	j	2f
1:	_asm_drec			// calculate 1/x
2:	lw	a2,4(sp)		// get constant -> A3:A2
	lw	a3,8(sp)
	call	dadd			// add constant
	j	9f

	// atan()
5:	li	t1,0x40568000		// T1:T0 <- 90 degrees
	li	t0,0x00000000
	bnez	t5,1f			// skip if degrees (T5 = flag 0=radians, 1=degrees)
	li	t1,0x3FF921FB		// T1:T0 <- pi/2 radians
	li	t0,0x54442D18
1:	slli	a4,a4,31		// shift sign to bit 31
	or	t1,t1,a4		// add sign

	li	t2,52			// minimal exponent
	bge	a3,t2,8f		// skip if big exponent - result is only 90 or pi/2

	sw	t0,4(sp)		// save T0
	sw	t1,8(sp)		// save T1
	beqz	t5,1f			// skip if radians (T5 = flag 0=radians, 1=degrees)
	_asm_drec			// calculate 1/x
	_asm_rad2deg			// convert radians to degrees
	j	2f
1:	_asm_drec			// calculate 1/x
2:	binvi	a1,a1,31		// negate result A1:A0
	lw	a2,4(sp)		// get constant -> A3:A2
	lw	a3,8(sp)
	call	dadd			// add constant
	j	9f

	// result is in T1:T0
8:	mv	a0,t0			// A0 <- result
	mv	a1,t1

	// exit
9:	lw	ra,0(sp)		// restore return address
	add	sp,sp,16		// destroy space in the stack
	ret

double_section _atan_cheb_tab
.p2align 2,,

// atan table of constants
_atan_tab:
	.word	0x9EDCF30E, 0xFFFFFFFD		// 0x00: coeff[15] = -10219621618 =		-2.2160271920363650519e-009 << 62
	.word	0x9FA34B68, 0x00000007	 	// 0x08: coeff[14] =  32743050088 =		 7.1000172079161104595e-009 << 62
	.word	0x77A2B727, 0xFFFFFFE7		// 0x10: coeff[13] = -105367029977 =		-2.2847832561836509202e-008 << 62
	.word	0x64443BAC, 0x0000004F		// 0x18: coeff[12] =  340984609708 =		 7.3939250925920962694e-008 << 62
	.word	0x77300FB7, 0xFFFFFEFD		// 0x20: coeff[11] = -1110396891209 =		-2.4077894435402600012e-007 << 62
	.word	0x1CE02B89, 0x00000350		// 0x28: coeff[10] =  3642616720265 =		 7.8986659232866866394e-007 << 62
	.word	0x2769262E, 0xFFFFF509		// 0x30: coeff[9]  = -12055311997394 =		-2.6140790915131184429e-006 << 62
	.word	0xCE51BEE6, 0x000024AD		// 0x38: coeff[8]  =  40328909405926 =		 8.7449382383752058361e-006 << 62
	.word	0x83311FBF, 0xFFFF83A3		// 0x40: coeff[7]  = -136736672768065 =		-2.9650039534715038092e-005 << 62
	.word	0x9F10294A, 0x0001ACFA		// 0x48: coeff[6]  =  471667387148618 =		 1.0227656116741852865e-004 << 62
	.word	0x6F6EDE24, 0xFFFA15EE		// 0x50: coeff[5]  = -1664736044327388 =	-3.6098208717493585339e-004 << 62
	.word	0xA83D0C74, 0x00158DF7		// 0x58: coeff[4]  =  6067069329935476 =	 1.3155859496272433493e-003 << 62
	.word	0x46F38D72, 0xFFAD9307		// 0x60: coeff[3]  = -23200763602563726 =	-5.0308636602444420689e-003 << 62
	.word	0xBFB7B522, 0x0155A300		// 0x68: coeff[2]  =  96162190670148898 =	 2.0851851207108148171e-002 << 62
	.word	0x981489DC, 0xF97ADACF		// 0x70: coeff[1]  = -469822625962227236 =	-1.0187654235021826540e-001 << 62
	.word	0x7A8196F2, 0x37B4F50B		// 0x78: coeff[0]  =  4014102597542713074 =	 8.7041975136710319745e-001 << 62
_atan_tab_end:
	.word	-3919405 - 45			// 0x80: coeff[23] = -3919405 =			-4.2494271502825301290e-013 << 63
	.word	6124579 + 5			// 0x84: coeff[22] =  6124579 =			 1.3280563277112973164e-012 << 62
	.word	-7416449			// 0x88: coeff[21] = -7416449 =			-1.6081860139168029037e-012 << 62
	.word	23399569			// 0x8C: coeff[20] =  23399569 =		 5.0739727982536712427e-012 << 62
	.word	-104884213			// 0x90: coeff[19] = -104884213 =		-2.2743138322260129705e-011 << 62
	.word	331273572			// 0x94: coeff[18] =  331273572 =		 7.1833505384564991396e-011 << 62
	.word	-1002726264			// 0x98: coeff[17] = -1002726264 =		-2.1743159875934996148e-010 << 62
	.word	3188444889			// 0x9C: coeff[16] =  3188444889 =		 6.9138377506130279270e-010 << 62

// ----------------------------------------------------------------------------
//                       asin(x)... - Chebyshev approximation
// ----------------------------------------------------------------------------
// double asin(double x);
// double asin_deg(double x);
// double acos(double x);
// double acos_deg(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A7, T0..T6

double_section _asin
.p2align 2,,

// arc cosine in degrees (result is 0 .. 180)
.global acos_deg
acos_deg:
	li	t5,1			// T5 <- flag 0=radians, 1=degrees
	li	t4,1			// T4 <- flag 0=asin(), 1=acos()

	// get sign -> A4
	srai	a4,a1,31		// A4 <- sign 0=positive, -1=negative
	slli	a4,a4,31		// shift sign back to position

	// absolute value of the number -> A2:(A0)
	bclri	a2,a1,31		// A2 <- clear sign bit of the number

	// acos() +1 returns 0
	// acos() -1 returns 180
	bnez	a0,4f			// check 1.0 = 0x3ff0000000000000
	li	t0,0x3ff00000
	bne	a2,t0,4f

	li	a1,0			// 0 rad
	beqz	a4,2f			// skip if positive
	li	a1,0x40668000		// 180 deg
2:	ret

	// get unsigned exponent -> A3
4:	srli	a3,a2,20		// A3 <- unsigned exponent

	// zero or small value - acos(0) returns +90 deg
	li	t0,DOUBLE_EXP1-53
	bge	a3,t0,_asm_asincos	// skip if not small value
	li	a1,0x40568000		// +90 deg
	li	a0,0
	ret

// arc cosine in radians (result is 0 .. pi)
.global __wrap_acos
.global acos
__wrap_acos:
acos:
	li	t5,0			// T5 <- flag 0=radians, 1=degrees
	li	t4,1			// T4 <- flag 0=asin(), 1=acos()

	// get sign -> A4
	srai	a4,a1,31		// A4 <- sign 0=positive, -1=negative
	slli	a4,a4,31		// shift sign back to position

	// absolute value of the number -> A2:(A0)
	bclri	a2,a1,31		// A2 <- clear sign bit of the number

	// acos() +1 returns 0
	// acos() -1 returns pi
	bnez	a0,4f			// check 1.0 = 0x3ff0000000000000
	li	t0,0x3ff00000
	bne	a2,t0,4f

	li	a1,0			// 0 rad
	beqz	a4,2f			// skip if positive
	li	a1,0x400921FB		// pi rad
	li	a0,0x54442D18
2:	ret

	// get unsigned exponent -> A3
4:	srli	a3,a2,20		// A3 <- unsigned exponent

	// zero or small value - acos(0) returns +pi/2 rad
	li	t0,DOUBLE_EXP1-54
	bge	a3,t0,_asm_asincos	// skip if not small value
	li	a1,0x3FF921FB		// +pi/2 rad
	li	a0,0x54442D18
	ret

// arc sine in degrees (result is -90 .. +90)
.global asin_deg
asin_deg:
	li	t5,1			// T5 <- flag 0=radians, 1=degrees

	// get sign -> A4
	srai	a4,a1,31		// A4 <- sign 0=positive, -1=negative
	slli	a4,a4,31		// shift sign back to position

	// absolute value of the number -> A2:(A0)
	bclri	a2,a1,31		// A2 <- clear sign bit of the number

	// asin() +-1 returns +-90 deg
	bnez	a0,_asin2		// check 1.0 = 0x3ff0000000000000
	li	t0,0x3ff00000
	bne	a2,t0,_asin2

	li	a1,0x40568000
	or	a1,a1,a4		// add sign
	ret

// arc sine in radians (result is -pi/2 .. +pi/2)
.global __wrap_asin
.global asin
__wrap_asin:
asin:
	li	t5,0			// T5 <- flag 0=radians, 1=degrees

	// get sign -> A4
	srai	a4,a1,31		// A4 <- sign 0=positive, -1=negative
	slli	a4,a4,31		// shift sign back to position

	// absolute value of the number -> A2:(A0)
	bclri	a2,a1,31		// A2 <- clear sign bit of the number

	// asin() +-1 returns +-pi/2 rad
	bnez	a0,_asin2		// check 1.0 = 0x3ff0000000000000
	li	t0,0x3ff00000
	bne	a2,t0,_asin2

	li	a1,0x3FF921FB
	li	a0,0x54442D18
	or	a1,a1,a4		// add sign
	ret

_asin2:	li	t4,0			// T4 <- flag 0=asin(), 1=acos()

	// get unsigned exponent -> A3
	srli	a3,a2,20		// A3 <- unsigned exponent

	// hack for small angles
	// - Simplified Taylor serie is used
	//  asin(x) = x + x^3/2/3 + x^5*3/2/4/5 + x^7*3*5/2/4/6/7 + ...
	li	t0,DOUBLE_EXP1-7
	bge	a3,t0,_asm_asincos	// skip if not small angle exp < -7

// ----- small asin()
// - Simplified Taylor serie is used: asin(x) = x + x^3/2/3 + x^5*3/2/4/5 + x^7*3*5/2/4/6/7 + ...
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result
// TRASHED: A2..A7,T0..T4

// SP+0: (4) original return address
// SP+4: (4) saved exponent
// SP+8: (8) saved asin() result
// SP+16: (8) saved x member of Taylor serie
// SP+24: (8) saved original x
// SP+32: (8) saved x*x
// SP+40: (4) saved T5 flag 0=radians, 1=degrees

	// push registers
	addi	sp,sp,-48
	sw	ra,(sp)			// save return address
	sw	t5,40(sp)		// saved T5 flag 0=radians, 1=degrees

// ---- exp < -26, asin(x) = x

	// very small angle - result is simple x
	li	t0,DOUBLE_EXP1-26
	blt	a3,t0,8f		// skip if angle is exp < -26

// ---- -16 <= exp < -13, asin(x) = x + x^3/2/3

	// exps < -13 requires 2 Taylor members
	// - SP+4: (4) saved exponent
	sw	a3,4(sp)		// save exponent

	// save x
	// - SP+24: (8) saved original x
	sw	a0,24(sp)		// save x LOW
	sw	a1,28(sp)		// save x HIGH

	// x*x
	mv	a2,a0
	mv	a3,a1
	call	dmul			// x*x
	// - SP+32: (8) saved x*x
	sw	a0,32(sp)		// save x*x LOW	
	sw	a1,36(sp)		// save x*x HIGH

	// x^3
	// - SP+24: (8) saved original x
	lw	a2,24(sp)		// x LOW
	lw	a3,28(sp)		// x HIGH
	call	dmul			// x*x*x
	// - SP+16: (8) saved x member of Taylor serie
	sw	a0,16(sp)		// save x member LOW
	sw	a1,20(sp)		// save x member HIGH

	// x^3/(2*3)
	li	a3,0x3fc55555		// 1/(2*3) = 1/6 = 0.16666666666666665
	li	a2,0x55555555
	call	dmul			// x^3/(2*3)

	// x + x^3/2/3
	// - SP+24: (8) saved original x
	lw	a2,24(sp)		// get x LOW
	lw	a3,28(sp)		// get x HIGH
	call	dadd			// result x + x^3/2/3

	// check exps < -13
	li	t0,DOUBLE_EXP1-13
	// - SP+4: (4) saved exponent
	lw	a4,4(sp)		// restore exponent
	blt	a4,t0,8f		// small number exps < -13

// ---- -13 <= exp < -9, asin(x) = x + x^3/2/3 + x^5*3/2/4/5

	// exps < -9 requires 3 Taylor members
	// - SP+8: (8) saved asin() result
	sw	a0,8(sp)		// save result LOW
	sw	a1,12(sp)		// save result HIGH

	// x^5
	// - SP+16: (8) saved x member of Taylor serie
	// - SP+32: (8) saved x*x
	lw	a0,16(sp)		// restore x member LOW
	lw	a1,20(sp)		// restore x member HIGH
	lw	a2,32(sp)		// restore x*x LOW	
	lw	a3,36(sp)		// restore x*x HIGH
	call	dmul			// x^5
	// - SP+16: (8) saved x member of Taylor serie
	sw	a0,16(sp)		// save x member LOW
	sw	a1,20(sp)		// save x member HIGH

	// x^5*3/2/4/5
	li	a3,0x3fb33333		// 3/2/4/5 = 3/40 = 0.075
	li	a2,0x33333333
	call	dmul			// x^3/(2*3)

	// x + x^3/2/3 + x^5*3/2/4/5
	// - SP+8: (8) saved asin() result
	lw	a2,8(sp)		// load result LOW
	lw	a3,12(sp)		// load result HIGH
	call	dadd			// result x + x^3/2/3 + x^5*3/2/4/5

	// check exps < -9
	li	t0,DOUBLE_EXP1-9
	// - SP+4: (4) saved exponent
	lw	a4,4(sp)		// restore exponent
	blt	a4,t0,8f		// small number exps < -9

// ---- -9 <= exp < -7, asin(x) = x + x^3/2/3 + x^5*3/2/4/5 + x^7*3*5/2/4/6/7

	// exps < -7 requires 4 Taylor members
	// - SP+8: (8) saved asin() result
	sw	a0,8(sp)		// save result LOW
	sw	a1,12(sp)		// save result HIGH

	// x^7
	// - SP+16: (8) saved x member of Taylor serie
	// - SP+32: (8) saved x*x
	lw	a0,16(sp)		// restore x member LOW
	lw	a1,20(sp)		// restore x member HIGH
	lw	a2,32(sp)		// restore x*x LOW	
	lw	a3,36(sp)		// restore x*x HIGH
	call	dmul			// x^5

	// x^7*3*5/2/4/6/7
	li	a3,0x3fa6db6d		// 3*5/2/4/6/7 = 15/336 = 0.04464285714285714286
	li	a2,0xb6db6db7
	call	dmul			// x^3/(2*3)

	// x + x^3/2/3 + x^5*3/2/4/5 + x^7*3*5/2/4/6/7
	// - SP+8: (8) saved asin() result
	lw	a2,8(sp)		// load result LOW
	lw	a3,12(sp)		// load result HIGH
	call	dadd			// result x + x^3/2/3 + x^5*3/2/4/5 + x^7*3*5/2/4/6/7

	// convert rad to deg
8:	lw	t5,40(sp)		// saved T5 flag 0=radians, 1=degrees
	beqz	t5,9f			// skip if not converting

	li	a3,0x404ca5dc		// 180/pi = 57.295779513082320877
	li	a2,0x1a63c1f8
	call	dmul

	// pop registers
9:	lw	ra,(sp)			// load return address
	addi	sp,sp,48		// clear stack
	ret

// arc sine/cosine
// INPUT: A1:A0 = argument X
//	  A3 = unsigned exponent
//	  A4 = sign 0=positive, B31=negative
//	  T4 = flag 0=asin(), 1=acos()
//	  T5 = flag 0=radians, 1=degrees
// OUTPUT: A1:A0 = result
// TRASHED: A2..A7, T0..T6
_asm_asincos:

	// absolute value of the number -> A1:A0
	bclri	a1,a1,31		// A1 <- clear sign bit of the number

	// get signed exponent -> A3
	addi	a3,a3,-DOUBLE_EXP1	// subtract exponent bias

	// invalid input value > 1.0 (case 1.0 has been serviced before)
	bgez	a3,_asm_asincos_nan	// skip if invalid value

	// if > 0.70710678118654755 (= sqrt(0.5)), use alternate method sqrt(1 - x^2)
	li	t6,0			// T6 <- 0 flag 0=no alternative
	li	t0,0x3FE6A09E		// sqrt(0.5) HIGH
	bltu	a1,t0,2f		// < sqrt(0.5)
	bgtu	a1,t0,1f		// > sqrt(0.5)
	li	t0,0x667F3BCD		// sqrt(0.5) LOW
	bleu	a0,t0,2f		// <= sqrt(0.5)

	// save registers
1:	addi	sp,sp,-16
	sw	ra,0(sp)		// save return address
	sw	a4,4(sp)		// save sign 0=positive, B31=negative
	sw	t4,8(sp)		// save flag 0=asin(), 1=acos()
	sw	t5,12(sp)		// save flag 0=radians, 1=degrees

	// x^2
	mv	a3,a1
	mv	a2,a0
	call	dmul			// A1:A0 <- x*x

	// 1 - x^2
	binvi	a1,a1,31		// negate -x*x
	li	a3,0x3ff00000		// A3:A2 <- 1.0
	li	a2,0
	call	dadd			// 1 - x*x

	// sqrt
	call	sqrt

	// get signed exponent -> A3
	srli	a3,a1,20		// A3 <- unsigned exponent
	addi	a3,a3,-DOUBLE_EXP1	// subtract exponent bias

	// restore registers
	lw	t5,12(sp)		// restore flag 0=radians, 1=degrees
	lw	t4,8(sp)		// restore flag 0=asin(), 1=acos()
	lw	a4,4(sp)		// restore sign 0=positive, B31=negative
	lw	ra,0(sp)		// resture return address
	addi	sp,sp,16	
	li	t6,-1			// T6 <- -1=alternative method

// x here is 0..0.7071 (= sqrt(0.5)), exponent is <= -1

// Here is:
//  A1:A0 = argument x (absolute value)
//  A3 = signed exponent
//  A4 = sign 0=positive, B31=negative
//  T4 = flag 0=asin(), 1=acos()
//  T5 = flag 0=radians, 1=degrees
//  T6 = flag 0=no alternative, -1=alternative method

	// normalize mantissa A1:A0 - shift to max position 1.63 and restore hidden bit '1'
	// xn at max: 0xB504F333F9DE6800
2:	slli	a1,a1,11		// A1 <- shift mantissa HIGH << 11 to destroy exponent
	bset	a1,a1,31		// set hidden bit "1"
	srli	t1,a0,21		// T1 <- carry from mantissa LOW
	or	a1,a1,t1		// add carry from mantissa LOW to mantissa HIGH
	slli	a0,a0,11		// A0 <- shift mantissa LOW << 11

// Consistency checks:
//  (no alt) x = 0.70710678118654746 0x3FE6A09E667F3BCC -> xn = 0xB504F333F9DE6000
//  (no alt) x = 0.70710678118654755 0x3FE6A09E667F3BCD -> xn = 0xB504F333F9DE6800
//  (alt)    x = 0.70710678118654772 0x3FE6A09E667F3BCE -> xn = 0xB504F333F9DE5800
//  (alt)    x = 0.70710678118654781 0x3FE6A09E667F3BCF -> xn = 0xB504F333F9DE5000

	// shift mantissa A1:A0 right by exponent bits, to get fixed number in format 1.63
	//  mantissa will be in range 0..0xB504F333F9DE6800 = 0..13043817825332783104
	// xn >>= -exp - 1; // number of shifts >= 0
	neg	t0,a3			// T0 <- number of shifts (= -exp)
	addi	t0,t0,-1		// T0 <- number of shifts >= 0
	addi	t1,t0,-32		// T1 <- number of shifts - 32
	bgez	t1,2f			// number of shifts is >= 32
	blez	t0,4f			// skip if number of shifts = 0

	// number of shifts = 1..31
	neg	t1,t1			// T1 <- 32 - number of shifts
	srl	a0,a0,t0		// shift mantissa LOW
	sll	t1,a1,t1		// T1 <- carry from mantissa HIGH
	or	a0,a0,t1		// add carry from mantissa HIGH to mantissa LOW
	srl	a1,a1,t0		// shift mantissa HIGH
	j	4f

	// number of shifts >= 64
1:	li	a0,0
	j	3f

	// number of shifts = 32..63
2:	addi	t0,t1,-32		// T0 <- number of shifts - 64
	bgez	t0,1b			// skip if number of shifts >= 64
	srl	a0,a1,t1		// A0 <- shift mantissa HIGH
3:	li	a1,0			// A1 <- mantissa HIGH = 0

	// convert A1:A0 to range 0..1 in format 2.62
	// - table of Chebyshev constants are prepared for multiply by 8/3 = 2.66666.. << 62 = 12297829382473034410 = 0xAAAAAAAAAAAAAAAB
	//   (otherwise should multiply by 1/sqrt(0.5) = sqrt(2) = 1.4142135623730950488 << 63 = 13043817825332782212 = 0xB504F333F9DE6484)
	//  mantissa will be in range 0..8695878550221855401 = 0..0x78ADF777FBE99AA9
4:	li	a3,0xAAAAAAAA
	li	a2,0xAAAAAAAB

// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high a0,a1,a2,a3,t0,t1

	// convert A1:A0 to range -1..+1 in signed format 2.62 -> T2:A2 (subtract 1)
        // - offset is theoretically 0x4000000000000000, empirically refined to 0x3ffffffffffffffe
	li	t1,0x3fffffff
	li	t0,0xfffffffe
	sub	a2,a0,t0
	sltu	t0,a0,a2
	sub	a3,a1,t1
	sub	t2,a3,t0

	// prepare pointer to data table -> A5
	la	a5,_asin_tab		// A5 <- start of table

	// shift T2:A2 mantissa << 2 -> A3:A2
	srli	t0,a2,32-2
	slli	a2,a2,2
	slli	a3,t2,2
	or	a3,a3,t0

	// load first constant -> A1:A0
	lw	a0,(a5)
	li	a1,0

// ---- add coefficients in case of positive mantissa, 1st part (slow mode, with negative acc)

	// check positive mantissa
	bltz	t2,4f				// skip if mantissa is negative

	// end of table, 1st part
	la	t3,_asin_tab_end2 - 8		// T3 <- end of table

	// absolute value of the accumulator A1:A0
2:	mv	t2,a1				// T2 <- save acc HIGH, for later check sign
	bgez	t2,1f				// skip if mantissa is not negative
	not	a0,a0				// use NOT instead of NEG
	not	a1,a1

	// multiply
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
1:	mulu64high a0,a1,a2,a3,t0,t1

	// negate result
	// - no carry from LOW, so we use more common case NOT instead of NEG
	bgez	t2,1f				// skip if mantissa was not negative
	not	a0,a0
	not	a1,a1

	// load next constant -> T1:T0
1:	addi	a5,a5,8				// shift table pointer
	lw	t0,0x00(a5)			// T0 <- coeff low
	lw	t1,0x04(a5)			// T1 <- coeff high

	// add coeff T1:T0 to acc A1:A0
	add	a0,a0,t0			// add coeff LOW
	sltu	t0,a0,t0			// T0 <- carry
	add	a1,a1,t1			// add coeff HIGH
	add	a1,a1,t0			// add carry

	// loop
	bne	a5,t3,2b			// next coeff

// ---- add coefficients in case of positive mantissa, 2nd part (fast mode, with positive acc)

	// end of table, 2nd part
	la	t3,_asin_tab_end - 8		// T3 <- end of table

	// multiply
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
2:	mulu64high a0,a1,a2,a3,t0,t1

	// load next constant -> T1:T0
	addi	a5,a5,8				// shift table pointer
	lw	t0,0x00(a5)			// T0 <- coeff low
	lw	t1,0x04(a5)			// T1 <- coeff high

	// add coeff T1:T0 to acc A1:A0
	add	a0,a0,t0			// add coeff LOW
	sltu	t0,a0,t0			// T0 <- carry
	add	a1,a1,t1			// add coeff HIGH
	add	a1,a1,t0			// add carry

	// loop
	bne	a5,t3,2b			// next coeff
	j	8f

// ---- add coefficients in case of negative mantissa

	// end of table
4:	la	t3,_asin_tab_end - 8		// T3 <- end of table

	// negate mantissa A3:A2 - use faster NOT instead of NEG
	not	a2,a2
	not	a3,a3

	// absolute value of the accumulator A1:A0
2:	mv	t2,a1				// T6 <- save acc HIGH, for later check sign
	bgez	t2,1f				// skip if mantissa is not negative
	not	a0,a0				// use NOT instead of NEG
	not	a1,a1

	// multiply
// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
1:	mulu64high a0,a1,a2,a3,t0,t1

	// negate result
	// - no carry from LOW, so we use more common case NOT instead of NEG
	bltz	t2,1f				// skip if mantissa was negative, but x was negative too
	not	a0,a0
	not	a1,a1

	// load next constant
1:	addi	a5,a5,8				// shift table pointer
	lw	t0,0x00(a5)			// T0 <- coeff low
	lw	t1,0x04(a5)			// T1 <- coeff high

	// add coeff T1:T0 to acc A1:A0
	add	a0,a0,t0			// add coeff LOW
	sltu	t0,a0,t0			// T0 <- carry
	add	a1,a1,t1			// add coeff HIGH
	add	a1,a1,t0			// add carry

	// loop
	bne	a5,t3,2b			// next coeff

// Checks limits and consistency of approximation:
//   (-) m = -0x3ffffffffffffffe -> acc = 0xFFFFFFFFFFFFFFFC
//   (-) m = -0x3ffffffffffffff5 -> acc = 0x0000000000000000
//   (-) m = -0x2000000000000000 -> acc = 0x0C124A76873C7DD8
//   (-) m = -0x0000000000000001 -> acc = 0x1899F4EDC962D304
//   (+) m = +0x0000000000000000 -> acc = 0x1899F4EDC962D305
//   (+) m = +0x2000000000000000 -> acc = 0x263BE81F7A639A04
//   (+) m = +0x38ADF777FBE99AAB -> acc = 0x3243F6A8885A31BE

	// check underflow
8:	bgez	a1,1f				// no underflow
	li	a1,0
	li	a0,0

// acc value is in range 0..pi/2 in 3.61 format (= pi/2 << 61 = 0..0x3243F6A8885A308D, practically 0..0x3243F6A8885A31BF)

	// prepare exponent -> T3
1:	li	t3,DOUBLE_EXP1 + 2

// Here is:
//  A1:A0 = result
//  A4 = sign 0=positive, B31=negative
//  T3 = signed exponent
//  T4 = flag 0=asin(), 1=acos()
//  T5 = flag 0=radians, 1=degrees
//  T6 = flag 0=no alternative, -1=alternative method

	// correction for alternate method
	// x = pi/2 - x
	// pi/2 << 62 = 0x6487ED5110B4611A = 7244019458077122842 (+1)
	beqz	t6,1f			// skip if not alternate
	li	t1,0x6487ED51
	li	t0,0x10B4611B
	sub	a0,t0,a0
	sltu	t0,t0,a0
	sub	a1,t1,a1
	sub	a1,a1,t0

// Consistency checks:
//  (no alt) x = 0.70710678118654746 0x3FE6A09E667F3BCC -> acc = 0x3243F6A8885A2EEA
//  (no alt) x = 0.70710678118654755 0x3FE6A09E667F3BCD -> acc = 0x3243F6A8885A31BE
//  (alt)    x = 0.70710678118654772 0x3FE6A09E667F3BCE -> acc = 0x3243F6A8885A3504
//  (alt)    x = 0.70710678118654781 0x3FE6A09E667F3BCF -> acc = 0x3243F6A8885A37DB

	// acos correction
1:	beqz	t4,3f			// skip if no acos()

	// acos(x) = pi/2 - asin(x)
	bnez	a4,1f			// skip if negative
	not	a0,a0			// use faster NOT instead of NEG
	not	a1,a1
1:	li	a4,0			// result will be positive

	li	t1,0x6487ED51
	li	t0,0x10B4611A
	add	a0,a0,t0
	sltu	t0,a0,t0
	add	a1,a1,t1
	add	a1,a1,t0

	// convert to degrees: x = x * (180/PI)
3:	beqz	t5,1f			// skip if radians

	// x = x * (180/PI)
	// ... (180/PI << 64) >> 6 = 16514384080961256899 + 14
	li	a3,0xE52EE0D3
	li	a2,0x1E0FBDD1

// multiply (u64*u64->u128)>>64 simplified (no carry from LOW)
// aL ... 1st operand LOW, result LOW
// aH ... 1st operand HIGH, result HIGH
// bL ... 2nd operand LOW
// bH ... 2nd operand HIGH
// tmp1, tmp2 ... temporary, trashed
//.macro mulu64high aL, aH, bL, bH, tmp1, tmp2
	mulu64high a0,a1,a2,a3,t0,t1

	addi	t3,t3,6			// exponent correction

	// zero result
1:	or	t0,a0,a1
	beqz	t0,9f

	// prepare number of shifts -> T1
	bnez	a1,1f			// skip if mantissa HIGH is not 0
	clz	t1,a0			// T1 <- count leading zeroes LOW
	addi	t1,t1,32
	j	2f
1:	clz	t1,a1			// T1 <- count leading zeroes HIGH
2:	sub	t3,t3,t1		// compensate exponent

	// normalize mantissa to max. position 1.63 (shift A1:A0 << T1)
	li	t2,32
	sub	t0,t1,t2		// T0 <- number of shifts - 32
	bgez	t0,2f			// number of shifts is >= 32
	beqz	t1,3f			// skip if number of shifts = 0

	sub	t0,t2,t1		// T0 <- shift inversion = 32 - shifts = 1..31 (T1 = 1..31)
	sll	a1,a1,t1		// shift mantissa HIGH left by 1..31
	srl	t0,a0,t0		// T0 <- carry from mantissa LOW
	or	a1,a1,t0		// add carry from mantissa LOW to mantissa HIGH
	sll	a0,a0,t1		// shift mantissa LOW left by 1..31
	j	3f

2:	sll	a1,a0,t0		// A1 <- shift mantissa left
	li	a0,0			// A0 <- mantissa LOW = 0

	// rounding mantissa
3:	li	t0,0x400		// rounding correction
	add	a0,a0,t0
	sltu	t1,a0,t0
	add	a1,a1,t1

	// check overflow
	bnez	a1,1f			// no overflow
	blt	a0,t0,2f		// overflow

	// no overflow - remove leading bit '1'
1:	srli	t0,a0,31
	sh1add	a1,a1,t0
	slli	a0,a0,1
	addi	t3,t3,-1		// compensate exponent

	// shift mantissa A1:A0 to position >> 12
2:	srli	a0,a0,12
	slli	t0,a1,32-12
	srli	a1,a1,12
	or	a0,a0,t0

	// compose result
	slli	t3,t3,20		// shift exponent to position
	or	a1,a1,t3		// add exponent to mantissa
	or	a1,a1,a4		// add sign to mantissa
9:	ret

// ---- invalid input value > 1.0

_asm_asincos_nan:
	li	a1,0x7ff80000
	li	a0,0
	ret

double_section _asin_cheb_tab
.p2align 2,,

// asin table of constants
_asin_tab:
	.word	0x5F67FB9F, 0		// coeff[37] = 1600650139+4 =		 3.4708567164600090860e-010 << 62
	.word	0xA476FF07, 0		// coeff[36] = 2759261959 = 		 5.9831956207860754844e-010 << 62
	.word	0x9160DC6E, 0xFFFFFFFD	// coeff[35] = -10445857682 =		-2.2650843184651493206e-009 << 62
	.word	0xCF807110, 0xFFFFFFFB	// coeff[34] = -17993535216 =		-3.9017259944777314656e-009 << 62
	.word	0x447BC820, 0x00000008	// coeff[33] = 35508701216 =		 7.6997222001122337242e-009 << 62
	.word	0x3DE70495, 0x0000000E	// coeff[32] = 61168092309 =		 1.3263715713699009926e-008 << 62
	.word	0x2C416338, 0xFFFFFFEF	// coeff[31] = -72271961288 =		-1.5671483487560937465e-008 << 62
	.word	0x077F82B3, 0xFFFFFFE3	// coeff[30] = -124428254541 =		-2.6981076778459631638e-008 << 62
	.word	0x8AB5D380, 0x00000019	// coeff[29] = 109701354368 =		 2.3787689346113323827e-008 << 62
	.word	0x0293F2FA, 0x0000002C	// coeff[28] = 189021811450 =		 4.0987571724329514300e-008 << 62
	.word	0x6D6B29E9, 0xFFFFFFE8	// coeff[27] = -101243475479 =		-2.1953679212795056195e-008 << 62
	.word	0x84F8C96F, 0xFFFFFFD7	// coeff[26] = -173862762129 =		-3.7700476882996859937e-008 << 62
	.word	0x5C215D03, 0x0000001C	// coeff[25] = 121804774659 =		 2.6412200260974878764e-008 << 62
	.word	0x39A98051, 0x00000031	// coeff[24] = 211420807249 =		 4.5844579705514679850e-008 << 62
_asin_tab_end2:
	.word	0x3F169BC9, 0x0000000C	// coeff[23] = 52598053833 =		 1.1405384846815019950e-008 << 62
	.word	0x952D3EA7, 0x00000016	// coeff[22] = 96992050855 =		 2.1031798450064501118e-008 << 62
	.word	0xEE2A0FD2, 0x00000053	// coeff[21] = 360478019538 =		 7.8166210383357868318e-008 << 62
	.word	0x33AD54BE, 0x00000096	// coeff[20] = 645112091838 =		 1.3988638629347848066e-007 << 62
	.word	0x272653E7, 0x000000FB	// coeff[19] = 1078693614567 =		 2.3390439207198458051e-007 << 62
	.word	0x851E090A, 0x000001C6	// coeff[18] = 1952148490506 =		 4.2330472688418869549e-007 << 62
	.word	0xCF91F2DD, 0x00000340	// coeff[17] = 3576895238877 =		 7.7561551774871925733e-007 << 62
	.word	0xC555C9E0, 0x000005F1	// coeff[16] = 6535955991008 =		 1.4172595369440573676e-006 << 62
	.word	0xE7B76567, 0x00000AEB	// coeff[15] = 12008321148263 =		 2.6038895753700331244e-006 << 62
	.word	0x295D236C, 0x00001437	// coeff[14] = 22227149726572 =		 4.8197448043419372559e-006 << 62
	.word	0x693364D8, 0x000025B8	// coeff[13] = 41473969186008 =		 8.9932334986133469979e-006 << 62
	.word	0x2650C6A7, 0x00004703	// coeff[12] = 78078853301927 =		 1.6930652475025236884e-005 << 62
	.word	0x43E930AE, 0x00008736	// coeff[11] = 148667137339566 =	 3.2237046656151586703e-005 << 62
	.word	0xFBB63F73, 0x0001047D	// coeff[10] = 286414117158771 =	 6.2106161610811407942e-005 << 62
	.word	0x4263D2BD, 0x0001FF19	// coeff[ 9] = 561958929814205 =	 1.2185541851043802362e-004 << 62
	.word	0x4DEBBEA2, 0x0003F8C2	// coeff[ 8] = 1117938344771234 =	 2.4241423642116406379e-004 << 62
	.word	0x75DDA933, 0x0008336F	// coeff[ 7] = 2308353625532723 =	 5.0054440313347368686e-004 << 62
	.word	0xA6A43AF9, 0x00110694	// coeff[ 6] = 4792310124788473 =	 1.0391666097039881692e-003 << 62
	.word	0xE6DAD04F, 0x00273F8C	// coeff[ 5] = 11047398492786767 =	 2.3955226892384999515e-003 << 62
	.word	0xAC2C9710, 0x0054B355	// coeff[ 4] = 23841078585890576 =	 5.1697098394440400625e-003 << 62
	.word	0xAF4FFC93, 0x010D7CF3	// coeff[ 3] = 75854154795318419 =	 1.6448247884227194745e-002 << 62
	.word	0x831AB46D, 0x021E433B	// coeff[ 2] = 152633360258872429 =	 3.3097084157286428629e-002 << 62
	.word	0x097F481D, 0x19E3A782	// coeff[ 1] = 1865518847608440861 =	 4.0451991747794525175e-001 << 62
	.word	0xC962D305, 0x1899F4ED	// coeff[ 0] = 1772717230470124293 =	 3.8439677449563908304e-001 << 62
_asin_tab_end:

#endif // !USE_DOUBLELIBC
#endif // USE_DOUBLE
