
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

#if USE_PLL	// use PLL phase-locked loop (sdk_pll.c, sdk_pll.h)

#ifndef _SDK_PLL_H
#define _SDK_PLL_H

//#include "resource.h"		// Resources (get frequency XOSC_MHZ 12)
#include "../sdk_addressmap.h"		// Register address offsets

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_pll.h"		// constants of original SDK
#else
#include "orig_rp2350/orig_pll.h"		// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

#define PLL_SYS		0	// system PLL (generate system clock, default 125 MHz, max. 133 MHz)
#define PLL_USB		1	// USB PLL (generate 48 MHz USB reference clock)

#define PLL(pll)	(PLL_SYS_BASE + (pll)*(PLL_USB_BASE - PLL_SYS_BASE))	// PLL base

// PLL hardware registers
#define PLL_CS(pll)	((volatile u32*)(PLL(pll)+0x00)) // control and status
#define PLL_PWR(pll)	((volatile u32*)(PLL(pll)+0x04)) // power control
#define PLL_DIV(pll)	((volatile u32*)(PLL(pll)+0x08)) // feedback divisor (0..4095, but should be 16..320)
#define PLL_PRIM(pll)	((volatile u32*)(PLL(pll)+0x0C)) // post dividers (they should be 1..7)

// PLL hardware interface
typedef struct {
	io32	cs;		// 0x00: Control and Status
	io32	pwr;		// 0x04: Controls the PLL power modes
	io32	fbdiv_int;	// 0x08: Feedback divisor
	io32	prim;		// 0x0C: Controls the PLL post dividers for the primary output
#if !RP2040
	io32	intr;		// 0x10: interrupt raw
	io32	inte;		// 0x14: interrupt enable
	io32	intf;		// 0x18: interrupt force
	io32	ints;		// 0x1C: interrupt state
#endif
} pll_hw_t;

#define pll_sys_hw ((pll_hw_t*)PLL_SYS_BASE)
#define pll_usb_hw ((pll_hw_t*)PLL_USB_BASE)

#define pll_sys pll_sys_hw
#define pll_usb pll_usb_hw

#if RP2040
STATIC_ASSERT(sizeof(pll_hw_t) == 0x10, "Incorrect pll_hw_t!");
#else
STATIC_ASSERT(sizeof(pll_hw_t) == 0x20, "Incorrect pll_hw_t!");
#endif

// ranges
#define PLL_VCO_MIN	400		// VCO min. frequency in MHz
#define PLL_VCO_MAX	1600		// VCO max. frequency in MHz
#define PLL_FBDIV_MIN	16		// min. feedback divisor
#define PLL_FBDIV_MAX	320		// max. feedback divisor
#define PLL_DIV_MIN	1		// min. post divider
#define PLL_DIV_MAX	7		// max. post divider

// PLL output frequency:
//  freq = (XOSC_MHZ / REFDIV) * FBDIV / (DIV1 * DIV2)
//  - minimum reference frequency XOSC_MHZ / REFDIV is 5 MHz
//  - oscillator frequency VOSC = (XOSC_MHZ / REFDIV) * FBDIV must be in range 400 MHz .. 1600 MHz
//      High VCO frequency minimises jittering, low VCO frequency reduces power consumption.
//  - feedback divider FBDIV must be in range 16..320
//  - post dividers DIV1 and DIV2 must be in range 1..7

// get hardware interface from the PLL index
INLINE pll_hw_t* PllGetHw(int pll) { return (pll_hw_t*)PLL(pll); }

// get PLL index from hardware interface
INLINE u8 PllGetInx(const pll_hw_t* hw) { return (hw == pll_sys_hw) ? PLL_SYS : PLL_USB; }

// get PLL refdiv divider (1..63)
INLINE u8 PllGetRefDiv(int pll) { return (u8)(*PLL_CS(pll) & 0x3f); }
INLINE u8 PllGetRefDiv_hw(const pll_hw_t* hw) { return (u8)(hw->cs & 0x3f); }

// get PLL feedback divisor (16..320)
INLINE int PllGetFBDiv(int pll) { return *PLL_DIV(pll) & 0xfff; }
INLINE int PllGetFBDiv_hw(const pll_hw_t* hw) { return hw->fbdiv_int & 0xfff; }

// get PLL post divider 1
INLINE u8 PllGetDiv1(int pll) { return (u8)((*PLL_PRIM(pll) >> 16) & 7); }
INLINE u8 PllGetDiv1_hw(const pll_hw_t* hw) { return (u8)((hw->prim >> 16) & 7); }

// get PLL post divider 2
INLINE u8 PllGetDiv2(int pll) { return (u8)((*PLL_PRIM(pll) >> 12) & 7); }
INLINE u8 PllGetDiv2_hw(const pll_hw_t* hw) { return (u8)((hw->prim >> 12) & 7); }

// get PLL VCO frequency in Hz
u32 PllGetVco(int pll);
u32 PllGetVco_hw(const pll_hw_t* hw);

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
u32 PllSetup(int pll, int refdiv, int fbdiv, int div1, int div2);

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
// Returns true if precise frequency has been found, or near frequency used otherwise.
Bool PllCalc(u32 reqkhz, u32 input, u32 vcomin, u32 vcomax, Bool lowvco,
		u32* outkhz, u32* outvco, u16* outfbdiv, u8* outpd1, u8* outpd2);

// Search PLL setup, using defaults
//  reqkhz ... required output frequency in kHz
// outputs:
//  outkhz ... output achieved frequency in kHz (0=not found)
//  outvco ... output VCO frequency in kHz
//  outfbdiv ... output fbdiv (16..320)
//  outpd1 ... output postdiv1 (1..7)
//  outpd2 ... output postdiv2 (1..7)
// Returns true if precise frequency has been found, or near frequency used otherwise.
Bool PllCalcDef(u32 reqkhz, u32* outkhz, u32* outvco, u16* outfbdiv, u8* outpd1, u8* outpd2);

// setup PLL frequency in kHz (returns result frequency in kHz, or 0 if cannot setup)
// Do not call this function directly, if SYS clock is connected, call the ClockPllSysFreq function.
u32 PllSetFreq(int pll, u32 freq);

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

typedef pll_hw_t *PLL;

#ifndef PICO_PLL_VCO_MIN_FREQ_MHZ
#define PICO_PLL_VCO_MIN_FREQ_MHZ 750
#endif

#ifndef PICO_PLL_VCO_MAX_FREQ_MHZ
#define PICO_PLL_VCO_MAX_FREQ_MHZ 1600
#endif

// Initialise specified PLL.
INLINE void pll_init(PLL pll, uint ref_div, uint vco_freq, uint post_div1, uint post_div2)
{
	u32 ref_mhz = XOSC_MHZ/ref_div;
	int fbdiv = vco_freq/(ref_mhz*MHZ);
	PllSetup(PllGetInx(pll), ref_div, fbdiv, post_div1, post_div2);
}

// Release/uninitialise specified PLL (set all pwr bits to power down)
INLINE void pll_deinit(PLL pll) { pll->pwr = PLL_PWR_BITS; }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_PLL_H

#endif // USE_PLL	// use PLL phase-locked loop (sdk_pll.c, sdk_pll.h)
