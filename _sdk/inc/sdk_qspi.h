
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

// 6 QSPI pins: SCLK, SS, SD0, SD1, SD2, SD3

#ifndef _SDK_QSPI_H
#define _SDK_QSPI_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "sdk_cpu.h"
#include "sdk_irq.h"
#include "sdk_sio.h"

#if USE_ORIGSDK		// include interface of original SDK
#include "orig/orig_io_qspi.h"		// constants of original SDK
#include "orig/orig_pads_qspi.h"	// constants of original SDK
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
#define QSPI_PAD_ALLMASK 0x3f		// mask of all QSPI pads

#define NUM_QSPI_GPIOS	QSPI_PAD_NUM

// pad control hardware registers
//#define PADS_QSPI_BASE		0x40020000	// QSPI pad control
#define QSPI_VOLT	((volatile u32*)(PADS_QSPI_BASE+0))	// voltage select
#define QSPI_PAD(pad)	((volatile u32*)(PADS_QSPI_BASE+(pad)*4+4)) // QSPI pad control register, pad = QSPI_PAD_*

// pad control hardware interface
typedef struct {
	io32	voltage_select;		// 0x00: voltage select
	io32	io[NUM_QSPI_GPIOS];	// 0x04: pad control register
} pads_qspi_hw_t;

#define pads_qspi_hw ((pads_qspi_hw_t*)PADS_QSPI_BASE)

STATIC_ASSERT(sizeof(pads_qspi_hw_t) == 0x1C, "Incorrect pads_qspi_hw_t!");

// === QSPI pin control (use QSPI_PIN_*)

#define QSPI_PIN_SCLK	0
#define QSPI_PIN_SS	1
#define QSPI_PIN_SD0	2
#define QSPI_PIN_SD1	3
#define QSPI_PIN_SD2	4
#define QSPI_PIN_SD3	5

#define QSPI_PIN_NUM	6		// number of QSPI pins
#define QSPI_PIN_ALLMASK 0x3f		// mask of all QSPI pins

// pin control hardware registers
//#define IO_QSPI_BASE		0x40018000	// QSPI pads
#define QSPI_STATUS(pin) ((volatile u32*)(IO_QSPI_BASE+(pin)*8)) // QSPI status (read only), pin = QSPI_PIN_*
#define QSPI_CTRL(pin) ((volatile u32*)(IO_QSPI_BASE+(pin)*8+4)) // QSPI pin control, pin = QSPI_PIN_*

// pin control hardware interface
typedef struct {
	io32	status;		// 0x00: GPIO status
	io32	ctrl;		// 0x04: GPIO control including function select and overrides
} ioqspi_status_ctrl_hw_t;

STATIC_ASSERT(sizeof(ioqspi_status_ctrl_hw_t) == 0x08, "Incorrect ioqspi_status_ctrl_hw_t!");

// === QSPI interrupt control (use QSPI_PIN_*)

// pin interrupt hardware registers (cpi = 0 or 1, or 2 for dormant wake)
#define QSPI_IRQ_INTR ((volatile u32*)(IO_QSPI_BASE+0x30)) // raw interrupt of both CPUs
#define QSPI_IRQ_INTE(cpu) ((volatile u32*)(IO_QSPI_BASE+(cpu)*12+0x34)) // interrupt enable
#define QSPI_IRQ_INTF(cpu) ((volatile u32*)(IO_QSPI_BASE+(cpu)*12+0x38)) // force interrupt
#define QSPI_IRQ_INTS(cpu) ((volatile u32*)(IO_QSPI_BASE+(cpu)*12+0x3C)) // interrupt status

// pin interrupt hardware interface
typedef struct {
	io32	inte;		// 0x00: Interrupt Enable for proc0
	io32	intf;		// 0x04: Interrupt Force for proc0
	io32	ints;		// 0x08: Interrupt status after masking & forcing for proc0
} io_qspi_ctrl_hw_t;

STATIC_ASSERT(sizeof(io_qspi_ctrl_hw_t) == 0x0C, "Incorrect io_qspi_ctrl_hw_t!");

// pin bank interface
typedef struct {
    ioqspi_status_ctrl_hw_t	io[NUM_QSPI_GPIOS];	// 0x00: pin control
    io32			intr;			// 0x30: Raw Interrupts
    io_qspi_ctrl_hw_t		proc0_qspi_ctrl;	// 0x34: interrupt control for proc0
    io_qspi_ctrl_hw_t		proc1_qspi_ctrl;	// 0x40: interrupt control for proc1
    io_qspi_ctrl_hw_t		dormant_wake_qspi_ctrl;	// 0x4C: dormant interrupt control
} ioqspi_hw_t;

#define ioqspi_hw ((ioqspi_hw_t*)IO_QSPI_BASE)

STATIC_ASSERT(sizeof(ioqspi_hw_t) == 0x58, "Incorrect ioqspi_hw_t!");

// === QSPI data (use QSPI_PIN_*)

// QSPI pin data registers (bit 0..5 = pin QSPI_PIN_*)
//#define SIO_BASE		0xd0000000	// SIO registers
//   (SIO does not support aliases for atomic access!)
#define QSPI_IN_DATA	((volatile u32*)(SIO_BASE+8))		// QSPI input
#define QSPI_OUT_DATA	((volatile u32*)(SIO_BASE+0x30))	// QSPI output (read: last value written to output)
#define QSPI_OUT_SET	((volatile u32*)(SIO_BASE+0x34))	// QSPI output set
#define QSPI_OUT_CLR	((volatile u32*)(SIO_BASE+0x38))	// QSPI output clear
#define QSPI_OUT_XOR	((volatile u32*)(SIO_BASE+0x3C))	// QSPI output XOR
#define QSPI_OE		((volatile u32*)(SIO_BASE+0x40))	// QSPI output enable (read: last value written)
#define QSPI_OE_SET	((volatile u32*)(SIO_BASE+0x44))	// QSPI output enable set
#define QSPI_OE_CLR	((volatile u32*)(SIO_BASE+0x48))	// QSPI output enable clear
#define QSPI_OE_XOR	((volatile u32*)(SIO_BASE+0x4C))	// QSPI output enable XOR

// QSPI pin functions
#define QSPI_FNC_XIP	0	// external Flash
#define QSPI_FNC_SIO	5	// SIO (GPIO)
#define QSPI_FNC_NULL	31	// no function (default)

// interrupt events
#define QSPI_EVENT_LEVELLOW	B0	// level low
#define QSPI_EVENT_LEVELHIGH	B1	// level high
#define QSPI_EVENT_EDGELOW	B2	// edge low
#define QSPI_EVENT_EDGEHIGH	B3	// edge high

#define QSPI_EVENT_ALL		0x0f	// all events
#define QSPI_EVENT_NONE		0	// no event

// ==== QSPI pad control (use pads QSPI_PAD_*)

// QSPI set voltage to 3.3V (DVDD >= 2V5; default state)
INLINE void QSPI_Voltage3V3(void) { pads_qspi_hw->voltage_select = 0; }

// QSPI set voltage to 1.8V (DVDD <= 1V8)
INLINE void QSPI_Voltage1V8(void) { pads_qspi_hw->voltage_select = B0; }

// get pointer to pad control interface (pad = QSPI_PAD_*)
INLINE io32* QSPI_PadHw(int pad) { return &pads_qspi_hw->io[pad]; }

// init pad to default state (pad = QSPI_PAD_*)
//  - slow slew rate, enable schmitt, pull-down, 4mA, in/out enable
INLINE void QSPI_PadInit(int pad) { *QSPI_PAD(pad) = B1|B2|B4|B6; }
INLINE void QSPI_PadInit_hw(io32* hw) { *hw = B1|B2|B4|B6; }

// QSPI output enable; disable has priority over QSPI_DirOut (default state; pad = QSPI_PAD_*)
INLINE void QSPI_OutEnable(int pad) { RegClr(QSPI_PAD(pad), B7); }
INLINE void QSPI_OutEnable_hw(io32* hw) { RegClr(hw, B7); }

// QSPI output disable; disable has priority over QSPI_DirOut (pad = QSPI_PAD_*)
INLINE void QSPI_OutDisable(int pad) { RegSet(QSPI_PAD(pad), B7); }
INLINE void QSPI_OutDisable_hw(io32* hw) { RegSet(hw, B7); }

// enable pad input (default state; pad = QSPI_PAD_*)
INLINE void QSPI_InEnable(int pad) { RegSet(QSPI_PAD(pad), B6); }
INLINE void QSPI_InEnable_hw(io32* hw) { RegSet(hw, B6); }

// disable pad input (pad = QSPI_PAD_*)
INLINE void QSPI_InDisable(int pad) { RegClr(QSPI_PAD(pad), B6); }
INLINE void QSPI_InDisable_hw(io32* hw) { RegClr(hw, B6); }

// set output strength to 2 mA (pad = QSPI_PAD_*)
INLINE void QSPI_Drive2mA(int pad) { RegClr(QSPI_PAD(pad), B4|B5); }
INLINE void QSPI_Drive2mA_hw(io32* hw) { RegClr(hw, B4|B5); }

// set output strength to 4 mA (default state; pad = QSPI_PAD_*)
INLINE void QSPI_Drive4mA(int pad) { RegMask(QSPI_PAD(pad), B4, B4|B5); }
INLINE void QSPI_Drive4mA_hw(io32* hw) { RegMask(hw, B4, B4|B5); }

// set output strength to 8 mA (pad = QSPI_PAD_*)
INLINE void QSPI_Drive8mA(int pad) { RegMask(QSPI_PAD(pad), B5, B4|B5); }
INLINE void QSPI_Drive8mA_hw(io32* hw) { RegMask(hw, B5, B4|B5); }

// set output strength to 12 mA (pad = QSPI_PAD_*)
INLINE void QSPI_Drive12mA(int pad) { RegSet(QSPI_PAD(pad), B4|B5); }
INLINE void QSPI_Drive12mA_hw(io32* hw) { RegSet(hw, B4|B5); }

// set no pulls (pad = QSPI_PAD_*)
INLINE void QSPI_NoPull(int pad) { RegClr(QSPI_PAD(pad), B2|B3); }
INLINE void QSPI_NoPull_hw(io32* hw) { RegClr(hw, B2|B3); }

// set pull down (default state; pad = QSPI_PAD_*)
INLINE void QSPI_PullDown(int pad) { RegMask(QSPI_PAD(pad), B2, B2|B3); }
INLINE void QSPI_PullDown_hw(io32* hw) { RegMask(hw, B2, B2|B3); }

// set pull up (pad = QSPI_PAD_*)
INLINE void QSPI_PullUp(int pad) { RegMask(QSPI_PAD(pad), B3, B2|B3); }
INLINE void QSPI_PullUp_hw(io32* hw) { RegMask(hw, B3, B2|B3); }

// set bus keep (weak pull up and down; pad = QSPI_PAD_*)
INLINE void QSPI_BusKeep(int pad) { RegSet(QSPI_PAD(pad), B2|B3); }
INLINE void QSPI_BusKeep_hw(io32* hw) { RegSet(hw, B2|B3); }

// enable schmitt trigger (use hysteresis on input; default state; pad = QSPI_PAD_*)
INLINE void QSPI_SchmittEnable(int pad) { RegSet(QSPI_PAD(pad), B1); }
INLINE void QSPI_SchmittEnable_hw(io32* hw) { RegSet(hw, B1); }

// disable schmitt trigger (do not use hysteresis on input; pad = QSPI_PAD_*)
INLINE void QSPI_SchmittDisable(int pad) { RegClr(QSPI_PAD(pad), B1); }
INLINE void QSPI_SchmittDisable_hw(io32* hw) { RegClr(hw, B1); }

// use slow slew rate control on output (default state; pad = QSPI_PAD_*)
INLINE void QSPI_Slow(int pad) { RegClr(QSPI_PAD(pad), B0); }
INLINE void QSPI_Slow_hw(io32* hw) { RegClr(hw, B0); }

// use fast slew rate control on output (pad = QSPI_PAD_*)
INLINE void QSPI_Fast(int pad) { RegSet(QSPI_PAD(pad), B0); }
INLINE void QSPI_Fast_hw(io32* hw) { RegSet(hw, B0); }

// ==== QSPI pin control (use pins QSPI_PIN_*)

// get pointer to pin control interface (pin = 0..29)
INLINE ioqspi_status_ctrl_hw_t* QSPI_PinHw(int pin) { return &ioqspi_hw->io[pin]; }

// get output (0 or 1) from peripheral, before override (pin = QSPI_PIN_*)
INLINE u8 QSPI_OutPeri(int pin) { return (u8)((*QSPI_STATUS(pin) >> 8) & 1); }
INLINE u8 QSPI_OutPeri_hw(const ioqspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 8) & 1); }

// get output (0 or 1) to pad, after override (pin = QSPI_PIN_*)
INLINE u8 QSPI_OutPad(int pin) { return (u8)((*QSPI_STATUS(pin) >> 9) & 1); }
INLINE u8 QSPI_OutPad_hw(const ioqspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 9) & 1); }

// get output enable (0 or 1) from peripheral, before override (pin = QSPI_PIN_*)
INLINE u8 QSPI_OePeri(int pin) { return (u8)((*QSPI_STATUS(pin) >> 12) & 1); }
INLINE u8 QSPI_OePeri_hw(const ioqspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 12) & 1); }

// get output enable (0 or 1) to pad, after override (pin = QSPI_PIN_*)
INLINE u8 QSPI_OePad(int pin) { return (u8)((*QSPI_STATUS(pin) >> 13) & 1); }
INLINE u8 QSPI_OePad_hw(const ioqspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 13) & 1); }

// get input (0 or 1) from pad, before override (pin = QSPI_PIN_*)
INLINE u8 QSPI_InPad(int pin) { return (u8)((*QSPI_STATUS(pin) >> 17) & 1); }
INLINE u8 QSPI_InPad_hw(const ioqspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 17) & 1); }

// get input (0 or 1) to peripheral, after override (pin = QSPI_PIN_*)
INLINE u8 QSPI_InPeri(int pin) { return (u8)((*QSPI_STATUS(pin) >> 19) & 1); }
INLINE u8 QSPI_InPeri_hw(const ioqspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 19) & 1); }

// get interrupt (0 or 1) from pad, before override (pin = QSPI_PIN_*)
INLINE u8 QSPI_IrqPad(int pin) { return (u8)((*QSPI_STATUS(pin) >> 24) & 1); }
INLINE u8 QSPI_IrqPad_hw(const ioqspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 24) & 1); }

// get interrupt (0 or 1) to processor, after override (pin = QSPI_PIN_*)
INLINE u8 QSPI_IrqProc(int pin) { return (u8)((*QSPI_STATUS(pin) >> 26) & 1); }
INLINE u8 QSPI_IrqProc_hw(const ioqspi_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 26) & 1); }

// set Output pin override - normal (default state; pin = QSPI_PIN_*)
INLINE void QSPI_OutOverNormal(int pin) { RegClr(QSPI_CTRL(pin), B8|B9); }
INLINE void QSPI_OutOverNormal_hw(ioqspi_status_ctrl_hw_t* hw) { RegClr(&hw->ctrl, B8|B9); }

// set Output pin override - invert (pin = QSPI_PIN_*)
INLINE void QSPI_OutOverInvert(int pin) { RegMask(QSPI_CTRL(pin), B8, B8|B9); }
INLINE void QSPI_OutOverInvert_hw(ioqspi_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B8, B8|B9); }

// set Output pin override - low (pin = QSPI_PIN_*)
INLINE void QSPI_OutOverLow(int pin) { RegMask(QSPI_CTRL(pin), B9, B8|B9); }
INLINE void QSPI_OutOverLow_hw(ioqspi_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B9, B8|B9); }

// set Output pin override - high (pin = QSPI_PIN_*)
INLINE void QSPI_OutOverHigh(int pin) { RegSet(QSPI_CTRL(pin), B8|B9); }
INLINE void QSPI_OutOverHigh_hw(ioqspi_status_ctrl_hw_t* hw) { RegSet(&hw->ctrl, B8|B9); }

// set Output enable pin override - normal (default state; pin = QSPI_PIN_*)
INLINE void QSPI_OEOverNormal(int pin) { RegClr(QSPI_CTRL(pin), B12|B13); }
INLINE void QSPI_OEOverNormal_hw(ioqspi_status_ctrl_hw_t* hw) { RegClr(&hw->ctrl, B12|B13); }

// set Output enable pin override - invert (pin = QSPI_PIN_*)
INLINE void QSPI_OEOverInvert(int pin) { RegMask(QSPI_CTRL(pin), B12, B12|B13); }
INLINE void QSPI_OEOverInvert_hw(ioqspi_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B12, B12|B13); }

// set Output enable pin override - disable (pin = QSPI_PIN_*)
INLINE void QSPI_OEOverDisable(int pin) { RegMask(QSPI_CTRL(pin), B13, B12|B13); }
INLINE void QSPI_OEOverDisable_hw(ioqspi_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B13, B12|B13); }

// set Output enable pin override - enable (pin = QSPI_PIN_*)
INLINE void QSPI_OEOverEnable(int pin) { RegSet(QSPI_CTRL(pin), B12|B13); }
INLINE void QSPI_OEOverEnable_hw(ioqspi_status_ctrl_hw_t* hw) { RegSet(&hw->ctrl, B12|B13); }

// set Input pin override - normal (default state; pin = QSPI_PIN_*)
INLINE void QSPI_InOverNormal(int pin) { RegClr(QSPI_CTRL(pin), B16|B17); }
INLINE void QSPI_InOverNormal_hw(ioqspi_status_ctrl_hw_t* hw) { RegClr(&hw->ctrl, B16|B17); }

// set Input pin override - invert (pin = QSPI_PIN_*)
INLINE void QSPI_InOverInvert(int pin) { RegMask(QSPI_CTRL(pin), B16, B16|B17); }
INLINE void QSPI_InOverInvert_hw(ioqspi_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B16, B16|B17); }

// set Input pin override - low (pin = QSPI_PIN_*)
INLINE void QSPI_InOverLow(int pin) { RegMask(QSPI_CTRL(pin), B17, B16|B17); }
INLINE void QSPI_InOverLow_hw(ioqspi_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B17, B16|B17); }

// set Input pin override - high (pin = QSPI_PIN_*)
INLINE void QSPI_InOverHigh(int pin) { RegSet(QSPI_CTRL(pin), B16|B17); }
INLINE void QSPI_InOverHigh_hw(ioqspi_status_ctrl_hw_t* hw) { RegSet(&hw->ctrl, B16|B17); }

// set IRQ pin override - normal (default state; pin = QSPI_PIN_*)
INLINE void QSPI_IRQOverNormal(int pin) { RegClr(QSPI_CTRL(pin), B28|B29); }
INLINE void QSPI_IRQOverNormal_hw(ioqspi_status_ctrl_hw_t* hw) { RegClr(&hw->ctrl, B28|B29); }

// set IRQ pin override - invert (pin = QSPI_PIN_*)
INLINE void QSPI_IRQOverInvert(int pin) { RegMask(QSPI_CTRL(pin), B28, B28|B29); }
INLINE void QSPI_IRQOverInvert_hw(ioqspi_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B28, B28|B29); }

// set IRQ pin override - low (pin = QSPI_PIN_*)
INLINE void QSPI_IRQOverLow(int pin) { RegMask(QSPI_CTRL(pin), B29, B28|B29); }
INLINE void QSPI_IRQOverLow_hw(ioqspi_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B29, B28|B29); }

// set IRQ pin override - high (pin = QSPI_PIN_*)
INLINE void QSPI_IRQOverHigh(int pin) { RegSet(QSPI_CTRL(pin), B28|B29); }
INLINE void QSPI_IRQOverHigh_hw(ioqspi_status_ctrl_hw_t* hw) { RegSet(&hw->ctrl, B28|B29); }

// set QSPI function QSPI_FNC_*, resets overrides, does not affect pad settings (pin = QSPI_PIN_*; default QSPI_FNC_NULL)
INLINE void QSPI_Fnc(int pin, int fnc) { *QSPI_CTRL(pin) = fnc; }
INLINE void QSPI_Fnc_hw(ioqspi_status_ctrl_hw_t* hw, int fnc) { hw->ctrl = fnc; }

// get current QSPI function QSPI_FNC_* (pin = QSPI_PIN_*)
INLINE u8 QSPI_GetFnc(int pin) { return (u8)(*QSPI_CTRL(pin) & 0x1f); }
INLINE u8 QSPI_GetFnc_hw(const ioqspi_status_ctrl_hw_t* hw) { return (u8)(hw->ctrl & 0x1f); }

// ==== QSPI pin interrupt control (use pins QSPI_PIN_*)

// get raw interrupt status (for both CPU)
//   pin = pin QSPI_PIN_*
//   returns events = bit mask with IRQ_EVENT_* of incoming events
INLINE u8 QSPI_IRQRaw(int pin) { return (u8)((*QSPI_IRQ_INTR >> (4*pin)) & QSPI_EVENT_ALL); }

// clear raw interrupt status - acknowledge interrupt (for both CPU)
//   pin = pin QSPI_PIN_*
//   events = bit mask with QSPI_EVENT_EDGE* of events to acknowledge
//  Interrupts LEVEL are not latched, they become inactive on end condition.
INLINE void QSPI_IRQAck(int pin, int events) { *QSPI_IRQ_INTR = (u32)events << (4*pin); }

// enable IRQ interrupt for selected/current CPU or dormant wake
//   cpu = CPU core 0 or 1 (or 2 dormant), use CpuID() to get current core
//   pin = pin QSPI_PIN_*
//   events = bit mask with QSPI_EVENT_* of events to enable
INLINE void QSPI_IRQEnableCpu(int cpu, int pin, int events) { RegSet(QSPI_IRQ_INTE(cpu), (u32)events << (4*pin)); }
INLINE void QSPI_IRQEnable(int pin, int events) { QSPI_IRQEnableCpu(CpuID(), pin, events); }
INLINE void QSPI_IRQEnableDorm(int pin, int events) { QSPI_IRQEnableCpu(2, pin, events); }

// disable IRQ interrupt for selected/current CPU or dormant wake (current state)
//   cpu = CPU core 0 or 1 (or 2 dormant), use CpuID() to get current core
//   pin = pin QSPI_PIN_*
//   events = bit mask with QSPI_EVENT_* of events to disable
INLINE void QSPI_IRQDisableCpu(int cpu, int pin, int events) { RegClr(QSPI_IRQ_INTE(cpu), (u32)events << (4*pin)); }
INLINE void QSPI_IRQDisable(int pin, int events) { QSPI_IRQDisableCpu(CpuID(), pin, events); }
INLINE void QSPI_IRQDisableDorm(int pin, int events) { QSPI_IRQDisableCpu(2, pin, events); }

// set IRQ handler
INLINE void QSPI_SetHandler(irq_handler_t handler) { SetHandler(IRQ_IO_QSPI, handler); }

// force IRQ interrupt for selected/current CPU or dormant wake
//   cpu = CPU core 0 or 1 (or 2 dormant), use CpuID() to get current core
//   pin = pin QSPI_PIN_*
//   events = bit mask with QSPI_EVENT_* of events to force
INLINE void QSPI_IRQForceCpu(int cpu, int pin, int events) { RegSet(QSPI_IRQ_INTF(cpu), (u32)events << (4*pin)); }
INLINE void QSPI_IRQForce(int pin, int events) { QSPI_IRQForceCpu(CpuID(), pin, events); }
INLINE void QSPI_IRQForceDorm(int pin, int events) { QSPI_IRQForceCpu(2, pin, events); }

// clear force IRQ interrupt for selected/current CPU or dormant wake
//   cpu = CPU core 0 or 1 (or 2 dormant), use CpuID() to get current core
//   pin = pin QSPI_PIN_*
//   events = bit mask with QSPI_EVENT_* of events to unforce
INLINE void QSPI_IRQUnforceCpu(int cpu, int pin, int events) { RegClr(QSPI_IRQ_INTF(cpu), (u32)events << (4*pin)); }
INLINE void QSPI_IRQUnforce(int pin, int events) { QSPI_IRQUnforceCpu(CpuID(), pin, events); }
INLINE void QSPI_IRQUnforceDorm(int pin, int events) { QSPI_IRQUnforceCpu(2, pin, events); }

// check IRQ interrupt status for selected/current CPU or dormant wake (returns 1 if IRQ is pending)
//   core = CPU core 0 or 1 (or 2 dormant), use CpuID() to get current core
//   pin = pin QSPI_PIN_*
//   event = event QSPI_EVENT_*
INLINE u8 QSPI_IRQIsPendingCpu(int cpu, int pin) { return (u8)((*QSPI_IRQ_INTS(cpu) >> (4*pin)) & QSPI_EVENT_ALL); }
INLINE u8 QSPI_IRQIsPending(int pin) { return QSPI_IRQIsPendingCpu(CpuID(), pin); }
INLINE u8 QSPI_IRQIsPendingDorm(int pin) { return QSPI_IRQIsPendingCpu(2, pin); }

// check if IRQ interrupt is forced for selected/current CPU or dormant wake
//   cpu = CPU core 0 or 1 (or 2 dormant), use CpuID() to get current core
//   pin = pin QSPI_PIN_*
//   returns events = bit mask with QSPI_EVENT_* of forced events
INLINE u8 QSPI_IRQIsForcedCpu(int cpu, int pin) { return (u8)((*QSPI_IRQ_INTF(cpu) >> (4*pin)) & QSPI_EVENT_ALL); }
INLINE u8 QSPI_IRQIsForced(int pin) { return QSPI_IRQIsForcedCpu(CpuID(), pin); }
INLINE u8 QSPI_IRQIsForcedDorm(int pin) { return QSPI_IRQIsForcedCpu(2, pin); }

// === QSPI data

// get input pin (returns 0 or 1; pin = QSPI_PIN_*)
INLINE u8 QSPI_In(int pin) { return (u8)((sio_hw->gpio_hi_in >> pin) & 1); }

// get all QSPI input pins (bit 0..5 = pin QSPI_PIN_*)
INLINE u8 QSPI_InAll(void) { return (u8)sio_hw->gpio_hi_in; }

// output 0 to pin (pin = QSPI_PIN_*)
INLINE void QSPI_Out0(int pin) { sio_hw->gpio_hi_clr = BIT(pin); }

// output 1 to pin (pin = QSPI_PIN_*)
INLINE void QSPI_Out1(int pin) { sio_hw->gpio_hi_set = BIT(pin); }

// flip output to pin (pin = QSPI_PIN_*)
INLINE void QSPI_Flip(int pin) { sio_hw->gpio_hi_togl = BIT(pin); }

// output value to pin (val = 0 or val != 0; pin = QSPI_PIN_*)
INLINE void QSPI_Out(int pin, int val) { if (val == 0) QSPI_Out0(pin); else QSPI_Out1(pin); }

// output all pins (bit 0..5 = pin 0..5)
INLINE void QSPI_OutAll(u32 value) { sio_hw->gpio_hi_out = value; }

// output masked values to pins
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_OutMask(int mask, int value) { sio_hw->gpio_hi_togl = (sio_hw->gpio_hi_out ^ value) & mask; }

// clear output pins masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_ClrMask(int mask) { sio_hw->gpio_hi_clr = mask; }

// set output pins masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_SetMask(int mask) { sio_hw->gpio_hi_set = mask; }

// flip output pins masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_FlipMask(int mask) { sio_hw->gpio_hi_togl = mask; }

// get last output value to pin (returns 0 or 1; pin = QSPI_PIN_*)
INLINE u8 QSPI_GetOut(int pin) { return (u8)((sio_hw->gpio_hi_out >> pin) & 1); }

// get all last output values to pins
INLINE u8 QSPI_GetOutAll(void) { return (u8)sio_hw->gpio_hi_out; }

// set output direction of pin (enable output mode; pin = QSPI_PIN_*)
INLINE void QSPI_DirOut(int pin) { sio_hw->gpio_hi_oe_set = BIT(pin); }

// set input direction of pin (disable output mode; pin = QSPI_PIN_*)
INLINE void QSPI_DirIn(int pin) { sio_hw->gpio_hi_oe_clr = BIT(pin); }

// set output direction of pin masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_DirOutMask(int mask) { sio_hw->gpio_hi_oe_set = mask; }

// set input direction of pin masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_DirInMask(int mask) { sio_hw->gpio_hi_oe_clr = mask; }

// set direction masked
INLINE void QSPI_SetDirMask(int mask, int outval) { sio_hw->gpio_hi_oe_togl = (sio_hw->gpio_hi_oe ^ outval) & mask; }

// set direction of all pins
INLINE void QSPI_SetDirAll(int outval) { sio_hw->gpio_hi_oe = outval; }

// get output direction of pin (returns 0=input or 1=output; pin = QSPI_PIN_*)
INLINE u8 QSPI_GetDir(int pin) { return (u8)((sio_hw->gpio_hi_oe >> pin) & 1); }

// get output direction of all pins (0=input, 1=output)
INLINE u8 QSPI_GetDirAll(void) { return (u8)sio_hw->gpio_hi_oe; }

// initialize QSPI pins to connect Flash memory
void NOFLASH(QSPI_InitFlash)(void);

#ifdef __cplusplus
}
#endif

#endif // _SDK_QSPI_H
