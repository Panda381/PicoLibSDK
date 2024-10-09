
// ****************************************************************************
//
//                         Random number generator
//
// ****************************************************************************

// Seed of random number generator
extern u64 MyRandSeed;

// get seed of random number generator
u64 MyRandGet();

// set seed of random number generator
void MyRandSet(u64 seed);

// shift random number generator and return 32-bit random number
u32 MyRandShift();

// generate 8-bit unsigned integer random number
u8 MyRandU8();

// generate 16-bit unsigned integer random number
u16 MyRandU16();

// generate 32-bit unsigned integer random number
INLINE u32 MyRandU32() { return MyRandShift(); }

// generate 64-bit unsigned integer random number
u64 MyRandU64();
u64 MyRandU64Fast();

// generate 8-bit signed integer random number
INLINE s8 MyRandS8() { return (s8)MyRandU8(); }

// generate 16-bit signed integer random number
INLINE s16 MyRandS16() { return (s16)MyRandU16(); }

// generate 32-bit signed integer random number
INLINE s32 MyRandS32() { return (s32)MyRandShift(); }

// generate 64-bit signed integer random number
INLINE s64 MyRandS64() { return (s64)MyRandU64(); }

// generate float random number in range 0 (including) to 1 (excluding)
// (maximum number is 0.99999994, minimal step 0.00000006)
float MyRandFloat();

// generate double random number in range 0 (including) to 1 (excluding)
// (maximum number is 0.999999999999996, step 0.0000000000000035, 14 valid digits)
double MyRandDouble();

// generate 8-bit unsigned integer random number in range 0 to MAX (including)
u8 MyRandU8Max(u8 max);

// generate 16-bit unsigned integer random number in range 0 to MAX (including)
u16 MyRandU16Max(u16 max);

// generate 32-bit unsigned integer random number in range 0 to MAX (including)
u32 MyRandU32Max(u32 max);

// generate 64-bit unsigned integer random number in range 0 to MAX (including)
u64 MyRandU64Max(u64 max);

// generate 8-bit signed integer random number in range 0 to MAX (including, 'max' can be negative)
s8 MyRandS8Max(s8 max);

// generate 16-bit signed integer random number in range 0 to MAX (including, 'max' can be negative)
s16 MyRandS16Max(s16 max);

// generate 32-bit signed integer random number in range 0 to MAX (including, 'max' can be negative)
s32 MyRandS32Max(s32 max);

// generate 64-bit signed integer random number in range 0 to MAX (including, 'max' can be negative)
s64 MyRandS64Max(s64 max);

// generate float random number in range 0 (including) to MAX (excluding)
float MyRandFloatMax(float max);

// generate double random number in range 0 (including) to MAX (excluding)
double MyRandDoubleMax(double max);

// generate 8-bit unsigned integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
u8 MyRandU8MinMax(u8 min, u8 max);

// generate 16-bit unsigned integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
u16 MyRandU16MinMax(u16 min, u16 max);

// generate 32-bit unsigned integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
u32 MyRandU32MinMax(u32 min, u32 max);

// generate 64-bit unsigned integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
u64 MyRandU64MinMax(u64 min, u64 max);

// generate 8-bit signed integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
s8 MyRandS8MinMax(s8 min, s8 max);

// generate 16-bit signed integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
s16 MyRandS16MinMax(s16 min, s16 max);

// generate 32-bit signed integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
s32 MyRandS32MinMax(s32 min, s32 max);

// generate 64-bit signed integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
s64 MyRandS64MinMax(s64 min, s64 max);

// generate float random number in range MIN (including) to MAX (excluding)
//  The order of MIN and MAX does not matter.
float MyRandFloatMinMax(float min, float max);

// generate double random number in range MIN (including) to MAX (excluding)
//  The order of MIN and MAX does not matter.
double MyRandDoubleMinMax(double min, double max);

// generate Gaussian float random number (mean = center, default use 0, sigma = width, default use 1)
float MyRandFloatGauss(float mean, float sigma);

// generate Gaussian double random number (mean = center, default use 0, sigma = width, default use 1)
double MyRandDoubleGauss(double mean, double sigma);

// generate random test integer number (with random number of bits)
u8 MyRandTestU8();
u16 MyRandTestU16();
u32 MyRandTestU32();
u64 MyRandTestU64();
s8 MyRandTestS8();
s16 MyRandTestS16();
s32 MyRandTestS32();
s64 MyRandTestS64();

// random float number for tests
float MyRandTestFloat();
float MyRandTestFloatMinMax(u8 expmin, u8 expmax);

// random double number for tests
double MyRandTestDouble();
double MyRandTestDoubleMinMax(u16 expmin, u16 expmax);
