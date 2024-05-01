
// ****************************************************************************
//
//                       IBM PC Emulator - DOS functions
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

#define PC_DOS_DISK	2		// current disk C:

#define PC_DOS_MEDIA	0x42		// offset of media descriptor in PSP

#define PC_DOS_FILE1	5		// index of first open file
#define PC_DOS_FILES	10		// max. number of open files
#define PC_DOS_MAXFILENAME 15		// max. length of filename

// extended character buffer (0 = no character)
extern u8 PC_Int21fncBuf;

// number of characters in input buffer AH=0Ah
extern u8 PC_Int21InNum;

// Initialize PSP (Program Segment Prefix)
//  0x00F00 - 0x00FFF: 256 bytes PSP Program Segment Prefix of user COM program (0x00F0:0x0000 - 0x00F0:0x00FF)
//  endseg ... segment of address beyond program
void PC_InitPSP(u16 endseg);

// close DOS files
void PC_CloseFiles();

// INT 21h service - function; returns instruction code I8086_INS_IRET if continue, or I8086_INS_NOP if waiting in loop
u8 PC_Int21fnc();
