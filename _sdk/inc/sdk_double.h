
// ****************************************************************************
//
//                                    Double
//                             Double-floating-point
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/

#ifndef _SDK_DOUBLE_H
#define _SDK_DOUBLE_H

#ifdef __cplusplus
extern "C" {
#endif

#define DOUBLE_MANT_BITS	52		// number of bits of mantissa
#define DOUBLE_MANT_BITS_H	20		// number of bits of mantissa in high u32
#define DOUBLE_MANT_MASK	0x000fffffffffffffULL // mask of mantissa
#define DOUBLE_MANT_MASK_H	0x000fffff	// mask of mantissa in high u32
#define DOUBLE_EXP_BITS		11		// number of bits of exponent
#define DOUBLE_EXP_MASK		0x7ff		// mask of exponent value
#define DOUBLE_EXP_BIAS		0x3ff		// bias of exponent (= value 1.0)
#define DOUBLE_EXP_INF		0x7ff		// exponent infinity (overflow)

#if USE_DOUBLE		// use double support 1=in RAM, 2=in Flash

#ifdef log2
#undef log2 // defined in math.h: #define log2(x) (log (x) / _M_LN2)
#endif

typedef union { double f; u64 n; } tmp_double_u64;

// retype number to double
INLINE double u64double(u64 n) { tmp_double_u64 tmp; tmp.n = n; return tmp.f; }

// retype double to number
INLINE u64 doubleu64(double f) { tmp_double_u64 tmp; tmp.f = f; return tmp.n; }

// get double exponent
INLINE int doublegetexp(double f) { return (int)((doubleu64(f) << 1) >> (DOUBLE_MANT_BITS+1)); }

// check if double is infinity
INLINE Bool doubleisinf(double f) { return doublegetexp(f) == DOUBLE_EXP_INF; }

// check if double is zero
INLINE Bool doubleiszero(double f) { return doublegetexp(f) == 0; }

// ==== common auxiliary functions, not in libc
// sdk_double.c

// compose floating point with magnitude of 'num' and sign of 'sign'
double copysign(double num, double sign);

// check if number is an integer
Bool isintd(double num);

// check if number is odd integer (must be an integer ..., -3, -1, 1, 3, 5,..)
Bool isoddintd(double num);

// check if number is power of 2
Bool ispow2d(double num);

// round to given number of significant digits (digits<=0 to use default number of digits)
// @TODO: probably will be deleted (accuracy cannot be guaranteed)
//double rounddig(double x, int digits);

// Check if comparison is unordered (either input is NaN)
Bool dcmpun(double x, double y);

// convert degrees to radians
INLINE double deg2rad(double x) { return x * (PI/180); }

// convert radians to degrees
INLINE double rad2deg(double x) { return x * (180/PI); }

// ----------------------------------------------------------------------------
//              Common functions - faster alternatives to libc
// ----------------------------------------------------------------------------
// sdk_double.c

#if !USE_DOUBLELIBC

// ==== auxiliary functions

// multiply number by power of 2 (num * 2^exp)
double ldexp(double num, int exp);

// absolute value
double absd(double x);

// ==== rounding

// round number towards zero
double trunc(double num);

// round number to nearest integer
double round(double num);

// round number down to integer
double floor(double num);

// round number up to integer
double ceil(double num);

// ==== scientific functions

// sine in degrees
double sin_deg(double x);

// cosine in degrees
double cos_deg(double x);

// sine-cosine in degrees
void sincos_deg(double x, double* psin, double* pcos);

// tangent in degrees
double tan_deg(double x);

// cotangent
double cotan(double x);

// cotangent in degrees
double cotan_deg(double x);

// arc sine in radians
double asin(double x);

// arc sine in degrees
double asin_deg(double x);

// arc cosine in radians
double acos(double x);

// arc cosine in degrees
double acos_deg(double x);

// RISC-V: atan cannot be implemented this way because atan is internally called from atan2 in libc
#if !RISCV
// arc tangent in radians
double atan(double x);
#endif

// arc tangent in degrees
double atan_deg(double x);

#if RISCV
// arc cotangent in radians
double acotan(double x);

// arc cotangent in degrees
double acotan_deg(double x);
#else
// arc cotangent in radians
INLINE double acotan(double x) { return PI/2 - atan(x); }

// arc cotangent in degrees
INLINE double acotan_deg(double x) { return 90.0 - atan_deg(x); }
#endif

// arc tangent of y/x in degrees
double atan2_deg(double y, double x);

// hyperbolic sine
double sinh(double x);

// hyperbolic cosine
double cosh(double x);

// hyperbolic tangent
double tanh(double x);

// inverse hyperbolic sine
double asinh(double x);

// inverse hyperbolic cosine
double acosh(double x);

// inverse hyperbolic tangent
double atanh(double x);

// exponent with base 2
double exp2(double x);

// logarithm with base 2
double log2(double x);

// exponent with base 10
double exp10(double x);

// logarithm with base 10
double log10(double x);

// exp(x) - 1
double expm1(double x);

// log(x + 1)
// - This function can be very inaccurate
double log1p(double x);

#if RP2040 || RISCV
// x*y + z
double fma(double x, double y, double z);
#endif

// power by integer, x^y
double powint(double x, int y);

// power x^y
double pow(double x, double y);

// square root of sum of squares (hypotenuse), sqrt(x*x + y*y)
double hypot(double x, double y);

// cube root, sqrt3(x), x^(1/3)
double cbrt(double x);

// ==== basic arithmetic

// get remainder of division x/y, rounded towards zero
//  fmod (x, y) = x - tquote*y, where tquote is truncated (i.e. rounded towards zero) result of x/y
double fmod(double x, double y);

// compute remainder and quotient of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
double remquo(double x, double y, int* quo);

// get remainder of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
double remainder(double x, double y);
double drem(double x, double y); // obsolete synonym

#endif // !USE_DOUBLELIBC && !RISCV

// ----------------------------------------------------------------------------
//                             RP2040 functions
// ----------------------------------------------------------------------------
// sdk_double_m0.S

#if RP2040 && !USE_DOUBLELIBC

// ==== basic arithmetic

// Addition, x + y
double dadd(double x, double y);

// Subtraction, x - y
double dsub(double x, double y);

// Multiplication, x * y
double dmul(double x, double y);

// Square, x^2 
double dsqr(double x);

// Division, x / y
double ddiv(double x, double y);

// Reciprocal 1 / x
double drec(double x);

// ==== comparison

// Compare, x ? y
// Returns: 0 if x==y, -1 if x<y, +1 if x>y
s8 dcmp(double x, double y);

// Compare if x==y
Bool dcmpeq(double x, double y);

// Compare if x<y
Bool dcmplt(double x, double y);

// Compare if x<=y
Bool dcmple(double x, double y);

// Compare if x>=y
Bool dcmpge(double x, double y);

// Compare if x>y
Bool dcmpgt(double x, double y);

// ==== convert integer to double

// Convert signed int to double
double i2d(s32 num);
double int2double(s32 num);

// Convert unsigned int to double
double ui2d(u32 num);
double uint2double(u32 num);

// Convert 64-bit signed int to double
double l2d(s64 num);
double int642double(s64 num);

// Convert 64-bit unsigned int to double
double ul2d(s64 num);
double uint642double(u64 num);

// Convert signed fixed point to double
//  e = number of bits of fractional part (binary exponent)
double fix2double(s32 num, int e);

// Convert unsigned fixed point to double
//  e = number of bits of fractional part (binary exponent)
double ufix2double(u32 num, int e);

// Convert 64-bit signed fixed point to double
//  e = number of bits of fractional part (binary exponent)
double fix642double(s64 num, int e);

// Convert 64-bit unsigned fixed point to double
//  e = number of bits of fractional part (binary exponent)
double ufix642double(u64 num, int e);

// ==== convert double to integer

// Convert double to signed int, rounding to zero (C-style int conversion)
s32 d2iz(double num);
s32 double2int_z(double num);

// Convert double to signed int, rounding down
s32 d2i(double num);
s32 double2int(double num);

// Convert double to unsigned int, rounding down
u32 d2ui(double num);
u32 double2uint(double num);

// Convert double to 64-bit signed int, rounding to zero (C-style int conversion)
s64 d2lz(double num);
s64 double2int64_z(double num);

// Convert double to 64-bit signed int, rounding down
s64 d2l(double num);
s64 double2int64(double num);

// Convert double to 64-bit unsigned int
u64 d2ul(double num);
u64 double2uint64(double num);

// Convert double to signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
s32 double2fix(double num, int e);

// Convert double to unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
u32 double2ufix(double num, int e);

// Convert double to 64-bit signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
s64 double2fix64(double num, int e);

// Convert double to 64-bit unsigned fixed point
//  e = number of bits of fractional part (binary exponent)
u64 double2ufix64(double num, int e);

// ==== scientific functions

// Square root
double sqrt(double x);

// sine in radians
double sin(double x);

// cosine in radians
double cos(double x);

// sine-cosine in radians
void sincos(double x, double* psin, double* pcos);

// tangent in radians
double tan(double x);

// arc tangent of y/x in radians
double atan2(double y, double x);

// Natural exponent
double exp(double x);

// Natural logarithm
double log(double x);

#endif // RP2040 && !USE_DOUBLELIBC

// ----------------------------------------------------------------------------
//                             RP2350-ARM functions
// ----------------------------------------------------------------------------

#if RP2350 && !USE_DOUBLELIBC && !RISCV

// ==== basic arithmetic

// Addition, x + y
double dadd(double x, double y);

// Subtraction, x - y
double dsub(double x, double y);

// Multiplication, x * y
double dmul(double x, double y);

// Square, x^2 
INLINE double dsqr(double x) { return x * x; }

// Division, x / y
double ddiv(double x, double y);

// Reciprocal 1 / x
INLINE double drec(double x) { return 1.0 / x; }

// ==== comparison

// Compare, x ? y ... Returns: 0 if x==y, -1 if x<y, +1 if x>y
INLINE s8 dcmp(double x, double y) { return (x == y) ? 0.0 : ((x < y) ? -1.0 : +1.0); }

// Compare if x==y
Bool dcmpeq(double x, double y);

// Compare if x<y
Bool dcmplt(double x, double y);

// Compare if x<=y
Bool dcmple(double x, double y);

// Compare if x>=y
Bool dcmpge(double x, double y);

// Compare if x>y
Bool dcmpgt(double x, double y);

// ==== convert integer to double

// Convert signed int to double
double i2d(s32 num);
double int2double(s32 num);

// Convert unsigned int to double
double ui2d(u32 num);
double uint2double(u32 num);

// Convert 64-bit signed int to double
double l2d(s64 num);
double int642double(s64 num);

// Convert 64-bit unsigned int to double
double ul2d(s64 num);
double uint642double(u64 num);

// Convert signed fixed point to double
//  e = number of bits of fractional part (binary exponent)
double fix2double(s32 num, int e);

// Convert unsigned fixed point to double
//  e = number of bits of fractional part (binary exponent)
double ufix2double(u32 num, int e);

// Convert 64-bit signed fixed point to double
//  e = number of bits of fractional part (binary exponent)
double fix642double(s64 num, int e);

// Convert 64-bit unsigned fixed point to double
//  e = number of bits of fractional part (binary exponent)
double ufix642double(u64 num, int e);

// ==== convert double to integer

// Convert double to signed int, rounding to zero (C-style int conversion)
s32 d2iz(double num);
s32 double2int_z(double num);

// Convert double to signed int, rounding down
INLINE s32 d2i(double num) { return (s32)floor(num); }
INLINE s32 double2int(double num) { return d2i(num); }

// Convert double to unsigned int, rounding down
u32 d2ui(double num);
u32 double2uint(double num);

// Convert double to 64-bit signed int, rounding to zero (C-style int conversion)
s64 d2lz(double num);
s64 double2int64_z(double num);

// Convert double to 64-bit signed int, rounding down
INLINE s64 d2l(double num) { return d2lz(floor(num)); }
INLINE s64 double2int64(double num) { return d2l(num); }

// Convert double to 64-bit unsigned int
u64 d2ul(double num);
u64 double2uint64(double num);

// Convert double to signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
//s32 double2fix(double num, int e);
INLINE s32 double2fix(double num, int e) { return d2i(ldexp(num, e)); }

// Convert double to unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
//u32 double2ufix(double num, int e);
INLINE u32 double2ufix(double num, int e) { return d2ui(ldexp(num, e)); }

// Convert double to 64-bit signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
//s64 double2fix64(double num, int e);
INLINE s64 double2fix64(double num, int e) { return d2l(ldexp(num, e)); }

// Convert double to 64-bit unsigned fixed point
//  e = number of bits of fractional part (binary exponent)
u64 double2ufix64(double num, int e);

// ==== scientific functions

// Square root
double sqrt(double x);

// sine in radians
double sin(double x);

// cosine in radians
double cos(double x);

// sine-cosine in radians
void sincos(double x, double* psin, double* pcos);

// tangent in radians
double tan(double x);

// arc tangent of y/x in radians
double atan2(double y, double x);

// Natural exponent
double exp(double x);

// Natural logarithm
double log(double x);

#endif // RP2350 && !USE_DOUBLELIBC && !RISCV

// ----------------------------------------------------------------------------
//                             RP2350-RISCV functions
// ----------------------------------------------------------------------------

#if RP2350 && !USE_DOUBLELIBC && RISCV

// ==== basic arithmetic

// Addition, x + y
double dadd(double x, double y);
//INLINE double dadd(double x, double y) { return x + y; }

// Subtraction, x - y
double dsub(double x, double y);
//INLINE double dsub(double x, double y) { return x - y; }

// Multiplication, x * y
double dmul(double x, double y);
//INLINE double dmul(double x, double y) { return x * y; }

// Square, x^2 
double dsqr(double x);
//INLINE double dsqr(double x) { return x * x; }

// Division, x / y
double ddiv(double x, double y);
//INLINE double ddiv(double x, double y) { return x / y; }

// Reciprocal 1 / x
double drec(double x);
//INLINE double drec(double x) { return 1.0 / x; }

// ==== comparison

// Compare, x ? y ... Returns: 0 if x==y, -1 if x<y, +1 if x>y
INLINE s8 dcmp(double x, double y) { return (x == y) ? 0.0 : ((x < y) ? -1.0 : +1.0); }

// Compare if x==y
INLINE Bool dcmpeq(double x, double y) { return x == y; }

// Compare if x<y
INLINE Bool dcmplt(double x, double y) { return x < y; }

// Compare if x<=y
INLINE Bool dcmple(double x, double y) { return x <= y; }

// Compare if x>=y
INLINE Bool dcmpge(double x, double y) { return x >= y; }

// Compare if x>y
INLINE Bool dcmpgt(double x, double y) { return x > y; }

// ==== convert integer to double

// Convert signed int to double
INLINE double i2d(s32 num) { return (double)num; }
INLINE double int2double(s32 num) { return (double)num; }

// Convert unsigned int to double
INLINE double ui2d(u32 num) { return (double)num; }
INLINE double uint2double(u32 num) { return (double)num; }

// Convert 64-bit signed int to double
INLINE double l2d(s64 num) { return (double)num; }
INLINE double int642double(s64 num) { return (double)num; }

// Convert 64-bit unsigned int to double
INLINE double ul2d(s64 num) { return (double)num; }
INLINE double uint642double(u64 num) { return (double)num; }

// Convert signed fixed point to double
//  e = number of bits of fractional part (binary exponent)
INLINE double fix2double(s32 num, int e) { return ldexp((double)num, -e); }

// Convert unsigned fixed point to double
//  e = number of bits of fractional part (binary exponent)
INLINE double ufix2double(u32 num, int e) { return ldexp((double)num, -e); }

// Convert 64-bit signed fixed point to double
//  e = number of bits of fractional part (binary exponent)
INLINE double fix642double(s64 num, int e) { return ldexp((double)num, -e); }

// Convert 64-bit unsigned fixed point to double
//  e = number of bits of fractional part (binary exponent)
INLINE double ufix642double(u64 num, int e) { return ldexp((double)num, -e); }

// ==== convert double to integer

// Convert double to signed int, rounding to zero (C-style int conversion)
INLINE s32 d2iz(double num) { return (s32)num; }
INLINE s32 double2int_z(double num) { return (s32)num; }

// Convert double to signed int, rounding down
INLINE s32 d2i(double num) { return (s32)floor(num); }
INLINE s32 double2int(double num) { return (s32)floor(num); }

// Convert double to unsigned int, rounding down
INLINE u32 d2ui(double num)
{
	s64 nn = (s64)num;
	if (nn < 0) return 0;
	if (nn > 0xffffffffull) return 0xfffffffful;
	return (u32)nn;
}
INLINE u32 double2uint(double num) { return d2ui(num); }

// Convert double to 64-bit signed int, rounding to zero (C-style int conversion)
INLINE s64 d2lz(double num)
{
	u64 n = *(u64*)&num;
	if ((s64)n >= 0) // num >= 0
	{
		if (n >= 0x43e0000000000000ull) return 0x7fffffffffffffffll; // overflow
	}
	else // num < 0
	{
		if (n >= 0xc3e0000000000000ull) return 0x8000000000000000ll; // underflow
	}
	return (s64)num;
}
INLINE s64 double2int64_z(double num) { return d2lz(num); }

// Convert double to 64-bit signed int, rounding down
INLINE s64 d2l(double num) { return d2lz(floor(num)); }
INLINE s64 double2int64(double num) { return d2l(num); }

// Convert double to 64-bit unsigned int, rounding down
INLINE u64 d2ul(double num)
{
	if (num < 0) return 0;
	return (u64)num;
}
INLINE u64 double2uint64(double num) { return d2ul(num); }

// Convert double to signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE s32 double2fix(double num, int e) { return d2i(ldexp(num, e)); }

// Convert double to unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE u32 double2ufix(double num, int e) { return d2ui(ldexp(num, e)); }

// Convert double to 64-bit signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE s64 double2fix64(double num, int e) { return d2l(ldexp(num, e)); }

// Convert double to 64-bit unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE u64 double2ufix64(double num, int e) { return d2ul(ldexp(num, e)); }

// ==== scientific functions

// Square root
double sqrt(double x);

// sine in radians
double sin(double x);

// cosine in radians
double cos(double x);

// sine-cosine in radians
void sincos(double x, double* psin, double* pcos);

// tangent in radians
double tan(double x);

// arc tangent of y/x in radians
double atan2(double y, double x);

// Natural exponent
double exp(double x);

// Natural logarithm
double log(double x);

#endif // RP2350 && !USE_DOUBLELIBC && RISCV

// ----------------------------------------------------------------------------
//                       LIBC not implemented functions
// ----------------------------------------------------------------------------

#if USE_DOUBLELIBC

// ==== auxiliary functions

// multiply number by power of 2 (num * 2^exp)
double ldexp(double num, int exp);

// absolute value
INLINE double absd(double x) { return (x < 0) ? -x : x; }

// ==== basic arithmetic

// Addition, x + y
INLINE double dadd(double x, double y) { return x + y; }

// Subtraction, x - y
INLINE double dsub(double x, double y) { return x - y; }

// Multiplication, x * y
INLINE double dmul(double x, double y) { return x * y; }

// Square, x^2 
INLINE double dsqr(double x) { return x * x; }

// Division, x / y
INLINE double ddiv(double x, double y) { return x / y; }

// Reciprocal 1 / x
INLINE double drec(double x) { return 1.0 / x; }

// ==== comparison

// Compare, x ? y ... Returns: 0 if x==y, -1 if x<y, +1 if x>y
INLINE s8 dcmp(double x, double y) { return (x == y) ? 0.0 : ((x < y) ? -1.0 : +1.0); }

// Compare if x==y
INLINE Bool dcmpeq(double x, double y) { return x == y; }

// Compare if x<y
INLINE Bool dcmplt(double x, double y) { return x < y; }

// Compare if x<=y
INLINE Bool dcmple(double x, double y) { return x <= y; }

// Compare if x>=y
INLINE Bool dcmpge(double x, double y) { return x >= y; }

// Compare if x>y
INLINE Bool dcmpgt(double x, double y) { return x > y; }

// ==== convert integer to double

// Convert signed int to double
INLINE double i2d(s32 num) { return (double)num; }
INLINE double int2double(s32 num) { return (double)num; }

// Convert unsigned int to double
INLINE double ui2d(u32 num) { return (double)num; }
INLINE double uint2double(u32 num) { return (double)num; }

// Convert 64-bit signed int to double
INLINE double l2d(s64 num) { return (double)num; }
INLINE double int642double(s64 num) { return (double)num; }

// Convert 64-bit unsigned int to double
INLINE double ul2d(s64 num) { return (double)num; }
INLINE double uint642double(u64 num) { return (double)num; }

// Convert signed fixed point to double
//  e = number of bits of fractional part (binary exponent)
INLINE double fix2double(s32 num, int e) { return ldexp((double)num, -e); }

// Convert unsigned fixed point to double
//  e = number of bits of fractional part (binary exponent)
INLINE double ufix2double(u32 num, int e) { return ldexp((double)num, -e); }

// Convert 64-bit signed fixed point to double
//  e = number of bits of fractional part (binary exponent)
INLINE double fix642double(s64 num, int e) { return ldexp((double)num, -e); }

// Convert 64-bit unsigned fixed point to double
//  e = number of bits of fractional part (binary exponent)
INLINE double ufix642double(u64 num, int e) { return ldexp((double)num, -e); }

// ==== convert double to integer

// Convert double to signed int, rounding to zero (C-style int conversion)
INLINE s32 d2iz(double num) { return (s32)num; }
INLINE s32 double2int_z(double num) { return (s32)num; }

// Convert double to signed int, rounding down
INLINE s32 d2i(double num) { return (s32)floor(num); }
INLINE s32 double2int(double num) { return (s32)floor(num); }

// Convert double to unsigned int, rounding down
INLINE u32 d2ui(double num)
{
	s64 nn = (s64)num;
	if (nn < 0) return 0;
	if (nn > 0xffffffffull) return 0xfffffffful;
	return (u32)nn;
}
INLINE u32 double2uint(double num) { return d2ui(num); }

// Convert double to 64-bit signed int, rounding to zero (C-style int conversion)
INLINE s64 d2lz(double num)
{
	u64 n = *(u64*)&num;
	if ((s64)n >= 0) // num >= 0
	{
		if (n >= 0x43e0000000000000ull) return 0x7fffffffffffffffll; // overflow
	}
	else // num < 0
	{
		if (n >= 0xc3e0000000000000ull) return 0x8000000000000000ll; // underflow
	}
	return (s64)num;
}
INLINE s64 double2int64_z(double num) { return d2lz(num); }

// Convert double to 64-bit signed int, rounding down
INLINE s64 d2l(double num) { return d2lz(floor(num)); }
INLINE s64 double2int64(double num) { return d2l(num); }

// Convert double to 64-bit unsigned int, rounding down
INLINE u64 d2ul(double num)
{
	if (num < 0) return 0;
	return (u64)num;
}
INLINE u64 double2uint64(double num) { return d2ul(num); }

// Convert double to signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE s32 double2fix(double num, int e) { return d2i(ldexp(num, e)); }

// Convert double to unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE u32 double2ufix(double num, int e) { return d2ui(ldexp(num, e)); }

// Convert double to 64-bit signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE s64 double2fix64(double num, int e) { return d2l(ldexp(num, e)); }

// Convert double to 64-bit unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE u64 double2ufix64(double num, int e) { return d2ul(ldexp(num, e)); }

// ==== scientific functions

// Square root
double sqrt(double x);

// sine in radians
double sin(double x);

// sine in degrees
INLINE double sin_deg(double x) { return sin(deg2rad(x)); }

// cosine in radians
double cos(double x);

// cosine in degrees
INLINE double cos_deg(double x) { return cos(deg2rad(x)); }

// sine-cosine in radians
void sincos(double x, double* psin, double* pcos);

// sine-cosine in degrees
INLINE void sincos_deg(double x, double* psin, double* pcos) { sincos(deg2rad(x), psin, pcos); }

// tangent in radians
double tan(double x);

// tangent in degrees
INLINE double tan_deg(double x) { return tan(deg2rad(x)); }

// cotangent
INLINE double cotan(double x) { return drec(tan(x)); }

// cotangent in degrees
INLINE double cotan_deg(double x) { return cotan(deg2rad(x)); }

// arc sine in radians
double asin(double x);

// arc sine in degrees
INLINE double asin_deg(double x) { return rad2deg(asin(x)); }

// arc cosine in radians
double acos(double x);

// arc cosine in degrees
INLINE double acos_deg(double x) { return rad2deg(acos(x)); }

// arc tangent in radians
double atan(double x);

// arc tangent in degrees
INLINE double atan_deg(double x) { return rad2deg(atan(x)); }

// arc cotangent in radians
INLINE double acotan(double x) { return PI/2 - atan(x); }

// arc cotangent in degrees
INLINE double acotan_deg(double x) { return 90.0 - atan_deg(x); }

// arc tangent of y/x in radians
double atan2(double y, double x);

// arc tangent of y/x in degrees
INLINE double atan2_deg(double y, double x) { return rad2deg(atan2(y, x)); }

// Natural exponent
double exp(double x);

// Natural logarithm
double log(double x);

// exponent with base 10
double exp10(double x);

// power by integer, x^y
INLINE double powint(double x, int y) { return pow(x, y); }

#endif // USE_DOUBLELIBC

// ----------------------------------------------------------------------------
//                               Alternatives
// ----------------------------------------------------------------------------

// compose floating point with magnitude of 'num' and sign of 'sign'
INLINE double copysignd(double num, double sign) { return copysign(num, sign); }

// multiply number by power of 2 (num * 2^exp)
INLINE double ldexpd(double num, int exp) { return ldexp(num, exp); }

// round number towards zero
INLINE double truncd(double num) { return trunc(num); }

// round number to nearest integer
INLINE double roundd(double num) { return round(num); }

// round number down to integer
INLINE double floord(double num) { return floor(num); }

// round number up to integer
INLINE double ceild(double num) { return ceil(num); }

// Square root
INLINE double sqrtd(double x) { return sqrt(x); }

// sine in radians
INLINE double sind(double x) { return sin(x); }

// cosine in radians
INLINE double cosd(double x) { return cos(x); }

// sine-cosine in radians
INLINE void sincosd(double x, double* psin, double* pcos) { sincos(x, psin, pcos); }

// tangent in radians
INLINE double tand(double x) { return tan(x); }

// arc sine in radians
INLINE double asind(double x) { return asin(x); }

// arc cosine in radians
INLINE double acosd(double x) { return acos(x); }

// arc tangent in radians
INLINE double atand(double x) { return atan(x); }

// arc tangent of y/x in radians
INLINE double atan2d(double y, double x) { return atan2(y, x); }

// hyperbolic sine
INLINE double sinhd(double x) { return sinh(x); }

// hyperbolic cosine
INLINE double coshd(double x) { return cosh(x); }

// hyperbolic tangent
INLINE double tanhd(double x) { return tanh(x); }

// inverse hyperbolic sine
INLINE double asinhd(double x) { return asinh(x); }

// inverse hyperbolic cosine
INLINE double acoshd(double x) { return acosh(x); }

// inverse hyperbolic tangent
INLINE double atanhd(double x) { return atanh(x); }

// Natural exponent
INLINE double expd(double x) { return exp(x); }

// Natural logarithm
INLINE double logd(double x) { return log(x); }

// exponent with base 2
INLINE double exp2d(double x) { return exp2(x); }

// logarithm with base 2
INLINE double log2d(double x) { return log2(x); }

// exponent with base 10
INLINE double exp10d(double x) { return exp10(x); }

// logarithm with base 10
INLINE double log10d(double x) { return log10(x); }

// exp(x) - 1
INLINE double expm1d(double x) { return expm1(x); }

// log(x + 1)
INLINE double log1pd(double x) { return log1p(x); }

// x*y + z
INLINE double fmad(double x, double y, double z) { return fma(x, y, z); }

// power by integer, x^y
INLINE double powintd(double x, int y) { return powint(x, y); }

// power x^y
INLINE double powd(double x, double y) { return pow(x, y); }

// square root of sum of squares (hypotenuse), sqrt(x*x + y*y)
INLINE double hypotd(double x, double y) { return hypot(x, y); }

// cube root, sqrt3(x), x^(1/3)
INLINE double cbrtd(double x) { return cbrt(x); }

// get remainder of division x/y, rounded towards zero
//  fmod (x, y) = x - tquote*y, where tquote is truncated (i.e. rounded towards zero) result of x/y
INLINE double fmodd(double x, double y) { return fmod(x, y); }

// compute remainder and quotient of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
INLINE double remquod(double x, double y, int* quo) { return remquo(x, y, quo); }

// get remainder of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
INLINE double remainderd(double x, double y) { return remainder(x, y); }
INLINE double dremd(double x, double y) { return drem(x, y); } // obsolete synonym

#endif  // USE_DOUBLE		// use double support 1=in RAM, 2=in Flash

#if USE_FLOAT || USE_DOUBLE	// use float or double support 1=in RAM, 2=in Flash
#if (RP2040 || (RP2350 && !RISCV)) && !USE_DOUBLELIBC
// Convert double to float
float d2f(double num);
float double2float(double num);
#else // RP2040
// Convert double to float
INLINE float d2f(double num) { return (float)num; }
INLINE float double2float(double num) { return (float)num; }
#endif // RP2040
#endif // USE_FLOAT || USE_DOUBLE	// use float or double support

#ifdef __cplusplus
}
#endif

#endif // _SDK_DOUBLE_H
