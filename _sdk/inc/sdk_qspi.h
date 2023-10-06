
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

#ifdef __cplusplus
extern "C" {
#endif

// === QSPI pads

#define QSPI_PAD_SCLK	0
#define QSPI_PAD_SD0	1
#define QSPI_PAD_SD1	2
#define QSPI_PAD_SD2	3
#define QSPI_PAD_SD3	4
#define QSPI_PAD_SS	5

#define QSPI_PAD_NUM	6		// number of QSPI pads
#define QSPI_PAD_ALLMASK 0x3f		// mask of all QSPI pads

//#define PADS_QSPI_BASE		0x40020000	// QSPI pad control
#define QSPI_VOLT	((volatile u32*)(PADS_QSPI_BASE+0))	// voltage select
#define QSPI_PAD(pad) ((volatile u32*)(PADS_QSPI_BASE+(pad)*4+4)) // QSPI pad control register, pad = QSPI_PAD_*

// === QSPI pins

#define QSPI_PIN_SCLK	0
#define QSPI_PIN_SS	1
#define QSPI_PIN_SD0	2
#define QSPI_PIN_SD1	3
#define QSPI_PIN_SD2	4
#define QSPI_PIN_SD3	5

#define QSPI_PIN_NUM	6		// number of QSPI pins
#define QSPI_PIN_ALLMASK 0x3f		// mask of all QSPI pins

//#define IO_QSPI_BASE		0x40018000	// QSPI pads
#define QSPI_STATUS(pin) ((volatile u32*)(IO_QSPI_BASE+(pin)*8)) // QSPI status (read only), pin = QSPI_PIN_*
#define QSPI_CTRL(pin) ((volatile u32*)(IO_QSPI_BASE+(pin)*8+4)) // QSPI pin control, pin = QSPI_PIN_*
#define QSPI_IRQ_INTR ((volatile u32*)(IO_QSPI_BASE+0x30)) // raw interrupt of both CPUs
#define QSPI_IRQ_INTE(cpu) ((volatile u32*)(IO_QSPI_BASE+(cpu)*12+0x34)) // interrupt enable
#define QSPI_IRQ_INTF(cpu) ((volatile u32*)(IO_QSPI_BASE+(cpu)*12+0x38)) // force interrupt
#define QSPI_IRQ_INTS(cpu) ((volatile u32*)(IO_QSPI_BASE+(cpu)*12+0x3C)) // interrupt status

// QSPI pin functions
#define QSPI_FNC_XIP	0	// external Flash
#define QSPI_FNC_SIO	5	// SIO (GPIO)
#define QSPI_FNC_NULL	31	// no function (default)

// interrupt events
#define QSPI_EVENT_LEVELLOW	0	// level low
#define QSPI_EVENT_LEVELHIGH	1	// level high
#define QSPI_EVENT_EDGELOW	2	// edge low
#define QSPI_EVENT_EDGEHIGH	3	// edge high

// === QSPI data

// QSPI pin data registers (bit 0..5 = pin QSPI_PIN_*)
//#define SIO_BASE		0xd0000000	// SIO registers
//   (SIO does not support aliases for atomic access!)
#define QSPI_IN		((volatile u32*)(SIO_BASE+8))		// QSPI input
#define QSPI_OUT	((volatile u32*)(SIO_BASE+0x30))	// QSPI output (read: last value written to output)
#define QSPI_OUT_SET	((volatile u32*)(SIO_BASE+0x34))	// QSPI output set
#define QSPI_OUT_CLR	((volatile u32*)(SIO_BASE+0x38))	// QSPI output clear
#define QSPI_OUT_XOR	((volatile u32*)(SIO_BASE+0x3C))	// QSPI output XOR
#define QSPI_OE		((volatile u32*)(SIO_BASE+0x40))	// QSPI output enable (read: last value written)
#define QSPI_OE_SET	((volatile u32*)(SIO_BASE+0x44))	// QSPI output enable set
#define QSPI_OE_CLR	((volatile u32*)(SIO_BASE+0x48))	// QSPI output enable clear
#define QSPI_OE_XOR	((volatile u32*)(SIO_BASE+0x4C))	// QSPI output enable XOR

// ==== QSPI pad control (use pads QSPI_PAD_*)

// QSPI set voltage to 3.3V (DVDD >= 2V5; default state)
INLINE void QSPI_Voltage3V3() { RegClr(QSPI_VOLT, B0); }

// QSPI set voltage to 1.8V (DVDD <= 1V8)
INLINE void QSPI_Voltage1V8() { RegSet(QSPI_VOLT, B0); }

// QSPI output enable; disable has priority over QSPI_DirOut (default state; pad = QSPI_PAD_*)
INLINE void QSPI_OutEnable(u8 pad) { RegClr(QSPI_PAD(pad), B7); }

// QSPI output disable; disable has priority over QSPI_DirOut (pad = QSPI_PAD_*)
INLINE void QSPI_OutDisable(u8 pad) { RegSet(QSPI_PAD(pad), B7); }

// enable pad input (default state; pad = QSPI_PAD_*)
INLINE void QSPI_InEnable(u8 pad) { RegSet(QSPI_PAD(pad), B6); }

// disable pad input (pad = QSPI_PAD_*)
INLINE void QSPI_InDisable(u8 pad) { RegClr(QSPI_PAD(pad), B6); }

// set output strength to 2 mA (pad = QSPI_PAD_*)
INLINE void QSPI_Drive2mA(u8 pad) { RegClr(QSPI_PAD(pad), B4|B5); }

// set output strength to 4 mA (default state; pad = QSPI_PAD_*)
INLINE void QSPI_Drive4mA(u8 pad) { RegMask(QSPI_PAD(pad), B4, B4|B5); }

// set output strength to 8 mA (pad = QSPI_PAD_*)
INLINE void QSPI_Drive8mA(u8 pad) { RegMask(QSPI_PAD(pad), B5, B4|B5); }

// set output strength to 12 mA (pad = QSPI_PAD_*)
INLINE void QSPI_Drive12mA(u8 pad) { RegSet(QSPI_PAD(pad), B4|B5); }

// set no pulls (pad = QSPI_PAD_*)
INLINE void QSPI_NoPull(u8 pad) { RegClr(QSPI_PAD(pad), B2|B3); }

// set pull down (default state; pad = QSPI_PAD_*)
INLINE void QSPI_PullDown(u8 pad) { RegMask(QSPI_PAD(pad), B2, B2|B3); }

// set pull up (pad = QSPI_PAD_*)
INLINE void QSPI_PullUp(u8 pad) { RegMask(QSPI_PAD(pad), B3, B2|B3); }

// set bus keep (weak pull up and down; pad = QSPI_PAD_*)
INLINE void QSPI_BusKeep(u8 pad) { RegSet(QSPI_PAD(pad), B2|B3); }

// enable schmitt trigger (use hysteresis on input; default state; pad = QSPI_PAD_*)
INLINE void QSPI_SchmittEnable(u8 pad) { RegSet(QSPI_PAD(pad), B1); }

// disable schmitt trigger (do not use hysteresis on input; pad = QSPI_PAD_*)
INLINE void QSPI_SchmittDisable(u8 pad) { RegClr(QSPI_PAD(pad), B1); }

// use slow slew rate control on output (default state; pad = QSPI_PAD_*)
INLINE void QSPI_Slow(u8 pad) { RegClr(QSPI_PAD(pad), B0); }

// use fast slew rate control on output (pad = QSPI_PAD_*)
INLINE void QSPI_Fast(u8 pad) { RegSet(QSPI_PAD(pad), B0); }

// init pad to default state (pad = QSPI_PAD_*) - slow slew rate, enable schmitt, pull-down, 4mA, in/out enable
INLINE void QSPI_PadInit(u8 pad) { *QSPI_PAD(pad) = B1|B2|B4|B6; }

// ==== QSPI pin control (use pins QSPI_PIN_*)

// get output (0 or 1) from peripheral, before override (pin = QSPI_PIN_*)
INLINE u8 QSPI_OutPeri(u8 pin) { return (u8)((*QSPI_STATUS(pin) >> 8) & 1); }

// get output (0 or 1) to pad, after override (pin = QSPI_PIN_*)
INLINE u8 QSPI_OutPad(u8 pin) { return (u8)((*QSPI_STATUS(pin) >> 9) & 1); }

// get output enable (0 or 1) from peripheral, before override (pin = QSPI_PIN_*)
INLINE u8 QSPI_OePeri(u8 pin) { return (u8)((*QSPI_STATUS(pin) >> 12) & 1); }

// get output enable (0 or 1) to pad, after override (pin = QSPI_PIN_*)
INLINE u8 QSPI_OePad(u8 pin) { return (u8)((*QSPI_STATUS(pin) >> 13) & 1); }

// get input (0 or 1) from pad, before override (pin = QSPI_PIN_*)
INLINE u8 QSPI_InPad(u8 pin) { return (u8)((*QSPI_STATUS(pin) >> 17) & 1); }

// get input (0 or 1) to peripheral, after override (pin = QSPI_PIN_*)
INLINE u8 QSPI_InPeri(u8 pin) { return (u8)((*QSPI_STATUS(pin) >> 19) & 1); }

// get interrupt (0 or 1) from pad, before override (pin = QSPI_PIN_*)
INLINE u8 QSPI_IrqPad(u8 pin) { return (u8)((*QSPI_STATUS(pin) >> 24) & 1); }

// get interrupt (0 or 1) to processor, after override (pin = QSPI_PIN_*)
INLINE u8 QSPI_IrqProc(u8 pin) { return (u8)((*QSPI_STATUS(pin) >> 26) & 1); }

// set Output pin override - normal (default state; pin = QSPI_PIN_*)
INLINE void QSPI_OutOverNormal(u8 pin) { RegClr(QSPI_CTRL(pin), B8|B9); }

// set Output pin override - invert (pin = QSPI_PIN_*)
INLINE void QSPI_OutOverInvert(u8 pin) { RegMask(QSPI_CTRL(pin), B8, B8|B9); }

// set Output pin override - low (pin = QSPI_PIN_*)
INLINE void QSPI_OutOverLow(u8 pin) { RegMask(QSPI_CTRL(pin), B9, B8|B9); }

// set Output pin override - high (pin = QSPI_PIN_*)
INLINE void QSPI_OutOverHigh(u8 pin) { RegSet(QSPI_CTRL(pin), B8|B9); }

// set Output enable pin override - normal (default state; pin = QSPI_PIN_*)
INLINE void QSPI_OEOverNormal(u8 pin) { RegClr(QSPI_CTRL(pin), B12|B13); }

// set Output enable pin override - invert (pin = QSPI_PIN_*)
INLINE void QSPI_OEOverInvert(u8 pin) { RegMask(QSPI_CTRL(pin), B12, B12|B13); }

// set Output enable pin override - disable (pin = QSPI_PIN_*)
INLINE void QSPI_OEOverDisable(u8 pin) { RegMask(QSPI_CTRL(pin), B13, B12|B13); }

// set Output enable pin override - enable (pin = QSPI_PIN_*)
INLINE void QSPI_OEOverEnable(u8 pin) { RegSet(QSPI_CTRL(pin), B12|B13); }

// set Input pin override - normal (default state; pin = QSPI_PIN_*)
INLINE void QSPI_InOverNormal(u8 pin) { RegClr(QSPI_CTRL(pin), B16|B17); }

// set Input pin override - invert (pin = QSPI_PIN_*)
INLINE void QSPI_InOverInvert(u8 pin) { RegMask(QSPI_CTRL(pin), B16, B16|B17); }

// set Input pin override - low (pin = QSPI_PIN_*)
INLINE void QSPI_InOverLow(u8 pin) { RegMask(QSPI_CTRL(pin), B17, B16|B17); }

// set Input pin override - high (pin = QSPI_PIN_*)
INLINE void QSPI_InOverHigh(u8 pin) { RegSet(QSPI_CTRL(pin), B16|B17); }

// set IRQ pin override - normal (default state; pin = QSPI_PIN_*)
INLINE void QSPI_IRQOverNormal(u8 pin) { RegClr(QSPI_CTRL(pin), B28|B29); }

// set IRQ pin override - invert (pin = QSPI_PIN_*)
INLINE void QSPI_IRQOverInvert(u8 pin) { RegMask(QSPI_CTRL(pin), B28, B28|B29); }

// set IRQ pin override - low (pin = QSPI_PIN_*)
INLINE void QSPI_IRQOverLow(u8 pin) { RegMask(QSPI_CTRL(pin), B29, B28|B29); }

// set IRQ pin override - high (pin = QSPI_PIN_*)
INLINE void QSPI_IRQOverHigh(u8 pin) { RegSet(QSPI_CTRL(pin), B28|B29); }

// set QSPI function QSPI_FNC_*, resets overrides, does not affect pad settings (pin = QSPI_PIN_*; default QSPI_FNC_NULL)
INLINE void QSPI_Fnc(u8 pin, u8 fnc) { *QSPI_CTRL(pin) = fnc; }

// ==== QSPI pin interrupt control (use pins QSPI_PIN_*)

// get raw interrupt status (for both CPU)
//   pin = pin QSPI_PIN_*
//   event = event QSPI_EVENT_*
INLINE u8 QSPI_IRQRaw(u8 pin, u8 event) { return (u8)((*QSPI_IRQ_INTR >> (4*pin + event)) & 1); }

// clear raw interrupt status - acknowledge interrupt (for both CPU)
//   pin = pin QSPI_PIN_*
//   event = event QSPI_EVENT_*
INLINE void QSPI_IRQAck(u8 pin, u8 event) { *QSPI_IRQ_INTR = BIT(4*pin + event); }

// enable IRQ interrupt for selected/current CPU
//   cpu = CPU core 0 or 1, use CpuID() to get current core
//   pin = QSPI_PIN_*
//   event = event QSPI_EVENT_*
INLINE void QSPI_IRQEnableCpu(u8 cpu, u8 pin, u8 event) { RegSet(QSPI_IRQ_INTE(cpu), BIT(4*pin + event)); }
INLINE void QSPI_IRQEnable(u8 pin, u8 event) { QSPI_IRQEnableCpu(CpuID(), pin, event); }

// disable IRQ interrupt for selected/current CPU (current state)
//   cpu = CPU core 0 or 1, use CpuID() to get current core
//   pin = QSPI_PIN_*
//   event = event QSPI_EVENT_*
INLINE void QSPI_IRQDisableCpu(u8 cpu, u8 pin, u8 event) { RegClr(QSPI_IRQ_INTE(cpu), BIT(4*pin + event)); }
INLINE void QSPI_IRQDisable(u8 pin, u8 event) { QSPI_IRQDisableCpu(CpuID(), pin, event); }

// check if IRQ interrupt is forced for selected/current CPU
//   cpu = CPU core 0 or 1, use CpuID() to get current core
//   pin = pin QSPI_PIN_*
//   event = event QSPI_EVENT_*
INLINE u8 QSPI_IRQIsForcedCpu(u8 cpu, u8 pin, u8 event) { return (u8)((*QSPI_IRQ_INTF(cpu) >> (4*pin + event)) & 1); }
INLINE u8 QSPI_IRQIsForced(u8 pin, u8 event) { return QSPI_IRQIsForcedCpu(CpuID(), pin, event); }

// force IRQ interrupt for selected/current CPU
//   cpu = CPU core 0 or 1, use CpuID() to get current core
//   pin = QSPI_PIN_*
//   event = event QSPI_EVENT_*
INLINE void QSPI_IRQForceCpu(u8 cpu, u8 pin, u8 event) { RegSet(QSPI_IRQ_INTF(cpu), BIT(4*pin + event)); }
INLINE void QSPI_IRQForce(u8 pin, u8 event) { QSPI_IRQForceCpu(CpuID(), pin, event); }

// clear force IRQ interrupt for selected/current CPU
//   cpu = CPU core 0 or 1, use CpuID() to get current core
//   pin = QSPI_PIN_*
//   event = event QSPI_EVENT_*
INLINE void QSPI_IRQUnforceCpu(u8 cpu, u8 pin, u8 event) { RegClr(QSPI_IRQ_INTF(cpu), BIT(4*pin + event)); }
INLINE void QSPI_IRQUnforce(u8 pin, u8 event) { QSPI_IRQUnforceCpu(CpuID(), pin, event); }

// check IRQ interrupt status for selected/current CPU (returns 1 if IRQ is pending)
//   core = CPU core 0 or 1, use CpuID() to get current core
//   pin = QSPI_PIN_*
//   event = event QSPI_EVENT_*
INLINE u8 QSPI_IRQIsPendingCpu(u8 cpu, u8 pin, u8 event) { return (u8)((*QSPI_IRQ_INTS(cpu) >> (4*pin + event)) & 1); }
INLINE u8 QSPI_IRQIsPending(u8 pin, u8 event) { return QSPI_IRQIsPendingCpu(CpuID(), pin, event); }

// === QSPI data

// get input pin (returns 0 or 1; pin = QSPI_PIN_*)
INLINE u8 QSPI_In(u8 pin) { return (u8)((*QSPI_IN >> pin) & 1); }

// get all QSPI input pins (bit 0..5 = pin QSPI_PIN_*)
INLINE u8 QSPI_InAll() { return (u8)*QSPI_IN; }

// output 0 to pin (pin = QSPI_PIN_*)
INLINE void QSPI_Out0(u8 pin) { *QSPI_OUT_CLR = BIT(pin); }

// output 1 to pin (pin = QSPI_PIN_*)
INLINE void QSPI_Out1(u8 pin) { *QSPI_OUT_SET = BIT(pin); }

// flip output to pin (pin = QSPI_PIN_*)
INLINE void QSPI_Flip(u8 pin) { *QSPI_OUT_XOR = BIT(pin); }

// output value to pin (val = 0 or val != 0; pin = QSPI_PIN_*)
INLINE void QSPI_Out(u8 pin, int val) { if (val == 0) QSPI_Out0(pin); else QSPI_Out1(pin); }

// output masked values to pins
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_OutMask(u8 mask, u8 value) { *QSPI_OUT_XOR = (*QSPI_OUT ^ value) & mask; }

// clear output pins masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_ClrMask(u8 mask) { *QSPI_OUT_CLR = mask; }

// set output pins masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_SetMask(u8 mask) { *QSPI_OUT_SET = mask; }

// flip output pins masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_FlipMask(u8 mask) { *QSPI_OUT_XOR = mask; }

// get last output value to pin (returns 0 or 1; pin = QSPI_PIN_*)
INLINE u8 QSPI_GetOut(u8 pin) { return (u8)((*QSPI_OUT >> pin) & 1); }

// get all last output values to pins
INLINE u8 QSPI_GetOutAll() { return (u8)*QSPI_OUT; }

// set output values to all pins
INLINE void QSPI_OutAll(u8 value) { *QSPI_OUT = value; }

// set output direction of pin (enable output mode; pin = QSPI_PIN_*)
INLINE void QSPI_DirOut(u8 pin) { *QSPI_OE_SET = BIT(pin); }

// set input direction of pin (disable output mode; pin = QSPI_PIN_*)
INLINE void QSPI_DirIn(u8 pin) { *QSPI_OE_CLR = BIT(pin); }

// set output direction of pin masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_DirOutMask(u8 mask) { *QSPI_OE_SET = mask; }

// set input direction of pin masked
// To use pin mask in range (first..last), use function RangeMask.
INLINE void QSPI_DirInMask(u8 mask) { *QSPI_OE_CLR = mask; }

// get output direction of pin (returns 0=input or 1=output; pin = QSPI_PIN_*)
INLINE u8 QSPI_GetDir(u8 pin) { return (u8)((*QSPI_OE >> pin) & 1); }

// get output direction of all pins (0=input, 1=output)
INLINE u8 QSPI_GetDirAll() { return (u8)*QSPI_OE; }

// initialize QSPI pins to connect Flash memory
void NOFLASH(QSPI_InitFlash)();

#ifdef __cplusplus
}
#endif

#endif // _SDK_QSPI_H
