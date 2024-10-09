
// ****************************************************************************
//
//                         Random number generator
//
// ****************************************************************************

#include "../include.h"

// Seed of random number generator
u64 MyRandSeed;

// order bit table
//                            0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
u8 MyOrdBitsTab[16] = { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };

// get bit order of value (logarithm, returns position of highest bit + 1: 1..32, 0=no bit)
u8 MyOrder(u32 val)
{
	int i = 0;
	if (val >= 0x10000)
	{
		i = 16;
		val >>= 16;
	}

	if (val >= 0x100)
	{
		i += 8;
		val >>= 8;
	}

	if (val >= 0x10)
	{
		i += 4;
		val >>= 4;
	}
	return MyOrdBitsTab[val] + i;
}

// get mask of value (0x123 returns 0x1FF)
u32 MyMask(u32 val)
{
	return (u32)-1 >> (32 - MyOrder(val));
}

// get seed of random number generator
u64 MyRandGet()
{
	u64 s = MyRandSeed;
	return s;
}

// set seed of random number generator
void MyRandSet(u64 seed)
{
	MyRandSeed = seed;
}

// shift random number generator and return 32-bit random number
u32 MyRandShift()
{
	// get pointer to the seed
	u64* s = &MyRandSeed;

	// shift seed
	u64 k = *s;
	k = k * 214013 + 2531011;
	*s = k;

	return (u32)(k >> 32);
}

// generate 8-bit unsigned integer random number
u8 MyRandU8()
{
	return (u8)(MyRandShift() >> 24);
}

// generate 16-bit unsigned integer random number
u16 MyRandU16()
{
	return (u16)(MyRandShift() >> 16);
}

// generate 64-bit unsigned integer random number
u64 MyRandU64()
{
	u32 n = MyRandShift();
	return MyRandShift() | ((u64)n << 32);
}

u64 MyRandU64Fast()
{
	MyRandShift();
	return MyRandSeed;
}

// generate float random number in range 0 (including) to 1 (excluding)
// (maximum number is 0.99999994, minimal step 0.00000006)
float MyRandFloat()
{
	return (float)(MyRandShift() >> (32-24))*(1.0f/0x1000000);
}

// generate double random number in range 0 (including) to 1 (excluding)
// (maximum number is 0.999999999999996, step 0.0000000000000035, 14 valid digits)
double MyRandDouble()
{
	MyRandShift();
	return (double)(MyRandGet() >> (64-48))*(1.0/0x1000000000000ULL);
}

// generate 8-bit unsigned integer random number in range 0 to MAX (including, signed can be negative)
u8 MyRandU8Max(u8 max)
{
	u8 res;
	u8 msk;

	// zero maximal value
	if (max == 0) return 0;

	// prepare mask
	msk = (u8)MyMask(max);

	// generate random number
	do {
		res = MyRandU8() & msk;
	} while (res > max);

	return res;
}

// generate 16-bit unsigned integer random number in range 0 to MAX (including)
u16 MyRandU16Max(u16 max)
{
	u16 res;
	u16 msk;

	// zero maximal value
	if (max == 0) return 0;

	// prepare mask
	msk = (u16)MyMask(max);

	// generate random number
	do {
		res = MyRandU16() & msk;
	} while (res > max);

	return res;
}

// generate 32-bit unsigned integer random number in range 0 to MAX (including)
u32 MyRandU32Max(u32 max)
{
	u32 res;
	u32 msk;

	// zero maximal value
	if (max == 0) return 0;

	// prepare mask
	msk = (u32)MyMask(max);

	// generate random number
	do {
		res = MyRandU32() & msk;
	} while (res > max);

	return res;
}

// generate 64-bit unsigned integer random number in range 0 to MAX (including)
u64 MyRandU64Max(u64 max)
{
	u64 res;
	u64 msk;
	u8 pos;

	// DWORD value
	if (max < 0x100000000ULL) return MyRandU32Max((u32)max);

	// zero maximal value
	if (max == 0) return 0;

	// prepare mask
	pos = MyOrder((u32)(max >> 32));
	msk = (u64)-1 >> (32 - pos);

	// generate random number
	do {
		res = MyRandU64() & msk;
	} while (res > max);

	return res;
}

// generate 8-bit signed integer random number in range 0 to MAX (including, 'max' can be negative)
s8 MyRandS8Max(s8 max)
{
	if (max >= 0)
		return (s8)MyRandU8Max((u8)max);
	else
		return -(s8)MyRandU8Max((u8)-max);
}

// generate 16-bit signed integer random number in range 0 to MAX (including, 'max' can be negative)
s16 MyRandS16Max(s16 max)
{
	if (max >= 0)
		return (s16)MyRandU16Max((u16)max);
	else
		return -(s16)MyRandU16Max((u16)-max);
}

// generate 32-bit signed integer random number in range 0 to MAX (including, 'max' can be negative)
s32 MyRandS32Max(s32 max)
{
	if (max >= 0)
		return (s32)MyRandU32Max((u32)max);
	else
		return -(s32)MyRandU32Max((u32)-max);
}

// generate 64-bit signed integer random number in range 0 to MAX (including, 'max' can be negative)
s64 MyRandS64Max(s64 max)
{
	if (max >= 0)
		return (s64)MyRandU64Max((u64)max);
	else
		return -(s64)MyRandU64Max((u64)-max);
}

// generate float random number in range 0 (including) to MAX (excluding)
float MyRandFloatMax(float max)
{
	return MyRandFloat() * max;
}

// generate double random number in range 0 (including) to MAX (excluding)
double MyRandDoubleMax(double max)
{
	return MyRandDouble() * max;
}

// generate 8-bit unsigned integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
u8 MyRandU8MinMax(u8 min, u8 max)
{
	return MyRandU8Max(max - min) + min;
}

// generate 16-bit unsigned integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
u16 MyRandU16MinMax(u16 min, u16 max)
{
	return MyRandU16Max(max - min) + min;
}

// generate 32-bit unsigned integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
u32 MyRandU32MinMax(u32 min, u32 max)
{
	return MyRandU32Max(max - min) + min;
}

// generate 64-bit unsigned integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
u64 MyRandU64MinMax(u64 min, u64 max)
{
	return MyRandU64Max(max - min) + min;
}

// generate 8-bit signed integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
s8 MyRandS8MinMax(s8 min, s8 max)
{
	return (s8)MyRandU8Max(max - min) + min;
}

// generate 16-bit signed integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
s16 MyRandS16MinMax(s16 min, s16 max)
{
	return (s16)MyRandU16Max(max - min) + min;
}

// generate 32-bit signed integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
s32 MyRandS32MinMax(s32 min, s32 max)
{
	return (s32)MyRandU32Max(max - min) + min;
}

// generate 64-bit signed integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
s64 MyRandS64MinMax(s64 min, s64 max)
{
	return (s64)MyRandU64Max(max - min) + min;
}

// generate float random number in range MIN to MAX (excluding)
// If MIN > MAX, then number is generated out of interval.
float MyRandFloatMinMax(float min, float max)
{
	return MyRandFloatMax(max - min) + min;
}

// generate double random number in range MIN to MAX (excluding)
// If MIN > MAX, then number is generated out of interval.
double MyRandDoubleMinMax(double min, double max)
{
	return MyRandDoubleMax(max - min) + min;
}

// generate Gaussian float random number (mean = center, default use 0, sigma = width, default use 1)
float MyRandFloatGauss(float mean, float sigma)
{
	float x, y, r;

	do {
		x = MyRandFloat()*2 - 1;
		y = MyRandFloat()*2 - 1;
		r = x*x + y*y;
	} while ((r == 0) || (r > 1));

	r = sqrtf(-2*logf(r)/r);
	return x*r*sigma + mean;
}

// generate Gaussian double random number (mean = center, default use 0, sigma = width, default use 1)
double MyRandDoubleGauss(double mean, double sigma)
{
	double x, y, r;

	do {
		x = MyRandDouble()*2 - 1;
		y = MyRandDouble()*2 - 1;
		r = x*x + y*y;
	} while ((r == 0) || (r > 1));

	r = sqrt(-2*log(r)/r);
	return x*r*sigma + mean;
}

// generate random test integer number (with random number of bits)
u8 MyRandTestU8()
{
	u8 num, shift;
	num = MyRandU8();
	shift = MyRandU8Max(8);
	return num >> shift;
}

u16 MyRandTestU16()
{
	u16 num;
	u8 shift;
	num = MyRandU16();
	shift = MyRandU8Max(16);
	return num >> shift;
}

u32 MyRandTestU32()
{
	u32 num;
	u8 shift;
	num = MyRandU32();
	shift = MyRandU8Max(32);
	return num >> shift;
}

u64 MyRandTestU64()
{
	u64 num;
	u8 shift;
	num = MyRandU64();
	shift = MyRandU8Max(64);
	return num >> shift;
}

s8 MyRandTestS8()
{
	s8 num;
	u8 shift;
	num = MyRandS8();
	shift = MyRandU8Max(8);
	return num >> shift;
}

s16 MyRandTestS16()
{
	s16 num;
	u8 shift;
	num = MyRandS16();
	shift = MyRandU8Max(16);
	return num >> shift;
}

s32 MyRandTestS32()
{
	s32 num;
	u8 shift;
	num = MyRandS32();
	shift = MyRandU8Max(32);
	return num >> shift;
}

s64 MyRandTestS64()
{
	s64 num;
	u8 shift;
	num = MyRandS64();
	shift = MyRandU8Max(64);
	return num >> shift;
}

// random float number for tests
float MyRandTestFloat()
{
	// get flags
	u32 flags = MyRandU32();

	// mode 7 bits (faster special cases)
	int mode = flags & 0x7f;
#if !CHECK_NOINF		// 1 = disable check of infinity ranges
	if (mode == 0)
		return Test_PInfF(); // +1.#INF
	else if (mode == 1)
		return Test_MInfF(); // -1.#INF
	else
#endif
	if (mode == 2)
		return Test_PZeroF(); // +0.0
	else if (mode == 3)
		return Test_MZeroF(); // -0.0

	// sign 1 bit
	int sign = flags & 1;

	// exponent 8 bits
	int exp = flags >> (32-8);
#if !CHECK_NOINF		// 1 = disable check of infinity ranges
	if (exp == FLOAT_EXPINF) exp = FLOAT_EXP1;
#endif

	// mantissa
	u32 mant;
	if ((exp == 0) || (exp >= FLOAT_EXPINF))
		mant = 0;
	else
		mant = MyRandU32() & FLOAT_MANTMASK;

	// random shift mantissa to get power of 2
	int shift = (flags >> 7) & 0x1f; // shift 5 bits
	if ((flags & 2) != 0)
		mant >>= shift;
	else
		mant <<= shift;
	mant &= FLOAT_MANTMASK;

	// compose
	return Test_CompF(mant, exp, sign);
}

// random float number for tests
float MyRandTestFloatMinMax(u8 expmin, u8 expmax)
{
	// get flags
	u32 flags = MyRandU32();

	// mode 7 bits (faster special cases)
	int mode = flags & 0x7f;
#if !CHECK_NOINF		// 1 = disable check of infinity ranges
	if (mode == 0)
		return Test_PInfF(); // +1.#INF
	else if (mode == 1)
		return Test_MInfF(); // -1.#INF
	else
#endif
	if (mode == 2)
		return Test_PZeroF(); // +0.0
	else if (mode == 3)
		return Test_MZeroF(); // -0.0

	// sign 1 bit
	int sign = flags & 1;

	// exponent 8 bits
	int exp = MyRandU8MinMax(expmin, expmax);
#if !CHECK_NOINF		// 1 = disable check of infinity ranges
	while (exp == FLOAT_EXPINF) exp = MyRandU8MinMax(expmin, expmax);
#endif

	// mantissa
	u32 mant;
	if ((exp == 0) || (exp >= FLOAT_EXPINF))
		mant = 0;
	else
		mant = MyRandU32() & FLOAT_MANTMASK;

	// random shift mantissa to get power of 2
	int shift = (flags >> 7) & 0x1f; // shift 5 bits
	if ((flags & 2) != 0)
		mant >>= shift;
	else
		mant <<= shift;
	mant &= FLOAT_MANTMASK;

	// compose
	return Test_CompF(mant, exp, sign);
}

// random double number for tests
double MyRandTestDouble()
{
	// get flags
	u32 flags = MyRandU32();

	// mode 7 bits (faster special cases)
	int mode = flags & 0x7f;
#if !CHECK_NOINF		// 1 = disable check of infinity ranges
	if (mode == 0)
		return Test_PInfD(); // +1.#INF
	else if (mode == 1)
		return Test_MInfD(); // -1.#INF
	else
#endif
	if (mode == 2)
		return Test_PZeroD(); // +0.0
	else if (mode == 3)
		return Test_MZeroD(); // -0.0

	// sign 1 bit
	int sign = flags & 1;

	// exponent 11 bits
	int exp = flags >> (32-11);
#if !CHECK_NOINF		// 1 = disable check of infinity ranges
	if (exp == DOUBLE_EXPINF) exp = DOUBLE_EXP1;
#endif

	// mantissa
	u64 mant;
	if ((exp == 0) || (exp >= DOUBLE_EXPINF))
		mant = 0;
	else
		mant = MyRandU64Fast() & DOUBLE_MANTMASK;

	// random shift mantissa to get power of 2
	int shift = (flags >> 7) & 0x3f; // shift 6 bits
	if ((flags & 2) != 0)
		mant >>= shift;
	else
		mant <<= shift;
	mant &= DOUBLE_MANTMASK;

	// compose
	return Test_CompD(mant, exp, sign);
}

double MyRandTestDoubleMinMax(u16 expmin, u16 expmax)
{
	// get flags
	u32 flags = MyRandU32();

	// mode 7 bits (faster special cases)
	int mode = flags & 0x7f;
#if !CHECK_NOINF		// 1 = disable check of infinity ranges
	if (mode == 0)
		return Test_PInfD(); // +1.#INF
	else if (mode == 1)
		return Test_MInfD(); // -1.#INF
	else
#endif
	if (mode == 2)
		return Test_PZeroD(); // +0.0
	else if (mode == 3)
		return Test_MZeroD(); // -0.0

	// sign 1 bit
	int sign = flags & 1;

	// exponent 11 bits
	int exp = MyRandU16MinMax(expmin, expmax);
#if !CHECK_NOINF		// 1 = disable check of infinity ranges
	while (exp == DOUBLE_EXPINF) exp = MyRandU16MinMax(expmin, expmax);
#endif

	// mantissa
	u64 mant;
	if ((exp == 0) || (exp >= DOUBLE_EXPINF))
		mant = 0;
	else
		mant = MyRandU64Fast() & DOUBLE_MANTMASK;

	// random shift mantissa to get power of 2
	int shift = (flags >> 7) & 0x3f; // shift 6 bits
	if ((flags & 2) != 0)
		mant >>= shift;
	else
		mant <<= shift;
	mant &= DOUBLE_MANTMASK;

	// compose
	return Test_CompD(mant, exp, sign);
}
