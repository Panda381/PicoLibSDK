
// ****************************************************************************
//
//                              Float RP2350 - RISC-V
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040

// Copyright (c) 2020 Mark Owen https://www.quinapalus.com - GPLv2 licence

/*
 * Copyright (c) 2024 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// >>> Some functions are adaptations of the functions from Pico1 M0+ (c) Raspberry Pi <<<

// >>> Warning - the original __mulsf3 function has a bug of unhandled
//     overflow during rounding - it is fixed here. <<<

// Functions were tested with the FPUTEST application from the PicoLibSDK  
// https://github.com/Panda381/PicoLibSDK/tree/main/PicoPad/TEST/FPUTEST

// .p2align 2,,
// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - adds incorrect alignment
// after the end of the section. Use commas ,, to work a little better.

#include "../asm_include.inc"

// u32 __addsf3(u32 x, u32 y);
// u32 __subsf3(u32 x, u32 y);
// u32 __mulsf3(u32 x, u32 y);  ... with bug fixed on rounding
// u32 __divsf3(u32 x, u32 y);	... new function (adaptation from Pico1 M0+)

#if USE_FLOAT		// use float support 1=in RAM, 2=in Flash

#if USE_FLOAT == 1
#define float_section section_noflash
#else
#define float_section section_text
#endif

#if !USE_FLOATLIBC

// ----------------------------------------------------------------------------
//                         Subtraction, x - y
// ----------------------------------------------------------------------------
// float fsub(float x, float y);
// INPUT: A0 = 1st argument X
//	  A1 = 2nd argument Y
// OUTPUT: A0 = result X - Y
// TRASHED: A1, A2, A3, A4, A5, T0, T1

float_section __addsf3
.p2align 2,,

.global __wrap___subsf3
.global __subsf3
.global fsub
__wrap___subsf3:
__subsf3:
fsub:

	// invert sign bit of 2nd argument Y
	binvi	a1,a1,31

// ---- continue to fadd

// ----------------------------------------------------------------------------
//                         Addition, x + y
// ----------------------------------------------------------------------------
// float fadd(float x, float y);
// INPUT: A0 = 1st argument X
//	  A1 = 2nd argument Y
// OUTPUT: A0 = result X + Y
// TRASHED: A1, A2, A3, A4, A5, T0, T1

.global __wrap___addsf3
.global __addsf3
.global fadd
__wrap___addsf3:
__addsf3:
fadd:
	// Unpack exponent
	h3.bextmi	a2,a0,23,8			// extract X exponent A2 <- (A0 >> 23) & 0xff
	h3.bextmi	a3,a1,23,8			// extract Y exponent A3 <- (A1 >> 23) & 0xff

	// Flush-to-zero => 0 + y = y applies, including nan, with the sole
	// exception of y being subnormal (which also needs to be flushed)
	beqz		a2,__addsf_return_y_flushed	// X is zero - result will be Y

	// Don't have to handle this case for x + 0 = 0 because we already know x is nonzero
	beqz		a3,__addsf_return_x		// Y is zero - result will be X

	// Unpack significand, plus 3 extra zeroes for working space
	slli		a4,a0,9				// get X mantissa A4 <- X << 9
	slli		a5,a1,9				// get Y mantissa A5 <- Y << 9

	// check nan/inf on input
	li		t0,255				// T0 <- 255
	beq		a2,t0,__addsf_x_nan_inf		// X is infinity or NAN
	beq		a3,t0,__addsf_y_nan_inf		// Y is infinity or NAN

	// (finish unpacking significand)
	srli		a4,a4,6				// A4 <- mantissa X with 3 lower bits reserve
	srli		a5,a5,6				// A5 <- mantissa Y with 3 lower bits reserve

	// If we're still on the straight path then we are adding two normal
	// values. Add implicit one (1.xx...xx000)
	bseti		a4,a4,23 + 3			// restore hidden implied bit '1' to X mantissa
	bseti		a5,a5,23 + 3			// restore hidden implied bit '1' to Y mantissa

	// Negate mantissa X if sign bit X is set
	bgez		a0,1f				// skip if X is not minus
	neg		a4,a4				// negate mantissa of X

	// (tuck this 16-bit here to avoid alignment penalty)
1:	li		t1,25				// T1 <- 25

	// Negate mantissa Y if sign bit Y is set
	bgez		a1,1f				// skip if X is not minus
	neg		a5,a5				// negate mantissa of Y

	// compare exponents
1:	bltu		a2,a3,__addsf_ye_gt_xe		// skip if exp X < exp Y

	// The main body is repeated twice with different register assignments.
	// lhs is the more-significant addend:

// === 1st operand is >= 2nd operand
// packed_lhs ... packed 1st operand
// packed_rhs ... packed 2nd operand
// sig_lhs ... signed mantissa of 1st operand
// sig_rhs ... signed mantissa of 2nd operand
// exp_lhs ... exponent of 1st operand (exp_lhs >= exp_rhs)
// exp_rhs ... exponent of 2ns operand
// rhs_is_x ... operands are inverted, 0=operands are X+Y, 1=operands are Y+X

.macro addsf_core packed_lhs, packed_rhs, sig_lhs, sig_rhs, exp_lhs, exp_rhs, rhs_is_x

	// get difference of exponents (exp_lhs >= exp_rhs)
	sub		\packed_rhs,\exp_lhs,\exp_rhs		// packed_rhs <- exp1 - exp2

	// If there is a large exponent difference then there is no effect on lhs
.if \rhs_is_x	// order Y + X
	bgeu		\packed_rhs,t1,__addsf_return_y		// Y is much greater than X, result will be Y
.else		// order X + Y
	bgeu		\packed_rhs,t1,__addsf_return_x		// X is much greater than Y, result will be X
.endif

	// Shift rhs down to correct relative significance
	sra		\packed_lhs,\sig_rhs,\packed_rhs	// shift mantissa of 2nd operand right, to the same exponent as 1st operand

	// Set sticky bit if ones were shifted out
	sll		\packed_rhs,\packed_lhs,\packed_rhs	// shift mantissa of 2nd operand back
	sltu		\packed_rhs,\packed_rhs,\sig_rhs	// set to 1 if some '1' bits were lost
	or		\packed_lhs,\packed_lhs,\packed_rhs	// add sticky bit '1' to mantissa of 2nd operand

	// Add significands
	add		\sig_lhs,\sig_lhs,\packed_lhs		// sum mantissas of both operands

	// Detect exact cancellation (may be beyond max normalisation shift; also
	// IEEE 754 requires +0 for exact cancellation, no matter input signs)
	beqz		\sig_lhs,__addsf_return_0		// result is 0

	// Convert two's complement back to sign + magnitude
	srai		\exp_rhs,\sig_lhs,31			// check sign of mantissa: get 0 if >= 0, or 0xffffffff if < 0
	xor		\sig_lhs,\sig_lhs,\exp_rhs		// invert result mantissa if negative
	sub		\sig_lhs,\sig_lhs,\exp_rhs		// sub -1 = add +1 = that means negate result

	// Renormalise significand: bit 31 is now implicit one
	clz		\packed_lhs,\sig_lhs			// get number of leading zeroes (there is at least hidden implied bit '1')
	sll		\sig_lhs,\sig_lhs,\packed_lhs		// shift result left, hidden implied bit '1' is on position 31

	// Adjust exponent
	addi		\packed_lhs,\packed_lhs,-5		// number of leading zeroes - 5
	sub		\exp_lhs,\exp_lhs,\packed_lhs		// result exponent = exponent of 1st operand - (number of zeroes - 8) - 3

	// Round to nearest, even on tie (bias upward if above odd number)
	bexti		\packed_lhs,\sig_lhs,8			// get lowest 8 bits
	addi		\sig_lhs,\sig_lhs,127			// add 0.5 to round up
	add		\sig_lhs,\sig_lhs,\packed_lhs		// add lowest 8 bits

	// Exponent may increase by one due to rounding up from all-ones; this is
	// detected by clearing of implicit one (there is a carry-out too)
	bgez		\sig_lhs,3f				// overflow to 100000...

	// Detect underflow/overflow
4:	bgeu		\exp_lhs,t0,1f				// compare exponent with 255 - skip if overflow

	// Pack and return
	packh		\exp_lhs,\exp_lhs,\exp_rhs
	slli		\exp_lhs,\exp_lhs,23
	slli		\sig_lhs,\sig_lhs,1
	srli		\sig_lhs,\sig_lhs,9
	add		a0,\sig_lhs,\exp_lhs
	ret

	// overflow?
1:	bgez		\exp_lhs,2f

	// Signed zero on underflow
	slli		a0,\exp_rhs,31
	ret

	// Signed infinity on overflow
2:	packh		a0,t0,\exp_rhs
	slli		a0,a0,23
	ret

	// Exponent increase due to rounding (uncommon)
3:	srli		\sig_lhs,\sig_lhs,1
	addi		\exp_lhs,\exp_lhs,1
	j		4b
.endm

	// exp X >= exp Y, 1st operand is X, 2nd operand is Y
__addsf_xe_gte_ye:
	addsf_core	a0,a1,a4,a5,a2,a3,0

	// exp X < exp Y, 1st operand is Y, 2nd operand is X
.p2align 2,,
__addsf_ye_gt_xe:
	addsf_core	a1,a0,a5,a4,a3,a2,1

	// X is infinity or NAN
__addsf_x_nan_inf:
	// When at least one operand is nan, we must propagate at least one of
	// those nan payloads (sign of nan result is unspecified, which we take
	// advantage of by implementing x - y as x + -y). Check x nan vs inf:
	bnez		a4,__addsf_return_x

__addsf_x_inf:
	// If x is +-inf, need to distinguish the following cases:
	bne		a3,t0,__addsf_return_x			// y is neither inf nor nan   -> return x (propagate inf)
	bnez		a5,__addsf_return_y			// y is nan:                  -> return y (propagate nan)
	xor		a5,a0,a1
	srli		a5,a5,31
	beqz		a5,__addsf_return_x			// y is inf of same sign      -> return either x or y (x is faster)
	li		a0,-1					// y is inf of different sign -> return nan
	ret

	// Y is infinity or NAN
__addsf_y_nan_inf:
	// Mirror of __addsf_x_nan_inf
	bnez		a5,__addsf_return_y

__addsf_y_inf:
	bne		a2,t0,__addsf_return_y
	bnez		a4,__addsf_return_x
	xor		a4,a0,a1
	srli		a4,a4,31
	beqz		a4,__addsf_return_x
	li		a0,-1
	ret

	// X is 0, result will be Y (sign stay unchanged)
__addsf_return_y_flushed:
	bnez		a3,1f					// exponent Y is not zero, return number Y as is
	srli		a1,a1,23				// result is zero, clear mantissa of Y
	slli		a1,a1,23				// restore mantissa 0, exponent and sign stay unchanged

	// result will be Y
__addsf_return_y:
1:	mv		a0,a1					// A0 <- result will be Y

	// Y is 0, result will be X
__addsf_return_x:
	ret

	// result will be 0
__addsf_return_0:
	li		a0,0					// A0 <- 0, result will be 0
	ret

// ----------------------------------------------------------------------------
//                        Multiplication, x * y
// ----------------------------------------------------------------------------
// float fmul(float x, float y);
// INPUT: A0 = 1st argument X
//	  A1 = 2nd argument Y
// OUTPUT: A0 = result X + Y
// TRASHED: A1, A2, A3, A4, A5, A6, T0

float_section __mulsf3
.p2align 2,,

.global __wrap___mulsf3
.global __mulsf3
.global fmul
__wrap___mulsf3:
__mulsf3:
fmul:

	// Force y to be positive (by possibly negating x) *before* unpacking.
	// This allows many special cases to be handled without repacking.
	bgez		a1,1f			// skip if Y is positive
	binvi		a0,a0,31		// negate X - X will contain result sign

	// Unpack exponent:
1:	h3.bextmi	a2,a0,23,8		// A2 <- exponent of X
	h3.bextmi	a3,a1,23,8		// A3 <- exponent of Y

	// Check special cases
	li		t0,255			// T0 <- 255 (exponent INF)
	beqz		a2,__mulsf_x_0		// X is zero
	beqz		a3,__mulsf_y_0		// Y is zero
	beq		a2,t0,__mulsf_x_nan_inf	// X is NAN or INF
	beq		a3,t0,__mulsf_y_nan_inf	// Y is NAN or INF

	// Finish unpacking sign
	srai		a6,a0,31		// A6 <- result sign (0 or -1)

	// Unpack significand (with implicit one in MSB - bit 31)
	slli		a4,a0,8			// A4 <- mantissa X
	slli		a5,a1,8			// A5 <- mantissa Y
	bseti		a4,a4,31		// restore hidden implied bit '1' to X mantissa, bit 31
	bseti		a5,a5,31		// restore hidden implied bit '1' to Y mantissa, bit 31

	// Get full 64-bit multiply result in a4:a1 (one cycle each half)
	// Going from Q1.23 to Q2.46 (both left-justified)
	mul		a1,a4,a5		// A1 <- multiply mantissas LOW
	mulhu		a4,a4,a5		// A4 <- multiply mantissas HIGH

	// Normalise (shift left by either 0 or 1) -- bit 8 is the LSB of the
	// final significand (ignoring rounding)
	clz		a0,a4			// A0 <- get leading zeroes of result HIGH (0 or 1)
	sll		a4,a4,a0		// shift result HIGH to high position
	sub		a2,a2,a0		// correct exponent

	// After normalising we can calculate the final exponent, since rounding
	// cannot increase the exponent for multiplication (unlike addition)
	add		a2,a2,a3		// sum exponents

	// Subtract redundant bias term (127), add 1 for normalisation correction
	addi		a2,a2,-126		// subtract bias and normalize exponent
	blez		a2,__mulsf_underflow	// underflow result
	bge		a2,t0,__mulsf_overflow	// overflow result

	// Gather sticky bits from low fraction:
	snez		a1,a1			// A1 <- 1 if result LOW is not zero
	or		a4,a4,a1		// add 1 to result HIGH, if result LOW is not zero

	// Round to nearest, even on tie (aka bias upward if odd)
	bexti		a1,a4,8			// A1 <- extract bit 8 from result HIGH
	add		a4,a4,a1		// add bit 8 to result HIGH
	addi		a4,a4,127		// rounding up

	// Exponent may increase by one due to rounding up from all-ones; this is
	// detected by clearing of implicit one (there is a carry-out too)
	bgez		a4,4f			// overflow to 100000...

	// Pack it and ship it
3:	packh		a2,a2,a6		// pack exponent byte with result sign
	slli		a2,a2,23		// shift exponent and sign to final position
	slli		a4,a4,1			// discard hidden implied bit '1' from the mantissa
	srli		a4,a4,9			// shift mantissa to final position
	add		a0,a4,a2		// A0 <- pack mantissa with exponent and sign
	ret

	// Exponent increase due to rounding (uncommon)
4:	srli		a4,a4,1
	addi		a2,a2,1
	blt		a2,t0,3b		// no overflow result

	// overflow result
__mulsf_overflow:
	// Signed inf
	packh		a0,t0,a6		// A0 <- pack exponent 0xff with sign
	slli		a0,a0,23		// shift INF exponent with sign to final position
	ret

	// underflow result
__mulsf_underflow:
	// Signed zero
	slli		a0,a6,31		// A0 <- +0.0 or -0.0
	ret

	// X is zero
__mulsf_x_0:
	// 0 times nan    -> propagate nan
	// 0 times inf    -> generate nan
	// 0 times others -> 0 (need to flush significand too as we are FTZ)
	bne		a3,t0,__mulsf_return_flushed_x // Y not inf
	slli		a5,a1,9			// A5 <- Y mant<<9 to check mantissa
	beqz		a5,1f			// mantissa is zero, Y is inf

	// Propagate nan from y
__mulsf_return_y:
	mv		a0,a1			// A0 <- Y
	ret

	// Generate new nan
1:	li		a0,-1			// A0 <- -1, -nan
	ret

	// Y is zero
__mulsf_y_0:
	// Mirror image of x_0 except we still return x for signed 0, since the
	// signs were already resolved.
	bne		a2,t0,__mulsf_return_flushed_x	// X is not inf
	slli		a1,a0,9			// A1 <- X mant<<9 to check mantissa
	bnez		a1,1f			// mantissa is not zero, X is nan
	li		a0,-1			// mantissa is zero, -nan
1:	ret

__mulsf_return_flushed_x:
	// If we don't support subnormals we at least need to flush to a canonical
	// zero. This is just a sign bit in bit 31.
	srli		a0,a0,31		// destroy unused bits
	slli		a0,a0,31		// return sign to final position
__mulsf_return_x:
	ret

	// X is NAN or INF
__mulsf_x_nan_inf:
	// We know that y is not zero and is positive. So...
	//      x is nan    -> return x
	// else y is nan    -> return y
	// else y is inf    -> return x
	// else y is normal -> return x
	// (the order of the first two clauses is actually our free choice)
	slli		a4,a0,9			// A4 <- X mant<<9 to check mantissa
	bnez		a4,__mulsf_return_x	// mantissa X is not zero, X is nan, return X
	bne		a3,t0,__mulsf_return_x	// exponent X is not inf, return X
	slli		a5,a1,9			// A5 <- Y mant<<9 to check mantissa
	bnez		a5,__mulsf_return_y	// mantsissa Y is not zero, Y is nan, return Y
	ret					// return x

	// Y is NAN or INF
__mulsf_y_nan_inf:
	// We know that x is not zero, nan, nor inf. That just leaves normals.
	// y is nan -> return y
	// y is inf -> return inf * sgn(x) (since we already merged the signs)
	slli		a5,a1,9			// A5 <- Y mant<<9 to check mantissa
	bnez		a5,__mulsf_return_y	// mantissa Y is not zero, Y is nan, return Y

//	srai		a0,a0,31		// shift sign bit to position 0
	srli		a0,a0,31		// shift sign bit to position 0

	packh		a0,t0,a0		// pack sign bit with exponent 0xFF
	slli		a0,a0,23		// shift exponent and sign to position
	ret

// ----------------------------------------------------------------------------
//                            Division, x / y
// ----------------------------------------------------------------------------
// float fdiv(float x, float y);
// INPUT: A0 = 1st argument X
//	  A1 = 2nd argument Y
// OUTPUT: A0 = result X / Y
// TRASHED: A1, A2, A3, A4, A5, A6, T0

float_section __divsf3
.p2align 2,,

.global __wrap___divsf3
.global __divsf3
.global fdiv
__wrap___divsf3:
__divsf3:
fdiv:

// This version of the division algorithm uses external divider hardware to estimate the
// reciprocal of the divisor to about 14 bits; then a multiplication step to get a first
// quotient estimate; then the remainder based on this estimate is used to calculate a
// correction to the quotient. The result is good to about 27 bits and so we only need
// to calculate the exact remainder when close to a rounding boundary.

	// prepare temporary dividend 0xffffffff
	li	a4,1		// A4 <- 1
	li	a6,-1		// A6 <- 0xffffffff

	// prepare 17 top bits of divisor -> A3
	slli	a4,a4,23	// A4 <- bit '1' on position 23 (0x00800000), implied hidden '1'
	slli	a2,a1,9		// clear sign and exponent from num2
	srli	a2,a2,9		// isolate mantissa of num2 -> A2
	or	a2,a2,a4	// set implied hidden bit '1' of num2 mantissa (= 24 bits of num2)
	srli	a3,a2,7		// 17 top bits of divisor (include hidden bit '1') -> A3

	// division to get rough reciprocal value of the divisor 1/num2
	divu	a5,a6,a3	// A5 <- temporary dividend / 17 top bits of divisor

// Here is:
// A0=packed dividend
// A1=packed divisor
// A2=24 bits of divisor mantissa (including hidden implied bit '1')
// A4=1<<23
// A5=reciprocal estimate 1/num2 (16 bits)

	// isolate exponent of dividend num1 -> A3
	srli	a6,a0,23	// get exponent and sign of dividend num1 -> A6
	zext.b	a3,a6		// get dividend exponent -> A3

	// prepare dividend mantissa num1 (including hidden implied bit '1') -> A0
	slli	a0,a0,9		// shift dividend num1 left to clear sign and exponent
	srli	a0,a0,9		// shift dividend num1 back, to isolate mantissa of num1
	or	a0,a0,a4	// set implied hidden bit '1' of num1 mantissa (= 24 bits of num1)

	// get exponent and sign of divisor num2 -> A1
	srli	a1,a1,23

	// prepare result sign in bit 31 -> A6
	xor	a6,a6,a1	// XOR exponents with signs of num1 and num2
	srli	a6,a6,8		// clear exponent, result sign in bit 0
	slli	a6,a6,31	// shift result sign to position 31 -> A6

// Here is:
// A0=24 bits of dividend num1 mantissa (including hidden implied bit '1')
// A1=exponent and sign of divisor num2
// A2=24 bits of divisor num2 mantissa (including hidden implied bit '1')
// A3=dividend num1 exponent
// A4=1<<23
// A5=reciprocal estimate 1/num2 (16 bits)
// A6=result sign in bit 31

	// get and check divisor num2 exponent -> A1
	zext.b	a1,a1		// get divisor exponent -> A1
	beqz	a1,retinf	// divisor num2 is zero, result will be infinity
	li	t0,255		// is divisor num2 infinity?
	beq	a1,t0,20f	// divisor is infinite, result will be zero

	// check dividend num1 exponent
	beqz	a3,retzero	// dividend num1 is zero, result will be zero
	beq	a3,t0,retinf	// dividend num1 is infinity, result will be infinity

	// prepare result exponent -> A3
	sub	a3,a3,a1	// difference of exponents
	addi	a3,a3,125	// add bias - 2

	// get high 16 bits of dividend num1 mantissa -> A1
	srli	a1,a0,8		// dividend mantissa, top 16 bits -> A1

// Here is:
// A0=24 bits of dividend num1 mantissa (including hidden implied bit '1')
// A1=16 bits of dividend num1 mantissa (including hidden implied bit '1')
// A2=24 bits of divisor num2 mantissa (including hidden implied bit '1')
// A3=result exponent
// A4=1<<23
// A5=reciprocal estimate 1/num2 (16 bits)
// A6=result sign in bit 31

	// get first estimate of quotient to 16 bits qu0 -> A1
	mul	a1,a1,a5	// multiply reciproval estimate 1/num2 (16 bits) * dividen num1 (16 bits) = quotient 16 bits << 16
	srli	a1,a1,16	// get first estimate of quotient (16 bits) -> A1

	// get difference of first estimation, remainder re0 -> A4
	slli	a0,a0,15	// bottom 17 bits of dividend num1 mantissa
	mv	a4,a2		// divisor -> A4
	mul	a4,a4,a1	// multiply first result qu0 * divisor num2 (Q38 qu0*x) = first estimate dividend
	sub	a4,a0,a4	// difference of dividend minus estimate dividend (Q38 re0=(y<<15)-qu0*x; note this remainder is signed)

	// get first refinement of quotient to 28 bits -> A1
	srai	a4,a4,10	// remainder re0 >> 10
	mul	a4,a4,a5	// multiply remainder re0 * reciprocal 1/num2 (Q44 qu1=(re0>>10)*u; this quotient correction is also signed)
	srai	a4,a4,16	// Q28
	slli	a1,a1,13	// first estimate quotient << 13
	add	a1,a1,a4	// first refinement Q28 qu=(qu0<<13)+(qu1>>16);

// Here is:
// A0=dividend mantissa Q38
// A1=quotient Q28
// A2=divisor mantissa Q23
// A3=initial result exponent
// R6b31=sign of result

	srli	a4,a1,28
	bnez	a4,1f

// here the quotient is less than 1<<28 (i.e., result mantissa <1.0)

	addi	a1,a1,5
	srli	a4,a1,4		// rounding + small reduction in systematic bias
	andi	t0,a1,8		// test bit 3
	beqz	t0,2f		// skip if we are not near a rounding boundary
	srli	a1,a1,3		// quotient Q25
	slli	a0,a0,10	// dividend mantissa Q48
	mul	a1,a1,a2	// quotient*divisor Q48
	sub	a0,a0,a1	// remainder Q48
	bltz	a0,2f
	j	3f

// here the quotient is at least 1<<28 (i.e., result mantissa >=1.0)

1:	addi	a3,a3,1		// bump exponent (and shift mantissa down one more place)
	addi	a1,a1,9
	srli	a4,a1,5		// rounding + small reduction in systematic bias
	andi	t0,a1,0x10	// test bit 4
	beqz	t0,2f		// skip if we are not near a rounding boundary
	srli	a1,a1,4		// quotient Q24
	slli	a0,a0,9		// dividend mantissa Q47
	mul	a1,a1,a2	// quotient*divisor Q47
	sub	a0,a0,a1	// remainder Q47
	bltz	a0,2f

3:	addi	a4,a4,1		// increment quotient as we are above the rounding boundary

// Here is:
// A3=result exponent
// A4=correctly rounded quotient Q23 in range [1,2] *note closed interval*
// R6b31=sign of result

2:	li	t0,254
	bgeu	a3,t0,10f		// this catches both underflow and overflow
	slli	a1,a3,23
	add	a0,a4,a1
	add	a0,a0,a6
	ret

	// divisor num2 is infinite, check dividend num1 in A3
20:	li	t0,255  	// is dividend infinity too?
	bne	a3,t0,retzero	// dividend is not infinity, result will be zero

retinf:
	// result will be infinity
	li	a0,255		// result exponent = infinity
21:	slli	a0,a0,23	// shift exponent to its position
	or	a0,a0,a6	// add sign to result
	ret

10:	bge	a3,t0,retinf	// overflow?
	addi	a1,a3,1
	bnez	a1,retzero	// exponent <-1? return 0

// here exponent is exactly -1

	srli	a1,a4,25
	srli	t0,a4,24
	andi	t0,t0,1
	beqz	t0,retzero	// mantissa is not 01000000?

// return minimum normal

	li	a0,1
	slli	a0,a0,23
	or	a0,a0,a6
	ret

retzero:
	// result will be zero
	mv	a0,a6		// sign -> result A0
	ret


// ==== convert integer to float

// ----------------------------------------------------------------------------
//                      Convert unsigned int to float
// ----------------------------------------------------------------------------
// float ui2f(u32 num);
// float uint2float(u32 num);
// input: A0 = integer number
// output: A0 = float number
// trashes: A1, A2, A4, A5

float_section _uint2float
.p2align 2,,

wrapper_func __floatunsisf
regular_func ui2f
regular_func uint2float

	// check if number is zero
	sub	a1,a1,a1		// prepare positive result sign -> A1
	bnez	a0,i2f_2		// number is not zero
	ret

// ----------------------------------------------------------------------------
//                      Convert signed int to float
// ----------------------------------------------------------------------------
// float i2f(s32 num);
// float int2float(s32 num);
// input: A0 = integer number
// output: A0 = float number
// trashes: A1, A2, A4, A5

wrapper_func __floatsisf
regular_func i2f
regular_func int2float

	// prepare result sign -> A1
	srli	a1,a0,31		// get sign bit -> A1
	slli	a1,a1,31		// shift sign bit back to position 31

	// absolute value of the number
	bgez	a1,1f			// number is positive
	neg	a0,a0			// absolute value of the number

	// check if number is zero
1:	beqz	a0,10f			// result is 0

	// count leading zeroes -> A0
i2f_2:	mv	a2,a1			// A2 <- sign
	mv	a1,a0			// A1 <- integer number, absolute value
	clz	a0,a0			// A0 <- count leading zeroes

	// normalize number (shift left) -> A1
	sll	a1,a1,a0		// normalize number - shift bits to left -> A1

	// prepare result exponent -> A0
	//   exponent 127 represents '1' + 31 bits of integer without high '1' - count of zeros (=number of shift left)
	addi	a0,a0,-158		// count of zeros - 158
	neg	a0,a0			// A0 <- 158 - count of zeros

	// rounding input number
	//  float mantissa is 23 bits + 1 bit '1' = 24 bits, 8 bits of integer stays unused (=hidden decimal part)
	addi	a1,a1,0x80		// rounding least bit of the mantissa
	bgez	a1,5f			// overflow (carry)

	// check bottom 8 bits (unused float bits) to carry rounding
	slli	a3,a1,24		// check bottom 8 bits of A1
	beqz	a3,6f			// lowest bits of mantissa are = 0, round to even number

	// remove leading hidden bit '1'
	slli	a1,a1,1			// remove leading 1 (hidden 1)

	// composing float number
3:	srli	a1,a1,9			// align mantissa to its position (shift to bits 0..22)
	slli	a0,a0,23		// align exponent (shift to bits 23..30)
	or	a0,a0,a2		// assembler exponent and mantissa
4:	or	a0,a0,a1		// apply sign (add to bit 31)
10:	ret

	// overflow number after rounding - increase exponent and skip shift left
5:	addi	a0,a0,1			// correct exponent
	j	3b			// skip shift left

	// lowest 8 bits are = 0, round mantissa to even number
6:	srli	a1,a1,9			// ensure even result (destroy lowest bit of mantissa)
	slli	a1,a1,10		// shift mantissa back and remove leading hidden bit '1'
	j	3b			// skip shift left

// ----------------------------------------------------------------------------
//                 Convert signed fixed point to float
// ----------------------------------------------------------------------------
//  e = number of bits of fractional part (binary exponent)
// float fix2float(s32 num, int e);
// input: A0 = integer number
//	  A1 = size of fractional part
// output: A0 = float number
// trashes: A1, A2, A4, A5, T0

float_section _fix2float
.p2align 2,,

regular_func fix2float

fix2float_1:
	li	a2,29
	sub	a2,a2,a1	// fix exponent

	// pack number
	li	a5,0		// pack and return, sticky bits=0
// input: A0 = signed mantissa (24 significant bits)
//        A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
//        A5 = sticky bits (used for rounding)
// output: A0 = float number
// trashes: A2, A4, A5, T0
	j	_asm_packx

// ---- continue to ufix2float

// ----------------------------------------------------------------------------
//                 Convert unsigned fixed point to float
// ----------------------------------------------------------------------------
//  e = number of bits of fractional part (binary exponent)
// float ufix2float(u32 num, int e);
// input: A0 = integer number
//	  A1 = size of fractional part
// output: A0 = float number
// trashes: A1, A2, A4, A5, T0

regular_func ufix2float

	bgez	a0,fix2float_1	// treat <2^31 as signed
	li	a2,30
	sub	a2,a2,a1	// fix exponent
	slli	a5,a0,31	// one sticky bit
	srli	a0,a0,1

	// pack number
// input: A0 = signed mantissa (24 significant bits)
//        A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
//        A5 = sticky bits (used for rounding)
// output: A0 = float number
// trashes: A2, A4, A5, T0
	j	_asm_packx

// ----------------------------------------------------------------------------
//                    Convert 64-bit unsigned int to float
// ----------------------------------------------------------------------------
// float ul2f(s64 num);
// float uint642float(u64 num);
// input: A1:A0 = integer number
// output: A0 = float number
// trashes: A1, A2, A4, A5, T0

float_section _uint642float
.p2align 2,,

wrapper_func __floatundisf
regular_func ul2f
regular_func uint642float

	li	a2,0		// fall through

// ---- continue to ufix642float

// ----------------------------------------------------------------------------
//                 Convert 64-bit unsigned fixed point to float
// ----------------------------------------------------------------------------
//  e = number of bits of fractional part (binary exponent)
// float ufix642float(u64 num, int e);
// input: A1:A0 = integer number
//	  A2 = size of fractional part
// output: A0 = float number
// trashes: A1, A2, A4, A5, T0

regular_func ufix642float

	bgez	a1,fix642float_3 // positive? we can use signed code

	slli	a5,a1,31	// contribution to sticky bits
	or	a5,a5,a0
	srli	a0,a1,1
	addi	a2,a2,-1
	j	fix642float_4

// ---- continue to int642float

// ----------------------------------------------------------------------------
//                     Convert 64-bit signed int to float
// ----------------------------------------------------------------------------
// float l2f(s64 num);
// float int642float(s64 num);
// input: A1:A0 = integer number
// output: A0 = float number
// trashes: A1, A2, A4, A5, T0

wrapper_func __floatdisf
regular_func l2f
regular_func int642float

	li	a2,0		// fall through

// ---- continue to fix642float

// ----------------------------------------------------------------------------
//                 Convert 64-bit signed fixed point to float
// ----------------------------------------------------------------------------
//  e = number of bits of fractional part (binary exponent)
// float fix642float(s64 num, int e);
// input: A1:A0 = integer number
//	  A2 = size of fractional part
// output: A0 = float number
// trashes: A1, A2, A4, A5, T0

regular_func fix642float

	// check if number is 0 (result will be +0.0)
fix642float_3:
3:	mv	a5,a0		// number low -> A5
	or	a5,a5,a1	// or number high -> A5
	beqz	a5,5f	// zero? return +0

	// prepare sign bit 0 or -1 -> A5
	srai	a5,a1,31	// sign bits

	// checking the upper 7 bits if they are insignificant
2:	srai	a4,a1,24	// try shifting 7 bits at a time
	bne	a4,a5,1f	// next shift will overflow?

	// shift number by 7 bits left (to fast normalize)
	slli	a1,a1,7		// shift number high by 7 bits left
	srli	a4,a0,25	// high 7 bits of number low -> A4
	or	a1,a1,a4	// carry 7 bits to number high -> A1
	slli	a0,a0,7		// shift number low by 7 bits left
	addi	a2,a2,7		// shift exponent by 7
	j	2b		// next normalisation iterations

	// pack float number
1:	mv	a5,a0		// sticky bits (used for rounding) -> A5
	mv	a0,a1		// partially normalised number -> A0

fix642float_4:
4:	neg	a2,a2		// negate exponent in A2
	addi	a2,a2,32+29	// absolute exponent -> A2

	// pack number
// input: A0 = signed mantissa (24 significant bits)
//        A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
//        A5 = sticky bits (used for rounding)
// output: A0 = float number
// trashes: A2, A4, A5, T0
	j	_asm_packx

5:	ret	


// ==== convert float to integer

// ----------------------------------------------------------------------------
//     Convert float to signed int, rounding to zero (C-style int conversion)
// ----------------------------------------------------------------------------
// s32 f2iz(float num);
// s32 float2int_z(float num);
// input: A0 = float number
// output: A0 = integer number
// trashes: A1, A2, A3

float_section _f2iz
.p2align 2,,

wrapper_func __fixsfsi
regular_func f2iz
regular_func float2int_z

	// get exponent of float number -> A2
	slli	a1,a0,1			// shift 1 bit left to get float number without sign -> A1
	srli	a2,a1,24		// shift 24 bits right to get exponent -> A2

	// check minimal exponent 0.5 - result will be 0
	li	a3,126			// check minimal exponent of 0.5
	ble	a2,a3,1f		// number is too small - result will be 0

	// prepare overflow number 0x80000000
	li	a3,0x80000000		// overflow number base

	// check maximal exponent
	add	a2,a2,-158		// exponent - exponent bias 127 - max. mantissa bits 31 = exponent - 158
	bgez	a2,2f			// number is too big, limit overflow result

	// prepare sign
	srai	a1,a0,31		// prepare sign: positive 0, negative 0xffffffff

	// prepare mantissa
	slli	a0,a0,9			// delete sign and exponent
	srli	a0,a0,1			// mantissa is in bits 8..30
	or	a0,a0,a3		// restore highest hidden bit '1'

	// relative exponent
	neg	a2,a2			// relative exponent -> R2

	// shift mantissa to integer position
	srl	a0,a0,a2		// shift mantissa right by exponent bits

	// compose mantissa with sign
	slli	a1,a1,1			// sign (0 or 0xffffffff) shift left -> 0 or 0xfffffffe
	addi	a1,a1,1			// sign + 1 -> positive 1, negative 0xffffffff
	mul	a0,a0,a1		// multiply mantissa with sign 1 or -1
	ret				// return

	// result is number 0
1:	li	a0,0			// A0 <- 0
	ret				// return

	// number is too big - limit number on overflow
2:	srli	a0,a0,31		// get sign flag into bit 0
	add	a0,a0,a3		// add overflow number 0x80000000
	addi	a0,a0,-1		// -1 -> limited result is 0x7FFFFFFF for positive number, 0x80000000 for negative
	ret				// return

// ----------------------------------------------------------------------------
//                   Convert float to signed int, rounding down
// ----------------------------------------------------------------------------
// s32 float2int(float num);
// s32 f2i(float num);
// input: A0 = float number
// output: A0 = integer number
// trashes: A1, A2, A3, A4

float_section _f2i
.p2align 2,,

regular_func f2i
regular_func float2int

// convert float to signed int, rounding towards -Inf, clamping

	li	a1,0		// fall through

// ---- continue to float2fix

// ----------------------------------------------------------------------------
//               Convert float to signed fixed point, rounding down
// ----------------------------------------------------------------------------
//  e = number of bits of fractional part (binary exponent)
// s32 float2fix(float num, int e);
// input: A0 = float number
//	  A1 = size of fractional part
// output: A0 = integer number
// trashes: A1, A2, A3, A4

regular_func float2fix
.global _asm_float2fix	// use in library with defined register usage
_asm_float2fix:

// convert float in a0 to signed fixed point in a0, clamping

	// unpack number A0 -> A0 mant, A2 exp
// input: A0 = float number
// output: A0 = signed mantissa 9Q23 (24 significant bits)
//         A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
// trashes: A4
	mv	a3,ra		// A3 <- save return address RA
	call	_asm_unpackx	// unpack number A0 -> A0 mant, A2 exp
	mv	ra,a3		// restore RA

	// check if number is zero
	addi	a3,a2,130	// unbias exponent
	bltz	a3,6f		// -0? number is zero

	// check shift direction
	add	a2,a2,a1	// incorporate binary point position into exponent
	addi	a2,a2,-23	// a2 is now amount of left shift required
	bltz	a2,1f		// requires right shift?

	// left shift - check overflow
	li	a3,7		// overflow?
	ble	a2,a3,float2ufix_4 // not overflow - left shift

	// overflow
	srai	a1,a0,31	// +ve:0 -ve:0xffffffff
	not	a1,a1		// invert, +ve:0xffffffff -ve:0
	li	a0,1
	slli	a0,a0,31

float2fix_5:
5:	xor	a0,a0,a1	// +ve:0x7fffffff -ve:0x80000000 (unsigned path: 0xffffffff)
	ret

	// right shift
float2fix_1:
1:	neg	a2,a2		// right shift for a0, >0
	li	a3,31		// max. shift 31
	blt	a2,a3,2f
	mv	a2,a3		// limit number of shifts to 31
2:	sra	a0,a0,a2
	ret

6:	li	a0,0
	ret

// ---- continue to float2uint

// ----------------------------------------------------------------------------
//                 Convert float to unsigned int, rounding down
// ----------------------------------------------------------------------------
// u32 f2ui(float num);
// u32 float2uint(float num);
// input: A0 = float number
// output: A0 = integer number
// trashes: A1, A2, A3, A4

wrapper_func __fixunssfsi
regular_func f2ui
regular_func float2uint

	li	a1,0		// fall through

// ---- continue to float2ufix

// ----------------------------------------------------------------------------
//             Convert float to unsigned fixed point, rounding down
// ----------------------------------------------------------------------------
//  e = number of bits of fractional part (binary exponent)
// u32 float2ufix(float num, int e);
// input: A0 = float number
//	  A1 = size of fractional part
// output: A0 = integer number
// trashes: A1, A2, A3, A4

regular_func float2ufix

	// unpack number A0 -> A0 mant, A2 exp
// input: A0 = float number
// output: A0 = signed mantissa 9Q23 (24 significant bits)
//         A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
// trashes: A4
	mv	a3,ra		// A3 <- save return address RA
	call	_asm_unpackx	// unpack number A0 -> A0 mant, A2 exp
	mv	ra,a3		// restore RA

	add	a2,a2,a1	// incorporate binary point position into exponent
	mv	a1,a0
	bltz	a1,float2fix_5	// negative? return zero

	addi	a2,a2,-23	// a2 is now amount of left shift required
	bltz	a2,float2fix_1	// requires right shift?

	not	a1,a0		// ready to return 0xffffffff
	li	a3,8
	bgt	a2,a3,float2fix_5 // overflow

	// left shift
float2ufix_4:
4:	sll	a0,a0,a2	// result fits, left shifted
	ret

// ----------------------------------------------------------------------------
// Convert float to 64-bit signed int, rounding to zero (C-style int conversion)
// ----------------------------------------------------------------------------
// s64 f2lz(float num);
// s64 float2int64_z(float num);
// input: A0 = float number
// output: A1:A0 = integer number
// trashes: A2, A3, A4, A5, T0, T1

float_section _f2lz
.p2align 2,,

wrapper_func __fixsfdi
regular_func f2lz
regular_func float2int64_z

	// if number is not negative, convert with rounding down
// input: A0 = float number
// output: A1:A0 = integer number
// trashes: A2, A3, A4, A5, T0, T1
	bgez	a0,float2int64	// number is not negative - convert with rounding down

	// convert as unsigned, using absolute value
	slli	a0,a0,1		// discard sign bit
	srli	a0,a0,1		// shift back

// input: A0 = float number
// output: A1:A0 = integer number
// trashes: A2, A3, A4, T0, T1
	mv	a5,ra		// save return address
	call	float2uint64	// convert to u64
	mv	ra,a5

	// check overflow signed range
	bltz	a1,1f		// overflow

	// negate s64 result
	neg	a0,a0		// negate result LOW -> A0
	snez	a2,a0		// AA <- 1 if A0 not zero
	neg	a1,a1		// negate result HIGH -> A1
	sub	a1,a1,a2	// correct result HIGH if carry from LOW
	ret

	// return max. negative value
1:	li	a1,128		// A1 <- 0x80
	slli	a1,a1,24	// A1 <- 0x800... max. negative value
	li	a0,0		// A0 <- 0
	ret

// ---- continue to float2int64

// ----------------------------------------------------------------------------
//              Convert float to 64-bit signed int, rounding down
// ----------------------------------------------------------------------------
// s64 f2l(float num);
// s64 float2int64(float num);
// input: A0 = float number
// output: A1:A0 = integer number
// trashes: A2, A3, A4, A5, T0, T1

regular_func f2l
regular_func float2int64

	li	a1,0		// 2nd parameter - number of bits = 0

// ---- continue to float2fix64

// ----------------------------------------------------------------------------
//         Convert float to 64-bit signed fixed point, rounding down
// ----------------------------------------------------------------------------
//  e = number of bits of fractional part (binary exponent)
// s64 float2fix64(float num, int e);
// input: A0 = float number
//	  A1 = size of fractional part
// output: A1:A0 = integer number
// trashes: A2, A3, A4, A5, T0, T1

regular_func float2fix64

	// convert float to signed fixed point
// input: A0 = float number
//        A1 = result exponent (number of bits after decimal point of output integer)
// output: A3:A1:A0 = signed fixed point (clamped, A3 is 0 or -1)
// trashes: A2, A4, T0, T1
	mv	a5,ra		// save return address -> A5
	call	f2fix		// convert float to signed fixed point
	mv	ra,a5

	// check overflow signed int64
	srai	a2,a1,31	// get sign bit of A1 -> A2
	bne	a2,a3,1f	// sign extension bits fail to match sign of result?
	ret

	// signed int64 overflow, return max. number
1:	not	a0,a3		// not sign -> A0 (0 on negative, -1 on positive)
	li	a1,1		// A1 <- 1
	slli	a1,a1,31	// A1 <- bit 31
	xor	a1,a1,a0	// max. integer -> A1 (0x800.. on negative, 0x7FF.. on positive)
	ret

// ----------------------------------------------------------------------------
//                    Convert float to 64-bit unsigned int
// ----------------------------------------------------------------------------
// u64 f2ul(float num);
// u64 float2uint64(float num);
// input: A0 = float number
// output: A1:A0 = integer number
// trashes: A2, A3, A4, T0, T1

float_section _f2ul
.p2align 2,,

wrapper_func __fixunssfdi
regular_func f2ul
regular_func float2uint64

	li	a1,0			// 2nd parameter - number of bits = 0

// ---- continue to float2ufix64

// ----------------------------------------------------------------------------
//                  Convert float to 64-bit unsigned fixed point
// ----------------------------------------------------------------------------
//  e = number of bits of fractional part (binary exponent)
// u64 float2ufix64(float num, int e);
// input: A0 = float number
//	  A1 = size of fractional part
// output: A1:A0 = integer number
// trashes: A2, A3, A4, T0, T1

regular_func float2ufix64

	// check if number is negative - return 0 to be clamped to unsigned int
	srai	a3,a0,23	// get sign and exponent -> A3
	bltz	a3,5f		// number is negative, return -0.0

// ---- convert float to signed fixed point, rounding down

// input: A0 = float number
//        A1 = result exponent (number of bits after decimal point of output integer)
// output: A3:A1:A0 = signed fixed point (clamped, A3 is 0 or -1)
// trashes: A2, A4, T0, T1

f2fix:
	// save register A1 -> T0
	mv	t0,a1		// required exponent -> T0

	// get sign -> A3
	srai	a3,a0,31	// get sign bit 0 or -1 -> A3
	slli	a0,a0,1		// delete sign bit from float

	// get exponent -> A2
	srli	a2,a0,24	// isolate exponent -> A2
	beqz	a2,1f		// zero number, result is zero
	li	t1,0xff
	beq	a2,t1,2f	// infinity, result is max number

	// get absolute exponent
	addi	a1,a2,-1	// exponent - 1 -> A1
	addi	a2,a2,-0x7f	// subtract exponent bias 127 - 1

	// isolate mantissa and restore hidden implied bit '1' -> A0
	slli	a1,a1,24	// shift 'exponent - 1' back to position 24
	sub	a0,a0,a1	// insert back hidden implied highest bit '1'

	// absolute value of mantissa
	xor	a0,a0,a3	// NOT mantissa with negative sign -1
	sub	a0,a0,a3	// negative mantissa correction to get two's complement

	// shift mantissa
	srai	a1,a0,4		// convert mantissa to int
	slli	a0,a0,28	// mantissa low

// input: A1:A0 = two's complement mantissa
//        A2 = unbaised exponent
//        A3 = mantissa sign extension bits (0 or -1)
//        T0 = required output exponent
// output: A3:A1:A0 = signed fixed point (clamped, A3 is 0 or -1)
// trashes: A2, A4, T1
	j	d2fix_a		// shift mantissa A3:A1:A0 by A2+T0 bits

	// result is plus zero +0.0
1:	li	a0,0		// 0 -> A0
	mv	a1,a0		// 0 -> A1
	mv	a3,a0		// 0 -> A3
	ret

	// result is overflow, return max integer (0 on negative number, -1 on positive)
2:	not	a0,a3		// inverse sign -> A0
	not	a1,a3		// inverse sign -> A1
	ret

	// result is minus zero -0.0 (here is A3 = sign 0 or -1)
5:	li	a0,0		// 0 -> A0
	li	a1,0		// 0 -> A1
	ret

// ---- continue conversion float/double to int64 ... shift mantissa A3:A1:A0 by A2+T0 bits

// input: A1:A0 = two's complement mantissa
//        A2 = unbaised exponent
//        A3 = mantissa sign extension bits (0 or -1)
//        T0 = required output exponent
// output: A3:A1:A0 = signed fixed point (clamped, A3 is 0 or -1)
// trashes: A2, A4, T1

regular_func d2fix_a

	// prepare shift up by A2 bits
	add	a2,a2,t0	// exponent plus offset for required binary point position
	addi	a2,a2,-52	// required shift
	bltz	a2,1f		// shift down?

	// check shift up overflow
	li	t1,12
	bge	a2,t1,2f	// clamp shift up
	beqz	a2,20f		// shift is 0

	// shift mantissa up (left) by A2 bits
	mv	a4,a0		// save mantissa low -> A4
	sll	a1,a1,a2	// shift mantissa high by A2 bits left -> A1
	sll	a0,a0,a2	// shift mantissa low by A2 bits left -> A0
	neg	a2,a2		// negate number of shifts
	addi	a2,a2,32	// complementary shift 32-A2
	srl	a4,a4,a2	// shift mantissa low by 32-A2 bits right
	or	a1,a1,a4	// add carry from mantissa low to new mantissa high -> A1
20:	ret

	// clamp shift up, return max integer (0 on negative number, -1 on positive)
2:	not	a0,a3		// not sign -> A0
	not	a1,a3		// not sign -> A1
	ret

	// prepare shift down (right) by A2 bits
1:	addi	a2,a2,32	// required shift
	blez	a2,1f		// long shift down

	// shift mantissa down by A2 bits
	mv	a4,a1		// save mantissa high -> A4
	sll	a4,a4,a2	// shift mantissa high by A2 bits left -> A4
	neg	a2,a2		// negate number of shifts
	addi	a2,a2,32	// complementary shift
	sra	a1,a1,a2	// shift mantissa high by A2 bits right -> A1
	srl	a0,a0,a2	// shift mantissa low by A2 bits right -> A0
	or	a0,a0,a4	// add carry from mantissa high to new mantissa low -> A0
	ret

	// long shift down
1:	mv	a0,a1		// A0 <- mantissa high
	srai	a1,a1,31	// shift mantissa high by 31 bits to get only sign 0 or -1
	addi	a2,a2,32	// required number of shifts
	blez	a2,2f		// very long shift
	neg	a2,a2		// negate number of shifts
	addi	a2,a2,32	// complementary shift 32-A2
	sra	a0,a0,a2	// shift mantissa low by A2 bits right
	ret

	// very long shift down
2:	mv	a0,a3		// result will be 0 or -1 (taken from sign A2)
	mv	a1,a3		// result high
	ret

// ----------------------------------------------------------------------------
//                           Exchange operands
// ----------------------------------------------------------------------------
// input: A0,A2 = 1st operand
//	  A1,A3 = 2nd operand
// output: A0,A2 = 2nd operand
//	  A1,A3 = 2nd operand
// trashes:

// ---- exchange a0<->a1, a2<->a3

float_section _asm_xchxy
.p2align 2,,

.global _asm_xchxy
_asm_xchxy:
	xor	a0,a0,a1	// A0 <- A0 ^ A1
	xor	a1,a0,a1	// A1 <- (A0 ^ A1) ^ A1 = A0
	xor	a0,a0,a1	// A0 <- (A0 ^ A1) ^ A0 = A1

	xor	a2,a2,a3	// A2 <- A2 ^ A3
	xor	a3,a2,a3	// A3 <- (A2 ^ A3) ^ A3 = A2
	xor	a2,a2,a3	// A2 <- (A2 ^ A3) ^ A2 = A3
	ret

// ----------------------------------------------------------------------------
//                       Unpack float number
// ----------------------------------------------------------------------------
// input: A0 = float number
// output: A0 = signed mantissa 9Q23 (24 significant bits)
//         A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
// trashes: A4

float_section _asm_unpackx
.p2align 2,,

.global _asm_unpackx
_asm_unpackx:

	// extract exponent and sign -> A2
	mv	a4,a0		// A4 <- save the number
	srli	a2,a0,23	// A2 <- exponent and sign

	// extract mantissa -> A0 (23 bits)
	slli	a0,a0,9
	srli	a0,a0,9

	// restore implied leading bit '1' in mantissa
	bseti	a0,a0,23	// set hidden bit '1'

	// negate mantissa
	bgez	a4,1f		// number is not negative
	bclri	a2,a2,8		// clear sign bit from exponent
	neg	a0,a0		// negate mantissa

	// check zero/inf
1:	addi	a2,a2,-1	// exponent - 1 (0 becomes 0xffffffff)
	li	a4,254
	bgeu	a2,a4,2f	// number is zero or infinity

	// convert exponent to signed
	addi	a2,a2,-126	// remove exponent bias (= 127 - 1)
	ret			// return

	// zero or infinity - limit mantissa to +1 or -1
2:	mv	a4,a0		// A4 <- result mantissa
	li	a0,0x00800000	// set mantissa to +1 (= 0x00800000)
	bgez	a4,3f		// mantissa is positive
	neg	a0,a0		// negate mantissa - set mantissa to -1 (= 0xff800000)

	// exponent correction
3:	addi	a2,a2,-126	// exponent - 126: zero -> -127, infinity -> 128
	slli	a2,a2,2		// exponent * 4: zero -> -508, infinity -> 512
	addi	a2,a2,128	// exponent + 128: zero -> -380, infinity -> 640
	ret			// return

// ----------------------------------------------------------------------------
//                      Normalize and pack signed mantissa
// ----------------------------------------------------------------------------
// input: A0 = signed mantissa (24 significant bits)
//        A2 = signed exponent (bias removed) (zero = -380, infinity = +640)
//        A5 = sticky bits (used for rounding)
// output: A0 = float number
// trashes: A2, A4, A5, T0

float_section _asm_packx
.p2align 2,,

.global _asm_packx
_asm_packx:

	// extract sign bit from mantissa -> A4 bit 31
	srli	a4,a0,31	// shift mantissa 31 bits right
	slli	a4,a4,31	// shift sign bit back to position 31

	// absolute value of mantissa, and add carry from sticky bits in A5
	bgez	a4,2f		// mantissa is positive
	beqz	a5,11f		// do not rounding
	addi	a0,a0,1		// round mantissa up
11:	neg	a0,a0		// negate mantissa
	bltz	a0,3f		// mantissa was 0x80000000, skip normalization

	// normalize mantissa
2:	beqz	a0,1f		// mantissa is zero, stop normalization
	clz	t0,a0		// T0 <- count leading zeroes
	sll	a0,a0,t0	// normalize mantissa
	sub	a2,a2,t0	// shift exponent

	// add exponent bias
3:	addi	a2,a2,129	// add bias + 2
	bnez	a2,12f		// not zero exponent (special case: highest denormal can round to lowest normal)
	addi	a0,a0,0x80	// zero exponent - add rounding 0x80 correction to mantissa
	bgez	a0,4f		// carry (A0 could have bit 31 set), skip rounding

	// rounding mantissa
12:	addi	a0,a0,0x80	// add rounding correction 128 to mantissa
	bgez	a0,4f		// carry (A0 could have bit 31 set), skip rounding from sticky bits
	beqz	a5,7f		// sticky bits are zero

	// shift mantissa left to remove leading bit 1
8:	slli	a0,a0,1		// remove leading bit 1
9:	addi	a2,a2,-1	// compensate exponent

	// check exponent overflow
4:	li	a5,254		// overflowed exponent
	bge	a2,a5,5f	// exponent overflow

	// correct exponent offset
	addi	a2,a2,1		// correct exponent offset
	blez	a2,10f		// zero or underflow

	// assemble exponent and mantissa
	srli	a0,a0,9		// align mantissa
	slli	a2,a2,23	// align exponent
	or	a0,a0,a2	// assemble exponent and mantissa
6:	or	a0,a0,a4	// apply sign
1:	ret			// return

	// overflow, return +-infinity
5:	li	a0,0xff		// result will be infinity
	slli	a0,a0,23	// shift exponent to position
	j	6b		// appli sign

	// underflow, return 0
10:	li	a0,0		// result will be 0
	ret			// return

	// rounding using sticky bits
7:	slli	a5,a0,24	// check bottom 8 bits of a0
	bnez	a5,8b		// rounding in normal way
	srli	a0,a0,9		// ensure even result of mantissa if sticky bits = 0
	slli	a0,a0,10	// shift mantissa back
	j	9b		// compentase exponent

#endif // !USE_FLOATLIBC
#endif // USE_FLOAT

#if !USE_FLOATLIBC

// This is a hack to improve soft float performance for the routines we don't
// implement (e.g. libm) in libraries built against a non-Zbb ISA dialect:

float_section __clz2si
.p2align 2,,

.global __wrap___clzsi2
.global __clz2si
.global __clzsi2
//.global clz
__wrap___clzsi2:
__clz2si:
__clzsi2:
//clz:
	clz		a0,a0
	ret

#endif // !USE_FLOATLIBC
