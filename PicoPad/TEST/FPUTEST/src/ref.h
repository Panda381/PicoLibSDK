
// ****************************************************************************
//
//                          Reference functions
//
// ****************************************************************************

// compose floating point with magnitude of 'num' and sign of 'sign'
float Ref_copysignf(float num, float sign);
double Ref_copysign(double num, double sign);

// check if number is an integer
Bool Ref_isintf(float num);
Bool Ref_isintd(double num);

// check if number is odd integer (must be an integer ..., -3, -1, 1, 3, 5,..)
Bool Ref_isoddintf(float num);
Bool Ref_isoddintd(double num);

// check if number is power of 2
Bool Ref_ispow2f(float num);
Bool Ref_ispow2d(double num);

// multiply number by power of 2 (num * 2^exp)
float Ref_ldexpf(float num, int exp);
double Ref_ldexp(double num, int exp);

// Addition, x + y
float Ref_fadd(float x, float y);
double Ref_dadd(double x, double y);

// Subtraction, x - y
float Ref_fsub(float x, float y);
double Ref_dsub(double x, double y);

// Multiplication, x * y
float Ref_fmul(float x, float y);
double Ref_dmul(double x, double y);

// Square, x^2 
float Ref_fsqr(float x);
double Ref_dsqr(double x);

// Division, x / y
float Ref_fdiv(float x, float y);
double Ref_ddiv(double x, double y);

// Reciprocal 1 / x
float Ref_frec(float x);
double Ref_drec(double x);

// get remainder of division x/y, rounded towards zero
//  fmod (x, y) = x - tquote*y, where tquote is truncated (i.e. rounded towards zero) result of x/y
float Ref_fmodf(float x, float y);
double Ref_fmod(double x, double y);

// compute remainder and quotient of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
float Ref_remquof(float x, float y, int* quo);
double Ref_remquo(double x, double y, int* quo);

// get remainder of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
float Ref_remainderf(float x, float y);
double Ref_remainder(double x, double y);

// Compare, x ? y
// Returns: 0 if x==y, -1 if x<y, +1 if x>y
s8 Ref_fcmp(float x, float y);
s8 Ref_dcmp(double x, double y);

// Compare if x==y
Bool Ref_fcmpeq(float x, float y);
Bool Ref_dcmpeq(double x, double y);

// Compare if x<y
Bool Ref_fcmplt(float x, float y);
Bool Ref_dcmplt(double x, double y);

// Compare if x<=y
Bool Ref_fcmple(float x, float y);
Bool Ref_dcmple(double x, double y);

// Compare if x>=y
Bool Ref_fcmpge(float x, float y);
Bool Ref_dcmpge(double x, double y);

// Compare if x>y
Bool Ref_fcmpgt(float x, float y);
Bool Ref_dcmpgt(double x, double y);

// Check if comparison is unordered (either input is NaN)
Bool Ref_fcmpun(float x, float y);
Bool Ref_dcmpun(double x, double y);

// Convert signed int to float
float Ref_int2float(s32 num);
double Ref_int2double(s32 num);

// Convert unsigned int to float
float Ref_uint2float(u32 num);
double Ref_uint2double(u32 num);

// Convert 64-bit signed int to float
float Ref_int642float(s64 num);
double Ref_int642double(s64 num);

// Convert 64-bit unsigned int to float
float Ref_uint642float(u64 num);
double Ref_uint642double(u64 num);

// Convert signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
float Ref_fix2float(s32 num, int e);
double Ref_fix2double(s32 num, int e);

// Convert unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
float Ref_ufix2float(u32 num, int e);
double Ref_ufix2double(u32 num, int e);

// Convert 64-bit signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
float Ref_fix642float(s64 num, int e);
double Ref_fix642double(s64 num, int e);

// Convert 64-bit unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
float Ref_ufix642float(u64 num, int e);
double Ref_ufix642double(u64 num, int e);

// Convert float to signed int, rounding to zero (C-style int conversion)
s32 Ref_float2int_z(float num);
s32 Ref_double2int_z(double num);

// Convert float to signed int, rounding down
s32 Ref_float2int(float num);
s32 Ref_double2int(double num);

// Convert float to unsigned int, rounding down
u32 Ref_float2uint(float num);
u32 Ref_double2uint(double num);

// Convert float to 64-bit signed int, rounding to zero (C-style int conversion)
s64 Ref_float2int64_z(float num);
s64 Ref_double2int64_z(double num);

// Convert float to 64-bit signed int, rounding down
s64 Ref_float2int64(float num);
s64 Ref_double2int64(double num);

// Convert float to 64-bit unsigned int, rounding down
u64 Ref_float2uint64(float num);
u64 Ref_double2uint64(double num);

// Convert float to signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
s32 Ref_float2fix(float num, int e);
s32 Ref_double2fix(double num, int e);

// Convert float to unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
u32 Ref_float2ufix(float num, int e);
u32 Ref_double2ufix(double num, int e);

// Convert float to 64-bit signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
s64 Ref_float2fix64(float num, int e);
s64 Ref_double2fix64(double num, int e);

// Convert float to 64-bit unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
u64 Ref_float2ufix64(float num, int e);
u64 Ref_double2ufix64(double num, int e);

// round number towards zero
float Ref_truncf(float num);
double Ref_trunc(double num);

// round number to nearest integer
float Ref_roundf(float num);
double Ref_round(double num);

// round number down to integer
float Ref_floorf(float num);
double Ref_floor(double num);

// round number up to integer
float Ref_ceilf(float num);
double Ref_ceil(double num);

// Square root
float Ref_sqrtf(float x);
double Ref_sqrt(double x);

// convert degrees to radians
float Ref_deg2radf(float x);
double Ref_deg2rad(double x);

// convert radians to degrees
float Ref_rad2degf(float x);
double Ref_rad2deg(double x);

// sine in radians
float Ref_sinf(float x);
double Ref_sin(double x);

// sine in degrees
float Ref_sinf_deg(float x);
double Ref_sin_deg(double x);

// cosine in radians
float Ref_cosf(float x);
double Ref_cos(double x);

// cosine in degrees
float Ref_cosf_deg(float x);
double Ref_cos_deg(double x);

// sine-cosine in radians
void Ref_sincosf(float x, float* psin, float* pcos);
void Ref_sincos(double x, double* psin, double* pcos);

// sine-cosine in degrees
void Ref_sincosf_deg(float x, float* psin, float* pcos);
void Ref_sincos_deg(double x, double* psin, double* pcos);

// tangent in radians
float Ref_tanf(float x);
double Ref_tan(double x);

// tangent in degrees
float Ref_tanf_deg(float x);
double Ref_tan_deg(double x);

// cotangent in radians
float Ref_cotanf(float x);
double Ref_cotan(double x);

// cotangent in degrees
float Ref_cotanf_deg(float x);
double Ref_cotan_deg(double x);

// arc sine in radians
float Ref_asinf(float x);
double Ref_asin(double x);

// arc sine in degrees
float Ref_asinf_deg(float x);
double Ref_asin_deg(double x);

// arc cosine in radians
float Ref_acosf(float x);
double Ref_acos(double x);

// arc cosine in degrees
float Ref_acosf_deg(float x);
double Ref_acos_deg(double x);

// arc tangent in radians
float Ref_atanf(float x);
double Ref_atan(double x);

// arc tangent in degrees
float Ref_atanf_deg(float x);
double Ref_atan_deg(double x);

// arc cotangent in radians
float Ref_acotanf(float x);
double Ref_acotan(double x);

// arc cotangent in degrees
float Ref_acotanf_deg(float x);
double Ref_acotan_deg(double x);

// arc tangent of y/x in radians
float Ref_atan2f(float y, float x);
double Ref_atan2(double y, double x);

// arc tangent of y/x in radians
float Ref_atan2f_deg(float y, float x);
double Ref_atan2_deg(double y, double x);

// hyperbolic sine
float Ref_sinhf(float x);
double Ref_sinh(double x);

// hyperbolic cosine
float Ref_coshf(float x);
double Ref_cosh(double x);

// hyperbolic tangent
float Ref_tanhf(float x);
double Ref_tanh(double x);

// inverse hyperbolic sine
float Ref_asinhf(float x);
double Ref_asinh(double x);

// inverse hyperbolic cosine
float Ref_acoshf(float x);
double Ref_acosh(double x);

// inverse hyperbolic tangent
float Ref_atanhf(float x);
double Ref_atanh(double x);

// Natural exponent
float Ref_expf(float x);
double Ref_exp(double x);

// Natural logarithm
float Ref_logf(float x);
double Ref_log(double x);

// exponent with base 2
float Ref_exp2f(float x);
double Ref_exp2(double x);

// logarithm with base 2
float Ref_log2f(float x);
double Ref_log2(double x);

// exponent with base 10
float Ref_exp10f(float x);
double Ref_exp10(double x);

// logarithm with base 10
float Ref_log10f(float x);
double Ref_log10(double x);

// expf(x) - 1
float Ref_expm1f(float x);
double Ref_expm1(double x);

// logf(x + 1)
float Ref_log1pf(float x);
double Ref_log1p(double x);

// x*y + z
float Ref_fmaf(float x, float y, float z);
double Ref_fma(double x, double y, double z);

// power by integer, x^y
float Ref_powintf(float x, int y);
double Ref_powint(double x, int y);

// power x^y
float Ref_powf(float x, float y);
double Ref_pow(double x, double y);

// square root of sum of squares (hypotenuse), sqrt(x*x + y*y)
float Ref_hypotf(float x, float y);
double Ref_hypot(double x, double y);

// cube root, sqrt3(x), x^(1/3)
float Ref_cbrtf(float x);
double Ref_cbrt(double x);

// absolute value
float Ref_absf(float x);
double Ref_absd(double x);

// Convert float to double
double Ref_float2double(float num);

// Convert double to float
float Ref_double2float(double num);

// round to given number of significant digits (digits<=0 to use default number of digits)
// @TODO: probably will be deleted (accuracy cannot be guaranteed)
//float Ref_rounddigf(float x, int digits);
//double Ref_rounddig(double x, int digits);





float expf_fast(float x);
float logf_fast(float x);

void sincosf_fast(float x, float* psin, float* pcos);
void sincosf_deg_fast(float x, float* psin, float* pcos);
float sinf_fast(float x);
float sinf_deg_fast(float x);
float cosf_fast(float x);
float cosf_deg_fast(float x);
float tanf_fast(float x);
float tanf_deg_fast(float x);
float cotanf_fast(float x);
float cotanf_deg_fast(float x);

float asinf_fast(float x);
float asinf_deg_fast(float x);
float acosf_fast(float x);
float acosf_deg_fast(float x);


float atanf_fast(float x);
float atanf_deg_fast(float x);
float acotanf_fast(float x);
float acotanf_deg_fast(float x);
