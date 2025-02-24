
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

// FLASHSIZE ... size of the Flash memory
// RAMSIZE ... size of the RAM memory

/*
// Possible voltage values
#define VREG_VOLTAGE_0_55	0	// 0.55V
#define VREG_VOLTAGE_0_60	1	// 0.60V
#define VREG_VOLTAGE_0_65	2	// 0.65V
#define VREG_VOLTAGE_0_70	3	// 0.70V
#define VREG_VOLTAGE_0_75	4	// 0.75V
#define VREG_VOLTAGE_0_80	5	// 0.80V
#define VREG_VOLTAGE_0_85	6	// 0.85V
#define VREG_VOLTAGE_0_90	7	// 0.90V
#define VREG_VOLTAGE_0_95	8	// 0.95V
#define VREG_VOLTAGE_1_00	9	// 1.00V
#define VREG_VOLTAGE_1_05	10	// 1.05V
#define VREG_VOLTAGE_1_10	11	// 1.10V *default state
#define VREG_VOLTAGE_1_15	12	// 1.15V
#define VREG_VOLTAGE_1_20	13	// 1.20V
#define VREG_VOLTAGE_1_25	14	// 1.25V
#define VREG_VOLTAGE_1_30	15	// 1.30V
// - higher values are protected, need to set vreg_ctrl
#define VREG_VOLTAGE_1_35	16	// 1.35V
#define VREG_VOLTAGE_1_40	17	// 1.40V
#define VREG_VOLTAGE_1_50	18	// 1.50V
#define VREG_VOLTAGE_1_60	19	// 1.60V
#define VREG_VOLTAGE_1_65	20	// 1.65V
#define VREG_VOLTAGE_1_70	21	// 1.70V
#define VREG_VOLTAGE_1_80	22	// 1.80V
#define VREG_VOLTAGE_1_90	23	// 1.90V
#define VREG_VOLTAGE_2_00	24	// 2.00V
#define VREG_VOLTAGE_2_35	25	// 2.35V
#define VREG_VOLTAGE_2_50	26	// 2.50V
#define VREG_VOLTAGE_2_65	27	// 2.65V
#define VREG_VOLTAGE_2_80	28	// 2.80V
#define VREG_VOLTAGE_3_00	29	// 3.00V
#define VREG_VOLTAGE_3_15	30	// 3.15V
#define VREG_VOLTAGE_3_30	31	// 3.30V

#define VREG_VOLTAGE_UNLOCK	B8	// OR required value with this flag, to enable unlock higher voltages than 1.30V
*/
#define VOLTAGE		0		// select voltage VREG_VOLTAGE_0_85 to VREG_VOLTAGE_1_30 (_05 steps), or 0=auto-select
#define CLKDIV		0		// select Flash clock divider, or 0=auto-select
#define FMIN		120		// minimal frequency in [MHz]
#define FSTEP		200		// frequency increment in [kHz]

// RAM test buffer
#define RAMBUFN	(RAMSIZE - 180000)
u8 ALIGNED RamBuf[RAMBUFN];

// error
void ErrLock(const char* msg)
{
	printf(msg);
	WaitMs(100);
	ClockPllSysFreq(PLL_KHZ);
	FlashSetClkDiv(4);
	VregSetVoltage(VREG_VOLTAGE_1_10);

	while (True)
	{
		printf("freq=%dMHz vreg=%.2fV clk=%d tmp=%dC \r",
			(ClockGetHz(CLK_PLL_SYS)+500000)/1000000, VregVoltageFloat(), FlashClkDiv(), (int)(ADC_Temp()+0.5));
		WaitMs(1000);
	}
}

// main function
int main()
{
	int i, vreg, vreg2, clkdiv, clkdiv2;
	u8 b;
	u32 crc, ramcrc, f;

	// enable temperature sensor
	ADC_TempEnable();

	// CRC of the flash memory
	crc = Crc32ADMA((void*)XIP_BASE, FLASHSIZE);
	printf("Flash memory CRC = 0x%08X\n", crc);
	WaitMs(100);

	// check calculations
	double s, c, s2, c2;
	sincos(0.9, &s, &c);

	// set start conditions
	f = FMIN*1000;		// frequency in kHz
	vreg = -1;
	clkdiv = -1;

	while (True)
	{
		// setup flash clock divider
#if CLKDIV
		clkdiv2 = CLKDIV;
#else
		clkdiv2 = GetClkDivBySysClock(f);
#endif
		if (clkdiv2 != clkdiv)
		{
			clkdiv = clkdiv2;
			FlashInit(clkdiv);
		}

		// setup voltage
#if VOLTAGE
		vreg2 = VOLTAGE;
#else
		vreg2 = GetVoltageBySysClock(f);
#endif
		if (vreg2 != vreg)
		{
			vreg = vreg2;
			VregSetVoltage(vreg);
		}

#if USE_PICOPADVGA
		// retune VGA
		VgaRetune(f*1000);
		WaitMs(2500);
#endif

		// set system clock frequency
		ClockPllSysFreq(f);
		printf("freq=%dMHz vreg=%.2fV clk=%d tmp=%dC\n",
			(ClockGetHz(CLK_PLL_SYS)+500000)/1000000, VregVoltageFloat(), FlashClkDiv(), (int)(ADC_Temp()+0.5));

		// fill-up RAM memory
		ramcrc = CRC32A_INIT;
		for (i = 0; i < RAMBUFN; i++)
		{
			b = RandU8();
			RamBuf[i] = b;
			ramcrc = Crc32AByteTab(ramcrc, b);
		}

		// check RAM memory
		if (ramcrc != Crc32ADMA(RamBuf, RAMBUFN)) ErrLock("RAM CRC ERROR!\n");

		// check calculations
		sincos(0.9, &s2, &c2);
		if ((s != s2) || (c != c2)) ErrLock("SinCos ERROR!\n");

		// check flash memory
		if (crc != Crc32ADMA((void*)XIP_BASE, FLASHSIZE)) ErrLock("Flash CRC ERROR!\n");

		// check RAM memory again
		if (ramcrc != Crc32ADMA(RamBuf, RAMBUFN)) ErrLock("RAM CRC ERROR!\n");

		// shift frequency
		f += FSTEP;

		// quit program
		if (KeyGet() == KEY_Y)
		{
			ClockPllSysFreq(PLL_KHZ);
			FlashSetClkDiv(4);
			VregSetVoltage(VREG_VOLTAGE_1_10);

#if USE_SCREENSHOT		// use screen shots

			ScreenShot();
#endif
			ResetToBootLoader();
		}
	}
}
