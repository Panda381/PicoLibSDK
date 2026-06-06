
// ****************************************************************************
//
//                       Float RP2350 - RISC-V science functions
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
//
// This file is not compiled in the PicoLibSDK library - it just serves as
// a repository for the original C functions before they are rewritten into
// assembly. C function alternatives can be used to compile for processors
// other than RISC-V Hazard3.

// Functions were tested with the FPUTEST application from the PicoLibSDK  
// https://github.com/Panda381/PicoLibSDK/tree/main/PicoPad/TEST/FPUTEST

// compilation setup
#define USE_RISCV_EXPF		1		// RISC-V expf(), exp2f() and exp10f() function: 1=Chebyshev (recommended), 2=Taylor
#define EXPF_MODE		1		// expf mode: 1=expf, 2=exp2f, 3=exp10f

#define USE_RISCV_LOGF		1		// RISC-V logf(), log2f() and log10f() function: 1=Chebyshev (recommended), 2=Mercator
#define LOGF_MODE		1		// logf mode: 1=logf, 2=log2f, 3=log10f

#define USE_RISCV_SINF		2		// RISC-V sinf(), cosf(), sincosf(), tanf(), cotanf() functions: 1=Chebyshev, 2=Taylor (recommended)

#define USE_RISCV_ASINF		2		// RISC-V asinf(), acosf() functions: 2=Taylor (recommended)
#define USE_RISCV_ATANF		1		// RISC-V atanf(), acotanf(), atan2f() functions: 1=Chebyshev (recommended)

// pre-definitions
#define FLOAT_BITS		32		// float - number of bits
#define FLOAT_MANTBITS		23		// float - number of bits of mantissa
#define FLOAT_EXPMASK		0xff		// float - exponent mask (8 bits)
#define FLOAT_EXP1		0x7f		// float - exponent of '1'

typedef signed long int s32;			// on 64-bit system use "signed int"
typedef unsigned long int u32;			// on 64-bit system use "unsigned int"

typedef union { float f; u32 n; } float_u32;

u32 Test_floatu32(float f) { float_u32 tmp; tmp.f = f; return tmp.n; }
float Test_u32float(u32 n) { float_u32 tmp; tmp.n = n; return tmp.f; }

float Test_PInfF()  { return Test_u32float(0x7f800000); }	// +1.#INF
float Test_PZeroF() { return Test_u32float(0x00000000); }	// +0.0
float Test_P1F()    { return Test_u32float(0x3F800000); }	// +1.0

int Test_GetExpF(float num) { return (int)((Test_floatu32(num) >> FLOAT_MANTBITS) & FLOAT_EXPMASK); }
int Test_GetSignF(float num) { return (int)((Test_floatu32(num) >> (FLOAT_BITS-1)) & 1); }

#define B31 (1UL<<31)

u32 MyUMulHigh(u32 a, u32 b) { return (u32)(((u64)a*b) >> 32); }
s32 MySMulHigh(s32 a, s32 b) { return (s32)(((s64)a*b) >> 32); }
u64 MyUMul(u32 a, u32 b) { return (u64)a*b; }
s64 MySMul(s32 a, s32 b) { return (s64)a*b; }

// ----------------------------------------------------------------------------
//                    Natural exponent - Chebyshev approximation
// ----------------------------------------------------------------------------

// 32-bit Chebyshev coefficients of Exp() - see calculations in real_c.h
// const REALEXT REALNAME(ConstChebExp)[CHEB_EXP] = {	// CHEB_EXP = 10
//	 { { 0x80000000 }, 0x3fffffff},	// 0  (1)
//	 { { 0xb17217f8 }, 0x3ffffffe},	// 1  (0.693147180601954)
//	 { { 0xf5fdeff0 }, 0x3ffffffc},	// 2  (0.240226506255567)
//	 { { 0xe35846b5 }, 0x3ffffffa},	// 3  (0.0555041086190613)
//	 { { 0x9d956176 }, 0x3ffffff8},	// 4  (0.00961813466710737)
//	 { { 0xaec4023e }, 0x3ffffff5},	// 5  (0.00133335616465047)
//	 { { 0xa1805ca9 }, 0x3ffffff2},	// 6  (0.000154019750482348)
//	 { { 0xffe1c9dc }, 0x3fffffee},	// 7  (1.52517549025788e-005)
//	 { { 0xb3c1ad20 }, 0x3fffffeb},	// 8  (1.3392906481613e-006)
//	 { { 0xdcf8ad51 }, 0x3fffffe7},	// 9  (1.02897824066917e-007)
// };

#if USE_RISCV_EXPF == 1		// RISC-V expf(), exp2f() and exp10f() function: 1=Chebyshev (recommended), 2=Taylor

// expf: 664ns avg=0.119 max=1
// exp2f: 596ns avg=0.106 max=1
// exp10f: 723ns avg=0.384 max=1
float expf_fast(float x)
{
	// get number as integer
	u32 xn = Test_floatu32(x);

	// max. value (x > 0), return +inf
	if ((s32)xn >= 0)
	{
#if EXPF_MODE == 3 // exp10f
		// exp10f max. value: 38.531837 = 0x421A209A, infinity: 38.531841 = 0x421A209B
		if (xn > 0x421A209A) return Test_PInfF();
#elif EXPF_MODE == 2 // exp2f
		// exp2f max. value: 127.99999 = 0x42FFFFFF, infinity: 128.00000 = 0x43000000
		if (xn > 0x42FFFFFF) return Test_PInfF();
#else // expf
		// expf max. value: 88.722832 = 0x42B17217, infinity: 88.722839 = 0x42B17218
		if (xn > 0x42B17217) return Test_PInfF();
#endif
	}

	// min. value (x < 0), return +0.0
	else
	{
#if EXPF_MODE == 3 // exp10f
		// exp10f min. value: -37.929779 = 0xC217B818, zero: -37.929783 = 0xC217B819
		if (xn > 0xC217B818) return Test_PZeroF();
#elif EXPF_MODE == 2 // exp2f
		// exp2f min. value: -126.00000 = 0xC2FC0000, zero: -126.00001 = 0xC2FC0001
		if (xn > 0xC2FC0000) return Test_PZeroF();
#else // expf
		// expf min. value: -87.336540 = 0xC2AEAC4F, zero: -87.336548 = 0xC2AEAC50
		if (xn > 0xC2AEAC4F) return Test_PZeroF();
#endif
	}

	// get signed exponent
	int exp = Test_GetExpF(x) - FLOAT_EXP1;

	// check minimal exponent - return 1.0
	//  exp10f: - value with result 1.0000001 = 0x3F800001: 2.5885969e-8 = 0x32DE5BD9, exponent FLOAT_EXP1-26
	//  exp10f: - value with result 1.0000000 = 0x3F800000: 2.5885967e-8 = 0x32DE5BD8, exponent FLOAT_EXP1-26
	//  exp2f: - value with result 1.0000001 = 0x3F800001: 8.5991324e-8 = 0x33B8AA3B, exponent FLOAT_EXP1-24
	//  exp2f: - value with result 1.0000000 = 0x3F800000: 8.5991317e-8 = 0x33B8AA3A, exponent FLOAT_EXP1-24
	//  expf: - value with result 1.0000001 = 0x3F800001: 5.9604652e-8 = 0x33800001, exponent FLOAT_EXP1-24
	//  expf: - value with result 1.0000000 = 0x3F800000: 5.9604641e-8 = 0x337FFFFF, exponent FLOAT_EXP1-25
#if EXPF_MODE == 3 // exp10f
	if (exp <= -26)
		{ if ((exp < -26) || ((xn << 1) < (0x32DE5BD9<<1))) return Test_P1F(); }
#else
	if (exp <= -25) return Test_P1F();
#endif

	// get sign
	int sign = Test_GetSignF(x);

	// normalize mantissa - shift to max position and restore hidden bit '1'
	// - mantissa will be 0x80000000..0xFFFFFF00
	xn = (xn << 8) | B31;

#if EXPF_MODE == 3 // exp10f
	// multiply mantissa by ln(10) << 30 (= 2472381918 round up) = mantissa/4
	xn = MyUMulHigh(xn, 2472381918);
	exp += 2;
	if (xn < 0x80000000) { xn <<= 1; exp--; }
#endif

	// shift mantissa right by exponent bits
	// - get x as signed fixed number in format 8.24
	// expf: - mantissa will be 0x00000001..0x58B90B80 or 0x57562780
	// exp2f: - mantissa will be 0x00000001..0x7FFFFF80
	xn >>= 31 - exp - 24; // exp is in range -24..+6, shift is in range 1..31

#if EXPF_MODE == 2 // exp2f
	// get integer part of exponent
	// - exponent will be 0..+127
	exp = xn >> 24;

	// convert mantissa to range 0..1.0 ... number will be in 0.32 format
	u32 xx = xn << 8;
#else // expf
	// convert number to 2-base and get integer part of exponent: multiply by 1/ln(2) and shift >> 24, round to zero
	//   1/ln(2) << 31 = 3098164010 round up
	// - exponent will be 0..+127
	exp = MyUMulHigh(xn, 3098164010) >> (24+31 - 32); // shift = 23

	// convert exponent back to e-base: multiply by ln(2)
	//   ln(2) << 32 = 2977044472 round up
	// Later we will discard the upper 8 bits of the result,
	// so we will not need the HIGH result of the multiplication.
	u32 d = exp * 2977044472;

	// subtract exponent correction from the number
	xn = (xn << 8) - d;

	// convert to range 0..1.0: multiply by 1/ln(2) ... number will be in 0.32 format
	//   1/ln(2) << 31 = 3098164010 round up
	u32 xx = MyUMulHigh(xn, 3098164010) << 1;
#endif

	// coeff[9]
	u32 acc = 0x000000DD;	// coeff[9] = 221 = 1.02897824066917e-007 << 31
	acc = MyUMulHigh(acc, xx);

	// number is positive
	if (!sign)
	{
		acc = 0x00000B3C + acc;	// coeff[8] = 2876 = 1.3392906481613e-006 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x00007FF1 + acc; // coeff[7] = 32753 = 1.52517549025788e-005 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x00050C03 + acc; // coeff[6] = 330755 = 0.000154019750482348 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x002BB101 + acc; // coeff[5] = 2863361 = 0.00133335616465047 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x013B2AC3 + acc; // coeff[4] = 20654787 = 0.00961813466710737 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x071AC236 + acc; // coeff[3] = 119194166 = 0.0555041086190613 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x1EBFBDFE + acc; // coeff[2] = 515882494 = 0.240226506255567 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x58B90BFC + acc; // coeff[1] = 1488522236 = 0.693147180601954 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x80000000 + acc; // coeff[0] = 2147483648 = 1.0f << 31

		// - result mantissa is acc = 0x80000000 .. 0xFFFFFFFF = 1.0 .. 1.9999
	}

	// number is negative
	else
	{
		// negate exponent
		exp = -exp;

		acc = 0x00000B3C - acc; // coeff[8] = 2876 = 1.3392906481613e-006 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x00007FF1 - acc; // coeff[7] = 32753 = 1.52517549025788e-005 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x00050C03 - acc; // coeff[6] = 330755 = 0.000154019750482348 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x002BB101 - acc; // coeff[5] = 2863361 = 0.00133335616465047 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x013B2AC3 - acc; // coeff[4] = 20654787 = 0.00961813466710737 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x071AC236 - acc; // coeff[3] = 119194166 = 0.0555041086190613 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x1EBFBDFE - acc; // coeff[2] = 515882494 = 0.240226506255567 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x58B90BFC - acc; // coeff[1] = 1488522236 = 0.693147180601954 << 31
		acc = MyUMulHigh(acc, xx);

		acc = 0x80000000 - acc; // coeff[0] = 2147483648 = 1.0f << 31

		// mantissa correction
		// - result mantissa is acc = 0x40000000 .. 0x7FFFFFFF = 0.5 .. 0.9999
		acc <<= 1;
		exp--;
	}

	// Rounding (do not do exponent correction - could overflow to inf)
	acc += 0x80;
	if (acc < 0x80000000) acc = 0xFFFFFFFF;

	// Normalize mantissa
	acc <<= 1;
	acc >>= 9;

	// return number
	return Test_u32float(acc | ((exp + FLOAT_EXP1) << 23));
}

#endif // USE_RISCV_EXPF == 1

// ----------------------------------------------------------------------------
//                      Natural exponent - Taylor serie
// ----------------------------------------------------------------------------
//  Taylor serie exp(x) = 1 + x/1! + x^2/2! + x^3/3! + x^4/4! + x^5/5! + ...

#if USE_RISCV_EXPF == 2		// RISC-V expf(), exp2f() and exp10f() function: 1=Chebyshev (recommended), 2=Taylor

// expf: 697ns avg=0.153 max=2
// exp2f: 630ns avg=0.257 max=1
// exp10f: 775ns avg=0.321 max=2
float expf_fast(float x)
{
	// get number as integer
	u32 xn = Test_floatu32(x);

	// max. value (x > 0), return +inf
	if ((s32)xn >= 0)
	{
#if EXPF_MODE == 3 // exp10f
		// exp10f max. value: 38.531837 = 0x421A209A, infinity: 38.531841 = 0x421A209B
		if (xn > 0x421A209A) return Test_PInfF();
#elif EXPF_MODE == 2 // exp2f
		// exp2f max. value: 127.99999 = 0x42FFFFFF, infinity: 128.00000 = 0x43000000
		if (xn > 0x42FFFFFF) return Test_PInfF();
#else // expf
		// expf max. value: 88.722832 = 0x42B17217, infinity: 88.722839 = 0x42B17218
		if (xn > 0x42B17217) return Test_PInfF();
#endif
	}

	// min. value (x < 0), return +0.0
	else
	{
#if EXPF_MODE == 3 // exp10f
		// exp10f min. value: -37.929779 = 0xC217B818, zero: -37.929783 = 0xC217B819
		if (xn > 0xC217B818) return Test_PZeroF();
#elif EXPF_MODE == 2 // exp2f
		// exp2f min. value: -126.00000 = 0xC2FC0000, zero: -126.00001 = 0xC2FC0001
		if (xn > 0xC2FC0000) return Test_PZeroF();
#else // expf
		// expf min. value: -87.336540 = 0xC2AEAC4F, zero: -87.336548 = 0xC2AEAC50
		if (xn > 0xC2AEAC4F) return Test_PZeroF();
#endif
	}

	// get signed exponent
	int exp = Test_GetExpF(x) - FLOAT_EXP1;

	// check minimal exponent - return 1.0
	//  exp10f: - value with result 1.0000001 = 0x3F800001: 2.5885969e-8 = 0x32DE5BD9, exponent FLOAT_EXP1-26
	//  exp10f: - value with result 1.0000000 = 0x3F800000: 2.5885967e-8 = 0x32DE5BD8, exponent FLOAT_EXP1-26
	//  exp2f: - value with result 1.0000001 = 0x3F800001: 8.5991324e-8 = 0x33B8AA3B, exponent FLOAT_EXP1-24
	//  exp2f: - value with result 1.0000000 = 0x3F800000: 8.5991317e-8 = 0x33B8AA3A, exponent FLOAT_EXP1-24
	//  expf: - value with result 1.0000001 = 0x3F800001: 5.9604652e-8 = 0x33800001, exponent FLOAT_EXP1-24
	//  expf: - value with result 1.0000000 = 0x3F800000: 5.9604641e-8 = 0x337FFFFF, exponent FLOAT_EXP1-25
#if EXPF_MODE == 3 // exp10f
	if (exp <= -26)
	{
		if ((exp < -26) || ((xn << 1) < (0x32DE5BD9<<1))) return Test_P1F();
	}
#else
	if (exp <= -25) return Test_P1F();
#endif

	// get sign
	int sign = Test_GetSignF(x);

	// normalize mantissa - shift to max position and restore hidden bit '1'
	// - mantissa will be 0x80000000..0xFFFFFF00
	xn = (xn << 8) | B31;

#if EXPF_MODE == 3 // exp10f
	// multiply mantissa by ln(10) << 30 (= 2472381918 round up) = mantissa/4
	xn = MyUMulHigh(xn, 2472381918);
	exp += 2;
	if (xn < 0x80000000) { xn <<= 1; exp--; }
#endif

	// shift mantissa right by exponent bits
	// - get x as signed fixed number in format 8.24
	// expf: - mantissa will be 0x00000001..0x58B90B80 or 0x57562780
	// exp2f: - mantissa will be 0x00000001..0x7FFFFF80
	xn >>= 31 - exp - 24; // exp is in range -24..+6, shift is in range 1..31

	// signed mantissa
	// - mantissa will be -0x57562780..0x58B90B80
	s32 m = xn;
	if (sign) m = -m;

#if EXPF_MODE == 2 // exp2f
	// get integer part of exponent
	// - exponent will be -126..+127
	exp = m >> 24;

	// convert exponent back to base
	s32 d = exp << 24;

	// subtract exponent correction from the number
	// - number will now be in range 0x00000001..+0x00B17217 (=0.00000006..0.69314712)
	xn = m - d;

	// convert to range ln(2)
	//   ln(2) << 32 = 2977044472 round up
	xn = MyUMulHigh(xn, 2977044472);
#else // expf
	// convert number to 2-base and get integer part of exponent: multiply by 1/ln(2) and shift >> 24, round to zero
	//   1/ln(2) << 30 = 1549082005 round up
	// - exponent will be -126..+127
	exp = MySMulHigh(m, 1549082005) >> (24+30 - 32); // shift = 22

	// convert exponent back to e-base: multiply by ln(2) and convert to 8.24 format, round to zero
	//   ln(2) << 31 = 1488522236 round up
	// - exponent correction will be -0x002BAB13E608..+0x002C03CCF204 >> 7 -> -0x575627CC..+0x580799E4
	//  Later we will discard the upper 8 bits of the result,
	//  so we will not need the HIGH result of the multiplication.
	s32 d = (s32)(exp * 1488522236) >> 7;	// shift 32 - 24 = 7

	// subtract exponent correction from the number
	// - number will now be in range 0x00000001..+0x00B17217 (=0.00000006..0.69314712)
	xn = m - d;
#endif

	// use mantissa in format 0.32
	xn <<= 8;

	// inconsistency correction (due to rounding)
	if (xn > 0xB1721700) xn = 0xB1721700;

	// prepare member
	u32 mem = xn;

	// prepare accumulator (without first member '1')
	u32 acc = mem;

	// Taylor serie exp(x) = 1 + x/1! + x^2/2! + x^3/3! + x^4/4! + x^5/5! + ...
	mem = MyUMulHigh(mem, xn); // x^2
	// if (mem >= 2)
	{
		mem >>= 1; // /2
		acc += mem;

		mem = MyUMulHigh(mem, xn); // x^3
		// if (mem >= 3)
		{
			mem = MyUMulHigh(mem, 1431655766); // /3
			acc += mem;

			mem = MyUMulHigh(mem, xn); // x^4
			// if (mem >= 4)
			{
				mem >>= 2; // /4
				acc += mem;

				mem = MyUMulHigh(mem, xn); // x^5
				if (mem >= 5)
				{
					mem = MyUMulHigh(mem, 858993460); // /5
					acc += mem;

					mem = MyUMulHigh(mem, xn); // x^6
					if (mem >= 6)
					{
						mem = MyUMulHigh(mem, 715827883); // /6
						acc += mem;

						mem = MyUMulHigh(mem, xn); // x^7
						// if (mem >= 7)
						{
							mem = MyUMulHigh(mem, 613566757); // /7
							acc += mem;

							mem = MyUMulHigh(mem, xn); // x^8
							// if (mem >= 8)
							{
								mem >>= 3; // /8
								acc += mem;

								mem = MyUMulHigh(mem, xn); // x^9
								if (mem >= 9)
								{
									mem = MyUMulHigh(mem, 477218589); // /9
									acc += mem;

									mem = MyUMulHigh(mem, xn); // x^10
									if (mem >= 10)
									{
										mem = MyUMulHigh(mem, 429496730); // /10
										acc += mem;

										mem = MyUMulHigh(mem, xn); // x^11
										// if (mem >= 11)
										{
											mem = MyUMulHigh(mem, 390451573); // /11
											acc += mem;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Rounding
	acc += 0x100;

	// acc before rounding is in range 0x00000100..0xFFFFFE0B, overflow will never occur, but beware
	//if (acc < 0x100) acc = 0xFFFFFFFF;

	// Normalize mantissa
	acc >>= 8+1;

	// return number
	return Test_u32float(acc | ((exp + FLOAT_EXP1) << 23));
}

#endif // USE_RISCV_EXPF == 2

// ----------------------------------------------------------------------------
//                   Natural logarithm - Chebyshev approximation
// ----------------------------------------------------------------------------

// 32-bit Chebyshev coefficients of Log() - see calculations in real_c.h
//const REALEXT REALNAME(ConstChebLn)[CHEB_LN] = {	// CHEB_LN = 13
//	 { { 0xcf991f66 }, 0x3ffffffd},	// 0  (0.405465108109638)
//	 { { 0xaaaaaaa9 }, 0x3ffffffd},	// 1  (0.333333333139308)
//	 { { 0xe38e38df }, 0xbffffffa},	// 2  (-0.0555555554892635)
//	 { { 0xca458e42 }, 0x3ffffff8},	// 3  (0.0123456849323702)
//	 { { 0xca458fcc }, 0xbffffff6},	// 4  (-0.00308642159143346)
//	 { { 0xd7be7b58 }, 0x3ffffff4},	// 5  (0.000822998312287382)
//	 { { 0xefb6ca45 }, 0xbffffff2},	// 6  (-0.000228609107864486)
//	 { { 0x8952049c }, 0x3ffffff1},	// 7  (6.54794611136822e-005)
//	 { { 0xa03b65c9 }, 0xbfffffef},	// 8  (-1.91011454901968e-005)
//	 { { 0xb4d13635 }, 0x3fffffed},	// 9  (5.38877348255085e-006)
//	 { { 0xd8a01e11 }, 0xbfffffeb},	// 10  (-1.61398543996327e-006)
//	 { { 0xbd6ca7ba }, 0x3fffffea},	// 11  (7.05661007405212e-007)
//	 { { 0xe877d73d }, 0xbfffffe8},	// 12  (-2.16502815508779e-007)
//};

//	coeff[ 0] =  1741459379 = 0.405465108109638 << 31
//	coeff[ 1] =  1431655765 = 0.333333333139308 << 31
//	coeff[ 2] = -238609294 = -0.0555555554892635 << 31
//	coeff[ 3] =  53024313 = 0.0123456849323702 << 31
//	coeff[ 4] = -13256080 = -0.00308642159143346 << 31
//	coeff[ 5] =  3534751 = 0.000822998312287382 << 31
//	coeff[ 6] = -981869 = -0.000228609107864486 << 31
//	coeff[ 7] =  281232 = 6.54794611136822e-005 << 31
//	coeff[ 8] = -82039 = -1.91011454901968e-005 << 31
//	coeff[ 9] =  23145 = 5.38877348255085e-006 << 31
//	coeff[10] = -6932 = -1.61398543996327e-006 << 31
//	coeff[11] =  3031 = 7.05661007405212e-007 << 31
//	coeff[12] = -930 = -2.16502815508779e-007 << 31

#if USE_RISCV_LOGF == 1		// RISC-V logf(), log2f() and log10f() function: 1=Chebyshev (recommended), 2=Mercator

// logf: 766ns avg=0.045 max=5
// log2f: 767ns avg=0.006 max=5
// log10f: 770ns avg=0.287 max=6
float logf_fast(float x)
{
	// get number as integer
	u32 xn = Test_floatu32(x);

	// result is 0 if number is 1
	if (xn == 0x3F800000) return 0;

	// get unsigned exponent
	int exp = (u32)(xn << 1) >> 24;

	// zero returns -inf (both positive and negative zero)
	if (exp == 0) return Test_MInfF();

	// negative number returns +inf
	if ((s32)xn < 0) return Test_PInfF();

	// positive infinity returns +inf
	if (Test_IsOverF(x)) return x;

	// get signed exponent
	exp -= FLOAT_EXP1;

	// get mantissa in 1.31 format signed, convert from range 1.0000..1.9999 to range -1.0000..0.9999
	//  s32 m = ((xn<<8)|B31);	// convert mantissa to 1.31 format, range 1.0000..1.9999
	//  m -= B31;			// subtract 1, convert to range 0.0000..0.9999
	//  m <<= 1;			// mantissa * 2, convert to range 0.0000..1.9999
	//  m -= B31;			// subtract 1, convert to range -1.0000..0.9999
	s32 m = (xn << 9) - B31;

	// Chebyshev approximation
	s32 acc;
	acc = -465;		// coeff[12] = -465 = -2.16502815508779e-007 << 31
	acc = MySMulHigh(acc, m) << 1;

	acc = 1515 + acc;	// coeff[11] = 1515 = 7.05661007405212e-007 << 31
	acc = MySMulHigh(acc, m) << 1;

	acc = -3466 + acc;	// coeff[10] = -3466 = -1.61398543996327e-006 << 31
	acc = MySMulHigh(acc, m) << 1;

	acc = 11572 + acc;	// coeff[ 9] =  11572 = 5.38877348255085e-006 << 31
	acc = MySMulHigh(acc, m) << 1;

	acc = -41019 + acc;	// coeff[ 8] = -41019 = -1.91011454901968e-005 << 31
	acc = MySMulHigh(acc, m) << 1;

	acc = 140616 + acc;	// coeff[ 7] =  140616 = 6.54794611136822e-005 << 31
	acc = MySMulHigh(acc, m) << 1;

	acc = -490934 + acc;	// coeff[ 6] = -490934 = -0.000228609107864486 << 31
	acc = MySMulHigh(acc, m) << 1;

	acc = 1767375 + acc;	// coeff[ 5] =  1767375 = 0.000822998312287382 << 31
	acc = MySMulHigh(acc, m) << 1;

	acc = -6628040 + acc;	// coeff[ 4] = -6628040 = -0.00308642159143346 << 31
	acc = MySMulHigh(acc, m) << 1;

	acc = 26512157 + acc;	// coeff[ 3] = 26512157 = 0.0123456849323702 << 31
	acc = MySMulHigh(acc, m) << 1;

	acc = -119304647 + acc;	// coeff[ 2] = -119304647 = -0.0555555554892635 << 31
	acc = MySMulHigh(acc, m) << 1;

	acc = 715827882 + acc;	// coeff[ 1] = 715827882 = 0.333333333139308 << 31
	acc = MySMulHigh(acc, m) << 1;

	acc = 870729689 + acc;	// coeff[ 0] = 870729689 = 0.405465108109638 << 31

// result mantissa is in 1.31 format, value 0..0.6931 (log(2)) = 0x00000001..0x58B90BFC

// log10f()
#if LOGF_MODE == 3	// logf mode: 1=logf, 2=log2f, 3=log10f

	// round mantissa up
	acc += 0x49;	// = 0x20 * log(10) = 0x49 round down

	// multiply 1.31 mantissa * 1/log(10) and convert to format 7.25
	// 1/log(10) << 26 = 29145009 round down
	acc = MyUMulHigh(acc, 29145009);

	// multiply exponent * log(10) and convert to format 7.25
	// log(2)/log(10) << 25 = 10100890 round down
	acc += exp*10100890;

	// prepare result exponent base
	exp = FLOAT_EXP1 - 2;

// log2f()
#elif LOGF_MODE == 2	// logf mode: 1=logf, 2=log2f, 3=log10f

	// round mantissa up
	acc += 0x2C;	// = 0x40 * log(2) = 0x2C round down

	// multiply 1.31 mantissa * 1/log(2) and convert to format 8.24
	// 1/log(2) << 25 = 48408813 round up
	acc = MyUMulHigh(acc, 48408813);

	// add exponent in format 8.24
	acc += (exp << 24);

	// prepare result exponent base
	exp = FLOAT_EXP1 - 1;

// logf()
#else

	// round mantissa up
	acc += 0x40;

	// convert mantissa from format 1.31 to format 8.24
	acc >>= 7;

	// multiply exponent * log(2) and convert to format 8.24
	// log(2) << 24 = 11629080 round up
	acc += exp*11629080;

	// prepare result exponent base
	exp = FLOAT_EXP1 - 1;

#endif

	// zero result
	if (acc == 0) return 0;

	// negative mantissa
	u32 sign = 0;
	if (acc < 0)
	{
		acc = -acc;
		sign = B31;
	}

	// get number of shifts
	int d = 8 - clz(acc);

	// prepare result exponent
	exp += d;

	// normalize mantissa
	if (d <= 0)
		// shift up
		acc <<= -d;
	else
	{
		// shift down with rounding
		acc >>= d-1;
		acc++;
		acc >>= 1;

		// carry on rounding
		if ((acc >> 24) != 0)
		{
			acc >>= 1;
			exp++;
		}
	}

	// clear hidden bit '1'
	acc &= FLOAT_MANTMASK;

	// compose result
	return Test_u32float(acc | (exp << 23) | sign);
}

#endif // USE_RISCV_LOGF == 1

// ----------------------------------------------------------------------------
//                      Natural logarithm - Mercator serie
// ----------------------------------------------------------------------------
// Mercator serie D = x-1, ln(x) = D - D^2/2 + D^3/3 - D^4/4 + ... ... for 0 < x <= 2

#if USE_RISCV_LOGF == 2		// RISC-V logf(), log2f() and log10f() function: 1=Chebyshev (recommended), 2=Mercator

// logf: 1020ns avg=0.034 max=6
// log2f: 1020ns avg=0.011 max=6
// log10f: 1040ns avg=0.288 max=4
float logf_fast(float x)
{
	// get number as integer
	u32 xn = Test_floatu32(x);

	// result is 0 if number is 1
	if (xn == 0x3F800000) return 0;

	// get unsigned exponent
	int exp = (u32)(xn << 1) >> 24;

	// zero returns -inf (both positive and negative zero)
	if (exp == 0) return Test_MInfF();

	// negative number returns +inf
	if ((s32)xn < 0) return Test_PInfF();

	// positive infinity returns +inf
	if (Test_IsOverF(x)) return x;

	// get signed exponent, to have mantissa in range 0.5..0.9999
	exp -= FLOAT_EXP1 - 1;

	// get signed mantissa in 1.31 format, range 0.5..0.9999 = 0x40000000..0x7FFFFF80
	u32 m = ((xn << 9) >> 2) | B30;		// set hidden implied bit '1' on position 0.5

	// Prepare number D = x - 1 (input interval is 0.5..0.9999)
	// For simplicity, we will work with the negative value of D, negating the result.
	//   D = 1 - x,  -ln(x) = + D + D^2/2 + D^3/3 + D^4/4 +
	u32 d = B31 - m;

	// result accumulator = D
	s32 acc = d;

	// member = D
	m = d;

	// pre-set D * 2, because mul-high uses >> 32 and we need >> 31
	d <<= 1;

	// D^2/2
	m = MyUMulHigh(m, d);
	acc += m >> 1;

	// D^3/3
	m = MyUMulHigh(m, d);
//	if (m >= 3)
	{
	acc += MyUMulHigh(m, 1431655766);	// 1/3 << 32, 1431655766

	// D^4/4
	m = MyUMulHigh(m, d);
	acc += m >> 2;

	// D^5/5
	m = MyUMulHigh(m, d);
//	if (m >= 5)
	{
	acc += MyUMulHigh(m, 858993460);	// 1/5 << 32, 858993460

	// D^6/6
	m = MyUMulHigh(m, d);
//	if (m >= 6)
	{
	acc += MyUMulHigh(m, 715827883);	// 1/6 << 32, 715827883

	// D^7/7
	m = MyUMulHigh(m, d);
//	if (m >= 7)
	{
	acc += MyUMulHigh(m, 613566757);	// 1/7 << 32, 613566757

	// D^8/8
	m = MyUMulHigh(m, d);
	acc += m >> 3;

	// D^9/9
	m = MyUMulHigh(m, d);
//	if (m >= 9)
	{
	acc += MyUMulHigh(m, 477218589);	// 1/9 << 32, 477218589

	// D^10/10
	m = MyUMulHigh(m, d);
//	if (m >= 10)
	{
	acc += MyUMulHigh(m, 429496730);	// 1/10 << 32, 429496730

	// D^11/11
	m = MyUMulHigh(m, d);
//	if (m >= 11)
	{
	acc += MyUMulHigh(m, 390451573);	// 1/11 << 32, 390451573

	// D^12/12
	m = MyUMulHigh(m, d);
	if (m >= 12)
	{
	acc += MyUMulHigh(m, 357913942);	// 1/12 << 32, 357913942

	// D^13/13
	m = MyUMulHigh(m, d);
//	if (m >= 13)
	{
	acc += MyUMulHigh(m, 330382100);	// 1/13 << 32, 330382100

	// D^14/14
	m = MyUMulHigh(m, d);
//	if (m >= 14)
	{
	acc += MyUMulHigh(m, 306783379);	// 1/14 << 32, 306783379

	// D^15/15
	m = MyUMulHigh(m, d);
//	if (m >= 15)
	{
	acc += MyUMulHigh(m, 286331153);	// 1/15 << 32, 286331153

	// D^16/16
	m = MyUMulHigh(m, d);
	if (m >= 16)
	{
	acc += m >> 4;

	// D^17/17
	m = MyUMulHigh(m, d);
//	if (m >= 17)
	{
	acc += MyUMulHigh(m, 252645135);	// 1/17 << 32, 252645135

	// D^18/18
	m = MyUMulHigh(m, d);
	if (m >= 18)
	{
	acc += MyUMulHigh(m, 238609295);	// 1/18 << 32, 238609295

	// D^19/19
	m = MyUMulHigh(m, d);
//	if (m >= 19)
	{
	acc += MyUMulHigh(m, 226050911);	// 1/19 << 32, 226050911

	// D^20/20
	m = MyUMulHigh(m, d);
//	if (m >= 20)
	{
	acc += MyUMulHigh(m, 214748365);	// 1/20 << 32, 214748365
	}}}}}}}}}}}}}}}}

// result mantissa is in 1.31 format, value 0..0.6931 (log(2)) = 0x00000001..0x58B90BFC

// log10f()
#if LOGF_MODE == 3	// logf mode: 1=logf, 2=log2f, 3=log10f

	// round mantissa up
	acc += 0x49;	// = 0x20 * log(10) = 0x49 round down

	// multiply 1.31 mantissa * 1/log(10) and convert to format 7.25
	// 1/log(10) << 26 = 29145009 round down
	acc = MyUMulHigh(acc, 29145009);

	// negate result
	acc = -acc;

	// multiply exponent * log(10) and convert to format 7.25
	// log(2)/log(10) << 25 = 10100890 round down
	acc += exp*10100890;

	// prepare result exponent base
	exp = FLOAT_EXP1 - 2;

// log2f()
#elif LOGF_MODE == 2	// logf mode: 1=logf, 2=log2f, 3=log10f

	// round mantissa up
	acc += 0x2C;	// = 0x40 * log(2) = 0x2C round down

	// multiply 1.31 mantissa * 1/log(2) and convert to format 8.24
	// 1/log(2) << 25 = 48408813 round up
	acc = MyUMulHigh(acc, 48408813);

	// negate result
	acc = -acc;

	// add exponent in format 8.24
	acc += (exp << 24);

	// prepare result exponent base
	exp = FLOAT_EXP1 - 1;

// logf()
#else

	// round mantissa up
	acc += 0x40;

	// convert mantissa from format 1.31 to format 8.24
	acc >>= 7;

	// negate result
	acc = -acc;

	// multiply exponent * log(2) and convert to format 8.24
	// log(2) << 24 = 11629080 round up
	acc += exp*11629080;

	// prepare result exponent base
	exp = FLOAT_EXP1 - 1;

#endif

	// zero result
	if (acc == 0) return 0;

	// negative mantissa
	u32 sign = 0;
	if (acc < 0)
	{
		acc = -acc;
		sign = B31;
	}

	// get number of shifts
	int shift = 8 - clz(acc);

	// prepare result exponent
	exp += shift;

	// normalize mantissa
	if (shift <= 0)
		// shift up
		acc <<= -shift;
	else
	{
		// shift down with rounding
		acc >>= shift-1;
		acc++;
		acc >>= 1;

		// carry on rounding
		if ((acc >> 24) != 0)
		{
			acc >>= 1;
			exp++;
		}
	}

	// clear hidden bit '1'
	acc &= FLOAT_MANTMASK;

	// compose result
	return Test_u32float(acc | (exp << 23) | sign);
}

#endif // USE_RISCV_LOGF == 2

// ----------------------------------------------------------------------------
//                       sinf(x) - Chebyshev approximation
// ----------------------------------------------------------------------------

#if USE_RISCV_SINF == 1		// RISC-V sinf(), cosf(), sincosf(), tanf(), cotanf() functions: 1=Chebyshev, 2=Taylor (recommended)

// Chebyshev coefficients of sin()
//const REALEXT REALNAME(ConstChebSin)[CHEB_SIN] = {	// CHEB_SIN = 13
//	 { { 0xb504f334 }, 0x3ffffffe},	// 0  (0.707106781192124)
//	 { { 0x8e2c18d7 }, 0x3ffffffe},	// 1  (0.555360367288813)
//	 { { 0xdf52db04 }, 0xbffffffc},	// 2  (-0.218089506262913)
//	 { { 0xe9dd2e0e }, 0xbffffffa},	// 3  (-0.0570956992160063)
//	 { { 0xb7ad24d9 }, 0x3ffffff8},	// 4  (0.0112107143249887)
//	 { { 0xe6d08314 }, 0x3ffffff5},	// 5  (0.00176097488474625)
//	 { { 0xf1b5585d }, 0xbffffff2},	// 6  (-0.000230511073652906)
//	 { { 0xd8f4f31f }, 0xbfffffef},	// 7  (-2.58632701317651e-005)
//	 { { 0xaa65c4ba }, 0x3fffffec},	// 8  (2.53912110714793e-006)
//	 { { 0xede60cfa }, 0x3fffffe8},	// 9  (2.21560369717544e-007)
//	 { { 0x957adcb7 }, 0xbfffffe5},	// 10  (-1.74017541973437e-008)
//	 { { 0xa8983627 }, 0xbfffffe1},	// 11  (-1.22668699113412e-009)
//	 { { 0xb0e079fd }, 0x3fffffdd},	// 12  (8.04342841517845e-011)
//};

// Chebyshev approximation of sin() mantissa
//  m = -1..+1 in format 2.30 (= angle 0..pi/2)
float _sinf_cheb(u32 m, s32 sign)
{
	s32 acc;

	acc = -19;		// coeff[10] =  -19 = -1.74017541973437e-008 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 238 + acc;	// coeff[ 9] =  238 = 2.21560369717544e-007 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 2726 + acc;	// coeff[ 8] = 2726 = 2.53912110714793e-006 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = -27770 + acc;	// coeff[ 7] = -27770  = -2.58632701317651e-005 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = -247509 + acc;	// coeff[ 6] = -247509 = -0.000230511073652906 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 1890832 + acc;	// coeff[ 5] = 1890832  = 0.00176097488474625 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 12037413 + acc;	// coeff[ 4] = 12037413 = 0.0112107143249887 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = -61306040 + acc;	// coeff[ 3] = -61306040 = -0.0570956992160063 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = -234171824+7 + acc; // coeff[ 2] = -234171824 = -0.218089506262913 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 596313654+5 + acc; // coeff[ 1] = 596313654 = 0.555360367288813 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 759250125 + 0x20 + acc;	// coeff[ 0] = 759250125 = 0.707106781192124 << 30 + rounding correction

// result mantissa is in 2.30 format, value 0..+1

	// convert mantissa from format 2.30 to format 8.24
	acc >>= 6;

	// zero result
	if (acc <= 0) return 0;

	// limit to max. value 1.0
	if (acc > 0x01000000) acc = 0x01000000;

	// get number of shifts
	int d = 8 - clz(acc);

	// prepare result exponent
	int exp = d + FLOAT_EXP1 - 1;

	// normalize mantissa
	if (d <= 0)
		// shift up
		acc <<= -d;
	else
	{
		// shift down with rounding
		acc >>= d-1;
		acc++;
		acc >>= 1;

		// carry on rounding
		if ((acc >> 24) != 0)
		{
			acc >>= 1;
			exp++;
		}
	}

	// clear hidden bit '1'
	acc &= FLOAT_MANTMASK;

	// compose result
	return Test_u32float(acc | (exp << 23) | (sign << 31));
}

// sine-cosine in radians - Chebyshev approximation
void sincosf_fast(float x, float* psin, float* pcos)
{
	// get number as integer
	u32 xn = Test_floatu32(x);

	// get unsigned exponent
	int exp = (xn << 1) >> 24;

	// normalize big angle
	if (exp >= FLOAT_EXP1+7) // normalize angles >= 128.0 (= 20*(2*pi))
	{
		// normalize using remainderf, modulo 2*pi (remainderf returns angle in range -pi .. +pi)
		x = remainderf(x, Test_u32float(0x40C90FDB));
		xn = Test_floatu32(x);
	}

	// get exponent with sign
	// - signed exponent here is max. +6
	exp = xn >> 23; // get exponent with sign

	// unpack mantissa to format 9.23
	s32 m = ((xn << 9) >> 9) | B23;

	// negate mantissa
	if ((s32)xn < 0)
	{
		// clear sign bit
		exp &= 0xff;

		// negate mantissa
		m = -m;
	}

	// convert mantissa to signed format 8.24 (rounding down)
	int s = exp - FLOAT_EXP1 + (24-23); // exps <= +6, number of shifts <= +7

	// convert to signed fixed point 8.24
	// right shift
	if (s <= 0)
	{
		s = -s;
		if (s >= 31) s = 31;
		m >>= s;
	}

	// left shift
	else
		m <<= s;

	// 2*pi in 8.24 fixed point
	u32 pi = 0x06487ED5;	// = 2*pi << 24

	// limit max. angle to modulo 2*PI
	// - mantissa will be in range 0 .. 0x06487ED4 = 105414356
	do m -= pi; while (m >= 0); // limit positive angle
	do m += pi; while (m < 0); // result angle will be in range 0 (include) .. 2*PI (exclude)

	// convert angle to unsigned format 3.29
	// - mantissa will be in range 0 .. 0xC90FDA80 = 3373259392
	xn = m << 5;

// ---- mantissa here is in unsigned fixed point format 3.29, range 0..0xC90FDA80 = 3373259392 (= 0..2*pi)

	s32 sin_sign = 0;
	s32 cos_sign = 0;

	// convert to range 0..pi
	// - mantissa will be in range = 0 .. 0x6487ED4F (= 0..pi)
	pi <<= 4; // pi = 0x6487ED50 = 1686629712
	if (xn >= pi) // x >= PI ?
	{
		xn -= pi;
		sin_sign = ~sin_sign;
		cos_sign = ~cos_sign;
	}

	// convert to range 0..pi/2
	//  mantissa will be in range = 0 .. 0x3243F6A8 = 843314856 (= 0..pi/2)
	u32 pi2 = pi >> 1; // pi/2 = 0x3243F6A8 = 843314856
	if (xn >= pi2)
	{
		xn = pi - xn;
		cos_sign = ~cos_sign;
	}

	// convert range 0..pi/2 in unsigned fixed point format 3.29 to range -1..+1 in signed fixed point format 2.30
	//  k = (0x80000000 / 0x3243F6A8 << 32) >> 2 = (2147483648 / 843314856 << 32) >> 2 = 2734261104
	m = MyUMulHigh(xn, 2734261104+1) << 2; // max. value 2147483648 = 0x80000000
	m -= 0x40000000; // subtract 1 to get range -1..+1 in signed format 2.30, range -0x40000000..+0x40000000

	// Chebyshev approximation
	*psin = _sinf_cheb(m, sin_sign);
	*pcos = _sinf_cheb(-m, cos_sign);
}

// sine-cosine in degrees - Chebyshev approximation
void sincosf_deg_fast(float x, float* psin, float* pcos)
{
	// get number as integer
	u32 xn = Test_floatu32(x);

	// normalize big angle >= 360
	if ((xn << 1) >= (0x43B40000 << 1)) // absolute value of angle >= 360 deg
	{
		// normalize using remainderf, modulo 360 (remainderf returns angle in range -180 .. +180)
		x = remainderf(x, Test_u32float(0x43B40000)); // 360: exps=+8
		xn = Test_floatu32(x);
	}

	// absolute value of the angle
	// - angle will be in range 0..360
	s32 sin_sign = 0;
	if ((s32)xn < 0)
	{
		sin_sign = ~sin_sign;
		xn &= ~B31;
	}	

	// get unsigned exponent
	// - signed exponent here is max. +8
	int exp = xn >> 23; // get exponent

	// unpack mantissa to unsigned format 9.23, set hidden bit '1'
	xn = ((xn << 9) >> 9) | B23;

	// prepare number of shifts to format 9.23 (rounding down)
	int s = exp - FLOAT_EXP1 + (23-23); // exps <= +8, number of shifts <= +8

	// convert mantissa to unsigned fixed point format 9.23 (rounding down)
	// - mantissa will be in range = 0 .. 0xB4000000
	// right shift
	if (s <= 0)
	{
		s = -s;
		if (s >= 31) s = 31;
		xn >>= s;
	}

	// left shift (max. 8)
	else
		xn <<= s;

	// 180 in 9.23 fixed point
	u32 a = 0x5A000000;	// = 180 << 23

	// convert to range 0..180
	// - mantissa will be in range = 0 .. 0x5A000000 (= 0..180)
	s32 cos_sign = 0;
	if (xn >= a) // x >= 180 ?
	{
		xn -= a;
		sin_sign = ~sin_sign;
		cos_sign = ~cos_sign;
	}

	// convert to range 0..90
	//  mantissa will be in range = 0 .. 0x2D000000 = 754974720 (= 0..90)
	u32 a2 = a >> 1; // 90deg = 0x2D000000 = 754974720
	if (xn >= a2)
	{
		xn = a - xn;
		cos_sign = ~cos_sign;
	}

	// convert to format 7.25
	// mantissa = 0 .. 0xB4000000 = 3019898880
	xn <<= 2;

	// convert range 0..90 in unsigned fixed point format 7.25 to range -1..+1 in signed fixed point format 2.30
	//  k = (0x80000000 / 0xB4000000 << 32) = (2147483648 / 3019898880 << 32) = 3054198967 round up
	xn = MyUMulHigh(xn, 3054198967); // max. value 2147483648 = 0x80000000
	s32 m = xn - 0x40000000; // subtract 1 to get range -1..+1 in signed format 2.30, range -0x40000000..+0x40000000

	// Chebyshev approximation
	*psin = _sinf_cheb(m, sin_sign);
	*pcos = _sinf_cheb(-m, cos_sign);
}

float sinf_fast(float x)
{
	// small angle
	u32 xn = Test_floatu32(x);
	int exp = (xn >> 23) & 0xff;
	if (exp < FLOAT_EXP1-7) return x;

	float res_sin, res_cos;
	sincosf_fast(x, &res_sin, &res_cos);
	return res_sin;
}

float sinf_deg_fast(float x)
{
	float res_sin, res_cos;
	sincosf_deg_fast(x, &res_sin, &res_cos);
	return res_sin;
}

float cosf_fast(float x)
{
	float res_sin, res_cos;
	sincosf_fast(x, &res_sin, &res_cos);
	return res_cos;
}

float cosf_deg_fast(float x)
{
	float res_sin, res_cos;
	sincosf_deg_fast(x, &res_sin, &res_cos);
	return res_cos;
}

float tanf_fast(float x)
{
	// small angle
	u32 xn = Test_floatu32(x);
	int exp = (xn >> 23) & 0xff;
	if (exp < FLOAT_EXP1-8) return x;

	float res_sin, res_cos;
	sincosf_fast(x, &res_sin, &res_cos);
	return res_sin/res_cos;
}

float tanf_deg_fast(float x)
{
	float res_sin, res_cos;
	sincosf_deg_fast(x, &res_sin, &res_cos);
	return res_sin/res_cos;
}

float cotanf_fast(float x)
{
	float res_sin, res_cos;
	sincosf_fast(x, &res_sin, &res_cos);
	return res_cos/res_sin;
}

float cotanf_deg_fast(float x)
{
	float res_sin, res_cos;
	sincosf_deg_fast(x, &res_sin, &res_cos);
	return res_cos/res_sin;
}

#endif // USE_RISCV_SINF == 1

// ----------------------------------------------------------------------------
//                       sinf(x) - Taylor serie
// ----------------------------------------------------------------------------
// sine in radians - Taylor serie
//  sin(x) = x/1! - x^3/3! + x^5/5! - x^7/7! + x^9/9! - ...

#if USE_RISCV_SINF == 2		// RISC-V sinf(), cosf(), sincosf(), tanf(), cotanf() functions: 1=Chebyshev, 2=Taylor (recommended)

// Taylor serie of sin() mantissa
//  sin(x) = x/1! - x^3/3! + x^5/5! - x^7/7! + x^9/9! - ...
//  m = 0..0x6487ED50 = 1686629712 (= 0..pi/2) in unsigned fixed point format 2.30
float _sinf_taylor(u32 n, s32 sign)
{
	// prepare Taylor serie
	u32 n2 = MyUMulHigh(n, n) << 2;

	// = x^1/1!
	s32 acc = n; // acc = x^1/1!

	// - x^3/3!
	n = MyUMulHigh(n, n2); // mant * x^2
//	if (n >= 2*3/4)
	{
	n = MyUMulHigh(n, 0xAAAAAAAB); // 2^34/(2*3) = 2863311531 round up
	acc -= n;

	// + x^5/5!
	n = MyUMulHigh(n, n2); // mant * x^2
//	if (n >= 4*5/4)
	{
	n = MyUMulHigh(n, 0x33333334); // 2^34/(4*5) = 858993460 round up
	acc += n;

	// - x^7/7!
	n = MyUMulHigh(n, n2); // mant * x^2
//	if (n >= 6*7/4)
	{
	n = MyUMulHigh(n, 0x18618619); // 2^34/(6*7) = 409044505 round up
	acc -= n;

	// + x^9/9!
	n = MyUMulHigh(n, n2); // mant * x^2
//	if (n >= 8*9/4)
	{
	n = MyUMulHigh(n, 0x0E38E38F); // 2^34/(8*9) = 238609295 round up
	acc += n;

	// - x^11/11!
	n = MyUMulHigh(n, n2); // mant * x^2
//	if (n >= 10*11/4)
	{
	n = MyUMulHigh(n, 0x094F2096); // 2^34/(10*11) = 156180630 round up
	acc -= n;

	// + x^13/13!
	n = MyUMulHigh(n, n2); // mant * x^2
//	if (n >= 12*13/4)
	{
	n = MyUMulHigh(n, 0x06906907); // 2^34/(12*13) = 110127367 round up
	acc += n;
	} } } } } }

	// zero result
	if ((s32)acc <= 0) return 0;

	// limit to max. value 1.0 in format 2.30
	if ((u32)acc > 0x40000000) acc = 0x40000000;

	// normalize mantissa to max. position 1.31
	int d = clz(acc);
	acc <<= d;

	// prepare exponent
	int exp = FLOAT_EXP1 - d + 2;

	// rounding mantissa
	acc += 0x80-2;	// "-2" is an empirical correction to increase accuracy
	if ((s32)acc < 0) // no overflow
	{
		acc <<= 1; 	// remove leading bit '1'
		exp--;		// compensate exponent
	}

	// compose result
	return Test_u32float(((u32)acc >> 9) | (exp << 23) | (sign << 31));
}

// sine-cosine in radians - Taylor serie
void sincosf_fast(float x, float* psin, float* pcos)
{
	// get number as integer
	u32 xn = Test_floatu32(x);

	// get unsigned exponent
	int exp = (xn << 1) >> 24;

	// normalize big angle
	if (exp >= FLOAT_EXP1+7) // normalize angles >= 128.0 (= 20*(2*pi))
	{
		// normalize using remainderf, modulo 2*pi (remainderf returns angle in range -pi .. +pi)
		x = remainderf(x, Test_u32float(0x40C90FDB));
		xn = Test_floatu32(x);
	}

	// get exponent with sign
	// - signed exponent here is max. +6
	exp = xn >> 23; // get exponent with sign

	// unpack mantissa to format 9.23
	s32 m = ((xn << 9) >> 9) | B23;

	// negate mantissa
	if ((s32)xn < 0)
	{
		// clear sign bit
		exp &= 0xff;

		// negate mantissa
		m = -m;
	}

	// convert mantissa to signed format 8.24 (rounding down)
	int s = exp - FLOAT_EXP1 + (24-23); // exps <= +6, number of shifts <= +7

	// convert to signed fixed point 8.24
	// right shift
	if (s <= 0)
	{
		s = -s;
		if (s >= 31) s = 31;
		m >>= s;
	}

	// left shift
	else
		m <<= s;

	// 2*pi in 8.24 fixed point
	u32 pi = 0x06487ED5;	// = 2*pi << 24

	// limit max. angle to modulo 2*PI
	// - mantissa will be in range 0 .. 0x06487ED4 = 105414356
	do m -= pi; while (m >= 0); // limit positive angle
	do m += pi; while (m < 0); // result angle will be in range 0 (include) .. 2*PI (exclude)

	// convert angle to unsigned format 3.29
	// - mantissa will be in range 0 .. 0xC90FDA80 = 3373259392
	xn = m << 5;

// ---- mantissa here is in unsigned fixed point format 3.29, range 0..0xC90FDA80 = 3373259392 (= 0..2*pi)

	s32 sin_sign = 0;
	s32 cos_sign = 0;

	// convert to range 0..pi
	// - mantissa will be in range = 0 .. 0x6487ED4F (= 0..pi)
	pi <<= 4; // pi = 0x6487ED50 = 1686629712
	if (xn >= pi) // x >= PI ?
	{
		xn -= pi;
		sin_sign = ~sin_sign;
		cos_sign = ~cos_sign;
	}

	// convert to range 0..pi/2
	//  mantissa will be in range = 0 .. 0x3243F6A8 = 843314856 (= 0..pi/2) in unsigned fixed point format 3.29
	u32 pi2 = pi >> 1; // pi/2 = 0x3243F6A8 = 843314856
	if (xn >= pi2)
	{
		xn = pi - xn;
		cos_sign = ~cos_sign;
	}

	// convert to format 2.30
	//  xn will be in range 0..0x6487ED50 = 1686629712 (= 0..pi/2), unsigned fixed point format 2.30
	xn <<= 1;

	// Taylor serie
	*psin = _sinf_taylor(xn, sin_sign);
	*pcos = _sinf_taylor(pi - xn, cos_sign);
};

// sine-cosine in degrees - Taylor serie
void sincosf_deg_fast(float x, float* psin, float* pcos)
{
	// get number as integer
	u32 xn = Test_floatu32(x);

	// normalize big angle >= 360
	if ((xn << 1) >= (0x43B40000 << 1)) // absolute value of angle >= 360 deg
	{
		// normalize using remainderf, modulo 360 (remainderf returns angle in range -180 .. +180)
		x = remainderf(x, Test_u32float(0x43B40000)); // 360: exps=+8
		xn = Test_floatu32(x);
	}

	// absolute value of the angle
	// - angle will be in range 0..360
	s32 sin_sign = 0;
	if ((s32)xn < 0)
	{
		sin_sign = ~sin_sign;
		xn &= ~B31;
	}	

	// get unsigned exponent
	// - signed exponent here is max. +8
	int exp = xn >> 23; // get exponent

	// unpack mantissa to unsigned format 9.23, set hidden bit '1'
	xn = ((xn << 9) >> 9) | B23;

	// prepare number of shifts to format 9.23 (rounding down)
	int s = exp - FLOAT_EXP1 + (23-23); // exps <= +8, number of shifts <= +8

	// convert mantissa to unsigned fixed point format 9.23 (rounding down)
	// - mantissa will be in range = 0 .. 0xB4000000
	// right shift
	if (s <= 0)
	{
		s = -s;
		if (s >= 31) s = 31;
		xn >>= s;
	}

	// left shift (max. 8)
	else
		xn <<= s;

	// 180 in 9.23 fixed point
	u32 a = 0x5A000000;	// = 180 << 23

	// convert to range 0..180
	// - mantissa will be in range = 0 .. 0x5A000000 (= 0..180)
	s32 cos_sign = 0;
	if (xn >= a) // x >= 180 ?
	{
		xn -= a;
		sin_sign = ~sin_sign;
		cos_sign = ~cos_sign;
	}

	// convert to range 0..90
	//  mantissa will be in range = 0 .. 0x2D000000 = 754974720 (= 0..90)
	u32 a2 = a >> 1; // 90deg = 0x2D000000 = 754974720
	if (xn >= a2)
	{
		xn = a - xn;
		cos_sign = ~cos_sign;
	}

	// convert range 0..90 in unsigned fixed point format 9.23 to range 0..pi/2 in format 2.30
	// - input range: 0..90 in 9.23 = 0..0x2D000000 = 754974720
	// - output range: 0..pi/2 in 2.30 = 0..0x6487ED51 = 1686629713
	// k = (1686629713 / 754974720 << 32) >> 2 = 2398762259 round up
	xn = MyUMulHigh(xn, 2398762259) << 2; // max. value = 421657428 << 2 = 1686629712

	// Taylor serie
	*psin = _sinf_taylor(xn, sin_sign);
	*pcos = _sinf_taylor(1686629712 - xn, cos_sign); // pi in 2.30 format = 1686629712
}

float sinf_fast(float x)
{
	// small angle
	u32 xn = Test_floatu32(x);
	int exp = (xn >> 23) & 0xff;
	if (exp < FLOAT_EXP1-7) return x;

	float res_sin, res_cos;
	sincosf_fast(x, &res_sin, &res_cos);
	return res_sin;
}

float sinf_deg_fast(float x)
{
	float res_sin, res_cos;
	sincosf_deg_fast(x, &res_sin, &res_cos);
	return res_sin;
}

float cosf_fast(float x)
{
	float res_sin, res_cos;
	sincosf_fast(x, &res_sin, &res_cos);
	return res_cos;
}

float cosf_deg_fast(float x)
{
	float res_sin, res_cos;
	sincosf_deg_fast(x, &res_sin, &res_cos);
	return res_cos;
}

float tanf_fast(float x)
{
	// small angle
	u32 xn = Test_floatu32(x);
	int exp = (xn >> 23) & 0xff;
	if (exp < FLOAT_EXP1-8) return x;

	float res_sin, res_cos;
	sincosf_fast(x, &res_sin, &res_cos);
	return res_sin/res_cos;
}

float tanf_deg_fast(float x)
{
	float res_sin, res_cos;
	sincosf_deg_fast(x, &res_sin, &res_cos);
	return res_sin/res_cos;
}

float cotanf_fast(float x)
{
	float res_sin, res_cos;
	sincosf_fast(x, &res_sin, &res_cos);
	return res_cos/res_sin;
}

float cotanf_deg_fast(float x)
{
	float res_sin, res_cos;
	sincosf_deg_fast(x, &res_sin, &res_cos);
	return res_cos/res_sin;
}

#endif // USE_RISCV_SINF == 2

// ----------------------------------------------------------------------------
//                       asinf(x)... - Taylor serie
// ----------------------------------------------------------------------------
// arc sine/cosine - Taylor serie
//  arcsin(x) = x + x^3/2/3 + 3*x^5/2/4/5 + 3*5*x^7/2/4/6/7 +

#if USE_RISCV_ASINF == 2		// RISC-V asinf(), acosf() functions: 2=Taylor (recommended)

// arc sine/cosine - Taylor serie (result is -pi/2 .. +pi/2)
float _asincosf_fast(float x, Bool deg, Bool useacos)
{
	// get number as integer
	u32 xn = Test_floatu32(x);

	// get sign
	s32 sign = (s32)xn >> 31;

	// absolute value
	xn &= ~B31;

	// acos() +1 returns 0
	// acos() -1 returns pi or 180
	// asin() +-1 returns +-pi/2 or +-90
	if (xn == 0x3F800000)
	{
		// acos()
		if (useacos)
		{
			if (sign)
			{
				if (deg)
					// 180
					return Test_u32float(0x43340000);
				else
					// pi
					return Test_u32float(0x40490FDB);
			}
			else
				return 0;
		}

		// asin()
		if (deg)
			// +-90 deg
			return Test_u32float(0x42B40000 | (sign << 31));
		else
			// +-pi/2
			return Test_u32float(0x3FC90FDB | (sign << 31));
	}

	// get signed exponent
	int exp = (xn >> 23) - FLOAT_EXP1;

	// zero or small value
	if (exp < -10)
	{
		// asin(x) returns x
		if (!useacos)
		{
			if (deg)
				return rad2degf(x);
			else
				return x;
		}

		// acos(0) returns pi/2 or 90
		if (exp < -20)
		{
			if (deg)
				// 90
				return Test_u32float(0x42B40000);
			else
				// pi/2
				return Test_u32float(0x3FC90FDB);
		}
	}

	// invalid input value > 1.0
	if (exp >= 0) return Test_QNanF();

	// if > 0.707107 (= sqrt(0.5)), use alternate method sqrt(1 - x^2)
	Bool alt = False;
	if (xn > 0x3F3504F3 + 400000) // sqrt(0.5), plus small improvement correction
	{
		// use alternate method
		alt = True;

		// x = sqrt(1 - x^2)
		x = sqrtf(1 - fsqr(x));

		// reload exponent
		xn = Test_floatu32(x);
		exp = (xn >> 23) - FLOAT_EXP1;
	}

	// unpack mantissa to format 1.31
	xn = (xn << 8) | B31;
	xn >>= -exp;

	// prepare square x^2
	u32 x2 = MyUMulHigh(xn, xn) << 2;

	// prepare result accumulator
	u32 acc = xn;

	// (1) + x^3/(2*3)
	xn = MyUMulHigh(xn, x2);
	// 1/(2*3) << 32 = 715827883 round up
	acc += MyUMulHigh(xn, 715827883+99); // plus small correction to increase precision

	// (2) + 3*x^5/(2*4*5)
	xn = MyUMulHigh(xn, x2);
	// 3/(2*4*5) << 32 = 322122548 round up
	acc += MyUMulHigh(xn, 322122548);

	// (3) + 3*5*x^7/(2*4*6*7)
	xn = MyUMulHigh(xn, x2);
	if (xn >= 22)	// = 4294967296/191739612
	{
	// (3*5)/(2*4*6*7) << 32 = 191739612 round up
	acc += MyUMulHigh(xn, 191739612);

	// (4) + 3*5*7*x^9/(2*4*6*8*9)
	xn = MyUMulHigh(xn, x2);
	// (3*5*7)/(2*4*6*8*9) << 32 = 130489458 round up
	acc += MyUMulHigh(xn, 130489458);

	// (5) + 3*5*7*9*x^11/(2*4*6*8*10*11)
	xn = MyUMulHigh(xn, x2);
	// (3*5*7*9)/(2*4*6*8*10*11) << 32 = 96087692 round up
	acc += MyUMulHigh(xn, 96087692);

	// (6) + 3*5*7*9*11*x^13/(2*4*6*8*10*12*13)
	xn = MyUMulHigh(xn, x2);
	if (xn >= 57)	// = 4294967296/74529556
	{
	// (3*5*7*9*11)/(2*4*6*8*10*12*13) << 32 = 74529556 round up
	acc += MyUMulHigh(xn, 74529556);

	// (7) + 3*5*7*9*11*13*x^15/(2*4*6*8*10*12*14*15)
	xn = MyUMulHigh(xn, x2);
	// (3*5*7*9*11*13)/(2*4*6*8*10*12*14*15) << 32 = 59978548 round up
	acc += MyUMulHigh(xn, 59978548);

	// (8) + 3*5*7*9*11*13*15*x^17/(2*4*6*8*10*12*14*16*17)
	xn = MyUMulHigh(xn, x2);
	// (3*5*7*9*11*13*15)/(2*4*6*8*10*12*14*16*17) << 32 = 49614608 round up
	acc += MyUMulHigh(xn, 49614608);

	// (9) + 3*5*7*9*11*13*15*17*x^19/(2*4*6*8*10*12*14*16*18*19)
	xn = MyUMulHigh(xn, x2);
	if (xn >= 102) // = 4294967296/41925794
	{
	// (3*5*7*9*11*13*15*17)/(2*4*6*8*10*12*14*16*18*19) << 32 = 41925794 round up
	acc += MyUMulHigh(xn, 41925794);

	// (10) + 3*5*7*9*11*13*15*17*19*x^21/(2*4*6*8*10*12*14*16*18*20*21)
	xn = MyUMulHigh(xn, x2);
	// (3*5*7*9*11*13*15*17*19)/(2*4*6*8*10*12*14*16*18*20*21) << 32 = 36036218 round up
	acc += MyUMulHigh(xn, 36036218);

	// (11) + 3*5*7*9*11*13*15*17*19*21*x^23/(2*4*6*8*10*12*14*16*18*20*22*23)
	xn = MyUMulHigh(xn, x2);
	// (3*5*7*9*11*13*15*17*19*21)/(2*4*6*8*10*12*14*16*18*20*22*23) << 32 = 31407060 round up
	acc += MyUMulHigh(xn, 31407060);

	// (12) + 3*5*7*9*11*13*15*17*19*21*23*x^25/(2*4*6*8*10*12*14*16*18*20*22*24*25)
	xn = MyUMulHigh(xn, x2);
	if (xn >= 155) // = 4294967296/27690558
	{
	// (3*5*7*9*11*13*15*17*19*21*23)/(2*4*6*8*10*12*14*16*18*20*22*24*25) << 32 = 27690558 round up
	acc += MyUMulHigh(xn, 27690558);

	// (13) + 3*5*7*9*11*13*15*17*19*21*23*25*x^27/(2*4*6*8*10*12*14*16*18*20*22*24*26*27)
	xn = MyUMulHigh(xn, x2);
	// (3*5*7*9*11*13*15*17*19*21*23*25)/(2*4*6*8*10*12*14*16*18*20*22*24*26*27) << 32 = 24653275 round up
	acc += MyUMulHigh(xn, 24653275);

	// (14) + 3*5*7*9*11*13*15*17*19*21*23*25*27*x^29/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*29)
	xn = MyUMulHigh(xn, x2);
	// (3*5*7*9*11*13*15*17*19*21*23*25*27)/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*29) << 32 = 22133297 round up
	acc += MyUMulHigh(xn, 22133297);

	// (15) + 3*5*7*9*11*13*15*17*19*21*23*25*27*29*x^31/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*31)
	xn = MyUMulHigh(xn, x2);
	if (xn >= 214) // = 4294967296/20015164
	{
	// (3*5*7*9*11*13*15*17*19*21*23*25*27*29)/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*31) << 32 = 20015164 round up
	acc += MyUMulHigh(xn, 20015164);

	// (16) + 3*5*7*9*11*13*15*17*19*21*23*25*27*29*31*x^33/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*32*33)
	xn = MyUMulHigh(xn, x2);
	// (3*5*7*9*11*13*15*17*19*21*23*25*27*29*31)/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*32*33) << 32 = 18214558 round up
	acc += MyUMulHigh(xn, 18214558);

	// (17) + 3*5*7*9*11*13*15*17*19*21*23*25*27*29*31*33*x^35/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*32*34*35)
	xn = MyUMulHigh(xn, x2);
	// (3*5*7*9*11*13*15*17*19*21*23*25*27*29*31*33)/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*32*34*35) << 32 = 16668616 round up
	acc += MyUMulHigh(xn, 16668616);

	// (18) + 3*5*7*9*11*13*15*17*19*21*23*25*27*29*31*33*35*x^37/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*32*34*36*37)
	xn = MyUMulHigh(xn, x2);
	// (3*5*7*9*11*13*15*17*19*21*23*25*27*29*31*33*35)/(2*4*6*8*10*12*14*16*18*20*22*24*26*28*30*32*34*36*37) << 32 = 15329621 round up
	acc += MyUMulHigh(xn, 15329621);
	} } } } }

// acc value here is in range = 0..3373259426 (=0..0xC90FDAA2)

	// recalc to degrees in 7.25 format
	u32 pi;
	if (deg)
	{
		// (180/pi << 32) >> 6 = 3845054676 round up, max. result value
		acc = MyUMulHigh(acc, 3845054676+30); // plus small correction to increase precision

		pi = 3019898880-21; // 90 in 7.25, plus small correction to increase precision
		exp = FLOAT_EXP1 + 1 + 6; // prepare exponent base
		if (acc > pi) acc = pi; // limit of overflow range
		if (alt) acc = pi - acc;
		if (useacos)
		{
			// acos_deg(x) = 90 - asin_deg(x)
			if (!sign)
			{
				// asin_deg(x) is positive
				acc = pi - acc;
			}
			else
			{
				// asin_deg(x) is negative
				acc = (acc >> 1) + 1509949435; // cos: add 90
				exp++;
				sign = False;
			}
		}
	}
	else
	{
		// pi/2 = 3373259426 = 0xC90FDAA2
		pi = 3373259426-6; // pi/2 in 1.31, plus small correction to increase precision
		exp = FLOAT_EXP1 + 1; // prepare exponent base
		if (acc > pi) acc = pi; // limit of overflow range
		if (alt) acc = pi - acc;
		if (useacos)
		{
			// acos(x) = pi/2 - asin(x)
			if (!sign)
			{
				// asin(x) is positive
				acc = pi - acc;
			}
			else
			{
				// asin(x) is negative
				acc = (acc >> 1) + 1686629712; // cos: add pi/2, format 2.30
				exp++;
				sign = False;
			}
		}
	}

	// zero result
	if (acc == 0) return 0;

	// normalize mantissa to max. position 1.31
	int d = clz(acc);
	acc <<= d;

	// prepare exponent
	exp -= d;

	// rounding mantissa
	acc += 0x80;
	if (acc >= 0x80) // no overflow
	{
		acc <<= 1; 	// remove leading bit '1'
		exp--;		// compensate exponent
	}

	// compose result
	return Test_u32float(((u32)acc >> 9) | (exp << 23) | (sign << 31));
}

// arc sine in radians - Taylor serie (result is -pi/2 .. +pi/2)
// 693ns 0.037 2 (full: 381ns 0.139 3)
float asinf_fast(float x)
{
	return _asincosf_fast(x, False, False);
}

// arc sine in degrees - Taylor serie (result is -90 .. +90)
// 704ns 0.190 2 (full: 549ns 0.350 5)
float asinf_deg_fast(float x)
{
	return _asincosf_fast(x, True, False);
}

// arc cosine in radians
// 733ns 0.025 9 (full: 564ns 0.280 4)
float acosf_fast(float x)
{
	return _asincosf_fast(x, False, True);
}

// arc cosine in degrees
// 715ns 0.117 5 (full: 554ns 0.301 4)
float acosf_deg_fast(float x)
{
	return _asincosf_fast(x, True, True);
}

#endif // USE_RISCV_ASINF == 2


// ----------------------------------------------------------------------------
//                    atanf(x)... - Chebyshev approximation
// ----------------------------------------------------------------------------
// arc tan/cotan - Chebyshev approximation

#if USE_RISCV_ATANF == 1	// RISC-V atanf(), acotanf(), atan2f() functions: 1=Chebyshev (recommended)

// Chebyshev coefficients of ATan()
//const REALEXT REALNAME(ConstChebATan)[CHEB_ATAN] = {	// CHEB_ATAN = 14
//	 { { 0xded3d42e }, 0x3ffffffe},	// 0  (0.870419751387089)
//	 { { 0xd0a4a60d }, 0xbffffffb},	// 1  (-0.10187654235051)
//	 { { 0xaad1804f }, 0x3ffffff9},	// 2  (0.0208518510844442)
//	 { { 0xa4d9f15d }, 0xbffffff7},	// 3  (-0.00503086362186878)
//	 { { 0xac6fcdd4 }, 0x3ffffff5},	// 4  (0.00131558787870745)
//	 { { 0xbd4246d1 }, 0xbffffff3},	// 5  (-0.000360982690949641)
//	 { { 0xd67728dc }, 0x3ffffff1},	// 6  (0.000102265103919308)
//	 { { 0xf8b14690 }, 0xbfffffef},	// 7  (-2.9646454208887e-005)
//	 { { 0x93414f63 }, 0x3fffffee},	// 8  (8.77708897562002e-006)
//	 { { 0xb01a5c32 }, 0xbfffffec},	// 9  (-2.624138732088e-006)
//	 { { 0xc7c664e4 }, 0x3fffffea},	// 10  (7.44219783577194e-007)
//	 { { 0xf333ceef }, 0xbfffffe8},	// 11  (-2.26499863253959e-007)
//	 { { 0xe161bc7f }, 0x3fffffe7},	// 12  (1.04951570961687e-007)
//	 { { 0x8bc6b5e6 }, 0xbfffffe6},	// 13  (-3.2544185651906e-008)
//};

//  arc tangent in radians: result is -pi/2 .. +pi/2
//  arc tangent in degrees: result is -90 .. +90
//  arc cotangent in radians: result is 0 .. pi
//  arc cotangent in degrees: result is 0 .. 180
float _atanf_fast(float x, Bool deg, Bool useacotan)
{
	// get number as integer
	u32 xn = Test_floatu32(x);

	// get sign
	s32 sign = (s32)xn >> 31;

	// absolute value
	xn &= ~B31;

	// get signed exponent
	int exp = (xn >> 23) - FLOAT_EXP1;

// acotan(x) = pi/2 - atan(x)

	// infinity or big value
	if (exp > 9)
	{
		// acotan()
		if (useacotan)
		{
			// positive
			if (sign == 0)
			{
				// positive infinity - return x
				if (deg)
					return rad2degf(frec(x));
				else
					return frec(x);
			}

			// negative
			else
			{
				float res;
				if (deg)
					// 180 deg
					res = Test_u32float(0x43340000);
				else
					// pi rad
					res = Test_u32float(0x40490FDB);

				if (exp < 23)
				{
					if (deg)
						// 180 deg
						res += rad2degf(frec(x));
					else
						// pi rad
						res += frec(x);
				}
				return res;
			}
		}

		// atan()
		else
		{
			float res;
			if (deg)
				// +-90 deg
				res = Test_u32float(0x42B40000 | (sign << 31));
			else
				// +-pi/2 rad
				res = Test_u32float(0x3FC90FDB | (sign << 31));

			if (exp < 23)
			{
				if (deg)
					// +-90 deg
					res -= rad2degf(frec(x));
				else
					// +-pi/2 rad
					res -= frec(x);
			}
			return res;
		}
	}

	// zero or small angle
	if (exp < -9)
	{
		exp += FLOAT_EXP1;
		if (deg && (exp != 0)) x = rad2degf(x);

		// acotan()
		if (useacotan)
		{
			float res;
			if (deg)
				// +90 deg
				res = Test_u32float(0x42B40000);
			else
				// +pi/2 rad
				res = Test_u32float(0x3FC90FDB);

			if (exp != 0) res -= x;
			return res;
		}

		// atan()
		else
		{
			return x;
		}
	}

	// if abs(x) > 1, use alternative method with acotan()
	s32 resadd = 0; // result addition = 0
	if (exp >= 0)
	{
		// use -1/x
		x = -frec(x);

		// result addition = pi/2
		resadd = 1686629713-4; // pi/2 in format 2.30

		// is x is positive, use -pi/2
		if (!Test_IsNegF(x)) resadd = -resadd;

		// get number as integer
		xn = Test_floatu32(x);
		sign = (s32)xn >> 31;	// get sign
		xn &= ~B31;		// absolute value
		exp = (xn >> 23) - FLOAT_EXP1; // get signed exponent
	}

// argument x is in range 0..+1 here

	// normalize mantissa - shift to max position 1.31 and restore hidden bit '1'
	xn = (xn << 8) | B31;

	// shift mantissa right by exponent bits, to get signed fixed number in format 2.30
	xn >>= 1 - exp; // exp is in range -20..0, shift is in range 1..21

	// calculate 2*x^2 - 1, signed fixed point format 2.30
	//   y = x*fnc(2*x^2 - 1)
	s32 m = MyUMulHigh(xn, xn) << (2+1);
	m = m - 0x40000000; // -1

// argument x is in range -1..+1 here, signed fixed point format 2.30

	s32 acc = -35-2;	// coeff[13] = -35 = -3.2544185651906e-008 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 113+1 + acc; 	// coeff[12] = 113 = 1.04951570961687e-007 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = -243-2 + acc;	// coeff[11] = -243 = -2.26499863253959e-007 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 799+20 + acc;	// coeff[10] = 799 = 7.44219783577194e-007 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = -2818+1 + acc;	// coeff[9] = -2818 = -2.624138732088e-006 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 9424+3 + acc;	// coeff[8] = 9424 = 8.77708897562002e-006 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = -31833+2 + acc;	// coeff[7] = -31833 = -2.9646454208887e-005 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 109806+3 + acc;	// coeff[6] = 109806 = 0.000102265103919308 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = -387602+3 + acc;	// coeff[5] = -387602 = -0.000360982690949641 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 1412602 + acc;	// coeff[4] = 1412602 = 0.00131558787870745 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = -5401849 + acc;	// coeff[3] = -5401849 = -0.00503086362186878 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 22389505+1 + acc;	// coeff[2] = 22389505 = 0.0208518510844442 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = -109389104-1 + acc; // coeff[1] = -109389104 = -0,10187654235051 << 30
	acc = MySMulHigh(acc, m) << 2;

	acc = 934606091 + acc; // coeff[0] = 934606091 = 0,870419751387089 << 30

	// result is -pi/2..+pi/2 in 2.30 format
	acc = MySMulHigh(acc, xn) << 2;
	if (sign) acc = -acc;

	// add alternative correction
	acc += resadd;

	// acotan() correction: acotan(x) = pi/2 - atan(x)
	exp = FLOAT_EXP1 + 2;
	if (useacotan)
	{
		if (acc >= 0)
			acc = 1686629713-3 - acc;
		else
		{
			acc = 843314857 - acc/2; // pi/2 in format 3.29
			exp++;
		}
	}

	// zero result
	if (acc == 0) return 0;

	// absolute value
	sign = acc >> 31;
	if (acc < 0) acc = -acc;

	// convert to degrees: x = x * (180/PI)
	if (deg)
	{
		// x = x * (180/PI) ... (180/PI << 32) >> 6 = 3845054675
		acc = MyUMulHigh(acc, (useacotan) ? (3845054675+3) : (3845054675+15));
		exp += 6;
	}

	// normalize mantissa to max. position 1.31
	int d = clz(acc);
	acc <<= d;

	// prepare exponent
	exp -= d;

	// rounding mantissa
	acc += 0x80;
	if ((u32)acc >= 0x80) // no overflow
	{
		acc <<= 1; 	// remove leading bit '1'
		exp--;		// compensate exponent
	}

	// compose result
	return Test_u32float(((u32)acc >> 9) | (exp << 23) | (sign << 31));
}

// arc tangent in radians (result is -pi/2 .. +pi/2)
// 1130ns 0.226 3 (full: 343ns 0.075 5)
float atanf_fast(float x)
{
	return _atanf_fast(x, False, False);
}

// arc tangent in degrees (result is -90 .. +90)
// 1150ns 0.307 4 (full: 496ns 0.115 6)
float atanf_deg_fast(float x)
{
	return _atanf_fast(x, True, False);
}

// arc cotangent in radians (result is 0 .. pi)
// 1180ns 0.154 3 (full: 551ns 0.044 5)
float acotanf_fast(float x)
{
	return _atanf_fast(x, False, True);
}

// arc cotangent in degrees (result is 0 .. 180)
// 1220ns 0.170 4 (full: 749ns 0.316 24)
float acotanf_deg_fast(float x)
{
	return _atanf_fast(x, True, True);
}

#endif // USE_RISCV_ATANF == 1
