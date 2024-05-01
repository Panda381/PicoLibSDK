
// ****************************************************************************
//
//                       IBM PC Emulator - INT 15h extension
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

// flag - waiting in fuction 86h
extern Bool PC_Int15Wait;

// INT 15h service - function; returns instruction code I8086_INS_IRET if continue, or I8086_INS_NOP if waiting in loop
u8 PC_Int15fnc();
