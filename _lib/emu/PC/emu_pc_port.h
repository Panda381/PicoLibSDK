
// ****************************************************************************
//
//                        IBM PC Emulator - Ports
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

// read port byte
u8 FASTCODE NOFLASH(PC_GetPort)(u16 addr);

// read port word
u16 FASTCODE NOFLASH(PC_GetPort16)(u16 addr);

// write port byte
void FASTCODE NOFLASH(PC_SetPort)(u16 addr, u8 data);

// write port word
void FASTCODE NOFLASH(PC_SetPort16)(u16 addr, u16 data);
