
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

// CPU has 8 PWM slices, each slice has 2 output channels (A or B).
// Same PWM output channel can be selected on two GPIO pins.
// If two PWM B pins are used as input, multiply GPIO pins will be OR of those inputs.

#if USE_PWM	// use PWM (sdk_pwm.c, sdk_pwm.h)

#ifndef _SDK_PWM_H
#define _SDK_PWM_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "../sdk_dreq.h"
#include "sdk_gpio.h"
#include "sdk_irq.h"

#if USE_ORIGSDK		// include interface of original SDK
#include "orig/orig_pwm.h"		// constants of original SDK
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

#define PWM_NUM			8		// number of PWM slices
#define NUM_PWM_SLICES		PWM_NUM

// PWM hardware registers (pwm = 0 to 7)
//#define PWM_BASE		0x40050000	// PWM Pulse Width Modulator
#define PWM(pwm)	(PWM_BASE + (pwm)*20)	// PWM base (pwm = 0 to 7)

#define PWM_CSR(pwm)	((volatile u32*)(PWM(pwm)+0)) // control and status register
#define PWM_DIV(pwm)	((volatile u32*)(PWM(pwm)+4)) // divider
#define PWM_CTR(pwm)	((volatile u32*)(PWM(pwm)+8)) // counter
#define PWM_CC(pwm)	((volatile u32*)(PWM(pwm)+12)) // counter compare
#define PWM_TOP(pwm)	((volatile u32*)(PWM(pwm)+16)) // counter wrap

#define PWM_EN		((volatile u32*)(PWM_BASE+0xa0)) // enables
#define PWM_INTR	((volatile u32*)(PWM_BASE+0xa4)) // raw interrups
#define PWM_INTE	((volatile u32*)(PWM_BASE+0xa8)) // interrupt enable
#define PWM_INTF	((volatile u32*)(PWM_BASE+0xac)) // interrupt force
#define PWM_INTS	((volatile u32*)(PWM_BASE+0xb0)) // interrupt status

// PWM slice hardware interface
typedef struct pwm_slice_hw {
	io32	csr;	// 0x00: Control and status register
	io32	div;	// 0x04: INT and FRAC form a fixed-point fractional number
	io32	ctr;	// 0x08: Direct access to the PWM counter
	io32	cc;	// 0x0C: Counter compare values
	io32	top;	// 0x10: Counter wrap value
} pwm_slice_hw_t;

STATIC_ASSERT(sizeof(pwm_slice_hw_t) == 0x14, "Incorrect pwm_slice_hw_t!");

// PWM hardware interface
typedef struct {
	pwm_slice_hw_t slice[PWM_NUM]; // 0x00: (8*20=160=0xA0) slices
	io32	en;	// 0xA0: This register aliases the CSR_EN bits for all channels
	io32	intr;	// 0xA4: Raw Interrupts
	io32	inte;	// 0xA8: Interrupt Enable
	io32	intf;	// 0xAC: Interrupt Force
	io32	ints;	// 0xB0: Interrupt status after masking & forcing
} pwm_hw_t;

#define pwm_hw ((pwm_hw_t*)PWM_BASE)

STATIC_ASSERT(sizeof(pwm_hw_t) == 0xB4, "Incorrect pwm_hw_t!");

// PWM slice configuration
typedef struct {
	u32	csr;
	u32	div;
	u32	top;	// counter wrap
} pwm_config;

// clkdiv mode
#define PWM_DIV_FREE	0	// free-running at rate of divider
#define PWM_DIV_HIGH	1	// divider gated by PWM B pin
#define PWM_DIV_RISE	2	// divider advances with rising edge of PWM B pin
#define PWM_DIV_FALL	3	// divider advances with falling edge of PWM B pin

// output channels
#define PWM_CHAN_A	0	// output channel A
#define PWM_CHAN_B	1	// output channel B

// reset PWM slice to default values
void PWM_Reset(int pwm);

// convert GPIO pin to PWM output channels PWM_CHAN_* (returns 0=A or 1=B)
INLINE u8 PWM_GpioToChan(int gpio) { return (u8)(gpio & 1); }
#define PWM_GPIOTOCHAN(gpio) ((gpio)&1)

// convert GPIO pin to PWM slice (returns 0 to 7)
INLINE u8 PWM_GpioToSlice(int gpio) { return (u8)((gpio >> 1) & 7); }
#define PWM_GPIOTOSLICE(gpio) (((gpio)>>1)&7)

// get PWM slice hardware interface from PWM slice index
INLINE pwm_slice_hw_t* PWM_GetHw(int pwm) { return &pwm_hw->slice[pwm]; }

// get PWM slice index from PWM slice hardware interface
INLINE u8 PWM_GetInx(const pwm_slice_hw_t* hw) { return (u8)(hw - pwm_hw->slice); }

// get DREQ
INLINE u8 PWM_GetDreq(int pwm) { return (u8)(DREQ_PWM_WRAP0 + pwm); }

// === PWM configuration structure (pwm_config)

// get default configuration structure
INLINE pwm_config PWM_CfgDef(void)
{
	pwm_config c = {
		0,	// csr: disable, no phase-correct, no invert, free running
		0x010,	// div: divider 1.00
		0xffff	// top: counter wrap = 0xffff
	};
	return c;
}

// apply PWM configuration
void PWM_Cfg_hw(pwm_slice_hw_t* hw, const pwm_config* cfg, Bool start);
INLINE void PWM_Cfg(int pwm, const pwm_config* cfg, Bool start) { PWM_Cfg_hw(PWM_GetHw(pwm), cfg, start); }

// config enable phase-correct modulation
INLINE void PWM_CfgPhaseCorrect(pwm_config* cfg) { cfg->csr |= B1; }

// config enable trailing-edge modulation (disable phase-correct modulation)
INLINE void PWM_CfgTrailingEdge(pwm_config* cfg) { cfg->csr &= ~B1; }

// config inverting output channel enable (chan is output channel PWM_CHAN_*)
INLINE void PWM_CfgInvEnable(pwm_config* cfg, int chan) { cfg->csr |= B2 << chan; }

// config inverting output channel disable (chan is output channel PWM_CHAN_*)
INLINE void PWM_CfgInvDisable(pwm_config* cfg, int chan) { cfg->csr &= ~(B2 << chan); }

// config set divider mode PWM_DIV_* (default free-running)
INLINE void PWM_CfgDivMode(pwm_config* cfg, int divmode) { cfg->csr = (cfg->csr & ~(B4+B5)) | ((u32)divmode << 4); }

// config set clock divider for free-running mode
//  clkdiv ... clock divider * 16, value 0..0xfff (lower 4 bits are fractional part, upper 8 bits are integer part)
INLINE void PWM_CfgClkDiv(pwm_config* cfg, int clkdiv) { cfg->div = clkdiv; }
INLINE void PWM_CfgClkDivFloat(pwm_config* cfg, float clkdiv) { cfg->div = (u32)(clkdiv*16 + 0.5f); }

// config set counter top wrap value (counter period = top + 1)
INLINE void PWM_CfgTop(pwm_config* cfg, u16 top) { cfg->top = top; }

// === PWM setup

// initialize GPIO to use as PWM pin
INLINE void PWM_GpioInit(int gpio) { GPIO_Fnc(gpio, GPIO_FNC_PWM); }

// enable PWM slice
INLINE void PWM_Enable(int pwm) { RegSet(PWM_CSR(pwm), B0); }
INLINE void PWM_Enable_hw(pwm_slice_hw_t* hw) { RegSet(&hw->csr, B0); }

// enable PWM slices by mask
INLINE void PWM_EnableMask(int mask) { RegSet(&pwm_hw->en, mask); }

// disable PWM slice
INLINE void PWM_Disable(int pwm) { RegClr(PWM_CSR(pwm), B0); }
INLINE void PWM_Disable_hw(pwm_slice_hw_t* hw) { RegClr(&hw->csr, B0); }

// disable PWM slices by mask
INLINE void PWM_DisableMask(int mask) { RegClr(&pwm_hw->en, mask); }

// enable phase-correct modulation
INLINE void PWM_PhaseCorrect(int pwm) { RegSet(PWM_CSR(pwm), B1); }
INLINE void PWM_PhaseCorrect_hw(pwm_slice_hw_t* hw) { RegSet(&hw->csr, B1); }

// enable trailing-edge modulation (disable phase-correct modulation)
INLINE void PWM_TrailingEdge(int pwm) { RegClr(PWM_CSR(pwm), B1); }
INLINE void PWM_TrailingEdge_hw(pwm_slice_hw_t* hw) { RegClr(&hw->csr, B1); }

// inverting output channel enable (chan is output channel PWM_CHAN_*)
INLINE void PWM_InvEnable(int pwm, int chan) { RegSet(PWM_CSR(pwm), B2 << chan); }
INLINE void PWM_InvEnable_hw(pwm_slice_hw_t* hw, int chan) { RegSet(&hw->csr, B2 << chan); }

// inverting output channel disable (chan is output channel PWM_CHAN_*)
INLINE void PWM_InvDisable(int pwm, int chan) { RegClr(PWM_CSR(pwm), B2 << chan); }
INLINE void PWM_InvDisable_hw(pwm_slice_hw_t* hw, int chan) { RegClr(&hw->csr, B2 << chan); }

// set divider mode PWM_DIV_* (default free-running)
INLINE void PWM_DivMode(int pwm, int divmode) { RegMask(PWM_CSR(pwm), (u32)divmode << 4, B4+B5); }
INLINE void PWM_DivMode_hw(pwm_slice_hw_t* hw, int divmode) { RegMask(&hw->csr, (u32)divmode << 4, B4+B5); }

// retard phase of counter by 1 (must be running)
INLINE void PWM_Retard(int pwm) { RegSet(PWM_CSR(pwm), B6); }
INLINE void PWM_Retard_hw(pwm_slice_hw_t* hw) { RegSet(&hw->csr, B6); }

// retard phase of counter by 1 and wait to complete (must be running)
INLINE void PWM_RetardWait(int pwm) { RegSet(PWM_CSR(pwm), B6); while ((*PWM_CSR(pwm) & B6) != 0) {} }
INLINE void PWM_RetardWait_hw(pwm_slice_hw_t* hw) { RegSet(&hw->csr, B6); while ((hw->csr & B6) != 0) {} }

// advance phase of counter by 1 (must be running)
INLINE void PWM_Advance(int pwm) { RegSet(PWM_CSR(pwm), B7); }
INLINE void PWM_Advance_hw(pwm_slice_hw_t* hw) { RegSet(&hw->csr, B7); }

// advance phase of counter by 1 and wait to complete (must be running)
INLINE void PWM_AdvanceWait(int pwm) { RegSet(PWM_CSR(pwm), B7); while ((*PWM_CSR(pwm) & B7) != 0) {} }
INLINE void PWM_AdvanceWait_hw(pwm_slice_hw_t* hw) { RegSet(&hw->csr, B7); while ((hw->csr & B7) != 0) {} }

// set clock divider for free-running mode
//  clkdiv ... clock divider * 16, value 0..0xfff (lower 4 bits are fractional part, upper 8 bits are integer part)
INLINE void PWM_ClkDiv(int pwm, int clkdiv) { *PWM_DIV(pwm) = clkdiv; }
INLINE void PWM_ClkDiv_hw(pwm_slice_hw_t* hw, int clkdiv) { hw->div = clkdiv; }

INLINE void PWM_ClkDivFloat(int pwm, float clkdiv) { *PWM_DIV(pwm) = (u32)(clkdiv*16 + 0.5f); }
INLINE void PWM_ClkDivFloat_hw(pwm_slice_hw_t* hw, float clkdiv) { hw->div = (u32)(clkdiv*16 + 0.5f); }

// set clock counter (16-bit value)
INLINE void PWM_Count(int pwm, u16 cnt) { *PWM_CTR(pwm) = cnt; }
INLINE void PWM_Count_hw(pwm_slice_hw_t* hw, u16 cnt) { hw->ctr = cnt; }

// get clock counter
INLINE u16 PWM_GetCount(int pwm) { return (u16)(*PWM_CTR(pwm) & 0xffff); }
INLINE u16 PWM_GetCount_hw(pwm_slice_hw_t* hw) { return (u16)(hw->ctr & 0xffff); }

// set compare value of output channel PWM_CHAN_* (cmp is 16-bit value)
INLINE void PWM_Comp(int pwm, int chan, u16 cmp) { RegMask(PWM_CC(pwm), (u32)cmp << (chan*16), 0xffff << (chan*16)); }
INLINE void PWM_Comp_hw(pwm_slice_hw_t* hw, int chan, u16 cmp) { RegMask(&hw->cc, (u32)cmp << (chan*16), 0xffff << (chan*16)); }

// get compare value of output channel PWM_CHAN_*
INLINE u16 PWM_GetComp(int pwm, int chan) { return (u16)(*PWM_CC(pwm) >> (chan*16)); }
INLINE u16 PWM_GetComp_hw(const pwm_slice_hw_t* hw, int chan) { return (u16)(hw->cc >> (chan*16)); }

// set counter top wrap value (counter period = top + 1)
INLINE void PWM_Top(int pwm, u16 top) { *PWM_TOP(pwm) = top; }
INLINE void PWM_Top_hw(pwm_slice_hw_t* hw, u16 top) { hw->top = top; }

// get counter top wrap value
INLINE u16 PWM_GetTop(int pwm) { return (u16)*PWM_TOP(pwm); }
INLINE u16 PWM_GetTop_hw(const pwm_slice_hw_t* hw) { return (u16)hw->top; }

// set clock frequency in Hz for free-running mode
//  freq ... frequency, for clk_sys = 125 MHz in range 488 kHz to 125 MHz
//     (for TOP=256 sampling rate 1.9 kHz to 488 kHz)
void PWM_Clock(int pwm, u32 freq);
void PWM_Clock_hw(pwm_slice_hw_t* hw, u32 freq);

// === PWM interrupt

// get raw interrupt flag (not masked by EN)
INLINE Bool PWM_IntRaw(int pwm) { return ((pwm_hw->intr >> pwm) & B0) != 0; }

// clear interrupt flag
INLINE void PWM_IntClear(int pwm) { pwm_hw->intr = BIT(pwm); }

// enable interrupt
INLINE void PWM_IntEnable(int pwm) { RegSet(&pwm_hw->inte, BIT(pwm)); }

// disable interrupt
INLINE void PWM_IntDisable(int pwm) { RegClr(&pwm_hw->inte, BIT(pwm)); }

// force interrupt
INLINE void PWM_IntForce(int pwm) { RegSet(&pwm_hw->intf, BIT(pwm)); }

// unforce interrupt
INLINE void PWM_IntUnforce(int pwm) { RegClr(&pwm_hw->intf, BIT(pwm)); }

// check if interrupt is forced
INLINE Bool PWM_IntIsForced(int pwm) { return ((pwm_hw->intf >> pwm) & B0) != 0; }

// get interrupt status (masked by EN)
INLINE Bool PWM_IntState(int pwm) { return ((pwm_hw->ints >> pwm) & B0) != 0; }

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// PWM Divider mode settings
enum pwm_clkdiv_mode
{
	PWM_DIV_FREE_RUNNING = 0,	// Free-running counting at rate dictated by fractional divider
	PWM_DIV_B_HIGH = 1,		// Fractional divider is gated by the PWM B pin
	PWM_DIV_B_RISING = 2,		// Fractional divider advances with each rising edge of the PWM B pin
	PWM_DIV_B_FALLING = 3		// Fractional divider advances with each falling edge of the PWM B pin
};

// PWM output channel
#undef PWM_CHAN_A
#undef PWM_CHAN_B
enum pwm_chan
{
	PWM_CHAN_A = 0,
	PWM_CHAN_B = 1
};

// Determine the PWM slice that is attached to the specified GPIO
INLINE uint pwm_gpio_to_slice_num(uint gpio) { return PWM_GpioToSlice(gpio); }

// Determine the PWM channel that is attached to the specified GPIO.
INLINE uint pwm_gpio_to_channel(uint gpio) { return PWM_GpioToChan(gpio); }

// Set phase correction in a PWM configuration
INLINE void pwm_config_set_phase_correct(pwm_config *c, bool phase_correct)
	{ if (phase_correct) PWM_CfgPhaseCorrect(c); else PWM_CfgTrailingEdge(c); }

// Set PWM clock divider in a PWM configuration
INLINE void pwm_config_set_clkdiv(pwm_config *c, float div) { PWM_CfgClkDivFloat(c, div); }

// Set PWM clock divider in a PWM configuration using an 8:4 fractional value
INLINE void pwm_config_set_clkdiv_int_frac(pwm_config *c, uint integer, u8 fract)
	{ PWM_CfgClkDiv(c, integer*16 + fract); }

// Set PWM clock divider in a PWM configuration
INLINE void pwm_config_set_clkdiv_int(pwm_config *c, uint div) { PWM_CfgClkDiv(c, div*16); }

// Set PWM counting mode in a PWM configuration
INLINE void pwm_config_set_clkdiv_mode(pwm_config *c, enum pwm_clkdiv_mode mode) { PWM_CfgDivMode(c, mode); }

// Set output polarity in a PWM configuration
INLINE void pwm_config_set_output_polarity(pwm_config *c, bool a, bool b)
{
	if (a) PWM_CfgInvEnable(c, PWM_CHAN_A); else PWM_CfgInvDisable(c, PWM_CHAN_A);
	if (b) PWM_CfgInvEnable(c, PWM_CHAN_B); else PWM_CfgInvDisable(c, PWM_CHAN_B);
}

// PWM counter wrap value in a PWM configuration
INLINE void pwm_config_set_wrap(pwm_config *c, u16 wrap) { PWM_CfgTop(c, wrap); }

// Initialise a PWM with settings from a configuration object
INLINE void pwm_init(uint slice_num, pwm_config *c, bool start) { PWM_Cfg(slice_num, c, start); }

// Get a set of default values for PWM configuration
INLINE pwm_config pwm_get_default_config(void) { return PWM_CfgDef(); }

// Set the current PWM counter wrap value
INLINE void pwm_set_wrap(uint slice_num, u16 wrap) { PWM_Top(slice_num, wrap); }

// Set the current PWM counter compare value for one channel
INLINE void pwm_set_chan_level(uint slice_num, uint chan, u16 level) { PWM_Comp(slice_num, chan, level); }

// Set PWM counter compare values
INLINE void pwm_set_both_levels(uint slice_num, u16 level_a, u16 level_b)
	{ pwm_hw->slice[slice_num].cc = (((uint)level_b) << 16) | (uint)level_a; }

// Helper function to set the PWM level for the slice and channel associated with a GPIO.
INLINE void pwm_set_gpio_level(uint gpio, u16 level)
	{ PWM_Comp(PWM_GpioToSlice(gpio), PWM_GpioToChan(gpio), level); }

// Get PWM counter
INLINE u16 pwm_get_counter(uint slice_num) { return PWM_GetCount(slice_num); }

// Set PWM counter
INLINE void pwm_set_counter(uint slice_num, u16 c) { PWM_Count(slice_num, c); }

// Advance PWM count
INLINE void pwm_advance_count(uint slice_num) { PWM_AdvanceWait(slice_num); }

// Retard PWM count
INLINE void pwm_retard_count(uint slice_num) { PWM_RetardWait(slice_num); }

// Set PWM clock divider using an 8:4 fractional value
INLINE void pwm_set_clkdiv_int_frac(uint slice_num, u8 integer, u8 fract)
	{ PWM_ClkDiv(slice_num, integer*16 + fract); }

// Set PWM clock divider
INLINE void pwm_set_clkdiv(uint slice_num, float divider) { PWM_ClkDivFloat(slice_num, divider); }

// Set PWM output polarity
INLINE void pwm_set_output_polarity(uint slice_num, bool a, bool b)
{
	if (a) PWM_InvEnable(slice_num, PWM_CHAN_A); else PWM_InvDisable(slice_num, PWM_CHAN_A);
	if (b) PWM_InvEnable(slice_num, PWM_CHAN_B); else PWM_InvDisable(slice_num, PWM_CHAN_B);
}

// Set PWM divider mode
INLINE void pwm_set_clkdiv_mode(uint slice_num, enum pwm_clkdiv_mode mode) { PWM_DivMode(slice_num, mode); }

// Set PWM phase correct on/off
INLINE void pwm_set_phase_correct(uint slice_num, bool phase_correct)
	{ if (phase_correct) PWM_PhaseCorrect(slice_num); else PWM_TrailingEdge(slice_num); }

// Enable/Disable PWM
INLINE void pwm_set_enabled(uint slice_num, bool enabled)
	{ if (enabled) PWM_Enable(slice_num); else PWM_Disable(slice_num); }

// Enable/Disable multiple PWM slices simultaneously
INLINE void pwm_set_mask_enabled(u32 mask) { pwm_hw->en = mask; }

// Enable PWM instance interrupt
INLINE void pwm_set_irq_enabled(uint slice_num, bool enabled)
	{ if (enabled) PWM_IntEnable(slice_num); else PWM_IntDisable(slice_num); }

// Enable multiple PWM instance interrupts
INLINE void pwm_set_irq_mask_enabled(uint32_t slice_mask, bool enabled)
	{ if (enabled) RegSet(&pwm_hw->inte, slice_mask); else RegClr(&pwm_hw->inte, slice_mask); }

// Clear a single PWM channel interrupt
INLINE void pwm_clear_irq(uint slice_num) { PWM_IntClear(slice_num); }

// Get PWM interrupt status, raw
INLINE u32 pwm_get_irq_status_mask(void) { return pwm_hw->ints; }

// Force PWM interrupt
INLINE void pwm_force_irq(uint slice_num) { PWM_IntForce(slice_num); }

// Return the DREQ to use for pacing transfers to a particular PWM slice
INLINE uint pwm_get_dreq(uint slice_num) { return PWM_GetDreq(slice_num); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_PWM_H

#endif // USE_PWM	// use PWM (sdk_pwm.c, sdk_pwm.h)
