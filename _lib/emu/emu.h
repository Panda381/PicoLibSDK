
// ****************************************************************************
//
//                                 Emulators
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

/*
Optimization note: For maximum emulation speed, the *_Exec() function is set to optimize
for maximum speed using attribute "__attribute__ ((optimize("-Ofast")))" and is placed
in RAM using the "__attribute__((section(".time_critical." #fnc)))" attribute.
The instructions are handled by the switch(op) statement - the compiler normally uses
a jump table, which it places in the .rodata section, which is normally placed in Flash
memory (except for optimization -Os, which uses a table of 16-bit offsets in RAM). For
the compiler to place the jump table in RAM, this is provided in the linker script. Only
the common .rodata.* symbols are placed in the .rodata section. The jump table is named
.rodata (no name) and is placed in RAM:
 *(EXCLUDE_FILE(*libgcc.a: *libc.a:*lib_a-mem*.o *libm.a:) .rodata.*).
*/

#if USE_EMU			// use emulators

#include "../../_sdk/sdk_addressmap.h"

#ifndef _EMU_H
#define _EMU_H

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
//                             Time synchronization
// ----------------------------------------------------------------------------

#ifndef EMU_DEBUG_SYNC
#define EMU_DEBUG_SYNC	0	// 1 = debug measure time synchronization
#endif

#if EMU_DEBUG_SYNC	// 1 = debug measure time synchronization
typedef struct {
	u32		old;		// old clock counter
	volatile u32	used;		// sum of used clock intervals
	volatile u32	total;		// sum of total clock intervals
	volatile u32	maxused;	// max used clock interval
	volatile u32	maxtot;		// total clock interval on max
	volatile u8	reset;		// request to reset measure
} sEmuDebClock;
extern sEmuDebClock EmuDebClock;
#endif // EMU_DEBUG_SYNC

// time synchronization
typedef struct {
	u32		clock;		// clock counter
	volatile u32*	timer;		// pointer to timer
} sEmuSync;

STATIC_ASSERT(sizeof(sEmuSync) == 0x08, "Incorrect sEmuSync!");

// initialize time synchronization (initialize PWM counter; returns real frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required frequency in Hz (for 125 MHz system clock supported range: 488 kHz to 125 MHz)
u32 EmuSyncInit(sEmuSync* s, int pwm, u32 freq);

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
void EmuSyncTerm(int pwm);

// start time synchronization (= sets "time 0"; should be called before first program execution)
INLINE void EmuSyncStart(sEmuSync* s)
{
	s->clock = *s->timer;
#if EMU_DEBUG_SYNC	// 1 = debug measure time synchronization
	EmuDebClock.reset = True; // request to reset measure
#endif
}

// time synchronization
INLINE void EmuSync(sEmuSync* s)
{
	u32 c = s->clock;
	volatile u32* t = s->timer;

#if EMU_DEBUG_SYNC	// 1 = debug measure time synchronization

	sEmuDebClock* ec = &EmuDebClock;

	u32 clockused = ec->used;
	u32 clocktotal = ec->total;

	// reset measure
	if (ec->reset)
	{
		ec->reset = False;
		ec->maxused = 0;
		clockused = 0;
		clocktotal = 0;
	}
	else
	{
		u32 clockold = ec->old;

		// total
		u16 total = (u16)(c - clockold);
		clocktotal += total;
		u16 used = (u16)(*t - clockold);
		clockused += used;

		// overflow, rollback
		if ((u32)used > clockused)
		{
			clocktotal -= total;
			clockused -= used;
		}

		// max
		if (used > ec->maxused)
		{
			ec->maxused = used;
			ec->maxtot = total;
		}
	}

	ec->used = clockused;
	ec->total = clocktotal;

	// new start interval
	ec->old = c;

#endif // EMU_DEBUG_SYNC

	if ((s16)(c - *t) < -20000)
		s->clock = *t; // time underflow, emulation is too slow
	else
		while ((s16)(c - *t) > 0) {}
}

// ----------------------------------------------------------------------------
//                          Interrupt control
// ----------------------------------------------------------------------------
// Interrupt flags can be used to atomically control interrupts between
// the main program (on CPU core 0) and the emulator (on CPU core 1).

// get interrupt register (register of PWM, which can be used to exclusive atomic bit access between cores)
//  Initialized to 0.
INLINE u32 EmuInterGet(sEmuSync* s) { return s->timer[1]; }

// set value of interrupt register
INLINE void EmuInterSet(sEmuSync* s, u32 val) { s->timer[1] = val; }

// set bits masked in interrupt register
INLINE void EmuInterSetMask(sEmuSync* s, u32 mask) { RegSet(&s->timer[1], mask); }

// clear bits masked in interrupt register
INLINE void EmuInterClrMask(sEmuSync* s, u32 mask) { RegClr(&s->timer[1], mask); }

// flip bits masked in interrupt register
INLINE void EmuInterFlipMask(sEmuSync* s, u32 mask) { RegXor(&s->timer[1], mask); }

// set bit in interrupt register
INLINE void EmuInterSetBit(sEmuSync* s, int bit) { RegSet(&s->timer[1], BIT(bit)); }

// clear bit in interrupt register
INLINE void EmuInterClrBit(sEmuSync* s, int bit) { RegClr(&s->timer[1], BIT(bit)); }

// flip bit in interrupt register
INLINE void EmuInterFlipBit(sEmuSync* s, int bit) { RegXor(&s->timer[1], BIT(bit)); }

// ----------------------------------------------------------------------------
//                          Callback functions
// ----------------------------------------------------------------------------

// --- no parameters, return Bool
typedef Bool (*pEmuBool)();

// --- no parameters, no return
typedef void (*pEmuVoid)();

// --- without address

// function to read byte from memory or port (without address)
typedef u8 (*pEmuRead8)();

// function to write byte to memory or port (without address)
typedef void (*pEmuWrite8)(u8 data);

// --- 8-bit address

// function to read byte from memory or port (8-bit address)
typedef u8 (*pEmu8Read8)(u8 addr);

// function to read word from memory or port (8-bit address)
typedef u16 (*pEmu8Read16)(u8 addr);

// function to write byte to memory or port (8-bit address)
typedef void (*pEmu8Write8)(u8 addr, u8 data);

// function to write word to memory or port (8-bit address)
typedef void (*pEmu8Write16)(u8 addr, u16 data);

// --- 16-bit address

// function to read byte from memory or port (16-bit address)
typedef u8 (*pEmu16Read8)(u16 addr);

// function to read word from memory or port (16-bit address)
typedef u16 (*pEmu16Read16)(u16 addr);

// function to read dword from memory or port (16-bit address)
typedef u32 (*pEmu16Read32)(u16 addr);

// function to write byte to memory or port (16-bit address)
typedef void (*pEmu16Write8)(u16 addr, u8 data);

// function to write word to memory or port (16-bit address)
typedef void (*pEmu16Write16)(u16 addr, u16 data);

// function to write dword to memory or port (16-bit address)
typedef void (*pEmu16Write32)(u16 addr, u32 data);

// --- 32-bit address

// function to read byte from memory or port (32-bit address)
typedef u8 (*pEmu32Read8)(u32 addr);

// function to read word from memory or port (32-bit address)
typedef u16 (*pEmu32Read16)(u32 addr);

// function to read dword from memory or port (32-bit address)
typedef u32 (*pEmu32Read32)(u32 addr);

// function to write byte to memory or port (32-bit address)
typedef void (*pEmu32Write8)(u32 addr, u8 data);

// function to write word to memory or port (32-bit address)
typedef void (*pEmu32Write16)(u32 addr, u16 data);

// function to write dword to memory or port (32-bit address)
typedef void (*pEmu32Write32)(u32 addr, u32 data);

// ----------------------------------------------------------------------------
//                               CPU emulators
// ----------------------------------------------------------------------------

#if USE_EMU_I4004		// use I4004 CPU emulator
#include "emu_i4004.h"		// I4004 CPU
#endif

#if USE_EMU_I4040		// use I4040 CPU emulator
#include "emu_i4040.h"		// I4040 CPU
#endif

#if USE_EMU_I8008		// use I8008 CPU emulator
#include "emu_i8008.h"		// I8008 CPU
#endif

#if USE_EMU_I8048		// use I8048 CPU emulator
#include "emu_i8048.h"		// I8048 CPU
#endif

#if USE_EMU_I8052		// use I8052 CPU emulator
#include "emu_i8052.h"		// I8052 CPU
#endif

#if USE_EMU_I8080		// use I8080 CPU emulator
#include "emu_i8080.h"		// I8080 CPU
#endif

#if USE_EMU_I8085		// use I8085 CPU emulator
#include "emu_i8085.h"		// I8085 CPU
#endif

#if USE_EMU_I8086		// use I8086/I8088 CPU emulator
#include "emu_i8086.h"		// I8086/I8088 CPU
#endif

#if USE_EMU_M6502		// use M6502 CPU emulator
#include "emu_m6502.h"		// M6502 CPU
#endif

#if USE_EMU_X80			// use X80 CPU emulator
#include "emu_x80.h"		// X80 CPU
#endif

#if USE_EMU_Z80			// use Z80 CPU emulator
#include "emu_z80.h"		// Z80 CPU
#endif

// ----------------------------------------------------------------------------
//                             Device emulators
// ----------------------------------------------------------------------------

#if USE_EMU_PC			// use PC emulator
#include "pc/emu_pc.h"		// PC emulator
#endif

#if USE_EMU_GB			// use Game Boy emulator
#include "gb/emu_gb.h"		// Game Boy emulator
#endif

#if USE_EMU_NES			// use NES emulator
#include "nes/emu_nes.h"	// NES emulator
#endif

#ifdef __cplusplus
}
#endif

#endif // _EMU_H

#endif // USE_EMU
