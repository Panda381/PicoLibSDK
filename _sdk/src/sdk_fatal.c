
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

#include "../../global.h"	// globals
#include "../inc/sdk_fatal.h"
#include "../inc/sdk_irq.h"
#include "../../_display/_include.h"
#include "../../_lib/inc/lib_print.h"
#include "../../_lib/inc/lib_draw.h"
#include "../../_lib/inc/lib_text.h"

#if !RISCV

#if USE_FATALERROR			// use fatal error message 0=no, 1=display LCD message (sdk_fatal.c, sdk_fatal.h)

#define FATAL_COL	COL_WHITE	// foreground color
#define FATAL_BGCOL	COL_BLUE	// background color

int FatalErrorMsgY;
static void FatalErrorMsgText(const char* text)
{
	DispDrawTextRow(text, 0, FatalErrorMsgY, FATAL_COL, FATAL_BGCOL);
	FatalErrorMsgY += 16;		// next row
}

// fatal error message
void FatalErrorMsg(sFatalFrame* f)
{
	// setup font
	SelFont8x16();

	// start Y
	FatalErrorMsgY = 0;

	// prepare title - interrupt type
#define MSGBUFSIZE 40
	char buf[MSGBUFSIZE];
	const char* t;
	switch (f->irq)
	{
	case IRQ_NMI:		t = "Unhandled NMI interrupt";		break;
	case IRQ_HARDFAULT:	t = "Hard Fault Error"; 		break;
#if !RP2040
	case IRQ_MEMFAULT:	t = "Memory Management Fault"; 		break;
	case IRQ_BUSFAULT:	t = "Bus Memory Access Fault"; 		break;
	case IRQ_USAGEFAULT:	t = "Invalid Instruction"; 		break;
	case IRQ_SECUREFAULT:	t = "Secure Fault";	 		break;
	case IRQ_DEBMONITOR:	t = "Debug Monitor Fault"; 		break;
#endif // !RP2040
	case IRQ_SVCALL:	t = "Unhandled SVC Call";		break;
	case IRQ_PENDSV:	t = "Unhandled PendSV Call";		break;
	case IRQ_SYSTICK:	t = "Unhandled SysTick Interrupt";	break;
	default:
		MemPrint(buf, MSGBUFSIZE, "Unhandled Interrupt %d ", f->irq);
		t = buf;
		break;
	}
	int len = StrLen(t);
	FatalErrorMsgText(t);

	// registers
	MemPrint(buf, MSGBUFSIZE, "R0=%08X R1=%08X R2=%08X", f->r0, f->r1, f->r2);
	FatalErrorMsgText(buf);

	MemPrint(buf, MSGBUFSIZE, "R3=%08X R4=%08X R5=%08X", f->r3, f->r4, f->r5);
	FatalErrorMsgText(buf);

	MemPrint(buf, MSGBUFSIZE, "R6=%08X R7=%08X R8=%08X", f->r6, f->r7, f->r8);
	FatalErrorMsgText(buf);

	MemPrint(buf, MSGBUFSIZE, "R9=%08X R10=%08X R11=%08X", f->r9, f->r10, f->r11);
	FatalErrorMsgText(buf);

	MemPrint(buf, MSGBUFSIZE, "R12=%08X SP=%08X LR=%08X", f->r12, f->sp, f->lr);
	FatalErrorMsgText(buf);

#if RP2040

	// system registers
	MemPrint(buf, MSGBUFSIZE, "CONTROL=%08X", f->ctrl);
	FatalErrorMsgText(buf);

#else // RP2040

	// system registers
	MemPrint(buf, MSGBUFSIZE, "CONTROL=%08X CFSR=%08X", f->ctrl, f->cfsr);
	FatalErrorMsgText(buf);

	MemPrint(buf, MSGBUFSIZE, "HFSR=%08X DFSR=%08X", f->hfsr, f->dfsr);
	FatalErrorMsgText(buf);

	MemPrint(buf, MSGBUFSIZE, "MMFAR=%08X BFAR=%08X", f->mmfar, f->bfar);
	FatalErrorMsgText(buf);

#endif // RP2040

	// program counter
	MemPrint(buf, MSGBUFSIZE, "PC=%08X xPSR=%08X stack:", f->pc, f->xpsr);
	FatalErrorMsgText(buf);

	// check if stack pointer is valid
	u32 sp = f->sp;
	u32* s = (u32*)sp;
	int i;
#if RP2040
	for (i = 0; i < 7; i++)
	{
		if ((sp < 0x20000010) || (sp > 0x20042000-4*4) || ((sp & 3) != 0)) break;
#else
	for (i = 0; i < 5; i++)
	{
		if ((sp < 0x20000010) || (sp > 0x20082000-4*4) || ((sp & 3) != 0)) break;
#endif
		MemPrint(buf, MSGBUFSIZE, "%08X %08X %08X %08X", s[0], s[1], s[2], s[3]);
		FatalErrorMsgText(buf);
		sp += 4*4;
		s += 4;
	}
}

#else // USE_FATALERROR

// fatal error message
void FatalErrorMsg(sFatalFrame* f) {}

#endif // USE_FATALERROR

#endif // !RISCV
