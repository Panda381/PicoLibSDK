
// ****************************************************************************
//
//                       IBM PC Emulator - Keyboard
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

// INT 09h keyboard
//   Passing keys through the scan code on the keyboard port is supported only partially,
//   for applications using direct code reading.
extern volatile u8 PC_Int9KeyNum; // number of scan codes in key buffer
extern volatile u8 PC_Int9KeyScan[4]; // scan code key buffer (can be read through keyboard port 60h)
extern volatile Bool PC_KeyMenuReq; // requirement for game menu (key Y)
extern volatile Bool PC_KeyZoomReq; // requirement for zoom screen (key X)
extern volatile u32 PC_Int9KeyTime; // time of last inserted key

// remap keypad codes to scan code (B7: extended scan code with 0xE0 prefix)
// - this table can be modified during program setup
extern u8 PC_KeypadScan[KEY_NUM+1];

// alternatives with modifier
extern u8 PC_KeypadScanU[KEY_NUM+1]; // up
extern u8 PC_KeypadScanL[KEY_NUM+1]; // left
extern u8 PC_KeypadScanR[KEY_NUM+1]; // right
extern u8 PC_KeypadScanD[KEY_NUM+1]; // down
extern u8 PC_KeypadScanX[KEY_NUM+1];
extern u8 PC_KeypadScanY[KEY_NUM+1];
extern u8 PC_KeypadScanA[KEY_NUM+1];
extern u8 PC_KeypadScanB[KEY_NUM+1];

// reset keyboard handler (on start of emulation)
void PC_Int9Reset();

// keyboard handler (called every 5 ms)
void PC_Int9Handler();

// INT 09h service - function
void PC_Int09fnc();

// input key into cpu->AX
//   ext ... use extended code
// Returns: I8086_INS_NOP if no key is ready (registers stay unchanged), or I8086_INS_IRET if key was loaded into cpu->AX
u8 PC_InCh(Bool ext);

// DOS Ctrl+Break or Ctrl+C
void PC_DosBreak();

// INT 16h service - function; returns instruction code I8086_INS_IRET if continue, or I8086_INS_NOP if waiting in loop
u8 PC_Int16fnc();
