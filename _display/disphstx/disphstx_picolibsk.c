
// ****************************************************************************
//
//  Reduced functions of the PicoLibSDK library if the original Raspberry SDK library PicoSDK is used
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

// Note: The following 2 switches are not yet defined in the PicoLibSDK at this point, so the global.h file is included.
#if USE_DISPHSTX && DISPHSTX_PICOSDK	// 0=use PicoLibSDK library, 1=use PicoSDK original Raspberry SDK library
#include "disphstx_picolibsk.h"
#else
#include "../../global.h"
#endif

#if USE_DISPHSTX && DISPHSTX_PICOSDK

#include "disphstx.h"

// set GPIO function GPIO_FNC_* (pin = 0..29 or 0..47)
// Resets pad overrides, enables input and output.
// To setup GPIO:
//  - reset GPIO to default state with GPIO_Reset()
//  - setup output level, output enable, pull-up/pull-down, schmitt, slew, drive strength
//  - set GPIO function
//  - if needed, disable input (needed for ADC) and setup overrides
void GPIO_Fnc(int pin, int fnc)
{
	// enable input and output
	io32* pad = GPIO_PadHw(pin);	// get pad control interface
	GPIO_OutEnable_hw(pad);		// output enable
	GPIO_InEnable_hw(pad);		// input enable
	*GPIO_CTRL(pin) = fnc;		// set function, reset all overrides
	GPIO_IsolationDisable_hw(pad);	// pad isolation disable
}

// set GPIO function GPIO_FNC_* with mask (bit '1' to set function of this pin)
// To use pin mask in range (first..last), use function RangeMask.
void GPIO_FncMask(gpio_mask_t mask, int fnc)
{
	int i;
	for (i = 0; i < GPIO_PIN_NUM; i++)
	{
		if ((mask & 1) != 0) GPIO_Fnc(i, fnc);
		mask >>= 1;
	}
}

// reset GPIO pins masked (return to reset state)
// To use pin mask in range (first..last), use function RangeMask.
void GPIO_ResetMask(gpio_mask_t mask)
{
	int i;
	for (i = 0; i < GPIO_PIN_NUM; i++)
	{
		if ((mask & 1) != 0) GPIO_Reset(i);
		mask >>= 1;
	}
}

// initialize GPIO pin base function masked (bit '1' to initialize this pin)
// To use pin mask in range (first..last), use function RangeMask.
void GPIO_InitMask(gpio_mask_t mask)
{
	GPIO_ResetMask(mask);		// reset pins
	GPIO_DirInMask(mask);		// disable output
	GPIO_ClrMask(mask);		// set output to LOW
	GPIO_FncMask(mask, GPIO_FNC_SIO); // set function, reset overrides, reset pad setup
}

// set DMA config, without trigger
//  dma = channel 0..11 (or 0..15)
//  src = source address
//  dst = destination address
//  count = number of transfers (can be combined with flags DMA_COUNT_TRIGGER or DMA_COUNT_ENDLESS)
//  ctrl = control word (can be prepared using DMA_Cfg* functions)
void DMA_Config(int dma, const volatile void* src, volatile void* dst, u32 count, u32 ctrl)
{
	DMA_SetRead(dma, src);
	DMA_SetWrite(dma, dst);
	DMA_SetCount(dma, count);
	DMA_SetCtrl(dma, ctrl);
}

// get recommended voltage by system clock in kHz (return VREG_VOLTAGE_1_10 .. VREG_VOLTAGE_1_30)
int GetVoltageBySysClock(u32 freq)
{
#if USE_VREG_LOCKED		// 1=enable vreg locked values > 1.30V from function GetVoltageBySysClock() of RP2350
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

// get recommended flash divider by system clock in kHz
int GetClkDivBySysClock(u32 freq)
{
	if (freq >= 500000) return 6;
	if (freq >= 400000) return 5;
	if (freq >= 300000) return 4;
	if (freq >= 200000) return 3;
	return 2;
}

// wait for regulated state
void VregWait(void)
{
	while (!VregIsOk()) {}
}

// set voltage VREG_VOLTAGE_*
void VregSetVoltage(int vreg)
{
	// enable VREG setup
	PowMan_SetBits(&powman_hw->vreg_ctrl, B13); // UNLOCK

	// unlock high voltages
	if ((vreg & VREG_VOLTAGE_UNLOCK) != 0)
		PowMan_SetBits(&powman_hw->vreg_ctrl, B8); // UNLOCK
	else
		PowMan_ClrBits(&powman_hw->vreg_ctrl, B8); // LOCK
	vreg &= 0x1f;

	// wait prior change
	VregWait();

	while ((powman_hw->vreg & B15) != 0) {} // UPDATE_IN_PROGRESS

	// set new value
	PowMan_Write(&powman_hw->vreg, (vreg << 4) | (powman_hw->vreg & B1));

	// wait update in progress
	VregWait();
}

// PLL setup (returns result frequency in Hz)
//   pll ... PLL select, PLL_SYS or PLL_USB
//   refdiv ... divide input reference clock, 1..63 (minimum reference frequency XOSC/refdiv is 5 MHz)
//   fbdiv ... feedback divisor, 16..320
//   div1 ... post divider 1, 1..7
//   div2 ... post divider 2, 1..7 (should be div1 >= div2, but auto-corrected)
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
	u32 ref = 12000000 / refdiv;

	// get VCO frequency
	u32 vco = fbdiv * ref;

	// result frequency
	u32 freq = vco / (div1*div2);

	// reset PLL
	ResetPeriphery(RESET_PLL_SYS+pll);

	// get harwdare interface
	pll_hw_t* hw = PllGetHw(pll);

	// load dividers
	hw->cs = refdiv; // set reference divider
	hw->fbdiv_int = fbdiv; // set feedback divisor

	// turn on PLL power of VCO oscillator
	hw_clear_bits(&hw->pwr, B0|B5); // power up PLL and VCO

	// wait for PLL to lock
	while ((hw->cs & B31) == 0) {}

	// set up post dividers
	hw->prim = ((u32)div1 << 16) | ((u32)div2 << 12);

	// turn on PLL power of post dividers
	hw_clear_bits(&hw->pwr, B3);

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
	// find PLL setup
	return PllCalc(reqkhz, 12000, PLL_VCO_MIN*1000, PLL_VCO_MAX*1000, False,
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

// set system clock PLL to new frequency in kHz and auto-set system voltage and flash divider (dependent clocks are not updated)
//   freq ... required frequency in [kHz]
void ClockPllSysFreqVolt(u32 freq)
{
	// get current frequency in kHz
	u32 old = clock_get_hz(clk_sys)/1000;

	// check current frequency
	if (freq == old) return;

	// temporary reconnect CLK_SYS to PLL_USB
	clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX, CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48000000, 48000000);

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
	clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX, CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, freq*1000, freq*1000);

	// re-init peripheral clock
	u32 f = clock_get_hz(clk_sys);
	if (f < 200*MHZ)
		clock_configure_undivided(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, f);
	else
		clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, f, SYS_CLK_HZ);
}

#endif // USE_DISPHSTX && DISPHSTX_PICOSDK
