
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define LOOPS		200000		// number of tests (best in range 100000 to 1000000, default 200000)
#define LOOPS_FAST	(LOOPS/10)	// number of tests - fast
#define LOOPS_SLOW	(LOOPS*4)	// number of tests - slow

void MyPrint(const char* name, double maxavg, int max, int loops)
{
	DrawPrint("%s:", name);
	while (DrawPrintPos < 11) DrawPrint(" ");

	int pos = (DrawPrintPos > 12) ? 4 : ((DrawPrintPos > 11) ? 5 : 6);
	DrawPrint("%s%*.3gns avg=%.3f max=%d\n",
		((DifMax <= max) && ((double)DifSum/loops <= maxavg)) ? "OK" : "ERR",
		 pos, (double)TestTime, (double)DifSum/loops, DifMax);
}

// Max deviation is not tested for:
// - goniometric functions due to periodicity - small deviations in PI
//   multiples can generate large differences.
// - logarithms, arc-hyp and pow functions - a small deviation from
//   a value of 1 can generate large differences in the results.
// - fma function - result can be near 0 and therefore inaccurate.

// To use "libc" functions, set USE_FLOATLIBC and USE_DOUBLELIBC in Makefile.inc to 1.

float myacotanf(float x) { return (float)(PI/2 - atan(x)); }

int main()
{
	// print title
	DrawPrint("\fCPU=%s%d CLK=%dMHz\n",

#if RP2040
		"RP2040-B", RomGetVersion() - 1,
#elif RISCV
		"RP2350-RISCV-A", RomGetVersion(),
#else
		"RP2350-ARM-A", RomGetVersion(),
#endif
		(ClockGetHz(CLK_SYS)+200000)/1000000);

	// calibrate time
	DrawPrint("Calibrating...");
	Test_Calib();
	DrawPrint("OK\n");

// single precision

	DrawPrint("--- Checking float functions:\n");

	// initialize random generator
	MyRandSet(123456789123456789ULL);

	// compose floating point with magnitude of 'num' and sign of 'sign'
	Test_copysignf(LOOPS_SLOW); MyPrint("copysignf", 0, 0, LOOPS_SLOW);

	// check if number is an integer
	Test_isintf(LOOPS_SLOW); MyPrint("isintf", 0, 0, LOOPS_SLOW);

	// check if number is odd integer (must be an integer ..., -3, -1, 1, 3, 5,..)
	Test_isoddintf(LOOPS_SLOW); MyPrint("isoddintf", 0, 0, LOOPS_SLOW);

	// check if number is power of 2
	Test_ispow2f(LOOPS_SLOW); MyPrint("ispow2f", 0, 0, LOOPS_SLOW);

	// multiply number by power of 2 (num * 2^exp)
	Test_ldexpf(LOOPS_SLOW); MyPrint("ldexpf", 0, 0, LOOPS_SLOW);

	// Addition, x + y
	Test_fadd(LOOPS); MyPrint("fadd", 0.01, 1, LOOPS);

	// Subtraction, x - y
	Test_fsub(LOOPS); MyPrint("fsub", 0.01, 1, LOOPS);

	// Multiplication, x * y
	Test_fmul(LOOPS); MyPrint("fmul", 0.01, 1, LOOPS);

	// Square, x^2 
	Test_fsqr(LOOPS); MyPrint("fsqr", 0.01, 1, LOOPS);

	// Division, x / y
	Test_fdiv(LOOPS_FAST); MyPrint("fdiv", 0.03, 1, LOOPS_FAST);

	// Reciprocal 1 / x
	Test_frec(LOOPS_FAST); MyPrint("frec", 0.01, 1, LOOPS_FAST);

	// get remainder of division x/y, rounded towards zero
	Test_fmodf(LOOPS_FAST); MyPrint("fmodf", 0.01, 1, LOOPS_FAST);

	// compute remainder and quotient of division x/y, rounded towards the even number
	Test_remquof(LOOPS); MyPrint("remquof", 0.01, 3, LOOPS);

	// get remainder of division x/y, rounded towards the even number
	Test_remainderf(LOOPS); MyPrint("remainderf", 0.01, 2, LOOPS);

	// Compare, x ? y
	Test_fcmp(LOOPS_SLOW); MyPrint("fcmp", 0, 0, LOOPS_SLOW);

	// Compare if x==y
	Test_fcmpeq(LOOPS_SLOW); MyPrint("fcmpeq", 0, 0, LOOPS_SLOW);

	// Compare if x<y
	Test_fcmplt(LOOPS_SLOW); MyPrint("fcmplt", 0, 0, LOOPS_SLOW);

	// Compare if x<=y
	Test_fcmple(LOOPS_SLOW); MyPrint("fcmple", 0, 0, LOOPS_SLOW);

	// Compare if x>=y
	Test_fcmpge(LOOPS_SLOW); MyPrint("fcmpge", 0, 0, LOOPS_SLOW);

	// Compare if x>y
	Test_fcmpgt(LOOPS_SLOW); MyPrint("fcmpgt", 0, 0, LOOPS_SLOW);

	// Check if comparison is unordered (both inputs are NaN)
	Test_fcmpun(LOOPS_SLOW); MyPrint("fcmpun", 0, 0, LOOPS_SLOW);

	// Convert signed int to float
	Test_int2float(LOOPS_SLOW); MyPrint("int2float", 0.02, 1, LOOPS_SLOW);

	// Convert unsigned int to float
	Test_uint2float(LOOPS_SLOW); MyPrint("uint2float", 0.02, 1, LOOPS_SLOW);

	// Convert 64-bit signed int to float
	Test_int642float(LOOPS_SLOW); MyPrint("int642float", 0.01, 1, LOOPS_SLOW);

	// Convert 64-bit unsigned int to float
	Test_uint642float(LOOPS_SLOW); MyPrint("uint642float", 0.01, 1, LOOPS_SLOW);

	// Convert signed fixed point to float
	Test_fix2float(LOOPS_SLOW); MyPrint("fix2float", 0.02, 1, LOOPS_SLOW);

	// Convert unsigned fixed point to float
	Test_ufix2float(LOOPS_SLOW); MyPrint("ufix2float", 0.02, 1, LOOPS_SLOW);

	// Convert 64-bit signed fixed point to float
	Test_fix642float(LOOPS_SLOW); MyPrint("fix642float", 0.01, 1, LOOPS_SLOW);

	// Convert 64-bit unsigned fixed point to float
	Test_ufix642float(LOOPS_SLOW); MyPrint("ufix642float", 0.01, 1, LOOPS_SLOW);

	// Convert float to signed int, rounding to zero (C-style int conversion)
	Test_float2int_z(LOOPS_SLOW); MyPrint("float2int_z", 0.02, 1, LOOPS_SLOW);

	// Convert float to signed int, rounding down
	Test_float2int(LOOPS_SLOW); MyPrint("float2int", 0.02, 1, LOOPS_SLOW);

	// Convert float to unsigned int, rounding down
	Test_float2uint(LOOPS_SLOW); MyPrint("float2uint", 0.4, 1, LOOPS_SLOW);

	// Convert float to 64-bit signed int, rounding to zero (C-style int conversion)
	Test_float2int64_z(LOOPS_SLOW); MyPrint("float2int64_z", 0.01, 1, LOOPS_SLOW);

	// Convert float to 64-bit signed int, rounding down
	Test_float2int64(LOOPS_SLOW); MyPrint("float2int64", 0.01, 1, LOOPS_SLOW);

	// Convert float to 64-bit unsigned int, rounding down
	Test_float2uint64(LOOPS_SLOW); MyPrint("float2uint64", 0.01, 1, LOOPS_SLOW);

	// Convert float to signed fixed point, rounding down
	Test_float2fix(LOOPS_SLOW); MyPrint("float2fix", 0.1, 1, LOOPS_SLOW);

	// Convert float to unsigned fixed point, rounding down
	Test_float2ufix(LOOPS_SLOW); MyPrint("float2ufix", 0.32, 1, LOOPS_SLOW);

	// Convert float to 64-bit signed fixed point, rounding down
	Test_float2fix64(LOOPS_SLOW); MyPrint("float2fix64", 0.1, 1, LOOPS_SLOW);

	// Convert float to 64-bit unsigned fixed point, rounding down
	Test_float2ufix64(LOOPS_SLOW); MyPrint("float2ufix64", 0.01, 1, LOOPS_SLOW);

  	// round number towards zero
	Test_truncf(LOOPS); MyPrint("truncf", 0.01, 1, LOOPS);

	// round number to nearest integer
	Test_roundf(LOOPS); MyPrint("roundf", 0.01, 1, LOOPS);

	// round to given number of significant digits
	// @TODO: probably will be deleted (accuracy cannot be guaranteed)
//	Test_rounddigf(LOOPS_FAST); MyPrint("rounddigf", 5, 24, LOOPS_FAST);

	// round number down to integer
	Test_floorf(LOOPS); MyPrint("floorf", 0.01, 1, LOOPS);

	// round number up to integer
	Test_ceilf(LOOPS); MyPrint("ceilf", 0.01, 1, LOOPS);

	// Square root
	Test_sqrtf(LOOPS_FAST); MyPrint("sqrtf", 0.15, 1, LOOPS_FAST);
	Test_conFF_sqrtf(); DrawPrint("sqrtf consistency: %s (%d)\n", (DifMax <= 4) ? "OK" : "ERROR", DifMax);

	// convert degrees to radians
	Test_deg2radf(LOOPS); MyPrint("deg2radf", 0.06, 1, LOOPS);

	// convert radians to degrees
	Test_rad2degf(LOOPS); MyPrint("rad2degf", 0.11, 1, LOOPS);

	// sine in radians
	Test_sinf(LOOPS_FAST); MyPrint("sinf", 3, 24, LOOPS_FAST);

	// sine in degrees
	Test_sinf_deg(LOOPS_FAST); MyPrint("sinf_deg", 3.2, 24, LOOPS_FAST);

	// cosine in radians
	Test_cosf(LOOPS_FAST); MyPrint("cosf", 0.6, 24, LOOPS_FAST);

	// cosine in degrees
	Test_cosf_deg(LOOPS_FAST); MyPrint("cosf_deg", 1.0, 24, LOOPS_FAST);

	// sine-cosine in radians
	Test_sincosf(LOOPS_FAST); MyPrint("sincosf", 1.7, 24, LOOPS_FAST);

	// sine-cosine in degrees
	Test_sincosf_deg(LOOPS_FAST); MyPrint("sincosf_deg", 2.0, 24, LOOPS_FAST);

	// tangent in radians
	Test_tanf(LOOPS_FAST); MyPrint("tanf", 3.0, 24, LOOPS_FAST);

	// tangent in degrees
	Test_tanf_deg(LOOPS_FAST); MyPrint("tanf_deg", 3.5, 24, LOOPS_FAST);

	// cotangent in radians
	Test_cotanf(LOOPS_FAST); MyPrint("cotanf", 3.0, 24, LOOPS_FAST);

	// cotangent in degrees
	Test_cotanf_deg(LOOPS_FAST); MyPrint("cotanf_deg", 3.5, 24, LOOPS_FAST);

	// arc sine in radians
	Test_asinf(LOOPS_FAST); MyPrint("asinf", 2.0, 10, LOOPS_FAST);

	// arc sine in degrees
	Test_asinf_deg(LOOPS_FAST); MyPrint("asinf_deg", 2.2, 12, LOOPS_FAST);

	// arc cosine in radians
	Test_acosf(LOOPS_FAST); MyPrint("acosf", 0.4, 12, LOOPS_FAST);

	// arc cosine in degrees
	Test_acosf_deg(LOOPS_FAST); MyPrint("acosf_deg", 0.4, 6, LOOPS_FAST);

	// arc tangent in radians
	Test_atanf(LOOPS_FAST); MyPrint("atanf", 1, 24, LOOPS_FAST);

	// arc tangent in degrees
	Test_atanf_deg(LOOPS_FAST); MyPrint("atanf_deg", 1.2, 24, LOOPS_FAST);

	// arc cotangent in radians
	Test_acotanf(LOOPS_FAST); MyPrint("acotanf", 2, 24, LOOPS_FAST);

	// arc cotangent in degrees
	Test_acotanf_deg(LOOPS_FAST); MyPrint("acotanf_deg", 2, 24, LOOPS_FAST);

	// arc tangent of y/x in radians
	Test_atan2f(LOOPS_FAST); MyPrint("atan2f", 10, 24, LOOPS_FAST);

	// arc tangent of y/x in degrees
	Test_atan2f_deg(LOOPS_FAST); MyPrint("atan2f_deg", 12, 24, LOOPS_FAST);

	// hyperbolic sine
	Test_sinhf(LOOPS_FAST); MyPrint("sinhf", 2, 12, LOOPS_FAST);

	// hyperbolic cosine
	Test_coshf(LOOPS_FAST); MyPrint("coshf", 0.4, 5, LOOPS_FAST);

	// hyperbolic tangent
	Test_tanhf(LOOPS_FAST); MyPrint("tanhf", 2.0, 12, LOOPS_FAST);

	// inverse hyperbolic sine
	Test_asinhf(LOOPS_FAST); MyPrint("asinhf", 8.0, 24, LOOPS_FAST);

	// inverse hyperbolic cosine
	Test_acoshf(LOOPS_FAST); MyPrint("acoshf", 0.2, 24, LOOPS_FAST);

	// inverse hyperbolic tangent
	Test_atanhf(LOOPS_FAST); MyPrint("atanhf", 1.5, 24, LOOPS_FAST);

	// Natural exponent
	Test_expf(LOOPS_FAST); MyPrint("expf", 0.6, 3, LOOPS_FAST);
	Test_conFF_expf(); DrawPrint("expf consistency: %s (%d)\n", (DifMax <= 2) ? "OK" : "ERROR", DifMax);

	// Natural logarithm
	Test_logf(LOOPS_FAST); MyPrint("logf", 0.2, 24, LOOPS_FAST);
	Test_conFF_logf(); DrawPrint("logf consistency: %s (%d)\n", (DifMax <= 10) ? "OK" : "ERROR", DifMax);

	// exponent with base 2
	Test_exp2f(LOOPS_FAST); MyPrint("exp2f", 1.0, 10, LOOPS_FAST);
	Test_conFF_exp2f(); DrawPrint("exp2f consistency: %s (%d)\n", (DifMax <= 8) ? "OK" : "ERROR", DifMax);

	// logarithm with base 2
	Test_log2f(LOOPS_FAST); MyPrint("log2f", 0.3, 24, LOOPS_FAST);
	Test_conFF_log2f(); DrawPrint("log2f consistency: %s (%d)\n", (DifMax <= 10) ? "OK" : "ERROR", DifMax);

	// exponent with base 10
	Test_exp10f(LOOPS_FAST); MyPrint("exp10f", 1.0, 10, LOOPS_FAST);
	Test_conFF_exp10f(); DrawPrint("exp10f consistency: %s (%d)\n", (DifMax <= 10) ? "OK" : "ERROR", DifMax);

	// logarithm with base 10
	Test_log10f(LOOPS_FAST); MyPrint("log10f", 0.6, 24, LOOPS_FAST);
	Test_conFF_log10f(); DrawPrint("log10f consistency: %s (%d)\n", (DifMax <= 12) ? "OK" : "ERROR", DifMax);

	// expf(x) - 1
	Test_expm1f(LOOPS_FAST); MyPrint("expm1f", 5.0, 20, LOOPS_FAST);

	// logf(x + 1)
	Test_log1pf(LOOPS_FAST); MyPrint("log1pf", 4, 24, LOOPS_FAST);

	// x*y + z
	Test_fmaf(LOOPS); MyPrint("fmaf", 0.05, 24, LOOPS);

	// power by integer, x^y
	Test_powintf(LOOPS_FAST); MyPrint("powintf", 0.1, 12, LOOPS_FAST);

	// power x^y
	Test_powf(LOOPS_FAST); MyPrint("powf", 0.3, 24, LOOPS_FAST);

	// square root of sum of squares (hypotenuse), sqrt(x*x + y*y)
	Test_hypotf(LOOPS_FAST); MyPrint("hypotf", 0.1, 2, LOOPS_FAST);

	// cube root, sqrt3(x), x^(1/3)
	Test_cbrtf(LOOPS_FAST); MyPrint("cbrtf", 0.5, 3, LOOPS_FAST);

	// absolute value
	Test_absf(LOOPS_SLOW); MyPrint("absf", 0, 0, LOOPS_SLOW);

	// Convert float to double
	Test_float2double(LOOPS_SLOW); MyPrint("float2double", 0.01, 1, LOOPS_SLOW);

// double precision

	DrawPrint("--- Checking double functions:\n");

	// initialize random generator
	MyRandSet(123456789123456789ULL);

	// compose floating point with magnitude of 'num' and sign of 'sign'
	Test_copysign(LOOPS_SLOW); MyPrint("copysign", 0, 0, LOOPS_SLOW);

	// check if number is an integer
	Test_isintd(LOOPS_SLOW); MyPrint("isintd", 0, 0, LOOPS_SLOW);

	// check if number is odd integer (must be an integer ..., -3, -1, 1, 3, 5,..)
	Test_isoddintd(LOOPS_SLOW); MyPrint("isoddintd", 0, 0, LOOPS_SLOW);

	// check if number is power of 2
	Test_ispow2d(LOOPS_SLOW); MyPrint("ispow2d", 0, 0, LOOPS_SLOW);

	// multiply number by power of 2 (num * 2^exp)
	Test_ldexp(LOOPS_SLOW); MyPrint("ldexp", 0, 0, LOOPS_SLOW);

	// Addition, x + y
	Test_dadd(LOOPS); MyPrint("dadd", 0.01, 1, LOOPS);

	// Subtraction, x - y
	Test_dsub(LOOPS); MyPrint("dsub", 0.01, 1, LOOPS);

	// Multiplication, x * y
	Test_dmul(LOOPS); MyPrint("dmul", 0.01, 1, LOOPS);

	// Square, x^2 
	Test_dsqr(LOOPS); MyPrint("dsqr", 0.01, 1, LOOPS);

	// Division, x / y
	Test_ddiv(LOOPS_FAST); MyPrint("ddiv", 0.02, 1, LOOPS_FAST);

	// Reciprocal 1 / x
	Test_drec(LOOPS_FAST); MyPrint("drec", 0.02, 1, LOOPS_FAST);

	// get remainder of division x/y, rounded towards zero
	Test_fmod(LOOPS_FAST); MyPrint("fmod", 0.01, 1, LOOPS_FAST);

	// compute remainder and quotient of division x/y, rounded towards the even number
	Test_remquo(LOOPS_FAST); MyPrint("remquo", 0.01, 1, LOOPS_FAST);

	// get remainder of division x/y, rounded towards the even number
	Test_remainder(LOOPS_FAST); MyPrint("remainder", 0.01, 1, LOOPS_FAST);

	// Compare, x ? y
	Test_dcmp(LOOPS_SLOW); MyPrint("dcmp", 0, 0, LOOPS_SLOW);

	// Compare if x==y
	Test_dcmpeq(LOOPS_SLOW); MyPrint("dcmpeq", 0, 0, LOOPS_SLOW);

	// Compare if x<y
	Test_dcmplt(LOOPS_SLOW); MyPrint("dcmplt", 0, 0, LOOPS_SLOW);

	// Compare if x<=y
	Test_dcmple(LOOPS_SLOW); MyPrint("dcmple", 0, 0, LOOPS_SLOW);

	// Compare if x>=y
	Test_dcmpge(LOOPS_SLOW); MyPrint("dcmpge", 0, 0, LOOPS_SLOW);

	// Compare if x>y
	Test_dcmpgt(LOOPS_SLOW); MyPrint("dcmpgt", 0, 0, LOOPS_SLOW);

	// Check if comparison is unordered (both inputs are NaN)
	Test_dcmpun(LOOPS_SLOW); MyPrint("dcmpun", 1.0, 1, LOOPS_SLOW);

	// Convert signed int to float
	Test_int2double(LOOPS_SLOW); MyPrint("int2double", 0.01, 1, LOOPS_SLOW);

	// Convert unsigned int to float
	Test_uint2double(LOOPS_SLOW); MyPrint("uint2double", 0.01, 1, LOOPS_SLOW);

	// Convert 64-bit signed int to float
	Test_int642double(LOOPS_SLOW); MyPrint("int642double", 0.01, 1, LOOPS_SLOW);

	// Convert 64-bit unsigned int to float
	Test_uint642double(LOOPS_SLOW); MyPrint("uint642double", 0.01, 1, LOOPS_SLOW);

	// Convert signed fixed point to float
	Test_fix2double(LOOPS_SLOW); MyPrint("fix2double", 0.01, 1, LOOPS_SLOW);

	// Convert unsigned fixed point to float
	Test_ufix2double(LOOPS_SLOW); MyPrint("ufix2double", 0.01, 1, LOOPS_SLOW);

	// Convert 64-bit signed fixed point to float
	Test_fix642double(LOOPS_SLOW); MyPrint("fix642double", 0.01, 1, LOOPS_SLOW);

	// Convert 64-bit unsigned fixed point to float
	Test_ufix642double(LOOPS_SLOW); MyPrint("ufix642double", 0.01, 1, LOOPS_SLOW);

	// Convert float to signed int, rounding to zero (C-style int conversion)
	Test_double2int_z(LOOPS_SLOW); MyPrint("double2int_z", 0.01, 1, LOOPS_SLOW);

	// Convert float to signed int, rounding down
	Test_double2int(LOOPS_SLOW); MyPrint("double2int", 0.01, 1, LOOPS_SLOW);

	// Convert float to unsigned int, rounding down
	Test_double2uint(LOOPS_SLOW); MyPrint("double2uint", 0.5, 1, LOOPS_SLOW);

	// Convert float to 64-bit signed int, rounding to zero (C-style int conversion)
	Test_double2int64_z(LOOPS_SLOW); MyPrint("double2int64_z", 0.01, 1, LOOPS_SLOW);

	// Convert float to 64-bit signed int, rounding down
	Test_double2int64(LOOPS_SLOW); MyPrint("double2int64", 0.01, 1, LOOPS_SLOW);

	// Convert float to 64-bit unsigned int, rounding down
	Test_double2uint64(LOOPS_SLOW); MyPrint("double2uint64", 0.01, 1, LOOPS_SLOW);

	// Convert float to signed fixed point, rounding down
	Test_double2fix(LOOPS_SLOW); MyPrint("double2fix", 0.01, 1, LOOPS_SLOW);

	// Convert float to unsigned fixed point, rounding down
	Test_double2ufix(LOOPS_SLOW); MyPrint("double2ufix", 0.6, 1, LOOPS_SLOW);

	// Convert float to 64-bit signed fixed point, rounding down
	Test_double2fix64(LOOPS_SLOW); MyPrint("double2fix64", 0.01, 1, LOOPS_SLOW);

	// Convert float to 64-bit unsigned fixed point, rounding down
	Test_double2ufix64(LOOPS_SLOW); MyPrint("double2ufix64", 0.01, 1, LOOPS_SLOW);

  	// round number towards zero
	Test_trunc(LOOPS); MyPrint("trunc", 0.01, 1, LOOPS);

	// round number to nearest integer
	Test_round(LOOPS); MyPrint("round", 0.01, 1, LOOPS);

	// round to given number of significant digits
	// @TODO: probably will be deleted (accuracy cannot be guaranteed)
//	Test_rounddig(LOOPS_FAST); MyPrint("rounddig", 8, 53, LOOPS_FAST);

	// round number down to integer
	Test_floor(LOOPS); MyPrint("floor", 0.01, 1, LOOPS);

	// round number up to integer
	Test_ceil(LOOPS); MyPrint("ceil", 0.01, 1, LOOPS);

	// Square root
	Test_sqrt(LOOPS_FAST); MyPrint("sqrt", 0.1, 1, LOOPS_FAST);

	// convert degrees to radians
	Test_deg2rad(LOOPS); MyPrint("deg2rad", 0.10, 1, LOOPS);

	// convert radians to degrees
	Test_rad2deg(LOOPS); MyPrint("rad2deg", 0.21, 1, LOOPS);

	// sine in radians
	Test_sin(LOOPS_FAST); MyPrint("sin", 0.6, 53, LOOPS_FAST);

	// sine in degrees
	Test_sin_deg(LOOPS_FAST); MyPrint("sin_deg", 0.8, 53, LOOPS_FAST);

	// cosine in radians
	Test_cos(LOOPS_FAST); MyPrint("cos", 0.3, 53, LOOPS_FAST);

	// cosine in degrees
	Test_cos_deg(LOOPS_FAST); MyPrint("cos_deg", 0.6, 53, LOOPS_FAST);

	// sine-cosine in radians
	Test_sincos(LOOPS_FAST); MyPrint("sincos", 0.4, 53, LOOPS_FAST);

	// sine-cosine in degrees
	Test_sincos_deg(LOOPS_FAST); MyPrint("sincos_deg", 0.9, 53, LOOPS_FAST);

	// tangent in radians
	Test_tan(LOOPS_FAST); MyPrint("tan", 1.0, 53, LOOPS_FAST);

	// tangent in degrees
	Test_tan_deg(LOOPS_FAST); MyPrint("tan_deg", 1.3, 53, LOOPS_FAST);

	// cotangent in radians
	Test_cotan(LOOPS_FAST); MyPrint("cotan", 1.0, 53, LOOPS_FAST);

	// cotangent in degrees
	Test_cotan_deg(LOOPS_FAST); MyPrint("cotan_deg", 1.3, 53, LOOPS_FAST);

	// arc sine in radians
	Test_asin(LOOPS_FAST); MyPrint("asin", 1.2, 10, LOOPS_FAST);

	// arc sine in degrees
	Test_asin_deg(LOOPS_FAST); MyPrint("asin_deg", 1.5, 12, LOOPS_FAST);

	// arc cosine in radians
	Test_acos(LOOPS_FAST); MyPrint("acos", 0.4, 5, LOOPS_FAST);

	// arc cosine in degrees
	Test_acos_deg(LOOPS_FAST); MyPrint("acos_deg", 0.6, 5, LOOPS_FAST);

	// arc tangent in radians
	Test_atan(LOOPS_FAST); MyPrint("atan", 1, 53, LOOPS_FAST);

	// arc tangent in degrees
	Test_atan_deg(LOOPS_FAST); MyPrint("atan_deg", 1.2, 53, LOOPS_FAST);

	// arc cotangent in radians
	// RISC-V libc: 34000ns 0.552080 11
	Test_acotan(LOOPS_FAST); MyPrint("acotan", 2, 53, LOOPS_FAST);

	// arc cotangent in degrees
	// RISC-V libc: 20100ns 0.420440 13
	Test_acotan_deg(LOOPS_FAST); MyPrint("acotan_deg", 2, 53, LOOPS_FAST);

	// arc tangent of y/x in radians
	Test_atan2(LOOPS_FAST); MyPrint("atan2", 10, 53, LOOPS_FAST);

	// arc tangent of y/x in degrees
	Test_atan2_deg(LOOPS_FAST); MyPrint("atan2_deg", 10, 53, LOOPS_FAST);

	// hyperbolic sine
	Test_sinh(LOOPS_FAST); MyPrint("sinh", 8, 53, LOOPS_FAST);

	// hyperbolic cosine
	Test_cosh(LOOPS_FAST); MyPrint("cosh", 0.2, 5, LOOPS_FAST);

	// hyperbolic tangent
	Test_tanh(LOOPS_FAST); MyPrint("tanh", 3, 53, LOOPS_FAST);

	// inverse hyperbolic sine
	Test_asinh(LOOPS_FAST); MyPrint("asinh", 3, 53, LOOPS_FAST);

	// inverse hyperbolic cosine
	Test_acosh(LOOPS_FAST); MyPrint("acosh", 0.3, 53, LOOPS_FAST);

	// inverse hyperbolic tangent
	Test_atanh(LOOPS_FAST); MyPrint("atanh", 3, 53, LOOPS_FAST);

	// Natural exponent
	Test_exp(LOOPS_FAST); MyPrint("exp", 0.2, 3, LOOPS_FAST);

	// Natural logarithm
	Test_log(LOOPS_FAST); MyPrint("log", 0.01, 53, LOOPS_FAST);

	// exponent with base 2
	Test_exp2(LOOPS_FAST); MyPrint("exp2", 0.4, 3, LOOPS_FAST);

	// logarithm with base 2
	Test_log2(LOOPS_FAST); MyPrint("log2", 0.2, 53, LOOPS_FAST);

	// exponent with base 10
	Test_exp10(LOOPS_FAST); MyPrint("exp10", 2.00, 14, LOOPS_FAST);

	// logarithm with base 10
	Test_log10(LOOPS_FAST); MyPrint("log10", 0.2, 53, LOOPS_FAST);

	// expf(x) - 1
	Test_expm1(LOOPS_FAST); MyPrint("expm1", 10, 53, LOOPS_FAST);

	// logf(x + 1)
	Test_log1p(LOOPS_FAST); MyPrint("log1p", 28, 53, LOOPS_FAST);

	// x*y + z
	Test_fma(LOOPS); MyPrint("fma", 0.01, 53, LOOPS);

	// power by integer, x^y
	Test_powint(LOOPS_FAST); MyPrint("powint", 0.1, 12, LOOPS_FAST);

	// power x^y
	Test_pow(LOOPS_FAST); MyPrint("pow", 0.5, 53, LOOPS_FAST);

	// square root of sum of squares (hypotenuse), sqrt(x*x + y*y)
	Test_hypot(LOOPS_FAST); MyPrint("hypot", 0.02, 2, LOOPS_FAST);

	// cube root, sqrt3(x), x^(1/3)
	Test_cbrt(LOOPS_FAST); MyPrint("cbrt", 0.2, 2, LOOPS_FAST);

	// absolute value
	Test_absd(LOOPS_SLOW); MyPrint("absd", 0, 0, LOOPS_SLOW);

	// Convert double to float
	Test_double2float(LOOPS_SLOW); MyPrint("double2float", 0.01, 1, LOOPS_SLOW);

	while (True)
	{
		LedFlip(LED1);
		WaitMs(500);
		u8 key = KeyGet();
		if (key == KEY_Y) ResetToBootLoader();
//		if (key == KEY_X) ScreenShot();
	}
}
