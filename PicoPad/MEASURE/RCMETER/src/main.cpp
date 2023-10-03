
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// In config, all interrupt handlers should be disabled so that it does not
// distort the capacitor measurement result.

// Resistors: range 0.1 ohms to 100 Mohms with 5% accuracy (or worse near the ends of the range)
// Capacitors: range 5 pF to 5 mF with 10% accuracy (or worse near the ends of the range)

#include "include.h"

// resistor real value in ohm
#define	R1_RAW		200.0		// R1 value (200 ohm)
#define	R2_RAW		2000.0 		// R2 value (2 Kohm)
#define	R3_RAW		20000.0		// R3 value (20 Kohm)
#define	R4_RAW		200000.0 	// R4 value (200 Kohm)
#define	R5_RAW		2000000.0   	// R5 value (2 Mohm)

// resistor GPIO pin
#if USE_PICOPADVGA
#define R1_GPIO		12		// R1 pin
#define R3_GPIO		7		// R3 pin
#else
#define R1_GPIO		0		// R1 pin
#define R3_GPIO		14		// R3 pin
#endif

#define R2_GPIO		1		// R2 pin
#define R4_GPIO		26		// R4 pin
#define R5_GPIO		27		// R5 pin

// ADC input
#define ADCIN_GPIO	28		// ADC input pin 1x
#define ADCIN_MUX	ADC_MUX_GPIO28	// ADC input mux 1x

// SETUP
#define ROUT_REF_VAL	23		// output R value added to reference resistor ... >>> SETUP! <<< (set middle value of lowest range)
#define ADC_RTOP	7300000.0	// resistance of 4 unconnected output pins ... >>> SETUP! <<< (set middle value of highest range)
#define ADC_RIN		1990000.0  	// ADC input resistance ... >>> SETUP! <<< (set high value of highest range)
#define C_UP		135e-12		// parasite capacitance - charging
#define C_DN		115e-12		// parasite capacitance - discharging

#define ADC_C_DET	50		// minimal ADC value for capacitor detection

// ADC max. virtual value (to correct linearity on high end) (set high values of the ranges)
#define ADC_MAX1	4107		// R1 (200 ohm)
#define ADC_MAX2	4104		// R2 (2 Kohm)
#define ADC_MAX3	4048		// R3 (20 Kohm)
#define ADC_MAX4	4058		// R4 (200 Kohm)
#define ADC_MAX5	4060 		// R5 (2 Mohm)

// ADC min. virtual value (to correct linearity on low end) (set low values of the ranges)
#define ADC_MIN1	19		// R1 (200 ohm)
#define ADC_MIN2	15		// R2 (2 Kohm)
#define ADC_MIN3	22		// R3 (20 Kohm)
#define ADC_MIN4	23		// R4 (200 Kohm)
#define ADC_MIN5	24		// R5 (2 Mohm)

// timings
//#define PERIOD	16667		// 60 Hz (USA)
#define PERIOD		20000		// 50 Hz (Europe)
					// measure period in [us] - It should be
					// equal to the period of the power grid,
					// to compensate for interference.

#define PERIOD_FAST	200		// fast measure period in [us]

#define WAIT_STAB	5		// wait time to stabilise signal (in [us])

// compensate output resistance
#define R1_VAL		(1.0/(1/R1_RAW + 1/ADC_RTOP) + ROUT_REF_VAL)	// 200 ohm
#define R2_VAL		(1.0/(1/R2_RAW + 1/ADC_RTOP) + ROUT_REF_VAL)	// 2 Kohm
#define R3_VAL		(1.0/(1/R3_RAW + 1/ADC_RTOP) + ROUT_REF_VAL)	// 20 Kohm
#define R4_VAL		(1.0/(1/R4_RAW + 1/ADC_RTOP) + ROUT_REF_VAL)	// 200 Kohm
#define R5_VAL		(1.0/(1/R5_RAW + 1/ADC_RTOP) + ROUT_REF_VAL)	// 2 Mohm

// resistor mask
#define	R1_MASK		BIT(R1_GPIO)	// R1
#define	R2_MASK		BIT(R2_GPIO)	// R2
#define	R3_MASK		BIT(R3_GPIO)	// R3
#define	R4_MASK		BIT(R4_GPIO)	// R4
#define	R5_MASK		BIT(R5_GPIO)	// R4

// mask of all pins
#define RALL_MASK	(R1_MASK | R2_MASK | R3_MASK | R4_MASK | R5_MASK)
#define R_NUM		5		// number of resistors

// resistor index
#define R1_INX		0		// R1 index
#define R2_INX		1		// R2 index
#define R3_INX		2		// R3 index
#define R4_INX		3		// R4 index
#define R5_INX		4		// R5 index

//typedef float	FLT;			// float type
typedef double FLT;			// double type

// raw measured value
u32 Raw;

// Resistor GPIO table
const u8 ResGpio[R_NUM] = { R1_GPIO, R2_GPIO, R3_GPIO, R4_GPIO, R5_GPIO };

// Resistor value in ohm
const FLT ResVal[R_NUM] = { R1_VAL, R2_VAL, R3_VAL, R4_VAL, R5_VAL };

// ADC max. virtual value (to correct linearity on high end)
const u16 AdcMax[R_NUM] = { ADC_MAX1, ADC_MAX2, ADC_MAX3, ADC_MAX4, ADC_MAX5 };

// ADC min. virtual value (to correct linearity on low end)
const u16 AdcMin[R_NUM] = { ADC_MIN1, ADC_MIN2, ADC_MIN3, ADC_MIN4, ADC_MIN5 };

#define LOG_MUL		16384	// log multiplier

// table of -log(ur)*LOG_MUL
//	ur = relative voltage 0, 1/4096, 2/4096 .. 4094/4096, 4095/4096
//	-log(ur) = values -, 8.3178, 7.6246 .. 4,8840e-4, 2.4417e-4
//	table = values 140000, 136278, 124922 .. 8, 4
int LogUTab[4096];

// table of -log(1-ur)*LOG_MUL (inverted voltage)
//	ur = relative voltage 4095/4096, 4094/4096 .. 2/4096, 1/4096, 0
//	-log(1-ur) = values 2.4417e-4, 4,8840e-4 .. 7.6246, 8.3178, -
//	table = values 4, 8 .. 124922, 136278, 140000
int LogUITab[4096];

// max. value of the filter
#define MAX_VAL 1050000000L // 1G

// value of overflow resistor (no resistor) in 0.1 ohms
#define RMAX_VAL MAX_VAL // 100M

// value of overflow capacitor (no capacitor) in 5 pF
#define CMAX_VAL MAX_VAL // 5 mF

// history size
#define HISTNUM 16		// length of value history

// value filter
typedef struct {
	u8 num;			// number of samples in value history
	u8 write;		// write index into value history
	u8 read;		// read index into value history
	s8 unstable;		// flag, result is unstable if > 0
	s32 sum;		// sum of samples in the value history
	s32 hist[HISTNUM];	// value history
} sFilter;

// declare value filter
#define FILTER(name) sFilter name = { 0, 0, 0, 0, 0, { 0 } };

// resistor history
FILTER(ResHist);

// capacitor history
FILTER(CapHist);

// ESR history
FILTER(ESRHist);

// delete oldest entry from the history
void FilterDel(sFilter* f)
{
	f->sum -= f->hist[f->read];
	f->read++;
	if (f->read == HISTNUM) f->read = 0;
	f->num--;
}

// get average value (should not be called before first sample)
s32 FilterGet(sFilter* f)
{
	if (f->unstable > 0) return MAX_VAL;
	if (f->sum >= 0)
		return ((f->sum + f->num/2) / f->num);
	else
		return -((-f->sum + f->num/2) / f->num);
}

// add value to value filter (returns new filtered value)
s32 FilterAdd(sFilter* f, s32 val)
{
	// limit value
	if ((val > MAX_VAL) || (val < -MAX_VAL)) val = MAX_VAL;

	// delete oldest value, if value history is full
	if (f->num == HISTNUM) FilterDel(f);

	// empty history, if value has been changed significantly
	if (f->num > 0)
	{
		s32 val2 = f->sum / f->num; // current average value
		s32 dif = val - val2; // change of the value
		if (dif < 0) dif = -dif; // absolute value of the change
		s32 val3 = val2 / 3; // 30% of current value
		if (val3 < 0) val3 = -val3; // 30% of current absolute value
		if (	((dif > 10) && // do not reset due small changes around zero
			(dif > val3)) || // value must be changed by 30% at least
			(val == MAX_VAL) || // or invalid new value
			(val2 == MAX_VAL)) // or invalid old value
		{
			f->num = 0;
			f->write = 0;
			f->read = 0;
			f->unstable += 1;
			if (f->unstable > 5) f->unstable = 5;
			f->sum = 0;
		}
	}

	// add to sum and check overflow
	if ((f->sum >= 0) && (val >= 0)) // both values are positive
	{
		f->sum += val;
		while (f->sum < val) // while positive overflow
		{
			// delete one entry
			FilterDel(f);
		}
	}
	else
	{
		if ((f->sum <= 0) && (val <= 0)) // both values are negative
		{
			f->sum += val;
			while (f->sum > val) // while negative overflow
			{
				// delete one entry
				FilterDel(f);
			}
		}
		else
			// no overflow
			f->sum += val;
	}

	// increase new value
	f->num++;
	f->hist[f->write] = val;
	f->write++;
	if (f->write == HISTNUM) f->write = 0;
	f->unstable--;
	if (f->unstable < -5) f->unstable = -5;

	// get average value
	return FilterGet(f);
}

// set all resistors off
void SetResOff()
{
	GPIO_DirInMask(RALL_MASK);
}

// set resistors HIGH, clear all other resistors
void SetRes1(u32 mask)
{
	// set all resistors off
	SetResOff();

	// set ouput HIGH
	GPIO_SetMask(mask);

	// set ouput direction
	GPIO_DirOutMask(mask);
}

// set resistors LOW, clear all other resistors
void SetRes0(u32 mask)
{
	// set all resistors off
	SetResOff();

	// set ouput LOW
	GPIO_ClrMask(mask);

	// set ouput direction
	GPIO_DirOutMask(mask);
}

// check one type of resistor (returns resistance in Ohms)
FLT CheckRes1(u8 rinx, Bool fast)
{
	// switch resistor UP
	SetRes1(BIT(ResGpio[rinx]));

	// wait for stabilisation
	WaitUs(WAIT_STAB);

	// start measure
	int n = 0; // measure counter
	s32 valsum = 0; // sum of values
	u32 t = Time(); // start time

	// load samples - value 0..0xFFF, one measure takes 2.6 us
	u32 period = fast ? PERIOD_FAST : PERIOD;
	while ((Time() - t) < period) // 20 ms ... max. 7700 samples ... max. 32'000'000 sum of values
	{
		valsum += ADC_Single(); // one measure
		n++;
	}

	// set all resistors off
	SetResOff();

	// raw measured value
	Raw = (valsum + n/2)/n;

	// zero
	valsum -= AdcMin[rinx]*n;
	if (valsum <= n) return 0;

	// divide coefficient
	s32 dsum = AdcMax[rinx]*n - valsum;
	if (dsum < n) dsum = n;

	// calculate resistor
	FLT r = (FLT)dsum / (ResVal[rinx] * valsum);

	// subtract ADC input resistance
	r -= 1.0/ADC_RIN;
	if (r <= 0) return RMAX_VAL*(FLT)0.1;
	return 1/r;
}

// measure resistance, without rounding
FLT CheckResR()
{
	// fast measure
	FLT rr[R_NUM];
	rr[R1_INX] = CheckRes1(R1_INX, True);
	rr[R2_INX] = CheckRes1(R2_INX, True);
	rr[R3_INX] = CheckRes1(R3_INX, True);
	rr[R4_INX] = CheckRes1(R4_INX, True);
	rr[R5_INX] = CheckRes1(R5_INX, True);

	// last resistor will not be interpolated
	if (rr[R5_INX] >= ResVal[R5_INX])
	{
		return CheckRes1(R5_INX, False);
	}

	// first resistor will not be interpolated
	if (rr[R1_INX] <= ResVal[R1_INX])
	{
		return CheckRes1(R1_INX, False);
	}

	// find boundary (R4+R5, R3+R4, R2+R3, R1+R2)
	int rinx;
	for (rinx = R4_INX; rinx > R1_INX; rinx--)
	{
		if (rr[rinx] >= ResVal[rinx]) break;
	}

	// interpolation coefficient (0..1)
	FLT k = (rr[rinx] - ResVal[rinx]) / (ResVal[rinx+1] - ResVal[rinx]);

	// slow measures
	rr[rinx] = CheckRes1(rinx, False);
	rr[rinx+1] = CheckRes1(rinx+1, False);

	// interpolate result
	return rr[rinx] + (rr[rinx+1] - rr[rinx])*k;
}

// measure resistance with rounding
FLT CheckRes()
{
	// measure without rounding
	FLT r = CheckResR();

	// limit range
	if (r < (FLT)0.1) r = 0; // minimal measured value is 0.1 ohm
	if (r >= RMAX_VAL*(FLT)0.1) r = RMAX_VAL*(FLT)0.1; // max. value is 100 Mohm

	// convert to 0.1 ohm
	s32 ri = (s32)(r*10 + 0.5);

	// return in ohms
	return FilterAdd(&ResHist, ri)*(FLT)0.1;
}

// generate log tables
void GenLogUTab()
{
	int i, k;
	LogUTab[0] = 140000;
	LogUITab[4095] = 140000;
	for (i = 1; i <= 4095; i++)
	{
		k = double2int(-log(i/4096.0)*LOG_MUL + 0.5);
		LogUTab[i] = k;
		LogUITab[4095-i] = k;
	}
}

// check one type of capacitor (returns capacitance in farads) ... method uses logarithmic regression of the capacitor charging and discharging curves 
// Capacitor charging:
//   v-u (or u) = v * exp(-(t+t0)/RC) ... v = supply voltage, R = charging resistor
//    t = inx*delta ... inx = sample index, delta = time per one sample
// logarithm:  -log(ur) = tinx0*delta/RC + tinx*delta/RC ... ur = relative voltage (v-u)/v or u/v
//   1/RC = (n*sum(tinx*-log(ur)) - sum(tinx)*sum(-log(ur))) / (n*sum(tinx^2) - sum(tinx)^2) / delta
//   C = (n*sum(tinx^2) - sum(tinx)^2) / (n*sum(tinx*-log(ur)) - sum(tinx)*sum(-log(ur))) / R * delta

#define U_LOW	(4096/4)	// low voltage level (= 1024)
#define U_HIGH	(4096/4*3)	// high voltage level (= 3072)
#define C_MAX	100.0		// max. capacitance

//FLT C1, C2;	// debug
int LoopN;	// loop counter

FLT CheckCap1(u8 rinx, Bool fast)
{
	u32 period = fast ? PERIOD_FAST : PERIOD; //  measure period in [us]
	u32 t = Time(); // start time
	u32 gpiomask = BIT(ResGpio[rinx]); // resistor mask

	// prepare sums - charging
	int up_n = 0;		// charging number of samples
	int up_sum_tinx = 0;	// charging sum of sample index
	s64 up_sum_tinx2 = 0;	// charging sum of square sample index
	int up_sum_ur = 0;	// charging sum of -log(ur)
	s64 up_sum_tinxur = 0;	// charging sum of sample index * -log(ur)

	// prepare sums - discharging
	int dn_n = 0;		// discharging number of samples
	int dn_sum_tinx = 0;	// discharging sum of sample index
	s64 dn_sum_tinx2 = 0;	// discharging sum of square sample index
	int dn_sum_ur = 0;	// discharging sum of -log(ur)
	s64 dn_sum_tinxur = 0;	// discharging sum of sample index * -log(ur)

	// check start voltage
	if (ADC_Single() < U_LOW)
	{
		// set charging resistor
		SetRes1(gpiomask);

		// wait for stabilisation
		WaitUs(WAIT_STAB);

		// wait for charging
		while ((Time() - t) < period)
		{
			if (ADC_Single() >= U_LOW) break;
		}
	}
	else
	{
		// set discharging resistor
		SetRes0(gpiomask);

		// wait for stabilisation
		WaitUs(WAIT_STAB);

		// wait for discharging
		while ((Time() - t) < period)
		{
			if (ADC_Single() < U_LOW) break;
		}
		SetResOff();
	}

	// start time
	t = Time();

	// start time
	u32 tim1 = Time();

	// measure loop
	// - one sample takes 2.6 us, 20 ms can capture 7692 sample
	int loop = 0;
	u16 u;
	s16 ur;
	int tinx;
	while ((Time() - t) < period)
	{
		// reset time index
		tinx = 0;

		// start charge
		SetRes1(gpiomask);

		// wait for stabilisation
		WaitUs(WAIT_STAB);

		// charging
		while ((Time() - t) < period)
		{
			// get voltage
			u = ADC_Single();

			// end of charging
			if (u >= U_HIGH) break;

			// get -log(1-ur) (4 .. 140000)
			ur = LogUITab[u];

			// sum
			up_n++;				// charging number of samples (max. 7692)
			up_sum_tinx += tinx;		// charging sum of sample index (max. 59 M)
			up_sum_tinx2 += (s32)(tinx*tinx); // charging sum of square sample index (max. 455 G)
			up_sum_ur += ur;		// charging sum of -log(ur) (max. 1 G)
			up_sum_tinxur += (s32)(tinx*ur); // charging sum of sample index * -log(ur) (max. 8283 G)

			// increment time index
			tinx++;
		}

		// time-out
		if ((Time() - t) >= period) break;

		// reset time index
		tinx = 0;

		// start discharge
		SetRes0(gpiomask);

		// wait for stabilisation
		WaitUs(WAIT_STAB);

		// discharging
		while ((Time() - t) < period)
		{
			// get voltage
			u = ADC_Single();

			// end of discharging
			if (u < U_LOW) break;

			// get -log(ur)
			ur = LogUTab[u];

			// sum
			dn_n++;				// discharging number of samples (max. 7692)
			dn_sum_tinx += tinx;		// discharging sum of sample index (max. 59 M)
			dn_sum_tinx2 += (s32)(tinx*tinx); // discharging sum of square sample index (max. 455 G)
			dn_sum_ur += ur;		// discharging sum of -log(ur) (max. 1 G)
			dn_sum_tinxur += (s32)(tinx*ur); // discharging sum of sample index * -log(ur) (max. 8283 G)

			// increment time index
			tinx++;
		}

		// loop counter
		loop++;
	}

	// stop time
	u32 tim2 = Time();

	// loop counter
	LoopN = loop;

	// time delta (time per one sample)
	FLT delta = (FLT)(u32)(tim2 - tim1) / (up_n + dn_n) * 1e-6;

	// resistor value
	FLT r = ResVal[rinx];

	// calculate charging capacitance
//C1 = 0;
	FLT k = up_n*(FLT)up_sum_tinxur - (FLT)up_sum_tinx*up_sum_ur;
	if (k == 0) return 0;
	FLT cup = (up_n*(FLT)up_sum_tinx2 - (FLT)up_sum_tinx*up_sum_tinx) / (k * r) * delta * LOG_MUL;
	cup -= C_UP;
//C1 = cup;
//C2 = 0;
	if (dn_n == 0) return cup;

	// calculate discharging capacitance
	k = dn_n*(FLT)dn_sum_tinxur - (FLT)dn_sum_tinx*dn_sum_ur;
	if (k == 0) return 0;
	FLT cdn = (dn_n*(FLT)dn_sum_tinx2 - (FLT)dn_sum_tinx*dn_sum_tinx) / (k * r) * delta * LOG_MUL;
	cdn -= C_DN;
//C2 = cdn;
	return (cup + cdn)/2;
}

// check capacitor, without rounding (returns capacitance in farads)
FLT CheckCapC()
{
	// measure with R4
	FLT c = CheckCap1(R4_INX, False);
	if (LoopN > 6) return c;

	// measure with R3
	c = CheckCap1(R3_INX, False);
	if (LoopN > 6) return c;

	// measure with R2
	c = CheckCap1(R2_INX, False);
	if (LoopN > 6) return c;

	// measure with R1
	return CheckCap1(R1_INX, False);
}

// check capacitor, with rounding (returns capacitance in farads)
FLT CheckCap()
{
	// measure without rounding
	FLT c = CheckCapC();

	// limit range
	if (c < 5e-12) c = 0; // minimal measured value is 5 pF
	if (c >= CMAX_VAL*5e-12) c = CMAX_VAL*5e-12; // max. value is 5 mF

	// convert to 5 pF
	s32 ci = (s32)(c/5e-12 + 0.5);

	// return in farads
	return FilterAdd(&CapHist, ci)*5e-12;
}

// measure ESR, without rounding (returns resistance in Ohms)
FLT CheckESR1()
{
	u32 period = PERIOD; //  measure period in [us]
	u32 t = Time(); // start time
	u32 gpiomask = BIT(ResGpio[R1_INX]); // resistor mask
	s16 v, v1, v2, v3, r;
	s32 a = 0;

	// loop
	int n = 0;
	while ((Time() - t) < period)
	{
		// discharge capacitor
		SetRes0(gpiomask);

		// delay 50 us to discharge capacitor
		WaitUs(50);

		// switch all resistors OFF
		SetResOff();

		// load sample at OFF state
		v1 = ADC_Single();

		// charge capacitor
		SetRes1(gpiomask);

		// load sample at HIGH state
		v2 = ADC_Single();

		// switch all resistors OFF
		SetResOff();

		// load sample at OFF state
		v3 = ADC_Single();

		// check overflow, capacitor is too small or ESR is too high
		r = v2 - v1;
		if (((v3 - v1) > 3000) || (r > 3000)) return RMAX_VAL*(FLT)0.1;

		// calculate resistor value
		// I = (Vcc - v2) / (R1 + R0)
		// R = (v2 - v1) / I = (v2 - v1) * (R1 + R0) / (Vcc - v2)
		v = (4095 + 2) - v2;
		if (r < 0) r = 0; // value 0..3000
		r = (r * R1_VAL * 100 + v/2) / v; // result in 0.01 Ohm

		// add to accumulator
		a += r;
		n++;
	}

	// get ESR value
	FLT res = (FLT)a / n * 0.01;

	// limit to 100 ohm
	if (res >= 100) res = 100;
	return res;
}

// measure ESR, with rounding (returns resistance in Ohms)
FLT CheckESR()
{
	// measure ESR, without rounding
	FLT r = CheckESR1();

	// convert to 0.01 ohm
	s32 ri = (s32)(r*100 + 0.5);

	// return in ohms
	return FilterAdd(&ESRHist, ri)*(FLT)0.01;
}

#define BUFN	50
char Buf[BUFN];

// main function
int main()
{
	int i, strip;
	FLT r, c, esr;

	// generate log tables
	GenLogUTab();

	// initialize ADC input
	// ...ADC_Init();	// ADC already initialized in BatInit()
	ADC_PinInit(ADCIN_GPIO);
	GPIO_NoPull(ADCIN_GPIO);
	GPIO_SchmittDisable(ADCIN_GPIO);
	ADC_Mux(ADCIN_MUX);

	// initialize probe outputs
	GPIO_InitMask(RALL_MASK);
	for (i = 0; i < R_NUM; i++)
	{
		GPIO_Drive12mA(ResGpio[i]);
		GPIO_NoPull(ResGpio[i]);
		GPIO_InDisable(ResGpio[i]);
	}
	esr = 0;

	// main loop
	while (True)
	{
		// measure
		r = CheckRes();
		c = CheckCap();
		if ((c >= 1e-7) && (c <= 5e-3)) esr = CheckESR();

		// wait for VSync (LCD does nothing)
		//VgaWaitVSync();

		// VGA loop all strips; LCD has only 1 strip
		for (strip = DISP_STRIP_NUM; strip > 0; strip--)
		{
			// next strip
			DispSetStripNext();

			// clear display
			DrawClear();

// DEBUG: Show measure of single resistors
#if 0
			// measure with R1 200 ohm
			MemPrint(Buf, BUFN, "R1:%.4T  ", CheckRes1(R1_INX, False));
			DrawTextBg2(Buf, 0, 0*32, COL_WHITE, COL_BLACK);

			// measure with R2 2 Kohm
			MemPrint(Buf, BUFN, "R2=%.4T  ", CheckRes1(R2_INX, False));
			DrawTextBg2(Buf, 160, 0*32, COL_WHITE, COL_BLACK);

			// measure with R3 20 Kohm
			MemPrint(Buf, BUFN, "R3=%.4T  ", CheckRes1(R3_INX, False));
			DrawTextBg2(Buf, 0, 1*32, COL_WHITE, COL_BLACK);

			// measrure with R4 200 Kohm
			MemPrint(Buf, BUFN, "R4=%.4T  ", CheckRes1(R4_INX, False));
			DrawTextBg2(Buf, 160, 1*32, COL_WHITE, COL_BLACK);

			// measure with R5 2 Mohm
			MemPrint(Buf, BUFN, "R5=%.4T  ", CheckRes1(R5_INX, False));
			DrawTextBg2(Buf, 0, 2*32, COL_WHITE, COL_BLACK);
#endif

// DEBUG: Show measure of single capacitor
#if 0
			// measure with R1 200 ohm
			CheckCap1(R1_INX, False);
			MemPrint(Buf, BUFN, "%d C1:%.4T %.4T  ", LoopN, C1, C2);
			DrawTextBg2(Buf, 0, 0*32, COL_WHITE, COL_BLACK);

			// measure with R2 2 Kohm
			CheckCap1(R2_INX, False);
			MemPrint(Buf, BUFN, "%d C2:%.4T %.4T  ", LoopN, C1, C2);
			DrawTextBg2(Buf, 0, 1*32, COL_WHITE, COL_BLACK);

			// measure with R3 20 Kohm
			CheckCap1(R3_INX, False);
			MemPrint(Buf, BUFN, "%d C3:%.4T %.4T  ", LoopN, C1, C2);
			DrawTextBg2(Buf, 0, 2*32, COL_WHITE, COL_BLACK);

			// measrure with R4 200 Kohm
			CheckCap1(R4_INX, False);
			MemPrint(Buf, BUFN, "%d C4:%.4T %.4T  ", LoopN, C1, C2);
			DrawTextBg2(Buf, 0, 3*32, COL_WHITE, COL_BLACK);

			// measure with R5 2 Mohm
			CheckCap1(R5_INX, False);
			MemPrint(Buf, BUFN, "%d C5:%.4T %.4T  ", LoopN, C1, C2);
			DrawTextBg2(Buf, 0, 4*32, COL_WHITE, COL_BLACK);
#endif

#define RES_Y	20
#define CAP_Y	130

			// titles
			DrawText2("Resistance:", 0, RES_Y, COL_YELLOW);
			DrawText2("Capacitance:", 0, CAP_Y, COL_YELLOW);

			// measure resistance
			if (r <= 100e6) // resistance is valid
			{
				// short circuit
				if (r < 0.1)
				{
					DrawTextBg4("shortcut", 0, RES_Y + 32, COL_RED, COL_BLACK);
				}

				// valid resistor value
				else
				{
					if (r < 100)
						MemPrint(Buf, BUFN, "%.1fohm", r);
					else
						MemPrint(Buf, BUFN, "%.4Tohm", r);
					DrawTextBg4(Buf, 0, RES_Y + 32, COL_WHITE, COL_BLACK);
				}
			}
			else
				// no resistance
				DrawTextBg4("-", 0, RES_Y + 32, COL_WHITE, COL_BLACK);

			// measure capacitance
			if ((c >= 5e-12) && (c <= 5e-3)) // capacitance is valid
			{
				MemPrint(Buf, BUFN, "%.4TF", c);
				DrawTextBg4(Buf, 0, CAP_Y + 32, COL_WHITE, COL_BLACK);

				// measure ESR resistance (>= 0.1 uF)
				if (c >= 1e-7)
				{
					if ((esr > 0) && (esr < 1000))
					{
						DrawTextBg2("ESR:         ", 0, RES_Y, COL_YELLOW, COL_BLACK);
						MemPrint(Buf, BUFN, "%.2fohm   ", esr);
						DrawTextBg4(Buf, 0, RES_Y + 32, COL_WHITE, COL_BLACK);
					}
				}
			}
			else
				// no capacitance
				DrawTextBg4("-", 0, CAP_Y + 32, COL_WHITE, COL_BLACK);

			// display update
			DispUpdate();
		}

		// quit program
		switch (KeyGet())
		{
		case KEY_Y:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			ResetToBootLoader();
		}
	}
}
