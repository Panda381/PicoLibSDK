
// ****************************************************************************
//
//                              Double RP2350 - RISC-V
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040

// Copyright (c) 2020 Mark Owen https://www.quinapalus.com - GPLv2 licence

/*
 * Copyright (c) 2024 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// This code contains fast RISC-V functions:
//  dadd (libc: 674ns, fast: 489ns)
//  dsub (libc: 714ns, fast: 498ns)
//  dmul (libc: 1420ns, fast: 528ns)
//  dsqr (libc: 1420ns, fast: 538ns)
//  ddiv (libc: 2500ns, fast: 824ns)
//  drec (libc: 2590ns, fast: 827ns)

// >>> Functions are adaptations of the functions from Pico1 M0+ (c) Raspberry Pi <<<

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

#if !USE_DOUBLELIBC

// macro - unpack double to unsigned mantissa
// input: RB:RA = double number
// output: RB:RA = unsigned mantissa with hidden implied '1' set (53 significant bits)
//         RE = exponent ((-128 if zero, 1919 if infinity) << 12)
//         RS = sign (0 positive or 1 negative)
// trashes: RT
.macro mdunpack ra,rb,re,rs,rt

	// isolate sign -> RS
	srli	\rs,\rb,31	// isolate sign bit

	// isolate exponent -> RE
	srli	\re,\rb,20	// extract exponent and sign
	bclri	\re,\re,11	// clear sign bit
	beqz	\re,l\@_1	// exponent is zero

	// isolate mantissa and add hidden implied bit '1' -> RB:RA
	slli	\rb,\rb,11	// clear sign and exponent, 1 bit reserved
	bseti	\rb,\rb,31	// set hidden implied bit '1'
	srli	\rb,\rb,11	// shift mantissa back to position

	// check if exponent is infinity
	addi	\rt,\re,1	// exponent + 1
	srli	\rt,\rt,11	// check infinity (value == 0x800)
	beqz	\rt,l\@_2	// not infinity

l\@_1:
	// exponent is zero or infinity
	li	\ra,0		// number low = 0
	li	\rb,1<<20	// number high = 1
	addi	\re,\re,-128	// exponent = -128 if zero, or 1919 if infinity
	slli	\re,\re,12	// shift exponent to position 12
l\@_2:
.endm

// macro - unpack double to signed mantissa
// input: RB:RA = double number
// output: RB:RA = signed mantissa with hidden implied '1' set (53 significant bits)
//         RE = exponent ((-128 if zero, 1919 if infinity) << 12)
// trashes: RT0, RT1
// +zero, +denormal -> exponent = -0x80000
// -zero, -denormal -> exponent = -0x80000
// +Inf, +NaN -> exponent = +0x77f000
// -Inf, -NaN -> exponent = +0x77e000
.macro mdunpacks ra,rb,re,rt0,rt1

	// extract sign and exponent -> RE
	srli	\re,\rb,20	// extract sign and exponent -> RE

	// extract sign -> RT1
	srli	\rt1,\rb,31

	// isolate mantissa (destroy exponent with sign) and add hidden implied bit '1'
	addi	\rt0,\re,-1	// exponent - 1 -> RT0 (to include implied 1)
	slli	\rt0,\rt0,20	// shift exponent-1 and sign back to position 20
	sub	\rb,\rb,\rt0	// clear sign and exponent in mantissa, insert implied 1

	// isolate exponent (destroy sign bit)
	slli	\re,\re,21	// shift exponent left, destroy sign bit
	beqz	\rt1,l\@_1	// skip if number is positive

	// number is negative, negate mantissa
	neg	\ra,\ra		// negate mantissa LOW
	not	\rb,\rb		// invert mantissa HIGH
	bnez	\ra,l\@_1	// skip if mantissa LOW is not zero
	addi	\rb,\rb,1	// carry to mantissa HIGH

l\@_1:
	// check if exponent is zero
	srli	\re,\re,21	// shift exponent back
	beqz	\re,l\@_2	// exponent is zero

	// check if exponent is infinity
	addi	\rt0,\re,1	// exponent + 1 -> RT0
	srli	\rt0,\rt0,11	// check infinity (value == 0x800)
	beqz	\rt0,l\@_3	// not infinity
	sub	\re,\re,\rt1	// exponent is infinity, subtract sign bit to get negative infinity

l\@_2:
	// exponent is zero or infinity
	li	\ra,0		// number low = 0
	slli	\rt1,\rt1,1	// +ve: 0  -ve: 2
	addi	\rb,\rt1,1	// +ve: 1  -ve: 3
	slli	\rb,\rb,30	// create +/-1 mantissa
	srai	\rb,\rb,10	// normalise mantissa
	addi	\re,\re,-128
	slli	\re,\re,12	// shift exponent to position 12
l\@_3:
.endm

// ----------------------------------------------------------------------------
//                         Subtraction, x - y
// ----------------------------------------------------------------------------
// double dsub(double x, double y);
// INPUT: A1:A0 = 1st argument X
//	  A3:A2 = 2nd argument Y
// OUTPUT: A1:A0 = result X - Y
// TRASHED: A2..A7, T0, T1

double_section __adddf3
.p2align 2,,

.global __wrap___subdf3
.global dsub
__wrap___subdf3:
dsub:

	// invert sign bit of 2nd argument Y
	binvi	a3,a3,31

// ---- continue to dadd

// ----------------------------------------------------------------------------
//                         Addition, x + y
// ----------------------------------------------------------------------------
// double dadd(double x, double y);
// INPUT: A1:A0 = 1st argument X
//	  A3:A2 = 2nd argument Y
// OUTPUT: A1:A0 = result X + Y
// TRASHED: A2..A7, T0, T1

.global __wrap___adddf3
.global dadd
__wrap___adddf3:
dadd:

	// unpack operands
// input: RB:RA = double number
// output: RB:RA = signed mantissa with hidden implied '1' set (53 significant bits)
//         RE = exponent (-128 if zero, 1919 if infinity)
// trashes: RT0, RT1
// +zero, +denormal -> exponent = -0x80000
// -zero, -denormal -> exponent = -0x80000
// +Inf, +NaN -> exponent = +0x77f000
// -Inf, -NaN -> exponent = +0x77e000
//.macro mdunpacks ra,rb,re,rt0,rt1

	mdunpacks a0,a1,a4,a6,a7 // unpack double x A1:A0 to mantissa xm A1:A0 and exponent xe A4, trashes A6, A7
	mdunpacks a2,a3,a5,a6,a7 // unpack double y A3:A2 to mantissa ym A3:A2 and exponent ye A5, trashes A6, A7

	// difference of exponents
	sub	a7,a5,a4	// ye - xe
	sub	a6,a4,a5	// xe - ye
	bltz	a6,da_ygtx	// y > x

	// xe = ye
	mv	t0,a4		// save exponent xe -> T0
	beqz	a6,da_yeqx	// y = x

	// here xe >= ye: need to shift y down A6 places
	li	t1,32 		// number of shifts > 32 ?
	bge	a6,t1,da_xrgty	// xe is rather greater than ye

	// prepare sticky bits of ym
	addi	a7,a7,32	// A7 <- 32 + ye - xe
	sll	a4,a2,a7	// A4 <- rounding bit of ym low + sticky bits

da_xgty0:
	// shift mantissa ym right, to be same exponent as x
	mv	a5,a3		// A5 <- mantissa ym high
	sll	a5,a5,a7	// A5 <- shift mantissa ym high left
	srl	a2,a2,a6	// shift mantissa ym low right
	sra	a3,a3,a6	// shift mantissa ym high right
	or	a2,a2,a5	// add carry from mantissa ym high to mantissa ym low

da_add:
	// sum mantissas x += y (A4 contains sticky bits)
	add	a0,a0,a2
	sltu	t1,a0,a2
	add	a1,a1,a3
	add	a1,a1,t1	// carry

da_pack:  // Pack result back ... here is: A1:A0 mantissa, T0 exponent, A4 rounding

	// get sign bit -> R3
	srli	a3,a1,31	// get sign bit -> A3
	beqz	a3,1f		// result is positive

	// absolute value of the mantissa (negate mantissa A1:A0:A4)
	not	a1,a1		// not mantissa high
	not	a0,a0		// not mantissa low
	neg	a4,a4		// negate mantissa sticky bits
	bnez	a4,1f
	addi	a0,a0,1		// carry to mantissa low
	bnez	a0,1f
	addi	a1,a1,1		// carry to mantissa high

	// check mantissa overflow - shift down required
1:	mv	a2,t0		// exponent -> A2
	srli	a5,a1,21	// check highest 11 bits of the mantissa, if overflow
	bnez	a5,da_0		// mantissa is > 53 bits, shift down required

	// check if mantissa is normalised (must have 53 bits)
	srli	a5,a1,20	// get highest 12 bits of mantissa high
	bnez	a5,da_1		// mantissa already normalised

	// check if mantissa is zero (zero mantissa cannot be normalised)
	beqz	a0,da_5		// mantissa could be zero, check it

	// shift result mantissa left, to normalise
da_2:	slli	a1,a1,1		// mantissa high << 1
	srli	t1,a0,31	// T1 <- carry bit
	or	a1,a1,t1	// add carry

	slli	a0,a0,1		// mantissa low << 1
	srli	t1,a4,31	// T1 <- carry bit
	or	a0,a0,t1	// add carry

	slli	a4,a4,1		// sticky bits << 1

	// adjust exponent
	addi	a2,a2,-1	// eponent - 1
	srli	a5,a1,20	// get highest 12 bits of mantissa high, to check normalisation
	beqz	a5,da_2		// not normalised yet, shift left one more time

	// normalisation done, check rounding bit
da_1:	srli	t1,a4,31	// carry
	slli	a4,a4,1		// check rounding bit
	beqz	t1,da_3		// no rounding

	// round mantissa up
da_4:	addi	a0,a0,1		// add mantissa low + 1 to round up
	bnez	a0,2f		// no carry
	addi	a1,a1,1		// add mantissa high + 1

	// if sticky bits are = 0, round mantissa to even number
2:	bnez	a4,da_3		// sticky bits are not zero
	bclri	a0,a0,0		// destroy bit 0

	// check exponent underflow
da_3:	addi	a2,a2,-1	// exponent - 1
	bltz	a2,da_6		// exponent underflow or zero - limit to 0

	// check exponent overflow
	addi	a4,a2,2		// exponent + 1
	srli	a4,a4,11	// check if exponent is >= 0x800
	bnez	a4,da_7		// exponent overflow

	// pack exponent and sign
	slli	a2,a2,20	// shift exponent - 1 (do destroy hidden bit '1') to position 52
	add	a1,a1,a2	// add exponent and mantissa high, destroy hidden bit '1'
	slli	a3,a3,31	// shift sign bit to position 31
	add	a1,a1,a3	// add sign bit to mantissa
	ret

	// exponent overflow - return signed infinity
da_7:	slli	a1,a3,31	// get sign -> A1
	li	a3,0x7ff00000	// overflow value -> A1
	or	a1,a1,a3	// add sign bit
	li	a0,0		// mantissa low = 0
	ret

	// exponent underflow - return signed zero
da_6:	slli	a1,a3,31	// get sign -> A1
	li	a0,0		// mantissa low = 0
	ret

	// check if mantissa is zero (and cannot normalise it)
da_5:	bnez	a1,da_2		// mantissa is not zero, can continue with normalisation
	bnez	a4,da_2		// mantissa is not zero, can continue with normalisation

	// mantissa is zero, result will be 0
	ret

	// shift mantissa down by 1 bit, to normalise it
da_0:	addi	a2,a2,1		// adjust exponent
	slli	a6,a0,31	// save rounding bit -> A6 bit 31
	srli	a0,a0,1		// shift mantissa low >> 1 bit
	slli	a5,a1,31	// get mantissa high lowest bit -> A5
	or	a0,a0,a5	// add carry from mantissa high to mantissa low
	srli	a1,a1,1		// shift mantissa high >> 1 bit
	beqz	a6,da_3		// rounding bit not set
	j	da_4		// round mantissa up

da_xrgty: 
	// xe is rather greater than ye (xe>ye and shift>=32 places)
	li	t1,60		// number of shift > 60 ?
	bge	a6,t1,da_xmgty	// xe is much greater than ye, result is just X

	// here A6 = 32..59, A7 = -32..-59
	addi	a6,a6,-32	// number of shifts - 32 (A6 <- 0..27)
	beqz	a6,da_xrgty32	// number of shits = 32
	addi	a7,a7,64	// 64 - number of shifts (A7 <- 32..5)
	mv	a4,a2		// A4 <- mantissa ym low
	sll	a4,a4,a7	// these would be shifted off the bottom of the sticky bits
	beqz	a4,1f		// sticky bits = 0
	li	a4,1

1:	srl	a2,a2,a6	// shift mantissa ym low right
	or	a4,a4,a2	// add carry from ym low to sticky bits
	mv	a2,a3		// mantissa ym high -> A2
	sll	a3,a3,a7	// shift mantissa ym high left
	or	a4,a4,a3	// sum carry and mantissa high
	srai	a3,a2,31	// propagate sign bit
	j	da_xgty0

	// A6 = 0, A7 = 32
da_xrgty32:
	mv	a4,a2		// add carry from ym low to sticky bits
	mv	a2,a3		// mantissa ym high -> A2
	srai	a3,a3,31	// propagate sign bit
	j	da_add

da_yeqx:
	// here xe=ye, no shift
	li	a4,0		// no sticky bits
	j	da_add

da_ygtx:
	// here ye>xe - need to shift x down A7 places
	mv	t0,a5		// save exponent -> T0
	li	t1,32
	bge	a7,t1,da_yrgtx	// ye rather greater than xe
	addi	a6,a6,32
	mv	a4,a0
	sll	a4,a4,a6	// rounding bit + sticky bits

da_ygtx0:
	// shift mantissa xm right, to be same exponent as y
	mv	a5,a1		// A5 <- mantissa xm high
	sll	a5,a5,a6	// A5 <- shift mantissa xm high left
	srl	a0,a0,a7	// shift mantissa xm low right
	sra	a1,a1,a7	// shift mantissa xm high right
	or	a0,a0,a5	// add carry from mantissa xm high to mantissa xm low
	j	da_add		// sum mantissas

da_yrgtx:
	// ye rather greater than xe
	li	t1,60		// number of shifts > 60 ?
	bge	a7,t1,da_ymgtx	// ye is much greater than xe, result is just Y

	// here A7 = 32..59, A6 = -32..-59
	addi	a7,a7,-32	// number of shifts - 32 (A7 <- 0..27)
	beqz	a7,da_yrgtx32	// number of shits = 32
	addi	a6,a6,64	// 64 - number of shifts (A6 <- 32..5)
	mv	a4,a0		// A4 <- mantissa xm low
	sll	a4,a4,a6	// these would be shifted off the bottom of the sticky bits
	beqz	a4,1f		// sticky bits = 0
	li	a4,1

1:	srl	a0,a0,a7	// shift mantissa xm low right
	or	a4,a4,a0	// add carry from xm low to sticky bits
	mv	a0,a1		// mantissa xm high -> A0
	sll	a1,a1,a6	// shift mantissa xm high left
	or	a4,a4,a1	// sum carry and mantissa high
	srai	a1,a0,31	// propagate sign bit
	j	da_ygtx0

	// A7 = 0, A6 = 32
da_yrgtx32:
	mv	a4,a0		// add carry from xm low to sticky bits
	mv	a0,a1		// mantissa xm high -> A0
	srai	a1,a1,31	// propagate sign bit
	j	da_add

da_ymgtx:
	// ye is much greater than xe, result is just Y
	mv	a0,a2		// use Y as result
	mv	a1,a3

da_xmgty:
	// xe is much greater than ye, result is just X
	li	a4,0		// clear sticky bits
	j	da_pack		// pack result back

// ----------------------------------------------------------------------------
//                             Square, x^2
// ----------------------------------------------------------------------------
// double dsqr(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result X^2
// TRASHED: A2..A7, T0..T6

double_section __muldf3
.p2align 2,,

.global dsqr
dsqr:

	// duplicate operand
	mv	a2,a0
	mv	a3,a1

// ---- continue to dmul

// ----------------------------------------------------------------------------
//                        Multiplication, x * y
// ----------------------------------------------------------------------------
// double dmul(double x, double y);
// INPUT: A1:A0 = 1st argument X
//	  A3:A2 = 2nd argument Y
// OUTPUT: A1:A0 = result X * Y
// TRASHED: A2..A7, T0..T2

.global __wrap___muldf3
.global dmul
__wrap___muldf3:
dmul:

// Here is:
//  A0 = xL
//  A1 = xH
//  A2 = yL
//  A3 = yH

	// unpack numbers
// macro - unpack double to unsigned mantissa
// input: RB:RA = double number
// output: RB:RA = unsigned mantissa with hidden implied '1' set (53 significant bits)
//         RE = exponent ((-128 if zero, 1919 if infinity) << 12)
//         RS = sign (0 positive or 1 negative)
// trashes: RT
//.macro mdunpack ra,rb,re,rs,rt

	mdunpack a0,a1,a4,a5,t0	// unpack number x in A1:A0 -> A1:A0 mantissa xm, A4 exponent xe, A5 sign xs

// Here is:
//  A0 = xmL
//  A1 = xmH
//  A2 = yL
//  A3 = yH
//  A4 = xe exponent X
//  A5 = xs sign X

	mdunpack a2,a3,a6,a7,t0	// unpack number y in A3:A2 -> A3:A2 mantissa ym, A6 exponent ye, A7 sign ys

// Here is:
//  A0 = xmL
//  A1 = xmH
//  A2 = ymL
//  A3 = ymH
//  A4 = xe exponent X
//  A5 = xs sign X
//  A6 = ye exponent Y
//  A7 = ys sign Y

	// prepare result sign -> T2
	xor	t2,a7,a5	// result sign -> T2

	// prepare result exponent -> T1
	add	t1,a6,a4	// result exponent -> T1

// Here is:
//  A0 = xmL
//  A1 = xmH
//  A2 = ymL
//  A3 = ymH
//  T1 = result exponent
//  T2 = result sign

	// xmL (A0) * ymL (A2) -> M0 (_:_:A5:A4)
	mulhu	a5,a0,a2
	mul	a4,a0,a2

// Here is:
//  A0 = xmL
//  A1 = xmH
//  A2 = ymL
//  A3 = ymH
//  A4 = M0L
//  A5 = M0H
//  T1 = result exponent
//  T2 = result sign

	// xmH (A1) * ymH (A3) -> M3 (A7:A6:_:_)
	mulhu	a7,a1,a3
	mul	a6,a1,a3

// Here is:
//  A0 = xmL
//  A1 = xmH
//  A2 = ymL
//  A3 = ymH
//  A4 = M0L
//  A5 = M0H
//  A6 = M3L
//  A7 = M3H
//  T1 = result exponent
//  T2 = result sign

	// xmL (A0) * ymH (A3) -> M1 (_:T0:A0:_)
	mulhu	t0,a0,a3
	mul	a0,a0,a3

// Here is:
//  A0 = M1L
//  A1 = xmH
//  A2 = ymL
//  A4 = M0L
//  A5 = M0H
//  A6 = M3L
//  A7 = M3H
//  T0 = M1H
//  T1 = result exponent
//  T2 = result sign

	// add middle part M1 (_:T0:A0:_) to high and low M3:M0 (A7:A6:A5:A4)
	add	a5,a5,a0	// M0H (A5) + M1L (A0) -> A5
	sltu	a3,a5,a0	// A3 <- carryL
	add	a6,a6,t0	// M3L (A6) + M1H (T0) -> A6
	sltu	a0,a6,t0	// A0 <- carryH
	add	a7,a7,a0	// add carryH
	add	a6,a6,a3	// add carryL
	sltu	a0,a6,a3	// A0 <- carry
	add	a7,a7,a0	// add carry

// Here is:
//  A1 = xmH
//  A2 = ymL
//  A4 = M0L
//  A5 = M0H+M1L
//  A6 = M3L+M1H
//  A7 = M3H
//  T1 = result exponent
//  T2 = result sign

	// xmH (A1) * ymL (A2) -> M2 (_:A3:A0:_)
	mulhu	a3,a1,a2
	mul	a0,a1,a2

// Here is:
//  A0 = M2L
//  A3 = M2H
//  A4 = M0L
//  A5 = M0H+M1L
//  A6 = M3L+M1H
//  A7 = M3H
//  T1 = result exponent
//  T2 = result sign

	// add middle part M2 (_:A3:A0:_) to high and low M3:M0 (A7:A6:A5:A4)
	add	a5,a5,a0	// M0H+M1L (A5) + M2L (A0) -> A5
	sltu	t0,a5,a0	// T0 <- carryL
	add	a6,a6,a3	// M3L+M1H (A6) + M2H (A3) -> A6
	sltu	a0,a6,a3	// A0 <- carryH
	add	a7,a7,a0	// add carryH
	add	a6,a6,t0	// add carryL
	sltu	t0,a6,t0	// T0 <- carry
	add	a7,a7,t0	// add carry

// Here is:
//  T1 = result exponent
//  T2 = result sign
//  A7:A6:A5:A4 = product of mantissas xm*ym (22 unused bits + 106 significant bits)

	// shift result mantissa to normal position, shift 11 bits left -> A1:A0(:A5), extra low bits A4
	slli	a1,a7,11	// A7 shift 11 bits left -> A1
	srli	a2,a6,21	// A6 shift 21 (=32-11) bits right -> A2
	or	a1,a1,a2	// assemble mantissa high -> A1
	slli	a0,a6,11	// A6 shift 11 bits left -> A0
	srli	a2,a5,21	// A5 shift 21 (=32-11) bits right -> A2
	or	a0,a0,a2	// assemble mantissa low -> A0
	slli	a5,a5,11	// shift sticky bits 11 bits left -> A5 (extra bits stay in A4)

	// check highest bit, if result mantissa is normalised
	srli	a2,a1,20	// check bit 52 -> A2
	bnez	a2,1f		// mantissa is normalised OK

	// mantissa is not normalized, shift mantissa 1 bit left (and correct exponent - 1)
	slli	a1,a1,1		// mantissa high << 1
	srli	t0,a0,31	// T0 <- carry bit
	or	a1,a1,t0	// add carry
	slli	a0,a0,1		// mantissa low << 1
	srli	t0,a5,31	// T0 <- carry bit
	or	a0,a0,t0	// add carry
	slli	a5,a5,1		// sticky bits << 1
	addi	t1,t1,-1	// correct exponent

	// exponent bias correction
1:	li	a6,0x3ff	// correction 1023
	sub	t1,t1,a6	// correct for exponent bias, exp - 1023

	// check exponent overflow or underflow
	slli	a6,a6,1		// A6 = 0x7fe
	bgeu	t1,a6,dm_0	// exponent over- or underflow

	// check sticky bits, if rounding is needed
	srli	t0,a5,31	// rounding bit
	slli	a5,a5,1		// rounding bit to carry
	beqz	t0,1f		// result is correctly rounded

	// add mantissa + 1 to round up
	add	a0,a0,1		// mantissa low + 1
	bnez	a0,4f
	add	a1,a1,1		// mantissa high + carry

	// check remaining sticky bits - if they are zero, round to even
4:	or	a5,a5,a4	// check sticky bits
	bnez	a5,1f		// some sticky bits are set
	bclri	a0,a0,0		// discard bit 0 (to round to even)

	// add exponent to result
1:	slli	t1,t1,20	// shift exponent to position 20
	add	a1,a1,t1	// add exponent to mantissa

	// add sign bit to result
dm_2:	slli	t2,t2,31	// shift sign bit to position 31
	add	a1,a1,t2	// add sign bit to result
	ret

	// here for exponent over- or underflow
dm_0:	bge	t1,a6,dm_1	// overflow

	// exponent underflow
	addi	t1,t1,1		// would-be zero exponent?
	bnez	t1,1f		// underflow, not zero

	// result is near zero
	addi	a0,a0,1		// check is mantissa low is 0xfff...
	bnez	a0,1f		// not all-ones, go underflow
	addi	a1,a1,1		// check is mantissa high is 0xfff...
	srli	a7,a1,21	// check is mantissa is all-ones
	beqz	a7,1f		// not all-ones
	srli	a1,a1,1		// mantissa is 0xfff with exponent 0
	j	dm_2		// add sign bit

	// exponent underflow, result is +0.0 or -0.0
1:	slli	a1,t2,31	// shift sign bit to position 31 -> A1
	li	a0,0		// mantissa low = 0
	ret

	// here for exponent overflow
dm_1:	addi	a6,a6,1		// A6 = 0x7ff, exponent overflow
	slli	a1,a6,20	// shift exponent to position 20 -> A1
	li	a0,0		// mantissa low = 0
	j	dm_2		// add sign bit

// ----------------------------------------------------------------------------
//                        Reciprocal 1 / x
// ----------------------------------------------------------------------------
// double drec(double x);
// INPUT: A1:A0 = argument X
// OUTPUT: A1:A0 = result 1/X
// TRASHED: A2..A7, T0..T6

double_section __divdf3
.p2align 2,,

.global drec
drec:
	// first operand is 1.0
	mv	a3,a1
	mv	a2,a0
	li	a1,0x3ff00000
	li	a0,0

// ---- continue to ddiv

// ----------------------------------------------------------------------------
//                         Division, y / x
// ----------------------------------------------------------------------------
// double ddivd(double y, double x);
// INPUT: A1:A0 = 1st argument Y
//	  A3:A2 = 2nd argument X
// OUTPUT: A1:A0 = result X / Y
// TRASHED: A2..A7, T0..T3

.global __wrap___divdf3
.global ddiv
__wrap___divdf3:
ddiv:

// Approach to division y/x is as follows.
//
// First generate u1, an approximation to 1/x to about 29 bits. Multiply this by the top
// 32 bits of y to generate m0, a first approximation to the result (good to 28 bits or so).
// Calculate the exact remainder r0=y-m0*x, which will be about 0. Calculate a correction
// d0=r0*u1, and then write a1=m0+d0. If near a rounding boundary, compute the exact
// remainder r1=y-a1*x (which can be done using r0 as a basis) to determine whether to
// round up or down.
//
// The calculation of 1/x is as given in dreciptest.c. That code verifies exhaustively
// that | u1*x-1 | < 10*2^-32.
//
// More precisely:
//
// x0=(q16)x;
// x1=(q30)x;
// y0=(q31)y;
// u0=(q15~)"(0xffffffffU/(unsigned int)roundq(x/x_ulp))/powq(2,16)"(x0);
//          q15 approximation to 1/x, "~" denotes rounding rather than truncation
// v=(q30)(u0*x1-1);
// u1=(q30)u0-(q30~)(u0*v);
//
// m0=(q30)(u1*y0);
// r0=(q82)y-m0*x;
// r0x=(q57)r0;
// d0=r0x*u1;
// a1=d0+m0;
//
// Error analysis
//
// Use Greek letters to represent the errors introduced by rounding and truncation.
//
//               r0 = y - a0x
//                  = y - [ u1 ( y - alpha ) - beta ] x    where 0 <= alpha < 2^-31, 0 <= beta < 2^-30
//                  = y ( 1 - u1*x ) + ( u1*alpha + beta) x
//
//     Hence
//
//       | r0 / x | < 2 * 10*2^-32 + 2^-31 + 2^-30
//                  = 26*2^-32
//
//               r1 = y - a1x
//                  = y - a0x - d0x
//                  = r0 - d0x
//                  = r0 - u1 ( r0 - lambda ) x    where 0 <= lambda < 2^-57
//                  = r0 ( 1 - u1x ) + u2*lambda*x
//
//     Hence
//
//       | r1 / x | < 26*2^-32 * 10*2^-32 + 2^-57
//                  = (260+128)*2^-64
//                  < 2^-55
//
// Empirically it seems to be nearly twice as good as this.
//
// To determine correctly whether the exact remainder calculation can be skipped we need a result
// accurate to < 0.25ulp. In the case where x>y the quotient will be shifted up one place for normalisation
// and so 1ulp is 2^-53 and so the calculation above suffices.

// Here is:
//  A0 = yL
//  A1 = yH
//  A2 = xL
//  A3 = xH

	// unpack divisor x A3:A2 -> A3:A2 mantissa xm, A4 exponent xe, A7 sign xs, A6 temporary
// macro - unpack double to unsigned mantissa
// input: RB:RA = double number
// output: RB:RA = unsigned mantissa with hidden implied '1' set (53 significant bits)
//         RE = exponent (-128 if zero, 1919 if infinity)
//         RS = sign (0 positive or 1 negative)
// trashes: RT
//.macro mdunpack ra,rb,re,rs,rt
	mdunpack a2,a3,a4,a7,t0	// unpack divisor

// Here is:
//  A0 = yL
//  A1 = yH
//  A2 = xmL
//  A3 = xmH
//  A4 = xe exponent X
//  A7 = xs sign X

	// prepare result sign bit in T1
	srli	a6,a1,31	// sign bit of dividend y -> A6
	add	t1,a6,a7	// add sign of xs -> T1, store result sign in t1 bit0 (t1 bit1 is trashed)

	// prepare exponent of dividend ye	
	slli	a1,a1,1		// destroys sign bit from dividend y (shift 1 bit left)
	srli	a7,a1,21	// exponent of dividen ye -> A7
	beqz	a7,1f		// zero exponent of y

	// check infinity of dividend ye
	addi	a6,a7,1		// exponent of y + 1 to check infinity
	srli	a6,a6,11	// check infinity
	beqz	a6,2f		// y is not infinity (exponent != 0x7ff)

	// dividend y is zero or infinity
1:	li	a0,0		// A0 <- 0 ymL
	li	a1,0		// A1 <- 0 ymH
	addi	a7,a7,-64	// less drastic fiddling of exponents to get 0/0, Inf/Inf correct
	slli	a7,a7,12

	// isolate mantissa of dividend y
2:	sub	a6,a7,a4	// result exponent y - x -> A6
	slli	a6,a6,2		// result exponent * 4
	add	t1,t1,a6	// store signed exponent into t1[31..8]
	addi	a7,a7,-1	// exponent y - 1, to add hidden implied bit '1' to mantissa
	slli	a7,a7,21	// shift exponent to position 21
	sub	a1,a1,a7	// isolate mantissa of y and add hidden implied bit '1'
	srli	a1,a1,1		// shift mantissa back to its position

	// calculate 1/x (1/divisor)
	li	a5,-1		// A5 <- -1
	srli	a6,a3,4		// x0=(q16)x (53 - 4 - 32 = 17 bits)
	divu	a6,a5,a6	// A6 <- 1/x

	// round 1/x division up (first approximation u0)
	addi	a6,a6,1		// result + 1
	srli	a6,a6,1		// (result + 1)/2

// Here is:
//  A0 = ymL
//  A1 = ymH
//  A2 = xmL
//  A3 = xmH
//  A6 = u0, first approximation to 1/x Q15
//  T1 = result sign and exponent

	// get second approximation
	slli	a4,a3,10	// A4 <- mantissa xmH << 10
	srli	a5,a2,22	// A5 <- mantissa xmL >> 22
	or	a5,a5,a4	// x1=(q30)x, mantissa xm << 10

	// v=(q30)(u0*x1-1);
	mul	a5,a5,a6	// x1*u0 Q45
	srai	a5,a5,15	// v=u0*x1-1 Q30

	// u1=(q30)u0-(q30~)(u0*v);
	mul	a5,a5,a6	// v*u0 Q45
	srai	a5,a5,14
	addi	a5,a5,1
	srai	a5,a5,1		// round u0*v to Q30
	slli	a6,a6,15
	sub	a6,a6,a5	// u1 Q30 (=u0-u0*v)

// Here is:
//  A0 = ymL
//  A1 = ymH
//  A2 = xmL
//  A3 = xmH
//  A6 = u1, second approximation to 1/x Q30
//  T1 = result sign and exponent

	// get m0=(q30)(u1*y0);
	mv	t2,a2		// T2 <- save xmL
	mv	t3,a3		// T3 <- save xmH
	slli	a4,a1,11
	srli	a5,a0,21
	or	a4,a4,a5	// y0=(q31)y

	// y0*u1 Q61
	mulhu	a5,a4,a6	// A5 <- mulH
	mul	a4,a4,a6	// A4 <- mulL

	srli	t0,a4,31	// T0 <- carry
	slli	a4,a4,1
	slli	a5,a5,1		// m0=(q30)(y0*u1)
	or	a5,a5,t0	// add carry

// Here is:
//  A0 = ymL
//  A1 = ymH
//  A2 = xmL
//  A5 = m0, first approximation to y/x Q30
//  A6 = u1, second approximation to 1/x Q30
//  T1 = result sign and exponent
//  T2 = save xmL
//  T3 = save xmH

	// get a0=(q82)y-m0*x, r0x=(q57)a0;
	mulhu	a3,a2,a5	// A3 <- xmL*m0 high
	mul	a2,a2,a5	// A2 <- xmL*m0 low
	mul	a4,t3,a5	// A4 <- xmH*m0
	add	a3,a3,a4	// a2:a3 now x*m0 Q82
	srli	a2,a2,25
	slli	a1,a3,7
	or	a2,a2,a1	// a2 now x*m0 Q57; a7:a2 is x*m0 Q89
	slli	a4,a0,5		// y Q57
	sub	a0,a4,a2	// r0x=y-x*m0 Q57 (signed)

// Here is:
//  A0 = r0x Q57
//  A4 = yL  Q57
//  A5 = m0, first approximation to y/x Q30
//  A6 = u1, second approximation to 1/x Q30
//  T1 = result sign and exponent
//  T2 = save xmL
//  T3 = save xmH

	mul	a7,a0,a6	// A7 <- r0x * u1 Q87 low
	mulhsu	a6,a0,a6	// A6 <- r0x * u1 Q87 high
	srai	a3,a6,25
	add	a5,a5,a3
	slli	a3,a6,7		// a3:a5 m1 Q62 (but bottom 7 bits are zero so 55 bits of precision after binary point)

// here we could recover another 7 bits of precision (but not accuracy) from the top of a7
// but these bits are thrown away in the rounding and conversion to Q52 below

// here
//  A3:A5 = m1 Q62 candidate quotient [0.5,2) or so
//  A4 = yL  Q57
//  T1 = result sign and exponent
//  T2 = save xmL
//  T3 = save xmH

	li	t0,128
	add	a3,a3,t0	// for initial rounding to Q53
	sltu	t0,a3,t0
	add	a5,a5,t0
	srli	a1,a5,30
	bnez	a1,dd_0

// here candidate quotient m1 is in range [0.5,1)
// so 30 significant bits in a5

	slli	a4,a4,1		// y now Q58
	srli	a1,a5,9		// to Q52
	slli	a0,a5,23
	bexti	t0,a3,8		// T0 <- extract carry bit 8
	srli	a3,a3,9		// 0.5ulp-significance bit in carry: if this is 1 we may need to correct result
	or	a0,a0,a3
	bnez	t0,dd_1
	j	dd_2

// here candidate quotient m1 is in range [1,2)
// so 31 significant bits in a5

dd_0:	li	a2,4
	add	t1,t1,a2	// fix exponent; a3:a5 now effectively Q61
	li	t0,128
	add	a3,a3,t0	// complete rounding to Q53
	sltu	t0,a3,t0
	add	a5,a5,t0
	srli	a1,a5,10
	slli	a0,a5,22
	bexti	t0,a3,9		// T0 <- extract carry bit 9
	srli	a3,a3,10	// 0.5ulp-significance bit in carry: if this is 1 we may need to correct result
	or	a0,a0,a3
	beqz	t0,dd_2

// here
//  A0:A1 = rounded result Q53 [0.5,1) or Q52 [1,2), but may not be correctly rounded-to-nearest
//  A4 = yL Q58 or Q57
//  T1 = result sign and exponent
//  T2 = save xmL
//  T3 = save xmH

dd_1:	srli	t0,a0,31	// T0 <- carry
	slli	a0,a0,1
	ori	a0,a0,1
	slli	a1,a1,1
	or	a1,a1,t0	// z Q53 with 1 in LSB

	slli	a4,a4,16	// Q105-32=Q73
	mv	a2,t2		// xL Q52
	mv	a3,t3		// xH Q20

	mv	a5,a1		// zH Q21
	mul	a5,a5,a2	// zH*xL Q73
	sub	a4,a4,a5
	mul	a3,a3,a0	// zL*xH Q73
	sub	a4,a4,a3

	mulhu	a3,a2,a0
	mul	a2,a2,a0	// xL*zL

	neg	a2,a2		// borrow from low half?
	sub	a4,a4,a3	// y-xz Q73 (remainder bits 52..73)
	beqz	a2,5f
	addi	a4,a4,-1
5:
	bltz	a4,1f
	addi	a0,a0,1
	bnez	a0,1f
	addi	a1,a1,1

1:	srli	a0,a0,1		// shift back down to Q52
	slli	a2,a1,31
	or	a0,a0,a2
	srli	a1,a1,1

dd_2:	mv	a2,t1
	slli	a7,a2,31	// result sign
	srai	a2,a2,2		// result exponent
	li	a3,0x3fd
	add	a2,a2,a3
	li	a3,0x7fe

	bgeu	a2,a3,dd_3	// over- or underflow?
	slli	a2,a2,20
	add	a1,a1,a2	// pack exponent

dd_5:	add	a1,a1,a7	// pack sign
	ret

dd_3:	li	a0,0
	bgtz	a2,dd_4		// overflow?
	mv	a1,a7
	ret

dd_4: 	addi	a3,a3,1		// 0x7ff
	slli	a1,a3,20
	j	dd_5

#endif // !USE_DOUBLELIBC
#endif // USE_DOUBLE
