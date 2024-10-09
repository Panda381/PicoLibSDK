
// ****************************************************************************
//
//                             Reset and power-on
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

#ifndef _SDK_RESET_H
#define _SDK_RESET_H

#include "../sdk_addressmap.h"		// Register address offsets

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_psm.h"	// constants of original SDK
#include "orig_rp2040/orig_resets.h"	// constants of original SDK
#else
#include "orig_rp2350/orig_psm.h"	// constants of original SDK
#include "orig_rp2350/orig_resets.h"	// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// === Power-on state machine (RP2040 datasheet page 191) - use mask of bits BIT(POWER_*)

// power-on hardware registers
#define PSM_ON		((volatile u32*)(PSM_BASE+0)) // force block out of reset (power it on)
#define PSM_OFF		((volatile u32*)(PSM_BASE+4)) // force into reset (power it off)
#define PSM_WDSEL	((volatile u32*)(PSM_BASE+8)) // 1=peripheral should be reset from watchdog
#define PSM_DONE	((volatile u32*)(PSM_BASE+12)) // 1=peripheral's registers are ready to access (read only)

// power-on peripherals
#if RP2040

#define POWER_PROC1	16	// processor core 1
#define POWER_PROC0	15	// processor core 0
#define POWER_SIO	14
#define POWER_VREG	13	// VREG and chip reset
#define POWER_XIP	12	// flash memory interface
#define POWER_SRAM5	11
#define POWER_SRAM4	10
#define POWER_SRAM3	9
#define POWER_SRAM2	8
#define POWER_SRAM1	7
#define POWER_SRAM0	6
#define POWER_ROM	5
#define POWER_BUSFABRIC	4	// bus fabric
#define POWER_RESETS	3
#define POWER_CLOCKS	2
#define POWER_XOSC	1
#define POWER_ROSC	0

// select which peripherals to reset by watchdog (reset everything apart from ROSC and XOSC)
#define PSM_WDSEL_RESETBITS	0x1fffc

#else // RP2350

#define POWER_PROC1	24	// processor core 1
#define POWER_PROC0	23	// processor core 0
#define POWER_ACCESSCTRL 22
#define POWER_SIO	21
#define POWER_XIP	20	// flash memory interface
#define POWER_SRAM9	19
#define POWER_SRAM8	18
#define POWER_SRAM7	17
#define POWER_SRAM6	16
#define POWER_SRAM5	15
#define POWER_SRAM4	14
#define POWER_SRAM3	13
#define POWER_SRAM2	12
#define POWER_SRAM1	11
#define POWER_SRAM0	10
#define POWER_BOOTRAM	9
#define POWER_ROM	8
#define POWER_BUSFABRIC	7	// bus fabric
#define POWER_PSM_READY	6
#define POWER_CLOCKS	5
#define POWER_RESETS	4
#define POWER_XOSC	3
#define POWER_ROSC	2
#define POWER_OTP	1
#define POWER_PROC_COLD	0

// select which peripherals to reset by watchdog (reset everything apart from ROSC and XOSC)
#define PSM_WDSEL_RESETBITS	0x1fffff3

#endif

// power-on hardware interface
typedef struct {
	io32	frce_on;	// 0x00: Force block out of reset
	io32	frce_off;	// 0x04: Force into reset
	io32	wdsel;		// 0x08: Set to 1 if this peripheral should be reset when the watchdog fires
	io32	done;		// 0x0C: R/O Indicates the peripheral's registers are ready to access
} psm_hw_t;

#define psm_hw ((psm_hw_t*)PSM_BASE)

STATIC_ASSERT(sizeof(psm_hw_t) == 0x10, "Incorrect psm_hw_t!");

// === Reset (RP2040 datasheet page 195) - use mask of bits BIT(RESET_*)

// reset hardware registers
#define RESETS_RESET	((volatile u32*)(RESETS_BASE+0)) // 1=peripheral is in reset
#define RESETS_WDSEL	((volatile u32*)(RESETS_BASE+4)) // 1=watchdog will reset this peripheral
#define RESETS_DONE	((volatile u32*)(RESETS_BASE+8)) // 1=peripheral's registers are ready to access (read only)

// reset peripherals (use BIT(RESET_...) to use in the mask)
#if RP2040

#define RESET_USBCTRL	24
#define RESET_UART1	23
#define RESET_UART0	22
#define RESET_TIMER	21
#define RESET_TBMAN	20
#define RESET_SYSINFO	19
#define RESET_SYSCFG	18
#define RESET_SPI1	17
#define RESET_SPI0	16
#define RESET_RTC	15
#define RESET_PWM	14
#define RESET_PLL_USB	13
#define RESET_PLL_SYS	12
#define RESET_PIO1	11
#define RESET_PIO0	10
#define RESET_PADS_QSPI	9
#define RESET_PADS_BANK0 8
#define RESET_JTAG	7
#define RESET_IO_QSPI	6
#define RESET_IO_BANK0	5
#define RESET_I2C1	4
#define RESET_I2C0	3
#define RESET_DMA	2
#define RESET_BUSCTRL	1
#define RESET_ADC	0

#define RESET_ALLBITS	0x01ffffff

#else // RP2350

#define RESET_USBCTRL	28
#define RESET_UART1	27
#define RESET_UART0	26
#define RESET_TRNG	25
#define RESET_TIMER1	24
#define RESET_TIMER0	23
#define RESET_TBMAN	22
#define RESET_SYSINFO	21
#define RESET_SYSCFG	20
#define RESET_SPI1	19
#define RESET_SPI0	18
#define RESET_SHA256	17
#define RESET_PWM	16
#define RESET_PLL_USB	15
#define RESET_PLL_SYS	14
#define RESET_PIO2	13
#define RESET_PIO1	12
#define RESET_PIO0	11
#define RESET_PADS_QSPI	10
#define RESET_PADS_BANK0 9
#define RESET_JTAG	8
#define RESET_IO_QSPI	7
#define RESET_IO_BANK0	6
#define RESET_I2C1	5
#define RESET_I2C0	4
#define RESET_HSTX	3
#define RESET_DMA	2
#define RESET_BUSCTRL	1
#define RESET_ADC	0

#define RESET_ALLBITS	0x01fffffff

#endif

// reset hardware interface
typedef struct {
	io32	reset;		// 0x00: Reset control
	io32	wdsel;		// 0x04: Watchdog select
	io32	reset_done;	// 0x08: Reset done
} resets_hw_t;

#define resets_hw ((resets_hw_t*)RESETS_BASE)

STATIC_ASSERT(sizeof(resets_hw_t) == 0x0C, "Incorrect resets_hw_t!");

// start resetting periphery specified by the mask RESET_*
INLINE void ResetPeripheryOnMask(u32 mask) { RegSet(RESETS_RESET, mask); }
INLINE void ResetPeripheryOn(int peri) { ResetPeripheryOnMask(BIT(peri)); }

// stop resetting periphery specified by the mask RESET_*
INLINE void ResetPeripheryOffMask(u32 mask) { RegClr(RESETS_RESET, mask); }
INLINE void ResetPeripheryOff(int peri) { ResetPeripheryOffMask(BIT(peri)); }

// check if periphery is ready to be accessed after reset
INLINE Bool ResetPeripheryDoneMask(u32 mask) { return (~*RESETS_DONE & mask) == 0; }
INLINE Bool ResetPeripheryDone(int peri) { return ResetPeripheryDoneMask(BIT(peri)); }

// stop resetting periphery specified by the mask RESET_* and wait
void ResetPeripheryOffWaitMask(u32 mask);
INLINE void ResetPeripheryOffWait(int peri) { ResetPeripheryOffWaitMask(BIT(peri)); }

// enable reset periphery by watchdog of mask RESET_*
INLINE void WDPeripheryEnableMask(u32 mask) { RegSet(RESETS_WDSEL, mask); }
INLINE void WDPeripheryEnable(int peri) { WDPeripheryEnableMask(BIT(peri)); }

// disable reset periphery by watchdog of mask RESET_*
INLINE void WDPeripheryDisableMask(u32 mask) { RegClr(RESETS_WDSEL, mask); }
INLINE void WDPeripheryDisable(int peri) { WDPeripheryDisableMask(BIT(peri)); }

// hard reset periphery (and wait to be accessed again)
//  Takes 0.5 us.
void ResetPeripheryMask(u32 mask);
INLINE void ResetPeriphery(int peri) { ResetPeripheryMask(BIT(peri)); }

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// Reset the specified HW blocks
INLINE void reset_block(u32 bits) { hw_set_bits(&resets_hw->reset, bits); }

// bring specified HW blocks out of reset
INLINE void unreset_block(u32 bits) { hw_clear_bits(&resets_hw->reset, bits); }

// Bring specified HW blocks out of reset and wait for completion
INLINE void unreset_block_wait(u32 bits)
{
	hw_clear_bits(&resets_hw->reset, bits);
	while (~resets_hw->reset_done & bits) {}
}

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_RESET_H
