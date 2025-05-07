
// ****************************************************************************
//
//                               Clocks
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#include "../../global.h"	// globals
#include "../inc/sdk_clocks.h"
#include "../inc/sdk_rosc.h"
#include "../inc/sdk_xosc.h"
#include "../inc/sdk_pll.h"
#include "../inc/sdk_watchdog.h"
#include "../inc/sdk_irq.h"
#include "../inc/sdk_gpio.h"
#include "../inc/sdk_ticks.h"
#include "../inc/sdk_timer.h"
#include "../inc/sdk_vreg.h"
#include "../inc/sdk_powman.h"
#include "../inc/sdk_psram.h"
#include "../inc/sdk_flash.h"

// current clock frequency in Hz (clock lines and clock generators)
u32 CurrentFreq[CLK_SRC_NUM];

// current clock source of clock lines (0 = no clock source)
u8 CurrentClkSrc[CLK_SRC_NUM];

// resus callback user function (NULL=not used)
resus_callback_t ResusCallback = NULL;

// convert clock source index to internal source index (-1 unsupported, B6=glitchless)
#if RP2040
const s8 ClockSourceTab[5*12] = {
// src:	REF	SYS	PERI	USB	ADC	RTC	ROSC	XOSC	PLLSYS	PLLUSB	GPIN0	GPIN1

// CLK_GPOUT0..CLK_GPOUT3 clock line
	10,	6,	-1,	7,	8,	9,	4,	5,	0,	3,	1,	2,

// CLK_REF clock line
	-1,	-1,	-1,	-1,	-1,	-1,	0+B6,	2+B6,	-1,	0,	1,	2,

// CLK_SYS clock line
	0+B6,	-1,	-1,	-1,	-1,	-1,	2,	3,	0,	1,	4,	5,

// CLK_PERI clock line
	-1,	0,	-1,	-1,	-1,	-1,	3,	4,	1,	2,	5,	6,

// CLK_USB, CLK_ADC, CLK_RTC clock line
	-1,	-1,	-1,	-1,	-1,	-1,	2,	3,	1,	0,	4,	5,
};
#else
const s8 ClockSourceTab[6*15] = {
// src:	REF	SYS	PERI	HSTX	USB	ADC	ROSC	XOSC	PLLSYS	PLLUSB	GPIN0	GPIN1	LPOSC	OTP2FC	PLLOPCG

// CLK_GPOUT0..CLK_GPOUT3 clock line
	11,	8,	12,	13,	9,	10,	5,	6,	0,	3,	1,	2,	7,	14,	4,

// CLK_REF clock line
	-1,	-1,	-1,	-1,	-1,	-1,	0+B6,	2+B6,	-1,	0,	1,	2,	3+B6,	-1,	3,

// CLK_SYS clock line
	0+B6,	-1,	-1,	-1,	-1,	-1,	2,	3,	0,	1,	4,	5,	-1,	-1,	-1,

// CLK_PERI clock line
	-1,	0,	-1,	-1,	-1,	-1,	3,	4,	1,	2,	5,	6,	-1,	-1,	-1,

// CLK_HSTX clock line
	-1,	0,	-1,	-1,	-1,	-1,	-1,	-1,	1,	2,	3,	4,	-1,	-1,	-1,

// CLK_USB, CLK_ADC clock line
	-1,	-1,	-1,	-1,	-1,	-1,	2,	3,	1,	0,	4,	5,	-1,	-1,	-1,
};
#endif

// convert clock line to frequency counter internal index
const u8 ClockFreqTab[] = {
#if RP2040
	8,	// 4: CLK_REF
	9,	// 5: CLK_SYS
	10,	// 6: CLK_PERI
	11,	// 7: CLK_USB
	12,	// 8: CLK_ADC
	13,	// 9: CLK_RTC
	3,	// 10: CLK_ROSC
	5,	// 11: CLK_XOSC
	1,	// 12: CLK_PLL_SYS
	2,	// 13: CLK_PLL_USB
	6,	// 14: CLK_GPIN0
	7,	// 15: CLK_GPIN1
#else
	8,	// 4: CLK_REF
	9,	// 5: CLK_SYS
	10,	// 6: CLK_PERI
	13,	// 7: CLK_HSTX
	11,	// 8: CLK_USB
	12,	// 9: CLK_ADC
	3,	// 10: CLK_ROSC
	5,	// 11: CLK_XOSC
	1,	// 12: CLK_PLL_SYS
	2,	// 13: CLK_PLL_USB
	6,	// 14: CLK_GPIN0
	7,	// 15: CLK_GPIN1
	14,	// 16: CLK_LPOSC
	15,	// 17: CLK_OTP_2FC
	16,	// 18: CLK_PLL_OPCG
#endif
};

// convert frequency counter internal index to clock line (CLK_SRC_NUM = not valid index)
const u8 ClockFC0Tab[] = {
	CLK_SRC_NUM,	// 0 NULL
	CLK_PLL_SYS,	// 1
	CLK_PLL_USB,	// 2
	CLK_ROSC,	// 3
	CLK_ROSC,	// 4 rosc_clksrc_ph
	CLK_XOSC,	// 5
	CLK_GPIN0,	// 6
	CLK_GPIN1,	// 7
	CLK_REF,	// 8
	CLK_SYS,	// 9
	CLK_PERI,	// 10
	CLK_USB,	// 11
	CLK_ADC,	// 12
#if RP2040
	CLK_RTC,	// 13
#else
	CLK_HSTX,	// 13
	CLK_LPOSC,	// 14
	CLK_OTP_2FC,	// 15
	CLK_PLL_OPCG,	// 16
#endif
};

// clock source names
const char* ClockName[CLK_SRC_NUM] =
#if RP2040
	{"GPOUT0", "GPOUT1", "GPOUT2", "GPOUT3", "REF", "SYS", "PERI", "USB",
	"ADC", "RTC", "ROSC", "XOSC", "PLL_SYS", "PLL_USB", "GPIN0", "GPIN1" };
#else
	{"GPOUT0", "GPOUT1", "GPOUT2", "GPOUT3", "REF", "SYS", "PERI", "HSTX", "USB", "ADC",
	"ROSC", "XOSC", "PLL_SYS", "PLL_USB", "GPIN0", "GPIN1", "LPOSC", "OTP_2FC", "PLL_OPCG" };
#endif

// stop clock CLK_* (cannot stop SYS and REF clocks)
void ClockStop(int clk)
{
	if ((clk != CLK_SYS) && (clk != CLK_REF))
	{
		// clear ENABLE bit (SYS and REF does not have ENABLE bit, will not stop)
		RegClr(CLK_CTRL(clk), B11);

		// clear current frequency
		CurrentFreq[clk] = 0;
	}
}

// setup clock line (returns new frequency in Hz or 0 on error)
//  clk ... clock line index CLK_GPOUT0..CLK_RTC (CLK_ADC)
//  clksrc ... clock source CLK_REF..CLK_GPIN1 (CLK_PLL_OPCG) (see table which sources are supported)
//  freq ... required frequency in Hz, 0=use source frequency
//  freqsrc ... frequency in Hz of source, 0=get from table (must be freqsrc >= freq)
u32 ClockSetup(int clk, int clksrc, u32 freq, u32 freqsrc)
{
	if ((u32)clk >= (u32)CLK_NUM) return 0; // invalid clock line index

	// get frequency from table
	if (freqsrc == 0) freqsrc = CurrentFreq[clksrc];
	if (freq == 0) freq = freqsrc;
	if (freq > freqsrc) return 0; // invalid frequency

	// convert clock source to internal source index
	clksrc -= 4; // CLK_GPOUTn as clock source not supported
	if ((u32)clksrc >= (u32)(CLK_SRC_NUM-4)) return 0; // invalid clock source index

	int i = clk - 3; // CLK_GPOUT3 as base of table
	if (i < 0) i = 0; // CLK_GPOUT0..CLK_GPOUT3 as index 0
	if (i > CLK_USB-3) i = CLK_USB-3; // combine CLK_USB, CLK_ADC and CLK_RTC (RP2350: without CLK_RTC)
	i = ClockSourceTab[i*(CLK_SRC_NUM-4) + clksrc]; // get internal source index
	if (i < 0) return 0; // invalid clock source

	// save current clock source
	CurrentClkSrc[clk] = (u8)(clksrc + 4);

	// prepare source indices
	int aux = (i & 0x1f); // auxiliary mux index
	int src = 1; // glitchless mux source is auxiliary mux
	if ((i & B6) != 0) // clock source is glitchless mux
	{
		src = aux; // glitchless mux source
		aux = 0; // not using auxiliary mux
	}

	// prepare registers
	clock_hw_t* hw = ClockGetHw(clk); // get clock line hardware interface
	volatile u32* ctrl = &hw->ctrl; // control
	volatile u32* div = &hw->div; // divisor
	volatile u32* sel = &hw->selected; // selected

	// divider, including 8-bit (or 16-bit) fraction (so multiply result by 2^8 or 2^16)
	u32 d = (u32)((((u64)freqsrc << CLK_DIV_FRAC_BITS) + freq/2) / freq);

	// increasing divisor, so set divisor before source (to avoid overspeed)
	if (d > *div) *div = d;

	// If swithing glitchless clock (REF or SYS) to aux source,
	// switch away from aux to avoid glitches.
	if (GLITCHLESS_MUX(clk) && (src == 1))
	{
		RegClr(ctrl, B0|B1);	// switch to base source 0
		while((*sel & B0) == 0) {} // wait for switching to source 0
	}
	else
	{
		// stop clock, on REF and SYS clock it does nothing
		RegClr(ctrl, B11);

		// if clock is running, wait a while
		if (CurrentFreq[clk] > 0)
		{
			// wait 3 cycles of target clock to stop
			//  required number of loops = system_clock / current_clock + 1
			u32 delay = CurrentFreq[CLK_SYS] * 3 / CurrentFreq[clk] + 1;

			// delay in assembler
			BusyWaitCycles(delay);
		}
	}

	// set aux mux first
	RegMask(ctrl, (u32)aux << 5, 0xf << 5);

	// set glitchless mux
	if (GLITCHLESS_MUX(clk))
	{
		RegMask(ctrl, src, 3); // set src
		while((*sel & BIT(src)) == 0) {} // wait for switching to src
	}

	// set divisor after selecting mux
	*div = d;

	// start clock, on REF and SYS clock it does nothing
	RegSet(ctrl, B11);

	// store new current frequency in Hz
	freq = (u32)(((u64)freqsrc << CLK_DIV_FRAC_BITS) / d);
	CurrentFreq[clk] = freq;

	return freq;
}

// initialize clocks after start
void ClockInit()
{
	int i;

	// initialize frequency table
	for (i = 0; i < CLK_SRC_NUM; i++) CurrentFreq[i] = 0; // clear all frequencies

	// clear clock source
	for (i = 0; i < CLK_SRC_NUM; i++) CurrentClkSrc[i] = 0;

	// diable resus
	*CLK_RES_CTRL = 0;

	// enable ROSC ring oscillator
#if USE_ROSC	// use ROSC ring oscillator (sdk_rosc.c, sdk_rosc.h)
	RoscInit();
#else
	CurrentFreq[CLK_ROSC] = ROSC_MHZ*MHZ; // default ROSC ring oscillator is 6 MHz (or 11 MHz)
#endif

	// enable XOSC crystal oscillator
#if USE_XOSC	// use XOSC crystal oscillator (sdk_xosc.c, sdk_xosc.h)
	XoscInit();
#else
	CurrentFreq[CLK_XOSC] = XOSC_MHZ*MHZ; // default XOR crystal oscillator is 12 MHz
#endif

	// start system ticks
#if RP2040

#if USE_WATCHDOG || USE_TIMER // use Watchdog or Timer
#if USE_XOSC	// use XOSC crystal oscillator (sdk_xosc.c, sdk_xosc.h)
	WatchdogStart(XOSC_MHZ);
#else // USE_XOSC
	WatchdogStart(ROSC_MHZ);
#endif // USE_XOSC
#endif // USE_WATCHDOG || USE_TIMER

#else // RP2350

#if USE_XOSC	// use XOSC crystal oscillator (sdk_xosc.c, sdk_xosc.h)
	TickStartAll(XOSC_MHZ);
#else // USE_XOSC
	TickStartAll(ROSC_MHZ);
#endif // USE_XOSC

#endif // RP2040

#if RP2350 && USE_TIMER		// 1=use MCU RP2350
	// Initialize system clock timer
	Timer2Init();
#endif

	// disable aux sources of SYS and REF, to enable changes of PLLs
	RegClr(CLK_CTRL(CLK_SYS), B0|B1);	// glitchless switch SYS to base source 0 (= CLK_REF)
	while((*CLK_SEL(CLK_SYS) & B0) == 0) {} // wait for switching to source 0
	RegClr(CLK_CTRL(CLK_REF), B0|B1);	// glitchless switch REF to base source 0 (= CLK_ROSC)
	while((*CLK_SEL(CLK_REF) & B0) == 0) {} // wait for switching to source 0

#if USE_PLL	// use PLL phase-locked loop (sdk_pll.c, sdk_pll.h)

	// set PLL_SYS to default 125 MHz (or 150 MHz)
	PllSetFreq(PLL_SYS, PLL_KHZ);

	// set PLL_USB to RP2040 2*48=96 MHz, RP2350 3*48=144 MHz
#if RP2040
	PllSetFreq(PLL_USB, 2*48000);
#else
	PllSetFreq(PLL_USB, 3*48000);
#endif

	// setup CLK_REF to XOSC
	ClockSetup(CLK_REF, CLK_XOSC, 0, 0);

	// setup CLK_SYS to PLL_SYS
	ClockSetup(CLK_SYS, CLK_PLL_SYS, 0, 0);

	// setup CLK_USB to PLL_USB
	ClockSetup(CLK_USB, CLK_PLL_USB, 48*MHZ, 0);

	// setup CLK_ADC to PLL_USB
	ClockSetup(CLK_ADC, CLK_PLL_USB, 48*MHZ, 0);

#if RP2040
	// setup CLK_RTC to PLL_USB, 48MHz/1024 = 46875 Hz
	ClockSetup(CLK_RTC, CLK_PLL_USB, 46875, 0);
#endif // RP2040

	// setup CLK_PERI to PLL_USB
	ClockSetup(CLK_PERI, CLK_PLL_USB, 0, 0);

#if !RP2040
	// setup CLK_HSTX to PLL_USB
	ClockSetup(CLK_HSTX, CLK_PLL_USB, 0, 0);
#endif

#elif USE_XOSC	// USE_PLL ... use XOSC crystal oscillator

	// setup CLK_REF to XOSC
	ClockSetup(CLK_REF, CLK_XOSC, 0, 0);

	// setup CLK_SYS to XOSC
	ClockSetup(CLK_SYS, CLK_XOSC, 0, 0);

	// setup CLK_USB to XOSC
	ClockSetup(CLK_USB, CLK_XOSC, 0, 0);

	// setup CLK_ADC to XOSC
	ClockSetup(CLK_ADC, CLK_XOSC, 0, 0);

#if RP2040
	// setup CLK_RTC to XOSC, 12MHz/256 = 46875 Hz
	ClockSetup(CLK_RTC, CLK_XOSC, 46875, 0);
#endif

	// setup CLK_PERI to XOSC
	ClockSetup(CLK_PERI, CLK_XOSC, 0, 0);

#if !RP2040
	// setup CLK_HSTX to XOSC
	ClockSetup(CLK_HSTX, CLK_XOSC, 0, 0);
#endif

#else 	// USE_PLL ... else use ROSC ring oscillator

	// setup CLK_REF to ROSC
	ClockSetup(CLK_REF, CLK_ROSC, 0, 0);

	// setup CLK_SYS to ROSC
	ClockSetup(CLK_SYS, CLK_ROSC, 0, 0);

	// setup CLK_USB to ROSC
	ClockSetup(CLK_USB, CLK_ROSC, 0, 0);

	// setup CLK_ADC to ROSC
	ClockSetup(CLK_ADC, CLK_ROSC, 0, 0);

#if RP2040
	// setup CLK_RTC to ROSC, 6MHz/128 = 46875 Hz
	ClockSetup(CLK_RTC, CLK_ROSC, 46875, 0);
#endif

	// setup CLK_PERI to ROSC
	ClockSetup(CLK_PERI, CLK_ROSC, 0, 0);

#if !RP2040
	// setup CLK_HSTX to ROSC
	ClockSetup(CLK_HSTX, CLK_ROSC, 0, 0);
#endif

#endif

}

#if USE_PLL	// use PLL phase-locked loop (sdk_pll.c, sdk_pll.h)

// set system clock PLL to new setup (dependent clocks are not updated)
//   fbdiv ... feedback divisor, 16..320
//   div1 ... post divider 1, 1..7
//   div2 ... post divider 2, 1..7 (should be div1 >= div2, but auto-corrected)
void ClockPllSysSetup(int fbdiv, int div1, int div2)
{
	// temporary reconnect CLK_SYS to PLL_USB
	ClockSetup(CLK_SYS, CLK_PLL_USB, 0, 0);

	// setup PLL generator
	PllSetup(PLL_SYS, 1, fbdiv, div1, div2);

	// reconnect CLK_SYS back to PLL_SYS
	ClockSetup(CLK_SYS, CLK_PLL_SYS, 0, 0);
}

// set system clock PLL to new frequency in kHz (dependent clocks are not updated)
//   freq ... required frequency in [kHz]
void ClockPllSysFreq(u32 freq)
{
	// check current frequency
	if (freq == ClockGetHz(CLK_PLL_SYS)/1000) return;

	// temporary reconnect CLK_SYS to PLL_USB
	ClockSetup(CLK_SYS, CLK_PLL_USB, 0, 0);

	// setup PLL generator
	PllSetFreq(PLL_SYS, freq);

	// reconnect CLK_SYS back to PLL_SYS
	ClockSetup(CLK_SYS, CLK_PLL_SYS, 0, 0);
}

// get recommended flash divider by system clock in kHz
int GetClkDivBySysClock(u32 freq)
{
#if RP2040
	if (freq >= 300000) return 6;
	return 4;
#else
	if (freq >= 500000) return 6;
	if (freq >= 400000) return 5;
	if (freq >= 300000) return 4;
	if (freq >= 200000) return 3;
	return 2;
#endif
}

// get recommended voltage by system clock in kHz (return VREG_VOLTAGE_1_10 .. VREG_VOLTAGE_1_30)
int GetVoltageBySysClock(u32 freq)
{
#if !RP2040 && USE_VREG_LOCKED		// 1=enable vreg locked values > 1.30V from function GetVoltageBySysClock() of RP2350
	if (freq >= 530000) return VREG_VOLTAGE_1_70 | VREG_VOLTAGE_UNLOCK;
	if (freq >= 500000) return VREG_VOLTAGE_1_65 | VREG_VOLTAGE_UNLOCK;
	if (freq >= 470000) return VREG_VOLTAGE_1_60 | VREG_VOLTAGE_UNLOCK;
	if (freq >= 440000) return VREG_VOLTAGE_1_50 | VREG_VOLTAGE_UNLOCK;
	if (freq >= 410000) return VREG_VOLTAGE_1_40 | VREG_VOLTAGE_UNLOCK;
	if (freq >= 380000) return VREG_VOLTAGE_1_35 | VREG_VOLTAGE_UNLOCK;
#endif
	if (freq >= 340000) return VREG_VOLTAGE_1_30;
	if (freq >= 300000) return VREG_VOLTAGE_1_25;
	if (freq >= 250000) return VREG_VOLTAGE_1_20;
	if (freq >= 200000) return VREG_VOLTAGE_1_15;
	return VREG_VOLTAGE_1_10;
}

// set system clock PLL to new frequency in kHz and auto-set system voltage and flash divider (dependent clocks are not updated)
//   freq ... required frequency in [kHz]
void ClockPllSysFreqVolt(u32 freq)
{
	// get current frequency in kHz
	u32 old = ClockGetHz(CLK_PLL_SYS)/1000;

	// check current frequency
	if (freq == old) return;

	// temporary reconnect CLK_SYS to PLL_USB
	ClockSetup(CLK_SYS, CLK_PLL_USB, 0, 0);

	// raise system clock
	if (freq > old)
	{
		FlashInit(GetClkDivBySysClock(freq));
		VregSetVoltage(GetVoltageBySysClock(freq));
	}

	// setup PLL generator
	PllSetFreq(PLL_SYS, freq);

	// lower system clock
	if (freq < old)
	{
		FlashInit(GetClkDivBySysClock(freq));
		VregSetVoltage(GetVoltageBySysClock(freq));
	}

	// reconnect CLK_SYS back to PLL_SYS
	ClockSetup(CLK_SYS, CLK_PLL_SYS, 0, 0);

#if USE_PSRAM	// use PSRAM Memory (sdk_psram.c, sdk_psram.h)
	// setup PSRAM timing (should be called after sys_clock change)
	PSRAM_Timing();
#endif
}

#endif // USE_PLL	// use PLL phase-locked loop (sdk_pll.c, sdk_pll.h)

// precise measure frequency of clock CLK_REF..CLK_GPIN1 with result in Hz (returns 0 on error)
//  - measure interval: 128 ms, resolution +-15 Hz
u32 FreqCount(int clk)
{
	// convert to internal clock index
	clk -= 4;
	if ((u32)clk >= (u32)(CLK_SRC_NUM-4)) return 0;
	clk = ClockFreqTab[clk];

	// set reference frequency in kHz * 4 (frequency counter is clocked by CLK_REF reference clock)
	fc_hw->ref_khz = CurrentFreq[CLK_REF]/250; // 20 bits, max. 1'048'575 kHz

	// set limits (25 bits, max. 33'554'431 kHz)
	fc_hw->min_khz = 0;
	fc_hw->max_khz = 0x1ffffff;

	// initial delay in number of reference clock periods
	fc_hw->delay = 3;

	// length of test interval (0..15 gives interval 2^0..2^15 reference ticks = 1 us ... 32 ms)
	fc_hw->interval = 15;

	// select source clock and start measurement
	fc_hw->src = clk;

	// wait to end of measurement
	while ((fc_hw->status & B4) == 0) {} // check DONE flag

	// get result in kHz
	u32 freq = (fc_hw->result * 125 + 4) >> 4;

	// stop frequency counter
	fc_hw->src = 0;

	return freq;
}

// fast measure frequency of clock CLK_REF..CLK_GPIN1 with result in kHz (returns 0 on error)
//  - measure interval: 2 ms, resolution +-1 kHz
u32 FreqCountkHz(int clk)
{
	// convert to internal clock index
	clk -= 4;
	if ((u32)clk >= (u32)(CLK_SRC_NUM-4)) return 0;
	clk = ClockFreqTab[clk];

	// set reference frequency in kHz * 4 (frequency counter is clocked by CLK_REF reference clock)
	fc_hw->ref_khz = CurrentFreq[CLK_REF]/1000; // 20 bits, max. 1'048'575 kHz

	// set limits (25 bits, max. 33'554'431 kHz)
	fc_hw->min_khz = 0;
	fc_hw->max_khz = 0x1ffffff;

	// initial delay in number of reference clock periods
	fc_hw->delay = 3;

	// length of test interval (0..15 gives interval 2^0..2^15 reference ticks = 1 us ... 32 ms)
	fc_hw->interval = 11;

	// select source clock and start measurement
	fc_hw->src = clk;

	// wait to end of measurement
	while ((fc_hw->status & B4) == 0) {} // check DONE flag

	// get result in kHz
	u32 freq = (fc_hw->result + 4) >> 5;

	// stop frequency counter
	fc_hw->src = 0;

	return freq;
}

// resus default handler
void ClockResusDefHandler(void)
{
	// check interrupt reason
	u32 ints = clocks_hw->ints;
	if ((ints & B0) != 0)
	{
		// set to reference clock (clk_sys is forced to clk_ref by resus)
		ClockSetup(CLK_SYS, CLK_REF, 0, 0);

		// remove resus flag
		RegSet(&clocks_hw->resus.ctrl, B16);
		RegClr(&clocks_hw->resus.ctrl, B16);

		// call user handler
		if (ResusCallback != NULL) ResusCallback();
	}
}

// enable resus function with callback handler (NULL=not used)
// - Resus event come when clk_sys is stopped
void ClockResusEnable(resus_callback_t cb)
{
	// save callback handler
	ResusCallback = cb;

	// set default handler
	SetHandler(IRQ_CLOCKS, ClockResusDefHandler);

	// setup resus interval and enable resus
	//   timeout cycles >= 2* clk_ref / clk_sys_min (clk_ref = 3 MHz, clk_sys_min = 1 MHz)
	clocks_hw->resus.ctrl = B8 + 10;

	// enable resus interrupt
	clocks_hw->inte = B0;

	// enable clock IRQ
	NVIC_IRQEnable(IRQ_CLOCKS);
}

// disable resus function
void ClockResusDisable()
{
	// disable resus (set to reset value)
	clocks_hw->resus.ctrl = 0xff;

	// disable resus interrupt
	clocks_hw->inte = 0;

	// disable clock IRQ
	NVIC_IRQDisable(IRQ_CLOCKS);
}

// enable divided clock to GPIO pin, set divider (returns new frequency in Hz or 0 on error)
//  gpio ... GPIO pin 21, 23, 24 or 25 (only GPIO21 is available on the Pico board) (RP2350: 13, 15, 21, 23, 24 or 25)
//  clksrc ... clock source CLK_REF..CLK_GPIN1 (see table which sources are supported)
//  clkdiv ... clock_divider * 256 (default 0x100, means clock_divider=1.00)
u32 ClockGpoutDiv(int gpio, int clksrc, u32 clkdiv)
{
	// prepare clock index
	int clk;
	if      (gpio == 21) clk = CLK_GPOUT0;
	else if (gpio == 23) clk = CLK_GPOUT1;
	else if (gpio == 24) clk = CLK_GPOUT2;
	else if (gpio == 25) clk = CLK_GPOUT3;
#if !PICO_RP2040
	else if (gpio == 13) clk = CLK_GPOUT0;
	else if (gpio == 15) clk = CLK_GPOUT1;
#endif
	else return 0;

	// convert clock source to internal source index
	clksrc -= 4; // CLK_GPOUTn as clock source not supported
	if ((u32)clksrc >= (u32)(CLK_SRC_NUM-4)) return 0; // invalid clock source index
	int i = ClockSourceTab[clksrc]; // get internal source index
	if (i < 0) return 0; // invalid clock source

	// save current clock source
	clksrc += 4;
	CurrentClkSrc[clk] = (u8)(clksrc);

	// increasing divisor, so set divisor before source (to avoid overspeed)
	if (clkdiv < BIT(CLK_DIV_FRAC_BITS)) clkdiv = BIT(CLK_DIV_FRAC_BITS); // minimal divider is 1.00
	if (clkdiv > clocks_hw->clk[clk].div) clocks_hw->clk[clk].div = clkdiv;

	// Setup clock generator
	clocks_hw->clk[clk].ctrl = (i << 5) | B11; // select source and enable clock

	// set divisor after selecting mux
	clocks_hw->clk[clk].div = clkdiv;

	// Set gpio pin to gpclock function
	GPIO_Fnc(gpio, GPIO_FNC_GPCK);

	// save new frequency
	u32 freqsrc = CurrentFreq[clksrc]; // source frequency
	u32 freq = (u32)(((u64)freqsrc << CLK_DIV_FRAC_BITS) / clkdiv);
	CurrentFreq[clk] = freq;

	return freq;
}

// enable divided clock to GPIO pin, set frequency (returns new frequency in Hz or 0 on error)
//  gpio ... GPIO pin 21, 23, 24 or 25 (only GPIO21 is available on the Pico board)
//  clksrc ... clock source CLK_REF..CLK_GPIN1 (see table which sources are supported)
//  freq ... required frequency in Hz, 0=use source frequency
u32 ClockGpoutFreq(int gpio, int clksrc, u32 freq)
{
	u32 freqsrc = CurrentFreq[clksrc]; // source frequency
	u32 clkdiv = (u32)((((u64)freqsrc << CLK_DIV_FRAC_BITS) + freq/2) / freq); // divider, including 8-bit fraction (so multiply result by 2^8)
	return ClockGpoutDiv(gpio, clksrc, clkdiv);
}

// disable divided clock to GPIO pin
//  gpio ... GPIO pin 21, 23, 24 or 25 (only GPIO21 is available on the Pico board)
void ClockGpoutDisable(int gpio)
{
	// prepare clock index
	int clk;
	if      (gpio == 21) clk = CLK_GPOUT0;
	else if (gpio == 23) clk = CLK_GPOUT1;
	else if (gpio == 24) clk = CLK_GPOUT2;
	else if (gpio == 25) clk = CLK_GPOUT3;
#if !PICO_RP2040
	else if (gpio == 13) clk = CLK_GPOUT0;
	else if (gpio == 15) clk = CLK_GPOUT1;
#endif
	else return;

	// stop clock generator
	clocks_hw->clk[clk].ctrl = 0;

	// reset GPIO
	GPIO_Fnc(gpio, GPIO_FNC_NULL);
}

// Configure a clock to come from a gpio input (returns new frequency in Hz or 0 on error)
//  clk ... clock line index CLK_GPOUT0..CLK_RTC (CLK_ADC)
//  gpio ... GPIO pin 20 or 22 (RP2350: 12, 14, 20 or 22)
//  freq ... required frequency in Hz
//  freqsrc ... frequency in Hz of source (must be freqsrc >= freq)
u32 ClockGpinSetup(int clk, int gpio, u32 freq, u32 freqsrc)
{
	// clock source index
	int clksrc;
	if      (gpio == 20) clksrc = CLK_GPIN0;
	else if (gpio == 22) clksrc = CLK_GPIN1;
#if PICO_RP2350
	else if (gpio == 12) clksrc = CLK_GPIN0;
	else if (gpio == 14) clksrc = CLK_GPIN1;
#endif
	else return 0;

	// set the GPIO function
	GPIO_Fnc(gpio, GPIO_FNC_GPCK);

	// setup clock
	return ClockSetup(clk, clksrc, freq, freqsrc);
}

#if USE_ORIGSDK		// include interface of original SDK

// Configure the specified clock
bool clock_configure(enum clock_index clk_index, u32 src, u32 auxsrc, u32 src_freq, u32 freq)
{
	u32 div;

	// destination frequency must not be greater than source frequency
	if (freq > src_freq) return false;

	// Div register is 24.8 (or 16.16) int.frac divider so multiply by 2^8 or 2^16 (left shift by 8 or 16)
	div = (u32)(((u64)src_freq << CLK_DIV_FRAC_BITS) / freq);

	// pointer to clock interface
	clock_hw_t *clock = ClockGetHw(clk_index);

	// increasing divisor, so set divisor before source (to avoid overspeed)
	if (div > clock->div) clock->div = div;

	// If swithing glitchless clock (REF or SYS) to aux source,
	// switch away from aux to avoid glitches.
	if (GLITCHLESS_MUX(clk_index) && (src == 1))
	{
		RegClr(&clock->ctrl, 3); // switch to base source 0
		while ((clock->selected & 1) == 0) {} // wait for switching to source 0
	}

	// If no glitchless mux, cleanly stop the clock to avoid glitches
	else
	{
		// stop clock, on REF and SYS clock it does nothing
		RegClr(&clock->ctrl, B11);

		// if clock is running, wait a while
		if (CurrentFreq[clk_index] > 0)
		{
			// wait 3 cycles of target clock to stop
			//  required number of loops = system_clock / current_clock + 1
			uint delay_cyc = CurrentFreq[clk_sys] * 3 / CurrentFreq[clk_index] + 1;

			// delay in assembler
			BusyWaitCycles(delay_cyc);
		}
	}

	// set aux mux
	RegMask(&clock->ctrl, (auxsrc << 5), 0xf << 5);

	// set glitchless mux
	if (GLITCHLESS_MUX(clk_index))
	{
		RegMask(&clock->ctrl, src, 3);
		while ((clock->selected & BIT(src)) == 0) {}
	}

	// start clock, on REF and SYS clock it does nothing
	RegSet(&clock->ctrl, B11);

	// set divisor after selecting mux
	clock->div = div;

	// Store the configured frequency
	CurrentFreq[clk_index] = (u32)(((u64)src_freq << CLK_DIV_FRAC_BITS) / div);

	return true;
}

// Output an optionally divided clock to the specified gpio pin (Valid GPIOs are: 21, 23, 24, 25)
void clock_gpio_init_int_frac(uint gpio, uint src, u32 div_int, u8 div_frac)
{
	// clock index
	uint gpclk;
	if      (gpio == 21) gpclk = clk_gpout0;
	else if (gpio == 23) gpclk = clk_gpout1;
	else if (gpio == 24) gpclk = clk_gpout2;
	else if (gpio == 25) gpclk = clk_gpout3;
#if !PICO_RP2040
	else if (gpio == 13) gpclk = clk_gpout0;
	else if (gpio == 15) gpclk = clk_gpout1;
#endif
	else return;

	// Setup clock generator
	clocks_hw->clk[gpclk].ctrl = (src << 5) | B11; // select source and enable clock
	clocks_hw->clk[gpclk].div = (div_int << CLK_DIV_FRAC_BITS) | div_frac; // set divider

	// Set gpio pin to gpclock function
	gpio_set_function(gpio, GPIO_FUNC_GPCK);
}

// internal index of GPIN0 source for the clock (GPIN1 is GPIN0 + 1)
const u8 gpin0_src[CLK_COUNT] = {
	1,	// CLK_GPOUT0
	1,	// CLK_GPOUT1
	1,	// CLK_GPOUT2
	1,	// CLK_GPOUT3
	1,	// CLK_REF
	4,	// CLK_SYS
	5,	// CLK_PERI
#if !RP2040
	3,	// CLK_HSTX
#endif
	4,	// CLK_USB
	4,	// CLK_ADC
#if RP2040
	4,	// CLK_RTC
#endif
};

// Configure a clock to come from a gpio input (Valid GPIOs are: 20 and 22)
bool clock_configure_gpin(enum clock_index clk_index, uint gpio, u32 src_freq, u32 freq)
{
	// clock index
	uint gpin;
	if      (gpio == 20) gpin = 0;
	else if (gpio == 22) gpin = 1;
#if PICO_RP2350
	else if (gpio == 12) gpin = 0;
	else if (gpio == 14) gpin = 1;
#endif
	else return False;

	// index of auxiliary source
	uint auxsrc = gpin0_src[clk_index] + gpin;

	// glitchless mux?
	uint src = GLITCHLESS_MUX(clk_index) ? 1 : 0;

	// set the GPIO function
	gpio_set_function(gpio, GPIO_FUNC_GPCK);

	// setup clock
	return clock_configure(clk_index, src, auxsrc, src_freq, freq);
}

// Initialise the system clock to 48MHz
void set_sys_clock_48mhz(void)
{
	// Change clk_sys to be 48MHz
	clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
		CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * MHZ, 48 * MHZ);

	// Turn off PLL sys
	pll_deinit(pll_sys);

	// update clk_peri
	clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, 48 * MHZ, 48 * MHZ);
}

// Initialise the system clock
void set_sys_clock_pll(uint32_t vco_freq, uint post_div1, uint post_div2)
{
	// Change clk_sys to be 48MHz
	clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
		CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * MHZ, 48 * MHZ);

	// initialize PLL_SYS
	pll_init(pll_sys, 1, vco_freq, post_div1, post_div2);
	u32 freq = vco_freq / (post_div1 * post_div2);

	// Configure clocks: CLK_REF = XOSC (12MHz) / 1 = 12MHz
	clock_configure(clk_ref, CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC, 0, 12 * MHZ, 12 * MHZ);

	// set CLK SYS = PLL SYS (125MHz) / 1 = 125MHz
	clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
		CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, freq, freq);

	// update clk_peri
	clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * MHZ, 48 * MHZ);
}

// Check if a given system clock frequency is valid/attainable
bool check_sys_clock_khz(uint32_t freq_khz, uint *vco_freq_out, uint *post_div1_out, uint *post_div2_out)
{
	uint crystal_freq_khz = clock_get_hz(clk_ref) / 1000;
	uint fbdiv, postdiv1, postdiv2;
	for (fbdiv = 320; fbdiv >= 16; fbdiv--)
	{
		uint vco = fbdiv * crystal_freq_khz;
		if (vco < PICO_PLL_VCO_MIN_FREQ_MHZ * 1000  || vco > PICO_PLL_VCO_MAX_FREQ_MHZ * 1000) continue;
		for (postdiv1 = 7; postdiv1 >= 1; postdiv1--)
		{
			for (postdiv2 = postdiv1; postdiv2 >= 1; postdiv2--)
			{
				uint out = vco / (postdiv1 * postdiv2);
				if (out == freq_khz && !(vco % (postdiv1 * postdiv2)))
				{
					*vco_freq_out = vco * 1000;
					*post_div1_out = postdiv1;
					*post_div2_out = postdiv2;
					return true;
				}
			}
		}
	}
	return false;
}

// Attempt to set a system clock frequency in khz
bool set_sys_clock_khz(u32 freq_khz, bool required)
{
	uint vco, postdiv1, postdiv2;
	if (check_sys_clock_khz(freq_khz, &vco, &postdiv1, &postdiv2))
	{
		set_sys_clock_pll(vco, postdiv1, postdiv2);
		return true;
	}
	return false;
}

#endif // USE_ORIGSDK
