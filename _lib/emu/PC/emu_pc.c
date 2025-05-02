
// ****************************************************************************
//
//                               PC Emulator
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

// >>> Keep tables and functions in RAM (without "const") for faster access.

#include "../../../_sdk/inc/sdk_timer.h"
#include "../../../_sdk/inc/sdk_systick.h"
#include "../../../_sdk/inc/sdk_flash.h"
#include "../../../_sdk/inc/sdk_clocks.h"
#include "../../inc/lib_alarm.h"
#include "../../inc/lib_decnum.h"
#include "../../inc/lib_draw.h"
#include "../../inc/lib_text.h"
#include "../../inc/lib_rand.h"
#include "../../inc/lib_pwmsnd.h"
#include "../../../_display/_include.h"
#include "../../../_devices/picopad/_include.h"
#include "../../../_sdk/usb_inc/sdk_usb_hid.h"
#include "../../../_sdk/usb_inc/sdk_usb_host_hid.h"

// current CPU
sI8086* PC_Cpu = NULL;
int PC_Pwm; // used PWM controller
u32 PC_Freq; // used frequency
u32 PC_RealFreq; // real frequency

// INT 08h timer
Bool PC_Int8IsReg = False; // INT 08h handler is registered
u32 PC_LastInt8; // time of last INT 08h timer service
sAlarm PC_AlarmInt8; // INT 08h alarm structure

#define PC_INT8_DELTA	54926		// Timer INT 08h interval in [us] (should be 0.0549255 sec)
volatile u32 PC_Int8Delta; // time interval in [us] of one INT 08h tick

// Key injection
u64 PC_StartTime;	// program start time in [us]
u32 PC_InjectTime[4*PC_INJECT_NUM]; // time of key injection
u8 PC_InjectKey[4*PC_INJECT_NUM]; // scan codes of key injection, B7 = release
u32 PC_InjectMask = 0;	// mask of injected keys

// error, emulation interrupted
volatile int PC_Error; // error code (-1 = no error, ..-2= specified error, 0..255 = unhandled interrupt 0..255)
volatile u16 PC_ErrorSeg; // segment of error address 
volatile u16 PC_ErrorOff; // offset of error address
u8 PC_ErrorIgnore[(PC_ERROR_TOTAL+7)>>3]; // mask of ignored errors, 33 bytes (1=ignore)

// pause on exit
Bool PC_PauseExit;

// fonts
#include "emu_pc_font8.c"	// IBM PC font 8x8 const: u8 PC_Font8[256*8]
#include "emu_pc_font14.c"	// IBM PC font 8x14 const: u8 PC_Font14[256*14]
#include "emu_pc_font16.c"	// IBM PC font 8x16 const: u8 PC_Font16[256*16]

#include "emu_pc_dos.c"		// DOS functions
#include "emu_pc_ext.c"		// INT 15h extension
#include "emu_pc_int.c"		// interrupts
#include "emu_pc_load.c"	// program loader
#include "emu_pc_mem.c"		// memory service
#include "emu_pc_port.c"	// ports
#include "emu_pc_video.c"	// video service, INT 10h
#include "emu_pc_lcd.c"		// output to LCD display
#include "emu_pc_menu.c"	// emulator menu
#include "emu_pc_snd.c"		// sound
#include "emu_pc_key.c"		// keyboard
#include "emu_pc_time.c"	// system time

// fatal error, break emulation (called from emulator callbacks)
//   error ... -1 = no error, ..-2= specified error, 0..255 = unhandled interrupt 0..255
//  Returns True to ignore error and continue emulation, False on abort emulation.
Bool PC_Fatal(int error, u16 seg, u16 off)
{
	// ignore this error
	if ((error < PC_ERROR_MIN) || (error > 255)) return True; // ignore unknown error
	int n = error - PC_ERROR_MIN; // error code relative
	if ((PC_ErrorIgnore[n >> 3] & BIT(n & 7)) != 0) return True; // ignore this error

	PC_Error = error;
	PC_ErrorSeg = seg;
	PC_ErrorOff = off;
	sI8086* cpu = PC_Cpu;
	if (cpu != NULL) cpu->stop = 1;

	return False;
}

// INT 08h timer and INT 09h keyboard service (should be called at least every 55 ms or more often)
//  The INT 08h timer of the emulator cannot be reprogrammed to a different interrupt rate.
void PC_Int8Handler(sAlarm* alarm)
{
	u32 last = PC_LastInt8;
	u32 delta = PC_Int8Delta;
	if ((u32)(Time() - last) >= delta)
	{
		do { last += delta; } while ((u32)(Time() - last) >= delta);
		PC_LastInt8 = last;

		sI8086* cpu = PC_Cpu;
		if (cpu != NULL) I8086_RaiseIRQ(cpu, I8086_IRQ_INT08);
	}

	// key injection
	u32 m = PC_InjectMask;
	if ((m != 0) && (PC_Int9KeyNum == 0))
	{
		// delta time
		u32 t = Time() - *(u32*)&PC_StartTime;
		int i = 0;
		do {
			// this key is required
			if ((m & B0) != 0)
			{
				// time is up
				if (t >= PC_InjectTime[i])
				{
					// clear key request
					PC_InjectMask &= ~BIT(i);

					// get key
					u8 scan = PC_InjectKey[i];

					// time of last key
					PC_Int9KeyTime = Time();

					// write scan code
					PC_Int9KeyScan[0] = scan;
					dmb(); // data memory barrier
					PC_Int9KeyNum = 1;

					// raise INT 09h
					I8086_RaiseIRQ(PC_Cpu, I8086_IRQ_INT09);
					break;
				}
			}

			// increase key index
			i++;

			// shift mask
			m >>= 1;

		} while (m != 0);
	}

	// keyboard handler
	PC_Int9Handler();
}

// set INT 08h timer interval
void PC_Int8SetDelta(u16 cnt)
{
	u32 d = (PC_INT8_DELTA * (cnt + 1)) >> 16;
	if (d < 1000) d = 1000; // min. 1 ms
	PC_Int8Delta = d;
}

// colors
#define PC_ERROR_COL_TITLE	PC_COLOR(PC_RED, PC_YELLOW)	// title
#define PC_ERROR_COL_TEXT	PC_COLOR(PC_BLACK, PC_WHITE)	// text
#define PC_ERROR_COL_SUBTEXT	PC_COLOR(PC_BLACK, PC_LTGRAY)	// sub-text
#define PC_ERROR_COL_HELP	PC_COLOR(PC_BLACK, PC_CYAN)	// help

// error texts
const char* PC_ErrorTxt[PC_ERROR_NUM] = {
	//2345678901234567890<--- max width
	" Lack of Video-RAM",	// PC_ERROR_VRAM  -3 error to allocate video-RAM
	"Lack of RAM to Write",	// PC_ERROR_WRAM  -2 no RAM to write to
	"",			// PC_ERROR_OK	  -1 no error
	"Divide by Zero INT 0",	// PC_ERROR_DIV0  0 divide by zero (INT 0)
	"  Single-Step INT 1",	// PC_ERROR_STEP  1 single-step interrupt (INT 1)
	" Non-Maskable INT 2",	// PC_ERROR_NMI   2 non-maskable interrupt (INT 2)
	"  Break-Point INT 3",	// PC_ERROR_BREAK 3 break-point interrupt (INT 3)
	"Overflow Error INT 4",	// PC_ERROR_OVER  4 overflow error (INT 4)
	" Bound Limits INT 5",	// PC_ERROR_BOUND 5 bound limits overflow (INT 5)
	"Unhandled Interrupt",	// PC_ERROR_INT   6 and more, unhandled interrupt
};

// error handler menu - should be called on termination of emulator, if PC_Error != PC_ERROR_OK
// - Returns True if emulation continues
Bool PC_ErrorMenu()
{
	// enter menu
	PC_MenuEnter();

	// unregister alarm
	PC_Stop();

	if (PC_Error == PC_ERROR_OK)
	{
		// unregister alarm
		PC_MenuLeave(False);

		// no error - should display program result screen?
		sPC_Vmode* m = &PC_Vmode;
		if (PC_PauseExit && // pause on exit?
			(m->vclass == PC_VCLASS_TEXT) && // text mode only
			(m->page == 0) && // only page 0
			((m->cur[0].curx != 0) || (m->cur[0].cury != 0))) // cursor not home
		{
			PC_LCDRedraw(); // redraw screen
			WaitMs(200); // short wait
			KeyFlush(); // flush keys

			for (;;)
			{
				if (KeyGet() != NOKEY) break;
#if USE_USB_HOST_HID
				if (UsbGetKey() != 0) break;
#endif
			}
		}
		return False; // no error, stop emulation
	}

	// print title
	PC_MenuNL(1);
	PC_MenuSpc(3, 0);
	PC_MenuText(" FATAL ERROR ", PC_ERROR_COL_TITLE);

	// print reason text
	PC_MenuNL(4);
	int inx = PC_Error;
	if (inx > PC_ERROR_INT) inx = PC_ERROR_INT;
	PC_MenuText(PC_ErrorTxt[inx - PC_ERROR_MIN], PC_ERROR_COL_TEXT);
	PC_MenuNL(1);

	// print interrupt number "   Interrupt 0x23   "
	if (PC_Error >= 0)
	{
		PC_MenuNL(1);
		PC_MenuText("   Interrupt 0x", PC_ERROR_COL_SUBTEXT);
		DecHexNum(DecNumBuf, PC_Error, 2);
		PC_MenuText(DecNumBuf, PC_ERROR_COL_SUBTEXT);
	}

	// print error address "  at 0x00F0:0x0123  "
	PC_MenuNL(1);
	PC_MenuText("  at 0x", PC_ERROR_COL_SUBTEXT);
	DecHexNum(DecNumBuf, PC_ErrorSeg, 4);
	PC_MenuText(DecNumBuf, PC_ERROR_COL_SUBTEXT);
	PC_MenuText(":0x", PC_ERROR_COL_SUBTEXT);
	DecHexNum(DecNumBuf, PC_ErrorOff, 4);
	PC_MenuText(DecNumBuf, PC_ERROR_COL_SUBTEXT);

	// print help
	PC_MenuNL(3);
	PC_MenuText("X ... exit\n", PC_ERROR_COL_HELP);
	PC_MenuText("A ... continue\n", PC_ERROR_COL_HELP);
	PC_MenuText("B ... ignore more\n", PC_ERROR_COL_HELP);

	// wait for a key
	WaitMs(200);
	u8 ch;
	for (;;)
	{
		// redraw screen
		PC_LCDRedraw();

		// get key
		ch = KeyGet();

		switch (ch)
		{
		// X: exit
		case KEY_X:
			PC_PauseExit = False;
			PC_MenuLeave(False);
			return False;

		// B: ignore more
		case KEY_B:
			{
				int n = PC_Error - PC_ERROR_MIN; // error code relative
				PC_ErrorIgnore[n >> 3] |= BIT(n & 7); // ignore this error
			}
		// A: continue
		case KEY_A:
			PC_MenuLeave(True);
			return True;
		}

#if USE_USB_HOST_HID
		// get USB key
		ch = UsbGetKey() & 0xff;

		switch (ch)
		{
		// X: exit
		case HID_KEY_X:
			PC_PauseExit = False;
			PC_MenuLeave(False);
			return False;

		// B: ignore more
		case HID_KEY_B:
			{
				int n = PC_Error - PC_ERROR_MIN; // error code relative
				PC_ErrorIgnore[n >> 3] |= BIT(n & 7); // ignore this error
			}
		// A: continue
		case HID_KEY_A:
			PC_MenuLeave(True);
			return True;
		}
#endif
	}
}

// start emulation (program must be prepared in memory - use PC_LoadProg())
//  cpu ... pointer to CPU structure
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz
u32 PC_Start(sI8086* cpu, int pwm, u32 freq)
{
	// error, emulation interrupted
	PC_Error = PC_ERROR_OK;
	memset(PC_ErrorIgnore, 0, sizeof(PC_ErrorIgnore)); // clear mask of ignored errors

	// initialize I8086 table
	I8086_InitTab();

	// save current CPU
	PC_Cpu = cpu;
	PC_Pwm = pwm;
	PC_Freq = freq;

	// setup sound output to default state
	PC_SoundDef();

	// setup timer to default state (should be called on program start)
	PC_TimerDef();

	// processing trap
	PC_IntTrapEnter = False;

	// extended character buffer (0 = no character)
	PC_Int21fncBuf = 0;

	// number of characters in input buffer AH=0Ah
	PC_Int21InNum = 0;

	// flag - waiting in fuction 86h
	PC_Int15Wait = False;

	// pause on exit
	PC_PauseExit = True;

	// setup
	cpu->readmem = PC_GetMem;	// read byte from memory
	cpu->writemem = PC_SetMem;	// write byte to memory
	cpu->readport8 = PC_GetPort;	// read byte from port
	cpu->writeport8 = PC_SetPort;	// write byte to port
	cpu->readport16 = PC_GetPort16;	// read word from port
	cpu->writeport16 = PC_SetPort16; // write word to port

#if USE_EMUSCREENSHOT		// use emulator screen shots
	DoEmuScreenShot = False;	// request to do emulator screenshot
#endif

	// reset CPU
	I8086_Reset(cpu);

	// segment of end of DOS allocated memory
	PC_AllocEnd = PC_PSP_SEG + PC_RomSize/16 + 0x280;

	// setup registers
	int f = (PC_VmemPageRAM - PC_NextPageRAM) << PC_PAGE_SHIFT; // empty RAM
	f = f*3/4; // use max. 3/4 free RAM
	f += PC_RomSize + 0x100; // add program size + stack reserve
	f &= ~1; // align
	cpu->ds = PC_PSP_SEG;
	cpu->es = PC_PSP_SEG;
	if (PC_ExeFileOK) // EXE program
	{
		cpu->cs = PC_ExeFile.init_cs + PC_PROG_SEG;
		cpu->ss = PC_ExeFile.init_ss + PC_PROG_SEG;
		cpu->ip = PC_ExeFile.init_ip;
		cpu->sp = PC_ExeFile.init_sp;
		PC_AllocEnd += PC_ExeFile.minmem;
	}

	// COM program
	else
	{
		cpu->cs = PC_PSP_SEG;
		cpu->ss = PC_PSP_SEG;
		cpu->ip = 0x0100;
		if (f > 0xfffe) f = 0xfffe; // limit to this segment
		cpu->sp = (u16)f;
	}

	cpu->flags |= I8086_IF; // enable interrupts

	// Initialize PSP (Program Segment Prefix)
	//  endseg ... segment of address beyond program
	PC_InitPSP(((f + 16) >> 4) + PC_PSP_SEG);

	// close DOS files
	PC_CloseFiles();

	// program start time
	PC_StartTime = Time64();

	// flush keyboard buffer
	KeyFlush();

	// reset keyboard handler (on start of emulation)
	PC_Int9Reset();

	// INT 08h timer handler
	PC_LastInt8 = Time(); // last INT 08h timer service
	PC_AlarmInt8.time = SysTime + SYSTICK_MS; // next time to alarm
	PC_AlarmInt8.delta = SYSTICK_MS; // repeat time in [ms]
	PC_AlarmInt8.callback = PC_Int8Handler; // callback
	PC_Int8Delta = PC_INT8_DELTA; // INT 08h interval in [us]

	// push address to return to CS:0 to exit program (this will allocate RAM page)
	I8086_PUSH(cpu, (cpu->flags & I8086_FLAGALLCTRL) | I8086_FLAGHW); // push flags
	I8086_PUSH(cpu, cpu->cs); // push current program segment
	I8086_PUSH(cpu, 0); // push reset to INT 20h

	// start emulation
	freq = I8086_Cont(cpu, pwm, freq);
	PC_RealFreq = freq;

	// register INT 08h handler
	if (!PC_Int8IsReg)
	{
		PC_Int8IsReg = True; // handler is registered
		AlarmReg(&PC_AlarmInt8); // register alarm
	}

	return freq;
}

// stop/pause emulation
void PC_Stop()
{
	KeyFlush();
	PC_SoundOutSave = PC_SoundOut;
	PC_SoundTerm(); // stop sound
	if (PC_Int8IsReg)
	{
		AlarmUnreg(&PC_AlarmInt8); // unregister alarm
		PC_Int8IsReg = False; // handler is not registered
	}
	sI8086* cpu = PC_Cpu;
	if (cpu != NULL) I8086_Stop(PC_Pwm);
}

// continue emulation
void PC_Cont()
{
	PC_Error = PC_ERROR_OK;
	PC_IntTrapEnter = False;

	// reset keyboard handler (on start of emulation)
	KeyFlush();
	PC_Int9Reset();

	// pause on exit
	PC_PauseExit = True;

#if USE_EMUSCREENSHOT		// use emulator screen shots
	DoEmuScreenShot = False;	// request to do emulator screenshot
#endif

	// INT 08h timer handler
	PC_LastInt8 = Time(); // last INT 08h timer service
	PC_AlarmInt8.time = SysTime + SYSTICK_MS; // next time to alarm
	PC_AlarmInt8.delta = SYSTICK_MS; // repeat time in [ms]
	PC_AlarmInt8.callback = PC_Int8Handler; // callback

	PC_SoundInit(PC_SoundOutSave);

	sI8086* cpu = PC_Cpu;
	if (cpu != NULL) I8086_Cont(PC_Cpu, PC_Pwm, PC_Freq);

	// register INT 08h handler
	if (!PC_Int8IsReg)
	{
		PC_Int8IsReg = True; // handler is registered
		AlarmReg(&PC_AlarmInt8); // register alarm
	}
}
