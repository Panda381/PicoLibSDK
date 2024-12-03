
// ****************************************************************************
//
//                       Double RP2350 - RISC-V science functions
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

#include "../include.h"

// compilation setup
#define USE_RISCV_EXP		1		// RISC-V exp(), exp2() and exp10() function: 1=Chebyshev (recommended), 2=Taylor
#define EXPD_MODE		1		// exp mode: 1=exp, 2=exp2, 3=exp10

#define USE_RISCV_LOG		1		// RISC-V log(), log2() and log10() function: 1=Chebyshev (recommended)
#define LOGD_MODE		1		// log mode: 1=log, 2=log2, 3=log10

INLINE u32 MyUMulHigh(u32 a, u32 b) { return (u32)(((u64)a*b) >> 32); }
INLINE s32 MySMulHigh(s32 a, s32 b) { return (s32)(((s64)a*b) >> 32); }
INLINE u64 MyUMul(u32 a, u32 b) { return (u64)a*b; }
INLINE s64 MySMul(s32 a, s32 b) { return (s64)a*b; }
INLINE s32 MySUMulHigh(s32 a, u32 b) { return (s32)((s64)((s64)a*(u64)b) >> 32); }

// (u64 * u64 -> u128) >> 64
u64 MyU64MulHigh(u64 a, u64 b)
{
	//       aHaL
	//     x bHbL
	// ----------
	//       aLbL ...   k0
	//     aLbH   ...  k1
	//     aHbL   ...  k2
	//   aHbH     ... k3

	// prepare elements
	u32 aL = (u32)a;
	u32 aH = (u32)(a >> 32);
	u32 bL = (u32)b;
	u32 bH = (u32)(b >> 32);

	// multiply elements (max. FFFF * FFFF = FFFE0001)
	u64 k0 = (u64)aL*bL;
	u64 k1 = (u64)aL*bH;
	u64 k2 = (u64)aH*bL;
	u64 k3 = (u64)aH*bH;

	// add komponents
	k1 += (k0 >> 32); // max. FFFE0001 + FFFE = FFFEFFFF, no carry yet
	k1 += k2; // max. FFFEFFFF + FFFE0001 = 1FFFD0000, it can carry
	if (k1 < k2) k3 += (u64)1 << 32; // add carry, FFFE0001 + 10000 = FFFF0001, no carry

	// result, max. FFFFFFFF * FFFFFFFF = FFFFFFFE:00000001
//	u64 rL = k0 + (k1 << 32); // result low, FFFF0000 + FFFF = FFFFFFFF, no carry
	u64 rH = k3 + (k1 >> 32); // result high, FFFF0001 + FFFD = FFFFFFFE, no carry 

	return rH;
}

// (u64 * u64 -> u128) >> 64 ... simplified
INLINE u64 MyU64MulHighSimple(u64 a, u64 b)
{
	//       aHaL
	//     x bHbL
	// ----------
	//       aLbL ...   k0
	//     aLbH   ...  k1
	//     aHbL   ...  k2
	//   aHbH     ... k3

	// prepare elements
	u32 aL = (u32)a;
	u32 aH = (u32)(a >> 32);
	u32 bL = (u32)b;
	u32 bH = (u32)(b >> 32);

	u32 k1 = (u32)(((u64)aL*bH)>>32);
	u32 k2 = (u32)(((u64)aH*bL)>>32);
	u64 k3 = (u64)aH*bH;
	return k3 + k2 + k1;
}


// (s64 * s64 -> s128) >> 64
s64 MyS64MulHigh(s64 a, s64 b)
{
	Bool neg = False;
	if (a < 0) { a = -a; neg = True; }
	if (b < 0) { b = -b; neg = !neg; }

	u32 aL = (u32)a;
	u32 aH = (u32)(a >> 32);
	u32 bL = (u32)b;
	u32 bH = (u32)(b >> 32);

	u64 k0 = (u64)aL*bL;
	u64 k1 = (u64)aL*bH;
	u64 k2 = (u64)aH*bL;
	u64 k3 = (u64)aH*bH;

	k1 += (k0 >> 32);
	k1 += k2;
	if (k1 < k2) k3 += (u64)1 << 32;

	u64 rL = (k1 << 32) + (u32)k0;
	u64 rH = k3 + (k1 >> 32);

	if (neg)
	{
		rH = ~rH;
		if (rL == 0) rH++;
	}

	return rH;
}


/*
// (s64 * s64 -> s128) >> 64
INLINE s64 MyS64MulHighSimple(s64 a, s64 b)
{
	Bool neg = False;
	if (a < 0) { a = -a; neg = True; }
	if (b < 0) { b = -b; neg = !neg; }

	u32 aL = (u32)a;
	u32 aH = (u32)(a >> 32);
	u32 bL = (u32)b;
	u32 bH = (u32)(b >> 32);

	u32 k1 = (u32)(((u64)aL*bH)>>32);
	u32 k2 = (u32)(((u64)aH*bL)>>32);
	u64 k3 = (u64)aH*bH;
	u64 rH = k3 + k2 + k1;

	if (neg) rH = ~rH;
	return rH;
}
*/

// (s64 * s64 -> s128) >> 64 (b is positive)
INLINE s64 MyS64MulHighSimple_Plus(s64 a, s64 b)
{
	s32 a2 = (s32)(a >> 32);
	if (a2 < 0) a = ~a; // use ~ instead of -, faster and better results

	u32 aL = (u32)a;
	u32 aH = (u32)(a >> 32);
	u32 bL = (u32)b;
	u32 bH = (u32)(b >> 32);

	u32 k1 = (u32)(((u64)aL*bH)>>32);
	u32 k2 = (u32)(((u64)aH*bL)>>32);
	u64 k3 = (u64)aH*bH;
	u64 rH = k3 + k2 + k1;

	if (a2 < 0) rH = ~rH;
	return rH;
}

// (s64 * s64 -> s128) >> 64 (b is negative)
INLINE s64 MyS64MulHighSimple_Minus(s64 a, s64 b)
{
	s32 a2 = (s32)(a >> 32);
	if (a2 < 0) a = ~a; // use ~ instead of -, faster and better results

	u32 aL = (u32)a;
	u32 aH = (u32)(a >> 32);
	u32 bL = (u32)b;
	u32 bH = (u32)(b >> 32);

	u32 k1 = (u32)(((u64)aL*bH)>>32);
	u32 k2 = (u32)(((u64)aH*bL)>>32);
	u64 k3 = (u64)aH*bH;
	u64 rH = k3 + k2 + k1;

	if (a2 >= 0) rH = ~rH;
	return rH;
}

INLINE int MyGetExpD(double num) { return (int)((Test_doubleu64(num) >> DOUBLE_MANTBITS) & DOUBLE_EXPMASK); }
INLINE int MyGetSignD(double num) { return (int)((Test_doubleu64(num) >> (DOUBLE_BITS-1)) & 1); }

// ----------------------------------------------------------------------------
//                      Natural exponent - Taylor serie
// ----------------------------------------------------------------------------
//  Taylor serie exp(x) = 1 + x/1! + x^2/2! + x^3/3! + x^4/4! + x^5/5! + ...

#if USE_RISCV_EXP == 2		// RISC-V exp(), exp2() and exp10() function: 1=Chebyshev (recommended), 2=Taylor

// exp: 2630ns avg=0.157 max=2
// exp2: 2310ns avg=0.343 max=2
// exp10: 2760ns avg=0.305 max=2
double FASTCODE NOFLASH(exp_fast)(double x)
{
	// get number as integer
	u64 xn = Test_doubleu64(x);

	// max. value (x > 0), return +inf
	if ((s64)xn >= 0)
	{
#if EXPD_MODE == 3 // exp10
		// exp10 max. value: 308.25471555991668 = 0x40734413509F79FE, infinity: 308.25471555991676 = 0x40734413509F79FF
		if (xn > 0x40734413509F79FEull) return Test_PInfD(); // exp=8
#elif EXPD_MODE == 2 // exp2
		// exp2 max. value: 1023.9999999999997 = 0x408FFFFFFFFFFFFF, infinity: 1024.0000000000000 = 0x4090000000000000
		if (xn > 0x408FFFFFFFFFFFFFull) return Test_PInfD(); // exp=9
#else // exp
		// exp max. value: 709.78271289338401 = 0x40862E42FEFA39EF, infinity: 709.78271289338419 = 0x40862E42FEFA39F1
		if (xn > 0x40862E42FEFA39EFull) return Test_PInfD(); // exp=9
#endif
	}

	// min. value (x < 0), return +0.0
	else
	{
#if EXPD_MODE == 3 // exp10
		// exp10 min. value: -307.65265556858873 = 0xC0733A7146F72A41, zero: -307.65265556858878 = 0xC0733A7146F72A42
		if (xn > 0xC0733A7146F72A41ull) return Test_PZeroD(); // exp=8
#elif EXPD_MODE == 2 // exp2
		// exp2 min. value: -1022.0000000000000 = 0xC08FF00000000000, zero: -1022.0000000000001 = 0xC08FF00000000001
		if (xn > 0xC08FF00000000000ull) return Test_PZeroD(); // exp=9
#else // exp
		// exp min. value: -708.39641853226407 = 0xC086232BDD7ABCD2, zero: -708.39641853226424 = 0xC086232BDD7ABCD3
		if (xn > 0xC086232BDD7ABCD2ull) return Test_PZeroD(); // exp=9
#endif
	}

	// get signed exponent
	int exp = MyGetExpD(x) - DOUBLE_EXP1;

	// check minimal exponent - return 1.0
#if EXPD_MODE == 3 // exp10
	if (exp <= -56) return Test_P1D();
#else
	if (exp <= -54) return Test_P1D();
#endif

// exp is in range -53..+9 for exp and exp2, -55..+8 for exp10

	// get sign
	int sign = MyGetSignD(x);

	// normalize mantissa - shift to max position and restore hidden bit '1', mantissa format 1.63
	// - mantissa will be 0x8000000000000000..0xFFFFFFFFFFFFF800
	//    max exp10: (+)0x9A209A84FBCFF000 exp=8, (-)0x99D38A37B9520800 exp=8
	//    max exp2: (+)0xFFFFFFFFFFFFF800 exp=9, (-)0xFF80000000000000 exp=9
	//    max exp: (+)0xB17217F7D1CF7800 exp=9, (-)0xB1195EEBD5E69000 exp=9
	xn = (xn << 11) | 0x8000000000000000ull;

#if EXPD_MODE == 3 // exp10
	// multiply mantissa by ln(10) << 62 (= 10618799479599967255 round up) = mantissa/4
	xn = MyU64MulHighSimple(xn, 10618799479599967255ull + 2);
	exp += 2;
#endif

// max exp10: (+)0x58B90BFBE8E7B876 exp=10, min exp10: (-)0x588CAF75EAF34495 exp=10
// max exp2: (+)0xFFFFFFFFFFFFF800 exp=9, (-)0xFF80000000000000 exp=9
// max exp: (+)0xB17217F7D1CF7800 exp=9, (-)0xB1195EEBD5E69000 exp=9
// exp is in range -53..+9 for exp and exp2, -53..+10 for exp10

	// shift mantissa right by exponent bits
	// - get x as signed fixed point number in format 11.53 (current format is 1.63)
	xn >>= 63 - exp - 53; // exp is in range -53..+10, shift is in range 0..63

	// signed mantissa, signed fixed point format 11.53
	s64 m = xn;
	if (sign) m = -m;

// exp10: -0x588CAF75EAF34495 .. +0x58B90BFBE8E7B876
// exp2: -0x7FC0000000000000 .. +0x7FFFFFFFFFFFFC00
// exp: -0x588CAF75EAF34800 .. +0x58B90BFBE8E7BC00

#if EXPD_MODE == 2 // exp2
	// get integer part of exponent (round down to negative)
	// - exponent will be -1022..+1023
	exp = (int)(m >> 53);

	// convert exponent back to base
	s64 d = (s64)exp << 53;

	// subtract exponent correction from the number
	// - number will now be in range 0 .. 0x001FFFFFFFFFFFFF
	xn = m - d;

	// convert to range ln(2), unsigned fixed point format 11.53 (without integer part)
	// ln(2) << 64 = 12786308645202655660 round up
	// - number will be in range 0 .. 0x00162E42FEFA39EE = 6243314768165358
	// The "+4100" correction increases accuracy and has been determined empirically.
	xn = MyU64MulHighSimple(xn, 12786308645202655660ull+4100);

#else // exp, exp10
	// convert number to 2-base (multiply by 1/ln(2)), format 12.52
	// 1/ln(2) << 63 = 13306513097844322492 round up
	s64 m2;
	if (m >= 0)
		m2 = MyU64MulHighSimple(m, 13306513097844322492ull);
	else
		m2 = -MyU64MulHighSimple(-m + 1, 13306513097844322492ull)-1;

// exp10: -0x3FDFFFFFFFFFFCD6 .. +0x3FFFFFFFFFFFFCD8
// exp: -0x3FDFFFFFFFFFFF4D .. +0x3FFFFFFFFFFFFF65

	// get integer part of exponent: shift >> 52, round to zero
	// - exponent will be -1022..+1023
	exp = (int)(m2 >> 52);

	// convert exponent back to e-base: multiply by ln(2) and convert to 11.53 format, round to zero
	//   ln(2) << 63 = 6393154322601327830 round up
	// - exponent correction will be -6380667693064997112 .. 6386911007833162470 = -0x588CAF75EAF348F8 .. +0x58A2DDB8E9ED82E6
	//  Later we will discard the upper 11 bits of the result,
	//  so we will not need the HIGH result of the multiplication.
	s64 d = (s64)(exp * (6393154322601327830ll)) >> 10;

// exp10: -0x588CAF75EAF34495 .. +0x58B90BFBE8E7B876
// exp: -0x588CAF75EAF34800 ..   +0x58B90BFBE8E7BC00
// correction = -0x588CAF75EAF348F8 .. +0x58A2DDB8E9ED82E6
// number will be in range 0x00000000000000F8 .. 0x00162E42FEFA391A

	// subtract exponent correction from the number
	xn = m - d;

#endif

	// use mantissa in format 0.64
	xn <<= 11;

	// inconsistency correction (due to rounding)
	if (xn > 0xB17217F7D1CF79ABull) xn = 0xB17217F7D1CF79ABull;

	// prepare member
	u64 mem = xn;

	// prepare accumulator (without first member '1')
	u64 acc = mem;

	// Taylor serie exp(x) = 1 + x/1! + x^2/2! + x^3/3! + x^4/4! + x^5/5! + ...
	mem = MyU64MulHighSimple(mem, xn); // x^2
//	if (mem >= 2)
	{
	mem >>= 1; // 1/2
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^3
//	if (mem >= 3)
	{
	// The "-6500" correction increases accuracy and has been determined empirically.
	mem = MyU64MulHighSimple(mem, 6148914691236517206ull - 6500); // 1/3 << 64
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^4
//	if (mem >= 4)
	{
	mem >>= 2; // 1/4
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^5
//	if (mem >= 5)
	{
	mem = MyU64MulHighSimple(mem, 3689348814741910324ull); // 1/5 << 64
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^6
//	if (mem >= 6)
	{
	mem = MyU64MulHighSimple(mem, 3074457345618258603ull); // 1/6 << 64
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^7
	if (mem >= 7)
	{
	mem = MyU64MulHighSimple(mem, 2635249153387078803ull); // 1/7 << 64
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^8
//	if (mem >= 8)
	{
	mem >>= 3; // 1/8
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^9
//	if (mem >= 9)
	{
	mem = MyU64MulHighSimple(mem, 2049638230412172402ull); // 1/9 << 64
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^10
	if (mem >= 10)
	{
	mem = MyU64MulHighSimple(mem, 1844674407370955162ull); // 1/10 << 64
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^11
//	if (mem >= 11)
	{
	mem = MyU64MulHighSimple(mem, 1676976733973595602ull); // 1/11 << 64
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^12
	if (mem >= 12)
	{
	mem = MyU64MulHighSimple(mem, 1537228672809129302ull); // 1/12 << 64
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^13
//	if (mem >= 13)
	{
	mem = MyU64MulHighSimple(mem, 1418980313362273202ull); // 1/13 << 64
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^14
//	if (mem >= 14)
	{
	mem = MyU64MulHighSimple(mem, 1317624576693539402ull); // 1/14 << 64
	acc += mem;

	mem = MyU64MulHighSimple(mem, xn); // x^15
//	if (mem >= 15)
	{
	mem = MyU64MulHighSimple(mem, 1229782938247303442ull); // 1/15 << 64
	acc += mem;
	} } } } } } } } } } } } } }

	// Rounding
	acc += 0x800;

	// overflow (overflow will never occur, but beware)
//	if (acc < 0x800) acc = 0xffffffffffffffffull;

	// Normalize mantissa
	acc >>= 11+1;

	// return number
	return Test_u64double(acc | (((u64)exp + DOUBLE_EXP1) << 52));
}

#endif // USE_RISCV_EXP == 2

// ----------------------------------------------------------------------------
//                    Natural exponent - Chebyshev approximation
// ----------------------------------------------------------------------------

// 64-bit Chebyshev coefficients of Exp() - see calculations in real_c.h
// const REALEXT REALNAME(ConstChebExp)[CHEB_EXP] = {	// CHEB_EXP = 17
//	 { { 0x00000000, 0x80000000 }, 0x3fffffff},	// 0  (1.0)
//	 { { 0xd1cf79ac, 0xb17217f7 }, 0x3ffffffe},	// 1  (6.9314718055994530943e-1)
//	 { { 0x162c7543, 0xf5fdeffc }, 0x3ffffffc},	// 2  (2.4022650695910071233e-1)
//	 { { 0x2505fc6e, 0xe35846b8 }, 0x3ffffffa},	// 3  (5.5504108664821580022e-2)
//	 { { 0xd273b952, 0x9d955b7d }, 0x3ffffff8},	// 4  (9.6181291076284771650e-3)
//	 { { 0x533964b8, 0xaec3ff3c }, 0x3ffffff5},	// 5  (1.3333558146428433721e-3)
//	 { { 0x363c3075, 0xa184897c }, 0x3ffffff2},	// 6  (1.5403530393381606220e-4)
//	 { { 0x45f6aee9, 0xffe5fe2c }, 0x3fffffee},	// 7  (1.5252733804065937795e-5)
//	 { { 0x2e63c17b, 0xb160111d }, 0x3fffffeb},	// 8  (1.3215486790146656182e-6)
//	 { { 0xf3d0701a, 0xda929e9b }, 0x3fffffe7},	// 9  (1.0178086007207596430e-7)
//	 { { 0xe90bb66d, 0xf267a8ab }, 0x3fffffe3},	// 10  (7.0549116200190313717e-9)
//	 { { 0xd9b72cff, 0xf46564fc }, 0x3fffffdf},	// 11  (4.4455386559996910448e-10)
//	 { { 0x0f14f07f, 0xe1deb362 }, 0x3fffffdb},	// 12  (2.5678437471925562869e-11)
//	 { { 0x35f77a35, 0xc0af4c35 }, 0x3fffffd7},	// 13  (1.3691075408642918388e-12)
//	 { { 0xe66fd500, 0x98a3c792 }, 0x3fffffd3},	// 14  (6.7785672188660199117e-14)
//	 { { 0xb2d4d7ee, 0xe36a918c }, 0x3fffffce},	// 15  (3.1560349252155140648e-15)
//	 { { 0xd4fde9f4, 0x9d806a98 }, 0x3fffffca},	// 16  (1.3661088453272162965e-16)
//};

#if USE_RISCV_EXP == 1		// RISC-V exp(), exp2() and exp10() function: 1=Chebyshev (recommended), 2=Taylor

u64 exp_fast_tab[10] = {
	65070074559ull,			// coeff[10] = 65070074559 = 7.0549116200190313717e-9 << 63
	938762738676ull,		// coeff[9] = 938762738676 = 1.0178086007207596430e-7 << 63
	12189135131366ull,		// coeff[8] = 12189135131366 = 1.3215486790146656182e-6 << 63
	140681638454011ull,		// coeff[7] = 140681638454011 = 1.5252733804065937795e-5 << 63
	1420724914991586ull,		// coeff[6] = 1420724914991586 = 1.5403530393381606220e-4 << 63
	12298036735954521ull,		// coeff[5] = 12298036735954521 = 1.3333558146428433721e-3 << 63
	88711583058159475ull,		// coeff[4] = 88711583058159475 = 9.6181291076284771650e-3 << 63
	511935043789664227ull,		// coeff[3] = 511935043789664227 = 5.5504108664821580022e-2 << 63
	2215698446797868712ull,		// coeff[2] = 2215698446797868712 = 2.4022650695910071233e-1 << 63
	6393154322601327830ull,		// coeff[1] = 6393154322601327830 = 6.9314718055994530943e-1 << 63
};

// exp: 2180ns avg=0.104 max=1
// exp2: 1870ns avg=0.089 max=1
// exp10: 2270ns avg=0.285 max=2
double /*FASTCODE*/ NOFLASH(exp_fast)(double x)
{
	// get number as integer
	u64 xn = Test_doubleu64(x);

	// max. value (x > 0), return +inf
	if ((s64)xn >= 0)
	{
#if EXPD_MODE == 3 // exp10
		// exp10 max. value: 308.25471555991668 = 0x40734413509F79FE, infinity: 308.25471555991676 = 0x40734413509F79FF
		if (xn > 0x40734413509F79FEull) return Test_PInfD(); // exp=8
#elif EXPD_MODE == 2 // exp2
		// exp2 max. value: 1023.9999999999997 = 0x408FFFFFFFFFFFFF, infinity: 1024.0000000000000 = 0x4090000000000000
		if (xn > 0x408FFFFFFFFFFFFFull) return Test_PInfD(); // exp=9
#else // exp
		// exp max. value: 709.78271289338401 = 0x40862E42FEFA39EF, infinity: 709.78271289338419 = 0x40862E42FEFA39F1
		if (xn > 0x40862E42FEFA39EFull) return Test_PInfD(); // exp=9
#endif
	}

	// min. value (x < 0), return +0.0
	else
	{
#if EXPD_MODE == 3 // exp10
		// exp10 min. value: -307.65265556858873 = 0xC0733A7146F72A41, zero: -307.65265556858878 = 0xC0733A7146F72A42
		if (xn > 0xC0733A7146F72A41ull) return Test_PZeroD(); // exp=8
#elif EXPD_MODE == 2 // exp2
		// exp2 min. value: -1022.0000000000000 = 0xC08FF00000000000, zero: -1022.0000000000001 = 0xC08FF00000000001
		if (xn > 0xC08FF00000000000ull) return Test_PZeroD(); // exp=9
#else // exp
		// exp min. value: -708.39641853226407 = 0xC086232BDD7ABCD2, zero: -708.39641853226424 = 0xC086232BDD7ABCD3
		if (xn > 0xC086232BDD7ABCD2ull) return Test_PZeroD(); // exp=9
#endif
	}

	// get signed exponent
	int exp = MyGetExpD(x) - DOUBLE_EXP1;

	// check minimal exponent - return 1.0
#if EXPD_MODE == 3 // exp10
	if (exp <= -56) return Test_P1D();
#else
	if (exp <= -54) return Test_P1D();
#endif

// exp is in range -53..+9 for exp and exp2, -55..+8 for exp10

	// get sign
	int sign = MyGetSignD(x);

	// normalize mantissa - shift to max position and restore hidden bit '1', mantissa format 1.63
	// - mantissa will be 0x8000000000000000..0xFFFFFFFFFFFFF800
	//    max exp10: (+)0x9A209A84FBCFF000 exp=8, (-)0x99D38A37B9520800 exp=8
	//    max exp2: (+)0xFFFFFFFFFFFFF800 exp=9, (-)0xFF80000000000000 exp=9
	//    max exp: (+)0xB17217F7D1CF7800 exp=9, (-)0xB1195EEBD5E69000 exp=9
	xn = (xn << 11) | 0x8000000000000000ull;

#if EXPD_MODE == 3 // exp10
	// multiply mantissa by ln(10) << 62 (= 10618799479599967255 round up) = mantissa/4
	xn = MyU64MulHighSimple(xn, 10618799479599967255ull + 2);
	exp += 2;
#endif

// max exp10: (+)0x58B90BFBE8E7B876 exp=10, min exp10: (-)0x588CAF75EAF34495 exp=10
// max exp2: (+)0xFFFFFFFFFFFFF800 exp=9, (-)0xFF80000000000000 exp=9
// max exp: (+)0xB17217F7D1CF7800 exp=9, (-)0xB1195EEBD5E69000 exp=9
// exp is in range -53..+9 for exp and exp2, -53..+10 for exp10

	// shift mantissa right by exponent bits
	// - get x as fixed point number in format 11.53 (current format is 1.63)
	xn >>= 63 - exp - 53; // exp is in range -53..+10, shift is in range 0..63

// exp10: max 0x58B90BFBE8E7B876
// exp2: max 0x7FFFFFFFFFFFFC00
// exp: max 0x58B90BFBE8E7BC00

#if EXPD_MODE == 2 // exp2
	// get integer part of exponent
	// - exponent will be 0..+1023
	exp = (int)(xn >> 53);

	// convert mantissa to range 0..1.0 ... number will be in 0.64 format
	u64 xx = xn << 11;
#else // exp, exp10

// exp10: max 0x58B90BFBE8E7B876 = 6393154322601326710
// exp: max 0x58B90BFBE8E7BC00 = 6393154322601327616

	// convert number to 2-base (multiply by 1/ln(2)), format 12.52
	// 1/ln(2) << 63 = 13306513097844322492 round up
	u64 m2 = MyU64MulHighSimple(xn, 13306513097844322492ull);

// exp10: 0 .. +0x3FFFFFFFFFFFFCD8 (= 4611686018427387096)
// exp: 0 .. +0x3FFFFFFFFFFFFF65 (= 4611686018427387749)

	// get integer part of exponent: shift >> 52, round to zero
	// - exponent will be 0..+1023
	exp = (int)(m2 >> 52);

	// convert exponent back to e-base: multiply by ln(2)
	//   ln(2) << 64 = 12786308645202655660 round up
	u64 d = (u64)exp * (12786308645202655660ull);

	// subtract exponent correction from the number
	xn = (xn << 11) - d;

	// convert to range 0..1.0: multiply by 1/ln(2) ... number will be in 0.64 format
	//   1/ln(2) << 63 = 13306513097844322492 round up
	u64 xx = MyU64MulHighSimple(xn, 13306513097844322492ull + 1);

	// overflow
	if ((s64)xx < 0) xx = (u64)-1;

	// convert to format 0.64
	xx <<= 1;

#endif

	// coeff[16]
	u64 acc;
	*(u32*)&acc = 1260;	// coeff[16] = 1.3661088453272162965e-16 << 63
	*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

#if 1		// set 1=use table, 0=long code

	// number is positive
	if (!sign)
	{
		*(u32*)&acc = 29109 + (u32)acc;	// coeff[15] = 3.1560349252155140648e-15 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 625212 + (u32)acc; // coeff[14] = 6.7785672188660199117e-14 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 12627788 + (u32)acc; // coeff[13] = 1.3691075408642918388e-12 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 236841782 + (u32)acc; // coeff[12] = 2.5678437471925562869e-11 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		acc = 4100285693 + (u32)acc; // coeff[11] = 4.4455386559996910448e-10 << 63
		acc = MyU64MulHighSimple(acc, xx);

		// table of constants
		u64* t = exp_fast_tab;
		int i = 10;

		for (; i > 0; i--)
		{
			acc = *t + acc;
			acc = MyU64MulHighSimple(acc, xx);
			t++;
		}

		acc = 9223372036854775808ull + acc + 6; // coeff[0] = 9223372036854775808 = 1.0 << 63

		// overflow
		if ((s32)(acc>>32) >= 0) acc = 0xFFFFFFFFFFFFFFFFull;
	}

	// number is negative
	else
	{
		// negate exponent
		exp = -exp;

		*(u32*)&acc = 29109 - (u32)acc; // coeff[15] = 3.1560349252155140648e-15 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 625212 - (u32)acc; // coeff[14] = 6.7785672188660199117e-14 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 12627788 - (u32)acc; // coeff[13] = 1.3691075408642918388e-12 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 236841782 - (u32)acc; // coeff[12] = 2.5678437471925562869e-11 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 4100285693 - (u32)acc; // coeff[11] = 4.4455386559996910448e-10 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		acc = 65070074559ull - (u32)acc; // coeff[10] = 7.0549116200190313717e-9 << 63
		acc = MyU64MulHighSimple(acc, xx);

		// table of constants
		u64* t = exp_fast_tab + 1;
		int i = 9;

		for (; i > 0; i--)
		{
			acc = *t - acc;
			acc = MyU64MulHighSimple(acc, xx);
			t++;
		}

		acc = 9223372036854775808ull - acc - 5; // coeff[0] = 9223372036854775808 = 1.0 << 63

		// mantissa correction
		acc <<= 1;
		exp--;
	}

#else

	// number is positive
	if (!sign)
	{
		*(u32*)&acc = 29109 + (u32)acc;	// coeff[15] = 3.1560349252155140648e-15 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 625212 + (u32)acc; // coeff[14] = 6.7785672188660199117e-14 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 12627788 + (u32)acc; // coeff[13] = 1.3691075408642918388e-12 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 236841782 + (u32)acc; // coeff[12] = 2.5678437471925562869e-11 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		acc = 4100285693 + (u32)acc; // coeff[11] = 4.4455386559996910448e-10 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 65070074559ull + acc; // coeff[10] = 7.0549116200190313717e-9 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 938762738676ull + acc; // coeff[9] = 1.0178086007207596430e-7 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 12189135131366ull + acc; // coeff[8] = 1.3215486790146656182e-6 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 140681638454011ull + acc; // coeff[7] = 1.5252733804065937795e-5 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 1420724914991586ull + acc; // coeff[6] = 1.5403530393381606220e-4 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 12298036735954521ull + acc; // coeff[5] = 1.3333558146428433721e-3 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 88711583058159475ull + acc; // coeff[4] = 9.6181291076284771650e-3 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 511935043789664227ull + acc; // coeff[3] = 5.5504108664821580022e-2 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 2215698446797868712ull + acc;// + 1; // coeff[2] = 2.4022650695910071233e-1 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 6393154322601327830ull + acc; // coeff[1] = 6.9314718055994530943e-1 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 9223372036854775808ull + acc + 6; // coeff[0] = 1.0 << 63

		// overflow
		if ((s32)(acc>>32) >= 0) acc = 0xFFFFFFFFFFFFFFFFull;
	}

	// number is negative
	else
	{
		// negate exponent
		exp = -exp;

		*(u32*)&acc = 29109 - (u32)acc; // coeff[15] = 3.1560349252155140648e-15 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 625212 - (u32)acc; // coeff[14] = 6.7785672188660199117e-14 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 12627788 - (u32)acc; // coeff[13] = 1.3691075408642918388e-12 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 236841782 - (u32)acc; // coeff[12] = 2.5678437471925562869e-11 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		*(u32*)&acc = 4100285693 - (u32)acc; // coeff[11] = 4.4455386559996910448e-10 << 63
		*(u32*)&acc = MyUMulHigh((u32)acc, (u32)(xx>>32));

		acc = 65070074559ull - (u32)acc; // coeff[10] = 7.0549116200190313717e-9 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 938762738676ull - acc; // coeff[9] = 1.0178086007207596430e-7 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 12189135131366ull - acc; // coeff[8] = 1.3215486790146656182e-6 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 140681638454011ull - acc; // coeff[7] = 1.5252733804065937795e-5 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 1420724914991586ull - acc;// - 1; // coeff[6] = 1.5403530393381606220e-4 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 12298036735954521ull - acc; // coeff[5] = 1.3333558146428433721e-3 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 88711583058159475ull - acc; // coeff[4] = 9.6181291076284771650e-3 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 511935043789664227ull - acc; // coeff[3] = 5.5504108664821580022e-2 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 2215698446797868712ull - acc; // coeff[2] = 2.4022650695910071233e-1 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 6393154322601327830ull - acc; // coeff[1] = 6.9314718055994530943e-1 << 63
		acc = MyU64MulHighSimple(acc, xx);

		acc = 9223372036854775808ull - acc - 5; // coeff[0] = 1.0 << 63

		// mantissa correction
		acc <<= 1;
		exp--;
	}

#endif

	// Rounding
	acc += 0x400 + 3;

	// overflow
	if ((s32)(acc>>32) >= 0)
	{
		acc = 0x8000000000000000ull;
		exp++; // it can become inf
	}

	// Normalize mantissa to 12.52
	acc >>= 11;

	// return number (use "exponent - 1" to clear hidden bit '1' of the mantissa)
	return Test_u64double(acc + (((s64)(exp + DOUBLE_EXP1 - 1)) << 52));
}

#endif // USE_RISCV_EXP == 1

// ----------------------------------------------------------------------------
//                   Natural logarithm - Chebyshev approximation
// ----------------------------------------------------------------------------

#if USE_RISCV_LOG == 1		// RISC-V log(), log2() and log10() function: 1=Chebyshev (recommended)

// 64-bit Chebyshev coefficients of Log() - see calculations in real_c.h
//const REALEXT REALNAME(ConstChebLn)[CHEB_LN] = {	// CHEB_LN = 27
//	 { { 0xfcc25f96, 0xcf991f65 }, 0x3ffffffd},	// 0  (0.405465108108164)
//	 { { 0xaaaaaaab, 0xaaaaaaaa }, 0x3ffffffd},	// 1  (0.333333333333333)
//	 { { 0x8e38e38f, 0xe38e38e3 }, 0xbffffffa},	// 2  (-0.0555555555555556)
//	 { { 0xb74f00e6, 0xca4587e6 }, 0x3ffffff8},	// 3  (0.0123456790123457)
//	 { { 0xb74f003f, 0xca4587e6 }, 0xbffffff6},	// 4  (-0.00308641975308642)
//	 { { 0x29f2f670, 0xd7c1a207 }, 0x3ffffff4},	// 5  (0.00082304526748973)
//	 { { 0xf5b98635, 0xefbab407 }, 0xbffffff2},	// 6  (-0.000228623685413815)
//	 { { 0x1c081632, 0x88fcf929 }, 0x3ffffff1},	// 7  (6.53210529750794e-005)
//	 { { 0xa061aed5, 0x9fd1cd5a }, 0xbfffffef},	// 8  (-1.90519737843892e-005)
//	 { { 0xdf6e9612, 0xbd6a6ea5 }, 0x3fffffed},	// 9  (5.64502927224592e-006)
//	 { { 0x2f3a3bbe, 0xe34c84c7 }, 0xbfffffeb},	// 10  (-1.69350878173468e-006)
//	 { { 0x9bcf0825, 0x89c1c48b }, 0x3fffffea},	// 11  (5.13184462837796e-007)
//	 { { 0xb96172a9, 0xa85e9ade }, 0xbfffffe8},	// 12  (-1.56806363378567e-007)
//	 { { 0x2f76ed31, 0xcf396e38 }, 0x3fffffe6},	// 13  (4.82481760125859e-008)
//	 { { 0x45d9b69d, 0x80482019 }, 0xbfffffe5},	// 14  (-1.4933960021896e-008)
//	 { { 0xe7c1c803, 0x9fa234af }, 0x3fffffe3},	// 15  (4.64594970811753e-009)
//	 { { 0x2fbb8f9a, 0xc78ab3c9 }, 0xbfffffe1},	// 16  (-1.45185772136676e-009)
//	 { { 0xb2ff6c1a, 0xfa946b80 }, 0x3fffffdf},	// 17  (4.55801937935576e-010)
//	 { { 0x34b28017, 0x9dc659b5 }, 0xbfffffde},	// 18  (-1.43495350433056e-010)
//	 { { 0x78d4e940, 0xc5800591 }, 0x3fffffdc},	// 19  (4.49063202187863e-011)
//	 { { 0x8f989cb9, 0xfa21b8d2 }, 0xbfffffda},	// 20  (-1.42183424951325e-011)
//	 { { 0x40a38e9f, 0xab63f671 }, 0x3fffffd9},	// 21  (4.87121039789384e-012)
//	 { { 0x6ee60d6c, 0xda4d119d }, 0xbfffffd7},	// 22  (-1.55112225309867e-012)
//	 { { 0x1d353341, 0xa3e41f7a }, 0x3fffffd5},	// 23  (2.91129085818348e-013)
//	 { { 0x565a5a95, 0xd07b628a }, 0xbfffffd3},	// 24  (-9.25845943719471e-014)
//	 { { 0x2c18d56d, 0xdb65a70e }, 0x3fffffd3},	// 25  (9.74318760393913e-014)
//	 { { 0xa13c8146, 0x8ced4e9c }, 0xbfffffd2},	// 26  (-3,12920757686657e-014)
//};

s64 log_fast_tab[18] = {
//	-288618,			// coeff[26] = -3.1292075768665710843e-014 << 63
//	 898650,			// coeff[25] =  9.7431876039391340245e-014 << 63
//	-853942,			// coeff[24] = -9.2584594371947089050e-014 << 63
//	 2685192,			// coeff[23] =  2.9112908581834786416e-013 << 63
//	-14306578,			// coeff[22] = -1.5511222530986678301e-012 << 63
//	 44928986,			// coeff[21] =  4.8712103978938373001e-012 << 63
//	-131141063,			// coeff[20] = -1.4218342495132474418e-011 << 63
//	 414187698,			// coeff[19] =  4.4906320218786305240e-011 << 63
//	-1323511003,			// coeff[18] = -1.4349535043305586936e-010 << 63

	 4204030849ll,			// coeff[17] =  4.5580193793557624035e-010 << 63
	-13391023909ll,			// coeff[16] = -1.4518577213667552372e-009 << 63
	 42851322622ll,			// coeff[15] =  4.6459497081175274052e-009 << 63
	-137741469265ll,		// coeff[14] = -1.4933960021895990116e-008 << 63
	 445010877464ll,		// coeff[13] =  4.8248176012585914163e-008 << 63
	-1446283427187ll,		// coeff[12] = -1.5680636337856674805e-007 << 63
	 4733291224286ll,		// coeff[11] =  5.1318446283779635237e-007 << 63
	-15619861541620ll,		// coeff[10] = -1.6935087817346793673e-006 << 63
	 52066205136860ll,		// coeff[9]  =  5.6450292722459159508e-006 << 63
	-175723442249826ll,		// coeff[8]  = -1.9051973784389208243e-005 << 63
	 602480373428256ll,		// coeff[7]  =  6.5321052975079351008e-005 << 63
	-2108681307008460ll,		// coeff[6]  = -2.2862368541381454891e-004 << 63
	 7591252705230431ll,		// coeff[5]  =  8.2304526748972956870e-004 << 63
	-28467197644613504ll,		// coeff[4]  = -3.0864197530864195951e-003 << 63
	 113868790578454018ll,		// coeff[3]  =  1.2345679012345678522e-002 << 63
	-512409557603043100ll,		// coeff[2]  = -5.5555555555555555558e-002 << 63
	 3074457345618258603ll,		// coeff[1]  =  3.3333333333333333334e-001 << 63

	 3739755540045141990ll-11,	// coeff[0]  =  4.0546510810816438199e-001 << 63
};

// log: 4250ns avg=0.074 max=1
// log2: 4330ns avg=0.001 max=1
// log10: 4420ns avg=0.035 max=1
double /*FASTCODE*/ NOFLASH(log_fast)(double x)
{
	// get number as integer
	u64 xn = Test_doubleu64(x);

	// result is 0 if number is 1
	if (xn == 0x3ff0000000000000ULL) return 0;

	// get unsigned exponent
	int exp = ((u32)(xn>>32) << 1) >> 21;

	// zero returns -inf (both positive and negative zero)
	if (exp == 0) return Test_MInfD();

	// negative number returns +inf
	if ((s32)(xn>>32) < 0) return Test_PInfD();

	// positive infinity returns +inf
	if (exp == 0x7ff) return x;

	// get signed exponent
	exp -= DOUBLE_EXP1;

	// get mantissa in 1.63 format signed, convert from range 1.0000..1.9999 to range -1.0000..0.9999
	//  s64 m = ((xn<<11)|B63);	// convert mantissa to 1.63 format, range 1.0000..1.9999
	//  m -= B63;			// subtract 1, convert to range 0.0000..0.9999
	//  m <<= 1;			// mantissa * 2, convert to range 0.0000..1.9999
	//  m -= B63;			// subtract 1, convert to range -1.0000..0.9999
	s64 m = (xn << 12) ^ (1ull << 63);

	s64 acc;
	s32 mH = (s32)(m >> 32);
	s32* pacc = (s32*)&acc;

	// coeff[26]
	*pacc = -577237;		// coeff[26] = -3.1292075768665710843e-014 << 64
	*pacc = MySMulHigh(*pacc, mH);

	// coeff[25]
	*pacc += 898650;		// coeff[25] =  9.7431876039391340245e-014 << 63
	*pacc = MySMulHigh(*pacc, mH) << 1;

	// coeff[24]
	*pacc -= 853942;		// coeff[24] = -9.2584594371947089050e-014 << 63
	*pacc = MySMulHigh(*pacc, mH) << 1;

	// coeff[23]
	*pacc += 2685192;		// coeff[23] =  2.9112908581834786416e-013 << 63
	*pacc = MySMulHigh(*pacc, mH) << 1;

	// coeff[22]
	*pacc -= 14306578;		// coeff[22] = -1.5511222530986678301e-012 << 63
	*pacc = MySMulHigh(*pacc, mH) << 1;

	// coeff[21]
	*pacc += 44928986;		// coeff[21] =  4.8712103978938373001e-012 << 63
	*pacc = MySMulHigh(*pacc, mH) << 1;

	// coeff[20]
	*pacc -= 131141063;		// coeff[20] = -1.4218342495132474418e-011 << 63
	*pacc = MySMulHigh(*pacc, mH) << 1;

	// coeff[19]
	*pacc += 414187698;		// coeff[19] =  4.4906320218786305240e-011 << 63
	*pacc = MySMulHigh(*pacc, mH) << 1;

	// coeff[18]
	*pacc -= 1323511003;		// coeff[18] = -1.4349535043305586936e-010 << 63
	acc = (s64)MySMulHigh(*pacc, mH) << 1;

	int i = 17;
	const s64* c = log_fast_tab;

	// add coefficients in case of positive mantissa
	if (m >= 0)
	{
		m <<= 1;
		for (; i > 0; i--)
		{
			acc += *c;
			c++;
			acc = MyS64MulHighSimple_Plus(acc, m);
		} 
		acc += *c;
	}

	// add coefficients in case of maximum negative mantissa
	else if (m == 0x8000000000000000ll)
	{
		for (; i > 0; i--)
		{
			acc += *c;
			c++;
			acc = -acc;
		} 
		acc += *c;
	}

	// add coefficients in case of negative mantissa
	else
	{
		m = ~m; // use faster ~ instead of -
		m <<= 1;
		for (; i > 0; i--)
		{
			acc += *c;
			c++;
			acc = MyS64MulHighSimple_Minus(acc, m);
		} 
		acc += *c;
	}

// result mantissa is in 1.63 format, value 0..0.6931 (log(2)) = 0x0000000000000002..0x58B90BFBE8E7B8C2
// should be max. 0x58B90BFBE8E7BCD5 = 6393154322601327829

	// It doesn't happen, but be careful
//	if (acc >= 0x58B90BFBE8E7BCD5) acc = 0x58B90BFBE8E7BCD5-1;
//	if (acc < 0) acc = 0;

// log10()
#if LOGD_MODE == 3	// log mode: 1=log, 2=log2, 3=log10

	// round mantissa up
	acc += 589 + 10; // = 0x100 * log(10) = 589

	// multiply 1.63 mantissa * 1/log(10) and convert to format 10.54
	// 1/log(10) << 55 = 15647107734948380
	acc = MyU64MulHigh(acc, 15647107734948380ll);

	// multiply exponent * log(10) and convert to format 10.54
	// log(2)/log(10) << 54 = 5422874305198591
	acc += exp*5422874305198591ll;

	// prepare result exponent base (-1 to clear hidden bit '1')
	exp = DOUBLE_EXP1 - 2 - 1;

// log2()
#elif LOGD_MODE == 2	// log mode: 1=log, 2=log2, 3=log10

	// round mantissa up
	acc += 354 - 6;	// = 0x200 * log(2) = 354

	// multiply 1.63 mantissa * 1/log(2) and convert to format 11.53
	// 1/log(2) << 54 = 25989283394227192
	acc = MyU64MulHigh(acc, 25989283394227192ull);

	// add exponent in format 11.53
	acc += ((u64)exp << 53);

	// prepare result exponent base (-1 to clear hidden bit '1')
	exp = DOUBLE_EXP1 - 1 - 1;

// log()
#else

	int exp2 = (exp < 0) ? -exp : exp;

	if (exp2 < 90) // max. 91
	{
		// round mantissa up
		acc += 0x20+30;

		// convert mantissa from format 1.63 to format 7.57
		acc >>= 6;

		// multiply exponent * log(2) and convert to format 7.57
		// log(2) << 57 = 99893036290645747
		acc += exp*99893036290645747ll;

		// prepare result exponent base (-1 to clear hidden bit '1')
		exp = DOUBLE_EXP1 - 5 - 1;
	}
	else if (exp2 < 368) // max. 369
	{
		// round mantissa up
		acc += 0x80+400;

		// convert mantissa from format 1.63 to format 9.55
		acc >>= 8;

		// multiply exponent * log(2) and convert to format 9.55
		// log(2) << 55 = 24973259072661437
		acc += exp*24973259072661437ll;

		// prepare result exponent base (-1 to clear hidden bit '1')
		exp = DOUBLE_EXP1 - 3 - 1;
	}
	else // exp2 >= 368
	{
		// round mantissa up
		acc += 0x200;

		// convert mantissa from format 1.63 to format 11.53
		acc >>= 10;

		// multiply exponent * log(2) and convert to format 11.53
		// log(2) << 53 = 6243314768165359
		acc += exp*6243314768165359ll;

		// prepare result exponent base (-1 to clear hidden bit '1')
		exp = DOUBLE_EXP1 - 1 - 1;
	}

#endif

	// zero result
	if (acc == 0) return 0;

	// negate mantissa
	int sign = 0;
	if (acc < 0)
	{
		acc = -acc;
		sign = 1;
	}

	// prepare number of shifts -> -52 (up) .. +11 (down)
	int d = 11;
	if ((acc>>32) == 0)
		d -= clz((u32)acc) + 32; // -52..-21
	else
		d -= clz((u32)(acc >> 32)); // -20..+11

	// prepare result exponent
	exp += d;

	// normalize mantissa
	if (d <= 0)
		// shift up 0..52
		acc <<= -d;
	else
	{
		// shift down with rounding 1..11
		acc >>= d-1;
		acc++;
		acc >>= 1;

		// carry on rounding
		if ((acc >> 53) != 0)
		{
			acc >>= 1;
			exp++;
		}
	}

	// compose result
	return Test_u64double((acc + ((u64)exp << 52)) | ((u64)sign<<63));
}

#endif // USE_RISCV_LOG == 1

// ----------------------------------------------------------------------------
//                       sin(x) - Chebyshev approximation
// ----------------------------------------------------------------------------

// Chebyshev coefficients of Sin()
//const REALEXT REALNAME(ConstChebSin)[CHEB_SIN] = {	// CHEB_SIN = 16
//	 { { 0xf9de6484, 0xb504f333 }, 0x3ffffffe},	// 0  (0.707106781186548)
//	 { { 0xeb17214f, 0x8e2c18d6 }, 0x3ffffffe},	// 1  (0.555360367269796)
//	 { { 0x9593108f, 0xdf52db03 }, 0xbffffffc},	// 2  (-0.218089506238715)
//	 { { 0x2fa3eec6, 0xe9dd2e0e }, 0xbffffffa},	// 3  (-0.0570956992187144)
//	 { { 0x4befe711, 0xb7ad24d9 }, 0x3ffffff8},	// 4  (0.0112107143260678)
//	 { { 0x10705d7f, 0xe6d0831c }, 0x3ffffff5},	// 5  (0.00176097488841343)
//	 { { 0x9a4913a7, 0xf1b55860 }, 0xbffffff2},	// 6  (-0.000230511073857694)
//	 { { 0x53cacfd5, 0xd8f4f9a6 }, 0xbfffffef},	// 7  (-2.586328200726e-005)
//	 { { 0x32974d7f, 0xaa65c7a6 }, 0x3fffffec},	// 8  (2.53912177168092e-006)
//	 { { 0xd56dbbf3, 0xedeb7ea9 }, 0x3fffffe8},	// 9  (2.21580175198894e-007)
//	 { { 0x2fd2defd, 0x957d4c94 }, 0xbfffffe5},	// 10  (-1.74028624024465e-008)
//	 { { 0x593ca3f5, 0xaac6bf9f }, 0xbfffffe1},	// 11  (-1.2425615672446e-009)
//	 { { 0xe1def889, 0xb2d398db }, 0x3fffffdd},	// 12  (8.13209004226994e-011)
//	 { { 0xaf3f74d8, 0xacdd270f }, 0x3fffffd9},	// 13  (4.91308689099828e-012)
//	 { { 0xd48caa73, 0x9994cee8 }, 0xbfffffd5},	// 14  (-2.72815161019069e-013)
//	 { { 0xf8a402c2, 0x80d1e123 }, 0xbfffffd1},	// 15  (-1.43018754196858e-014)
//};

s64 sin_fast_tab[12] = {
//	-65956,			// coeff[15] = -1.4301875419685838441e-14 << 62
//	-1258138,		// coeff[14] = -2.7281516101906876443e-13 << 62
//	22657614,		// coeff[13] =  4.9130868909982775974e-12 << 62
//	375026459,		// coeff[12] =  8.1320900422699374499e-11 << 62
	-5730303807,		// coeff[11] = -1.2425615672446040571e-9 << 62
	-80256537222,		// coeff[10] = -1.7402862402446451959e-8 << 62
	1021858195925,		// coeff[9]  =  2.2158017519889359529e-7 << 62
	11709632373545,		// coeff[8]  =  2.5391217716809155258e-6 << 62
	-119273336023525,	// coeff[7]  = -2.5863282007259964339e-5 << 62
	-1063044696402212,	// coeff[6]  = -2.3051107385769441652e-4 << 62
	8121063271697932,	// coeff[5]  =  1.7609748884134271780e-3 << 62
	51700294514110439,	// coeff[4]  =  1.1210714326067788982e-2 << 62
	-263307437799280571,	// coeff[3]  = -5.7095699218714364755e-2 << 62
	-1005760326686814473,	// coeff[2]  = -2.1808950623871498270e-1 << 62
	2561147640926816340,	// coeff[1]  =  5.5536036726979578088e-1 << 62
	3260954456333195553,	// coeff[0]  =  7.0710678118654752438e-1 << 62
};

// Chebyshev approximation of sin() mantissa
//  m = -1..+1 in format 1.63 (= angle 0..pi/2)
double /*FASTCODE*/ NOFLASH(_sin_cheb)(s64 m, u32 sign)
{
	s64 acc;
	s32 mH = (s32)(m >> 32);
	s32* pacc = (s32*)&acc;

	*pacc = -131912;		// coeff[15] = -1,4301875419685838441e-14 << 63
	*pacc = MySMulHigh(*pacc, mH);

	*pacc -= 1258138;		// coeff[14] = -2.7281516101906876443e-13 << 62
	*pacc = MySMulHigh(*pacc, mH) << 1;

	*pacc += 22657614;		// coeff[13] =  4.9130868909982775974e-12 << 62
	*pacc = MySMulHigh(*pacc, mH) << 1;

	*pacc += 375026459;		// coeff[12] =  8.1320900422699374499e-11 << 62
	acc = (s64)MySMulHigh(*pacc, mH) << 1;

	const s64* t = sin_fast_tab;
	int i;
	acc += *t;

	if (m >= 0)
	{
		m <<= 1;
		for (i = 11; i > 0; i--)
		{
			t++;
			acc = MyS64MulHighSimple_Plus(acc, m);
			acc += *t;
		}
	}
	else
	{
		m <<= 1;
		m = ~m; // use faster NOT instead of NEG
		for (i = 11; i > 0; i--)
		{
			t++;
			acc = MyS64MulHighSimple_Minus(acc, m);
			acc += *t;
		}
	}

// result mantissa is in 2.62 format, value 0..+1

	// zero result
	if (acc <= 0) return 0;

	// limit to max. value 1.0
	if (acc >= 0x4000000000000000ll) acc = 0x3FFFFFFFFFFFFFFFll;

	// prepare number of shifts -> -52 (up) .. +11 (down)
	int d = 11;
	if ((acc>>32) == 0)
		d -= clz((u32)acc) + 32; // -52..-21
	else
		d -= clz((u32)(acc >> 32)); // -20..+11

	// prepare result exponent (subtract 1 to clear hidden bit '1')
	int exp = d + DOUBLE_EXP1-10 - 1;

	// normalize mantissa
	if (d <= 0)
		// shift up 0..52
		acc <<= -d;
	else
	{
		// shift down with rounding 1..11
		acc >>= d-1;
		acc++;
		acc >>= 1;

		// carry on rounding
		if ((acc >> 53) != 0)
		{
			acc >>= 1;
			exp++;
		}
	}

	// compose result
	return Test_u64double((acc + ((u64)exp << 52)) | ((u64)sign<<63));
}

// sine-cosine in radians - Chebyshev approximation
void /*FASTCODE*/ NOFLASH(_sincos_fast)(double x, double* psin, double* pcos)
{
	// get number as integer
	u64 xn = Test_doubleu64(x);

	// get unsigned exponent
	int exp = ((u32)(xn>>32) << 1) >> 21;

	// normalize big angle
	if (exp >= DOUBLE_EXP1+7) // normalize angles >= 128.0 (= 20*(2*pi))
	{
		// normalize using remainder, modulo 2*pi (remainder returns angle in range -pi .. +pi)
		x = remainder(x, Test_u64double(0x401921FB54442D18ull));
		xn = Test_doubleu64(x);
		exp = ((u32)(xn>>32) << 1) >> 21;
	}

	// get signed exponent
	// - signed exponent here is max. +6
	exp -= DOUBLE_EXP1;

	// unpack mantissa to format 12.52
	s64 m = (xn & 0x000fffffffffffffull) | (1ull << 52);

	// negate mantissa
	if ((s32)(xn>>32) < 0) m = -m;

	// prepare number of shifts, to convert mantissa to signed format 8.56
	int s = exp + (56 - 52); // exps <= +6, number of shifts <= +10

	// convert to signed fixed point 8.56
	if (s < 0)
	{
		// right shift
		s = -s;
		m >>= s;
	}
	else
		// left shift, number of shifts = 0..10
		m <<= s;

	// 2*pi in 8.56 fixed point
	u64 pi = 0x06487ED5110B4612ull;	// = 2*pi << 56 (= 452751216129820178)

	// limit max. angle to modulo 2*PI
	// - mantissa will be in range 0 .. 0x06487ED5110B4611 = 452751216129820177
	do m -= pi; while (m >= 0); // limit positive angle
	do m += pi; while (m < 0); // result angle will be in range 0 (include) .. 2*PI (exclude)

	// convert angle to unsigned format 3.61
	// - mantissa will be in range 0 .. 0xC90FDAA22168C220 = 14488038916154245664
	xn = m << 5;

// ---- mantissa here is in unsigned fixed point format 3.61, range 0..0xC90FDAA22168C220 = 14488038916154245664 (= 0..2*pi)

	s32 sin_sign = 0;
	s32 cos_sign = 0;

	// convert to range 0..pi
	// - mantissa will be in range = 0 .. 0x6487ED5110B4611F (= 0..pi)
	pi = 0x6487ED5110B46120ull; // pi <<= 4; // 2*pi * 16 = 0x6487ED5110B46120 = 7244019458077122848
	if (xn >= pi) // x >= PI ?
	{
		xn -= pi;
		sin_sign = ~sin_sign;
		cos_sign = ~cos_sign;
	}

	// convert to range 0..pi/2
	//  mantissa will be in range = 0 .. 0x3243F6A8885A3090 = 3622009729038561424 (= 0..pi/2)
	u64 pi2 = 0x3243F6A8885A3090ull; // u64 pi2 = pi >> 1; // pi/2 = 0x3243F6A8885A3090 = 3622009729038561424
	if (xn >= pi2)
	{
		xn = pi - xn;
		cos_sign = ~cos_sign;
	}

	// convert range 0..pi/2 in unsigned fixed point format 3.61 to range -1..+1 in signed fixed point format 1.63
	//  k = (0x8000000000000000 / 0x3243F6A8885A3090 << 64) >> 2 = (9223372036854775808 / 3622009729038561424 << 64) >> 2 = 11743562013128004897
	//  11743562013128004897+2 = 0xA2F9836E4E441523
	m = MyU64MulHighSimple(xn, 11743562013128004897ull+2); // max. value 2305843009213693952 = 0x2000000000000000
	if (m >= 0x2000000000000000ull) m = 0x1fffffffffffffffull; // limit range
	m <<= 3; // value 0 .. 0xFFFFFFFFFFFFFFF8
	m -= 0x7FFFFFFFFFFFFFFCll; // subtract 1 to get range -1..+1 in signed format 1.63, range -0x7FFFFFFFFFFFFFFC..+0x7FFFFFFFFFFFFFFC

	// Chebyshev approximation
	if (psin != NULL) *psin = _sin_cheb(m, sin_sign);
	if (pcos != NULL) *pcos = _sin_cheb(-m, cos_sign);
}

// sine-cosine in degrees - Chebyshev approximation
void /*FASTCODE*/ NOFLASH(_sincos_deg_fast)(double x, double* psin, double* pcos)
{
	// get number as integer
	u64 xn = Test_doubleu64(x);

	// normalize big angle >= 360
	if ((xn << 1) >= (0x4076800000000000ull << 1)) // absolute value of angle >= 360 deg
	{
		// normalize using remainder, modulo 360 (remainder returns angle in range -180 .. +180)
		x = remainder(x, Test_u64double(0x4076800000000000ull)); // 360: exps=+8
		xn = Test_doubleu64(x);
	}

	// absolute value of the angle
	// - angle will be in range 0..360
	s32 sin_sign = 0;
	if ((s32)(xn >> 32) < 0)
	{
		sin_sign = ~sin_sign;
		xn &= ~(1ull << 63);
	}

	// get signed exponent
	// - signed exponent here is max. +8
	int exp = ((u32)(xn>>32) >> 20) - DOUBLE_EXP1;

	// unpack mantissa to unsigned format 12.52, set hidden bit '1'
	xn = (xn & 0x000fffffffffffffull) | (1ull << 52);

	// prepare number of shifts to convert mantissa to format 9.55
	int s = exp + (55 - 52); // exps <= +8, number of shifts <= +11

	// convert mantissa to unsigned fixed point format 9.55 (rounding down)
	// - mantissa will be in range = 0 .. 0xB400000000000000
	if (s < 0)
	{
		// right shift
		s = -s;
		xn >>= s;
	}
	else
		// left shift, number of shifts = 0..11
		xn <<= s;

	// 180 in 9.55 fixed point
	u64 a = 0x5A00000000000000ull;	// = 180 << 55 (= 6485183463413514240)

	// convert to range 0..180
	// - mantissa will be in range = 0 .. 0x5A00000000000000 (= 0..180)
	s32 cos_sign = 0;
	if (xn >= a) // x >= 180 ?
	{
		xn -= a;
		sin_sign = ~sin_sign;
		cos_sign = ~cos_sign;
	}

	// convert to range 0..90
	//  mantissa will be in range = 0 .. 0x2D00000000000000 = 3242591731706757120 (= 0..90)
	u64 a2 = 0x2D00000000000000ull; // a >> 1; // 90deg = 0x2D00000000000000 = 3242591731706757120
	if (xn >= a2)
	{
		xn = a - xn;
		cos_sign = ~cos_sign;
	}

	// convert to format 7.57
	// mantissa = 0 .. 0xB400000000000000 = 12970366926827028480
	xn <<= 2;

	// convert range 0..90 in unsigned fixed point format 7.57 to range -1..+1 in signed fixed point format 1.63
	//  k = (0x8000000000000000 / 0xB400000000000000 << 64) = (9223372036854775808 / 12970366926827028480 << 64) = 13117684674637903372 round up
	xn = MyU64MulHigh(xn, 13117684674637903372ull-2); // max. value 9223372036854775807 = 0x7FFFFFFFFFFFFFFF
	if (xn >= 0x8000000000000000ull) xn = 0x7fffffffffffffffull; // limit range
	xn <<= 1;
	s64 m = xn - 0x7FFFFFFFFFFFFFFFll; // subtract 1 to get range -1..+1 in signed format 1.63, range -0x7FFFFFFFFFFFFFFF..+0x7FFFFFFFFFFFFFFF

	// Chebyshev approximation
	if (psin != NULL) *psin = _sin_cheb(m, sin_sign);
	if (pcos != NULL) *pcos = _sin_cheb(-m, cos_sign);
}

// sin() of small angle (exps < -6)
// - Simplified Taylor serie is used: sin(x) = x/1! - x^3/3! + x^5/5! - ...
double NOFLASH(small_sin_fast)(double x)
{
	// get number as integer
	u64 xn = Test_doubleu64(x);

	// get signed exponent
	int exp = ((u32)((u32)(xn >> 32) << 1) >> 21) - DOUBLE_EXP1;

	// use 1 member
	if (exp < -26) return x;

	// get sign
	u32 sign = (u32)(xn >> 63);

	// unpack mantissa to format 2.62, add hidden bit '1'
	// - mantissa will be in range 0x4000000000000000 .. 0x7FFFFFFFFFFFFFFF
	xn = ((xn << 11) | (1ull << 63)) >> 1;

	// add 2nd member x^3/3!
	// 1/3! = 0.1666666666666666667 << 64 = 3074457345618258603
	u64 xn2 = MyU64MulHighSimple(xn, xn); // x^2
	u64 xn3 = MyU64MulHighSimple(xn2, xn); // x^3
	xn3 = MyU64MulHighSimple(xn3, 3074457345618258603ull);
	int d = -exp*2 - 2*2;
	u64 m = xn3 >> d;
	xn -= m;

	// add 3rd member x^5/5!
	if (exp >= -13)
	{
		// 1/(4*5) = 0.05 << 64 = 922337203685477581
		u64 xn5 = MyU64MulHighSimple(xn3, xn2);
		xn5 = MyU64MulHighSimple(xn5, 922337203685477581ull);
		d = -exp*4 - 4*2;
		m = xn5 >> d;
		xn += m;
	}

	// normalize mantissa
	if ((xn >> 62) == 0) //	if (xn < 0x4000000000000000ull)
	{
		xn >>= 8;
		exp--;
	}
	else
		xn >>= 9;

	xn++; // rounding
	xn >>= 1;

	// carry on rounding
	if ((xn >> 53) != 0)
	{
		xn >>= 1;
		exp++;
	}

	// compose result ('exponent - 1' to compensate hidden bit '1')
	exp += DOUBLE_EXP1-1;
	return Test_u64double((xn + ((u64)exp << 52)) | ((u64)sign << 63));
}

double /*FASTCODE*/ NOFLASH(sin_fast)(double x)
{
	// hack for small angles
	// - Simplified Taylor serie is used
	//   sin(x) = x/1! - x^3/3! + x^5/5! - ...
	u64 xn = Test_doubleu64(x);
	int exp = ((u32)(xn>>32) << 1) >> 21;
	if (exp < DOUBLE_EXP1-6)
	{
#if 1  // set 1=use faster version, 0=use slower version
		return small_sin_fast(x);
#else
		double res = x;
		if (exp >= DOUBLE_EXP1-26)
		{
			double xx = x*x;
			double x3 = x*xx*0.1666666666666666667; // x^3/3!
			res -= x3;
			if (exp >= DOUBLE_EXP1-12)
			{
				double x5 = x3*xx*0.05; // x^5/5!
				res += x5;
			}
		}
		return res;
#endif

	}

	double res_sin;
	_sincos_fast(x, &res_sin, NULL);
	return res_sin;
}

double /*FASTCODE*/ NOFLASH(cos_fast)(double x)
{
	// hack for small angles
	// - Simplified Taylor serie is used
	//   cos(x) = 1/1! - x^2/2! + ...
	u64 xn = Test_doubleu64(x);
	int exp = ((u32)(xn>>32) << 1) >> 21;
	if (exp < DOUBLE_EXP1-14)
	{
		double res = 1.0;
		if (exp >= DOUBLE_EXP1-27)
		{
			res -= x*x*0.5; // x^2/2!
		}
		return res;
	}

	double res_cos;
	_sincos_fast(x, NULL, &res_cos);
	return res_cos;
}

// sine-cosine in radians - Chebyshev approximation
void /*FASTCODE*/ NOFLASH(sincos_fast)(double x, double* psin, double* pcos)
{
	// hack for small angles
	// - Simplified Taylor series are used
	//   sin(x) = x/1! - x^3/3! + x^5/5! - ...
	//   cos(x) = 1/1! - x^2/2! + ...
	u64 xn = Test_doubleu64(x);
	int exp = ((u32)(xn>>32) << 1) >> 21;
	if (exp < DOUBLE_EXP1-6)
	{
		double res;

		// calculate sin()
#if 1  // set 1=use faster version, 0=use slower version
		*psin = small_sin_fast(x);
#else
		res = x;
		if (exp >= DOUBLE_EXP1-26)
		{
			double xx = x*x;
			double x3 = x*xx*0.1666666666666666667; // x^3/3!
			res -= x3;
			if (exp >= DOUBLE_EXP1-12)
			{
				double x5 = x3*xx*0.05; // x^5/5!
				res += x5;
			}
		}
		*psin = res;
#endif

		// calculate cos()
		if (exp < DOUBLE_EXP1-14)
		{
			res = 1.0;
			if (exp >= DOUBLE_EXP1-27)
			{
				res -= x*x*0.5; // x^2/2!
			}
			*pcos = res;
		}
		else
		{
			_sincos_fast(x, NULL, pcos);
		}
		return;
	}

	_sincos_fast(x, psin, pcos);
}

double /*FASTCODE*/ NOFLASH(tan_fast)(double x)
{
	double res_sin, res_cos;
	sincos_fast(x, &res_sin, &res_cos);
	return res_sin/res_cos;
}

double /*FASTCODE*/ NOFLASH(cotan_fast)(double x)
{
	double res_sin, res_cos;
	sincos_fast(x, &res_sin, &res_cos);
	return res_cos/res_sin;
}

double /*FASTCODE*/ NOFLASH(sin_deg_fast)(double x)
{
	// hack for small angles
	// - Simplified Taylor series are used
	//   sin(x) = x/1! - x^3/3! + x^5/5! - ...
	u64 xn = Test_doubleu64(x);
	int exp = ((u32)(xn>>32) << 1) >> 21;
	if (exp < DOUBLE_EXP1-2)
	{
		x *= 0.01745329251994329577; // convert deg to rad (x * pi/180)
#if 1  // set 1=use faster version, 0=use slower version
		return small_sin_fast(x);
#else
		exp -= 6; // so we can later user common code for sin()
		double res = x;
		if (exp >= DOUBLE_EXP1-26)
		{
			double xx = x*x;
			double x3 = x*xx*0.1666666666666666667; // x^3/3!
			res -= x3;
			if (exp >= DOUBLE_EXP1-12)
			{
				double x5 = x3*xx*0.05; // x^5/5!
				res += x5;
			}
		}
		return res;
#endif
	}

	double res_sin;
	_sincos_deg_fast(x, &res_sin, NULL);
	return res_sin;
}

double /*FASTCODE*/ NOFLASH(cos_deg_fast)(double x)
{
	// hack for small angles
	// - Simplified Taylor series are used
	//   cos(x) = 1/1! - x^2/2! + ...
	u64 xn = Test_doubleu64(x);
	int exp = ((u32)(xn>>32) << 1) >> 21;
	if (exp < DOUBLE_EXP1-10)
	{
		double res = 1.0;
		if (exp >= DOUBLE_EXP1-21)
		{
			res -= x*x*1.523087098933542997e-4; // x^2/2! and convert deg to rad = (pi/180)^2/2 = 1.523087098933542997e-4
		}
		return res;
	}

	double res_cos;
	_sincos_deg_fast(x, NULL, &res_cos);
	return res_cos;
}

// sine-cosine in degrees - Chebyshev approximation
void /*FASTCODE*/ NOFLASH(sincos_deg_fast)(double x, double* psin, double* pcos)
{
	// hack for small angles
	// - Simplified Taylor series are used
	//   sin(x) = x/1! - x^3/3! + x^5/5! - ...
	//   cos(x) = 1/1! - x^2/2! + ...
	u64 xn = Test_doubleu64(x);
	int exp = ((u32)(xn>>32) << 1) >> 21;
	if (exp < DOUBLE_EXP1-2)
	{
		// convert deg to rad (x * pi/180)
		double xd = x * 0.01745329251994329577;
		double res;

		// calculate sin()
#if 1  // set 1=use faster version, 0=use slower version
		*psin = small_sin_fast(xd);
#else
		res = xd;
		if (exp >= DOUBLE_EXP1-20)
		{
			double xx = xd*xd;
			double x3 = xd*xx*0.1666666666666666667; // x^3/3!
			res -= x3;
			if (exp >= DOUBLE_EXP1-6)
			{
				double x5 = x3*xx*0.05; // x^5/5!
				res += x5;
			}
		}
		*psin = res;
#endif

		// calculate cos()
		if (exp < DOUBLE_EXP1-10)
		{
			res = 1.0;
			if (exp >= DOUBLE_EXP1-21)
			{
				res -= xd*xd*0.5; // x^2/2!
			}
			*pcos = res;
		}
		else
		{
			_sincos_deg_fast(x, NULL, pcos);
		}
		return;
	}

	_sincos_deg_fast(x, psin, pcos);
}

double /*FASTCODE*/ NOFLASH(tan_deg_fast)(double x)
{
	double res_sin, res_cos;
	sincos_deg_fast(x, &res_sin, &res_cos);
	return res_sin/res_cos;
}

double /*FASTCODE*/ NOFLASH(cotan_deg_fast)(double x)
{
	double res_sin, res_cos;
	sincos_deg_fast(x, &res_sin, &res_cos);
	return res_cos/res_sin;
}

// ----------------------------------------------------------------------------
//                       atan(x) - Chebyshev approximation
// ----------------------------------------------------------------------------

// Chebyshev coefficients of ATan()
//const REALEXT REALNAME(ConstChebATan)[CHEB_ATAN] = {	// CHEB_ATAN = 24
//	 { { 0xea065bc8, 0xded3d42d }, 0x3ffffffe},	// 0  (0.870419751367103)
//	 { { 0xfd6ec476, 0xd0a4a60c }, 0xbffffffb},	// 1  (-0.101876542350218)
//	 { { 0xdbda910f, 0xaad1805f }, 0x3ffffff9},	// 2  (0.0208518512071081)
//	 { { 0x18e51c74, 0xa4d9f172 }, 0xbffffff7},	// 3  (-0.00503086366024444)
//	 { { 0xe8639fe6, 0xac6fbd41 }, 0x3ffffff5},	// 4  (0.00131558594962724)
//	 { { 0x243b8587, 0xbd423212 }, 0xbffffff3},	// 5  (-0.000360982087174936)
//	 { { 0x14a4c682, 0xd67d4f88 }, 0x3ffffff1},	// 6  (0.000102276561167419)
//	 { { 0xc0814523, 0xf8b8f99d }, 0xbfffffef},	// 7  (-2.9650039534715e-005)
//	 { { 0xfb97dfdd, 0x92b73946 }, 0x3fffffee},	// 8  (8.74493823837521e-006)
//	 { { 0x9d26aa46, 0xaf6d896d }, 0xbfffffec},	// 9  (-2.61407909151312e-006)
//	 { { 0xe2406ed8, 0xd407380a }, 0x3fffffea},	// 10  (7.89866592328669e-007)
//	 { { 0x24957900, 0x814467f8 }, 0xbfffffe9},	// 11  (-2.40778944354026e-007)
//	 { { 0x5820c477, 0x9ec88877 }, 0x3fffffe7},	// 12  (7.3939250925921e-008)
//	 { { 0xc654dd01, 0xc442ea46 }, 0xbfffffe5},	// 13  (-2.28478325618365e-008)
//	 { { 0x0ae616b3, 0xf3f4696d }, 0x3fffffe3},	// 14  (7.10001720791611e-009)
//	 { { 0x7e043dcc, 0x9848c33c }, 0xbfffffe2},	// 15  (-2.21602719203637e-009)
//	 { { 0xd1587926, 0xbe0bced8 }, 0x3fffffe0},	// 16  (6.91383775061303e-010)
//	 { { 0xd9ea5717, 0xef118ddf }, 0xbfffffde},	// 17  (-2.1743159875935e-010)
//	 { { 0x7e36387b, 0x9df6ab23 }, 0x3fffffdd},	// 18  (7.1833505384565e-011)
//	 { { 0x8279ad34, 0xc80cfea0 }, 0xbfffffdb},	// 19  (-2.27431383222601e-011)
//	 { { 0xaee4e54c, 0xb28648b4 }, 0x3fffffd9},	// 20  (5.07397279825367e-012)
//	 { { 0x2b93c313, 0xe25501e9 }, 0xbfffffd7},	// 21  (-1.6081860139168e-012)
//	 { { 0xac632856, 0xbae84598 }, 0x3fffffd7},	// 22  (1.3280563277113e-012)
//	 { { 0x2d97d849, 0xef38b305 }, 0xbfffffd5},	// 23  (-4.24942715028253e-013)
//};

s64 atan_fast_tab[17] = {
//	-1959702,		// coeff[23] = -4.2494271502825301290e-013 << 62
//	6124579,		// coeff[22] =  1.3280563277112973164e-012 << 62
//	-7416449,		// coeff[21] = -1.6081860139168029037e-012 << 62
//	23399569,		// coeff[20] =  5.0739727982536712427e-012 << 62
//	-104884213,		// coeff[19] = -2.2743138322260129705e-011 << 62
//	331273572,		// coeff[18] =  7.1833505384564991396e-011 << 62
//	-1002726264,		// coeff[17] = -2.1743159875934996148e-010 << 62
	3188444889ll,		// coeff[16] =  6.9138377506130279270e-010 << 62
	-10219621618ll,		// coeff[15] = -2.2160271920363650519e-009 << 62
	32743050088ll,		// coeff[14] =  7.1000172079161104595e-009 << 62
	-105367029977ll,	// coeff[13] = -2.2847832561836509202e-008 << 62
	340984609708ll,		// coeff[12] =  7.3939250925920962694e-008 << 62
	-1110396891209ll,	// coeff[11] = -2.4077894435402600012e-007 << 62
	3642616720265ll,	// coeff[10] =  7.8986659232866866394e-007 << 62
	-12055311997394ll,	// coeff[9]  = -2.6140790915131184429e-006 << 62
	40328909405926ll,	// coeff[8]  =  8.7449382383752058361e-006 << 62
	-136736672768065ll,	// coeff[7]  = -2.9650039534715038092e-005 << 62
	471667387148618ll,	// coeff[6]  =  1.0227656116741852865e-004 << 62
	-1664736044327388ll,	// coeff[5]  = -3.6098208717493585339e-004 << 62
	6067069329935476ll,	// coeff[4]  =  1.3155859496272433493e-003 << 62
	-23200763602563726ll,	// coeff[3]  = -5.0308636602444420689e-003 << 62
	96162190670148898ll,	// coeff[2]  =  2.0851851207108148171e-002 << 62
	-469822625962227236ll,	// coeff[1]  = -1.0187654235021826540e-001 << 62
	4014102597542713074ll,	// coeff[0]  =  8.7041975136710319745e-001 << 62
};

// Chebyshev approximation of atan()
double /*FASTCODE*/ NOFLASH(_atan_fast)(double x, Bool deg, Bool useacotan)
{
	// get number as integer
	u64 xn = Test_doubleu64(x);

	// get sign
	u32 sign = (u32)(xn >> 63);

	// absolute value
	xn &= ~(1ull << 63);

	// get signed exponent
	int exp = ((u32)((u32)(xn >> 32) << 1) >> 21) - DOUBLE_EXP1;

// acotan(x) = pi/2 - atan(x)

	// infinity or big value
	if (exp > 23)
	{
		// acotan()
		if (useacotan)
		{
			// positive
			if (sign == 0)
			{
				// positive infinity - return x
				if (deg)
					return rad2deg(drec(x));
				else
					return drec(x);
			}

			// negative
			else
			{
				double res;
				if (deg)
					// 180 deg
					res = Test_u64double(0x4066800000000000ull);
				else
					// pi rad
					res = Test_u64double(0x400921FB54442D18ull);

				if (exp < 52)
				{
					if (deg)
						// 180 deg
						res += rad2deg(drec(x));
					else
						// pi rad
						res += drec(x);
				}
				return res;
			}
		}

		// atan()
		else
		{
			double res;
			if (deg)
				// +-90 deg
				res = Test_u64double(0x4056800000000000ull | ((u64)sign << 63));
			else
				// +-pi/2 rad
				res = Test_u64double(0x3FF921FB54442D18ull | ((u64)sign << 63));

			if (exp < 52)
			{
				if (deg)
					// +90deg - x, -90deg + x
					res -= rad2deg(drec(x));
				else
					// +pi/2rad - x, -pi/2rad + x
					res -= drec(x);
			}
			return res;
		}
	}

	if (exp < -20)
	{
		exp += DOUBLE_EXP1;
		if (deg && (exp != 0)) x = rad2deg(x);

		// acotan()
		if (useacotan)
		{
			double res;
			if (deg)
				// +90 deg
				res = Test_u64double(0x4056800000000000ull);
			else
				// +pi/2 rad
				res = Test_u64double(0x3FF921FB54442D18ull);

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
	s64 resadd = 0; // result addition = 0
	if (exp >= 0)
	{
		// use -1/x
		x = -drec(x);

		// result addition = pi/2
		resadd = 7244019458077122842ll-3; // pi/2 in format 2.62

		// is x is positive, use -pi/2
		if (!Test_IsNegD(x)) resadd = -7244019458077122842ll+60;

		// get number as integer
		xn = Test_doubleu64(x);
		sign = (u32)(xn >> 63);	// get sign
		xn &= ~(1ull << 63);	// absolute value
		exp = ((u32)((u32)(xn >> 32) << 1) >> 21) - DOUBLE_EXP1; // get signed exponent
	}

// argument x is in range 0..+1 here, exponent is -23..0

	// normalize mantissa - shift to max position 1.63 and restore hidden bit '1'
	xn = (xn << 11) | (1ull << 63);

	// shift mantissa right by exponent bits, to get signed fixed number in format 2.62
	xn >>= -exp;

	// calculate 2*x^2 - 1, signed fixed point format 2.62
	//   y = x*fnc(2*x^2 - 1)
	s64 m = MyU64MulHighSimple(xn, xn); // get x*x
	m <<= 1;
	m = m - 0x4000000000000000ll; // -1

// argument x is in range -1..+1 here, signed fixed point format 2.62

	s64 acc;
	s32 mH = (s32)(m >> 32);
	s32* pacc = (s32*)&acc;

	// coeff[23]
	*pacc = -3919405 - 45;		// coeff[23] = -4,2494271502825301290e-013 << 63
	*pacc = MySMulHigh(*pacc, mH) << 1;

	*pacc += 6124579 + 5;		// coeff[22] =  1.3280563277112973164e-012 << 62
	*pacc = MySMulHigh(*pacc, mH) << 2;

	*pacc -= 7416449;		// coeff[21] = -1.6081860139168029037e-012 << 62
	*pacc = MySMulHigh(*pacc, mH) << 2;

	*pacc += 23399569;		// coeff[20] =  5.0739727982536712427e-012 << 62
	*pacc = MySMulHigh(*pacc, mH) << 2;

	*pacc -= 104884213;		// coeff[19] = -2.2743138322260129705e-011 << 62
	*pacc = MySMulHigh(*pacc, mH) << 2;

	*pacc += 331273572;		// coeff[18] =  7.1833505384564991396e-011 << 62
	*pacc = MySMulHigh(*pacc, mH) << 2;

	*pacc -= 1002726264;		// coeff[17] = -2.1743159875934996148e-010 << 62
	acc = (s64)(MySMulHigh(*pacc, mH) << 2);

	const s64* t = atan_fast_tab;
	int i;
	acc += *t;
	if (m >= 0)
	{
		for (i = 16; i > 0; i--)
		{
			t++;
			acc = MyS64MulHighSimple_Plus(acc, m) << 2;
			acc += *t;
		}
	}
	else
	{
		m <<= 2;
		m = ~m; // use faster NOT instead of NEG
		for (i = 16; i > 0; i--)
		{
			t++;
			acc = MyS64MulHighSimple_Minus(acc, m);
			acc += *t;
		}
	}

// result is -pi/2..+pi/2 in 2.62 format

	// multiply by original x
	acc = MyS64MulHighSimple_Plus(acc, xn) << 1;
	if (sign) acc = -acc;

	// add alternative correction
	acc += resadd;

	// acotan() correction: acotan(x) = pi/2 - atan(x)
	exp = DOUBLE_EXP1 + 2;

	if (useacotan)
	{
		if (acc >= 0)
			acc = 7244019458077122842 - acc; // pi/2 in format 2.62
		else
		{
                        // overflow
			acc = 3622009729038561421 - acc/2 - 35; // pi/2 in format 3.61
			exp++;
		}
	}

	// zero result
	if (acc == 0) return 0;

	// absolute value
	sign = (u32)(acc >> 63);
	if (acc < 0) acc = -acc;

	// convert to degrees: x = x * (180/PI)
	if (deg)
	{
		// x = x * (180/PI) ... (180/PI << 64) >> 6 = 16514384080961256899
		acc = MyU64MulHighSimple(acc, 16514384080961256899ull - 30);
		if (acc == 0) return 0;
		exp += 6;
	}

	// normalize mantissa to max. position 1.63
	int d;
	if ((acc>>32) == 0)
		d = clz((u32)acc) + 32;
	else
		d = clz((u32)(acc >> 32));
	acc <<= d;

	// prepare exponent
	exp -= d;

	// rounding mantissa
#define ATAN_ROUND_CORR	(0x400 + 125)
	acc += ATAN_ROUND_CORR;
	if ((u64)acc >= ATAN_ROUND_CORR) // no overflow
	{
		acc <<= 1; 	// remove leading bit '1'
		exp--;		// compensate exponent
	}

	xn = (u64)acc >> 12;

	// compose result
	return Test_u64double(xn | ((u64)exp << 52) | ((u64)sign << 63));
}

// arc tangent in radians (result is -pi/2 .. +pi/2)
double atan_fast(double x)
{
	return _atan_fast(x, False, False);
}

// arc tangent in degrees (result is -90 .. +90)
double atan_deg_fast(double x)
{
	return _atan_fast(x, True, False);
}

// arc cotangent in radians (result is 0 .. pi)
double acotan_fast(double x)
{
	return _atan_fast(x, False, True);
}

// arc cotangent in degrees (result is 0 .. 180)
double acotan_deg_fast(double x)
{
	return _atan_fast(x, True, True);
}

// ----------------------------------------------------------------------------
//                   asinf(x)... - Chebyshev approximation
// ----------------------------------------------------------------------------
// arc sine/cosine

// Chebyshev coefficients of ASin()
s64 asin_fast_tab[38] = {
	1600650139 + 4,			// coeff[37] =  3.4708567164600090860e-010 << 62
	2759261959ll,			// coeff[36] =  5.9831956207860754844e-010 << 62
	-10445857682ll,			// coeff[35] = -2.2650843184651493206e-009 << 62
	-17993535216ll,			// coeff[34] = -3.9017259944777314656e-009 << 62
	35508701216ll,			// coeff[33] =  7.6997222001122337242e-009 << 62
	61168092309ll,			// coeff[32] =  1.3263715713699009926e-008 << 62
	-72271961288ll,			// coeff[31] = -1.5671483487560937465e-008 << 62
	-124428254541ll,		// coeff[30] = -2.6981076778459631638e-008 << 62
	109701354368ll,			// coeff[29] =  2.3787689346113323827e-008 << 62
	189021811450ll,			// coeff[28] =  4.0987571724329514300e-008 << 62
	-101243475479ll,		// coeff[27] = -2.1953679212795056195e-008 << 62
	-173862762129ll,		// coeff[26] = -3.7700476882996859937e-008 << 62
	121804774659ll,			// coeff[25] =  2.6412200260974878764e-008 << 62
	211420807249ll,			// coeff[24] =  4.5844579705514679850e-008 << 62
	52598053833ll,			// coeff[23] =  1.1405384846815019950e-008 << 62
	96992050855ll,			// coeff[22] =  2.1031798450064501118e-008 << 62
	360478019538ll,			// coeff[21] =  7.8166210383357868318e-008 << 62
	645112091838ll,			// coeff[20] =  1.3988638629347848066e-007 << 62
	1078693614567ll,		// coeff[19] =  2.3390439207198458051e-007 << 62
	1952148490506ll,		// coeff[18] =  4.2330472688418869549e-007 << 62
	3576895238877ll,		// coeff[17] =  7.7561551774871925733e-007 << 62
	6535955991008ll,		// coeff[16] =  1.4172595369440573676e-006 << 62
	12008321148263ll,		// coeff[15] =  2.6038895753700331244e-006 << 62
	22227149726572ll,		// coeff[14] =  4.8197448043419372559e-006 << 62
	41473969186008ll,		// coeff[13] =  8.9932334986133469979e-006 << 62
	78078853301927ll,		// coeff[12] =  1.6930652475025236884e-005 << 62
	148667137339566ll,		// coeff[11] =  3.2237046656151586703e-005 << 62
	286414117158771ll,		// coeff[10] =  6.2106161610811407942e-005 << 62
	561958929814205ll,		// coeff[ 9] =  1.2185541851043802362e-004 << 62
	1117938344771234ll,		// coeff[ 8] =  2.4241423642116406379e-004 << 62
	2308353625532723ll,		// coeff[ 7] =  5.0054440313347368686e-004 << 62
	4792310124788473ll,		// coeff[ 6] =  1.0391666097039881692e-003 << 62
	11047398492786767ll,		// coeff[ 5] =  2.3955226892384999515e-003 << 62
	23841078585890576ll,		// coeff[ 4] =  5.1697098394440400625e-003 << 62
	75854154795318419ll,		// coeff[ 3] =  1.6448247884227194745e-002 << 62
	152633360258872429ll,		// coeff[ 2] =  3.3097084157286428629e-002 << 62
	1865518847608440861ll,		// coeff[ 1] =  4.0451991747794525175e-001 << 62
	1772717230470124293ll,		// coeff[ 0] =  3.8439677449563908304e-001 << 62
};

// arc sine/cosine (result is -pi/2 .. +pi/2)
double NOFLASH(_asincos_fast)(double x, Bool deg, Bool useacos)
{
	// get number as integer
	u64 xn = Test_doubleu64(x);

	// get sign
	u32 sign = (u32)(xn >> 63);

	// absolute value
	xn &= ~(1ull << 63);

	// get signed exponent
	int exp = ((u32)(xn >> 32) >> 20) - DOUBLE_EXP1;

	// invalid input value > 1.0 (case 1.0 has been serviced before)
	if (exp >= 0) return Test_QNanD();

	// if > 0.70710678118654755 (= sqrt(0.5)), use alternate method sqrt(1 - x^2)
	Bool alt = False;
	if (xn > 0x3FE6A09E667F3BCD) // sqrt(0.5)
	{
		// use alternate method
		alt = True;

		// x = sqrt(1 - x^2)
		x = sqrt(1 - dsqr(x));

		// reload exponent
		xn = Test_doubleu64(x);
		exp = ((u32)(xn >> 32) >> 20) - DOUBLE_EXP1;
	}

// x here is 0..0.7071 (= sqrt(0.5)), exponent is <= -1

	// normalize mantissa - shift to max position 1.63 and restore hidden bit '1'
	// xn at max: 0xB504F333F9DE6800
	xn = (xn << 11) | (1ull << 63);

// Consistency checks:
//  (no alt) x = 0.70710678118654746 0x3FE6A09E667F3BCC -> xn = 0xB504F333F9DE6000
//  (no alt) x = 0.70710678118654755 0x3FE6A09E667F3BCD -> xn = 0xB504F333F9DE6800
//  (alt)    x = 0.70710678118654772 0x3FE6A09E667F3BCE -> xn = 0xB504F333F9DE5800
//  (alt)    x = 0.70710678118654781 0x3FE6A09E667F3BCF -> xn = 0xB504F333F9DE5000

	// shift mantissa right by exponent bits, to get fixed number in format 1.63
	//  mantissa will be in range 0..0xB504F333F9DE6800 = 0..13043817825332783104
	xn >>= -exp - 1; // number of shifts >= 0

	// convert to range 0..1 in format 2.62
	// - table of Chebyshev constants are prepared for multiply by 8/3 = 2.66666.. << 62 = 12297829382473034410 = 0xAAAAAAAAAAAAAAAB
	//   (otherwise should multiply by 1/sqrt(0.5) = sqrt(2) = 1.4142135623730950488 << 63 = 13043817825332782212 = 0xB504F333F9DE6484)
	//  mantissa will be in range 0..8695878550221855401 = 0..0x78ADF777FBE99AA9
	xn = MyU64MulHighSimple(xn, 0xAAAAAAAAAAAAAAAB);

	// convert to range -1..+1 in signed format 2.62
        // - offset is theoretically 0x4000000000000000, empirically refined to 0x3ffffffffffffffe
	s64 m = xn - 0x3ffffffffffffffell; // m will be in range -1..+1 in signed format 2.62, m = -0x3ffffffffffffffe..+0x38ADF777FBE99AAB

	// prescale from format 2.62 to format 0.64
	s32 msign = (s32)(m >> 32);
	m <<= 2;

	s64 acc;
	const s64* t = asin_fast_tab;
	int i;
	acc = *t;

	if (msign >= 0)
	{
		for (i = 37; i > 24; i--) // use i > 25 or less
		{
			t++;
			acc = MyS64MulHighSimple_Plus(acc, m);
			acc += *t;
		}

		for (; i > 0; i--)
		{
			t++;
			acc = MyU64MulHighSimple(acc, m);
			acc += *t;
		}
	}
	else
	{
		m = ~m; // use faster NOT instead of NEG
		for (i = 37; i > 0; i--)
		{
			t++;
			acc = MyS64MulHighSimple_Minus(acc, m);
			acc += *t;
		}
	}

// Checks limits and consistency of approximation:
//   (-) m = -0x3ffffffffffffffe -> acc = 0xFFFFFFFFFFFFFFFC
//   (-) m = -0x3ffffffffffffff5 -> acc = 0x0000000000000000
//   (-) m = -0x2000000000000000 -> acc = 0x0C124A76873C7DD8
//   (-) m = -0x0000000000000001 -> acc = 0x1899F4EDC962D304
//   (+) m = +0x0000000000000000 -> acc = 0x1899F4EDC962D305
//   (+) m = +0x2000000000000000 -> acc = 0x263BE81F7A639A04
//   (+) m = +0x38ADF777FBE99AAB -> acc = 0x3243F6A8885A31BE

	// check underflow
	if (acc < 0) acc = 0;

// acc value is in range 0..pi/2 in 3.61 format (= pi/2 << 61 = 0..0x3243F6A8885A308D, practically 0..0x3243F6A8885A31BF)

	// prepare exponent
	exp = DOUBLE_EXP1 + 2;

	// correction for alternate method
	if (alt)
	{
		// x = pi/2 - x
		// pi/2 << 62 = 0x6487ED5110B4611A = 7244019458077122842
		acc = 0x6487ED5110B4611All + 1 - acc;
	}

// Consistency checks:
//  (no alt) x = 0.70710678118654746 0x3FE6A09E667F3BCC -> acc = 0x3243F6A8885A2EEA
//  (no alt) x = 0.70710678118654755 0x3FE6A09E667F3BCD -> acc = 0x3243F6A8885A31BE
//  (alt)    x = 0.70710678118654772 0x3FE6A09E667F3BCE -> acc = 0x3243F6A8885A3504
//  (alt)    x = 0.70710678118654781 0x3FE6A09E667F3BCF -> acc = 0x3243F6A8885A37DB

	// acos
	if (useacos)
	{
		// acos(x) = pi/2 - asin(x)
		if (!sign) acc = ~acc; // use faster NOT instead of NEG
		sign = False;
		acc += 0x6487ED5110B4611All;
	}

	// convert to degrees: x = x * (180/PI)
	if (deg)
	{
		// x = x * (180/PI)
		// ... (180/PI << 64) >> 6 = 16514384080961256899
		acc = MyU64MulHighSimple(acc, 16514384080961256899ull + 14);
		exp += 6;
	}

	// zero result
	if (acc == 0) return 0;

	// normalize mantissa to max. position 1.63
	int d;
	if ((acc>>32) == 0)
		d = clz((u32)acc) + 32;
	else
		d = clz((u32)(acc >> 32));
	acc <<= d;

	// prepare exponent
	exp -= d;

	// rounding mantissa
#define ASIN_ROUND_CORR	0x400
	acc += ASIN_ROUND_CORR;
	if ((u64)acc >= ASIN_ROUND_CORR) // no overflow
	{
		acc <<= 1; 	// remove leading bit '1'
		exp--;		// compensate exponent
	}

	xn = (u64)acc >> 12;

	// compose result
	return Test_u64double(xn | ((u64)exp << 52) | ((u64)sign << 63));
}

// arc sine in radians (result is -pi/2 .. +pi/2)
double NOFLASH(asin_fast)(double x)
{
	// get number as integer
	u64 xn = Test_doubleu64(x);

	// get sign
	u32 sign = (u32)(xn >> 63);

	// absolute value
	xn &= ~(1ull << 63);

	// asin() +-1 returns +-pi/2 rad
	if (xn == 0x3ff0000000000000ULL) return Test_u64double(0x3FF921FB54442D18ull | ((u64)sign << 63));

	// get unsigned exponent
	int exp = (u32)(xn >> 32) >> 20;

	// hack for small angles
	// - Simplified Taylor serie is used
	//  asin(x) = x + x^3/2/3 + x^5*3/2/4/5 + x^7*3*5/2/4/6/7 + ...
	if (exp < DOUBLE_EXP1-7)
	{
		double res = x;
		if (exp >= DOUBLE_EXP1-26)
		{
			double xx = x*x;
			double x3 = x*xx;
			res += x3*0.1666666666666666667; // x^3/2/3

			if (exp >= DOUBLE_EXP1-13)
			{
				double x5 = x3*xx;
				res += x5*0.075; // x^5*3/2/4/5

				if (exp >= DOUBLE_EXP1-9)
				{
					double x7 = x5*xx;
					res += x7*0.04464285714285714286; // x^7*3*5/2/4/6/7
				}
			}
		}
		return res;
	}
	return _asincos_fast(x, False, False);
}

// arc sine in degrees (result is -90 .. +90)
double NOFLASH(asin_deg_fast)(double x)
{
	// get number as integer
	u64 xn = Test_doubleu64(x);

	// get sign
	u32 sign = (u32)(xn >> 63);

	// absolute value
	xn &= ~(1ull << 63);

	// asin() +-1 returns +-90 deg
	if (xn == 0x3ff0000000000000ULL) return Test_u64double(0x4056800000000000ull | ((u64)sign << 63));

	// get unsigned exponent
	int exp = (u32)(xn >> 32) >> 20;

	// hack for small angles
	// - Simplified Taylor serie is used
	//  asin(x) = x + x^3/2/3 + x^5*3/2/4/5 + x^7*3*5/2/4/6/7 + ...
	if (exp < DOUBLE_EXP1-7)
	{
		double res = x;
		if (exp >= DOUBLE_EXP1-26)
		{
			double xx = x*x;
			double x3 = x*xx;
			res += x3*0.1666666666666666667; // x^3/2/3

			if (exp >= DOUBLE_EXP1-13)
			{
				double x5 = x3*xx;
				res += x5*0.075; // x^5*2/2/4/5

				if (exp >= DOUBLE_EXP1-9)
				{
					double x7 = x5*xx;
					res += x7*0.04464285714285714286; // x^7*3*5/2/4/6/7
				}
			}
		}
		return res * 57.295779513082320877; // 180/PI
	}
	return _asincos_fast(x, True, False);
}

// arc cosine in radians
double NOFLASH(acos_fast)(double x)
{
	// get number as integer
	u64 xn = Test_doubleu64(x);

	// get sign
	u32 sign = (u32)(xn >> 63);

	// absolute value
	xn &= ~(1ull << 63);

	// acos() +1 returns 0
	// acos() -1 returns pi
	if (xn == 0x3ff0000000000000ULL)
	{
		if (sign)
			// pi rad
			return Test_u64double(0x400921FB54442D18ull);
		else
			return 0;
	}

	// get unsigned exponent
	int exp = (u32)(xn >> 32) >> 20;

	// zero or small value - acos(0) returns +pi/2 rad
	if (exp < DOUBLE_EXP1-54) return Test_u64double(0x3FF921FB54442D18ull);
	return _asincos_fast(x, False, True);
}

// arc cosine in degrees
double NOFLASH(acos_deg_fast)(double x)
{
	// get number as integer
	u64 xn = Test_doubleu64(x);

	// get sign
	u32 sign = (u32)(xn >> 63);

	// absolute value
	xn &= ~(1ull << 63);

	// acos() +1 returns 0
	// acos() -1 returns 180
	if (xn == 0x3ff0000000000000ULL)
	{
		if (sign)
			// 180 deg
			return Test_u64double(0x4066800000000000ull);
		else
			return 0;
	}

	// get unsigned exponent
	int exp = (u32)(xn >> 32) >> 20;

	// zero or small value - acos(0) returns +90 deg
	if (exp < DOUBLE_EXP1-53) return Test_u64double(0x4056800000000000ull);
	return _asincos_fast(x, True, True);
}
