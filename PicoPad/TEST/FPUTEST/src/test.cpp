
// ****************************************************************************
//
//                                 Test
//
// ****************************************************************************

#include "../include.h"

#if (USE_FLOAT == 1) || (USE_DOUBLE == 1)
#define MYPREFIX(x) FASTCODE NOFLASH(x)
#else
#define MYPREFIX(x) NOINLINE FASTCODE x
#endif

// extract unsigned exponent from the number
int Test_GetExpF(float num) { return (int)((Test_floatu32(num) >> FLOAT_MANTBITS) & FLOAT_EXPMASK); }
int Test_GetExpD(double num) { return (int)((Test_doubleu64(num) >> DOUBLE_MANTBITS) & DOUBLE_EXPMASK); }
int Test_GetExpSignF(float num) { return (int)(Test_floatu32(num) >> FLOAT_MANTBITS); }
int Test_GetExpSignD(double num) { return (int)(Test_doubleu64(num) >> DOUBLE_MANTBITS); }

// extract sign from the number (0 or 1)
int Test_GetSignF(float num) { return (int)((Test_floatu32(num) >> (FLOAT_BITS-1)) & 1); }
int Test_GetSignD(double num) { return (int)((Test_doubleu64(num) >> (DOUBLE_BITS-1)) & 1); }

// extract mantisa from the number
u32 Test_GetMantF(float num) { return Test_floatu32(num) & FLOAT_MANTMASK; }
u64 Test_GetMantD(double num) { return Test_doubleu64(num) & DOUBLE_MANTMASK; }

// compose float number
float Test_CompF(u32 mant, int exp, int sign)
	{ return Test_u32float(mant | (exp << FLOAT_MANTBITS) | (sign << (FLOAT_BITS-1))); }
double Test_CompD(u64 mant, int exp, int sign)
	{ return Test_u64double(mant | ((u64)exp << DOUBLE_MANTBITS) | ((u64)sign << (DOUBLE_BITS-1))); }

// check - number is overflow
Bool Test_IsOverF(float num) { return Test_GetExpF(num) == FLOAT_EXPINF; }
Bool Test_IsOverD(double num) { return Test_GetExpD(num) == DOUBLE_EXPINF; }
Bool Test_IsPOverF(float num) { return Test_GetExpSignF(num) == FLOAT_EXPPINF; }
Bool Test_IsPOverD(double num) { return Test_GetExpSignD(num) == DOUBLE_EXPPINF; }
Bool Test_IsMOverF(float num) { return Test_GetExpSignF(num) == FLOAT_EXPMINF; }
Bool Test_IsMOverD(double num) { return Test_GetExpSignD(num) == DOUBLE_EXPMINF; }

// check - number is zero
Bool Test_IsZeroF(float num) { return Test_GetExpF(num) == FLOAT_EXP0; }
Bool Test_IsZeroD(double num) { return Test_GetExpD(num) == DOUBLE_EXP0; }

// check - number is negative
Bool Test_IsNegF(float num) { return Test_GetSignF(num) != 0; }
Bool Test_IsNegD(double num) { return Test_GetSignD(num) != 0; }

// check difference of numbers (returns number of different bits 0..23, 0..52)
int Text_CheckDifF(float num1, float num2)
{
	// load signs
	int sign1 = Test_GetSignF(num1);
	int sign2 = Test_GetSignF(num2);

	// load exponents
	int exp1 = Test_GetExpF(num1);
	int exp2 = Test_GetExpF(num2);

#if !CHECK_FULLRANGE		// 1=check full range (incl. inf), 0=check precision and speed
	if ((exp1 == FLOAT_EXPINF) || (exp2 == FLOAT_EXPINF)) return 0;
#else
	// Infinities with opposite signs are unequal
	if ((exp1 == FLOAT_EXPINF) && (exp2 == FLOAT_EXPINF) && (sign1 != sign2))
	{
		return FLOAT_MANTBITS;
	}

	// infinity and near-infinity are equal
	if (((exp1 == FLOAT_EXPINF) && (exp2 == FLOAT_EXPINF-1)) ||
		((exp2 == FLOAT_EXPINF) && (exp1 == FLOAT_EXPINF-1)))
	{
		return 0;
	}

	// limit overflow of num1
	if (exp1 == FLOAT_EXPINF)
	{
		exp1--;
		num1 = Test_CompF(FLOAT_MANTMASK, exp1, sign1);
	}

	// limit overflow of num2
	if (exp2 == FLOAT_EXPINF)
	{
		exp2--;
		num2 = Test_CompF(FLOAT_MANTMASK, exp2, sign2);
	}
#endif

	// zero and near-zero are equal
	if (((exp1 == FLOAT_EXP0) && (exp2 == FLOAT_EXP0+1)) ||
		((exp2 == FLOAT_EXP0) && (exp1 == FLOAT_EXP0+1)))
	{
		return 0;
	}

	// limit underflow of num1
	if (exp1 == FLOAT_EXP0)
	{
		exp1++;
		sign1 = 0;
		num1 = Test_CompF(0, exp1, sign1);
	}

	// limit underflow of num2
	if (exp2 == FLOAT_EXP0)
	{
		exp2++;
		sign2 = 0;
		num2 = Test_CompF(0, exp2, sign2);
	}

	// difference of numbers
	float dif = num1 - num2;

	// numbers are equal
	if (Test_IsZeroF(dif)) return 0;

	// prepare highest exponent
	if (exp2 > exp1) exp1 = exp2;
	exp2 = Test_GetExpF(dif);
	if (exp2 > exp1) exp1 = exp2;

	// check difference of exponents
	exp1 -= exp2;
	if (exp1 > FLOAT_MANTBITS) exp1 = FLOAT_MANTBITS;
	return FLOAT_MANTBITS+1 - exp1;
}

int Text_CheckDifD(double num1, double num2)
{
	// load signs
	int sign1 = Test_GetSignD(num1);
	int sign2 = Test_GetSignD(num2);

	// load exponents
	int exp1 = Test_GetExpD(num1);
	int exp2 = Test_GetExpD(num2);

#if !CHECK_FULLRANGE		// 1=check full range (incl. inf), 0=check precision and speed
	if ((exp1 == DOUBLE_EXPINF) || (exp2 == DOUBLE_EXPINF)) return 0;
#else
	// Infinities with opposite signs are unequal
	if ((exp1 == DOUBLE_EXPINF) && (exp2 == DOUBLE_EXPINF) && (sign1 != sign2))
	{
		return DOUBLE_MANTBITS;
	}

	// infinity and near-infinity are equal
	if (((exp1 == DOUBLE_EXPINF) && (exp2 == DOUBLE_EXPINF-1)) ||
		((exp2 == DOUBLE_EXPINF) && (exp1 == DOUBLE_EXPINF-1)))
	{
		return 0;
	}

	// limit overflow of num1
	if (exp1 == DOUBLE_EXPINF)
	{
		exp1--;
		num1 = Test_CompD(DOUBLE_MANTMASK, exp1, sign1);
	}

	// limit overflow of num2
	if (exp2 == DOUBLE_EXPINF)
	{
		exp2--;
		num2 = Test_CompD(DOUBLE_MANTMASK, exp2, sign2);
	}
#endif

	// zero and near-zero are equal
	if (((exp1 == DOUBLE_EXP0) && (exp2 == DOUBLE_EXP0+1)) ||
		((exp2 == DOUBLE_EXP0) && (exp1 == DOUBLE_EXP0+1)))
	{
		return 0;
	}

	// limit underflow of num1
	if (exp1 == DOUBLE_EXP0)
	{
		exp1++;
		sign1 = 0;
		num1 = Test_CompF(0, exp1, sign1);
	}

	// limit underflow of num2
	if (exp2 == DOUBLE_EXP0)
	{
		exp2++;
		sign2 = 0;
		num2 = Test_CompF(0, exp2, sign2);
	}

	// difference of numbers
	double dif = num1 - num2;

	// numbers are equal
	if (Test_IsZeroD(dif)) return 0;

	// prepare highest exponent
	if (exp2 > exp1) exp1 = exp2;
	exp2 = Test_GetExpD(dif);
	if (exp2 > exp1) exp1 = exp2;

	// check difference of exponents
	exp1 -= exp2;
	if (exp1 > DOUBLE_MANTBITS) exp1 = DOUBLE_MANTBITS;
	return DOUBLE_MANTBITS+1 - exp1;
}

// ============ test prototypes

#define TEST_SPEEDNUM	1000		// number of tests to check speed

int DifSum, DifMax;
int TestTime;

union {
	struct {
		volatile float TestAF_[TEST_SPEEDNUM];
		volatile float TestBF_[TEST_SPEEDNUM];
		volatile float TestCF_[TEST_SPEEDNUM];
	} data_f;

	struct {
		volatile double TestAD_[TEST_SPEEDNUM];
		volatile double TestBD_[TEST_SPEEDNUM];
		volatile double TestCD_[TEST_SPEEDNUM];
	} data_d;
} TestData;

#define TestAF TestData.data_f.TestAF_
#define TestBF TestData.data_f.TestBF_
#define TestCF TestData.data_f.TestCF_

#define TestAD TestData.data_d.TestAD_
#define TestBD TestData.data_d.TestBD_
#define TestCD TestData.data_d.TestCD_

volatile s32 TestI[TEST_SPEEDNUM];
volatile s32 TestI2[TEST_SPEEDNUM];
volatile s64 TestI64[TEST_SPEEDNUM];

// add difference with float result
NOINLINE void TestInfoAddF(float ref, float tst)
{
	int dif = Text_CheckDifF(ref, tst);
	DifSum += dif;
	if (dif > DifMax) DifMax = dif;
}

// add difference with double result
NOINLINE void TestInfoAddD(double ref, double tst)
{
	int dif = Text_CheckDifD(ref, tst);
	DifSum += dif;
	if (dif > DifMax) DifMax = dif;
}

// add difference with int result
NOINLINE void TestInfoAddI(int ref, int tst)
{
	int dif = ref - tst;
	if (dif < 0) dif = -dif;
	if (dif == 0x80000000) dif = 99;
	if (dif > 99) dif = 99;
	DifSum += dif;
	if (dif > DifMax) DifMax = dif;
}

// add difference with int64 result
NOINLINE void TestInfoAddI64(s64 ref, s64 tst)
{
	s64 dif = ref - tst;
	if (dif < 0) dif = -dif;
	if (dif > 99) dif = 99;
	DifSum += (int)dif;
	if (dif > DifMax) DifMax = (int)dif;
}

// float fnc(float, float)
int Test_FFF_Calib = 0;
float MYPREFIX(Test_FFF_Nop)(float a, float b) { return a; }
typedef float (*pTest_FFF)(float, float);
void MYPREFIX(Test_FFF)(pTest_FFF ref_fnc, pTest_FFF fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, b, tst;
	for (; loop > 0; loop--) {
		a = MyRandTestFloat(); b = MyRandTestFloat(); tst = fnc(a, b);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(b) && !Test_IsOverF(tst))
			{ TestAF[num] = a; TestBF[num] = b; num++; }
		TestInfoAddF(ref_fnc(a, b), tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FFF: %.8g %.8g %.8g %.8g\n", a, b, ref_fnc(a, b), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; TestBF[num] = TestBF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float *pb = TestBF; volatile float a2, b2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2);
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FFF_Calib; }

// double fnc(double, double)
int Test_DDD_Calib = 0;
double MYPREFIX(Test_DDD_Nop)(double a, double b) { return a; }
typedef double (*pTest_DDD)(double, double);
void MYPREFIX(Test_DDD)(pTest_DDD ref_fnc, pTest_DDD fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, b, tst;
	for (; loop > 0; loop--) {
		a = MyRandTestDouble(); b = MyRandTestDouble(); tst = fnc(a, b);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(b) && !Test_IsOverD(tst))
			{ TestAD[num] = a; TestBD[num] = b; num++; }
		TestInfoAddD(ref_fnc(a, b), tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DDD: %.8g %.8g %.8g %.8g\n", a, b, ref_fnc(a, b), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; TestBD[num] = TestBD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double *pb = TestBD; volatile double a2, b2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2);
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DDD_Calib; }

// Bool fnc(float)
int Test_BF_Calib = 0;
Bool MYPREFIX(Test_BF_Nop)(float a) { return False; }
typedef Bool (*pTest_BF)(float);
void MYPREFIX(Test_BF)(pTest_BF ref_fnc, pTest_BF fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a;
	for (; loop > 0; loop--) {
		a = MyRandTestFloat();
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a)) { TestAF[num] = a; num++; }
		TestInfoAddI(ref_fnc(a), fnc(a));
#if 0
		if (DifMax >= 23) {
			DrawPrint("BF: %.8g %d %d\n", a, ref_fnc(a), fnc(a));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2; volatile s32 res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_BF_Calib; }

// Bool fnc(double)
int Test_BD_Calib = 0;
Bool MYPREFIX(Test_BD_Nop)(double a) { return False; }
typedef Bool (*pTest_BD)(double);
void MYPREFIX(Test_BD)(pTest_BD ref_fnc, pTest_BD fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a;
	for (; loop > 0; loop--) {
		a = MyRandTestDouble();
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a)) { TestAD[num] = a; num++; }
		TestInfoAddI(ref_fnc(a), fnc(a));
#if 0
		if (DifMax >= 52) {
			DrawPrint("BD: %.8g %d %d\n", a, ref_fnc(a), fnc(a));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2; volatile s32 res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); 
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_BD_Calib; }

// float fnc(float, int shift)
int Test_FFIshift_Calib = 0;
float MYPREFIX(Test_FFIshift_Nop)(float a, int b) { return a; }
typedef float (*pTest_FFIshift)(float, int);
void MYPREFIX(Test_FFIshift)(pTest_FFIshift ref_fnc, pTest_FFIshift fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, tst; int b;
	for (; loop > 0; loop--) {
		a = MyRandTestFloat(); b = MyRandS16MinMax(-FLOAT_EXP1, FLOAT_EXP1);
		tst = fnc(a, b);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(tst))
			{ TestAF[num] = a; TestI[num] = b; num++; }
		TestInfoAddF(ref_fnc(a, b), tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FFIs: %.8g %d %.8g %.8g\n", a, b, ref_fnc(a, b), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; TestI[num] = TestI[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile s32 *pb = TestI; volatile float a2, res;
	volatile s32 b2; di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); 
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FFIshift_Calib; }

// double fnc(double, int shift)
int Test_DDIshift_Calib = 0;
double MYPREFIX(Test_DDIshift_Nop)(double a, int b) { return a; }
typedef double (*pTest_DDIshift)(double, int);
void MYPREFIX(Test_DDIshift)(pTest_DDIshift ref_fnc, pTest_DDIshift fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, tst; int b;
	for (; loop > 0; loop--) {
		a = MyRandTestDouble(); b = MyRandS16MinMax(-DOUBLE_EXP1, DOUBLE_EXP1);
		tst = fnc(a, b);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(tst))
			{ TestAD[num] = a; TestI[num] = b; num++; }
		TestInfoAddD(ref_fnc(a, b), tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DDIs: %.8g %d %.8g %.8g\n", a, b, ref_fnc(a, b), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; TestI[num] = TestI[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile s32 *pb = TestI; volatile double a2, res;
	volatile s32 b2; di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); 
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DDIshift_Calib; }

// float fnc(float)
int Test_FF_Calib = 0;
float MYPREFIX(Test_FF_Nop)(float a) { return a; }
typedef float (*pTest_FF)(float);
void MYPREFIX(Test_FF)(pTest_FF ref_fnc, pTest_FF fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, tst;
	for (; loop > 0; loop--) {
		do { a = MyRandTestFloat(); } while ((a >= 0.99f) && (a <= 1.01f));
		tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(tst))
			{ TestAF[num] = a; num++; }
		TestInfoAddF(ref_fnc(a), tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FF: %.8g %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FF_Calib; }

// double fnc(double)
int Test_DD_Calib = 0;
double MYPREFIX(Test_DD_Nop)(double a) { return a; }
typedef double (*pTest_DD)(double);
void MYPREFIX(Test_DD)(pTest_DD ref_fnc, pTest_DD fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, tst;
	for (; loop > 0; loop--) {
		a = MyRandTestDouble(); tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(tst))
			{ TestAD[num] = a; num++; }
		TestInfoAddD(ref_fnc(a), tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DD: %.8g %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DD_Calib; }

// float fnc(float) logarithm
int Test_FFlog_Calib = 0;
float MYPREFIX(Test_FFlog_Nop)(float a) { return a; }
typedef float (*pTest_FFlog)(float);
void MYPREFIX(Test_FFlog)(pTest_FFlog ref_fnc, pTest_FFlog fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, tst;
	for (; loop > 0; loop--) {
		do { a = MyRandTestFloat(); } while ((a >= 0.99f) && (a <= 1.01f));
		tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(tst))
			{ TestAF[num] = a; num++; }
		TestInfoAddF(ref_fnc(a), tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FFlog: %.8g %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FFlog_Calib; }

// double fnc(double) logarithm
int Test_DDlog_Calib = 0;
double MYPREFIX(Test_DDlog_Nop)(double a) { return a; }
typedef double (*pTest_DDlog)(double);
void MYPREFIX(Test_DDlog)(pTest_DDlog ref_fnc, pTest_DDlog fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, tst;
	for (; loop > 0; loop--) {
		do { a = MyRandTestDouble(); } while ((a >= 0.0999) && (a <= 1.0001));
		tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(tst))
			{ TestAD[num] = a; num++; }
		TestInfoAddD(ref_fnc(a), tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DDlog: %.8g %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DDlog_Calib; }

// float fnc(float, float) modulo
int Test_FFFmod_Calib = 0;
float MYPREFIX(Test_FFFmod_Nop)(float a, float b) { return a; }
typedef float (*pTest_FFFmod)(float, float);
void MYPREFIX(Test_FFFmod)(pTest_FFFmod ref_fnc, pTest_FFFmod fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, b, tst; int exp1, exp2;
	for (; loop > 0; loop--) {
		do {	a = MyRandTestFloat(); b = MyRandTestFloat();
			exp1 = Test_GetExpF(a);
			if ((exp1 == FLOAT_EXP0) || (exp1 == FLOAT_EXPINF)) break;
			exp2 = Test_GetExpF(b);
			if ((exp2 == FLOAT_EXP0) || (exp2 == FLOAT_EXPINF)) break;
		} while (exp1 - exp2 > FLOAT_MANTBITS/2); tst = fnc(a, b);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(b) && !Test_IsOverF(tst))
			{ TestAF[num] = a; TestBF[num] = b; num++; }
		TestInfoAddF(ref_fnc(a, b), tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FFFm: %.8g %.8g %.8g %.8g\n", a, b, ref_fnc(a, b), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; TestBF[num] = TestBF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float *pb = TestBF; volatile float a2, b2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2);
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FFFmod_Calib; }

// double fnc(double, double) modulo
int Test_DDDmod_Calib = 0;
double MYPREFIX(Test_DDDmod_Nop)(double a, double b) { return a; }
typedef double (*pTest_DDDmod)(double, double);
void MYPREFIX(Test_DDDmod)(pTest_DDDmod ref_fnc, pTest_DDDmod fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, b, tst; int exp1, exp2;
	for (; loop > 0; loop--) {
		do {	a = MyRandTestDouble(); b = MyRandTestDouble();
			exp1 = Test_GetExpD(a);
			if ((exp1 == DOUBLE_EXP0) || (exp1 == DOUBLE_EXPINF)) break;
			exp2 = Test_GetExpD(b);
			if ((exp2 == DOUBLE_EXP0) || (exp2 == DOUBLE_EXPINF)) break;
		} while (exp1 - exp2 > DOUBLE_MANTBITS/2); tst = fnc(a, b);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(b) && !Test_IsOverD(tst))
			{ TestAD[num] = a; TestBD[num] = b; num++; }
		TestInfoAddD(ref_fnc(a, b), tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DDDm: %.8g %.8g %.8g %.8g\n", a, b, ref_fnc(a, b), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; TestBD[num] = TestBD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double *pb = TestBD; volatile double a2, b2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2);
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DDDmod_Calib; }

// float fnc(float, float, int*) remquo
int Test_FFFPIremquo_Calib = 0;
float MYPREFIX(Test_FFFPIremquo_Nop)(float a, float b, int* quo) { *quo = 0; return a; }
typedef float (*pTest_FFFPIremquo)(float, float, int*);
void MYPREFIX(Test_FFFPIremquo)(pTest_FFFPIremquo ref_fnc, pTest_FFFPIremquo fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; int q, t; float a, b, tst; int exp1, exp2;
	for (; loop > 0; loop--) {
		do {	a = MyRandTestFloat(); b = MyRandTestFloat();
			exp1 = Test_GetExpF(a);
			if ((exp1 == FLOAT_EXP0) || (exp1 == FLOAT_EXPINF)) break;
			exp2 = Test_GetExpF(b);
			if ((exp2 == FLOAT_EXP0) || (exp2 == FLOAT_EXPINF)) break;
		} while (exp1 - exp2 > FLOAT_MANTBITS/2); tst = fnc(a, b, &t);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(b) && !Test_IsOverF(tst))
			{ TestAF[num] = a; TestBF[num] = b; num++; }
		TestInfoAddF(ref_fnc(a, b, &q), tst); TestInfoAddI(q, t);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FFFPI: %.8g %.8g %.8g %d %.8g %d\n", a, b, ref_fnc(a, b, &q), q, tst, t);
			while(True) {} }
#endif
	}
	DifSum /= 2;
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; TestBF[num] = TestBF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float *pb = TestBF; volatile float a2, b2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2, &t); res = fnc(a2, b2, &t); res = fnc(a2, b2, &t); res = fnc(a2, b2, &t);
		res = fnc(a2, b2, &t); res = fnc(a2, b2, &t); res = fnc(a2, b2, &t); res = fnc(a2, b2, &t); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FFFPIremquo_Calib; }

// double fnc(double, double, int*) remquo
int Test_DDDPIremquo_Calib = 0;
double MYPREFIX(Test_DDDPIremquo_Nop)(double a, double b, int* quo) { *quo = 0; return a; }
typedef double (*pTest_DDDPIremquo)(double, double, int*);
void MYPREFIX(Test_DDDPIremquo)(pTest_DDDPIremquo ref_fnc, pTest_DDDPIremquo fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; int q, t; double a, b, tst; int exp1, exp2;
	for (; loop > 0; loop--) {
		do {	a = MyRandTestDouble(); b = MyRandTestDouble();
			exp1 = Test_GetExpD(a);
			if ((exp1 == DOUBLE_EXP0) || (exp1 == DOUBLE_EXPINF)) break;
			exp2 = Test_GetExpD(b);
			if ((exp2 == DOUBLE_EXP0) || (exp2 == DOUBLE_EXPINF)) break;
		} while (exp1 - exp2 > DOUBLE_MANTBITS/2); tst = fnc(a, b, &t);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(b) && !Test_IsOverD(tst))
			{ TestAD[num] = a; TestBD[num] = b; num++; }
		TestInfoAddD(ref_fnc(a, b, &q), tst); TestInfoAddI(q, t);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DDDPI: %.8g %.8g %.8g %d %.8g %d\n", a, b, ref_fnc(a, b, &q), q, tst, t);
			while(True) {} }
#endif
	}
	DifSum /= 2;
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; TestBD[num] = TestBD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double *pb = TestBD; volatile double a2, b2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2, &t); res = fnc(a2, b2, &t); res = fnc(a2, b2, &t); res = fnc(a2, b2, &t);
		res = fnc(a2, b2, &t); res = fnc(a2, b2, &t); res = fnc(a2, b2, &t); res = fnc(a2, b2, &t); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DDDPIremquo_Calib; }

// s8 fnc(float, float)
int Test_BFF_Calib = 0;
s8 MYPREFIX(Test_BFF_Nop)(float a, float b) { return 0; }
typedef s8 (*pTest_BFF)(float, float);
void MYPREFIX(Test_BFF)(pTest_BFF ref_fnc, pTest_BFF fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, b;
	for (; loop > 0; loop--) {
		a = MyRandTestFloat(); b = MyRandTestFloat();
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(b))
			{ TestAF[num] = a; TestBF[num] = b; num++; }
		TestInfoAddI(ref_fnc(a, b), fnc(a, b));
#if 0
		if (DifMax >= 23) {
			DrawPrint("BFF: %.8g %.8g %d %d\n", a, b, ref_fnc(a, b), fnc(a, b));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; TestBF[num] = TestBF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float *pb = TestBF; volatile float a2, b2; volatile int res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2);
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_BFF_Calib; }

// s8 fnc(double, double)
int Test_BDD_Calib = 0;
s8 MYPREFIX(Test_BDD_Nop)(double a, double b) { return 0; }
typedef s8 (*pTest_BDD)(double, double);
void MYPREFIX(Test_BDD)(pTest_BDD ref_fnc, pTest_BDD fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, b;
	for (; loop > 0; loop--) {
		a = MyRandTestDouble(); b = MyRandTestDouble();
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(b))
			{ TestAD[num] = a; TestBD[num] = b; num++; }
		TestInfoAddI(ref_fnc(a, b), fnc(a, b));
#if 0
		if (DifMax >= 52) {
			DrawPrint("BDD: %.8g %.8g %d %d\n", a, b, ref_fnc(a, b), fnc(a, b));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; TestBD[num] = TestBD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double *pb = TestBD; volatile double a2, b2; volatile int res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2);
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_BDD_Calib; }

// float fnc(s32)
int Test_FI_Calib = 0;
float MYPREFIX(Test_FI_Nop)(s32 a) { return 0; }
typedef float (*pTest_FI)(s32);
void MYPREFIX(Test_FI)(pTest_FI ref_fnc, pTest_FI fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; s32 a; float tst;
	for (; loop > 0; loop--) {
		a = MyRandS32(); tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(tst))
			{ TestI[num] = a; num++; }
		TestInfoAddF(ref_fnc(a), tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FI: %d %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestI[num] = TestI[num0]; num++; num0++; }
	volatile s32 *pa = TestI; volatile s32 a2; volatile float res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FI_Calib; }

// double fnc(s32)
int Test_DI_Calib = 0;
double MYPREFIX(Test_DI_Nop)(s32 a) { return 0; }
typedef double (*pTest_DI)(s32);
void MYPREFIX(Test_DI)(pTest_DI ref_fnc, pTest_DI fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; s32 a; double tst;
	for (; loop > 0; loop--) {
		a = MyRandS32(); tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(tst))
			{ TestI[num] = a; num++; }
		TestInfoAddD(ref_fnc(a), tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DI: %d %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestI[num] = TestI[num0]; num++; num0++; }
	volatile s32 *pa = TestI; volatile s32 a2; volatile double res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DI_Calib; }

// float fnc(s64)
int Test_FI64_Calib = 0;
float MYPREFIX(Test_FI64_Nop)(s64 a) { return 0; }
typedef float (*pTest_FI64)(s64);
void MYPREFIX(Test_FI64)(pTest_FI64 ref_fnc, pTest_FI64 fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; s64 a; float tst;
	for (; loop > 0; loop--) {
		a = MyRandS64(); tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(tst))
			{ TestI64[num] = a; num++; }
		TestInfoAddF(ref_fnc(a), tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FI64: %lld %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestI64[num] = TestI64[num0]; num++; num0++; }
	volatile s64 *pa = TestI64; volatile s64 a2; volatile float res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FI64_Calib; }

// double fnc(s64)
int Test_DI64_Calib = 0;
double MYPREFIX(Test_DI64_Nop)(s64 a) { return 0; }
typedef double (*pTest_DI64)(s64);
void MYPREFIX(Test_DI64)(pTest_DI64 ref_fnc, pTest_DI64 fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; s64 a; double tst;
	for (; loop > 0; loop--) {
		a = MyRandS64(); tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(tst))
			{ TestI64[num] = a; num++; }
		TestInfoAddD(ref_fnc(a), tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DI64: %lld %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestI64[num] = TestI64[num0]; num++; num0++; }
	volatile s64 *pa = TestI64; volatile s64 a2; volatile double res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DI64_Calib; }

// float fnc(s32, int)
int Test_FII_Calib = 0;
float MYPREFIX(Test_FII_Nop)(s32 a, int b) { return 0; }
typedef float (*pTest_FII)(s32, int);
void MYPREFIX(Test_FII)(pTest_FII ref_fnc, pTest_FII fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; s32 a, b; float tst;
	for (; loop > 0; loop--) {
		a = MyRandS32(); b = MyRandS8(); tst = fnc(a, b);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(tst))
			{ TestI[num] = a; TestI2[num] = b; num++; }
		TestInfoAddF(ref_fnc(a, b), tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FII: %d %d %.8g %.8g\n", a, b, ref_fnc(a, b), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestI[num] = TestI[num0]; TestI2[num] = TestI2[num0]; num++; num0++; }
	volatile s32 *pa = TestI; volatile s32 *pb = TestI2; volatile s32 a2, b2; volatile float res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2);
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FII_Calib; }

// double fnc(s32, int)
int Test_DII_Calib = 0;
double MYPREFIX(Test_DII_Nop)(s32 a, int b) { return 0; }
typedef double (*pTest_DII)(s32, int);
void MYPREFIX(Test_DII)(pTest_DII ref_fnc, pTest_DII fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; s32 a, b; double tst;
	for (; loop > 0; loop--) {
		a = MyRandS32(); b = MyRandS8(); tst = fnc(a, b);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(tst))
			{ TestI[num] = a; TestI2[num] = b; num++; }
		TestInfoAddD(ref_fnc(a, b), tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DII: %d %d %.8g %.8g\n", a, b, ref_fnc(a, b), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestI[num] = TestI[num0]; TestI2[num] = TestI2[num0]; num++; num0++; }
	volatile s32 *pa = TestI; volatile s32 *pb = TestI2; volatile s32 a2, b2; volatile double res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2);
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DII_Calib; }

// float fnc(s64, int)
int Test_FI64I_Calib = 0;
float MYPREFIX(Test_FI64I_Nop)(s64 a, int b) { return 0; }
typedef float (*pTest_FI64I)(s64, int);
void MYPREFIX(Test_FI64I)(pTest_FI64I ref_fnc, pTest_FI64I fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; s64 a; int b; float tst;
	for (; loop > 0; loop--) {
		a = MyRandS64(); b = MyRandS8(); tst = fnc(a, b);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(tst))
			{ TestI64[num] = a; TestI2[num] = b; num++; }
		TestInfoAddF(ref_fnc(a, b), tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FI64I: %lld %d %.8g %.8g\n", a, b, ref_fnc(a, b), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestI64[num] = TestI64[num0]; TestI2[num] = TestI2[num0]; num++; num0++; }
	volatile s64 *pa = TestI64; volatile s32 *pb = TestI2; volatile s64 a2; volatile int b2; volatile float res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2);
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FI64I_Calib; }

// double fnc(s64, int)
int Test_DI64I_Calib = 0;
double MYPREFIX(Test_DI64I_Nop)(s64 a, int b) { return 0; }
typedef double (*pTest_DI64I)(s64, int);
void MYPREFIX(Test_DI64I)(pTest_DI64I ref_fnc, pTest_DI64I fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; s64 a; int b; double tst;
	for (; loop > 0; loop--) {
		a = MyRandS64(); b = MyRandS8(); tst = fnc(a, b);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(tst))
			{ TestI64[num] = a; TestI2[num] = b; num++; }
		TestInfoAddD(ref_fnc(a, b), tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DI64I: %lld %d %.8g %.8g\n", a, b, ref_fnc(a, b), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestI64[num] = TestI64[num0]; TestI2[num] = TestI2[num0]; num++; num0++; }
	volatile s64 *pa = TestI64; volatile s32 *pb = TestI2; volatile s64 a2; volatile int b2; volatile double res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2);
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DI64I_Calib; }

// s32 fnc(float)
int Test_IF_Calib = 0;
s32 MYPREFIX(Test_IF_Nop)(float a) { return 0; }
typedef s32 (*pTest_IF)(float);
void MYPREFIX(Test_IF)(pTest_IF ref_fnc, pTest_IF fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a;
	for (; loop > 0; loop--) {
		a = MyRandTestFloat();
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a)) { TestAF[num] = a; num++; }
		TestInfoAddI(ref_fnc(a), fnc(a));
#if 0
		if (DifMax >= 23) {
			DrawPrint("IF: %.8g %d %d\n", a, ref_fnc(a), fnc(a));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2; volatile s32 res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_IF_Calib; }

// s32 fnc(double)
int Test_ID_Calib = 0;
s32 MYPREFIX(Test_ID_Nop)(double a) { return 0; }
typedef s32 (*pTest_ID)(double);
void MYPREFIX(Test_ID)(pTest_ID ref_fnc, pTest_ID fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a;
	for (; loop > 0; loop--) {
		a = MyRandTestDouble();
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a)) { TestAD[num] = a; num++; }
		TestInfoAddI(ref_fnc(a), fnc(a));
#if 0
		if (DifMax >= 52) {
			DrawPrint("ID: %.8g %d %d\n", a, ref_fnc(a), fnc(a));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2; volatile s32 res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); 
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_ID_Calib; }

// s64 fnc(float)
int Test_I64F_Calib = 0;
s64 MYPREFIX(Test_I64F_Nop)(float a) { return 0; }
typedef s64 (*pTest_I64F)(float);
void MYPREFIX(Test_I64F)(pTest_I64F ref_fnc, pTest_I64F fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a;
	for (; loop > 0; loop--) {
		a = MyRandTestFloat();
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a)) { TestAF[num] = a; num++; }
		TestInfoAddI64(ref_fnc(a), fnc(a));
#if 0
		if (DifMax >= 23) {
			DrawPrint("I64F: %.8g %lld %lld\n", a, ref_fnc(a), fnc(a));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2; volatile s64 res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_I64F_Calib; }

// s64 fnc(double)
int Test_I64D_Calib = 0;
s64 MYPREFIX(Test_I64D_Nop)(double a) { return 0; }
typedef s64 (*pTest_I64D)(double);
void MYPREFIX(Test_I64D)(pTest_I64D ref_fnc, pTest_I64D fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a;
	for (; loop > 0; loop--) {
		a = MyRandTestDouble();
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a)) { TestAD[num] = a; num++; }
		TestInfoAddI64(ref_fnc(a), fnc(a));
#if 0
		if (DifMax >= 52) {
			DrawPrint("I64D: %.8g %lld %lld\n", a, ref_fnc(a), fnc(a));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2; volatile s64 res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); 
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_I64D_Calib; }

// s32 fnc(float, int)
int Test_IFI_Calib = 0;
s32 MYPREFIX(Test_IFI_Nop)(float a, int e) { return 0; }
typedef s32 (*pTest_IFI)(float, int);
void MYPREFIX(Test_IFI)(pTest_IFI ref_fnc, pTest_IFI fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a; int b;
	for (; loop > 0; loop--) {
		a = MyRandTestFloat(); b = MyRandS8();
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a)) { TestAF[num] = a; TestI[num] = b; num++; }
		TestInfoAddI(ref_fnc(a, b), fnc(a, b));
#if 0
		if (DifMax >= 23) {
			DrawPrint("IFI: %.8g %d, %d %d\n", a, b, ref_fnc(a, b), fnc(a, b));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; TestI[num] = TestI[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile s32 *pb = TestI; volatile float a2; volatile int b2; volatile s32 res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2);
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_IFI_Calib; }

// s32 fnc(double, int)
int Test_IDI_Calib = 0;
s32 MYPREFIX(Test_IDI_Nop)(double a, int e) { return 0; }
typedef s32 (*pTest_IDI)(double, int);
void MYPREFIX(Test_IDI)(pTest_IDI ref_fnc, pTest_IDI fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a; int b;
	for (; loop > 0; loop--) {
		a = MyRandTestDouble(); b = MyRandS8();
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a)) { TestAD[num] = a; TestI[num] = b; num++; }
		TestInfoAddI(ref_fnc(a, b), fnc(a, b));
#if 0
		if (DifMax >= 52) {
			DrawPrint("IDI: %.8g %d, %d %d\n", a, b, ref_fnc(a, b), fnc(a, b));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; TestI[num] = TestI[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile s32 *pb = TestI; volatile double a2; volatile int b2; volatile s32 res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); 
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_IDI_Calib; }

// s64 fnc(float, int)
int Test_I64FI_Calib = 0;
s64 MYPREFIX(Test_I64FI_Nop)(float a, int e) { return 0; }
typedef s64 (*pTest_I64FI)(float, int);
void MYPREFIX(Test_I64FI)(pTest_I64FI ref_fnc, pTest_I64FI fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a; int b;
	for (; loop > 0; loop--) {
		a = MyRandTestFloat(); b = MyRandS8();
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a)) { TestAF[num] = a; TestI[num] = b; num++; }
		TestInfoAddI64(ref_fnc(a, b), fnc(a, b));
#if 0
		if (DifMax >= 23) {
			DrawPrint("I64FI: %.8g %d, %lld %lld\n", a, b, ref_fnc(a, b), fnc(a, b));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; TestI[num] = TestI[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile s32 *pb = TestI; volatile float a2; volatile int b2; volatile s64 res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2);
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_I64FI_Calib; }

// s64 fnc(double, int)
int Test_I64DI_Calib = 0;
s64 MYPREFIX(Test_I64DI_Nop)(double a, int e) { return 0; }
typedef s64 (*pTest_I64DI)(double, int);
void MYPREFIX(Test_I64DI)(pTest_I64DI ref_fnc, pTest_I64DI fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a; int b;
	for (; loop > 0; loop--) {
		a = MyRandTestDouble(); b = MyRandS8();
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a)) { TestAD[num] = a; TestI[num] = b; num++; }
		TestInfoAddI64(ref_fnc(a, b), fnc(a, b));
#if 0
		if (DifMax >= 52) {
			DrawPrint("I64FI: %.8g %d, %lld %lld\n", a, b, ref_fnc(a, b), fnc(a, b));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; TestI[num] = TestI[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile s32 *pb = TestI; volatile double a2; volatile int b2; volatile s64 res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); 
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_I64DI_Calib; }

// float fnc(float) angle
int Test_FFangle_Calib = 0;
float MYPREFIX(Test_FFangle_Nop)(float a) { return a; }
typedef float (*pTest_FFangle)(float);
void MYPREFIX(Test_FFangle)(pTest_FFangle ref_fnc, pTest_FFangle fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, tst, ref, k;
	for (; loop > 0; loop--) {
		do a = MyRandTestFloatMinMax(FLOAT_EXP1 - 5, FLOAT_EXP1 + 2);
			while (Test_IsOverF(a)); // sin(inf) is undefined
		ref = ref_fnc(a); tst = fnc(a); k = (absf(ref) < 0.5f) ? ((ref < 0) ? -0.25f : 0.25f) : 0; // small angles have undefined precision
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(tst)) { TestAF[num] = a; num++; }
		TestInfoAddF(ref + k, tst + k);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FFa: %.8g %.8g %.8g\n", a, ref, tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FFangle_Calib; }

// double fnc(double) angle
int Test_DDangle_Calib = 0;
double MYPREFIX(Test_DDangle_Nop)(double a) { return a; }
typedef double (*pTest_DDangle)(double);
void MYPREFIX(Test_DDangle)(pTest_DDangle ref_fnc, pTest_DDangle fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, tst, ref, k;
	for (; loop > 0; loop--) {
		do a = MyRandTestDoubleMinMax(DOUBLE_EXP1 - 8, DOUBLE_EXP1 + 2); 
			while (Test_IsOverD(a)); // sin(inf) is undefined
		ref = ref_fnc(a); tst = fnc(a); k = (absd(ref) < 0.5) ? ((ref < 0) ? -0.25 : 0.25) : 0; // small angles have undefined precision
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(tst)) { TestAD[num] = a; num++; }
		TestInfoAddD(ref + k, tst + k);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DDa: %.8g %.8g %.8g\n", a, ref, tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DDangle_Calib; }

// void fnc(float, float*, float*) angle
int Test_VPFPFangle_Calib = 0;
void MYPREFIX(Test_VPFPFangle_Nop)(float a, float* si, float* co) { *si = 0; *co = 0; }
typedef void (*pTest_VPFPFangle)(float, float*, float*);
void MYPREFIX(Test_VPFPFangle)(pTest_VPFPFangle ref_fnc, pTest_VPFPFangle fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, si_ref, co_ref, si_tst, co_tst, si_k, co_k;
	for (; loop > 0; loop--) {
		do a = MyRandTestFloatMinMax(FLOAT_EXP1 - 5, FLOAT_EXP1 + 2);
			while (Test_IsOverF(a)); // sincos(inf) is undefined
		ref_fnc(a, &si_ref, &co_ref); fnc(a, &si_tst, &co_tst);
		si_k = (absf(si_ref) < 0.5f) ? ((si_ref < 0) ? -0.25f : 0.25f) : 0; // small angles have undefined precision
		co_k = (absf(co_ref) < 0.5f) ? ((co_ref < 0) ? -0.25f : 0.25f) : 0; // small angles have undefined precision
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(si_tst) && !Test_IsOverF(co_tst))
			{ TestAF[num] = a; num++; }
		TestInfoAddF(si_ref + si_k, si_tst + si_k); TestInfoAddF(co_ref + co_k, co_tst + co_k);
#if 0
		if (DifMax >= 23) {
			DrawPrint("VPFPFa: %.8g %.8g %.8g %.8g %.8g\n", a, si_ref, si_tst, co_ref, co_tst);
			while(True) {} }
#endif
	}
	DifSum /= 2;
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2; float sr, cr;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr);
		fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_VPFPFangle_Calib; }

// void fnc(double, double*, double*) angle
int Test_VPDPDangle_Calib = 0;
void MYPREFIX(Test_VPDPDangle_Nop)(double a, double* si, double* co) { *si = 0; *co = 0; }
typedef void (*pTest_VPDPDangle)(double, double*, double*);
void MYPREFIX(Test_VPDPDangle)(pTest_VPDPDangle ref_fnc, pTest_VPDPDangle fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, si_ref, co_ref, si_tst, co_tst, si_k, co_k;
	for (; loop > 0; loop--) {
		do a = MyRandTestDoubleMinMax(DOUBLE_EXP1 - 8, DOUBLE_EXP1 + 2);
			while (Test_IsOverD(a)); // sincos(inf) is undefined
		ref_fnc(a, &si_ref, &co_ref); fnc(a, &si_tst, &co_tst);
		si_k = (absd(si_ref) < 0.5) ? ((si_ref < 0) ? -0.25 : 0.25) : 0; // small angles have undefined precision
		co_k = (absd(co_ref) < 0.5) ? ((co_ref < 0) ? -0.25 : 0.25) : 0; // small angles have undefined precision
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(si_tst) && !Test_IsOverD(co_tst))
			{ TestAD[num] = a; num++; }
		TestInfoAddD(si_ref + si_k, si_tst + si_k); TestInfoAddD(co_ref + co_k, co_tst + co_k);
#if 0
		if (DifMax >= 52) {
			DrawPrint("VPDPDa: %.8g %.8g %.8g %.8g %.8g\n", a, si_ref, si_tst, co_ref, co_tst);
			while(True) {} }
#endif
	}
	DifSum /= 2;
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2; double sr, cr;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr);
		fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_VPDPDangle_Calib; }

// float fnc(float) arc-angle
int Test_FFarcangle_Calib = 0;
float MYPREFIX(Test_FFarcangle_Nop)(float a) { return a; }
typedef float (*pTest_FFarcangle)(float);
void MYPREFIX(Test_FFarcangle)(pTest_FFarcangle ref_fnc, pTest_FFarcangle fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, tst, ref;
	for (; loop > 0; loop--) {
#if CHECK_FULLRANGE		// 1=check full range (incl. inf), 0=check precision and speed
		a = MyRandTestFloatMinMax(FLOAT_EXP1 - 25, FLOAT_EXP1);
#else
		a = MyRandTestFloatMinMax(FLOAT_EXP1 - 7, FLOAT_EXP1-1);
#endif
		ref = ref_fnc(a); tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(tst))
			{ TestAF[num] = a; num++; }
		TestInfoAddF(ref, tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FFarc: %.8g %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FFarcangle_Calib; }

// double fnc(double) arc-angle
int Test_DDarcangle_Calib = 0;
double MYPREFIX(Test_DDarcangle_Nop)(double a) { return a; }
typedef double (*pTest_DDarcangle)(double);
void MYPREFIX(Test_DDarcangle)(pTest_DDarcangle ref_fnc, pTest_DDarcangle fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, tst, ref;
	for (; loop > 0; loop--) {
#if CHECK_FULLRANGE		// 1=check full range (incl. inf), 0=check precision and speed
		a = MyRandTestDoubleMinMax(DOUBLE_EXP1 - 54, DOUBLE_EXP1);
#else
		a = MyRandTestDoubleMinMax(DOUBLE_EXP1 - 10, DOUBLE_EXP1-1);
#endif
		ref = ref_fnc(a); tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(tst))
			{ TestAD[num] = a; num++; }
		TestInfoAddD(ref, tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DDarc: %.8g %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DDarcangle_Calib; }

// float fnc(float) arc-tan
int Test_FFarctan_Calib = 0;
float MYPREFIX(Test_FFarctan_Nop)(float a) { return a; }
typedef float (*pTest_FFarctan)(float);
void MYPREFIX(Test_FFarctan)(pTest_FFarctan ref_fnc, pTest_FFarctan fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, tst, ref;
	for (; loop > 0; loop--) {
#if CHECK_FULLRANGE		// 1=check full range (incl. inf), 0=check precision and speed
		a = MyRandTestFloat();
#else
		a = MyRandTestFloatMinMax(FLOAT_EXP1 - 7, FLOAT_EXP1 + 7);
#endif
		ref = ref_fnc(a); tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(tst))
			{ TestAF[num] = a; num++; }
		TestInfoAddF(ref, tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FFarc: %.8g %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FFarctan_Calib; }

// double fnc(double) arc-tan
int Test_DDarctan_Calib = 0;
double MYPREFIX(Test_DDarctan_Nop)(double a) { return a; }
typedef double (*pTest_DDarctan)(double);
void MYPREFIX(Test_DDarctan)(pTest_DDarctan ref_fnc, pTest_DDarctan fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, tst, ref;
	for (; loop > 0; loop--) {
#if CHECK_FULLRANGE		// 1=check full range (incl. inf), 0=check precision and speed
		a = MyRandTestDouble();
#else
		a = MyRandTestDoubleMinMax(DOUBLE_EXP1 - 10, DOUBLE_EXP1 + 10);
#endif
		ref = ref_fnc(a); tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(tst))
			{ TestAD[num] = a; num++; }
		TestInfoAddD(ref, tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DDarc: %.8g %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DDarctan_Calib; }

// float fnc(float) angledeg
int Test_FFangledeg_Calib = 0;
float MYPREFIX(Test_FFangledeg_Nop)(float a) { return a; }
typedef float (*pTest_FFangledeg)(float);
void MYPREFIX(Test_FFangledeg)(pTest_FFangledeg ref_fnc, pTest_FFangledeg fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, tst, ref, k;
	for (; loop > 0; loop--) {
		do a = MyRandTestFloatMinMax(FLOAT_EXP1 - 5+6, FLOAT_EXP1 + 2+6);
			while (Test_IsOverF(a)); // sin(inf) is undefined
		ref = ref_fnc(a); tst = fnc(a); k = (absf(ref) < 0.5f) ? ((ref < 0) ? -0.25f : 0.25f) : 0; // small angles have undefined precision
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(tst)) { TestAF[num] = a; num++; }
		TestInfoAddF(ref + k, tst + k);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FFad: %.8g %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FFangledeg_Calib; }

// double fnc(double) angledeg
int Test_DDangledeg_Calib = 0;
double MYPREFIX(Test_DDangledeg_Nop)(double a) { return a; }
typedef double (*pTest_DDangledeg)(double);
void MYPREFIX(Test_DDangledeg)(pTest_DDangledeg ref_fnc, pTest_DDangledeg fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, tst, ref, k;
	for (; loop > 0; loop--) {
		do a = MyRandTestDoubleMinMax(DOUBLE_EXP1 - 8+6, DOUBLE_EXP1 + 2+6);
			while (Test_IsOverD(a)); // sin(inf) is undefined
		ref = ref_fnc(a); tst = fnc(a); k = (absd(ref) < 0.5) ? ((ref < 0) ? -0.25 : 0.25) : 0; // small angles have undefined precision
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(tst))
			{ TestAD[num] = a; num++; }
		TestInfoAddD(ref + k, tst + k);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DDad: %.8g %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DDangledeg_Calib; }

// void fnc(float, float*, float*) angledeg
int Test_VPFPFangledeg_Calib = 0;
void MYPREFIX(Test_VPFPFangledeg_Nop)(float a, float* si, float* co) { *si = 0; *co = 0; }
typedef void (*pTest_VPFPFangledeg)(float, float*, float*);
void MYPREFIX(Test_VPFPFangledeg)(pTest_VPFPFangledeg ref_fnc, pTest_VPFPFangledeg fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, si_ref, co_ref, si_tst, co_tst, si_k, co_k;
	for (; loop > 0; loop--) {
		do a = MyRandTestFloatMinMax(FLOAT_EXP1 - 5+6, FLOAT_EXP1 + 2+6);
			while (Test_IsOverF(a)); // sincos(inf) is undefined
		ref_fnc(a, &si_ref, &co_ref); fnc(a, &si_tst, &co_tst);
		si_k = (absf(si_ref) < 0.5f) ? ((si_ref < 0) ? -0.25f : 0.25f) : 0; // small angles have undefined precision
		co_k = (absf(co_ref) < 0.5f) ? ((co_ref < 0) ? -0.25f : 0.25f) : 0; // small angles have undefined precision
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(si_tst) && !Test_IsOverF(co_tst))
			{ TestAF[num] = a; num++; }
		TestInfoAddF(si_ref + si_k, si_tst + si_k); TestInfoAddF(co_ref + co_k, co_tst + co_k);
#if 0
		if (DifMax >= 23) {
			DrawPrint("VPFPFad: %.8g %.8g %.8g %.8g %.8g\n", a, si_ref, si_tst, co_ref, co_tst);
			while(True) {} }
#endif
	}
	DifSum /= 2;
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2; float sr, cr;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr);
		fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_VPFPFangledeg_Calib; }

// void fnc(double, double*, double*) angledeg
int Test_VPDPDangledeg_Calib = 0;
void MYPREFIX(Test_VPDPDangledeg_Nop)(double a, double* si, double* co) { *si = 0; *co = 0; }
typedef void (*pTest_VPDPDangledeg)(double, double*, double*);
void MYPREFIX(Test_VPDPDangledeg)(pTest_VPDPDangledeg ref_fnc, pTest_VPDPDangledeg fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, si_ref, co_ref, si_tst, co_tst, si_k, co_k;
	for (; loop > 0; loop--) {
		do a = MyRandTestDoubleMinMax(DOUBLE_EXP1 - 8+6, DOUBLE_EXP1 + 2+6);
			while (Test_IsOverD(a)); // sincos(inf) is undefined
		ref_fnc(a, &si_ref, &co_ref); fnc(a, &si_tst, &co_tst);
		si_k = (absd(si_ref) < 0.5) ? ((si_ref < 0) ? -0.25 : 0.25) : 0; // small angles have undefined precision
		co_k = (absd(co_ref) < 0.5) ? ((co_ref < 0) ? -0.25 : 0.25) : 0; // small angles have undefined precision
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(si_tst) && !Test_IsOverD(co_tst))
			{ TestAD[num] = a; num++; }
		TestInfoAddD(si_ref + si_k, si_tst + si_k); TestInfoAddD(co_ref + co_k, co_tst + co_k);
#if 0
		if (DifMax >= 52) {
			DrawPrint("VPDPDad: %.8g %.8g %.8g %.8g %.8g\n", a, si_ref, si_tst, co_ref, co_tst);
			while(True) {} }
#endif
	}
	DifSum /= 2;
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2; double sr, cr;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr);
		fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); fnc(a2, &sr, &cr); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_VPDPDangledeg_Calib; }

// float fnc(float) exp
int Test_FFexp_Calib = 0;
float MYPREFIX(Test_FFexp_Nop)(float a) { return a; }
typedef float (*pTest_FFexp)(float);
void MYPREFIX(Test_FFexp)(pTest_FFexp ref_fnc, pTest_FFexp fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, tst;
	for (; loop > 0; loop--) {
		a = MyRandTestFloatMinMax(FLOAT_EXP1 - 26, FLOAT_EXP1 + 7); tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(tst) && !Test_IsZeroF(tst) && (Test_GetExpF(tst) != FLOAT_EXP1))
			{ TestAF[num] = a; num++; }
		TestInfoAddF(ref_fnc(a), tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FFexp: %.8g %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FFexp_Calib; }

// double fnc(double) exp
int Test_DDexp_Calib = 0;
double MYPREFIX(Test_DDexp_Nop)(double a) { return a; }
typedef double (*pTest_DDexp)(double);
void MYPREFIX(Test_DDexp)(pTest_DDexp ref_fnc, pTest_DDexp fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, tst;
	for (; loop > 0; loop--) {
		a = MyRandTestDoubleMinMax(DOUBLE_EXP1 - 54, DOUBLE_EXP1 + 10); tst = fnc(a);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(tst) && !Test_IsZeroD(tst) && (Test_GetExpD(tst) != DOUBLE_EXP1))
			{ TestAD[num] = a; num++; }
		TestInfoAddD(ref_fnc(a), tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DDexp: %.8g %.8g %.8g\n", a, ref_fnc(a), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DDexp_Calib; }

// float fnc(float, float, float)
int Test_FFFF_Calib = 0;
float MYPREFIX(Test_FFFF_Nop)(float a, float b, float c) { return a; }
typedef float (*pTest_FFFF)(float, float, float);
void MYPREFIX(Test_FFFF)(pTest_FFFF ref_fnc, pTest_FFFF fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, b, c, tst;
	for (; loop > 0; loop--) {
		a = MyRandTestFloat(); b = MyRandTestFloat(); c = MyRandTestFloat(); tst = fnc(a, b, c);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(b) && !Test_IsOverF(c) && !Test_IsOverF(tst))
			{ TestAF[num] = a; TestBF[num] = b; TestCF[num] = c; num++; }
		TestInfoAddF(ref_fnc(a, b, c), tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FFFF: %.8g %.8g %.8g %.8g %.8g\n", a, b, c, ref_fnc(a, b, c), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; TestBF[num] = TestBF[num0]; TestCF[num] = TestCF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float *pb = TestBF; volatile float *pc = TestCF; volatile float a2, b2, c2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++; c2 = *pc++;
		res = fnc(a2, b2, c2); res = fnc(a2, b2, c2); res = fnc(a2, b2, c2); res = fnc(a2, b2, c2);
		res = fnc(a2, b2, c2); res = fnc(a2, b2, c2); res = fnc(a2, b2, c2); res = fnc(a2, b2, c2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FFFF_Calib; }

// double fnc(double, double)
int Test_DDDD_Calib = 0;
double MYPREFIX(Test_DDDD_Nop)(double a, double b, double c) { return a; }
typedef double (*pTest_DDDD)(double, double, double);
void MYPREFIX(Test_DDDD)(pTest_DDDD ref_fnc, pTest_DDDD fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, b, c, tst;
	for (; loop > 0; loop--) {
		a = MyRandTestDouble(); b = MyRandTestDouble(); c = MyRandTestDouble(); tst = fnc(a, b, c);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(b) && !Test_IsOverD(c) && !Test_IsOverD(tst))
			{ TestAD[num] = a; TestBD[num] = b; TestCD[num] = c; num++; }
		TestInfoAddD(ref_fnc(a, b, c), tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DDDD: %.8g %.8g %.8g %.8g %.8g\n", a, b, c, ref_fnc(a, b, c), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; TestBD[num] = TestBD[num0]; TestCD[num] = TestCD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double *pb = TestBD; volatile double *pc = TestCD; volatile double a2, b2, c2, res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++; c2 = *pc++;
		res = fnc(a2, b2, c2); res = fnc(a2, b2, c2); res = fnc(a2, b2, c2); res = fnc(a2, b2, c2);
		res = fnc(a2, b2, c2); res = fnc(a2, b2, c2); res = fnc(a2, b2, c2); res = fnc(a2, b2, c2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DDDD_Calib; }

// double fnc(float)
int Test_DF_Calib = 0;
double MYPREFIX(Test_DF_Nop)(float a) { return 0.0; }
typedef double (*pTest_DF)(float);
void MYPREFIX(Test_DF)(pTest_DF ref_fnc, pTest_DF fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a;
	for (; loop > 0; loop--) {
		a = MyRandTestFloat();
		if (num < TEST_SPEEDNUM) { TestAF[num] = a; num++; }
		TestInfoAddD(ref_fnc(a), fnc(a));
#if 0
		if (DifMax >= 52) {
			DrawPrint("DF: %.8g %.8g %.8g\n", a, ref_fnc(a), fnc(a));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile float a2; volatile double res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DF_Calib; }

// float fnc(doble)
int Test_FD_Calib = 0;
float MYPREFIX(Test_FD_Nop)(double a) { return 0.0f; }
typedef float (*pTest_FD)(double);
void MYPREFIX(Test_FD)(pTest_FD ref_fnc, pTest_FD fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a;
	for (; loop > 0; loop--) {
		a = MyRandTestDouble();
		if (num < TEST_SPEEDNUM) { TestAD[num] = a; num++; }
		TestInfoAddF(ref_fnc(a), fnc(a));
#if 0
		if (DifMax >= 23) {
			DrawPrint("FD: %.8g %.8g %.8g\n", a, ref_fnc(a), fnc(a));
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile double a2; volatile float res;
	di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++;
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2);
		res = fnc(a2); res = fnc(a2); res = fnc(a2); res = fnc(a2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FD_Calib; }

// float fnc(float, int digits) digits
int Test_FFIdig_Calib = 0;
float MYPREFIX(Test_FFIdig_Nop)(float a, int b) { return a; }
typedef float (*pTest_FFIdig)(float, int);
void MYPREFIX(Test_FFIdig)(pTest_FFIdig ref_fnc, pTest_FFIdig fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; float a, tst; int b;
	for (; loop > 0; loop--) {
		a = MyRandTestFloat(); b = MyRandS16MinMax(-1, 10);
		tst = fnc(a, b);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverF(a) && !Test_IsOverF(tst))
			{ TestAF[num] = a; TestI[num] = b; num++; }
		TestInfoAddF(ref_fnc(a, b), tst);
#if 0
		if (DifMax >= 23) {
			DrawPrint("FFId: %.8g %d %.8g %.8g\n", a, b, ref_fnc(a, b), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAF[num] = TestAF[num0]; TestI[num] = TestI[num0]; num++; num0++; }
	volatile float *pa = TestAF; volatile s32 *pb = TestI; volatile float a2, res;
	volatile s32 b2; di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); 
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_FFIdig_Calib; }

// double fnc(double, int shift)
int Test_DDIdig_Calib = 0;
double MYPREFIX(Test_DDIdig_Nop)(double a, int b) { return a; }
typedef double (*pTest_DDIdig)(double, int);
void MYPREFIX(Test_DDIdig)(pTest_DDIdig ref_fnc, pTest_DDIdig fnc, int loop) {
	DifSum = 0; DifMax = 0; int num = 0; int num0 = 0; double a, tst; int b;
	for (; loop > 0; loop--) {
		a = MyRandTestDouble(); b = MyRandS16MinMax(-1, 18);
		tst = fnc(a, b);
		if ((num < TEST_SPEEDNUM) && !Test_IsOverD(a) && !Test_IsOverD(tst))
			{ TestAD[num] = a; TestI[num] = b; num++; }
		TestInfoAddD(ref_fnc(a, b), tst);
#if 0
		if (DifMax >= 52) {
			DrawPrint("DDId: %.8g %d %.8g %.8g\n", a, b, ref_fnc(a, b), tst);
			while(True) {} }
#endif
	}
	while (num < TEST_SPEEDNUM) { TestAD[num] = TestAD[num0]; TestI[num] = TestI[num0]; num++; num0++; }
	volatile double *pa = TestAD; volatile s32 *pb = TestI; volatile double a2, res;
	volatile s32 b2; di(); dmb(); u32 t1 = Time();
	for (; num > 0; num--) { a2 = *pa++; b2 = *pb++;
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); 
		res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); res = fnc(a2, b2); }
	u32 t2 = Time(); dmb(); ei(); TestTime = (t2 - t1 + 4)/8 - Test_DDIdig_Calib; }

// calibrate base time
void Test_Calib()
{
	Test_FFF(Test_FFF_Nop, Test_FFF_Nop, 2000);
	Test_FFF_Calib = TestTime;

	Test_DDD(Test_DDD_Nop, Test_DDD_Nop, 2000);
	Test_DDD_Calib = TestTime;

	Test_BF(Test_BF_Nop, Test_BF_Nop, 2000);
	Test_BF_Calib = TestTime;

	Test_BD(Test_BD_Nop, Test_BD_Nop, 2000);
	Test_BD_Calib = TestTime;

	Test_FFIshift(Test_FFIshift_Nop, Test_FFIshift_Nop, 2000);
	Test_FFIshift_Calib = TestTime;

	Test_DDIshift(Test_DDIshift_Nop, Test_DDIshift_Nop, 2000);
	Test_DDIshift_Calib = TestTime;

	Test_FF(Test_FF_Nop, Test_FF_Nop, 2000);
	Test_FF_Calib = TestTime;

	Test_DD(Test_DD_Nop, Test_DD_Nop, 2000);
	Test_DD_Calib = TestTime;

	Test_FFlog(Test_FFlog_Nop, Test_FFlog_Nop, 2000);
	Test_FFlog_Calib = TestTime;

	Test_DDlog(Test_DDlog_Nop, Test_DDlog_Nop, 2000);
	Test_DDlog_Calib = TestTime;

	Test_FFFmod(Test_FFFmod_Nop, Test_FFFmod_Nop, 2000);
	Test_FFFmod_Calib = TestTime;

	Test_DDDmod(Test_DDDmod_Nop, Test_DDDmod_Nop, 2000);
	Test_DDDmod_Calib = TestTime;

	Test_FFFPIremquo(Test_FFFPIremquo_Nop, Test_FFFPIremquo_Nop, 2000);
	Test_FFFPIremquo_Calib = TestTime;

	Test_DDDPIremquo(Test_DDDPIremquo_Nop, Test_DDDPIremquo_Nop, 2000);
	Test_DDDPIremquo_Calib = TestTime;

	Test_BFF(Test_BFF_Nop, Test_BFF_Nop, 2000);
	Test_BFF_Calib = TestTime;

	Test_BDD(Test_BDD_Nop, Test_BDD_Nop, 2000);
	Test_BDD_Calib = TestTime;

	Test_FI(Test_FI_Nop, Test_FI_Nop, 2000);
	Test_FI_Calib = TestTime;

	Test_DI(Test_DI_Nop, Test_DI_Nop, 2000);
	Test_DI_Calib = TestTime;

	Test_FI64(Test_FI64_Nop, Test_FI64_Nop, 2000);
	Test_FI64_Calib = TestTime;

	Test_DI64(Test_DI64_Nop, Test_DI64_Nop, 2000);
	Test_DI64_Calib = TestTime;

	Test_FII(Test_FII_Nop, Test_FII_Nop, 2000);
	Test_FII_Calib = TestTime;

	Test_DII(Test_DII_Nop, Test_DII_Nop, 2000);
	Test_DII_Calib = TestTime;

	Test_FI64I(Test_FI64I_Nop, Test_FI64I_Nop, 2000);
	Test_FI64I_Calib = TestTime;

	Test_DI64I(Test_DI64I_Nop, Test_DI64I_Nop, 2000);
	Test_DI64I_Calib = TestTime;

	Test_IF(Test_IF_Nop, Test_IF_Nop, 2000);
	Test_IF_Calib = TestTime;

	Test_ID(Test_ID_Nop, Test_ID_Nop, 2000);
	Test_ID_Calib = TestTime;

	Test_I64F(Test_I64F_Nop, Test_I64F_Nop, 2000);
	Test_I64F_Calib = TestTime;

	Test_I64D(Test_I64D_Nop, Test_I64D_Nop, 2000);
	Test_I64D_Calib = TestTime;

	Test_IFI(Test_IFI_Nop, Test_IFI_Nop, 2000);
	Test_IFI_Calib = TestTime;

	Test_IDI(Test_IDI_Nop, Test_IDI_Nop, 2000);
	Test_IDI_Calib = TestTime;

	Test_I64FI(Test_I64FI_Nop, Test_I64FI_Nop, 2000);
	Test_I64FI_Calib = TestTime;

	Test_I64DI(Test_I64DI_Nop, Test_I64DI_Nop, 2000);
	Test_I64DI_Calib = TestTime;

	Test_FFangle(Test_FFangle_Nop, Test_FFangle_Nop, 2000);
	Test_FFangle_Calib = TestTime;

	Test_DDangle(Test_DDangle_Nop, Test_DDangle_Nop, 2000);
	Test_DDangle_Calib = TestTime;

	Test_VPFPFangle(Test_VPFPFangle_Nop, Test_VPFPFangle_Nop, 2000);
	Test_VPFPFangle_Calib = TestTime;

	Test_VPDPDangle(Test_VPDPDangle_Nop, Test_VPDPDangle_Nop, 2000);
	Test_VPDPDangle_Calib = TestTime;

	Test_FFarcangle(Test_FFarcangle_Nop, Test_FFarcangle_Nop, 2000);
	Test_FFarcangle_Calib = TestTime;

	Test_DDarcangle(Test_DDarcangle_Nop, Test_DDarcangle_Nop, 2000);
	Test_DDarcangle_Calib = TestTime;

	Test_FFarctan(Test_FFarctan_Nop, Test_FFarctan_Nop, 2000);
	Test_FFarctan_Calib = TestTime;

	Test_DDarctan(Test_DDarctan_Nop, Test_DDarctan_Nop, 2000);
	Test_DDarctan_Calib = TestTime;

	Test_FFangledeg(Test_FFangledeg_Nop, Test_FFangledeg_Nop, 2000);
	Test_FFangledeg_Calib = TestTime;

	Test_DDangledeg(Test_DDangledeg_Nop, Test_DDangledeg_Nop, 2000);
	Test_DDangledeg_Calib = TestTime;

	Test_VPFPFangledeg(Test_VPFPFangledeg_Nop, Test_VPFPFangledeg_Nop, 2000);
	Test_VPFPFangledeg_Calib = TestTime;

	Test_VPDPDangledeg(Test_VPDPDangledeg_Nop, Test_VPDPDangledeg_Nop, 2000);
	Test_VPDPDangledeg_Calib = TestTime;

	Test_FFexp(Test_FFexp_Nop, Test_FFexp_Nop, 2000);
	Test_FFexp_Calib = TestTime;

	Test_DDexp(Test_DDexp_Nop, Test_DDexp_Nop, 2000);
	Test_DDexp_Calib = TestTime;

	Test_FFFF(Test_FFFF_Nop, Test_FFFF_Nop, 2000);
	Test_FFFF_Calib = TestTime;

	Test_DDDD(Test_DDDD_Nop, Test_DDDD_Nop, 2000);
	Test_DDDD_Calib = TestTime;

	Test_DF(Test_DF_Nop, Test_DF_Nop, 2000);
	Test_DF_Calib = TestTime;

	Test_FD(Test_FD_Nop, Test_FD_Nop, 2000);
	Test_FD_Calib = TestTime;

	Test_FFIdig(Test_FFIdig_Nop, Test_FFIdig_Nop, 2000);
	Test_FFIdig_Calib = TestTime;

	Test_DDIdig(Test_DDIdig_Nop, Test_DDIdig_Nop, 2000);
	Test_DDIdig_Calib = TestTime;
}

// ============ tests

// compose floating point with magnitude of 'num' and sign of 'sign'
void Test_copysignf(int loop) { Test_FFF(Ref_copysignf, copysignf, loop); }
void Test_copysign(int loop) { Test_DDD(Ref_copysign, copysign, loop); }

// check if number is an integer
void Test_isintf(int loop) { Test_BF(Ref_isintf, isintf, loop); }
void Test_isintd(int loop) { Test_BD(Ref_isintd, isintd, loop); }

// check if number is odd integer (must be an integer ..., -3, -1, 1, 3, 5,..)
void Test_isoddintf(int loop) { Test_BF(Ref_isoddintf, isoddintf, loop); }
void Test_isoddintd(int loop) { Test_BD(Ref_isoddintd, isoddintd, loop); }

// check if number is power of 2
void Test_ispow2f(int loop) { Test_BF(Ref_ispow2f, ispow2f, loop); }
void Test_ispow2d(int loop) { Test_BD(Ref_ispow2d, ispow2d, loop); }

// multiply number by power of 2 (num * 2^exp)
void Test_ldexpf(int loop) { Test_FFIshift(Ref_ldexpf, ldexpf, loop); }
void Test_ldexp(int loop) { Test_DDIshift(Ref_ldexp, ldexp, loop); }

// Addition, x + y
void Test_fadd(int loop) { Test_FFF(Ref_fadd, fadd, loop); }
void Test_dadd(int loop) { Test_DDD(Ref_dadd, dadd, loop); }

// Subtraction, x - y
void Test_fsub(int loop) { Test_FFF(Ref_fsub, fsub, loop); }
void Test_dsub(int loop) { Test_DDD(Ref_dsub, dsub, loop); }

// Multiplication, x * y
void Test_fmul(int loop) { Test_FFF(Ref_fmul, fmul, loop); }
void Test_dmul(int loop) { Test_DDD(Ref_dmul, dmul, loop); }

// Square, x^2 
void Test_fsqr(int loop) { Test_FF(Ref_fsqr, fsqr, loop); }
void Test_dsqr(int loop) { Test_DD(Ref_dsqr, dsqr, loop); }

// Division, x / y
void Test_fdiv(int loop) { Test_FFF(Ref_fdiv, fdiv, loop); }
void Test_ddiv(int loop) { Test_DDD(Ref_ddiv, ddiv, loop); }

// Reciprocal 1 / x
void Test_frec(int loop) { Test_FF(Ref_frec, frec, loop); }
void Test_drec(int loop) { Test_DD(Ref_drec, drec, loop); }

// get remainder of division x/y, rounded towards zero
//  fmod (x, y) = x - tquote*y, where tquote is truncated (i.e. rounded towards zero) result of x/y
void Test_fmodf(int loop) { Test_FFFmod(Ref_fmodf, fmodf, loop); }
void Test_fmod(int loop) { Test_DDDmod(Ref_fmod, fmod, loop); }

// compute remainder and quotient of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
void Test_remquof(int loop) { Test_FFFPIremquo(Ref_remquof, remquof, loop); }
void Test_remquo(int loop) { Test_DDDPIremquo(Ref_remquo, remquo, loop); }

// get remainder of division x/y, rounded towards the even number
//  remainder (x, y) = x - rquote*y, where rquote is result of x/y, rounded towards the nearest integral
void Test_remainderf(int loop) { Test_FFFmod(Ref_remainderf, remainderf, loop); }
void Test_remainder(int loop) { Test_DDDmod(Ref_remainder, remainder, loop); }

// Compare, x ? y
// Returns: 0 if x==y, -1 if x<y, +1 if x>y
void Test_fcmp(int loop) { Test_BFF(Ref_fcmp, fcmp, loop); }
void Test_dcmp(int loop) { Test_BDD(Ref_dcmp, dcmp, loop); }

// Compare if x==y
void Test_fcmpeq(int loop) { Test_BFF((pTest_BFF)Ref_fcmpeq, (pTest_BFF)fcmpeq, loop); }
void Test_dcmpeq(int loop) { Test_BDD((pTest_BDD)Ref_dcmpeq, (pTest_BDD)dcmpeq, loop); }

// Compare if x<y
void Test_fcmplt(int loop) { Test_BFF((pTest_BFF)Ref_fcmplt, (pTest_BFF)fcmplt, loop); }
void Test_dcmplt(int loop) { Test_BDD((pTest_BDD)Ref_dcmplt, (pTest_BDD)dcmplt, loop); }

// Compare if x<=y
void Test_fcmple(int loop) { Test_BFF((pTest_BFF)Ref_fcmple, (pTest_BFF)fcmple, loop); }
void Test_dcmple(int loop) { Test_BDD((pTest_BDD)Ref_dcmple, (pTest_BDD)dcmple, loop); }

// Compare if x>=y
void Test_fcmpge(int loop) { Test_BFF((pTest_BFF)Ref_fcmpge, (pTest_BFF)fcmpge, loop); }
void Test_dcmpge(int loop) { Test_BDD((pTest_BDD)Ref_dcmpge, (pTest_BDD)dcmpge, loop); }

// Compare if x>y
void Test_fcmpgt(int loop) { Test_BFF((pTest_BFF)Ref_fcmpgt, (pTest_BFF)fcmpgt, loop); }
void Test_dcmpgt(int loop) { Test_BDD((pTest_BDD)Ref_dcmpgt, (pTest_BDD)dcmpgt, loop); }

// Check if comparison is unordered (either input is NaN)
void Test_fcmpun(int loop) { Test_BFF((pTest_BFF)Ref_fcmpun, (pTest_BFF)fcmpun, loop); }
void Test_dcmpun(int loop) { Test_BDD((pTest_BDD)Ref_dcmpun, (pTest_BDD)dcmpun, loop); }

// Convert signed int to float
void Test_int2float(int loop) { Test_FI(Ref_int2float, int2float, loop); }
void Test_int2double(int loop) { Test_DI(Ref_int2double, int2double, loop); }

// Convert unsigned int to float
void Test_uint2float(int loop) { Test_FI((pTest_FI)Ref_uint2float, (pTest_FI)uint2float, loop); }
void Test_uint2double(int loop) { Test_DI((pTest_DI)Ref_uint2double, (pTest_DI)uint2double, loop); }

// Convert 64-bit signed int to float
void Test_int642float(int loop) { Test_FI64(Ref_int642float, int642float, loop); }
void Test_int642double(int loop) { Test_DI64(Ref_int642double, int642double, loop); }

// Convert 64-bit unsigned int to float
void Test_uint642float(int loop) { Test_FI64((pTest_FI64)Ref_uint642float, (pTest_FI64)uint642float, loop); }
void Test_uint642double(int loop) { Test_DI64((pTest_DI64)Ref_uint642double, (pTest_DI64)uint642double, loop); }

// Convert signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
void Test_fix2float(int loop) { Test_FII(Ref_fix2float, fix2float, loop); }
void Test_fix2double(int loop) { Test_DII(Ref_fix2double, fix2double, loop); }

// Convert unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
void Test_ufix2float(int loop) { Test_FII((pTest_FII)Ref_ufix2float, (pTest_FII)ufix2float, loop); }
void Test_ufix2double(int loop) { Test_DII((pTest_DII)Ref_ufix2double, (pTest_DII)ufix2double, loop); }

// Convert 64-bit signed fixed point to float
//  e = number of bits of fractional part (binary exponent)
void Test_fix642float(int loop) { Test_FI64I(Ref_fix642float, fix642float, loop); }
void Test_fix642double(int loop) { Test_DI64I(Ref_fix642double, fix642double, loop); }

// Convert 64-bit unsigned fixed point to float
//  e = number of bits of fractional part (binary exponent)
void Test_ufix642float(int loop) { Test_FI64I((pTest_FI64I)Ref_ufix642float, (pTest_FI64I)ufix642float, loop); }
void Test_ufix642double(int loop) { Test_DI64I((pTest_DI64I)Ref_ufix642double, (pTest_DI64I)ufix642double, loop); }

// Convert float to signed int, rounding to zero (C-style int conversion)
void Test_float2int_z(int loop) { Test_IF(Ref_float2int_z, float2int_z, loop); }
void Test_double2int_z(int loop) { Test_ID(Ref_double2int_z, double2int_z, loop); }

// Convert float to signed int, rounding down
void Test_float2int(int loop) { Test_IF(Ref_float2int, float2int, loop); }
void Test_double2int(int loop) { Test_ID(Ref_double2int, double2int, loop); }

// Convert float to unsigned int, rounding down
void Test_float2uint(int loop) { Test_IF((pTest_IF)Ref_float2uint, (pTest_IF)float2uint, loop); }
void Test_double2uint(int loop) { Test_ID((pTest_ID)Ref_double2uint, (pTest_ID)double2uint, loop); }

// Convert float to 64-bit signed int, rounding to zero (C-style int conversion)
void Test_float2int64_z(int loop) { Test_I64F(Ref_float2int64_z, float2int64_z, loop); }
void Test_double2int64_z(int loop) { Test_I64D(Ref_double2int64_z, double2int64_z, loop); }

// Convert float to 64-bit signed int, rounding down
void Test_float2int64(int loop) { Test_I64F(Ref_float2int64, float2int64, loop); }
void Test_double2int64(int loop) { Test_I64D(Ref_double2int64, double2int64, loop); }

// Convert float to 64-bit unsigned int, rounding down
void Test_float2uint64(int loop) { Test_I64F((pTest_I64F)Ref_float2uint64, (pTest_I64F)float2uint64, loop); }
void Test_double2uint64(int loop) { Test_I64D((pTest_I64D)Ref_double2uint64, (pTest_I64D)double2uint64, loop); }

// Convert float to signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
void Test_float2fix(int loop) { Test_IFI(Ref_float2fix, float2fix, loop); }
void Test_double2fix(int loop) { Test_IDI(Ref_double2fix, double2fix, loop); }

// Convert float to unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
void Test_float2ufix(int loop) { Test_IFI((pTest_IFI)Ref_float2ufix, (pTest_IFI)float2ufix, loop); }
void Test_double2ufix(int loop) { Test_IDI((pTest_IDI)Ref_double2ufix, (pTest_IDI)double2ufix, loop); }

// Convert float to 64-bit signed fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
void Test_float2fix64(int loop) { Test_I64FI(Ref_float2fix64, float2fix64, loop); }
void Test_double2fix64(int loop) { Test_I64DI(Ref_double2fix64, double2fix64, loop); }

// Convert float to 64-bit unsigned fixed point, rounding down
//  e = number of bits of fractional part (binary exponent)
void Test_float2ufix64(int loop) { Test_I64FI((pTest_I64FI)Ref_float2ufix64, (pTest_I64FI)float2ufix64, loop); }
void Test_double2ufix64(int loop) { Test_I64DI((pTest_I64DI)Ref_double2ufix64, (pTest_I64DI)double2ufix64, loop); }

// round number towards zero
void Test_truncf(int loop) { Test_FF(Ref_truncf, truncf, loop); }
void Test_trunc(int loop) { Test_DD(Ref_trunc, trunc, loop); }

// round number to nearest integer
void Test_roundf(int loop) { Test_FF(Ref_roundf, roundf, loop); }
void Test_round(int loop) { Test_DD(Ref_round, round, loop); }

// round number down to integer
void Test_floorf(int loop) { Test_FF(Ref_floorf, floorf, loop); }
void Test_floor(int loop) { Test_DD(Ref_floor, floor, loop); }

// round number up to integer
void Test_ceilf(int loop) { Test_FF(Ref_ceilf, ceilf, loop); }
void Test_ceil(int loop) { Test_DD(Ref_ceil, ceil, loop); }

// Square root
void Test_sqrtf(int loop) { Test_FF(Ref_sqrtf, sqrtf, loop); }
void Test_sqrt(int loop) { Test_DD(Ref_sqrt, sqrt, loop); }

// convert degrees to radians
void Test_deg2radf(int loop) { Test_FF(Ref_deg2radf, deg2radf, loop); }
void Test_deg2rad(int loop) { Test_DD(Ref_deg2rad, deg2rad, loop); }

// convert radians to degrees
void Test_rad2degf(int loop) { Test_FF(Ref_rad2degf, rad2degf, loop); }
void Test_rad2deg(int loop) { Test_DD(Ref_rad2deg, rad2deg, loop); }

// sine in radians
void Test_sinf(int loop) { Test_FFangle(Ref_sinf, sinf, loop); }
void Test_sin(int loop) { Test_DDangle(Ref_sin, sin, loop); }

// sine in degrees
void Test_sinf_deg(int loop) { Test_FFangledeg(Ref_sinf_deg, sinf_deg, loop); }
void Test_sin_deg(int loop) { Test_DDangledeg(Ref_sin_deg, sin_deg, loop); }

// cosine in radians
void Test_cosf(int loop) { Test_FFangle(Ref_cosf, cosf, loop); }
void Test_cos(int loop) { Test_DDangle(Ref_cos, cos, loop); }

// cosine in degrees
void Test_cosf_deg(int loop) { Test_FFangledeg(Ref_cosf_deg, cosf_deg, loop); }
void Test_cos_deg(int loop) { Test_DDangledeg(Ref_cos_deg, cos_deg, loop); }

// sine-cosine in radians
void Test_sincosf(int loop) { Test_VPFPFangle(Ref_sincosf, sincosf, loop); }
void Test_sincos(int loop) { Test_VPDPDangle(Ref_sincos, sincos, loop); }

// sine-cosine in degrees
void Test_sincosf_deg(int loop) { Test_VPFPFangledeg(Ref_sincosf_deg, sincosf_deg, loop); }
void Test_sincos_deg(int loop) { Test_VPDPDangledeg(Ref_sincos_deg, sincos_deg, loop); }

// tangent in radians
void Test_tanf(int loop) { Test_FFangle(Ref_tanf, tanf, loop); }
void Test_tan(int loop) { Test_DDangle(Ref_tan, tan, loop); }

// tangent in degrees
void Test_tanf_deg(int loop) { Test_FFangledeg(Ref_tanf_deg, tanf_deg, loop); }
void Test_tan_deg(int loop) { Test_DDangledeg(Ref_tan_deg, tan_deg, loop); }

// cotangent in radians
void Test_cotanf(int loop) { Test_FFangle(Ref_cotanf, cotanf, loop); }
void Test_cotan(int loop) { Test_DDangle(Ref_cotan, cotan, loop); }

// cotangent in degrees
void Test_cotanf_deg(int loop) { Test_FFangledeg(Ref_cotanf_deg, cotanf_deg, loop); }
void Test_cotan_deg(int loop) { Test_DDangledeg(Ref_cotan_deg, cotan_deg, loop); }

// arc sine in radians
void Test_asinf(int loop) { Test_FFarcangle(Ref_asinf, asinf, loop); }
void Test_asin(int loop) { Test_DDarcangle(Ref_asin, asin, loop); }

// arc sine in degrees
void Test_asinf_deg(int loop) { Test_FFarcangle(Ref_asinf_deg, asinf_deg, loop); }
void Test_asin_deg(int loop) { Test_DDarcangle(Ref_asin_deg, asin_deg, loop); }

// arc cosine in radians
void Test_acosf(int loop) { Test_FFarcangle(Ref_acosf, acosf, loop); }
void Test_acos(int loop) { Test_DDarcangle(Ref_acos, acos, loop); }

// arc cosine in degrees
void Test_acosf_deg(int loop) { Test_FFarcangle(Ref_acosf_deg, acosf_deg, loop); }
void Test_acos_deg(int loop) { Test_DDarcangle(Ref_acos_deg, acos_deg, loop); }

// arc tangent in radians
void Test_atanf(int loop) { Test_FFarctan(Ref_atanf, atanf, loop); }
void Test_atan(int loop) { Test_DDarctan(Ref_atan, atan, loop); }

// arc tangent in degrees
void Test_atanf_deg(int loop) { Test_FFarctan(Ref_atanf_deg, atanf_deg, loop); }
void Test_atan_deg(int loop) { Test_DDarctan(Ref_atan_deg, atan_deg, loop); }

// arc cotangent in radians
void Test_acotanf(int loop) { Test_FFarctan(Ref_acotanf, acotanf, loop); }
void Test_acotan(int loop) { Test_DDarctan(Ref_acotan, acotan, loop); }

// arc cotangent in degrees
void Test_acotanf_deg(int loop) { Test_FFarctan(Ref_acotanf_deg, acotanf_deg, loop); }
void Test_acotan_deg(int loop) { Test_DDarctan(Ref_acotan_deg, acotan_deg, loop); }

// arc tangent of y/x in radians
void Test_atan2f(int loop) { Test_FFF(Ref_atan2f, atan2f, loop); }
void Test_atan2(int loop) { Test_DDD(Ref_atan2, atan2, loop); }

// arc tangent of y/x in degrees
void Test_atan2f_deg(int loop) { Test_FFF(Ref_atan2f_deg, atan2f_deg, loop); }
void Test_atan2_deg(int loop) { Test_DDD(Ref_atan2_deg, atan2_deg, loop); }

// hyperbolic sine
void Test_sinhf(int loop) { Test_FFexp(Ref_sinhf, sinhf, loop); }
void Test_sinh(int loop) { Test_DDexp(Ref_sinh, sinh, loop); }

// hyperbolic cosine
void Test_coshf(int loop) { Test_FFexp(Ref_coshf, coshf, loop); }
void Test_cosh(int loop) { Test_DDexp(Ref_cosh, cosh, loop); }

// hyperbolic tangent
void Test_tanhf(int loop) { Test_FFexp(Ref_tanhf, tanhf, loop); }
void Test_tanh(int loop) { Test_DDexp(Ref_tanh, tanh, loop); }

// inverse hyperbolic sine
void Test_asinhf(int loop) { Test_FFexp(Ref_asinhf, asinhf, loop); }
void Test_asinh(int loop) { Test_DDexp(Ref_asinh, asinh, loop); }

// inverse hyperbolic cosine
void Test_acoshf(int loop) { Test_FFexp(Ref_acoshf, acoshf, loop); }
void Test_acosh(int loop) { Test_DDexp(Ref_acosh, acosh, loop); }

// inverse hyperbolic tangent
void Test_atanhf(int loop) { Test_FFexp(Ref_atanhf, atanhf, loop); }
void Test_atanh(int loop) { Test_DDexp(Ref_atanh, atanh, loop); }

// Natural exponent
void Test_expf(int loop) { Test_FFexp(Ref_expf, expf, loop); }
void Test_exp(int loop) { Test_DDexp(Ref_exp, exp, loop); }

// Natural logarithm
void Test_logf(int loop) { Test_FFlog(Ref_logf, logf, loop); }
void Test_log(int loop) { Test_DDlog(Ref_log, log, loop); }

// exponent with base 2
void Test_exp2f(int loop) { Test_FFexp(Ref_exp2f, exp2f, loop); }
void Test_exp2(int loop) { Test_DDexp(Ref_exp2, exp2, loop); }

// logarithm with base 2
void Test_log2f(int loop) { Test_FF(Ref_log2f, log2f, loop); }
void Test_log2(int loop) { Test_DD(Ref_log2, log2, loop); }

// exponent with base 10
void Test_exp10f(int loop) { Test_FFexp(Ref_exp10f, exp10f, loop); }
void Test_exp10(int loop) { Test_DDexp(Ref_exp10, exp10, loop); }

// logarithm with base 10
void Test_log10f(int loop) { Test_FF(Ref_log10f, log10f, loop); }
void Test_log10(int loop) { Test_DD(Ref_log10, log10, loop); }

// expf(x) - 1
void Test_expm1f(int loop) { Test_FFexp(Ref_expm1f, expm1f, loop); }
void Test_expm1(int loop) { Test_DDexp(Ref_expm1, expm1, loop); }

// logf(x + 1)
void Test_log1pf(int loop) { Test_FF(Ref_log1pf, log1pf, loop); }
void Test_log1p(int loop) { Test_DD(Ref_log1p, log1p, loop); }

// x*y + z
void Test_fmaf(int loop) { Test_FFFF(Ref_fmaf, fmaf, loop); }
void Test_fma(int loop) { Test_DDDD(Ref_fma, fma, loop); }

// power by integer, x^y
void Test_powintf(int loop) { Test_FFIshift(Ref_powintf, powintf, loop); }
void Test_powint(int loop) { Test_DDIshift(Ref_powint, powint, loop); }

// power x^y
void Test_powf(int loop) { Test_FFF(Ref_powf, powf, loop); }
void Test_pow(int loop) { Test_DDD(Ref_pow, pow, loop); }

// square root of sum of squares (hypotenuse), sqrt(x*x + y*y)
void Test_hypotf(int loop) { Test_FFF(Ref_hypotf, hypotf, loop); }
void Test_hypot(int loop) { Test_DDD(Ref_hypot, hypot, loop); }

// cube root, sqrt3(x), x^(1/3)
void Test_cbrtf(int loop) { Test_FF(Ref_cbrtf, cbrtf, loop); }
void Test_cbrt(int loop) { Test_DD(Ref_cbrt, cbrt, loop); }

// absolute value
void Test_absf(int loop) { Test_FF(Ref_absf, absf, loop); }
void Test_absd(int loop) { Test_DD(Ref_absd, absd, loop); }

// Convert float to double
void Test_float2double(int loop) { Test_DF(Ref_float2double, float2double, loop); }

// Convert double to float
void Test_double2float(int loop) { Test_FD(Ref_double2float, double2float, loop); }

// round to given number of significant digits
// @TODO: probably will be deleted (accuracy cannot be guaranteed)
//void Test_rounddigf(int loop) { Test_FFIdig(Ref_rounddigf, rounddigf, loop); }
//void Test_rounddig(int loop) { Test_DDIdig(Ref_rounddig, rounddig, loop); }

// consistency test sqrtf (whether exponent is synchronous with mantissa)
typedef float (*pTest_conFFsqrt)(float);
void Test_conFFsqrt(pTest_conFFsqrt invfnc, pTest_conFFsqrt reffnc, pTest_conFFsqrt fnc)
{
	DifMax = 0;
	u32 e;
	u32 xn;
	int i;
	float a, ref, tst, old;

	old = Test_MMaxF();
	for (e = 127-62; e < 127+64; e++)
	{
		if (e == 127) continue;
		a = invfnc(Test_u32float(e << 23));
		xn = Test_floatu32(a);
		for (i = -10; i <= 10; i++)
		{
			a = Test_u32float(xn + i);
			ref = reffnc(a);
			tst = fnc(a);
			int dif = Text_CheckDifF(ref, tst);
			if (tst < old) dif = 100;
			if (dif > DifMax) DifMax = dif;
#if 0
			if (dif > 8) {
				DrawPrint("conFFsqrt: %.8g %.8g %.8g %.8g\n", a, ref, tst, old);
				while(True) {} }
#endif
			old = tst;
		}
	}
}

// consistency test expf (whether exponent is synchronous with mantissa)
typedef float (*pTest_conFFexp)(float);
typedef float (*pTest_conFFlog)(float);
void Test_conFFexp(pTest_conFFlog invfnc, pTest_conFFexp reffnc, pTest_conFFexp fnc)
{
	DifMax = 0;
	u32 e;
	u32 xn;
	int i;
	float a, ref, tst, old;

	old = Test_MMaxF();
	for (e = 2; e <= 254; e++)
	{
		if (e == 127) continue; // skip testing around 1.0
		a = invfnc(Test_u32float(e << 23));
		xn = Test_floatu32(a);
		for (i = -10; i <= 10; i++)
		{
			if ((s32)xn < 0)
				a = Test_u32float(xn - i);
			else
				a = Test_u32float(xn + i);
			ref = reffnc(a);
			tst = fnc(a);
			int dif = Text_CheckDifF(ref, tst);
			if (tst < old) dif = 100;
			if (dif > DifMax) DifMax = dif;
#if 0
			if (dif > 8) {
				DrawPrint("conFFexp: %.8g %.8g %.8g %.8g\n", a, ref, tst, old);
				while(True) {} }
#endif
			old = tst;
		}
	}
}

// consistency test (whether exponent is synchronous with mantissa)
void Test_conFFlog(pTest_conFFexp invfnc, pTest_conFFlog reffnc, pTest_conFFlog fnc)
{
	DifMax = 0;
	u32 e;
	u32 xn;
	int i;
	float a, ref, tst, old;

// Do not use exponent below 127-7 - it would lose its precision around 1.0

	// positive number
	for (e = 127-7; e <= 127+7; e++)
	{
		old = Test_MMaxF();
		a = invfnc(Test_u32float(e << 23));
		if (Test_IsOverF(ref)) continue;
		xn = Test_floatu32(a);
		for (i = -10; i <= 10; i++)
		{
			a = Test_u32float(xn + i);
			ref = reffnc(a);
			if (!Test_IsOverF(ref))
			{
				tst = fnc(a);
				int dif = Text_CheckDifF(ref, tst);
				if (tst < old) dif = 100;
				if (dif > DifMax) DifMax = dif;
#if 0
				if (dif > 8) {
					DrawPrint("conFFlog+: %.8g %.8g %.8g %.8g\n", a, ref, tst, old);
					while(True) {} }
#endif
				old = tst;
			}
		}
	}

	// negative number
	for (e = 127+7; e >= 127-7; e--)
	{
		old = Test_MMaxF();
		a = invfnc(Test_u32float((e << 23) | B31));
		if (Test_IsOverF(ref)) continue;
		xn = Test_floatu32(a);
		for (i = 10; i >= -10; i--)
		{
			a = Test_u32float(xn + i);
			ref = reffnc(a);
			if (!Test_IsOverF(ref))
			{
				tst = fnc(a);
				int dif = Text_CheckDifF(ref, tst);
				if (tst < old) dif = 100;
				if (dif > DifMax) DifMax = dif;
#if 0
				if (dif > 8) {
					DrawPrint("conFFlog-: %.8g %.8g %.8g %.8g\n", a, ref, tst, old);
					while(True) {} }
#endif
				old = tst;
			}
		}
	}
}

// sqrtf() constistency check
void Test_conFF_sqrtf() { Test_conFFsqrt(Ref_fsqr, Ref_sqrtf, sqrtf); }

// expf() constistency check
void Test_conFF_expf() { Test_conFFexp(Ref_logf, Ref_expf, expf); }
void Test_conFF_exp2f() { Test_conFFexp(Ref_log2f, Ref_exp2f, exp2f); }
void Test_conFF_exp10f() { Test_conFFexp(Ref_log10f, Ref_exp10f, exp10f); }

// logf() constistency check
void Test_conFF_logf() { Test_conFFlog(Ref_expf, Ref_logf, logf); }
void Test_conFF_log2f() { Test_conFFlog(Ref_exp2f, Ref_log2f, log2f); }
void Test_conFF_log10f() { Test_conFFlog(Ref_exp10f, Ref_log10f, log10f); }
