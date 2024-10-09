
// ****************************************************************************
//
//                       Float RP2350 - RISC-V science functions
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040

#include "../asm_include.inc"

#if USE_FLOAT		// use float support 1=in RAM, 2=in Flash

#if USE_FLOAT == 1
#define float_section section_noflash
#else
#define float_section section_text
#endif

#if !USE_FLOATLIBC

#define FLOAT_EXP1	0x7f		// exponent of '1' (= 127)
#define SINF_FRACT	28		// number of fraction bits
#define SINF_INT	(32-SINF_FRACT)	// number of integer bits (= 4)

// multiply 2 registers and shift right
// dst ... destination register
// num1 ... source register 1 (can be the same as dst)
// num2 ... source register 2 (can be the same as num2, but not as dst)
// tmp ... temporary register
.macro sinf_mulx2 dst num1 num2 tmp
	mulhu	\tmp,\num1,\num2	// multiply HIGH
	mul	\dst,\num1,\num2	// multiply LOW
	srli	\dst,\dst,SINF_FRACT	// LOW >> SINF_FRACT
	slli	\tmp,\tmp,SINF_INT	// HIGH << 32-SINF_FRACT
	or	\dst,\dst,\tmp		// complete result
.endm

// multiply member in A2 by constant and shift right
// reg ... register with member
// num ... constant
// shft ... shift result right
// tmp ... temporary register
.macro sinf_mulc reg num shft tmp
	li	\tmp,\num		// load constant
	mulhu	\reg,\reg,\tmp		// multiply HIGH
	srli	\reg,\reg,\shft		// shift riht
.endm

// ----------------------------------------------------------------------------
//                               sinf(x)
// ----------------------------------------------------------------------------
// float sinf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result sin(x)
// TRASHED: A1, A2, A3, A4 (+ registers from remainderf function)
// Function size: 396 bytes

float_section _sinf
.p2align 2,,
// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - adds incorrect alignment
// after the end of the section. Use commas ,, to work a little better.

.global __wrap_sinf
.global sinf
__wrap_sinf:
sinf:
	// get exponent -> A1
	srl	a1,a0,23		// A1 <- X >> 23
	zext.b	a1,a1			// mask exponent (zero extend byte)

	// infinity
	li	a2,255			// A2 <- 255, exponent of infinity
	beq	a1,a2,sinf_ret		// undefined result

	// small angle not changed (exp < FLOAT_EXP1 - 9)
	li	a2,FLOAT_EXP1-9		// minimal exponent (= 127-9 = 118)
	bltu	a1,a2,sinf_ret		// small angle not changed

	// normalize big angles (if abs(x) >= 8), A0 = angle
	li	a2,FLOAT_EXP1+SINF_INT-1 // (= 127+4-1 = 130)
	bltu	a1,a2,1f		// angle is < 8

	add	sp,sp,-8		// space in the stack
	sw	ra,4(sp)		// save return address
	li	a1,0x40c90fdb		// 2*pi float
	call	remainderf		// call function "remainder"
	lw	ra,4(sp)		// restore return address
	add	sp,sp,8			// destroy space in the stack

	// small angle
1:	srl	a1,a0,23		// A1 <- X >> 23
	zext.b	a1,a1			// mask exponent (zero extend byte)
	li	a2,FLOAT_EXP1-9		// minimal exponent (= 127-9 = 118)
	bltu	a1,a2,sinf_ret		// small angle not changed

	// save sign -> A4 (value 0 or 1)
	srli	a4,a0,31		// A4 <- get sign (value 0 or 1)

	// normalize mantissa - shift to max position and restore hidden bit '1'
	slli	a0,a0,8			// shift X left by 8 bits
	bset	a0,a0,31		// set hidden bit '1'

	// shift mantissa right by exponent bits
	// - get angle as signed fixed number in format 4.28
	li	a2,FLOAT_EXP1+31-SINF_FRACT // 127+31-28 = 130
	sub	a2,a2,a1		// exp = 130 - exp
	srl	a0,a0,a2		// shift mantissa right

	// modulo angle to range 0..2pi
	li	a1,26986075409ll >> SINF_INT // A1 <- 2pi in format 4.28 (= (2*pi * 2^32) >> 4 )
1:	sub	a0,a0,a1		// mantissa - 2*pi
	bgez	a0,1b			// while mantissa >= 0
1:	add	a0,a0,a1		// mantissa + 2*pi
	bltz	a0,1b			// while mantissa < 0

	// convert to range 0..PI
	srli	a1,a1,1			// A1 <- pi in format 4.28
	blt	a0,a1,1f		// mantissa < PI
	sub	a0,a0,a1		// mantissa - PI
	xor	a4,a4,1			// invert sign
	
	// convert to range 0..PI/2
1:	srli	a2,a1,1			// A2 <- pi/2 in format 4.28
	blt	a0,a2,1f		// mantissa < PI/2
	sub	a0,a1,a0		// mantissa <- PI - mantissa

	// prepare x^2 -> A1
1:	sinf_mulx2 a1 a0 a0 a2		// A1 <- (x*x = x^2) >> SINF_FRACT

	// prepare first member x^1/1! -> A2, accumulator -> A0
	mv	a2,a0			// A2 (=m member) <- x, A0 (=acc accumulator)

	// - x^3/3!
	sinf_mulc a2 0xAAAAAAAB 2 a3	// A2 (m) <- (m*const) >> 2 ... 2^(32+2)/(2*3) = 2863311531 round up
	sinf_mulx2 a2 a2 a1 a3		// A2 (m) <- (m*x^2) >> SINF_FRACT
	sub	a0,a0,a2		// acc = acc - m

	// + x^5/5!
	sinf_mulx2 a2 a2 a1 a3		// A2 (m) <- (m*x^2) >> SINF_FRACT
	li	a3,4*5			// minimal value of member
	blt	a2,a3,sinf_ok		// member will become 0
	sinf_mulc a2 0xCCCCCCCD 4 a3	// A2 (m) <- (m*const) >> 4 ... 2^(32+4)/(4*5) = 3435973837 round up
	add	a0,a0,a2		// acc = acc + m

	// - x^7/7!
	sinf_mulx2 a2 a2 a1 a3		// A2 (m) <- (m*x^2) >> SINF_FRACT
	li	a3,6*7			// minimal value of member
	blt	a2,a3,sinf_ok		// member will become 0
	sinf_mulc a2 0xC30C30C4 5 a3	// A2 (m) <- (m*const) >> 5 ... 2^(32+5)/(6*7) = 3272356036 round up
	sub	a0,a0,a2		// acc = acc - m

	// + x^9/9!
	sinf_mulx2 a2 a2 a1 a3		// A2 (m) <- (m*x^2) >> SINF_FRACT
	li	a3,8*9			// minimal value of member
	blt	a2,a3,sinf_ok		// member will become 0
	sinf_mulc a2 0xE38E38E4 6 a3	// A2 (m) <- (m*const) >> 6 ... 2^(32+6)/(8*9) = 3817748708 round up
	add	a0,a0,a2		// acc = acc + m

	// - x^11/11!
	sinf_mulx2 a2 a2 a1 a3		// A2 (m) <- (m*x^2) >> SINF_FRACT
	li	a3,10*11		// minimal value of member
	blt	a2,a3,sinf_ok		// member will become 0
	sinf_mulc a2 0x94F20950 6 a3	// A2 (m) <- (m*const) >> 6 ... 2^(32+6)/(10*11) = 2498890064 round up
	sub	a0,a0,a2		// acc = acc - m

	// + x^13/13!
	sinf_mulx2 a2 a2 a1 a3		// A2 (m) <- (m*x^2) >> SINF_FRACT
	li	a3,12*13		// minimal value of member
	blt	a2,a3,sinf_ok		// member will become 0
	sinf_mulc a2 0xD20D20D3 7 a3	// A2 (m) <- (m*const) >> 7 ... 2^(32+7)/(12*13) = 3524075731 round up
	add	a0,a0,a2		// acc = acc + m

sinf_ok:
	// result is 0
	beqz	a0,sinf_ret

	// convert mantissa to float
	clz	a1,a0		// get number of leading zeroes
	sll	a0,a0,a1	// shift mantissa to max. position
	slli	a0,a0,1		// destroy hidden bit '1'
	srli	a0,a0,9		// shift mantissa to final position
	li	a2,FLOAT_EXP1+31-SINF_FRACT // 127+31-28 = 130
	sub	a2,a2,a1	// exponent
	slli	a2,a2,23	// shift exponent to final position
	or	a0,a0,a2	// add exponent to mantissa

	// correct sign
	slli	a4,a4,31	// shift sign to bit 31
	xor	a0,a0,a4	// correct sign

sinf_ret:
	// return
	ret

#endif // !USE_FLOATLIBC
#endif // USE_FLOAT


/*
INLINE u64 UMul(u32 a, u32 b) { return (u64)a*b; }
INLINE u32 UMulHigh(u32 a, u32 b) { return (u32)(((u64)a*b) >> 32); }

// sine in radians
float sinf_fast(float x)
{
	// get exponent
	int exp = Test_GetExpF(x); // get exponent, unsigned

	// infinity
	if (exp == 0xff) return x;

	// small angle
	if (exp < FLOAT_EXP1 - 9) return x; // 11 = FLOAT_MANTBITS/2

#define SINF_FRACT	28
#define SINF_INT	(32-SINF_FRACT)	// = 4

	// normalize big angles
	if (exp >= FLOAT_EXP1+SINF_INT-1) x = remainderf(x, (float)PI2);

	// small angle
	exp = Test_GetExpF(x); // get exponent, unsigned
	if (exp < FLOAT_EXP1 - 9) return x; // 11 = FLOAT_MANTBITS/2

	// save sign
	Bool sign = (Bool)Test_GetSignF(x);

	// normalize mantissa - shift to max position and restore hidden bit '1'
	u32 m = (Test_floatu32(x) << 8) | B31;

	// shift mantissa right by exponent bits
	// - get angle as signed fixed number in format 4.28
	m >>= -(exp + SINF_FRACT - 127 - 31);

	// modulo angle to range 0..2pi
	s32 p = (s32)(26986075409ll >> SINF_INT); // 2pi in format 4.28 (= (2*pi * 2^32) >> 4 )
	s32 n = m;
	do n -= p; while (n >= 0);
	do n += p; while (n < 0);

	// convert to range 0..PI
	p >>= 1; // pi
	if (n >= p) // x >= PI ?
	{
		n -= p;
		sign = !sign;
	}

	// convert to range 0..PI/2
	s32 p2 = p >> 1; // pi/2
	if (n >= p2) n = p - n;

	// prepare Taylor serie
	u32 n2 = (u32)(UMul(n, n) >> SINF_FRACT); // mant * x^2

	// = x^1/1!
	s32 acc = n; // acc = x^1/1!

	// - x^3/3!
	n = (u32)(UMul(n, n2) >> SINF_FRACT); // mant * x^2
	n = UMulHigh(n, 0xAAAAAAAB) >> 2; // 2^34/(2*3) = 2863311531 round up
	acc -= n;

	// + x^5/5!
	n = (u32)(UMul(n, n2) >> SINF_FRACT); // mant * x^2
	if (n >= 4*5)
	{
		n = UMulHigh(n, 0xCCCCCCCD) >> 4; // 2^36/(4*5) = 3435973837 round up
		acc += n;

		// - x^7/7!
		n = (u32)(UMul(n, n2) >> SINF_FRACT); // mant * x^2
		if (n >= 6*7)
		{
			n = UMulHigh(n, 0xC30C30C4) >> 5; // 2^37/(6*7) = 3272356036 round up
			acc -= n;

			// + x^9/9!
			n = (u32)(UMul(n, n2) >> SINF_FRACT); // mant * x^2
			if (n >= 8*9)
			{
				n = UMulHigh(n, 0xE38E38E4) >> 6; // 2^38/(8*9) = 3817748708 round up
				acc += n;

				// - x^11/11!
				n = (u32)(UMul(n, n2) >> SINF_FRACT); // mant * x^2
				if (n >= 10*11)
				{
					n = UMulHigh(n, 0x94F20950) >> 6; // 2^38/(10*11) = 2498890064 round up
					acc -= n;

					// + x^13/13!
					n = (u32)(UMul(n, n2) >> SINF_FRACT); // mant * x^2
					if (n >= 12*13)
					{
						n = UMulHigh(n, 0xD20D20D3) >> 7; // 2^39/(12*13) = 3524075731 round up
						acc += n;
					}
				}
			}
		}
	}

	// correct sign
	if (sign) acc = -acc;

	// convert back to float
	return fix2float(acc, SINF_FRACT);
}
*/

// ----------------------------------------------------------------------------
//                            Square root
// ----------------------------------------------------------------------------
// float sqrtf(float x);
// INPUT: A0 = argument x
// OUTPUT: A0 = result sqrtf(x)
// TRASHED: A1, A2, A3, A4
// Function size: 396 bytes

float_section _sqrtf
.p2align 2,,
// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - adds incorrect alignment
// after the end of the section. Use commas ,, to work a little better.

.global __wrap_sqrtf
.global sqrtf
__wrap_sqrtf:
sqrtf:

// >>> This code is an adaptation of the fsqrt function from Pico1 M0+ (c) Raspberry Pi <<<

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
