
// ****************************************************************************
//
//                                     Float
//                              Single-floating-point
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/

/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../../global.h"	// globals

#if USE_FLOAT		// use float support 1=in RAM, 2=in Flash

#include "../inc/sdk_float.h"
#include "../inc/sdk_double.h"
#include "../sdk_sftable.h"
#include "../inc/sdk_bootrom.h"

#if USE_FLOAT == 1
#define MYPREFIX(x) FASTCODE NOFLASH(x)
#else
#define MYPREFIX(x) FASTCODE x
#endif

#if USE_FLOATLIBC
#undef WRAPPER_FUNC
#define WRAPPER_FUNC(x) x
#endif

#if RP2040 && !USE_FLOATLIBC
// table of ROM single-floating-point functions (size 336 bytes)
u32 SFTable[SF_TABLE_SIZE/4];

// initialize floating-point service
void FloatInit()
{
	// find single-floating-point function table
	void* rom_table = RomData(ROM_TABLE_CODE('S', 'F'));

	// copy version 1 table into RAM
	memcpy(SFTable, rom_table, SF_TABLE_SIZE);
}
#endif // RP2040

// constants
#define FPINF		( HUGE_VALF)		// plus infinity
#define FMINF		(-HUGE_VALF)		// minus infinity
#define NANF		((float)NAN)		// infinity, NaN
#define PZERO		(+0.0)			// plus zero
#define MZERO		(-0.0)			// minus zero
#define PIf		3.14159265358979323846f	// pi
#define LOG2f		0.69314718055994530941f	// log(2)
#define LOG10f		2.30258509299404568401f	// log(10)
#define LOG2Ef		1.44269504088896340737f	// 1/log(2)
#define LOG10Ef		0.43429448190325182765f	// 1/log(10)
#define ONETHIRDf	0.33333333333333333333f	// 1/3

#define LOG2     0.69314718055994530941	// log(2)
#define LOG10    2.30258509299404568401	// log(10)

// unpack float to exponent and mantissa
#define FUNPACK(x,e,m) e=((x) >> 23) & 0xff, m=((x) & 0x007fffff) | 0x00800000

// unpack float to sign, exponent and mantissa
#define FUNPACKS(x,s,e,m) s=((x) >> 31), FUNPACK((x),(e),(m))

// union to take number as float or as u32
typedef union {
	float	f;	// float
	u32	ix;	// u32
} float_u32;

// get number as float type
INLINE float u322float(u32 ix)
{
	float_u32 tmp;
	tmp.ix = ix;	// set number as u32
	return tmp.f;	// return as float
}

// get number as u32 type
INLINE u32 float2u32(float f)
{
	float_u32 tmp;
	tmp.f = f;	// set number as float
	return tmp.ix;	// return as u32
}

// get sign and exponent
INLINE int fgetsignexp(float x)
{
	u32 ix = float2u32(x);	// get number as u32
	return (ix >> 23) & 0x1ff; // mask exponent with sign
}

// get exponent
INLINE int fgetexp(float x)
{
	u32 ix = float2u32(x);	// get number as u32
	return (ix >> 23) & 0xff; // mask exponent
}

// checks
INLINE int fiszero(float x)  { return fgetexp    (x)==0; }	// check if number is zero
INLINE int fispzero(float x) { return fgetsignexp(x)==0; }	// check if number is plus zero
INLINE int fismzero(float x) { return fgetsignexp(x)==0x100; }	// check if number is minus zero
INLINE int fisinf(float x)   { return fgetexp    (x)==0xff; }	// check if number is infinity
INLINE int fispinf(float x)  { return fgetsignexp(x)==0xff; }	// check if number is plus infinity
INLINE int fisminf(float x)  { return fgetsignexp(x)==0x1ff; }	// check if number is minus infinity
INLINE Bool fisnan(float x)  { return (float2u32(x)<<1)>0xff000000; } // check if number is NaN

// check if number is negative but not zero (strict negative)
INLINE Bool fisstrictneg(float x)
{
	u32 ix = float2u32(x);	// get number as u32
	if (fiszero(x)) return False; // number must not be zero
	return (ix >> 31) != 0;	// check sign bit
}

// check if number is negative (can be negative zero)
INLINE Bool fisneg(float x)
{
	u32 ix = float2u32(x);	// get number as u32
	return (ix >> 31) != 0; // check sign bit
}

// negate number
INLINE float fneg(float x)
{
	u32 ix = float2u32(x);	// get number as u32
	ix ^= 0x80000000;	// invert sign bit
	return u322float(ix);	// return number as float
}

// ==== common auxiliary functions, not in libc

// compose floating point with magnitude of 'num' and sign of 'sign'.
float MYPREFIX(WRAPPER_FUNC(copysignf))(float num, float sign)
{
	u32 ix = float2u32(num); // get number 'x' as u32
	u32 iy = float2u32(sign); // get number 'sign' as u32
	ix=((ix & 0x7fffffff) | (iy & 0x80000000)); // compose number
	return u322float(ix);	// return number as float
}

// check if number is an integer
Bool MYPREFIX(isintf)(float x)
{
	// get number as u32
	u32 ix = float2u32(x);

	// 0 is integer
	int e = fgetexp(x);	// get exponent
	if (e == 0) return True; // 0 is an integer

	// number abs(x) < 1 is not integer
	e -= 0x7f;		// remove exponent bias (= exponent of 1)
	if (e < 0) return False; // asb(x) < 1, non-integer

	// check max exponent (mantissa is 23 bits long)
	e = 23 - e;		// remaining bits up to 23 bits
	if (e <= 0) return True; // very large number is always integer (no fractional part left)

	// check bits of fractional part - it must be 0
	u32 m = (1 << e) - 1; 	// mask for bits of significance <1
	if ((ix & m) != 0) return False; // number is not integer if fractional bits are not 0

	// otherwise number is an interger
	return True;
}

// check if number is odd integer (must be an integer ..., -3, -1, 1, 3, 5,..)
Bool MYPREFIX(isoddintf)(float x)
{
	// get number as u32
	u32 ix = float2u32(x);

	// get absolute integer
	int e = fgetexp(x);	// get exponent
	e -= 0x7f;		// remove exponent bias (= exponent of 1)

	// number abs(x) < 1 (including 0) is not odd
	if (e < 0) return False; // abs(x) < 1 or 0, number is not odd

	// check max exponent (mantissa is 23 bits long)
	e = 23 - e;		// remaining bits up to 23 bits
	if (e < 0) return False; // large number is not odd

	// check bits of fractional part - it must be 0
	u32 m = (1 << e) - 1;	// mask for bits of significance <1
	if ((ix & m) != 0) return False; // number is not integer if fractional bits are not 0

	// value 1 is odd
	if (e == 23) return True; // value is exactly 1, odd integer

	// check mantissa, if number is odd
	return ((ix >> e) & 1) != 0; // check bit 0, it must be '1'
}

// check if number is power of 2
Bool MYPREFIX(ispow2f)(float x)
{
	u32 ix = float2u32(x);		// get number as u32
	if (fiszero(x)) return False;	// zero is not power of 2
	if (fisinf(x)) return False;	// infinity is not power of 2
	ix &= 0x007fffff;		// mask mantissa
	return ix == 0;			// mantissa must be zero to be a power of two
}

// round to given number of significant digits (digits<=0 to use default number of digits)
// @TODO: probably will be deleted (accuracy cannot be guaranteed)
/*
float MYPREFIX(rounddigf)(float x, int digits)
{
	// invalid values
	if (fiszero(x) || fisinf(x)) return x;

	// absolute value
	Bool neg = fisneg(x);
	if (neg) x = -x;

	// get scale of the number
	if (digits < 1) digits = 6;
	float scale = exp10f(digits - 1 - (int)floorf(log10f(x)));

	// round number
	x = roundf(x * scale) / scale;

	// negate
	if (neg) x = -x;
	return x;
}
*/
#if USE_FLOATLIBC || RISCV
// Check if comparison is unordered (either input is NaN)
Bool MYPREFIX(fcmpun)(float x, float y)
{
	return fisnan(x) || fisnan(y);
}
#endif

#if USE_FLOATLIBC || !RISCV || (USE_RISCV_SINF==0) || (USE_RISCV_SINF==3)
// sine in degrees
float MYPREFIX(sinf_deg)(float x) { return sinf(deg2radf(x)); }

// cosine in degrees
float MYPREFIX(cosf_deg)(float x) { return cosf(deg2radf(x)); }

// sine-cosine in degrees
void MYPREFIX(sincosf_deg)(float x, float* psin, float* pcos) { sincosf(deg2radf(x), psin, pcos); }

// tangent in degrees
float MYPREFIX(tanf_deg)(float x) { return tanf(deg2radf(x)); }

// cotangent in degrees
float MYPREFIX(cotanf_deg)(float x) { return cotanf(deg2radf(x)); }
#endif

#if USE_FLOATLIBC || !RISCV || (USE_RISCV_SINF==0)
// cotangent
float MYPREFIX(cotanf)(float x) { return frec(tanf(x)); }
#endif

#if USE_FLOATLIBC || !RISCV || (USE_RISCV_ASINF!=2)
// arc sine in degrees
float MYPREFIX(asinf_deg)(float x) { return rad2degf(asinf(x)); }


// arc cosine in degrees
float MYPREFIX(acosf_deg)(float x) { return rad2degf(acosf(x)); }
#endif

#if USE_FLOATLIBC || !RISCV || (USE_RISCV_ATANF!=1)
// arc cotangent in radians
float acotanf(float x)
{
	if (fgetexp(x) >= 127+20) return fisneg(x) ? PIf : frec(x);
	return (float)(PI/2) - atanf(x);
}

// arc cotangent in degrees
float acotanf_deg(float x)
{
	if (fgetexp(x) >= 127+20) return fisneg(x) ? 180 : rad2degf(frec(x));
	return 90.0f - atanf_deg(x);
}

// arc tangent in degrees
float MYPREFIX(atanf_deg)(float x) { return rad2degf(atanf(x)); }
#endif

// arc tangent of y/x in degrees
float MYPREFIX(atan2f_deg)(float y, float x) { return rad2degf(atan2f(y, x)); }

// ----------------------------------------------------------------------------
//                 Common functions - faster alternative to libc
// ----------------------------------------------------------------------------

#if !USE_FLOATLIBC

// ==== auxiliary functions

// multiply number by power of 2 (num * 2^exp)
float MYPREFIX(WRAPPER_FUNC(ldexpf))(float num, int de)
{
	// get number as u32
	u32 iy;
	u32 ix = float2u32(num);	// get number as u32

	// get exponent
	int e = fgetexp(num); // get exponent
	if ((e == 0) || (e == 0xff)) return num; // number is 0 or infinity NaN, return number without changes

	// shift exponent
	e += de;

	// check result
	if (e <= 0) // underflow?
		iy = ix & 0x80000000; // underflow - result will be signed zero (+0 or -0)
	else if (e >= 0xff) // overflow?
		iy = (ix & 0x80000000) | 0x7f800000; // overflow - result will be signed infinity NaN
	else
		iy = ix + ((u32)de << 23); // shift exponent if result is OK

	// get number as float
	return u322float(iy);
}

// absolute value
float MYPREFIX(absf)(float x)
{
	u32 ix=float2u32(x);	// get number as u32
	ix &= 0x7fffffff;	// absolute value
	return u322float(ix);	// return number as float
}

// ==== rounding

// round number towards zero
float MYPREFIX(WRAPPER_FUNC(truncf))(float x)
{
	// get number as u32
	u32 ix = float2u32(x);

	// get absolute exponent
	int e = fgetexp(x);	// get exponent
	e -= 0x7f;		// remove exponent bias (= exponent of 1)

	// truncate number abs(x) < 1 to zero
	if (e < 0)		// number is abs(x) < 1
	{
		ix &= 0x80000000; // set to zero, left only sign bit
		return u322float(ix); // return float 0 or -0
	}

	// check max exponent (mantissa is 23 bits long)
	e = 23 - e;		// remaining bits up to 23 bits
	if (e <= 0) return x;	// very large number is always integer (no fractional part left)

	// clear fractional part
	u32 m = (1 << e) - 1;	// prepare mask for bits of significance <1
	ix &= ~m;		// clear fractional bits

	return u322float(ix);	// return number as float
}

// round number to nearest integer
float MYPREFIX(WRAPPER_FUNC(roundf))(float x)
{
	// get number as u32
	u32 ix = float2u32(x);

	// get absolute exponent
	int e = fgetexp(x);	// get exponent
	e -= 0x7f;		// remove exponent bias (= exponent of 1)

	// truncate number abs(x) < 0.5 to zero
	if (e < -1)		// number if abs(x) < 0.5
	{
		ix &= 0x80000000; // set to zero, left only sign bit
		return u322float(ix); // return float 0 or -0
	}

	// round number (abs(x) >= 0.5) && (abs(x) < 1) to +-1
	if (e == -1)		// number 0.5..1 (+-)
	{
		ix &= 0x80000000; // set to zero, left only sign bit
		ix |= 0x3f800000; // add exponent bias 127, set number to -1 or +1
		return u322float(ix); // return float -1 or +1
	}

	// check max exponent (mantissa is 23 bits long)
	e = 23 - e;		// remaining bits up to 23 bits
	if (e <= 0) return x;	// very large number is always integer (no fractional part left)

	// add rounding 0.5
	u32 m = 1 << (e - 1);	// prepare mask for bit of significance 0.5
	ix += m;		// add rounding correction

	// clear fractional part
	m = m + m - 1;		// prepare mask for bits of significance <1
	ix &= ~m;		// clear fractional bits

	return u322float(ix);	// return number as float
}

// round number down to integer
float MYPREFIX(WRAPPER_FUNC(floorf))(float x)
{
	// get number as u32
	u32 ix = float2u32(x);

	// get exponent
	int e = fgetexp(x);

	// zero - return +0 or -0
	if (e == 0)		// number is zero
	{
		ix &= 0x80000000; // set to zero, left only sign bit
		return u322float(ix); // return float +0 or -0
	}

	// get absolute exponent
	e -= 0x7f;		// remove exponent bias (= exponent of 1)

	// number is -1 < x < +1, return -1 or 0
	if (e < 0)		// number is abs(x) < 1, but not zero
	{
		if (fisneg(x)) return -1.0; // number is -1 < x < 0, return -1
		return PZERO;	// return +0.0
	}

	// check max exponent (mantissa is 23 bits long)
	e = 23 - e;		// remaining bits up to 23 bits
	if (e <= 0) return x;	// very large number is always integer (no fractional part left)

	// prepare ask of fractional part
	u32 m = (1 << e) - 1;	// prepare mask for bits of significance <1

	// if the number is negative, round the number up
	if (fisneg(x)) ix += m;	// add mask to round number up

	// truncate - clear fractional bits
	ix &= ~m;

	return u322float(ix);	// return number as float
}

// round number up to integer
float MYPREFIX(WRAPPER_FUNC(ceilf))(float x)
{
	// get number as u32
	u32 ix = float2u32(x);

	// get exponent
	int e = fgetexp(x);

	// zero - return +0 or -0
	if (e == 0)		// number is zero
	{
		ix &= 0x80000000; // set to zero, left only sign bit
		return u322float(ix); // return float +0 or -0
	}

	// get absolute exponent
	e -= 0x7f;		// remove exponent bias (= exponent of 1)

	// number is -1 < x < +1, return -0 or +1
	if (e < 0)		// number is abs(x) < 1, but not zero
	{
		if (fisneg(x)) return MZERO; // number is -1 < x < 0, return -0.0
		return 1.0;	// return +1
	}

	// check max exponent (mantissa is 23 bits long)
	e = 23 - e;		// remaining bits up to 23 bits
	if (e <= 0) return x;	// very large number is always integer (no fractional part left)

	// prepare ask of fractional part
	u32 m = (1 << e) - 1;	// prepare mask for bits of significance <1

	// if the number is not negative, round the number up
	if (!fisneg(x)) ix += m; // add mask to round number up

	// truncate - clear fractional bits
	ix &= ~m;

	return u322float(ix);	// return number as float
}

// ==== scientific functions

#if !RISCV || (USE_RISCV_ASINF!=2)
// arc sine
float MYPREFIX(WRAPPER_FUNC(asinf))(float x)
{
	// check range -1.0 .. +1.0
	float u = (1.0f - x)*(1.0f + x);
	if (fisstrictneg(u)) return NANF; // negative is invalid
	if (fiszero(x)) return x; // zero

	// get asinf from atan2f
	return atan2f(x, sqrtf(u));
}

// arc cosine
float MYPREFIX(WRAPPER_FUNC(acosf))(float x)
{
	// check range -1.0 .. +1.0
	float u = (1.0f - x)*(1.0f + x);
	if (fisstrictneg(u)) return NANF; // negative is invalid

	// get acosf from atan2f
	return atan2f(sqrtf(u), x);
}
#endif

#if RISCV && (USE_RISCV_ATANF == 1)
// arc tangent of y/x in radians
float MYPREFIX(WRAPPER_FUNC(atan2f))(float y, float x)
{
	// get exponents
	int ex = fgetexp(x);
	int ey = fgetexp(y);

	// get signs
	Bool sx = fisneg(x);
	Bool sy = fisneg(y);
	
	// y is infinity
	float res;
	if (ey == 255)
	{
		// prepare result PI/2
		res = (float)(PI/2);

		// x is infinity too - result is PI/4
		if (ex == 255) res = (float)(PI/4);
	}
	else
	{
		// x is infinity, y is not - result is 0
		if (ex == 255)
			res = 0;
		else
		{
			// y is zero - result is 0
			if (ey == 0)
				res = 0;
			else
			{
				// x is zero - result is pi/2
				if (ex == 0)
					res = (float)(PI/2);
				// x and y are in usual range
				else
				{
					// calculate atan(y/x)
					float a = y/x;
					res = atanf(a);
					if (fisneg(res)) res = fneg(res);
				}
			}
		}
	}

	// shift result by X value (= PI - num)
	if (sx) res = (float)PI - res;

	// set sign by the Y
	if (sy) res = fneg(res);

	return res;
}
#endif

#if /*USE_FLOATLIBC ||*/ !RISCV || (USE_RISCV_EXPF == 0) || (USE_RISCV_EXPF == 3)
// exponent with base 2
float MYPREFIX(WRAPPER_FUNC(exp2f))(float x)
{
	// exp(x * log(2))
	return expf(x*LOG2f);
}

// exponent with base 10
float MYPREFIX(WRAPPER_FUNC(exp10f))(float x)
{
	// exp(x * log(10))
	return expf(x*LOG10f);
}
#endif

#if /*USE_FLOATLIBC ||*/ !RISCV || (USE_RISCV_LOGF == 0) || (USE_RISCV_LOGF == 3)
// logarithm with base 2
float MYPREFIX(WRAPPER_FUNC(log2f))(float x)
{
	// log(x) / log(2)
	return logf(x)*LOG2Ef;
}

// logarithm with base 10
float MYPREFIX(WRAPPER_FUNC(log10f))(float x)
{
	// log(x) / log(10)
	return logf(x)*LOG10Ef;
}
#endif

// RISC-V: atanf cannot be implemented this way because atanf is internally called from atan2f in libc
#if !RISCV || (USE_RISCV_ATANF == 3)
// arc tangent
float MYPREFIX(WRAPPER_FUNC(atanf))(float x)
{
	// limit values -inf and +inf
	if (fispinf(x)) return (float)(+PIf/2); // atanf(+inf) = +PI/2
	if (fisminf(x)) return (float)(-PIf/2);	// atanf(-inf) = -PI/2
	if (fiszero(x)) return x; // zero

	// get atanf from atan2f
	return atan2f(x, 1.0f);
}
#endif // !RISCV

// hyperbolic sine
float MYPREFIX(WRAPPER_FUNC(sinhf))(float x)
{
	int e = fgetexp(x);
	if (e < 127 - 9) return x;

	// (e^x - e^-x)/2
	return ldexpf(expf(x) - expf(fneg(x)), -1);
}

// hyperbolic cosine
float MYPREFIX(WRAPPER_FUNC(coshf))(float x)
{
	// (e^x + e^-x)/2
	return ldexpf(expf(x) + expf(fneg(x)), -1);
}

// hyperbolic tangent
float MYPREFIX(WRAPPER_FUNC(tanhf))(float x)
{
	int e = fgetexp(x);
	if (e < 127 - 9) return x;

	// check big number
	if (e >= 127 + 4) // check exponent 4 (+ bias 127); number abs(x) >= 16?
	{
		if (!fisneg(x)) return 1;  // limit positive result to 1
		return -1; // limit negative result to -1
	}

	// small number = (e^2x - 1) / (e^2x + 1)
	float u=expf(ldexpf(x, 1)); // e^(2*x)
	return (u - 1.0f)/(u + 1.0f);
}

// inverse hyperbolic sine
float MYPREFIX(WRAPPER_FUNC(asinhf))(float x)
{
	int e = fgetexp(x);
	if (e < 127 - 8) return x;

	// check big number
	if (e >= 127 + 16) // check exponent 16 (+ bias 127); number abs(x) >= 2^16?
	{
		if (!fisneg(x)) return logf(x) + LOG2f;  // 1/x^2 << 1
		return fneg(logf(fneg(x)) + LOG2f); // 1/x^2 << 1
	}

	// positive number
	if (x > 0) // log(sqrt(x^2 + 1) + x)
		return logf(sqrtf(x*x + 1.0f) + x);
	// negative number
	else	// -log(sqrt(x^2 + 1) - x)
		return fneg(logf(sqrtf(x*x + 1.0f) - x));
}

// inverse hyperbolic cosine
float MYPREFIX(WRAPPER_FUNC(acoshf))(float x)
{
	// absolute value of the number
	if (fisneg(x)) x=fneg(x);
	if (x < 1.0f) return NANF; // invalid

	// get exponent
	int e = fgetexp(x);

	// check big number; check exponent 16 (+ bias 127); number abs(x) >= 2^16?
	if (e >= 16+0x7f) return logf(x) + LOG2f;

	// log(sqrt((x + 1)*(x - 1)) + x)
	return logf(sqrtf((x + 1.0f)*(x - 1.0f)) + x);
}

// inverse hyperbolic tangent
float MYPREFIX(WRAPPER_FUNC(atanhf))(float x)
{
	int e = fgetexp(x);
	if (e < 127 - 8) return x;

	// log((1 + x) / (1 - x)) / 2
	return ldexpf(logf((1.0f + x)/(1.0f - x)), -1);
}

// expf(x) - 1
float MYPREFIX(WRAPPER_FUNC(expm1f))(float x)
{
	int e = fgetexp(x);
	if (e < 127 - 15) return x;

	return expf(x) - 1;
}

// logf(x + 1)
float MYPREFIX(WRAPPER_FUNC(log1pf))(float x)
{
	if (fgetexp(x) < 127 - 12) return x;
	return logf(1 + x);
}

// x*y + z
float MYPREFIX(WRAPPER_FUNC(fmaf))(float x, float y, float z)
{
	return x*y + z;
}

// general power, x>0
INLINE float fpow_1(float x, float y)
{
	return expf(logf(x)*y);
}

// integer power by 2^y
float MYPREFIX(fpow_int2)(float x, int y)
{
	// end of iterations
	if (y==1) return x;

	// power by even number
	float u=fpow_int2(x, y/2);

	// y^2
	u *= u;

	// odd multiply, y *= x
	if ((y & 1) != 0) u *= x;

	return u;
}

// integer power by 2^y, y can be negative
INLINE float fpowint_1(float x, int y)
{
	// y is negative
	if (y < 0)
	{
		x = 1.0f/x;
		y = -y;
	}
	return fpow_int2(x, y);
}

// integer power by 2^y, y can be negative, x can be negative
float MYPREFIX(fpowint_0)(float x, int y)
{
	int e;

	// x is negative (result will be negative if y is odd)
	if (fisneg(x))
	{
		x = fpowint_0(fneg(x),y); // power by 2^y (x positive)
		if ((y & 1) != 0) x = fneg(x); // negate result if y is odd
		return x;
	}

	// x is power of 2
	if (ispow2f(x)) // check if x is power of 2
	{
		e=fgetexp(x) - 0x7f; // get absolute exponent, without base
		if (y >= 256) y = 255;  // max. power
		if (y < -256) y = -256; // min. power
		y *= e; // get new exponent
		return ldexpf(1, y); // get power 2^y
	}

	// power x^0, result is 1
	if (y == 0) return 1;

	// exponent is small
	if ((y >= -32) && (y <= 32)) return fpowint_1(x, y);

	// exponent is large - use float calculations
	return fpow_1(x, y);
}

// power by integer, x^y
float MYPREFIX(WRAPPER_FUNC(powintf))(float x, int y)
{
	// x = 1 or y = 0, result is 1
	if ((x == 1.0f) || (y == 0)) return 1;

	// x = 0
	// do not use "x == 0.0f" - in that case compiler can later use positive zero instead of real X
	if (fiszero(x))
	{
		// exponent is positive
		if (y > 0)
		{
			if ((y & 1) != 0) return x; // exponent is odd, result is x = +0.0 or -0.0
			return 0; // exponent is even, result is +0.0
		}

		// exponent is negative and odd - result is +-INF
		if ((y & 1) != 0) return copysignf(FPINF,x);

		// result is +INF
		return FPINF;
	}

	// x is positive infinity NaN
	if (fispinf(x))
	{
		// exponent is negative, result will be 0
		if (y < 0) return 0;
		return FPINF; // exponent is positive, result will be infinity NaN
	}

	// x is negative infinity NaN
	if (fisminf(x))
	{
		// exponent is positive
		if (y > 0)
		{
			if ((y & 1) != 0) return FMINF; // exponent is odd, result is negative infinity
			return FPINF; // exponent is even, result is positive infinity
		}

		// exponent is negative and odd - result is -0.0
		if ((y & 1) != 0) return MZERO;

		// result is +0.0
		return PZERO;
	}

	// integer power by 2^y, y can be negative, x can be negative
	return fpowint_0(x, y);
}

// power x^y, y is integer
float MYPREFIX(fpow_0)(float x, float y)
{
	int e, p;

	// x is negative
	if (fisneg(x))
	{
		// y is odd integer - negate result
		if (isoddintf(y)) return fneg(fpow_0(fneg(x),y));

		// y is even or not integer - use absolute value of x
		return fpow_0(fneg(x), y);
	}

	// get integer of power
	p = (int)y;

	// x is power of 2
	if (ispow2f(x)) // check if x is power of 2
	{
		e=fgetexp(x) - 0x7f; // get absolute exponent, without base
		if (p >= 256) p = 255;  // max. power
		if (p < -256) p = -256; // min. power
		p *= e; // get new exponent
		return ldexpf(1, p); // get power 2^p
	}

	// power is 0, result is 1
	if (p == 0) return 1;

	// power is small number, use integer
	if ((p >= -32) && (p <= 32)) return fpowint_1(x,p);

	// exponent is large - use float calculations
	return fpow_1(x, y);
}

// power x^y
float MYPREFIX(WRAPPER_FUNC(powf))(float x, float y)
{
	// x is 1 or y is 0, result is 1
	if ((x == 1.0f) || fiszero(y)) return 1;

	// x is -1 and y is infinity, result is +1
	if ((x == -1.0f) && fisinf(y)) return 1;

	// x is zero
	if (fiszero(x))
	{
		// y is positive
		if (!fisneg(y))
		{
			// y is odd and integer, return +0.0 or -0.0
			if (isoddintf(y)) return x;
			return 0; // else result is 0
		}

		// y is negative, odd and integer - return +-infinity
		if (isoddintf(y)) return copysignf(FPINF, x);

		// y is negative, return positive infinity
		return FPINF;
	}

	// x is positive infinity
	if (fispinf(x))
	{
		// y is negative, result will be 0
		if (fisneg(y)) return 0;
		return FPINF; // y is positive, result is positive infinity
	}

	// x is negative infinity
	if (fisminf(x))
	{
		// y is positive
		if (!fisneg(y))
		{
			// y is positive and odd integer, result is negative infinity
			if (isoddintf(y)) return FMINF;

			// y is negative or not odd integer, result is positive infinity
			return FPINF;
		}

		// y is negative and odd integer, result will be -0.0
		if (isoddintf(y)) return MZERO;

		// y is negative, result will be +0.0
		return PZERO;
	}

	// y is positive infinity
	if (fispinf(y))
	{
		// x is < 1, result is +0.0
		if (fgetexp(x) < 0x7f) return PZERO;

		// x is >= 1, result is positive infinity
		return FPINF;
	}

	// y is negative infinity
	if (fisminf(y))
	{
		// x is < 1, result is positive infinity
		if (fgetexp(x) < 0x7f) return FPINF;

		// x is >= 1, result is +0.0
		return PZERO;
	}

	// y is integer, use power with integer y
	if (isintf(y)) return fpow_0(x,y);

	// x is negative - invalid result
	if (fisneg(x)) return FPINF;

	// get power using floats
	return fpow_1(x,y);
}

// square root of sum of squares (hypotenuse), sqrt(x*x + y*y)
float MYPREFIX(WRAPPER_FUNC(hypotf))(float x, float y)
{
	// get exponents
	int ex = fgetexp(x);
	int ey = fgetexp(y);

	// possible overflow of some square (some number has exponent >= 50)
	if ((ex >= 0x7f + 50) || (ey >= 0x7f + 50))
	{
		// reduce exponents (correction must be even number)
		x = ldexpf(x, -70);
		y = ldexpf(y, -70);

		// calculate result with reduced exponents
		return ldexpf(sqrtf(x*x + y*y), 70); // reduce result back
	}

	// possible underflow of both squares (both numbers have exponent <= -50)
	if ((ex <= 0x7f - 50) && (ey <= 0x7f - 50))
	{
		// reduce exponents (correction must be even number)
		x = ldexpf(x, 70);
		y = ldexpf(y, 70);

		// calculate result with reduced exponents
		return ldexpf(sqrtf(x*x + y*y), -70); // reduce result back
	}

	// calculate result in normal range
	return sqrtf(x*x + y*y);
}

// cube root, sqrt3(x), x^(1/3)
float MYPREFIX(WRAPPER_FUNC(cbrtf))(float x)
{
	int e;

	// x is zero, result will be +0.0 or -0.0
	if (fiszero(x)) return copysignf(PZERO, x);

	// x is negative, change sign
	if (fisneg(x)) return fneg(cbrtf(fneg(x)));

	// get absolute exponent, without base
	e = fgetexp(x) - 0x7f;

	// prepare integer exponent = e/3 (rounded)
	//   0x10000 / 3 = 21845.333 = 0x5555, rounded down
	e = (e*0x5555 + 0x8000) >> 16;

	// shift x number by exponent/3
	x = ldexpf(x, -e*3);

	// calculate cube root = exp(log(x)/3)
	x = expf(logf(x)*ONETHIRDf);

	// restore exponent
	return ldexpf(x, e);
}

// ==== basic arithmetic

// reduces mx*2^e modulo my, returning bottom bits of quotient at *pquo
// 2^23<=|mx|, my<2^24, e>=0; 0<=result<my
//  mx ... mantissa X
//  my ... mantissa Y
//  e ... binary exponent
s32 MYPREFIX(frem_0)(s32 mx, s32 my, int e, int* pquo)
{
	int quo = 0, q, r = 0, s;
	if (e > 0)
	{
		r = 0xffffffffU/(u32)(my >> 7);  // reciprocal estimate Q16
	}

	while (e > 0)
	{
		s = e;
		if (s > 12) s = 12;	// gain up to 12 bits on each iteration
		q = (mx >> 9)*r;	// Q30
		q = ((q >> (29 - s)) + 1) >> 1;  // Q(s), rounded
		mx = (mx << s) - my*q;
		quo = (quo << s) + q;
		e -= s;
	}

	if (mx >= my) mx -= my, quo++; // when e==0 mx can be nearly as big as 2my
	if (mx >= my) mx -= my, quo++;
	if (mx < 0) mx += my, quo--;
	if (mx < 0) mx += my, quo--;
	if (pquo) *pquo = quo;

	return mx;
}

// get remainder of division x/y, rounded towards zero
//  fmod (x, y) = x - tquote*y, where tquote is truncated (i.e. rounded towards zero) result of x/y
float MYPREFIX(WRAPPER_FUNC(fmodf))(float x, float y)
{
	// get numbers as u32
	u32 ix = float2u32(x);
	u32 iy = float2u32(y);

	// unpack numbers
	int sx,ex,ey;
	s32 mx,my;
	FUNPACKS(ix,sx,ex,mx);	// unpack X to sign, exponent and mantissa
	FUNPACK(iy,ey,my);	// unpack Y to exponent and mantissa

	// X is infinity NaN or Y is zero, return infinity
	if ((ex == 0xff) || (ey == 0))
		return FPINF;	// plus infinity

	// X is zero, return +0.0 or -0.0
	if (ex == 0)
	{
		if (!fisneg(x)) return PZERO; // positive, return +0.0
		return MZERO;	// negative, return -0.0
	}

	// if abs(x) < abs(y), return whole X as remainder
	if (ex < ey) return x;

	// get remainder
	mx = frem_0(mx, my, ex-ey, 0);

	// correct sign
	if (sx) mx =- mx;

	// convert result to float
	return fix2float(mx, 0x7f - ey + 23);
}

// compute remainder and quotient of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
float MYPREFIX(WRAPPER_FUNC(remquof))(float x, float y, int* quo)
{
	// get numbers as u32
	u32 ix = float2u32(x);
	u32 iy = float2u32(y);

	// unpack numbers
	int sx,sy,ex,ey,q;
	s32 mx,my;
	FUNPACKS(ix,sx,ex,mx);	// unpack X to sign, exponent and mantissa
	FUNPACKS(iy,sy,ey,my);	// unpack Y to sign, exponent and mantissa

	// preset quotient
	if (quo) *quo = 0;

	// limit values
	if (ex == 0xff) return FPINF; // X is infinity, result is plus infinity
	if (ey == 0) return FPINF; // Y is zero, result is plus infinity
	if (ex == 0) return PZERO; // X is zero, result is +0.0
	if (ey == 0xff) return x; // Y is infinity, result is X
	if (ex < ey - 1) return x;  // |x|<|y|/2 ... X is smaller than Y, result is X

	// abs(x) == abs(y)/2
	if (ex == ey - 1)
	{
		if (mx <= my) return x;  // |x|<=|y|/2, even quotient

		// here |y|/2<|x|<|y|
		if (!sx) // x>|y|/2
		{
			mx -= my + my;
			ey--;
			q = 1;
		}
		else // x<-|y|/2
		{
			mx = my + my - mx;
			ey--;
			q = -1;
		}
	}
	else // abs(x) > abs(y)/2
	{
		if (sx) mx = -mx;
		mx = frem_0(mx,my,ex-ey,&q); // get remainder and quotient
		if ((mx+mx > my) || ((mx+mx == my) && (q & 1)) )
		{ // |x|>|y|/2, or equality and an odd quotient?
			mx -= my;
			q++;
		}
	}

	if (sy) q =- q;
	if (quo) *quo = q;
	return fix2float(mx, 0x7f - ey + 23);
}

// get remainder of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
float MYPREFIX(WRAPPER_FUNC(remainderf))(float x, float y)
{
	return remquof(x,y,0);
}

// get remainder of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
float MYPREFIX(WRAPPER_FUNC(dremf))(float x, float y) // obsolete synonym
{
	return remquof(x,y,0);
}

#endif // !USE_FLOATLIBC

#endif // USE_FLOAT		// use float support
