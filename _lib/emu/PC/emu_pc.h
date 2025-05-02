
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

// Required config:
//  PC_RAM_PAGE_NUM ... number of 4 KB RAM pages
//  PC_RAM_BASE ... RAM base address (declared as u8*)
//  PC_ROM_BASE ... base address to load program to the flash (must be aligned to 4 KB)

#include "../../inc/lib_fat.h"
#include "../../../_devices/key.h"

// current CPU
extern sI8086* PC_Cpu;
extern int PC_Pwm; // used PWM controller
extern u32 PC_Freq; // used frequency
extern u32 PC_RealFreq; // real frequency

extern u32 PC_LastInt8; // time of last INT 08h timer service
extern volatile u32 PC_Int8Delta; // time interval in [us] of one INT 08h tick

// Key injection
#define PC_INJECT_NUM	8	// number of injection keys
extern u64 PC_StartTime;	// program start time in [us]
extern u32 PC_InjectTime[4*PC_INJECT_NUM]; // time of key injection
extern u8 PC_InjectKey[4*PC_INJECT_NUM]; // scan codes of key injection
extern u32 PC_InjectMask;	// mask of injected keys

// error codes
#define PC_ERROR_VRAM	-3	// error to allocate video-RAM
#define PC_ERROR_WRAM	-2	// no RAM to write to
#define PC_ERROR_OK	-1	// no error
#define PC_ERROR_DIV0	0	// divide by zero (INT 0)
#define PC_ERROR_STEP	1	// single-step interrupt (INT 1)
#define PC_ERROR_NMI	2	// non-maskable interrupt (INT 2)
#define PC_ERROR_BREAK	3	// break-point interrupt (INT 3)
#define PC_ERROR_OVER	4	// overflow error (INT 4)
#define PC_ERROR_BOUND	5	// bound limits overflow (INT 5)
#define PC_ERROR_INT	6	// 6 and more, unhandled interrupt
//  value 0..255: unhandled interrupt 0..255

#define PC_ERROR_MIN	-3	// printable error minimal value
#define PC_ERROR_MAX	6	// printable error maximal value
#define PC_ERROR_NUM	(PC_ERROR_MAX+1-PC_ERROR_MIN) // number of printable errors

#define PC_ERROR_TOTAL	(256 - PC_ERROR_MIN) // number of total errors (= 259)

// error, emulation interrupted
extern volatile int PC_Error; // error code (-1 = no error, ..-2= specified error, 0..255 = unhandled interrupt 0..255)
extern volatile u16 PC_ErrorSeg; // segment of error address 
extern volatile u16 PC_ErrorOff; // offset of error address
extern u8 PC_ErrorIgnore[(PC_ERROR_TOTAL+7)>>3]; // mask of ignored errors, 33 bytes (1=ignore)

// pause on exit
extern Bool PC_PauseExit;

// fatal error, break emulation (called from emulator callbacks)
//   error ... -1 = no error, ..-2= specified error, 0..255 = unhandled interrupt 0..255
//  Returns True to ignore error and continue emulation, False on abort emulation.
Bool PC_Fatal(int error, u16 seg, u16 off);

// set INT 08h timer interval
void PC_Int8SetDelta(u16 cnt);

// IBM PC font 8x8
extern const u8 PC_Font8[256*8];

// IBM PC font 8x14
extern const u8 PC_Font14[256*14];

// IBM PC font 8x16
extern const u8 PC_Font16[256*16];

#include "emu_pc_dos.h"		// DOS functions
#include "emu_pc_ext.h"		// INT 15h extension
#include "emu_pc_int.h"		// interrupts
#include "emu_pc_mem.h"		// memory service
#include "emu_pc_port.h"	// ports
#include "emu_pc_load.h"	// program loader
#include "emu_pc_video.h"	// video service, INT 10h
#include "emu_pc_lcd.h"		// output to LCD display
#include "emu_pc_menu.h"	// emulator menu
#include "emu_pc_snd.h"		// sound
#include "emu_pc_key.h"		// keyboard
#include "emu_pc_time.h"	// system time

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
extern ALIGNED FRAMETYPE FrameBuf2[];
#endif

// error handler menu - should be called on termination of emulator, if PC_Error != PC_ERROR_OK
// - Returns True if emulation continues
Bool PC_ErrorMenu();

// start emulation (program must be prepared in memory - use PC_LoadProg())
//  cpu ... pointer to CPU structure
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz
u32 PC_Start(sI8086* cpu, int pwm, u32 freq);

// stop/pause emulation
void PC_Stop();

// continue emulation
void PC_Cont();

// check if emulation is running
INLINE Bool PC_IsRunning() { return I8086_IsRunning(); }
