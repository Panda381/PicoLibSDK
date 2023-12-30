
// ****************************************************************************
//
//                                    PWM
//                           Pulse Width Modulation
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

#if USE_PWM	// use PWM (sdk_pwm.c, sdk_pwm.h)

#include "../inc/sdk_pll.h"
#include "../inc/sdk_pwm.h"
#include "../inc/sdk_clocks.h"

// reset PWM slice to default values
void PWM_Reset(int pwm)
{
	PWM_IntDisable(pwm);	// interrupt disable
	pwm_slice_hw_t* hw = PWM_GetHw(pwm);
	hw->csr = 0;		// stop PWM
	PWM_IntClear(pwm);	// clear interrupt flag
	hw->div = 0x10;		// divider = 1.0
	hw->ctr = 0;		// reset counter
	hw->cc = 0;		// reset compare values
	hw->top = 0xffff;	// init wrap value
}

// apply PWM configuration
void PWM_Cfg_hw(pwm_slice_hw_t* hw, const pwm_config* cfg, Bool start)
{
	hw->csr = 0;		// stop PWM
	hw->ctr = 0;		// reset counter
	hw->cc = 0;		// reset compare values
	hw->top = cfg->top;	// wrap value
	hw->div = cfg->div;	// divider
	hw->csr = cfg->csr | (start ? B0 : 0); // set control register
}

// set clock frequency in Hz for free-running mode
//  freq ... frequency, for clk_sys = 125 MHz in range 488 kHz to 125 MHz
//     (for TOP=256 sampling rate 1.9 kHz to 488 kHz)
void PWM_Clock(int pwm, u32 freq)
{
	// get system frequency
	u32 clksys = CurrentFreq[CLK_SYS];
	if (freq > clksys) freq = clksys;

	// get clock divider * 16 (calculation is limited to clksys max. 500 MHz)
	freq /= 2;
	u32 clkdiv = (clksys*8 + freq/2) / freq;

	// limit
	if (clkdiv > 0xfff) clkdiv = 0xfff;
	if (clkdiv < 0x010) clkdiv = 0x010;

	// set clock divider
	PWM_ClkDiv(pwm, clkdiv);
}

void PWM_Clock_hw(pwm_slice_hw_t* hw, u32 freq)
{
	// get system frequency
	u32 clksys = CurrentFreq[CLK_SYS];
	if (freq > clksys) freq = clksys;

	// get clock divider * 16 (calculation is limited to clksys max. 500 MHz)
	freq /= 2;
	u32 clkdiv = (clksys*8 + freq/2) / freq;

	// limit
	if (clkdiv > 0xfff) clkdiv = 0xfff;
	if (clkdiv < 0x010) clkdiv = 0x010;

	// set clock divider
	PWM_ClkDiv_hw(hw, clkdiv);
}

// get real frequency of the PWM clock (in Hz)
u32 PWM_GetClock(int pwm)
{
	u32 clkdiv = PWM_GetClkDiv(pwm);
	if (clkdiv < 0x010) clkdiv = 0x010;
	u32 clksys = CurrentFreq[CLK_SYS];
	// calculation is limited to clksys max. 500 MHz
	return ((clksys*8 + clkdiv/2) / clkdiv)*2;
}

u32 PWM_GetClock_hw(pwm_slice_hw_t* hw)
{
	u32 clkdiv = PWM_GetClkDiv_hw(hw);
	if (clkdiv < 0x010) clkdiv = 0x010;
	u32 clksys = CurrentFreq[CLK_SYS];
	// calculation is limited to clksys max. 500 MHz
	return ((clksys*8 + clkdiv/2) / clkdiv)*2;
}

// Find system clock in Hz that sets the most accurate PWM clock frequency.
u32 PWM_FindSysClk(u32 min_hz, u32 max_hz, u32 freq)
{
	u16 fbdiv;
	u32 input, hz, vco, f, clkdiv;
	u8 pd1, pd2;
	int dif;
	int dif_best = 1000000000;
	u32 hz_best = min_hz;

	// get reference frequency in Hz (should be 12 MHz)
	input = CurrentFreq[CLK_XOSC];

	// fbdiv loop
	for (fbdiv = 16; fbdiv <= 320; fbdiv++)
	{
		// get current vco in Hz
		vco = fbdiv * input;

		// check vco range
		if ((vco >= PLL_VCO_MIN*1000000) && (vco <= PLL_VCO_MAX*1000000))
		{
			// pd1 loop
			for (pd1 = 7; pd1 >= 1; pd1--)
			{
				// pd2 loop
				for (pd2 = pd1; pd2 >= 1; pd2--)
				{
					// current output frequency
					hz = vco / (pd1 * pd2);

					// check range
					if ((hz >= min_hz) && (hz <= max_hz))
					{
						// required frequency
						f = freq;
						if (f > hz) f = hz;

						// get clock divider * 16
						f /= 2;
						clkdiv = (hz*8 + f/2) / f;
						if (clkdiv > 0xfff) clkdiv = 0xfff;
						if (clkdiv < 0x010) clkdiv = 0x010;

						// new frequency
						f = ((hz*8 + clkdiv/2) / clkdiv)*2;

						// check best frequency
						dif = f - freq;
						if (dif < 0) dif = -dif;

						// better frequency (if equal distance - lower frequency takes precedence)
						if ((dif < dif_best) || ((dif == dif_best) && (hz < hz_best)))
						{
							dif_best = dif;
							hz_best = hz;
						}
					}
				}
			}
		}
	}

	return hz_best;
}

#endif // USE_PWM	// use PWM (sdk_pwm.c, sdk_pwm.h)
