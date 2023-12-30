
// ****************************************************************************
//
//                            PLL phase-locked loop
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

#if USE_PLL	// use PLL phase-locked loop (sdk_pll.c, sdk_pll.h)

#include "../inc/sdk_pll.h"
#include "../inc/sdk_clocks.h"
#include "../inc/sdk_reset.h"

// get PLL VCO frequency in Hz
u32 PllGetVco(int pll)
{
	return CurrentFreq[CLK_XOSC] / PllGetRefDiv(pll) * PllGetFBDiv(pll);
}

u32 PllGetVco_hw(const pll_hw_t* hw)
{
	return CurrentFreq[CLK_XOSC] / PllGetRefDiv_hw(hw) * PllGetFBDiv_hw(hw);
}

// PLL setup (returns result frequency in Hz)
//   pll ... PLL select, PLL_SYS or PLL_USB
//   refdiv ... divide input reference clock, 1..63 (minimum reference frequency XOSC/refdiv is 5 MHz)
//   fbdiv ... feedback divisor, 16..320
//   div1 ... post divider 1, 1..7
//   div2 ... post divider 2, 1..7 (should be div1 >= div2, but auto-corrected)
// XOSC must be initialized to get its frequency from the table.
// All clocks should be disconnected from the PLL.
// Result frequency = (XOSC / refdiv) * fbdiv / (div1 * div2).
// Do not call this function directly, if SYS clock is connected, call the ClockPllSysSetup function.
u32 PllSetup(int pll, int refdiv, int fbdiv, int div1, int div2)
{
	// correct order of post dividers, should be div1 >= div2
	if (div1 < div2)
	{
		div1 ^= div2;
		div2 ^= div1;
		div1 ^= div2;
	}

	// reference frequency
	u32 ref = CurrentFreq[CLK_XOSC] / refdiv;

	// get VCO frequency
	u32 vco = fbdiv * ref;

	// result frequency
	u32 freq = vco / (div1*div2);
	CurrentFreq[CLK_PLL_SYS+pll] = freq;

	// reset PLL
	ResetPeriphery(RESET_PLL_SYS+pll);

	// get harwdare interface
	pll_hw_t* hw = PllGetHw(pll);

	// load dividers
	hw->cs = refdiv; // set reference divider
	hw->fbdiv_int = fbdiv; // set feedback divisor

	// turn on PLL power of VCO oscillator
	RegClr(&hw->pwr, B0|B5); // power up PLL and VCO

	// wait for PLL to lock
	while ((hw->cs & B31) == 0) {}

	// set up post dividers
	hw->prim = ((u32)div1 << 16) | ((u32)div2 << 12);

	// turn on PLL power of post dividers
	RegClr(&hw->pwr, B3);

	return freq;
}

// Search PLL setup
//  reqkhz ... required output frequency in kHz
//  input ... PLL input reference frequency in kHz (default 12000)
//  vcomin ... minimal VCO frequency in kHz (default 400000)
//  vcomax ... maximal VCO frequency in kHz (default 1600000)
//  lowvco ... prefer low VCO (lower power but more jiter)
// outputs:
//  outkhz ... output achieved frequency in kHz (0=not found)
//  outvco ... output VCO frequency in kHz
//  outfbdiv ... output fbdiv (16..320)
//  outpd1 ... output postdiv1 (1..7)
//  outpd2 ... output postdiv2 (1..7)
// Returns True if precise frequency has been found, or near frequency used otherwise.
Bool PllCalc(u32 reqkhz, u32 input, u32 vcomin, u32 vcomax, Bool lowvco,
		u32* outkhz, u32* outvco, u16* outfbdiv, u8* outpd1, u8* outpd2)
{
	u32 khz, vco;
	s32 margin;
	u16 fbdiv;
	u8 pd1, pd2;
	s32 margin_best = 100000;
	*outkhz = 0;

	// fbdiv loop
	fbdiv = lowvco ? 16 : 320;
	for (;;)
	{
		// get current vco
		vco = fbdiv * input;

		// check vco range
		if ((vco >= vcomin) && (vco <= vcomax))
		{
			// pd1 loop
			for (pd1 = 7; pd1 >= 1; pd1--)
			{
				// pd2 loop
				for (pd2 = pd1; pd2 >= 1; pd2--)
				{
					// current output frequency
					khz = vco / (pd1 * pd2);

					// check best frequency
					margin = (int)(khz - reqkhz);
					if (margin < 0) margin = -margin;
					if (margin < margin_best)
					{
						margin_best = margin;
						*outkhz = khz;
						*outvco = vco;
						*outfbdiv = fbdiv;
						*outpd1 = pd1;
						*outpd2 = pd2;
					}
				}
			}
		}

		// shift fbdiv
		if (lowvco)
		{
			fbdiv++;
			if (fbdiv > 320) break;
		}
		else
		{
			fbdiv--;
			if (fbdiv < 16) break;
		}
	}

	// check precise frequency
	return (*outkhz == reqkhz) && (*outvco == *outkhz * *outpd1 * *outpd2);
}

// Search PLL setup, using defaults
//  reqkhz ... required output frequency in kHz
// outputs:
//  outkhz ... output achieved frequency in kHz (0=not found)
//  outvco ... output VCO frequency in kHz
//  outfbdiv ... output fbdiv (16..320)
//  outpd1 ... output postdiv1 (1..7)
//  outpd2 ... output postdiv2 (1..7)
// Returns true if precise frequency has been found, or near frequency used otherwise.
Bool PllCalcDef(u32 reqkhz, u32* outkhz, u32* outvco, u16* outfbdiv, u8* outpd1, u8* outpd2)
{
	// get reference frequency in kHz (should be 12 MHz)
	u32 input = (CurrentFreq[CLK_XOSC] + 500)/1000;

	// find PLL setup
	return PllCalc(reqkhz, input, PLL_VCO_MIN*1000, PLL_VCO_MAX*1000, False,
		outkhz, outvco, outfbdiv, outpd1, outpd2);
}

// setup PLL frequency in kHz (returns result frequency in kHz, or 0 if cannot setup)
// Do not call this function directly, if SYS clock is connected, call the ClockPllSysFreq function.
u32 PllSetFreq(int pll, u32 freq)
{
	u32 khz, vco;
	u16 fbdiv;
	u8 pd1, pd2;
	PllCalcDef(freq, &khz, &vco, &fbdiv, &pd1, &pd2);
	if (khz == 0) return 0;
	return PllSetup(pll, 1, fbdiv, pd1, pd2) / 1000;
}

#endif // USE_PLL	// use PLL phase-locked loop (sdk_pll.c, sdk_pll.h)
