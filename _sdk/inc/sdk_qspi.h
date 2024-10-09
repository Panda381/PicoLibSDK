
// ****************************************************************************
//
//                           QSPI flash pins
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

#ifndef _SDK_QSPI_H
#define _SDK_QSPI_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "sdk_cpu.h"
#include "sdk_irq.h"
#include "sdk_sio.h"

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_io_qspi.h"		// constants of original SDK
#include "orig_rp2040/orig_pads_qspi.h"	// constants of original SDK
#else
#include "orig_rp2350/orig_io_qspi.h"		// constants of original SDK
#include "orig_rp2350/orig_pads_qspi.h"	// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// === QSPI pad control (use QSPI_PAD_*)
#define QSPI_PAD_SCLK	0
#define QSPI_PAD_SD0	1
#define QSPI_PAD_SD1	2
#define QSPI_PAD_SD2	3
#define QSPI_PAD_SD3	4
#define QSPI_PAD_SS	5

#define QSPI_PAD_NUM	6		// number of QSPI pads

// pad control hardware registers
#define QSPI_VOLT	((volatile u32*)(PADS_QSPI_BASE+0))	// voltage select
#define QSPI_PAD(pad)	((volatile u32*)(PADS_QSPI_BASE+(pad)*4+4)) // QSPI pad control register, pad = QSPI_PAD_*

// pad control hardware interface
typedef struct {
	io32	voltage_select;		// 0x00: voltage select
	io32	io[QSPI_PAD_NUM];	// 0x04: pad control register
} pads_qspi_hw_t;

#define pads_qspi_hw ((pads_qspi_hw_t*)PADS_QSPI_BASE)

STATIC_ASSERT(sizeof(pads_qspi_hw_t) == 0x1C, "Incorrect pads_qspi_hw_t!");

// === QSPI pin control (use QSPI_PIN_*)

#if RP2040

#define QSPI_PIN_SCLK	0
#define QSPI_PIN_SS	1
#define QSPI_PIN_SD0	2
#define QSPI_PIN_SD1	3
#define QSPI_PIN_SD2	4
#define QSPI_PIN_SD3	5

#define QSPI_PIN_NUM	6		// number of QSPI pins

#else

#define QSPI_PIN_DP	0
#define QSPI_PIN_DM	1
#define QSPI_PIN_SCLK	2
#define QSPI_PIN_SS	3
#define QSPI_PIN_SD0	4
#define QSPI_PIN_SD1	5
#define QSPI_PIN_SD2	6
#define QSPI_PIN_SD3	7

#define QSPI_PIN_NUM	8		// number of QSPI pins

#endif

// pin control hardware registers
#define QSPI_STATUS(pin) ((volatile u32*)(IO_QSPI_BASE+(pin)*8)) // QSPI status (read only), pin = QSPI_PIN_*
#define QSPI_CTRL(pin) ((volatile u32*)(IO_QSPI_BASE+(pin)*8+4)) // QSPI pin control, pin = QSPI_PIN_*

// pin control hardware interface
typedef struct {
	io32	status;		// 0x00: GPIO status
	io32	ctrl;		// 0x04: GPIO control including function select and overrides
} io_qspi_status_ctrl_hw_t;

STATIC_ASSERT(sizeof(io_qspi_status_ctrl_hw_t) == 0x08, "Incorrect io_qspi_status_ctrl_hw_t!");

#define ioqspi_status_ctrl_hw_t io_qspi_status_ctrl_hw_t

// === QSPI interrupt control (use QSPI_PIN_*)

// pin interrupt hardware registers (cpi = 0 or 1, or 2 for dormant wake)
#if RP2040
#define QSPI_IRQ_INTR ((volatile u32*)(IO_QSPI_BASE+0x30)) // raw interrupt of both CPUs
#define QSPI_IRQ_INTE(cpu) ((volatile u32*)(IO_QSPI_BASE+(cpu)*12+0x34)) // interrupt enable
#define QSPI_IRQ_INTF(cpu) ((volatile u32*)(IO_QSPI_BASE+(cpu)*12+0x34+4)) // force interrupt
#define QSPI_IRQ_INTS(cpu) ((volatile u32*)(IO_QSPI_BASE+(cpu)*12+0x34+8)) // interrupt status
#else
#define QSPI_IRQ_INTR ((volatile u32*)(IO_QSPI_BASE+0x218)) // raw interrupt of both CPUs
#define QSPI_IRQ_INTE(cpu) ((volatile u32*)(IO_QSPI_BASE+(cpu)*12+0x21C)) // interrupt enable
#define QSPI_IRQ_INTF(cpu) ((volatile u32*)(IO_QSPI_BASE+(cpu)*12+0x21C+4)) // force interrupt
#define QSPI_IRQ_INTS(cpu) ((volatile u32*)(IO_QSPI_BASE+(cpu)*12+0x21C+8)) // interrupt status
#endif

// pin interrupt hardware interface
typedef struct {
	io32	inte;		// 0x00: Interrupt Enable for proc0
	io32	intf;		// 0x04: Interrupt Force for proc0
	io32	ints;		// 0x08: Interrupt status after masking & forcing for proc0
} io_qspi_irq_ctrl_hw_t;

STATIC_ASSERT(sizeof(io_qspi_irq_ctrl_hw_t) == 0x0C, "Incorrect io_qspi_irq_ctrl_hw_t!");

// pin bank interface
typedef struct {
	io_qspi_status_ctrl_hw_t	io[QSPI_PIN_NUM];	// 0x00: pin control
#if !RP2040
	io32			_pad0[112];				// 0x40:
	io32			irqsummary_proc0_secure;		// 0x200:
	io32			irqsummary_proc0_nonsecure;		// 0x204:
	io32			irqsummary_proc1_secure;		// 0x208:
	io32			irqsummary_proc1_nonsecure;		// 0x20C:
	io32			irqsummary_dormant_wake_secure;		// 0x210:
	io32			irqsummary_dormant_wake_nonsecure;	// 0x214:
#endif
	io32			intr;			// 0x30 (0x218): Raw Interrupts

	union {
		struct {
			io_qspi_irq_ctrl_hw_t	proc0_qspi_ctrl;	// 0x34 (0x21C): interrupt control for proc0
			io_qspi_irq_ctrl_hw_t	proc1_qspi_ctrl;	// 0x40 (0x228): interrupt control for proc1
			io_qspi_irq_ctrl_hw_t	dormant_wake_qspi_ctrl;	// 0x4C (0x234): dormant interrupt control
		};
		io_qspi_irq_ctrl_hw_t		irq_ctrl[3];		// 0x34 (0x21C):
	};
} io_qspi_hw_t;

#define io_qspi_hw ((io_qspi_hw_t*)IO_QSPI_BASE)

#if RP2040
STATIC_ASSERT(sizeof(io_qspi_hw_t) == 0x58, "Incorrect io_qspi_hw_t!");
#else
STATIC_ASSERT(sizeof(io_qspi_hw_t) == 0x240, "Incorrect io_qspi_hw_t!");
#endif

#define ioqspi_hw_t io_qspi_hw_t
#define ioqspi_hw io_qspi_hw

// === QSPI data (use QSPI_PIN_*)

// QSPI pin data registers (bit 0..5 = pin QSPI_PIN_*)
//   (SIO does not support aliases for atomic access!)
#if RP2040

#define QSPI_IN_DATA	((volatile u32*)(SIO_BASE+8))		// QSPI input
#define QSPI_OUT_DATA	((volatile u32*)(SIO_BASE+0x30))	// QSPI output (read: last value written to output)
#define QSPI_OUT_SET	((volatile u32*)(SIO_BASE+0x34))	// QSPI output set
#define QSPI_OUT_CLR	((volatile u32*)(SIO_BASE+0x38))	// QSPI output clear
#define QSPI_OUT_XOR	((volatile u32*)(SIO_BASE+0x3C))	// QSPI output XOR
#define QSPI_OE		((volatile u32*)(SIO_BASE+0x40))	// QSPI output enable (read: last value written)
#define QSPI_OE_SET	((volatile u32*)(SIO_BASE+0x44))	// QSPI output enable set
#define QSPI_OE_CLR	((volatile u32*)(SIO_BASE+0x48))	// QSPI output enable clear
#define QSPI_OE_XOR	((volatile u32*)(SIO_BASE+0x4C))	// QSPI output enable XOR

#define QSPI_DATA_SHIFT	0					// shift data

#else

#define QSPI_IN_DATA	((volatile u32*)(SIO_BASE+8))		// QSPI input
#define QSPI_OUT_DATA	((volatile u32*)(SIO_BASE+0x14))	// QSPI output (read: last value written to output)
#define QSPI_OUT_SET	((volatile u32*)(SIO_BASE+0x1C))	// QSPI output set
#define QSPI_OUT_CLR	((volatile u32*)(SIO_BASE+0x24))	// QSPI output clear
#define QSPI_OUT_XOR	((volatile u32*)(SIO_BASE+0x2C))	// QSPI output XOR
#define QSPI_OE		((volatile u32*)(SIO_BASE+0x34))	// QSPI output enable (read: last value written)
#define QSPI_OE_SET	((volatile u32*)(SIO_BASE+0x3C))	// QSPI output enable set
#define QSPI_OE_CLR	((volatile u32*)(SIO_BASE+0x44))	// QSPI output enable clear
#define QSPI_OE_XOR	((volatile u32*)(SIO_BASE+0x4C))	// QSPI output enable XOR

#define QSPI_DATA_SHIFT	24					// shift data

#endif

// QSPI pin functions
#define QSPI_FNC_XIP	0	// XIP
#define QSPI_FNC_SIO	5	// SIO (GPIO)
#define QSPI_FNC_NULL	31	// no function (default)
#if !RP2040
#define QSPI_FNC_UART	2	// UART
#define QSPI_FNC_I2C	3	// I2C
#define QSPI_FNC_AUX	11	// UART AUX (select TX/RX instead of CTS/RTS)
#endif

// interrupt events
#define QSPI_EVENT_LEVELLOW	B0	// level low
#define QSPI_EVENT_LEVELHIGH	B1	// level high
#define QSPI_EVENT_EDGELOW	B2	// edge low
#define QSPI_EVENT_EDGEHIGH	B3	// edge high

#define QSPI_EVENT_ALL		0x0f	// all events
#define QSPI_EVENT_NONE		0	// no event

// drive strength
#define QSPI_DRIVE_2MA		0
#define QSPI_DRIVE_4MA		1
#define QSPI_DRIVE_8MA		2
#define QSPI_DRIVE_12MA		3

// peripheral signal override
#define QSPI_OVER_NORMAL	0
#define QSPI_OVER_INVERT	1
#define QSPI_OVER_LOW		2
#define QSPI_OVER_HIGH		3
#define QSPI_OVER_MASK		3

// ==== QSPI pad control (use pads QSPI_PAD_* 0..5)
// Warning: pins and pads use different numbering (use predefined constants).

// QSPI set voltage to 3.3V (DVDD >= 2V5; default state)
INLINE void QSPI_Voltage3V3(void) { pads_qspi_hw->voltage_select = 0; }

// QSPI set voltage to 1.8V (DVDD <= 1V8)
INLINE void QSPI_Voltage1V8(void) { pads_qspi_hw->voltage_select = B0; }

// get pointer to pad control interface (pad = QSPI_PAD_* 0..5)
INLINE io32* QSPI_PadHw(int pad) { return &pads_qspi_hw->io[pad]; }

// init pad to default state (pad = QSPI_PAD_* 0..5)
//  - slow slew rate, enable schmitt, pull-up, 4mA, in/out enable (RP2350: enable isolation state)
#if RP2040
INLINE void QSPI_PadInit(int pad) { *QSPI_PAD(pad) = B1|B3|B4|B6; }
INLINE void QSPI_PadInit_hw(io32* hw) { *hw = B1|B3|B4|B6; }
#else
INLINE void QSPI_PadInit(int pad) { *QSPI_PAD(pad) = B1|B3|B4|B6|B8; }
INLINE void QSPI_PadInit_hw(io32* hw) { *hw = B1|B3|B4|B6|B8; }
#endif

#if !RP2040
// Enable isolation latches (default state; pad = QSPI_PAD_* 0..5)
//  If pad isolation is enabled, all output settings of the pad are latched.
//  Disable pad isolation after pad is configured.
INLINE void QSPI_IsolationEnable(int pad) { RegSet(QSPI_PAD(pad), B8); }
INLINE void QSPI_IsolationEnable_hw(io32* hw) { RegSet(hw, B8); }

// Disable isolation latches (pad = QSPI_PAD_* 0..5)
//  If pad isolation is disable, all output settings are transparent to the pad.
INLINE void QSPI_IsolationDisable(int pad) { RegClr(QSPI_PAD(pad), B8); }
INLINE void QSPI_IsolationDisable_hw(io32* hw) { RegClr(hw, B8); }
#endif

// QSPI output enable; disable has priority over QSPI_DirOut (default state; pad = QSPI_PAD_* 0..5)
INLINE void QSPI_OutEnable(int pad) { RegClr(QSPI_PAD(pad), B7); }
INLINE void QSPI_OutEnable_hw(io32* hw) { RegClr(hw, B7); }

// QSPI output disable; disable has priority over QSPI_DirOut (pad = QSPI_PAD_* 0..5)
INLINE void QSPI_OutDisable(int pad) { RegSet(QSPI_PAD(pad), B7); }
INLINE void QSPI_OutDisable_hw(io32* hw) { RegSet(hw, B7); }

// enable pad input (default state; pad = QSPI_PAD_* 0..5)
INLINE void QSPI_InEnable(int pad) { RegSet(QSPI_PAD(pad), B6); }
INLINE void QSPI_InEnable_hw(io32* hw) { RegSet(hw, B6); }

// disable pad input (pad = QSPI_PAD_* 0..5)
INLINE void QSPI_InDisable(int pad) { RegClr(QSPI_PAD(pad), B6); }
INLINE void QSPI_InDisable_hw(io32* hw) { RegClr(hw, B6); }

// set output strength to 2 mA (pad = QSPI_PAD_* 0..5)
INLINE void QSPI_Drive2mA(int pad) { RegClr(QSPI_PAD(pad), B4|B5); }
INLINE void QSPI_Drive2mA_hw(io32* hw) { RegClr(hw, B4|B5); }

// set output strength to 4 mA (default state; pad = QSPI_PAD_* 0..5)
INLINE void QSPI_Drive4mA(int pad) { RegMask(QSPI_PAD(pad), B4, B4|B5); }
INLINE void QSPI_Drive4mA_hw(io32* hw) { RegMask(hw, B4, B4|B5); }

// set output strength to 8 mA (pad = QSPI_PAD_* 0..5)
INLINE void QSPI_Drive8mA(int pad) { RegMask(QSPI_PAD(pad), B5, B4|B5); }
INLINE void QSPI_Drive8mA_hw(io32* hw) { RegMask(hw, B5, B4|B5); }

// set output strength to 12 mA (pad = QSPI_PAD_* 0..5)
INLINE void QSPI_Drive12mA(int pad) { RegSet(QSPI_PAD(pad), B4|B5); }
INLINE void QSPI_Drive12mA_hw(io32* hw) { RegSet(hw, B4|B5); }

// Set drive strength QSPI_DRIVE_* (pad = QSPI_PAD_* 0..5)
INLINE void QSPI_Drive(int pin, int drive) { RegMask(QSPI_PAD(pin), drive << 4, B4|B5); }
INLINE void QSPI_Drive_hw(io32* hw, int drive) { RegMask(hw, drive << 4, B4|B5); }

// set no pulls (pad = QSPI_PAD_* 0..5)
INLINE void QSPI_NoPull(int pad) { RegClr(QSPI_PAD(pad), B2|B3); }
INLINE void QSPI_NoPull_hw(io32* hw) { RegClr(hw, B2|B3); }

// set pull down (default state; pad = QSPI_PAD_* 0..5)
INLINE void QSPI_PullDown(int pad) { RegMask(QSPI_PAD(pad), B2, B2|B3); }
INLINE void QSPI_PullDown_hw(io32* hw) { RegMask(hw, B2, B2|B3); }

// set pull up (pad = QSPI_PAD_* 0..5)
INLINE void QSPI_PullUp(int pad) { RegMask(QSPI_PAD(pad), B3, B2|B3); }
INLINE void QSPI_PullUp_hw(io32* hw) { RegMask(hw, B3, B2|B3); }

// set bus keep (weak pull up and down; pad = QSPI_PAD_* 0..5)
INLINE void QSPI_BusKeep(int pad) { RegSet(QSPI_PAD(pad), B2|B3); }
INLINE void QSPI_BusKeep_hw(io32* hw) { RegSet(hw, B2|B3); }

// enable schmitt trigger (use hysteresis on input; default state; pad = QSPI_PAD_* 0..5)
INLINE void QSPI_SchmittEnable(int pad) { RegSet(QSPI_PAD(pad), B1); }
INLINE void QSPI_SchmittEnable_hw(io32* hw) { RegSet(hw, B1); }

// disable schmitt trigger (do not use hysteresis on input; pad = QSPI_PAD_* 0..5)
INLINE void QSPI_SchmittDisable(int pad) { RegClr(QSPI_PAD(pad), B1); }
INLINE void QSPI_SchmittDisable_hw(io32* hw) { RegClr(hw, B1); }

// use slow slew rate control on output (default state; pad = QSPI_PAD_* 0..5)
INLINE void QSPI_Slow(int pad) { RegClr(QSPI_PAD(pad), B0); }
INLINE void QSPI_Slow_hw(io32* hw) { RegClr(hw, B0); }

// use fast slew rate control on output (pad = QSPI_PAD_* 0..5)
INLINE void QSPI_Fast(int pad) { RegSet(QSPI_PAD(pad), B0); }
INLINE void QSPI_Fast_hw(io32* hw) { RegSet(hw, B0); }

// ==== QSPI pin control (use pins QSPI_PIN_* 0..5 or 0..7)
// Warning: pins and pads use different numbering (use predefined constants).

// get pointer to pin control interface (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE io_qspi_status_ctrl_hw_t* QSPI_PinHw(int pin) { return &io_qspi_hw->io[pin]; }

#if RP2040
// get output (0 or 1) from peripheral, before override (pin = QSPI_PIN_* 0..5)
INLINE u8 QSPI_OutPeri(int pin) { return (u8)((*QSPI_STATUS(pin) >> 8) & 1); }
INLINE u8 QSPI_OutPeri_hw(const io_qspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 8) & 1); }
#endif

// get output (0 or 1) to pad, after override (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE u8 QSPI_OutPad(int pin) { return (u8)((*QSPI_STATUS(pin) >> 9) & 1); }
INLINE u8 QSPI_OutPad_hw(const io_qspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 9) & 1); }

#if RP2040
// get output enable (0 or 1) from peripheral, before override (pin = QSPI_PIN_* 0..5)
INLINE u8 QSPI_OePeri(int pin) { return (u8)((*QSPI_STATUS(pin) >> 12) & 1); }
INLINE u8 QSPI_OePeri_hw(const io_qspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 12) & 1); }
#endif

// get output enable (0 or 1) to pad, after override (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE u8 QSPI_OePad(int pin) { return (u8)((*QSPI_STATUS(pin) >> 13) & 1); }
INLINE u8 QSPI_OePad_hw(const io_qspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 13) & 1); }

// get input (0 or 1) from pad, before override (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE u8 QSPI_InPad(int pin) { return (u8)((*QSPI_STATUS(pin) >> 17) & 1); }
INLINE u8 QSPI_InPad_hw(const io_qspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 17) & 1); }

#if RP2040
// get input (0 or 1) to peripheral, after override (pin = QSPI_PIN_* 0..5)
INLINE u8 QSPI_InPeri(int pin) { return (u8)((*QSPI_STATUS(pin) >> 19) & 1); }
INLINE u8 QSPI_InPeri_hw(const io_qspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 19) & 1); }
#endif

#if RP2040
// get interrupt (0 or 1) from pad, before override (pin = QSPI_PIN_* 0..5)
INLINE u8 QSPI_IrqPad(int pin) { return (u8)((*QSPI_STATUS(pin) >> 24) & 1); }
INLINE u8 QSPI_IrqPad_hw(const io_qspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 24) & 1); }
#endif

// get interrupt (0 or 1) to processor, after override (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE u8 QSPI_IrqProc(int pin) { return (u8)((*QSPI_STATUS(pin) >> 26) & 1); }
INLINE u8 QSPI_IrqProc_hw(const io_qspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 26) & 1); }

#if RP2040
#define QSPI_OUTOVER_LSB	8
#else
#define QSPI_OUTOVER_LSB	12
#endif

// set Output pin override - normal (default state; pin = QSPI_PIN_*)
INLINE void QSPI_OutOverNormal(int pin)
	{ RegClr(QSPI_CTRL(pin), QSPI_OVER_MASK << QSPI_OUTOVER_LSB); }
INLINE void QSPI_OutOverNormal_hw(io_qspi_status_ctrl_hw_t* hw)
	{ RegClr(&hw->ctrl, QSPI_OVER_MASK << QSPI_OUTOVER_LSB); }

// set Output pin override - invert (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_OutOverInvert(int pin)
	{ RegMask(QSPI_CTRL(pin), QSPI_OVER_INVERT << QSPI_OUTOVER_LSB, QSPI_OVER_MASK << QSPI_OUTOVER_LSB); }
INLINE void QSPI_OutOverInvert_hw(io_qspi_status_ctrl_hw_t* hw)
	{ RegMask(&hw->ctrl, QSPI_OVER_INVERT << QSPI_OUTOVER_LSB, QSPI_OVER_MASK << QSPI_OUTOVER_LSB); }

// set Output pin override - low (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_OutOverLow(int pin)
	{ RegMask(QSPI_CTRL(pin), QSPI_OVER_LOW << QSPI_OUTOVER_LSB, QSPI_OVER_MASK << QSPI_OUTOVER_LSB); }
INLINE void QSPI_OutOverLow_hw(io_qspi_status_ctrl_hw_t* hw)
	{ RegMask(&hw->ctrl, QSPI_OVER_LOW << QSPI_OUTOVER_LSB, QSPI_OVER_MASK << QSPI_OUTOVER_LSB); }

// set Output pin override - high (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_OutOverHigh(int pin)
	{ RegSet(QSPI_CTRL(pin), QSPI_OVER_HIGH << QSPI_OUTOVER_LSB); }
INLINE void QSPI_OutOverHigh_hw(io_qspi_status_ctrl_hw_t* hw)
	{ RegSet(&hw->ctrl, QSPI_OVER_HIGH << QSPI_OUTOVER_LSB); }

#if RP2040
#define QSPI_OEOVER_LSB		12
#else
#define QSPI_OEOVER_LSB		14
#endif

// set Output enable pin override - normal (default state; pin = QSPI_PIN_*)
INLINE void QSPI_OEOverNormal(int pin)
	{ RegClr(QSPI_CTRL(pin), QSPI_OVER_MASK << QSPI_OEOVER_LSB); }
INLINE void QSPI_OEOverNormal_hw(io_qspi_status_ctrl_hw_t* hw)
	{ RegClr(&hw->ctrl, QSPI_OVER_MASK << QSPI_OEOVER_LSB); }

// set Output enable pin override - invert (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_OEOverInvert(int pin)
	{ RegMask(QSPI_CTRL(pin), QSPI_OVER_INVERT << QSPI_OEOVER_LSB, QSPI_OVER_MASK << QSPI_OEOVER_LSB); }
INLINE void QSPI_OEOverInvert_hw(io_qspi_status_ctrl_hw_t* hw)
	{ RegMask(&hw->ctrl, QSPI_OVER_INVERT << QSPI_OEOVER_LSB, QSPI_OVER_MASK << QSPI_OEOVER_LSB); }

// set Output enable pin override - disable (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_OEOverDisable(int pin)
	{ RegMask(QSPI_CTRL(pin), QSPI_OVER_LOW << QSPI_OEOVER_LSB, QSPI_OVER_MASK << QSPI_OEOVER_LSB); }
INLINE void QSPI_OEOverDisable_hw(io_qspi_status_ctrl_hw_t* hw)
	{ RegMask(&hw->ctrl, QSPI_OVER_LOW << QSPI_OEOVER_LSB, QSPI_OVER_MASK << QSPI_OEOVER_LSB); }

// set Output enable pin override - enable (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_OEOverEnable(int pin)
	{ RegSet(QSPI_CTRL(pin), QSPI_OVER_HIGH << QSPI_OEOVER_LSB); }
INLINE void QSPI_OEOverEnable_hw(io_qspi_status_ctrl_hw_t* hw)
	{ RegSet(&hw->ctrl, QSPI_OVER_HIGH << QSPI_OEOVER_LSB); }

// set Input pin override - normal (default state; pin = QSPI_PIN_*)
INLINE void QSPI_InOverNormal(int pin) { RegClr(QSPI_CTRL(pin), B16|B17); }
INLINE void QSPI_InOverNormal_hw(io_qspi_status_ctrl_hw_t* hw) { RegClr(&hw->ctrl, B16|B17); }

// set Input pin override - invert (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_InOverInvert(int pin) { RegMask(QSPI_CTRL(pin), B16, B16|B17); }
INLINE void QSPI_InOverInvert_hw(io_qspi_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B16, B16|B17); }

// set Input pin override - low (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_InOverLow(int pin) { RegMask(QSPI_CTRL(pin), B17, B16|B17); }
INLINE void QSPI_InOverLow_hw(io_qspi_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B17, B16|B17); }

// set Input pin override - high (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_InOverHigh(int pin) { RegSet(QSPI_CTRL(pin), B16|B17); }
INLINE void QSPI_InOverHigh_hw(io_qspi_status_ctrl_hw_t* hw) { RegSet(&hw->ctrl, B16|B17); }

// set IRQ pin override - normal (default state; pin = QSPI_PIN_*)
INLINE void QSPI_IRQOverNormal(int pin) { RegClr(QSPI_CTRL(pin), B28|B29); }
INLINE void QSPI_IRQOverNormal_hw(io_qspi_status_ctrl_hw_t* hw) { RegClr(&hw->ctrl, B28|B29); }

// set IRQ pin override - invert (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_IRQOverInvert(int pin) { RegMask(QSPI_CTRL(pin), B28, B28|B29); }
INLINE void QSPI_IRQOverInvert_hw(io_qspi_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B28, B28|B29); }

// set IRQ pin override - low (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_IRQOverLow(int pin) { RegMask(QSPI_CTRL(pin), B29, B28|B29); }
INLINE void QSPI_IRQOverLow_hw(io_qspi_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B29, B28|B29); }

// set IRQ pin override - high (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_IRQOverHigh(int pin) { RegSet(QSPI_CTRL(pin), B28|B29); }
INLINE void QSPI_IRQOverHigh_hw(io_qspi_status_ctrl_hw_t* hw) { RegSet(&hw->ctrl, B28|B29); }

// set QSPI function QSPI_FNC_*, resets overrides, does not affect pad settings (pin = QSPI_PIN_*; default QSPI_FNC_NULL)
// RP2350: After setup pin, disable isolation latches with QSPI_IsolationDisable()
INLINE void QSPI_Fnc(int pin, int fnc) { *QSPI_CTRL(pin) = fnc; }
INLINE void QSPI_Fnc_hw(io_qspi_status_ctrl_hw_t* hw, int fnc) { hw->ctrl = fnc; }

// get current QSPI function QSPI_FNC_* (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE u8 QSPI_GetFnc(int pin) { return (u8)(*QSPI_CTRL(pin) & 0x1f); }
INLINE u8 QSPI_GetFnc_hw(const io_qspi_status_ctrl_hw_t* hw) { return (u8)(hw->ctrl & 0x1f); }

// ==== QSPI pin interrupt control (use pins QSPI_PIN_* 0..5 or 0..7)
// Warning: pins and pads use different numbering (use predefined constants).

// get raw interrupt status (for both CPU)
//   pin = pin QSPI_PIN_* 0..5 or 0..7
//   returns events = bit mask with IRQ_EVENT_* of incoming events
INLINE u8 QSPI_IRQRaw(int pin) { return (u8)((*QSPI_IRQ_INTR >> (4*pin)) & QSPI_EVENT_ALL); }

// clear raw interrupt status - acknowledge interrupt (for both CPU)
//   pin = pin QSPI_PIN_* 0..5 or 0..7
//   events = bit mask with QSPI_EVENT_EDGE* of events to acknowledge
//  Interrupts LEVEL are not latched, they become inactive on end condition.
INLINE void QSPI_IRQAck(int pin, int events) { *QSPI_IRQ_INTR = (u32)events << (4*pin); }

// enable IRQ interrupt for selected/current CPU or dormant wake
//   cpu = CPU core 0 or 1 (or 2 dormant), use CpuID() to get current core
//   pin = pin QSPI_PIN_* 0..5 or 0..7
//   events = bit mask with QSPI_EVENT_* of events to enable
INLINE void QSPI_IRQEnableCpu(int cpu, int pin, int events) { RegSet(QSPI_IRQ_INTE(cpu), (u32)events << (4*pin)); }
INLINE void QSPI_IRQEnable(int pin, int events) { QSPI_IRQEnableCpu(CpuID(), pin, events); }
INLINE void QSPI_IRQEnableDorm(int pin, int events) { QSPI_IRQEnableCpu(2, pin, events); }

// disable IRQ interrupt for selected/current CPU or dormant wake (current state)
//   cpu = CPU core 0 or 1 (or 2 dormant), use CpuID() to get current core
//   pin = pin QSPI_PIN_* 0..5 or 0..7
//   events = bit mask with QSPI_EVENT_* of events to disable
INLINE void QSPI_IRQDisableCpu(int cpu, int pin, int events) { RegClr(QSPI_IRQ_INTE(cpu), (u32)events << (4*pin)); }
INLINE void QSPI_IRQDisable(int pin, int events) { QSPI_IRQDisableCpu(CpuID(), pin, events); }
INLINE void QSPI_IRQDisableDorm(int pin, int events) { QSPI_IRQDisableCpu(2, pin, events); }

// set IRQ handler
INLINE void QSPI_SetHandler(irq_handler_t handler) { SetHandler(IRQ_IO_QSPI, handler); }

// force IRQ interrupt for selected/current CPU or dormant wake
//   cpu = CPU core 0 or 1 (or 2 dormant), use CpuID() to get current core
//   pin = pin QSPI_PIN_* 0..5 or 0..7
//   events = bit mask with QSPI_EVENT_* of events to force
INLINE void QSPI_IRQForceCpu(int cpu, int pin, int events) { RegSet(QSPI_IRQ_INTF(cpu), (u32)events << (4*pin)); }
INLINE void QSPI_IRQForce(int pin, int events) { QSPI_IRQForceCpu(CpuID(), pin, events); }
INLINE void QSPI_IRQForceDorm(int pin, int events) { QSPI_IRQForceCpu(2, pin, events); }

// clear force IRQ interrupt for selected/current CPU or dormant wake
//   cpu = CPU core 0 or 1 (or 2 dormant), use CpuID() to get current core
//   pin = pin QSPI_PIN_* 0..5 or 0..7
//   events = bit mask with QSPI_EVENT_* of events to unforce
INLINE void QSPI_IRQUnforceCpu(int cpu, int pin, int events) { RegClr(QSPI_IRQ_INTF(cpu), (u32)events << (4*pin)); }
INLINE void QSPI_IRQUnforce(int pin, int events) { QSPI_IRQUnforceCpu(CpuID(), pin, events); }
INLINE void QSPI_IRQUnforceDorm(int pin, int events) { QSPI_IRQUnforceCpu(2, pin, events); }

// check IRQ interrupt status for selected/current CPU or dormant wake (returns 1 if IRQ is pending)
//   core = CPU core 0 or 1 (or 2 dormant), use CpuID() to get current core
//   pin = pin QSPI_PIN_* 0..5 or 0..7
//   event = event QSPI_EVENT_*
INLINE u8 QSPI_IRQIsPendingCpu(int cpu, int pin) { return (u8)((*QSPI_IRQ_INTS(cpu) >> (4*pin)) & QSPI_EVENT_ALL); }
INLINE u8 QSPI_IRQIsPending(int pin) { return QSPI_IRQIsPendingCpu(CpuID(), pin); }
INLINE u8 QSPI_IRQIsPendingDorm(int pin) { return QSPI_IRQIsPendingCpu(2, pin); }

// check if IRQ interrupt is forced for selected/current CPU or dormant wake
//   cpu = CPU core 0 or 1 (or 2 dormant), use CpuID() to get current core
//   pin = pin QSPI_PIN_* 0..5 or 0..7
//   returns events = bit mask with QSPI_EVENT_* of forced events
INLINE u8 QSPI_IRQIsForcedCpu(int cpu, int pin) { return (u8)((*QSPI_IRQ_INTF(cpu) >> (4*pin)) & QSPI_EVENT_ALL); }
INLINE u8 QSPI_IRQIsForced(int pin) { return QSPI_IRQIsForcedCpu(CpuID(), pin); }
INLINE u8 QSPI_IRQIsForcedDorm(int pin) { return QSPI_IRQIsForcedCpu(2, pin); }

// === QSPI data (use pins QSPI_PIN_* 0..5 or 0..7)
// Warning: pins and pads use different numbering (use predefined constants).

// get input pin (returns 0 or 1; pin = QSPI_PIN_* 0..5 or 0..7)
INLINE u8 QSPI_In(int pin) { return (u8)((sio_hw->gpio_hi_in >> (pin + QSPI_DATA_SHIFT)) & 1); }

// get all QSPI input pins (bit 0..5 or 0..7 = pin QSPI_PIN_*)
INLINE u8 QSPI_InAll(void) { return (u8)(sio_hw->gpio_hi_in >> QSPI_DATA_SHIFT); }

// output 0 to pin (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_Out0(int pin) { sio_hw->gpio_hi_clr = BIT(pin + QSPI_DATA_SHIFT); }

// output 1 to pin (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_Out1(int pin) { sio_hw->gpio_hi_set = BIT(pin + QSPI_DATA_SHIFT); }

// flip output to pin (pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_Flip(int pin) { sio_hw->gpio_hi_togl = BIT(pin + QSPI_DATA_SHIFT); }

// output value to pin (val = 0 or val != 0; pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_Out(int pin, int val) { if (val == 0) QSPI_Out0(pin); else QSPI_Out1(pin); }

// output masked values to pins
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_OutMask(u32 mask, u32 value) { sio_hw->gpio_hi_togl =
	(sio_hw->gpio_hi_out ^ (value << QSPI_DATA_SHIFT)) & (mask << QSPI_DATA_SHIFT); }

// output all pins (bit 0..5 or 0..7 = pin QSPI_PIN_*)
INLINE void QSPI_OutAll(u32 value) { QSPI_OutMask(0xff, value); }

// clear output pins masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_ClrMask(u32 mask) { sio_hw->gpio_hi_clr = (mask << QSPI_DATA_SHIFT); }

// set output pins masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_SetMask(u32 mask) { sio_hw->gpio_hi_set = (mask << QSPI_DATA_SHIFT); }

// flip output pins masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_FlipMask(u32 mask) { sio_hw->gpio_hi_togl = (mask << QSPI_DATA_SHIFT); }

// get last output value to pin (returns 0 or 1; pin = QSPI_PIN_* 0..5 or 0..7)
INLINE u8 QSPI_GetOut(int pin) { return (u8)((sio_hw->gpio_hi_out >> (pin + QSPI_DATA_SHIFT)) & 1); }

// get all last output values to pins
INLINE u8 QSPI_GetOutAll(void) { return (u8)(sio_hw->gpio_hi_out >> QSPI_DATA_SHIFT); }

// set output direction of pin (enable output mode; pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_DirOut(int pin) { sio_hw->gpio_hi_oe_set = BIT(pin + QSPI_DATA_SHIFT); }

// set input direction of pin (disable output mode; pin = QSPI_PIN_* 0..5 or 0..7)
INLINE void QSPI_DirIn(int pin) { sio_hw->gpio_hi_oe_clr = BIT(pin + QSPI_DATA_SHIFT); }

// set output direction of pin masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_DirOutMask(u32 mask) { sio_hw->gpio_hi_oe_set = (mask << QSPI_DATA_SHIFT); }

// set input direction of pin masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_DirInMask(int mask) { sio_hw->gpio_hi_oe_clr = (mask << QSPI_DATA_SHIFT); }

// set direction masked
INLINE void QSPI_SetDirMask(int mask, int outval) { sio_hw->gpio_hi_oe_togl =
	(sio_hw->gpio_hi_oe ^ (outval << QSPI_DATA_SHIFT)) & (mask << QSPI_DATA_SHIFT); }

// set direction of all pins
INLINE void QSPI_SetDirAll(int outval) { QSPI_SetDirMask(0xff, outval); }

// get output direction of pin (returns 0=input or 1=output; pin = QSPI_PIN_* 0..5 or 0..7)
INLINE u8 QSPI_GetDir(int pin) { return (u8)((sio_hw->gpio_hi_oe >> (pin + QSPI_DATA_SHIFT)) & 1); }

// get output direction of all pins (0=input, 1=output)
INLINE u8 QSPI_GetDirAll(void) { return (u8)(sio_hw->gpio_hi_oe >> QSPI_DATA_SHIFT); }

// initialize QSPI pins to connect Flash memory (must be run from RAM)
void NOFLASH(QSPI_InitFlash)(void);

#ifdef __cplusplus
}
#endif

#endif // _SDK_QSPI_H
