
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
void PWM_Reset(u8 pwm)
{
	PWM_IntDisable(pwm);
	*PWM_CSR(pwm) = 0;
	PWM_IntClear(pwm);
	*PWM_DIV(pwm) = 0x10;
	*PWM_CTR(pwm) = 0;
	*PWM_CC(pwm) = 0;
	*PWM_TOP(pwm) = 0xffff;
}

// set clock frequency in Hz for free-running mode
//  freq ... frequency, for clk_sys = 125 MHz in range 488 kHz to 125 MHz
//     (for TOP=256 sampling rate 1.9 kHz to 488 kHz)
void PWM_Clock(u8 pwm, u32 freq)
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

#endif // USE_PWM	// use PWM (sdk_pwm.c, sdk_pwm.h)
