
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

	// get clock divider * 16
	u32 clkdiv = (u32)(((u64)clksys*16 + freq/2) / freq) - 1;

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

	// get clock divider * 16
	u32 clkdiv = (u32)(((u64)clksys*16 + freq/2) / freq) - 1;

	// limit
	if (clkdiv > 0xfff) clkdiv = 0xfff;
	if (clkdiv < 0x010) clkdiv = 0x010;

	// set clock divider
	PWM_ClkDiv_hw(hw, clkdiv);
}

#endif // USE_PWM	// use PWM (sdk_pwm.c, sdk_pwm.h)
