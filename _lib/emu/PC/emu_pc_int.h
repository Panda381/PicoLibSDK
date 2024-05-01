
// ****************************************************************************
//
//                       IBM PC Emulator - Interrupts
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

/* Interrupts:
	0		CPU hw: divide by zero
	1		CPU hw: single step
	2		CPU hw: nonmaskable
	3		CPU hw: breakpoint
	4		CPU hw: overflow
	5		BIOS sw: print screen
	6-7		-
	8		BIOS hw: time of day
	9		BIOS hw: keyboard
	0x0A		-
	0x0B,0x0C	BIOS hw: COM
	0x0D		BIOS hw: LPT
	0x0E		BIOS hw: floppy
	0x0F		BIOS hw: LPT
	0x10		BIOS sw: video
	0x11		BIOS sw: equipment check
	0x12		BIOS sw: memory
	0x13		BIOS sw: disk
	0x14		BIOS sw: communication
	0x15		BIOS sw: cassette
	0x16		BIOS sw: keyboard
	0x17		BIOS sw: printer
	0x18		BIOS sw: resident BASIC
	0x19		BIOS sw: bootstrap
	0x1A		BIOS sw: time of day
	0x1B		BIOS hw: keyboard break (called from IRQ 9 when Ctrl+Break is pressed)
	0x1C		BIOS hw: time tick (called from IRQ 8)
	0x1D		BIOS pointer: video initialization data
	0x1E		BIOS pointer: diskette parameters
	0x1F		BIOS pointer: video graphics chars 8x8 with code 0x80-0xFF
	0x20		DOS sw: pogram terminate
	0x21		DOS sw: function call
	0x22		DOS : terminate address
	0x23		DOS pointer: ctrl+Break exit address
	0x24		DOS pointer: fatal error vector
	0x25		DOS sw: disk read
	0x26		DOS sw: disk write
	0x27		DOS sw: terminate
	0x28-0x3F	reserved for DOS
	0x40		BIOS pointer: diskette initialization parameters
	0x41		BIOS pointer: 1st fixed disk 16-byte initialization parameters (points to F000h:E401h) ... other 3 disk parameters may follow
	0x42-0x45	reserved for BIOS
	0x46		BIOS pointer: 2nd fixed disk 16-byte initialization parameters
	0x47-0x5F	reserved for BIOS
	0x60-0x67	reserved for user program interrupts
	0x68-0x6F	-
	0x70		IRQ 8 realtime clock INT
	0x71		IRQ 9
	0x72		IRQ 10
	0x73		IRQ 11
	0x74		IRQ 12
	0x75		IRQ 13
	0x76		IRQ 14
	0x77		IRQ 15
	0x78-0x7F	-
	0x80-0x85	reserved for BASIC
	0x86-0xF0	used by BASIC interpreter
	0xF1-0xFF	-
*/

// processing trap
extern Bool PC_IntTrapEnter;

// interrupt trap: returns instruction code I8086_INS_IRET if continue, or I8086_INS_NOP if waiting in loop
u8 PC_IntTrap(int num);
