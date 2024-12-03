
// ****************************************************************************
//
//                          Reference functions
//
// ****************************************************************************

#include "../include.h"

// ============= reference functions

// compose floating point with magnitude of 'num' and sign of 'sign'
float Ref_copysignf(float num, float sign)
{
	if (Test_GetSignF(num) ^ Test_GetSignF(sign)) num = -num;
	return num;
}

double Ref_copysign(double num, double sign)
{
	if (Test_GetSignD(num) ^ Test_GetSignD(sign)) num = -num;
	return num;
}

// check if number is an integer
Bool Ref_isintf(float num)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	return REALNAME(IsInt)(&t);
}

Bool Ref_isintd(double num)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	return REALNAME(IsInt)(&t);
}

// check if number is odd integer (must be an integer ..., -3, -1, 1, 3, 5,..)
Bool Ref_isoddintf(float num)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	return REALNAME(IsOddInt)(&t);
}

Bool Ref_isoddintd(double num)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	return REALNAME(IsOddInt)(&t);
}

// check if number is power of 2
Bool Ref_ispow2f(float num)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	return REALNAME(IsPow2)(&t);
}

Bool Ref_ispow2d(double num)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	return REALNAME(IsPow2)(&t);
}

// multiply number by power of 2 (num * 2^exp)
float Ref_ldexpf(float num, int exp)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	REALNAME(Pow2)(&t, exp);
	return REALNAME(ToFloat)(&t);
}

double Ref_ldexp(double num, int exp)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	REALNAME(Pow2)(&t, exp);
	return REALNAME(ToDouble)(&t);
}

// Addition, x + y
float Ref_fadd(float x, float y)
{
	if ((Test_IsPOverF(x) && Test_IsMOverF(y)) ||
		(Test_IsMOverF(x) && Test_IsPOverF(y)))
#if RP2350
#if USE_FLOATLIBC
#if RISCV
			return Test_QNanF();
#else
			return Test_IsPOverF(x) ? Test_QNanF() : Test_IndF();
#endif
#else
			return Test_IndF();
#endif
#else
			return Test_PInfF();
#endif
	if (Test_IsOverF(x)) return x;
	if (Test_IsOverF(y)) return y;
	REAL tr, xr, yr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(FromFloat)(&yr, y);
	REALNAME(Add)(&tr, &xr, &yr);
	return REALNAME(ToFloat)(&tr);
}

double Ref_dadd(double x, double y)
{
	if ((Test_IsPOverD(x) && Test_IsMOverD(y)) ||
		(Test_IsMOverD(x) && Test_IsPOverD(y)))
#if RP2350
#if USE_DOUBLELIBC
#if RISCV
			return Test_QNanD();
#else
			return Test_IsPOverD(x) ? Test_QNanD() : Test_IndD();
#endif
#else // USE_DOUBLELIBC
#if RISCV
			return Test_QNanD();
#else // RISCV
			return Test_IndD();
#endif // RISCV
#endif // USE_DOUBLELIBC
#else // RP2350
			return Test_PInfD();
#endif // RP2350
	if (Test_IsOverD(x)) return x;
	if (Test_IsOverD(y)) return y;
	REAL tr, xr, yr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(FromDouble)(&yr, y);
	REALNAME(Add)(&tr, &xr, &yr);
	return REALNAME(ToDouble)(&tr);
}

// Subtraction, x - y
float Ref_fsub(float x, float y)
{
	if ((Test_IsPOverF(x) && Test_IsPOverF(y)) ||
		(Test_IsMOverF(x) && Test_IsMOverF(y)))
#if RP2350
#if USE_FLOATLIBC
#if RISCV
			return Test_QNanF();
#else
			return Test_IsPOverF(x) ? Test_QNanF() : Test_IndF();
#endif
#else
			return Test_IndF();
#endif
#else
			return Test_PInfF();
#endif
	if (Test_IsOverF(y)) return -y;
	if (Test_IsOverF(x)) return x;
	REAL tr, xr, yr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(FromFloat)(&yr, y);
	REALNAME(Sub)(&tr, &xr, &yr);
	return REALNAME(ToFloat)(&tr);
}

double Ref_dsub(double x, double y)
{
	if ((Test_IsPOverD(x) && Test_IsPOverD(y)) ||
		(Test_IsMOverD(x) && Test_IsMOverD(y)))
#if RP2350
#if USE_DOUBLELIBC
#if RISCV
			return Test_QNanD();
#else
			return Test_IsPOverD(x) ? Test_QNanD() : Test_IndD();
#endif
#else
#if RISCV
			return Test_QNanD();
#else // RISCV
			return Test_IndD();
#endif // RISCV
#endif
#else
			return Test_PInfD();
#endif
	if (Test_IsOverD(y)) return -y;
	if (Test_IsOverD(x)) return x;
	REAL tr, xr, yr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(FromDouble)(&yr, y);
	REALNAME(Sub)(&tr, &xr, &yr);
	return REALNAME(ToDouble)(&tr);
}

// Multiplication, x * y
float Ref_fmul(float x, float y)
{
	if ((Test_IsOverF(x) && Test_IsZeroF(y)) ||
		(Test_IsZeroF(x) && Test_IsOverF(y)))
#if RP2350
#if USE_FLOATLIBC
#if RISCV
			return Test_QNanF();
#else // RISCV
		{
			if (Test_IsOverF(x))
				return Test_GetSignF(x) ? Test_IndF() : Test_QNanF();
			else
				return Test_GetSignF(y) ? Test_IndF() : Test_QNanF();
		}
#endif // RISCV
#else // FLOATLIBC
			return Test_IndF();
#endif // DOUBLELIBC
#elif USE_FLOATLIBC // RP2040 FLOATLIBC
			return Test_QNanF();
#else // RP2040 !FLOATLIBC
			return (Test_GetSignF(x) ^ Test_GetSignF(y)) ? Test_MInfF() : Test_PInfF();
#endif
	REAL tr, xr, yr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(FromFloat)(&yr, y);
	REALNAME(Mul)(&tr, &xr, &yr);
	return REALNAME(ToFloat)(&tr);
}

double Ref_dmul(double x, double y)
{
	if ((Test_IsOverD(x) && Test_IsZeroD(y)) ||
		(Test_IsZeroD(x) && Test_IsOverD(y)))
#if RP2350
#if USE_DOUBLELIBC
#if RISCV
			return Test_QNanD();
#else // RISCV
		{
			if (Test_IsOverD(x))
				return Test_GetSignD(x) ? Test_IndD() : Test_QNanD();
			else
				return Test_GetSignD(y) ? Test_IndD() : Test_QNanD();
		}
#endif // RISCV
#else // DOUBLELIBC

			return Test_QNanD();
//			return Test_IndD();

#endif // DOUBLELIBC
#elif USE_DOUBLELIBC // RP2040 DOUBLELIBC
			return Test_QNanD();
#else // RP2040 !DOUBLELIBX
			return (Test_GetSignD(x) ^ Test_GetSignD(y)) ? Test_MInfD() : Test_PInfD();
#endif
	REAL tr, xr, yr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(FromDouble)(&yr, y);
	REALNAME(Mul)(&tr, &xr, &yr);
	return REALNAME(ToDouble)(&tr);
}

// Square, x^2 
float Ref_fsqr(float x)
{
	REAL xr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(Sqr)(&xr);
	return REALNAME(ToFloat)(&xr);
}

double Ref_dsqr(double x)
{
	REAL xr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(Sqr)(&xr);
	return REALNAME(ToDouble)(&xr);
}

// Division, x / y
float Ref_fdiv(float x, float y)
{
	if ((Test_IsOverF(x) && Test_IsOverF(y)) ||
		(Test_IsZeroF(x) && Test_IsZeroF(y)))
#if RP2350
#if RISCV
#if USE_FLOATLIBC
			return Test_QNanF();
#else
			return (Test_GetSignF(x) ^ Test_GetSignF(y)) ? Test_MInfF() : Test_PInfF();
#endif
#else // RISCV
#if USE_FLOATLIBC
		{
			if (Test_IsOverF(y))
				return Test_GetSignF(y) ? Test_IndF() : Test_QNanF();
			else
				return Test_GetSignF(x) ? Test_IndF() : Test_QNanF();
		}
#else // USE_FLOATLIBC
			return Test_IndF();
#endif // USE_FLOATLIBC
#endif // RISCV
#elif USE_FLOATLIBC
			return Test_QNanF();
#else
			return (Test_GetSignF(x) ^ Test_GetSignF(y)) ? Test_MInfF() : Test_PInfF();
#endif
	REAL tr, xr, yr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(FromFloat)(&yr, y);
	REALNAME(Div)(&tr, &xr, &yr);
	return REALNAME(ToFloat)(&tr);
}

double Ref_ddiv(double x, double y)
{
	if ((Test_IsOverD(x) && Test_IsOverD(y)) ||
		(Test_IsZeroD(x) && Test_IsZeroD(y)))
#if RP2350
#if RISCV
			return Test_QNanD();
#else // RISCV
#if USE_DOUBLELIBC
		{
			if (Test_IsOverD(x))
				return Test_GetSignD(x) ? Test_IndD() : Test_QNanD();
			else if (Test_IsOverD(y))
				return Test_GetSignD(y) ? Test_IndD() : Test_QNanD();
			else
				return Test_GetSignD(x) ? Test_IndD() : Test_QNanD();
		}
#else // USE_DOUBLELIBC
//#if RISCV
//			return Test_QNanD();
//#else // RISCV
			return Test_IndD();
//#endif // RISCV
#endif // USE_DOUBLELIBC
#endif // RISCV
#elif USE_DOUBLELIBC
			return Test_QNanD();
#else
			return (Test_GetSignD(x) ^ Test_GetSignD(y)) ? Test_MInfD() : Test_PInfD();
#endif
	REAL tr, xr, yr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(FromDouble)(&yr, y);
	REALNAME(Div)(&tr, &xr, &yr);
	return REALNAME(ToDouble)(&tr);
}

// Reciprocal 1 / x
float Ref_frec(float x)
{
	REAL xr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(Rec)(&xr);
	return REALNAME(ToFloat)(&xr);
}

double Ref_drec(double x)
{
	REAL xr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(Rec)(&xr);
	return REALNAME(ToDouble)(&xr);
}

// get remainder of division x/y, rounded towards zero
//  fmod (x, y) = x - tquote*y, where tquote is truncated (i.e. rounded towards zero) result of x/y
float Ref_fmodf(float x, float y)
{
#if RISCV
	if (Test_IsOverF(x) || Test_IsZeroF(y)) return Test_QNanF();
#else // RISCV
#if RP2350 && USE_FLOATLIBC
	if (Test_IsZeroF(y)) return Test_QNanF();
	if (Test_IsOverF(x)) return (Test_GetSignF(x) ^ Test_GetSignF(y)) ? Test_IndF() : Test_QNanF();
#else
	if (Test_IsOverF(x) || Test_IsZeroF(y)) return Test_PInfF();
#endif
#endif
	if (Test_GetExpF(x) < Test_GetExpF(y)) return x;
	REAL tr, xr, yr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(FromFloat)(&yr, y);
	REALNAME(ModTrunc)(&tr, &xr, &yr);
	return REALNAME(ToFloat)(&tr);
}

double Ref_fmod(double x, double y)
{
#if RISCV
	if (Test_IsOverD(x) || Test_IsZeroD(y)) return Test_QNanD();
#else // RISCV
#if RP2350 && USE_DOUBLELIBC
	if (Test_IsZeroD(y)) return Test_QNanD();
	if (Test_IsOverD(x)) return (Test_GetSignD(x) ^ Test_GetSignD(y)) ? Test_IndD() : Test_QNanD();
#else
	if (Test_IsOverD(x) || Test_IsZeroD(y)) return Test_PInfD();
#endif
#endif
	if (Test_GetExpD(x) < Test_GetExpD(y)) return x;
	REAL tr, xr, yr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(FromDouble)(&yr, y);
	REALNAME(ModTrunc)(&tr, &xr, &yr);
	return REALNAME(ToDouble)(&tr);
}

// compute remainder and quotient of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
float Ref_remquof(float x, float y, int* quo)
{
	if (quo != NULL) *quo = 0;
#if RISCV
	if (Test_IsOverF(x) || Test_IsZeroF(y))
			return Test_QNanF();
#else // RISCV
#if RP2350 && USE_FLOATLIBC
	if (Test_IsOverF(x) || Test_IsZeroF(y))
	{
		if (Test_IsOverF(x) && Test_IsZeroF(y))
			return (Test_GetSignF(x)) ? Test_IndF() : Test_QNanF();
		else
			return (Test_GetSignF(x) ^ Test_GetSignF(y)) ? Test_IndF() : Test_QNanF();
	}
#else
	if (Test_IsOverF(x) || Test_IsZeroF(y)) return Test_PInfF();
#endif
#endif
	if (Test_IsZeroF(x)) return 0;
	if (Test_IsOverF(y)) return x;
	if (Test_GetExpF(x) < Test_GetExpF(y) - 1) return x;

	REAL tr, tr2, xr, yr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(FromFloat)(&yr, y);
	REALNAME(Div)(&tr, &xr, &yr);	// t = x/y

	// rounding to even ... negative number
	if (REALNAME(IsNeg)(&tr))
	{
		REALNAME(Add)(&tr2, &tr, &REALNAME(ConstM05)); // + 0.5
		if (REALNAME(IsInt)(&tr2) && REALNAME(IsOddInt)(&tr2)) // odd integer
			REALNAME(IncFrom(&tr, &tr2));	// +1
		else
			REALNAME(Round)(&tr);		// rquote t = round(x/y)
	}

	// rounding to even ... negative number
	else
	{
		REALNAME(Add)(&tr2, &tr, &REALNAME(Const05));	// -0.5
		if (REALNAME(IsInt)(&tr2) && REALNAME(IsOddInt)(&tr2)) // odd integer
			REALNAME(DecFrom(&tr, &tr2));	// -1
		else
			REALNAME(Round)(&tr);		// rquote t = round(x/y)
	}

	if (quo != NULL) *quo = REALNAME(GetS32)(&tr);	// get result quotient rquote
	REALNAME(Mul)(&tr, &tr, &yr);	// t = rquote*y = round(x/y)*y
	REALNAME(Sub)(&xr, &xr, &tr);	// x = x - rquote*y
	return REALNAME(ToFloat)(&xr);
}

double Ref_remquo(double x, double y, int* quo)
{
	if (quo != NULL) *quo = 0;
#if RISCV
	if (Test_IsOverD(x) || Test_IsZeroD(y))
			return Test_QNanD();
#else // RISCV
#if RP2350 && USE_DOUBLELIBC
	if (Test_IsOverD(x) || Test_IsZeroD(y))
	{
		if (Test_IsOverD(x) && Test_IsZeroD(y))
			return (Test_GetSignD(x)) ? Test_IndD() : Test_QNanD();
		else
			return (Test_GetSignD(x) ^ Test_GetSignD(y)) ? Test_IndD() : Test_QNanD();
	}
#else
	if (Test_IsOverD(x) || Test_IsZeroD(y)) return Test_PInfD();
#endif
#endif
	if (Test_IsZeroD(x)) return 0;
	if (Test_IsOverD(y)) return x;
	if (Test_GetExpD(x) < Test_GetExpD(y) - 1) return x;

	REAL tr, tr2, xr, yr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(FromDouble)(&yr, y);
	REALNAME(Div)(&tr, &xr, &yr);	// x/y -> tr

	// rounding to even ... negative number
	if (REALNAME(IsNeg)(&tr))
	{
		REALNAME(Add)(&tr2, &tr, &REALNAME(ConstM05)); // + 0.5
		if (REALNAME(IsInt)(&tr2) && REALNAME(IsOddInt)(&tr2)) // odd integer
			REALNAME(IncFrom(&tr, &tr2));	// +1
		else
			REALNAME(Round)(&tr);		// rquote t = round(x/y)
	}

	// rounding to even ... negative number
	else
	{
		REALNAME(Add)(&tr2, &tr, &REALNAME(Const05));	// -0.5
		if (REALNAME(IsInt)(&tr2) && REALNAME(IsOddInt)(&tr2)) // odd integer
			REALNAME(DecFrom(&tr, &tr2));	// -1
		else
			REALNAME(Round)(&tr);		// rquote t = round(x/y)
	}

	if (quo != NULL) *quo = REALNAME(GetS32)(&tr);	// get result quotient rquote
	REALNAME(Mul)(&tr, &tr, &yr);	// multiply back rquote*y -> tr
	REALNAME(Sub)(&xr, &xr, &tr);	// remainder x - rquote*y -> xr
	return REALNAME(ToDouble)(&xr);
}

// get remainder of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
float Ref_remainderf(float x, float y)
{
#if RISCV
	if (Test_IsOverF(x) || Test_IsZeroF(y)) return Test_QNanF();
#else // RISCV
#if RP2350 && USE_FLOATLIBC
	if (Test_IsZeroF(y)) return Test_QNanF();
	if (Test_IsOverF(x)) return (Test_GetSignF(x) ^ Test_GetSignF(y)) ? Test_IndF() : Test_QNanF();
#endif
#endif
	return Ref_remquof(x, y, NULL);
}

double Ref_remainder(double x, double y)
{
#if RISCV
	if (Test_IsOverD(x) || Test_IsZeroD(y)) return Test_QNanD();
#else // RISCV
#if RP2350 && USE_DOUBLELIBC
	if (Test_IsZeroD(y)) return Test_QNanD();
	if (Test_IsOverD(x)) return (Test_GetSignD(x) ^ Test_GetSignD(y)) ? Test_IndD() : Test_QNanD();
#endif
#endif
	return Ref_remquo(x, y, NULL);
}

// Compare, x ? y
// Returns: 0 if x==y, -1 if x<y, +1 if x>y
s8 Ref_fcmp(float x, float y)
{
	REAL xr, yr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(FromFloat)(&yr, y);
	return REALNAME(Comp)(&xr, &yr);
}

s8 Ref_dcmp(double x, double y)
{
	REAL xr, yr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(FromDouble)(&yr, y);
	return REALNAME(Comp)(&xr, &yr);
}

// Compare if x==y
Bool Ref_fcmpeq(float x, float y) { return Ref_fcmp(x, y) == 0; }
Bool Ref_dcmpeq(double x, double y) { return Ref_dcmp(x, y) == 0; }

// Compare if x<y
Bool Ref_fcmplt(float x, float y) { return Ref_fcmp(x, y) < 0; }
Bool Ref_dcmplt(double x, double y) { return Ref_dcmp(x, y) < 0; }

// Compare if x<=y
Bool Ref_fcmple(float x, float y) { return Ref_fcmp(x, y) <= 0; }
Bool Ref_dcmple(double x, double y) { return Ref_dcmp(x, y) <= 0; }

// Compare if x>=y
Bool Ref_fcmpge(float x, float y) { return Ref_fcmp(x, y) >= 0; }
Bool Ref_dcmpge(double x, double y) { return Ref_dcmp(x, y) >= 0; }

// Compare if x>y
Bool Ref_fcmpgt(float x, float y) { return Ref_fcmp(x, y) > 0; }
Bool Ref_dcmpgt(double x, double y) { return Ref_dcmp(x, y) > 0; }

// Check if comparison is unordered (both inputs are NaN)
Bool Ref_fcmpun(float x, float y) { return Test_IsNanF(x) || Test_IsNanF(y); }
Bool Ref_dcmpun(double x, double y) { return Test_IsNanD(x) || Test_IsNanD(y); }

// Convert signed int to float
float Ref_int2float(s32 num)
{
	REAL tr;
	REALNAME(SetS32)(&tr, num);
	return REALNAME(ToFloat)(&tr);
}

double Ref_int2double(s32 num)
{
	REAL tr;
	REALNAME(SetS32)(&tr, num);
	return REALNAME(ToDouble)(&tr);
}

// Convert unsigned int to float
float Ref_uint2float(u32 num)
{
	REAL tr;
	REALNAME(SetU32)(&tr, num);
	return REALNAME(ToFloat)(&tr);
}

double Ref_uint2double(u32 num)
{
	REAL tr;
	REALNAME(SetU32)(&tr, num);
	return REALNAME(ToDouble)(&tr);
}

// Convert 64-bit signed int to float
float Ref_int642float(s64 num)
{
	REAL tr;
	REALNAME(SetS64)(&tr, num);
	return REALNAME(ToFloat)(&tr);
}

double Ref_int642double(s64 num)
{
	REAL tr;
	REALNAME(SetS64)(&tr, num);
	return REALNAME(ToDouble)(&tr);
}

// Convert 64-bit unsigned int to float
float Ref_uint642float(u64 num)
{
	REAL tr;
	REALNAME(SetU64)(&tr, num);
	return REALNAME(ToFloat)(&tr);
}

double Ref_uint642double(u64 num)
{
	REAL tr;
	REALNAME(SetU64)(&tr, num);
	return REALNAME(ToDouble)(&tr);
}

// Convert signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
float Ref_fix2float(s32 num, int e)
{
	REAL tr;
	REALNAME(SetS32)(&tr, num);
	REALNAME(Pow2)(&tr, -e);
	return REALNAME(ToFloat)(&tr);
}

double Ref_fix2double(s32 num, int e)
{
	REAL tr;
	REALNAME(SetS32)(&tr, num);
	REALNAME(Pow2)(&tr, -e);
	return REALNAME(ToDouble)(&tr);
}

// Convert unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
float Ref_ufix2float(u32 num, int e)
{
	REAL tr;
	REALNAME(SetU32)(&tr, num);
	REALNAME(Pow2)(&tr, -e);
	return REALNAME(ToFloat)(&tr);
}

double Ref_ufix2double(u32 num, int e)
{
	REAL tr;
	REALNAME(SetU32)(&tr, num);
	REALNAME(Pow2)(&tr, -e);
	return REALNAME(ToDouble)(&tr);
}

// Convert 64-bit signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
float Ref_fix642float(s64 num, int e)
{
	REAL tr;
	REALNAME(SetS64)(&tr, num);
	REALNAME(Pow2)(&tr, -e);
	return REALNAME(ToFloat)(&tr);
}

double Ref_fix642double(s64 num, int e)
{
	REAL tr;
	REALNAME(SetS64)(&tr, num);
	REALNAME(Pow2)(&tr, -e);
	return REALNAME(ToDouble)(&tr);
}

// Convert 64-bit unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
float Ref_ufix642float(u64 num, int e)
{
	REAL tr;
	REALNAME(SetU64)(&tr, num);
	REALNAME(Pow2)(&tr, -e);
	return REALNAME(ToFloat)(&tr);
}

double Ref_ufix642double(u64 num, int e)
{
	REAL tr;
	REALNAME(SetU64)(&tr, num);
	REALNAME(Pow2)(&tr, -e);
	return REALNAME(ToDouble)(&tr);
}

// Convert float to signed int, rounding to zero (C-style int conversion)
s32 Ref_float2int_z(float num)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	return REALNAME(GetS32)(&t);
}

s32 Ref_double2int_z(double num)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	return REALNAME(GetS32)(&t);
}

// Convert float to signed int, rounding down
s32 Ref_float2int(float num)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	REALNAME(Floor)(&t);
	return REALNAME(GetS32)(&t);
}

s32 Ref_double2int(double num)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	REALNAME(Floor)(&t);
	return REALNAME(GetS32)(&t);
}

// Convert float to unsigned int, rounding down
u32 Ref_float2uint(float num)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	return REALNAME(GetU32)(&t);
}

u32 Ref_double2uint(double num)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	return REALNAME(GetU32)(&t);
}

// Convert float to 64-bit signed int, rounding to zero (C-style int conversion)
s64 Ref_float2int64_z(float num)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	return REALNAME(GetS64)(&t);
}

s64 Ref_double2int64_z(double num)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	return REALNAME(GetS64)(&t);
}

// Convert float to 64-bit signed int, rounding down
s64 Ref_float2int64(float num)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	REALNAME(Floor)(&t);
	return REALNAME(GetS64)(&t);
}

s64 Ref_double2int64(double num)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	REALNAME(Floor)(&t);
	return REALNAME(GetS64)(&t);
}

// Convert float to 64-bit unsigned int, rounding down
u64 Ref_float2uint64(float num)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	return REALNAME(GetU64)(&t);
}

u64 Ref_double2uint64(double num)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	return REALNAME(GetU64)(&t);
}

// Convert float to signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
s32 Ref_float2fix(float num, int e)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	REALNAME(Pow2)(&t, e);
	REALNAME(Floor)(&t);
	return REALNAME(GetS32)(&t);
}
s32 Ref_double2fix(double num, int e)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	REALNAME(Pow2)(&t, e);
	REALNAME(Floor)(&t);
	return REALNAME(GetS32)(&t);
}

// Convert float to unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
// Note: MS VC++ does not support direct conversion to u32, it uses conversion to s32 instead
u32 Ref_float2ufix(float num, int e)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	REALNAME(Pow2)(&t, e);
	return REALNAME(GetU32)(&t);
}
u32 Ref_double2ufix(double num, int e)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	REALNAME(Pow2)(&t, e);
	return REALNAME(GetU32)(&t);
}

// Convert float to 64-bit signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
s64 Ref_float2fix64(float num, int e)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	REALNAME(Pow2)(&t, e);
	REALNAME(Floor)(&t);
	return REALNAME(GetS64)(&t);
}
s64 Ref_double2fix64(double num, int e)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	REALNAME(Pow2)(&t, e);
	REALNAME(Floor)(&t);
	return REALNAME(GetS64)(&t);
}

// Convert float to 64-bit unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
// Note: MS VC++ does not support direct conversion to u64, it uses conversion to s64 instead
u64 Ref_float2ufix64(float num, int e)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	REALNAME(Pow2)(&t, e);
	return REALNAME(GetU64)(&t);
}
u64 Ref_double2ufix64(double num, int e)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	REALNAME(Pow2)(&t, e);
	return REALNAME(GetU64)(&t);
}

// round number towards zero
float Ref_truncf(float num)
{
	if (Test_IsOverF(num)) return num;
	REAL t;
	REALNAME(FromFloat)(&t, num);
	REALNAME(Trunc)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_trunc(double num)
{
	if (Test_IsOverD(num)) return num;
	REAL t;
	REALNAME(FromDouble)(&t, num);
	REALNAME(Trunc)(&t);
	return REALNAME(ToDouble)(&t);
}

// round number to nearest integer
float Ref_roundf(float num)
{
	if (Test_IsOverF(num)) return num;
	REAL t;
	REALNAME(FromFloat)(&t, num);
	REALNAME(Round)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_round(double num)
{
	if (Test_IsOverD(num)) return num;
	REAL t;
	REALNAME(FromDouble)(&t, num);
	REALNAME(Round)(&t);
	return REALNAME(ToDouble)(&t);
}

// round number down to integer
float Ref_floorf(float num)
{
	if (Test_IsOverF(num)) return num;
	REAL t;
	REALNAME(FromFloat)(&t, num);
	REALNAME(Floor)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_floor(double num)
{
	if (Test_IsOverD(num)) return num;
	REAL t;
	REALNAME(FromDouble)(&t, num);
	REALNAME(Floor)(&t);
	return REALNAME(ToDouble)(&t);
}

// round number up to integer
float Ref_ceilf(float num)
{
	if (Test_IsOverF(num)) return num;
	REAL t;
	REALNAME(FromFloat)(&t, num);
	REALNAME(Ceil)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_ceil(double num)
{
	if (Test_IsOverD(num)) return num;
	REAL t;
	REALNAME(FromDouble)(&t, num);
	REALNAME(Ceil)(&t);
	return REALNAME(ToDouble)(&t);
}

// Square root
float Ref_sqrtf(float x)
{
#if USE_FLOATLIBC || (RISCV && (USE_RISCV_SQRTF == 0))
	if (x < 0) return Test_QNanF();
#else
#if RP2040
	if (x < 0) return (RomGetVersion() > 1) ? Test_IndF() : Test_PInfF();
#else
	if (x < 0) return Test_IndF();
#endif
#endif
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(Sqrt)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_sqrt(double x)
{
#if USE_DOUBLELIBC
	if (x < 0) return Test_QNanF();
#else
	if (x < 0) return Test_IndD();
#endif
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(Sqrt)(&t);
	return REALNAME(ToDouble)(&t);
}

// convert degrees to radians
float Ref_deg2radf(float x)
{
	REAL t;
	REALNAME(FromFloat)(&t, x);
	Unit = UNIT_DEG;
	REALNAME(ToRad)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_deg2rad(double x)
{
	REAL t;
	REALNAME(FromDouble)(&t, x);
	Unit = UNIT_DEG;
	REALNAME(ToRad)(&t);
	return REALNAME(ToDouble)(&t);
}

// convert radians to degrees
float Ref_rad2degf(float x)
{
	REAL t;
	REALNAME(FromFloat)(&t, x);
	Unit = UNIT_DEG;
	REALNAME(FromRad)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_rad2deg(double x)
{
	REAL t;
	REALNAME(FromDouble)(&t, x);
	Unit = UNIT_DEG;
	REALNAME(FromRad)(&t);
	return REALNAME(ToDouble)(&t);
}

// sine in radians
float Ref_sinf(float x)
{
	if (Test_IsOverF(x)) return sinf(x); // inf is undefined
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(Sin)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_sin(double x)
{
	if (Test_IsOverD(x)) return sin(x); // inf is undefined
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(Sin)(&t);
	return REALNAME(ToDouble)(&t);
}

// sine in degrees
float Ref_sinf_deg(float x)
{
	if (Test_IsOverF(x)) return sinf(x); // inf is undefined
	REAL t;
	REALNAME(FromFloat)(&t, x);
	Unit = UNIT_DEG;
	REALNAME(ToRad)(&t);
	REALNAME(Sin)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_sin_deg(double x)
{
	if (Test_IsOverD(x)) return sin(x); // inf is undefined
	REAL t;
	REALNAME(FromDouble)(&t, x);
	Unit = UNIT_DEG;
	REALNAME(ToRad)(&t);
	REALNAME(Sin)(&t);
	return REALNAME(ToDouble)(&t);
}

// cosine in radians
float Ref_cosf(float x)
{
	if (Test_IsOverF(x)) return cosf(x); // inf is undefined
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(Cos)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_cos(double x)
{
	if (Test_IsOverD(x)) return cos(x); // inf is undefined
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(Cos)(&t);
	return REALNAME(ToDouble)(&t);
}

// cosine in degrees
float Ref_cosf_deg(float x)
{
	if (Test_IsOverF(x)) return cosf(x); // inf is undefined
	REAL t;
	REALNAME(FromFloat)(&t, x);
	Unit = UNIT_DEG;
	REALNAME(ToRad)(&t);
	REALNAME(Cos)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_cos_deg(double x)
{
	if (Test_IsOverD(x)) return cos(x); // inf is undefined
	REAL t;
	REALNAME(FromDouble)(&t, x);
	Unit = UNIT_DEG;
	REALNAME(ToRad)(&t);
	REALNAME(Cos)(&t);
	return REALNAME(ToDouble)(&t);
}

// sine-cosine in radians
void Ref_sincosf(float x, float* psin, float* pcos)
{
	if (Test_IsOverF(x))
	{
		sincosf(x, psin, pcos); // inf is undefined
		return;
	}
	REAL tr, xr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(SinFrom)(&tr, &xr);
	*psin = REALNAME(ToFloat)(&tr);
	REALNAME(CosFrom)(&tr, &xr);
	*pcos = REALNAME(ToFloat)(&tr);
}

void Ref_sincos(double x, double* psin, double* pcos)
{
	if (Test_IsOverD(x))
	{
		sincos(x, psin, pcos); // inf is undefined
		return;
	}
	REAL tr, xr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(SinFrom)(&tr, &xr);
	*psin = REALNAME(ToDouble)(&tr);
	REALNAME(CosFrom)(&tr, &xr);
	*pcos = REALNAME(ToDouble)(&tr);
}

// sine-cosine in degrees
void Ref_sincosf_deg(float x, float* psin, float* pcos)
{
	if (Test_IsOverF(x))
	{
		sincosf(x, psin, pcos); // inf is undefined
		return;
	}
	REAL tr, xr;
	REALNAME(FromFloat)(&xr, x);
	Unit = UNIT_DEG;
	REALNAME(ToRad)(&xr);
	REALNAME(SinFrom)(&tr, &xr);
	*psin = REALNAME(ToFloat)(&tr);
	REALNAME(CosFrom)(&tr, &xr);
	*pcos = REALNAME(ToFloat)(&tr);
}

void Ref_sincos_deg(double x, double* psin, double* pcos)
{
	if (Test_IsOverD(x))
	{
		sincos(x, psin, pcos); // inf is undefined
		return;
	}
	REAL tr, xr;
	REALNAME(FromDouble)(&xr, x);
	Unit = UNIT_DEG;
	REALNAME(ToRad)(&xr);
	REALNAME(SinFrom)(&tr, &xr);
	*psin = REALNAME(ToDouble)(&tr);
	REALNAME(CosFrom)(&tr, &xr);
	*pcos = REALNAME(ToDouble)(&tr);
}

// tangent in radians
float Ref_tanf(float x)
{
	if (Test_IsOverF(x)) return tanf(x); // inf is undefined
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(Tan)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_tan(double x)
{
	if (Test_IsOverD(x)) return tan(x); // inf is undefined
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(Tan)(&t);
	return REALNAME(ToDouble)(&t);
}

// tangent in degrees
float Ref_tanf_deg(float x)
{
	if (Test_IsOverF(x)) return tanf(x); // inf is undefined
	REAL t;
	REALNAME(FromFloat)(&t, x);
	Unit = UNIT_DEG;
	REALNAME(ToRad)(&t);
	REALNAME(Tan)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_tan_deg(double x)
{
	if (Test_IsOverD(x)) return tan(x); // inf is undefined
	REAL t;
	REALNAME(FromDouble)(&t, x);
	Unit = UNIT_DEG;
	REALNAME(ToRad)(&t);
	REALNAME(Tan)(&t);
	return REALNAME(ToDouble)(&t);
}

// cotangent in radians
float Ref_cotanf(float x)
{
	if (Test_IsOverF(x) || Test_IsZeroF(x)) return 1/tanf(x); // inf is undefined
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(CoTan)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_cotan(double x)
{
	if (Test_IsOverD(x) || Test_IsZeroD(x)) return 1/tan(x); // inf is undefined
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(CoTan)(&t);
	return REALNAME(ToDouble)(&t);
}

// cotangent in degrees
float Ref_cotanf_deg(float x)
{
	if (Test_IsOverF(x) || Test_IsZeroF(x)) return 1/tanf(x); // inf is undefined
	REAL t;
	REALNAME(FromFloat)(&t, x);
	Unit = UNIT_DEG;
	REALNAME(ToRad)(&t);
	REALNAME(CoTan)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_cotan_deg(double x)
{
	if (Test_IsOverD(x) || Test_IsZeroD(x)) return 1/tan(x); // inf is undefined
	REAL t;
	REALNAME(FromDouble)(&t, x);
	Unit = UNIT_DEG;
	REALNAME(ToRad)(&t);
	REALNAME(CoTan)(&t);
	return REALNAME(ToDouble)(&t);
}

// arc sine in radians
float Ref_asinf(float x)
{
	if (Test_IsOverF(x) || (x > 1.0f) || (x < -1.0f)) return Test_PInfF();
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(ASin)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_asin(double x)
{
	if (Test_IsOverD(x) || (x > 1.0f) || (x < -1.0f)) return Test_PInfD();
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(ASin)(&t);
	return REALNAME(ToDouble)(&t);
}

// arc sine in degrees
float Ref_asinf_deg(float x)
{
	if (Test_IsOverF(x) || (x > 1.0f) || (x < -1.0f)) return Test_PInfF();
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(ASin)(&t);
	Unit = UNIT_DEG;
	REALNAME(FromRad)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_asin_deg(double x)
{
	if (Test_IsOverD(x) || (x > 1.0f) || (x < -1.0f)) return Test_PInfD();
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(ASin)(&t);
	Unit = UNIT_DEG;
	REALNAME(FromRad)(&t);
	return REALNAME(ToDouble)(&t);
}

// arc cosine in radians
float Ref_acosf(float x)
{
	if (Test_IsOverF(x) || (x > 1.0f) || (x < -1.0f)) return Test_PInfF();
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(ACos)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_acos(double x)
{
	if (Test_IsOverD(x) || (x > 1.0f) || (x < -1.0f)) return Test_PInfD();
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(ACos)(&t);
	return REALNAME(ToDouble)(&t);
}

// arc cosine in degrees
float Ref_acosf_deg(float x)
{
	if (Test_IsOverF(x) || (x > 1.0f) || (x < -1.0f)) return Test_PInfF();
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(ACos)(&t);
	Unit = UNIT_DEG;
	REALNAME(FromRad)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_acos_deg(double x)
{
	if (Test_IsOverD(x) || (x > 1.0f) || (x < -1.0f)) return Test_PInfD();
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(ACos)(&t);
	Unit = UNIT_DEG;
	REALNAME(FromRad)(&t);
	return REALNAME(ToDouble)(&t);
}

// arc tangent in radians
float Ref_atanf(float x)
{
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(ATan)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_atan(double x)
{
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(ATan)(&t);
	return REALNAME(ToDouble)(&t);
}

// arc tangent in degrees
float Ref_atanf_deg(float x)
{
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(ATan)(&t);
	Unit = UNIT_DEG;
	REALNAME(FromRad)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_atan_deg(double x)
{
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(ATan)(&t);
	Unit = UNIT_DEG;
	REALNAME(FromRad)(&t);
	return REALNAME(ToDouble)(&t);
}

// arc cotangent in radians
float Ref_acotanf(float x)
{
	if (!Test_IsNegF(x) && (Test_GetExpF(x) > FLOAT_EXP1+FLOAT_MANTBITS/2)) return 1/x;
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(ACoTan)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_acotan(double x)
{
	if (!Test_IsNegD(x) && (Test_GetExpD(x) > DOUBLE_EXP1+DOUBLE_MANTBITS/2)) return 1/x;
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(ACoTan)(&t);
	return REALNAME(ToDouble)(&t);
}

// arc cotangent in degrees
float Ref_acotanf_deg(float x)
{
	if (!Test_IsNegF(x) && (Test_GetExpF(x) > FLOAT_EXP1+FLOAT_MANTBITS/2)) return rad2degf(1/x);
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(ACoTan)(&t);
	Unit = UNIT_DEG;
	REALNAME(FromRad)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_acotan_deg(double x)
{
	if (!Test_IsNegD(x) && (Test_GetExpD(x) > DOUBLE_EXP1+DOUBLE_MANTBITS/2)) return rad2deg(1/x);
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(ACoTan)(&t);
	Unit = UNIT_DEG;
	REALNAME(FromRad)(&t);
	return REALNAME(ToDouble)(&t);
}

// arc tangent of y/x in radians
float Ref_atan2f(float y, float x)
{
#if !RISCV
	if (Test_IsOverF(y) && Test_IsOverF(x)) return Test_PInfF();
#endif
	REAL tr, xr, yr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(FromFloat)(&yr, y);
	REALNAME(ATan2)(&tr, &xr, &yr);
	return REALNAME(ToFloat)(&tr);
}

double Ref_atan2(double y, double x)
{
#if !RISCV
	if (Test_IsOverD(y) && Test_IsOverD(x)) return Test_PInfD();
#endif
	REAL tr, xr, yr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(FromDouble)(&yr, y);
	REALNAME(ATan2)(&tr, &xr, &yr);
	return REALNAME(ToDouble)(&tr);
}

// arc tangent of y/x in radians
float Ref_atan2f_deg(float y, float x)
{
#if !RISCV
	if (Test_IsOverF(y) && Test_IsOverF(x)) return Test_PInfF();
#endif
	REAL tr, xr, yr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(FromFloat)(&yr, y);
	REALNAME(ATan2)(&tr, &xr, &yr);
	Unit = UNIT_DEG;
	REALNAME(FromRad)(&tr);
	return REALNAME(ToFloat)(&tr);
}

double Ref_atan2_deg(double y, double x)
{
#if !RISCV
	if (Test_IsOverD(y) && Test_IsOverD(x)) return Test_PInfD();
#endif
	REAL tr, xr, yr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(FromDouble)(&yr, y);
	REALNAME(ATan2)(&tr, &xr, &yr);
	Unit = UNIT_DEG;
	REALNAME(FromRad)(&tr);
	return REALNAME(ToDouble)(&tr);
}

// hyperbolic sine
float Ref_sinhf(float x)
{
	if (Test_IsOverF(x) || (Test_GetExpF(x) < FLOAT_EXP1-17)) return x;
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(SinH)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_sinh(double x)
{
	if (Test_IsOverD(x) || (Test_GetExpD(x) < DOUBLE_EXP1-33)) return x;
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(SinH)(&t);
	return REALNAME(ToDouble)(&t);
}

// hyperbolic cosine
float Ref_coshf(float x)
{
	if (Test_IsOverF(x)) return Test_PInfF();
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(CosH)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_cosh(double x)
{
	if (Test_IsOverD(x)) return Test_PInfD();
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(CosH)(&t);
	return REALNAME(ToDouble)(&t);
}

// hyperbolic tangent
float Ref_tanhf(float x)
{
	if (Test_GetExpF(x) >= FLOAT_EXP1+4) // check if abs(x) >= 16
		return (x >= 0) ? 1.0f : -1.0f;
	if (Test_GetExpF(x) < FLOAT_EXP1-17) return x;
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(TanH)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_tanh(double x)
{
	if (Test_GetExpD(x) >= DOUBLE_EXP1+5) // check if abs(x) >= 32
		return (x >= 0) ? 1.0 : -1.0;
	if (Test_GetExpD(x) < DOUBLE_EXP1-33) return x;
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(TanH)(&t);
	return REALNAME(ToDouble)(&t);
}

// inverse hyperbolic sine
float Ref_asinhf(float x)
{
	if (Test_IsOverF(x) || (Test_GetExpF(x) < FLOAT_EXP1-17)) return x;

	REAL t;
	REALNAME(FromFloat)(&t, x);

	if (Test_GetExpF(x) > FLOAT_EXP1+16)
	{
		if (x >= 0)
		{
			REALNAME(Ln)(&t);
			REALNAME(Add)(&t, &t, &REALNAME(ConstLn2));
		}
		else
		{
			REALNAME(Neg)(&t);
			REALNAME(Ln)(&t);
			REALNAME(Add)(&t, &t, &REALNAME(ConstLn2));
			REALNAME(Neg)(&t);
		}
	}
	else
		REALNAME(ArSinH)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_asinh(double x)
{
	if (Test_IsOverD(x) || (Test_GetExpD(x) < DOUBLE_EXP1-33)) return x;

	REAL t;
	REALNAME(FromDouble)(&t, x);

	if (Test_GetExpD(x) > DOUBLE_EXP1+16)
	{
		if (x >= 0)
		{
			REALNAME(Ln)(&t);
			REALNAME(Add)(&t, &t, &REALNAME(ConstLn2));
		}
		else
		{
			REALNAME(Neg)(&t);
			REALNAME(Ln)(&t);
			REALNAME(Add)(&t, &t, &REALNAME(ConstLn2));
			REALNAME(Neg)(&t);
		}
	}
	else
		REALNAME(ArSinH)(&t);
	return REALNAME(ToDouble)(&t);
}

// inverse hyperbolic cosine
float Ref_acoshf(float x)
{
#if !USE_FLOATLIBC
	x = fabs(x);
#endif
	if (x < 1.0f) return Test_PInfF();
	REAL t;
	REALNAME(FromFloat)(&t, x);
	if (Test_GetExpF(x) >= FLOAT_EXP1+16)
	{
		REALNAME(Ln)(&t);
		REALNAME(Add)(&t, &t, &REALNAME(ConstLn2));
	}
	else
		REALNAME(ArCosH)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_acosh(double x)
{
#if !USE_FLOATLIBC
	x = abs(x);
#endif
	if (x < 1.0) return Test_PInfD();
	REAL t;
	REALNAME(FromDouble)(&t, x);
	if (Test_GetExpD(x) >= DOUBLE_EXP1+32)
	{
		REALNAME(Ln)(&t);
		REALNAME(Add)(&t, &t, &REALNAME(ConstLn2));
	}
	else
		REALNAME(ArCosH)(&t);
	return REALNAME(ToDouble)(&t);
}

// inverse hyperbolic tangent
float Ref_atanhf(float x)
{
#if USE_FLOATLIBC || RISCV
	if ((x > 1.0f) || (x < -1.0f)) return Test_QNanF();
#else
	if ((x > 1.0f) || (x < -1.0f)) return Test_IndF();
#endif
	if (x == 1.0f) return Test_PInfF();
	if (x == -1.0f) return Test_MInfF();

	// check small number
	if (Test_GetExpF(x) < FLOAT_EXP1-30) return x;

	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(ArTanH)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_atanh(double x)
{
#if USE_FLOATLIBC || RISCV
	if ((x > 1.0) || (x < -1.0)) return Test_QNanD();
#else
	if ((x > 1.0) || (x < -1.0)) return Test_IndD();
#endif
	if (x == 1.0) return Test_PInfD();
	if (x == -1.0) return Test_MInfD();

	// check small number
	if (Test_GetExpD(x) < DOUBLE_EXP1-40) return x;

	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(ArTanH)(&t);
	return REALNAME(ToDouble)(&t);
}

// Natural exponent
float Ref_expf(float x)
{
	if (Test_IsOverF(x)) return Test_IsNegF(x) ? 0 : Test_PInfF();
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(Exp)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_exp(double x)
{
	if (Test_IsOverD(x)) return Test_IsNegD(x) ? 0 : Test_PInfD();
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(Exp)(&t);
	return REALNAME(ToDouble)(&t);
}

// Natural logarithm
float Ref_logf(float x)
{
	if (Test_IsPOverF(x)) return x;
	if (Test_IsZeroF(x)) return Test_MInfF();
#if USE_FLOATLIBC || RISCV
	if (Test_IsNegF(x)) return Test_QNanF();
#else
	if (Test_IsNegF(x)) return Test_IndF();
#endif
	if (x == 1) return 0;
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(Ln)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_log(double x)
{
	if (Test_IsPOverD(x)) return x;
	if (Test_IsZeroD(x)) return Test_MInfD();
#if USE_FLOATLIBC || RISCV
	if (Test_IsNegD(x)) return Test_QNanD();
#else
	if (Test_IsNegD(x)) return Test_IndD();
#endif
	if (x == 1) return 0;
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(Ln)(&t);
	return REALNAME(ToDouble)(&t);
}

// exponent with base 2
float Ref_exp2f(float x)
{
	if (Test_IsOverF(x)) return Test_IsNegF(x) ? 0 : Test_PInfF();
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(Exp2)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_exp2(double x)
{
	if (Test_IsOverD(x)) return Test_IsNegD(x) ? 0 : Test_PInfD();
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(Exp2)(&t);
	return REALNAME(ToDouble)(&t);
}

// logarithm with base 2
float Ref_log2f(float x)
{
	if (Test_IsPOverF(x)) return x;
	if (Test_IsZeroF(x)) return Test_MInfF();
#if USE_FLOATLIBC || RISCV
	if (Test_IsNegF(x)) return Test_QNanF();
#else
	if (Test_IsNegF(x)) return Test_IndF();
#endif
	if (x == 1) return 0;
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(Log2)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_log2(double x)
{
	if (Test_IsPOverD(x)) return x;
	if (Test_IsZeroD(x)) return Test_MInfD();
#if USE_FLOATLIBC || RISCV
	if (Test_IsNegD(x)) return Test_QNanD();
#else
	if (Test_IsNegD(x)) return Test_IndD();
#endif
	if (x == 1) return 0;
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(Log2)(&t);
	return REALNAME(ToDouble)(&t);
}

// exponent with base 10
float Ref_exp10f(float x)
{
	if (Test_IsOverF(x)) return Test_IsNegF(x) ? 0 : Test_PInfF();
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(Exp10)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_exp10(double x)
{
	if (Test_IsOverD(x)) return Test_IsNegD(x) ? 0 : Test_PInfD();
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(Exp10)(&t);
	return REALNAME(ToDouble)(&t);
}

// logarithm with base 10
float Ref_log10f(float x)
{
	if (Test_IsPOverF(x)) return x;
	if (Test_IsZeroF(x)) return Test_MInfF();
#if USE_FLOATLIBC || RISCV
	if (Test_IsNegF(x)) return Test_QNanF();
#else
	if (Test_IsNegF(x)) return Test_IndF();
#endif
	if (x == 1) return 0;
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(Log10)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_log10(double x)
{
	if (Test_IsPOverD(x)) return x;
	if (Test_IsZeroD(x)) return Test_MInfD();
#if USE_FLOATLIBC || RISCV
	if (Test_IsNegD(x)) return Test_QNanD();
#else
	if (Test_IsNegD(x)) return Test_IndD();
#endif
	if (x == 1) return 0;
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(Log10)(&t);
	return REALNAME(ToDouble)(&t);
}

// expf(x) - 1
float Ref_expm1f(float x)
{
	if (Test_IsZeroF(x)) return 0.0f; // zero
	if (Test_IsOverF(x)) return Test_IsNegF(x) ? -1 : Test_PInfF();
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(Exp)(&t);
	REALNAME(Dec)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_expm1(double x)
{
	if (Test_IsZeroD(x)) return 0.0; // zero
	if (Test_IsOverD(x)) return Test_IsNegD(x) ? -1 : Test_PInfD();
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(Exp)(&t);
	REALNAME(Dec)(&t);
	return REALNAME(ToDouble)(&t);
}

// logf(x + 1)
float Ref_log1pf(float x)
{
	if (Test_IsPOverF(x)) return x;
	if (Test_GetExpF(x) < FLOAT_EXP1 - 12) return x;
	if (x == -1) return Test_MInfF();
#if USE_FLOATLIBC || RISCV
	if (x < -1) return Test_QNanF();
#else
	if (x < -1) return Test_IndF();
#endif
	REAL t;
	REALNAME(FromFloat)(&t, x);
	REALNAME(Inc)(&t);
	REALNAME(Ln)(&t);
	return REALNAME(ToFloat)(&t);
}

double Ref_log1p(double x)
{
	if (Test_IsPOverD(x)) return x;
	if (Test_GetExpD(x) < DOUBLE_EXP1 - 26) return x;
	if (x == -1) return Test_MInfD();
#if USE_FLOATLIBC || RISCV
	if (x < -1) return Test_QNanD();
#else
	if (x < -1) return Test_IndD();
#endif
	REAL t;
	REALNAME(FromDouble)(&t, x);
	REALNAME(Inc)(&t);
	REALNAME(Ln)(&t);
	return REALNAME(ToDouble)(&t);
}

// x*y + z
float Ref_fmaf(float x, float y, float z)
{
	// simulate invalid states
	float tmp = x*y + z;
	if (Test_IsIndF(tmp) || Test_IsOverF(tmp)) return tmp;

	REAL xr, yr, zr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(FromFloat)(&yr, y);
	REALNAME(FromFloat)(&zr, z);
	REALNAME(Mul)(&xr, &xr, &yr);
	REALNAME(Add)(&xr, &xr, &zr);
	return REALNAME(ToFloat)(&xr);
}

double Ref_fma(double x, double y, double z)
{
	// simulate invalid states
	double tmp = x*y + z;
	if (Test_IsIndD(tmp) || Test_IsOverD(tmp)) return tmp;

	REAL xr, yr, zr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(FromDouble)(&yr, y);
	REALNAME(FromDouble)(&zr, z);
	REALNAME(Mul)(&xr, &xr, &yr);
	REALNAME(Add)(&xr, &xr, &zr);
	return REALNAME(ToDouble)(&xr);
}

// power by integer, x^y
float Ref_powintf(float x, int y)
{
	// x = 1 or y = 0, result is 1
	if ((x == 1.0f) || (y == 0)) return 1;

	// x = 0
	// do not use "x == 0.0f" - in that case compiler can later use positive zero instead of real X
	if (Test_IsZeroF(x))
	{
		// exponent is positive
		if (y > 0)
		{
			if ((y & 1) != 0) return x; // exponent is odd, result is x = +0.0 or -0.0
			return 0; // exponent is even, result is +0.0
		}

		// exponent is negative and odd - result is +-INF
		if ((y & 1) != 0) return Test_IsNegF(x) ? Test_MInfF() : Test_PInfF();

		// result is +INF
		return Test_PInfF();
	}

	// x is positive infinity NaN
	if (Test_IsPOverF(x))
	{
		// exponent is negative, result will be 0
		if (y < 0) return 0;
		return Test_PInfF(); // exponent is positive, result will be infinity NaN
	}

	// x is negative infinity NaN
	if (Test_IsMOverF(x))
	{
		// exponent is positive
		if (y > 0)
		{
			if ((y & 1) != 0) return Test_MInfF(); // exponent is odd, result is negative infinity
			return Test_PInfF(); // exponent is even, result is positive infinity
		}

		// exponent is negative and odd - result is -0.0
		if ((y & 1) != 0) return Test_MZeroF();

		// result is +0.0
		return 0;
	}

	REAL tr, xr, yr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(SetS32)(&yr, y);
	REALNAME(Pow)(&tr, &xr, &yr);
	return REALNAME(ToFloat)(&tr);
}

double Ref_powint(double x, int y)
{
	// x = 1 or y = 0, result is 1
	if ((x == 1.0) || (y == 0)) return 1;

	// x = 0
	// do not use "x == 0.0f" - in that case compiler can later use positive zero instead of real X
	if (Test_IsZeroD(x))
	{
		// exponent is positive
		if (y > 0)
		{
			if ((y & 1) != 0) return x; // exponent is odd, result is x = +0.0 or -0.0
			return 0; // exponent is even, result is +0.0
		}

		// exponent is negative and odd - result is +-INF
		if ((y & 1) != 0) return Test_IsNegD(x) ? Test_MInfD() : Test_PInfD();

		// result is +INF
		return Test_PInfD();
	}

	// x is positive infinity NaN
	if (Test_IsPOverD(x))
	{
		// exponent is negative, result will be 0
		if (y < 0) return 0;
		return Test_PInfD(); // exponent is positive, result will be infinity NaN
	}

	// x is negative infinity NaN
	if (Test_IsMOverD(x))
	{
		// exponent is positive
		if (y > 0)
		{
			if ((y & 1) != 0) return Test_MInfD(); // exponent is odd, result is negative infinity
			return Test_PInfD(); // exponent is even, result is positive infinity
		}

		// exponent is negative and odd - result is -0.0
		if ((y & 1) != 0) return Test_MZeroD();

		// result is +0.0
		return 0;
	}

	REAL tr, xr, yr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(SetS32)(&yr, y);
	REALNAME(Pow)(&tr, &xr, &yr);
	return REALNAME(ToDouble)(&tr);
}

// power x^y
float Ref_powf(float x, float y)
{
	// x is 1 or y is 0, result is 1
	if ((x == 1.0f) || Test_IsZeroF(y)) return 1;

	// x is -1 and y is infinity, result is +1
	if ((x == -1.0f) && Test_IsOverF(y)) return 1;

	// x is zero
	if (Test_IsZeroF(x))
	{
		// y is positive
		if (!Test_IsNegF(y))
		{
			// y is odd and integer, return +0.0 or -0.0
			if (Ref_isoddintf(y)) return x;
			return 0; // else result is 0
		}

		// y is negative, odd and integer - return +-infinity
		if (Ref_isoddintf(y)) return Test_IsNegF(x) ? Test_MInfF() : Test_PInfF();

		// y is negative, return positive infinity
		return Test_PInfF();
	}

	// x is positive infinity
	if (Test_IsPOverF(x))
	{
		// y is negative, result will be 0
		if (Test_IsNegF(y)) return 0;
		return Test_PInfF(); // y is positive, result is positive infinity
	}

	// x is negative infinity
	if (Test_IsMOverF(x))
	{
		// y is positive
		if (!Test_IsNegF(y))
		{
			// y is positive and odd integer, result is negative infinity
			if (Ref_isoddintf(y)) return Test_MInfF();

			// y is negative or not odd integer, result is positive infinity
			return Test_PInfF();
		}

		// y is negative and odd integer, result will be -0.0
		if (Ref_isoddintf(y)) return Test_MZeroF();

		// y is negative, result will be +0.0
		return 0;
	}

	// y is positive infinity
	if (Test_IsPOverF(y))
	{
		// x is < 1, result is +0.0
		if (Test_GetExpF(x) < 0x7f) return 0;

		// x is >= 1, result is positive infinity
		return Test_PInfF();
	}

	// y is negative infinity
	if (Test_IsMOverF(y))
	{
		// x is < 1, result is positive infinity
		if (Test_GetExpF(x) < 0x7f) return Test_PInfF();

		// x is >= 1, result is +0.0
		return 0;
	}

	// x is negative - invalid if y is not integer
	if (Test_IsNegF(x) && !Ref_isintf(y)) return Test_PInfF();

	REAL tr, xr, yr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(FromFloat)(&yr, y);
	REALNAME(Pow)(&tr, &xr, &yr);
	return REALNAME(ToFloat)(&tr);
}

double Ref_pow(double x, double y)
{
	// x is 1 or y is 0, result is 1
	if ((x == 1.0) || Test_IsZeroD(y)) return 1;

	// x is -1 and y is infinity, result is +1
	if ((x == -1.0) && Test_IsOverD(y)) return 1;

	// x is zero
	if (Test_IsZeroD(x))
	{
		// y is positive
		if (!Test_IsNegD(y))
		{
			// y is odd and integer, return +0.0 or -0.0
			if (Ref_isoddintd(y)) return x;
			return 0; // else result is 0
		}

		// y is negative, odd and integer - return +-infinity
		if (Ref_isoddintd(y)) return Test_IsNegD(x) ? Test_MInfD() : Test_PInfD();

		// y is negative, return positive infinity
		return Test_PInfD();
	}

	// x is positive infinity
	if (Test_IsPOverD(x))
	{
		// y is negative, result will be 0
		if (Test_IsNegD(y)) return 0;
		return Test_PInfD(); // y is positive, result is positive infinity
	}

	// x is negative infinity
	if (Test_IsMOverD(x))
	{
		// y is positive
		if (!Test_IsNegD(y))
		{
			// y is positive and odd integer, result is negative infinity
			if (Ref_isoddintd(y)) return Test_MInfD();

			// y is negative or not odd integer, result is positive infinity
			return Test_PInfD();
		}

		// y is negative and odd integer, result will be -0.0
		if (Ref_isoddintd(y)) return Test_MZeroD();

		// y is negative, result will be +0.0
		return 0;
	}

	// y is positive infinity
	if (Test_IsPOverD(y))
	{
		// x is < 1, result is +0.0
		if (Test_GetExpD(x) < 0x3ff) return 0;

		// x is >= 1, result is positive infinity
		return Test_PInfD();
	}

	// y is negative infinity
	if (Test_IsMOverD(y))
	{
		// x is < 1, result is positive infinity
		if (Test_GetExpD(x) < 0x3ff) return Test_PInfD();

		// x is >= 1, result is +0.0
		return 0;
	}

	// x is negative
	if (Test_IsNegD(x) && !Ref_isintd(y)) return Test_PInfD();

	REAL tr, xr, yr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(FromDouble)(&yr, y);
	REALNAME(Pow)(&tr, &xr, &yr);
	return REALNAME(ToDouble)(&tr);
}

// square root of sum of squares (hypotenuse), sqrt(x*x + y*y)
float Ref_hypotf(float x, float y)
{
	REAL xr, yr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(FromFloat)(&yr, y);
	REALNAME(Radius)(&xr, &xr, &yr);
	return REALNAME(ToFloat)(&xr);
}

double Ref_hypot(double x, double y)
{
	REAL xr, yr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(FromDouble)(&yr, y);
	REALNAME(Radius)(&xr, &xr, &yr);
	return REALNAME(ToDouble)(&xr);
}

// cube root, sqrt3(x), x^(1/3)
float Ref_cbrtf(float x)
{
	Bool sign = (x < 0);
	if (sign) x = -x;
	if (x == 0) return 0; // for case of -0.0
	REAL xr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(Root)(&xr, &xr, &REALNAME(Const3));
	x = REALNAME(ToFloat)(&xr);
	return sign ? -x : x;
}

double Ref_cbrt(double x)
{
	Bool sign = (x < 0);
	if (sign) x = -x;
	if (x == 0) return 0; // for case of -0.0
	REAL xr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(Root)(&xr, &xr, &REALNAME(Const3));
	x = REALNAME(ToDouble)(&xr);
	return sign ? -x : x;
}

// absolute value
float Ref_absf(float x)
	{ return Test_u32float(Test_floatu32(x) & 0x7fffffff); }
double Ref_absd(double x)
	{ return Test_u64double(Test_doubleu64(x) & 0x7fffffffffffffffULL); }

// Convert float to double
double Ref_float2double(float num)
{
	REAL t;
	REALNAME(FromFloat)(&t, num);
	return REALNAME(ToDouble)(&t);
}

// Convert double to float
float Ref_double2float(double num)
{
	REAL t;
	REALNAME(FromDouble)(&t, num);
	return REALNAME(ToFloat)(&t);
}

// round to given number of significant digits (digits<=0 to use default number of digits)
// @TODO: probably will be deleted (accuracy cannot be guaranteed)
/*
float Ref_rounddigf(float x, int digits)
{
	// invalid values
	if (Test_IsZeroF(x) || Test_IsOverF(x)) return x;

	// absolute value
	Bool neg = Test_IsNegF(x);
	if (neg) x = -x;

	// get scale of the number: scale = exp10(digits - 1 - (int)floor(log10(x)))
	if (digits < 1) digits = 6;
	REAL xr, sr;
	REALNAME(FromFloat)(&xr, x);
	REALNAME(Log10From)(&sr, &xr);
	REALNAME(Floor)(&sr);
	s32 n = REALNAME(GetS32)(&sr);
	REALNAME(SetS32)(&sr, digits - 1 - n);
	REALNAME(Exp10)(&sr);

	// round number: x = roundf(x * scale) / scale;
	REALNAME(Mul)(&xr, &xr, &sr);
	REALNAME(Round)(&xr);
	REALNAME(Div)(&xr, &xr, &sr);
	x = REALNAME(ToFloat)(&xr);

	// negate
	if (neg) x = -x;
	return x;
}

double Ref_rounddig(double x, int digits)
{
	// invalid values
	if (Test_IsZeroD(x) || Test_IsOverD(x)) return x;

	// absolute value
	Bool neg = Test_IsNegD(x);
	if (neg) x = -x;

	// get scale of the number: scale = exp10(digits - 1 - (int)floor(log10(x)))
	if (digits < 1) digits = 13;
	REAL xr, sr;
	REALNAME(FromDouble)(&xr, x);
	REALNAME(Log10From)(&sr, &xr);
	REALNAME(Floor)(&sr);
	s32 n = REALNAME(GetS32)(&sr);
	REALNAME(SetS32)(&sr, digits - 1 - n);
	REALNAME(Exp10)(&sr);

	// round number: x = roundf(x * scale) / scale;
	REALNAME(Mul)(&xr, &xr, &sr);
	REALNAME(Round)(&xr);
	REALNAME(Div)(&xr, &xr, &sr);
	x = REALNAME(ToDouble)(&xr);

	// negate
	if (neg) x = -x;
	return x;
}
*/
