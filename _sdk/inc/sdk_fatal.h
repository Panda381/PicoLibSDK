
// ****************************************************************************
//
//                           Fatal error message
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

#ifndef _SDK_FATAL_H
#define _SDK_FATAL_H

#ifdef __cplusplus
extern "C" {
#endif

#if !RISCV

#if RP2040

// fatal error frame descriptor (on change update entries in crt0_*.S, too)
typedef struct {
	int	irq;			// 0x00: IRQ number IRQ_* (including system faults)
	u32	r4;			// 0x04: register R4
	u32	r5;			// 0x08: register R5
	u32	r6;			// 0x0C: register R6
	u32	r7;			// 0x10: register R7
	u32	r8;			// 0x14: register R8
	u32	r9;			// 0x18: register R9
	u32	r10;			// 0x1c: register R10
	u32	r11;			// 0x20: register R11
	u32	sp;			// 0x24: current stack pointer (R13)
	u32	ctrl;			// 0x28: CONTROL register
	u32	res;			// 0x2C: ... reserved (align)

// ARM exception fault stack frame:

	u32	r0;			// 0x30: register R0
	u32	r1;			// 0x34: register R1
	u32	r2;			// 0x38: register R2
	u32	r3;			// 0x3C: register R3
	u32	r12;			// 0x40: scratch register R12 (IPC)
	u32	lr;			// 0x44: linke register R14 (LR)
	u32	pc;			// 0x48: program counter PC (register R15)
	u32	xpsr;			// 0x4C: status register xPSR

// extended stack frame may follow if floating point is used, start pointed by "current stack pointer SP":
//  S0-S15, FPSCR

} sFatalFrame;

STATIC_ASSERT(sizeof(sFatalFrame) == 0x50, "Incorrect sFatalFrame!");

#else // RP2040

// fatal error frame descriptor (on change update entries in crt0_*.S, too)
typedef struct {
	int	irq;			// 0x00: IRQ number IRQ_* (including system faults)
	u32	r4;			// 0x04: register R4
	u32	r5;			// 0x08: register R5
	u32	r6;			// 0x0C: register R6
	u32	r7;			// 0x10: register R7
	u32	r8;			// 0x14: register R8
	u32	r9;			// 0x18: register R9
	u32	r10;			// 0x1c: register R10
	u32	r11;			// 0x20: register R11
	u32	sp;			// 0x24: current stack pointer (R13)
	u32	ctrl;			// 0x28: CONTROL register
	u32	cfsr;			// 0x2C: configurable flag status registers
	u32	hfsr;			// 0x30: Hard Fault Cause
	u32	dfsr;			// 0x34: debug event
	u32	mmfar;			// 0x38: memory fault address
	u32	bfar;			// 0x3C: bus fault address

// ARM exception fault stack frame:

	u32	r0;			// 0x40: register R0
	u32	r1;			// 0x44: register R1
	u32	r2;			// 0x48: register R2
	u32	r3;			// 0x4C: register R3
	u32	r12;			// 0x50: scratch register R12 (IPC)
	u32	lr;			// 0x54: linke register R14 (LR)
	u32	pc;			// 0x58: program counter PC (register R15)
	u32	xpsr;			// 0x5C: status register xPSR

// extended stack frame may follow if floating point is used, start pointed by "current stack pointer SP":
//  S0-S15, FPSCR

} sFatalFrame;

STATIC_ASSERT(sizeof(sFatalFrame) == 0x60, "Incorrect sFatalFrame!");

#endif

// fatal error message
void FatalErrorMsg(sFatalFrame* f);

#endif // !RISCV

#ifdef __cplusplus
}
#endif

#endif // _SDK_FATAL_H
