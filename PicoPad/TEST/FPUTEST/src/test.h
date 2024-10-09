
// ****************************************************************************
//
//                                 Test
//
// ****************************************************************************

#define FLOAT_BYTES		4			// float - number of bytes
#define FLOAT_BITS		32			// float - number of bits
#define FLOAT_MANTBITS		23			// float - number of bits of mantissa
#define FLOAT_MANTMASK		0x007fffff		// float - mask of mantissa (23 bits)
#define FLOAT_EXPMASK		0xff			// float - exponent mask (8 bits)
#define FLOAT_EXPINF		0xff			// float - exponent infinity (overflow)
#define FLOAT_EXPPINF		0x0ff			// float - exponent positive infinity (overflow)
#define FLOAT_EXPMINF		0x1ff			// float - exponent negative infinity (overflow)
#define FLOAT_EXP1		0x7f			// float - exponent of '1'
#define FLOAT_EXP0		0			// float - exponent zero

#define DOUBLE_BYTES		8			// double - number of bytes
#define DOUBLE_BITS		64			// double - number of bits
#define DOUBLE_MANTBITS		52			// double - number of bits of mantissa
#define DOUBLE_MANTMASK		0x000fffffffffffffULL	// double - mask of mantissa (52 bits)
#define DOUBLE_EXPMASK		0x7ff			// double - exponent mask (11 bits)
#define DOUBLE_EXPINF		0x7ff			// double - exponent infinity (overflow)
#define DOUBLE_EXPPINF		0x7ff			// double - exponent positive infinity (overflow)
#define DOUBLE_EXPMINF		0xfff			// double - exponent negative infinity (overflow)
#define DOUBLE_EXP1		0x3ff			// double - exponent of '1'
#define DOUBLE_EXP0		0			// double - exponent zero

typedef union { float f; u32 n; } float_u32;
typedef union { double f; u64 n; } double_u64;

// convert number to float
INLINE float Test_u32float(u32 n) { float_u32 tmp; tmp.n = n; return tmp.f; }
INLINE double Test_u64double(u64 n) { double_u64 tmp; tmp.n = n; return tmp.f; }

// convert float to number
INLINE u32 Test_floatu32(float f) { float_u32 tmp; tmp.f = f; return tmp.n; }
INLINE u64 Test_doubleu64(double f) { double_u64 tmp; tmp.f = f; return tmp.n; }

// get special numbers
INLINE float Test_PInfF() { return Test_u32float(0x7f800000); }		// +1.#INF
INLINE float Test_MInfF() { return Test_u32float(0xff800000); }		// -1.#INF
INLINE float Test_QNanF() { return Test_u32float(0x7fc00000); }		// +1.#QNAN
INLINE float Test_IndF() { return Test_u32float(0xffc00000); }		// -1.#IND
INLINE float Test_PZeroF() { return Test_u32float(0x00000000); }	// +0.0
INLINE float Test_MZeroF() { return Test_u32float(0x80000000); }	// -0.0

INLINE double Test_PInfD() { return Test_u64double(0x7ff0000000000000ULL); }	// +1.#INF
INLINE double Test_MInfD() { return Test_u64double(0xfff0000000000000ULL); }	// -1.#INF
INLINE double Test_QNanD() { return Test_u64double(0x7ff8000000000000ULL); }	// +1.#QNAN
INLINE double Test_IndD() { return Test_u64double(0xfff8000000000000ULL); }	// -1.#IND
INLINE double Test_PZeroD() { return Test_u64double(0x0000000000000000ULL); }	// +0.0
INLINE double Test_MZeroD() { return Test_u64double(0x8000000000000000ULL); }	// -0.0

// extract signed exponent from the number
int Test_GetExpF(float num);
int Test_GetExpD(double num);
int Test_GetExpSignF(float num);
int Test_GetExpSignD(double num);

// extract sign from the number (0 or 1)
int Test_GetSignF(float num);
int Test_GetSignD(double num);

// extract mantisa from the number
u32 Test_GetMantF(float num);
u64 Test_GetMantD(double num);

// compose float number
float Test_CompF(u32 mant, int exp, int sign);
double Test_CompD(u64 mant, int exp, int sign);

// check - number is overflow
Bool Test_IsOverF(float num);
Bool Test_IsOverD(double num);
Bool Test_IsPOverF(float num);
Bool Test_IsPOverD(double num);
Bool Test_IsMOverF(float num);
Bool Test_IsMOverD(double num);
INLINE Bool Test_IsIndF(float num) { return Test_floatu32(num) == Test_floatu32(Test_IndF()); }
INLINE Bool Test_IsIndD(double num) { return Test_doubleu64(num) == Test_doubleu64(Test_IndD()); }
INLINE Bool Test_IsNanF(float num) { return Test_IsOverF(num) && (Test_GetMantF(num) != 0); }
INLINE Bool Test_IsNanD(double num) { return Test_IsOverD(num) && (Test_GetMantD(num) != 0); }

// check - number is zero
Bool Test_IsZeroF(float num);
Bool Test_IsZeroD(double num);

// check - number is negative
Bool Test_IsNegF(float num);
Bool Test_IsNegD(double num);

// check difference of numbers (returns number of different bits 0..23, 0..52)
int Text_CheckDifF(float num1, float num2);
int Text_CheckDifD(double num1, double num2);

// ============ tests

extern int DifSum, DifMax;
extern int TestCalib, TestTime;

// calibrate time
void Test_Calib();

// add difference to info list
void TestInfoAddF(float ref, float tst);
void TestInfoAddD(double ref, double tst);

// compose floating point with magnitude of 'num' and sign of 'sign'
void Test_copysignf(int loop);
void Test_copysign(int loop);

// check if number is an integer
void Test_isintf(int loop);
void Test_isintd(int loop);

// check if number is odd integer (must be an integer ..., -3, -1, 1, 3, 5,..)
void Test_isoddintf(int loop);
void Test_isoddintd(int loop);

// check if number is power of 2
void Test_ispow2f(int loop);
void Test_ispow2d(int loop);

// multiply number by power of 2 (num * 2^exp)
void Test_ldexpf(int loop);
void Test_ldexp(int loop);

// Addition, x + y
void Test_fadd(int loop);
void Test_dadd(int loop);

// Subtraction, x - y
void Test_fsub(int loop);
void Test_dsub(int loop);

// Multiplication, x * y
void Test_fmul(int loop);
void Test_dmul(int loop);

// Square, x^2 
void Test_fsqr(int loop);
void Test_dsqr(int loop);

// Division, x / y
void Test_fdiv(int loop);
void Test_ddiv(int loop);

// Fast fivision, x / y
void Test_fdiv_fast(int loop);
void Test_ddiv_fast(int loop);

// Reciprocal 1 / x
void Test_frec(int loop);
void Test_drec(int loop);

// get remainder of division x/y, rounded towards zero
//  fmod (x, y) = x - tquote*y, where tquote is truncated (i.e. rounded towards zero) result of x/y
void Test_fmodf(int loop);
void Test_fmod(int loop);

// compute remainder and quotient of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
void Test_remquof(int loop);
void Test_remquo(int loop);

// get remainder of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
void Test_remainderf(int loop);
void Test_remainder(int loop);

// Compare, x ? y
// Returns: 0 if x==y, -1 if x<y, +1 if x>y
void Test_fcmp(int loop);
void Test_dcmp(int loop);

// Compare if x==y
void Test_fcmpeq(int loop);
void Test_dcmpeq(int loop);

// Compare if x<y
void Test_fcmplt(int loop);
void Test_dcmplt(int loop);

// Compare if x<=y
void Test_fcmple(int loop);
void Test_dcmple(int loop);

// Compare if x>=y
void Test_fcmpge(int loop);
void Test_dcmpge(int loop);

// Compare if x>y
void Test_fcmpgt(int loop);
void Test_dcmpgt(int loop);

// Check if comparison is unordered (either input is NaN)
void Test_fcmpun(int loop);
void Test_dcmpun(int loop);

// Convert signed int to float
void Test_int2float(int loop);
void Test_int2double(int loop);

// Convert unsigned int to float
void Test_uint2float(int loop);
void Test_uint2double(int loop);

// Convert 64-bit signed int to float
void Test_int642float(int loop);
void Test_int642double(int loop);

// Convert 64-bit unsigned int to float
void Test_uint642float(int loop);
void Test_uint642double(int loop);

// Convert signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
void Test_fix2float(int loop);
void Test_fix2double(int loop);

// Convert unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
void Test_ufix2float(int loop);
void Test_ufix2double(int loop);

// Convert 64-bit signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
void Test_fix642float(int loop);
void Test_fix642double(int loop);

// Convert 64-bit unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
void Test_ufix642float(int loop);
void Test_ufix642double(int loop);

// Convert float to signed int, rounding to zero (C-style int conversion)
void Test_float2int_z(int loop);
void Test_double2int_z(int loop);

// Convert float to signed int, rounding down
void Test_float2int(int loop);
void Test_double2int(int loop);

// Convert float to unsigned int, rounding down
void Test_float2uint(int loop);
void Test_double2uint(int loop);

// Convert float to 64-bit signed int, rounding to zero (C-style int conversion)
void Test_float2int64_z(int loop);
void Test_double2int64_z(int loop);

// Convert float to 64-bit signed int, rounding down
void Test_float2int64(int loop);
void Test_double2int64(int loop);

// Convert float to 64-bit unsigned int, rounding down
void Test_float2uint64(int loop);
void Test_double2uint64(int loop);

// Convert float to signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
void Test_float2fix(int loop);
void Test_double2fix(int loop);

// Convert float to unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
void Test_float2ufix(int loop);
void Test_double2ufix(int loop);

// Convert float to 64-bit signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
void Test_float2fix64(int loop);
void Test_double2fix64(int loop);

// Convert float to 64-bit unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
void Test_float2ufix64(int loop);
void Test_double2ufix64(int loop);

// round number towards zero
void Test_truncf(int loop);
void Test_trunc(int loop);

// round number to nearest integer
void Test_roundf(int loop);
void Test_round(int loop);

// round number down to integer
void Test_floorf(int loop);
void Test_floor(int loop);

// round number up to integer
void Test_ceilf(int loop);
void Test_ceil(int loop);

// Square root
void Test_sqrtf(int loop);
void Test_sqrt(int loop);

// Fast square root
void Test_sqrtf_fast(int loop);
void Test_sqrt_fast(int loop);

// convert degrees to radians
void Test_deg2radf(int loop);
void Test_deg2rad(int loop);

// convert radians to degrees
void Test_rad2degf(int loop);
void Test_rad2deg(int loop);

// sine in radians
void Test_sinf(int loop);
void Test_sin(int loop);

// sine in degrees
void Test_sinf_deg(int loop);
void Test_sin_deg(int loop);

// cosine in radians
void Test_cosf(int loop);
void Test_cos(int loop);

// cosine in degrees
void Test_cosf_deg(int loop);
void Test_cos_deg(int loop);

// sine-cosine in radians
void Test_sincosf(int loop);
void Test_sincos(int loop);

// sine-cosine in degrees
void Test_sincosf_deg(int loop);
void Test_sincos_deg(int loop);

// tangent in radians
void Test_tanf(int loop);
void Test_tan(int loop);

// tangent in degrees
void Test_tanf_deg(int loop);
void Test_tan_deg(int loop);

// arc sine in radians
void Test_asinf(int loop);
void Test_asin(int loop);

// arc sine in degrees
void Test_asinf_deg(int loop);
void Test_asin_deg(int loop);

// arc cosine in radians
void Test_acosf(int loop);
void Test_acos(int loop);

// arc cosine in degrees
void Test_acosf_deg(int loop);
void Test_acos_deg(int loop);

// arc tangent in radians
void Test_atanf(int loop);
void Test_atan(int loop);

// arc tangent in degrees
void Test_atanf_deg(int loop);
void Test_atan_deg(int loop);

// arc tangent of y/x in radians
void Test_atan2f(int loop);
void Test_atan2(int loop);

// arc tangent of y/x in degrees
void Test_atan2f_deg(int loop);
void Test_atan2_deg(int loop);

// hyperbolic sine
void Test_sinhf(int loop);
void Test_sinh(int loop);

// hyperbolic cosine
void Test_coshf(int loop);
void Test_cosh(int loop);

// hyperbolic tangent
void Test_tanhf(int loop);
void Test_tanh(int loop);

// inverse hyperbolic sine
void Test_asinhf(int loop);
void Test_asinh(int loop);

// inverse hyperbolic cosine
void Test_acoshf(int loop);
void Test_acosh(int loop);

// inverse hyperbolic tangent
void Test_atanhf(int loop);
void Test_atanh(int loop);

// Natural exponent
void Test_expf(int loop);
void Test_exp(int loop);

// Natural logarithm
void Test_logf(int loop);
void Test_log(int loop);

// exponent with base 2
void Test_exp2f(int loop);
void Test_exp2(int loop);

// logarithm with base 2
void Test_log2f(int loop);
void Test_log2(int loop);

// exponent with base 10
void Test_exp10f(int loop);
void Test_exp10(int loop);

// logarithm with base 10
void Test_log10f(int loop);
void Test_log10(int loop);

// expf(x) - 1
void Test_expm1f(int loop);
void Test_expm1(int loop);

// logf(x + 1)
void Test_log1pf(int loop);
void Test_log1p(int loop);

// x*y + z
void Test_fmaf(int loop);
void Test_fma(int loop);

// power by integer, x^y
void Test_powintf(int loop);
void Test_powint(int loop);

// power x^y
void Test_powf(int loop);
void Test_pow(int loop);

// square root of sum of squares (hypotenuse), sqrt(x*x + y*y)
void Test_hypotf(int loop);
void Test_hypot(int loop);

// cube root, sqrt3(x), x^(1/3)
void Test_cbrtf(int loop);
void Test_cbrt(int loop);

// absolute value
void Test_absf(int loop);
void Test_absd(int loop);

// Convert float to double
void Test_float2double(int loop);

// Convert double to float
void Test_double2float(int loop);

// round to given number of significant digits
void Test_rounddigf(int loop);
void Test_rounddig(int loop);
