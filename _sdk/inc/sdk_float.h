
// ****************************************************************************
//
//                                    Float
//                             Single-floating-point
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/

// Format of float number:
//
//  3 2        1         0
//  10987654321098765432109876543210
//  S\--EXP-/\----------MANT-------/
//
// S ... sign of mantissa, bit position 31 (1=negative, 0=positive)
// EXP ... unsigned biased exponent 8 bits, bit position 23..30, bias 127
// MANT ... mantissa (fraction) 23 bit, bit position 0..22, with hidden bit '1' on position 23

#ifndef _SDK_FLOAT_H
#define _SDK_FLOAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define FLOAT_MANT_BITS		23		// number of bits of mantissa
#define FLOAT_MANT_MASK		0x007fffff	// mask of mantissa
#define FLOAT_EXP_BITS		8		// number of bits of exponent
#define FLOAT_EXP_MASK		0xff		// mask of exponent value
#define FLOAT_EXP_BIAS		127		// bias of exponent (= value 1.0)
#define FLOAT_EXP_INF		0xff		// exponent infinity (overflow)

#if USE_FLOAT		// use float support 1=in RAM, 2=in Flash

#if RP2040 && !USE_FLOATLIBC
// initialize floating-point service
void FloatInit();
#endif // RP2040

typedef union { float f; u32 n; } tmp_float_u32;

// retype u32 to float
INLINE float u32float(u32 n) { tmp_float_u32 tmp; tmp.n = n; return tmp.f; }

// retype float to number
INLINE u32 floatu32(float f) { tmp_float_u32 tmp; tmp.f = f; return tmp.n; }

// get float exponent
INLINE int floatgetexp(float f) { return (int)((floatu32(f) << 1) >> (FLOAT_MANT_BITS+1)); }

// check if float is infinity
INLINE Bool floatisinf(float f) { return floatgetexp(f) == FLOAT_EXP_INF; }

// check if float is zero
INLINE Bool floatiszero(float f) { return floatgetexp(f) == 0; }

// ==== common auxiliary functions, not in libc
// sdk_float.c

// compose floating point with magnitude of 'num' and sign of 'sign'
float copysignf(float num, float sign);

// check if number is an integer
Bool isintf(float num);

// check if number is odd integer (must be an integer ..., -3, -1, 1, 3, 5,..)
Bool isoddintf(float num);

// check if number is power of 2
Bool ispow2f(float num);

// round to given number of significant digits (digits<=0 to use default number of digits)
// @TODO: probably will be deleted (accuracy cannot be guaranteed)
//float rounddigf(float x, int digits);

// Check if comparison is unordered (either input is NaN)
Bool fcmpun(float x, float y);

// convert degrees to radians
INLINE float deg2radf(float x) { return x * (float)(PI/180); }

// convert radians to degrees
INLINE float rad2degf(float x) { return x * (float)(180/PI); }

// ----------------------------------------------------------------------------
//              Common functions - faster alternatives to libc
// ----------------------------------------------------------------------------
// sdk_float.c

#if !USE_FLOATLIBC

// ==== auxiliary functions

// multiply number by power of 2 (num * 2^exp)
float ldexpf(float num, int exp);

// absolute value
float absf(float x);

// ==== rounding

// round number towards zero
float truncf(float num);

// round number to nearest integer
float roundf(float num);

// round number down to integer
float floorf(float num);

// round number up to integer
float ceilf(float num);

// ==== scientific functions

// sine in degrees
float sinf_deg(float x);

// cosine in degrees
float cosf_deg(float x);

// sine-cosine in degrees
void sincosf_deg(float x, float* psin, float* pcos);

// tangent in degrees
float tanf_deg(float x);

// cotangent in degrees
float cotanf_deg(float x);

// arc sine in radians
float asinf(float x);

// arc sine in degrees
float asinf_deg(float x);

// arc cosine in radians
float acosf(float x);

// arc cosine in degrees
float acosf_deg(float x);

// RISC-V: atanf cannot be implemented this way because atanf is internally called from atan2f in libc
//#if !RISCV
// arc tangent in radians
float atanf(float x);
//#endif

// arc tangent in degrees
float atanf_deg(float x);

// arc cotangent in radians
float acotanf(float x);

// arc cotangent in degrees
float acotanf_deg(float x);

// arc tangent of y/x in degrees
float atan2f_deg(float y, float x);

// hyperbolic sine
float sinhf(float x);

// hyperbolic cosine
float coshf(float x);

// hyperbolic tangent
float tanhf(float x);

// inverse hyperbolic sine
float asinhf(float x);

// inverse hyperbolic cosine
float acoshf(float x);

// inverse hyperbolic tangent
float atanhf(float x);

// exponent with base 2
float exp2f(float x);

// logarithm with base 2
float log2f(float x);

// exponent with base 10
float exp10f(float x);

// logarithm with base 10
float log10f(float x);

// expf(x) - 1
float expm1f(float x);

// logf(x + 1)
// - This function can be very inaccurate
float log1pf(float x);

// x*y + z
float fmaf(float x, float y, float z);

// power by integer, x^y
float powintf(float x, int y);

// power x^y
float powf(float x, float y);

// square root of sum of squares (hypotenuse), sqrt(x*x + y*y)
float hypotf(float x, float y);

// cube root, sqrt3(x), x^(1/3)
float cbrtf(float x);

// ==== basic arithmetic

// get remainder of division x/y, rounded towards zero
//  fmod (x, y) = x - tquote*y, where tquote is truncated (i.e. rounded towards zero) result of x/y
float fmodf(float x, float y);

// compute remainder and quotient of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
float remquof(float x, float y, int* quo);

// get remainder of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
float remainderf(float x, float y);
float dremf(float x, float y); // obsolete synonym

#endif // !USE_FLOATLIBC

// ----------------------------------------------------------------------------
//                             RP2040 functions
// ----------------------------------------------------------------------------
// sdk_float_m0.S

#if RP2040 && !USE_FLOATLIBC

// ==== basic arithmetic

// Addition, x + y
float fadd(float x, float y);

// Subtraction, x - y
float fsub(float x, float y);

// Multiplication, x * y
float fmul(float x, float y);

// Multiply by unsigned integer u16, x * n
//float fmulu16(float x, u16 n);

// Square, x^2 
float fsqr(float x);

// Division, x / y
float fdiv(float x, float y);

// Divide by unsigned integer u16, x / n
//float fdivu16(float x, u16 n);

// Reciprocal 1 / x
float frec(float x);

// ==== comparison

// Compare, x ? y
// Returns: 0 if x==y, -1 if x<y, +1 if x>y
s8 fcmp(float x, float y);

// Compare if x==y
Bool fcmpeq(float x, float y);

// Compare if x<y
Bool fcmplt(float x, float y);

// Compare if x<=y
Bool fcmple(float x, float y);

// Compare if x>=y
Bool fcmpge(float x, float y);

// Compare if x>y
Bool fcmpgt(float x, float y);

// ==== convert integer to float

// Convert signed int to float
float i2f(s32 num);
float int2float(s32 num);

// Convert unsigned int to float
float ui2f(u32 num);
float uint2float(u32 num);

// Convert 64-bit signed int to float
float l2f(s64 num);
float int642float(s64 num);

// Convert 64-bit unsigned int to float
float ul2f(s64 num);
float uint642float(u64 num);

// Convert signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
float fix2float(s32 num, int e);

// Convert unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
float ufix2float(u32 num, int e);

// Convert 64-bit signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
float fix642float(s64 num, int e);

// Convert 64-bit unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
float ufix642float(u64 num, int e);

// ==== convert float to integer

// Convert float to signed int, rounding to zero (C-style int conversion)
s32 f2iz(float num);
s32 float2int_z(float num);

// Convert float to signed int, rounding down
s32 f2i(float num);
s32 float2int(float num);

// Convert float to unsigned int, rounding down
u32 f2ui(float num);
u32 float2uint(float num);

// Convert float to 64-bit signed int, rounding to zero (C-style int conversion)
s64 f2lz(float num);
s64 float2int64_z(float num);

// Convert float to 64-bit signed int, rounding down
s64 f2l(float num);
s64 float2int64(float num);

// Convert float to 64-bit unsigned int, rounding down
u64 f2ul(float num);
u64 float2uint64(float num);

// Convert float to signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
s32 float2fix(float num, int e);

// Convert float to unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
u32 float2ufix(float num, int e);

// Convert float to 64-bit signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
s64 float2fix64(float num, int e);

// Convert float to 64-bit unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
u64 float2ufix64(float num, int e);

// ==== scientific functions

// Square root
float sqrtf(float x);

// sine in radians
float sinf(float x);

// cosine in radians
float cosf(float x);

// sine-cosine in radians
void sincosf(float x, float* psin, float* pcos);

// tangent in radians
float tanf(float x);

// cotangent in radians
float cotanf(float x);

// arc tangent of y/x in radians
float atan2f(float y, float x);

// Natural exponent
float expf(float x);

// Natural logarithm
float logf(float x);

#endif // RP2040 && !USE_FLOATLIBC

// ----------------------------------------------------------------------------
//                             RP2350-ARM functions
// ----------------------------------------------------------------------------

#if RP2350 && !USE_FLOATLIBC && !RISCV

// ==== basic arithmetic

#if USE_M33_VFP	// 1=enable VFP floating point instructions on RP2350-ARM, 0=emulate with DCP coprocessor
// Addition, x + y
INLINE float fadd(float x, float y) { return x+y; }

// Subtraction, x - y
INLINE float fsub(float x, float y) { return x-y; }

// Multiplication, x * y
INLINE float fmul(float x, float y) { return x*y; }

// Division, x / y
INLINE float fdiv(float x, float y) { return x/y; }
#else // USE_M33_VFP
// Addition, x + y
float fadd(float x, float y);

// Subtraction, x - y
float fsub(float x, float y);

// Multiplication, x * y
float fmul(float x, float y);

// Division, x / y
float fdiv(float x, float y);
#endif // USE_M33_VFP

// Multiply by unsigned integer u16, x * n
//INLINE float fmulu16(float x, u16 n) { return x * n; }

// Divide by unsigned integer u16, x / n
//INLINE float fdivu16(float x, u16 n) { return x / n; }

// Square, x^2 
INLINE float fsqr(float x) { return x * x; }

// Reciprocal 1 / x
INLINE float frec(float x) { return 1.0f / x; }

// ==== comparison

// Compare, x ? y ... Returns: 0 if x==y, -1 if x<y, +1 if x>y
INLINE s8 fcmp(float x, float y) { return (x == y) ? 0.0f : ((x < y) ? -1.0f : +1.0f); }

// Compare if x==y
Bool fcmpeq(float x, float y);

// Compare if x<y
Bool fcmplt(float x, float y);

// Compare if x<=y
Bool fcmple(float x, float y);

// Compare if x>=y
Bool fcmpge(float x, float y);

// Compare if x>y
Bool fcmpgt(float x, float y);

// ==== convert integer to float

// Convert signed int to float
float i2f(s32 num);
float int2float(s32 num);

// Convert unsigned int to float
float ui2f(u32 num);
float uint2float(u32 num);

// Convert 64-bit signed int to float
float l2f(s64 num);
float int642float(s64 num);

// Convert 64-bit unsigned int to float
float ul2f(s64 num);
float uint642float(u64 num);

// Convert signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
float fix2float(s32 num, int e);

// Convert unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
float ufix2float(u32 num, int e);

// Convert 64-bit signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
float fix642float(s64 num, int e);

// Convert 64-bit unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
float ufix642float(u64 num, int e);

// ==== convert float to integer

// Convert float to signed int, rounding to zero (C-style int conversion)
s32 f2iz(float num);
s32 float2int_z(float num);

// Convert float to signed int, rounding down
INLINE s32 f2i(float num) { return (s32)floorf(num); }
INLINE s32 float2int(float num) { return f2i(num); }

// Convert float to unsigned int, rounding down
u32 f2ui(float num);
u32 float2uint(float num);

// Convert float to 64-bit signed int, rounding to zero (C-style int conversion)
s64 f2lz(float num);
s64 float2int64_z(float num);

// Convert float to 64-bit signed int, rounding down
INLINE s64 f2l(float num) { return f2lz(floorf(num)); }
INLINE s64 float2int64(float num) { return f2l(num); }

// Convert float to 64-bit unsigned int, rounding down
u64 f2ul(float num);
u64 float2uint64(float num);

// Convert float to signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE s32 float2fix(float num, int e) { return f2i(ldexpf(num, e)); }

// Convert float to unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE u32 float2ufix(float num, int e) { return f2ui(ldexpf(num, e)); }

// Convert float to 64-bit signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE s64 float2fix64(float num, int e) { return f2l(ldexpf(num, e)); }

// Convert float to 64-bit unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
u64 float2ufix64(float num, int e);

// ==== scientific functions

// Square root
float sqrtf(float x);

// sine in radians
float sinf(float x);

// cosine in radians
float cosf(float x);

// sine-cosine in radians
void sincosf(float x, float* psin, float* pcos);

// tangent in radians
float tanf(float x);

// cotangent in radians
float cotanf(float x);

// arc tangent of y/x in radians
float atan2f(float y, float x);

// Natural exponent
float expf(float x);

// Natural logarithm
float logf(float x);

#endif // RP2350 && !USE_FLOATLIBC && !RISCV

// ----------------------------------------------------------------------------
//                             RP2350-RISCV functions
// ----------------------------------------------------------------------------

#if RP2350 && !USE_FLOATLIBC && RISCV

// ==== basic arithmetic

// Addition, x + y
float __addsf3(float x, float y);
float fadd(float x, float y);

// Subtraction, x - y
float __subsf3(float x, float y);
float fsub(float x, float y);

// Multiplication, x * y
float __mulsf3(float x, float y);
float fmul(float x, float y);

// Multiply by unsigned integer u16, x * n
//INLINE float fmulu16(float x, u16 n) { return x * n; }

// Square, x^2 
INLINE float fsqr(float x) { return x * x; }

// Division, x / y
float __divsf3(float x, float y);
float fdiv(float x, float y);

// Divide by unsigned integer u16, x / n
//INLINE float fdivu16(float x, u16 n) { return x / n; }

// Reciprocal 1 / x
INLINE float frec(float x) { return 1.0f / x; }

// ==== comparison

// Compare, x ? y ... Returns: 0 if x==y, -1 if x<y, +1 if x>y
INLINE s8 fcmp(float x, float y) { return (x == y) ? 0.0f : ((x < y) ? -1.0f : +1.0f); }

// Compare if x==y
INLINE Bool fcmpeq(float x, float y) { return x == y; }

// Compare if x<y
INLINE Bool fcmplt(float x, float y) { return x < y; }

// Compare if x<=y
INLINE Bool fcmple(float x, float y) { return x <= y; }

// Compare if x>=y
INLINE Bool fcmpge(float x, float y) { return x >= y; }

// Compare if x>y
INLINE Bool fcmpgt(float x, float y) { return x > y; }

// ==== convert integer to float

// Convert signed int to float
float i2f(s32 num);
float int2float(s32 num);

// Convert unsigned int to float
float ui2f(u32 num);
float uint2float(u32 num);

// Convert 64-bit signed int to float
float l2f(s64 num);
float int642float(s64 num);

// Convert 64-bit unsigned int to float
float ul2f(s64 num);
float uint642float(u64 num);

// Convert signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
float fix2float(s32 num, int e);

// Convert unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
float ufix2float(u32 num, int e);

// Convert 64-bit signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
float fix642float(s64 num, int e);

// Convert 64-bit unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
float ufix642float(u64 num, int e);

// ==== convert float to integer

// Convert float to signed int, rounding to zero (C-style int conversion)
s32 f2iz(float num);
s32 float2int_z(float num);

// Convert float to signed int, rounding down
s32 f2i(float num);
s32 float2int(float num);

// Convert float to unsigned int, rounding down
u32 f2ui(float num);
u32 float2uint(float num);


// Convert float to 64-bit signed int, rounding to zero (C-style int conversion)
s64 f2lz(float num);
s64 float2int64_z(float num);

// Convert float to 64-bit signed int, rounding down
s64 f2l(float num);
s64 float2int64(float num);

// Convert float to 64-bit unsigned int, rounding down
u64 f2ul(float num);
u64 float2uint64(float num);

// Convert float to signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
s32 float2fix(float num, int e);

// Convert float to unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
u32 float2ufix(float num, int e);

// Convert float to 64-bit signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
s64 float2fix64(float num, int e);

// Convert float to 64-bit unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
u64 float2ufix64(float num, int e);

// ==== scientific functions

// Square root
float sqrtf(float x);

// sine in radians
float sinf(float x);

// cosine in radians
float cosf(float x);

// sine-cosine in radians
void sincosf(float x, float* psin, float* pcos);

// tangent in radians
float tanf(float x);

// cotangent in radians
float cotanf(float x);

// arc tangent of y/x in radians
float atan2f(float y, float x);

// Natural exponent
float expf(float x);

// Natural logarithm
float logf(float x);

#endif // RP2350 && !USE_FLOATLIBC && RISCV

// ----------------------------------------------------------------------------
//                   LIBC not implemented functions
// ----------------------------------------------------------------------------

#if USE_FLOATLIBC

// ==== auxiliary functions

// multiply number by power of 2 (num * 2^exp)
float ldexpf(float num, int exp);

// absolute value
INLINE float absf(float x) { return fabsf(x); }

// ==== basic arithmetic

// Addition, x + y
INLINE float fadd(float x, float y) { return x + y; }

// Subtraction, x - y
INLINE float fsub(float x, float y) { return x - y; }

// Multiplication, x * y
INLINE float fmul(float x, float y) { return x * y; }

// Multiply by unsigned integer u16, x * n
//INLINE float fmulu16(float x, u16 n) { return x * n; }

// Square, x^2 
INLINE float fsqr(float x) { return x * x; }

// Division, x / y
INLINE float fdiv(float x, float y) { return x / y; }

// Divide by unsigned integer u16, x / n
//INLINE float fdivu16(float x, u16 n) { return x / n; }

// Reciprocal 1 / x
INLINE float frec(float x) { return 1.0f / x; }

// ==== comparison

// Compare, x ? y ... Returns: 0 if x==y, -1 if x<y, +1 if x>y
INLINE s8 fcmp(float x, float y) { return (x == y) ? 0.0f : ((x < y) ? -1.0f : +1.0f); }

// Compare if x==y
INLINE Bool fcmpeq(float x, float y) { return x == y; }

// Compare if x<y
INLINE Bool fcmplt(float x, float y) { return x < y; }

// Compare if x<=y
INLINE Bool fcmple(float x, float y) { return x <= y; }

// Compare if x>=y
INLINE Bool fcmpge(float x, float y) { return x >= y; }

// Compare if x>y
INLINE Bool fcmpgt(float x, float y) { return x > y; }

// ==== convert integer to float

// Convert signed int to float
INLINE float i2f(s32 num) { return (float)num; }
INLINE float int2float(s32 num) { return (float)num; }

// Convert unsigned int to float
INLINE float ui2f(u32 num) { return (float)num; }
INLINE float uint2float(u32 num) { return (float)num; }

// Convert 64-bit signed int to float
INLINE float l2f(s64 num) { return (float)num; }
INLINE float int642float(s64 num) { return (float)num; }

// Convert 64-bit unsigned int to float
INLINE float ul2f(s64 num) { return (float)num; }
INLINE float uint642float(u64 num) { return (float)num; }

// Convert signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
INLINE float fix2float(s32 num, int e) { return ldexpf((float)num, -e); }

// Convert unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
INLINE float ufix2float(u32 num, int e) { return ldexpf((float)num, -e); }

// Convert 64-bit signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
INLINE float fix642float(s64 num, int e) { return ldexpf((float)num, -e); }

// Convert 64-bit unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
INLINE float ufix642float(u64 num, int e) { return ldexpf((float)num, -e); }

// ==== convert float to integer

// Convert float to signed int, rounding to zero (C-style int conversion)
INLINE s32 f2iz(float num) { return (s32)num; }
INLINE s32 float2int_z(float num) { return (s32)num; }

// Convert float to signed int, rounding down
INLINE s32 f2i(float num) { return (s32)floorf(num); }
INLINE s32 float2int(float num) { return (s32)floorf(num); }

// Convert float to unsigned int, rounding down
INLINE u32 f2ui(float num)
{
	s64 nn = (s64)num;
	if (nn < 0) return 0;
	if (nn > 0xffffffffull) return 0xfffffffful;
	return (u32)nn;
}
INLINE u32 float2uint(float num) { return f2ui(num); }

// Convert float to 64-bit signed int, rounding to zero (C-style int conversion)
INLINE s64 f2lz(float num)
{
	u32 n = *(u32*)&num;
	if ((s32)n >= 0) // num >= 0
	{
		if (n >= 0x5f000000ul) return 0x7fffffffffffffffll; // overflow
	}
	else // num < 0
	{
		if (n >= 0xdf000000ul) return 0x8000000000000000ll; // underflow
	}
	return (s64)num;
}
INLINE s64 float2int64_z(float num) { return f2lz(num); }

// Convert float to 64-bit signed int, rounding down
INLINE s64 f2l(float num) { return f2lz(floorf(num)); }
INLINE s64 float2int64(float num) { return f2l(num); }

// Convert float to 64-bit unsigned int, rounding down
INLINE u64 f2ul(float num)
{
	if (num < 0) return 0;
	return (u64)num;
}
INLINE u64 float2uint64(float num) { return f2ul(num); }

// Convert float to signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE s32 float2fix(float num, int e) { return f2i(ldexpf(num, e)); }

// Convert float to unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE u32 float2ufix(float num, int e) { return f2ui(ldexpf(num, e)); }

// Convert float to 64-bit signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE s64 float2fix64(float num, int e) { return f2l(ldexpf(num, e)); }

// Convert float to 64-bit unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
INLINE u64 float2ufix64(float num, int e) { return f2ul(ldexpf(num, e)); }

// ==== scientific functions

// Square root
float sqrtf(float x);

// sine in radians
float sinf(float x);

// sine in degrees
float sinf_deg(float x);

// cosine in radians
float cosf(float x);

// cosine in degrees
float cosf_deg(float x);

// sine-cosine in radians
void sincosf(float x, float* psin, float* pcos);

// sine-cosine in degrees
void sincosf_deg(float x, float* psin, float* pcos);

// tangent in radians
float tanf(float x);

// tangent in degrees
float tanf_deg(float x);

// cotangent
float cotanf(float x);

// cotangent in degrees
float cotanf_deg(float x);

// arc sine in degrees
float asinf_deg(float x);

// arc cosine in degrees
float acosf_deg(float x);

// arc tangent of y/x in radians
float atan2f(float y, float x);

// arc tangent in degrees
float atanf_deg(float x);

// arc cotangent in radians
float acotanf(float x);

// arc cotangent in degrees
float acotanf_deg(float x);

// arc tangent of y/x in degrees
float atan2f_deg(float y, float x);

// Natural exponent
float expf(float x);

// Natural logarithm
float logf(float x);

// exponent with base 10
float exp10f(float x);

// power by integer, x^y
INLINE float powintf(float x, int y) { return powf(x, y); }

#endif // USE_FLOATLIBC

#endif // USE_FLOAT		// use float support

#if USE_FLOAT || USE_DOUBLE	// use float or double support 1=in RAM, 2=in Flash
#if (RP2040 || (RP2350 && !RISCV)) && !USE_DOUBLELIBC
// Convert float to double
double f2d(float num);
double float2double(float num);
#else // RP2040
// Convert float to double
INLINE double f2d(float num) { return (double)num; }
INLINE double float2double(float num) { return (double)num; }
#endif // RP2040
#endif // USE_FLOAT || USE_DOUBLE	// use float or double support

#ifdef __cplusplus
}
#endif

#endif // _SDK_FLOAT_H
