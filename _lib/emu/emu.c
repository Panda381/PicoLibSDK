
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

#include "../../global.h"	// globals

#if USE_EMU			// use emulators

#include "../../_sdk/inc/sdk_pwm.h"
#include "../../_sdk/inc/sdk_multicore.h"
#include "../../_display/disphstx/disphstx.h"

#include "emu.h"

#if EMU_DEBUG_SYNC	// 1 = debug measure time synchronization
sEmuDebClock EmuDebClock;
#endif // EMU_DEBUG_SYNC

// ----------------------------------------------------------------------------
//                             Time synchronization
// ----------------------------------------------------------------------------

// initialize time synchronization (initialize PWM counter; returns real frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required frequency in Hz (for 125 MHz system clock supported range: 488 kHz to 125 MHz)
u32 EmuSyncInit(sEmuSync* s, int pwm, u32 freq)
{
	// reset PWM to default state
	PWM_Reset(pwm);

	// set PWM frequency
	PWM_Clock(pwm, freq);

	// set period to 16 bits
	PWM_Top(pwm, 0xffff);

	// enable PWM
	PWM_Enable(pwm);

	// save pointer to PWM counter
	s->timer = PWM_CTR(pwm);

	// reset interrupt bit register
	EmuInterSet(s, 0);

	// start synchronization
	EmuSyncStart(s);

	// return real frequency
	return PWM_GetClock(pwm);
}

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
void EmuSyncTerm(int pwm)
{
	// reset PWM to default state
	PWM_Reset(pwm);
}

// ----------------------------------------------------------------------------
//                               CPU emulators
// ----------------------------------------------------------------------------

#if USE_EMU_I4004		// use I4004 CPU emulator
#include "emu_i4004.c"		// I4004 CPU
#endif

#if USE_EMU_I4040		// use I4040 CPU emulator
#include "emu_i4040.c"		// I4040 CPU
#endif

#if USE_EMU_I8008		// use I8008 CPU emulator
#include "emu_i8008.c"		// I8008 CPU
#endif

#if USE_EMU_I8048		// use I8048 CPU emulator
#include "emu_i8048.c"		// I8048 CPU
#endif

#if USE_EMU_I8052		// use I8052 CPU emulator
#include "emu_i8052.c"		// I8052 CPU
#endif

#if USE_EMU_I8080		// use I8080 CPU emulator
#include "emu_i8080.c"		// I8080 CPU
#endif

#if USE_EMU_I8085		// use I8085 CPU emulator
#include "emu_i8085.c"		// I8085 CPU
#endif

#if USE_EMU_I8086		// use I8086/I8088 CPU emulator
#include "emu_i8086.c"		// I8086/I8088 CPU
#endif

#if USE_EMU_M6502		// use M6502 CPU emulator
#include "emu_m6502.c"		// M6502 CPU
#endif

#if USE_EMU_X80			// use X80 CPU emulator
#include "emu_x80.c"		// X80 CPU
#endif

#if USE_EMU_Z80			// use Z80 CPU emulator
#include "emu_z80.c"		// Z80 CPU
#endif

// ----------------------------------------------------------------------------
//                             Device emulators
// ----------------------------------------------------------------------------

#if USE_EMU_PC			// use PC emulator
#include "pc/emu_pc.c"		// PC emulator
#endif

#if USE_EMU_GB			// use Game Boy emulator
#include "gb/emu_gb.c"		// Game Boy emulator
#endif

#if USE_EMU_NES			// use NES emulator
#include "nes/emu_nes.c"	// NES emulator
#endif

#endif // USE_EMU
