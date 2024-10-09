
// ****************************************************************************
//
//                                 CPU control
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
#include "../inc/sdk_cpu.h"
#include "../inc/sdk_bootrom.h"
#include "../../_lib/inc/lib_print.h"

// Work-around of compiler error
void _close_r(void) {}
void _lseek_r(void) {}
void _read_r(void) {}
void _write_r(void) {}

#if !RP2040
// temporary saved result of divider emulation
u32 DivModResult_Div[CORE_NUM];
u32 DivModResult_Mod[CORE_NUM];
#endif

#if RP2040
// reverse bit table
const u8 RevBitsTab[16] = {
	0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E,
	0x01, 0x09, 0x05, 0x0D, 0x03, 0x0B, 0x07, 0x0F };

// order bit table
//                          0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
const u8 OrdBitsTab[16] = { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };

// reverse 8 bits (takes 170 ns)
u8 Reverse8(u8 val)
{
	return RevBitsTab[val >> 4] | (RevBitsTab[val & 0xf] << 4);
}

// get bit order of 8-bit value (takes 152 ns) (logarithm, returns position of highest bit + 1: 1..8, 0=no bit)
u8 Order8(u8 val)
{
	u8 i = 0;
	if (val >= 0x10)
	{
		i += 4;
		val >>= 4;
	}
	return OrdBitsTab[val] + i;
}
#endif

#if USE_STACKCHECK	// use Stack check (sdk_cpu.c, sdk_cpu.h)

// feed stack of current CPU core with the food (returns free space, or 0 on stack error)
u32 StackFeed()
{
	// prepare stack pointers
	u32 bot = (((u32)StackBottom() + 3) & ~3); // stack bottom
	u32 top = ((u32)StackTop() & ~3); // stack top
	u32 sp = (u32)GetSp(); // stack pointer

	// check stack pointer
	if ((sp < bot) || (sp > top)) return 0;

	// feed stack
	u32* s = (u32*)bot;
	int n = (sp - bot)/4;
	for (; n > 0; n--) *s++ = 0x600DF00D; // fill with pattern "Good Food"

	return sp - bot;
}

// check stack limit of current CPU core (check used stack space)
u32 StackCheck()
{
	u32 bot = (((u32)StackBottom() + 3) & ~3); // stack bottom
	u32* s = (u32*)bot;
	while (*s == 0x600DF00D) s++; // check pattern "Good Food"
	return (u32)s - bot;
}

#endif // USE_STACKCHECK	// use Stack check (sdk_cpu.c, sdk_cpu.h)

// fatal error - display panic message and halt execution
void __attribute__((noreturn)) panic(const char *fmt, ...)
{
	va_list args;

	// print start of panic
	PrintText("\n*** PANIC ***\n");

	// print message
	if (fmt)
	{
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}

	// print end of panic
	PrintText("\n*************\n");

	// stop execution
	FatalErrorStop();
}
