
// ****************************************************************************
//
//                       IBM PC Emulator - System time
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

extern u8 PC_TimerMode; // time counter 0 setup, port 40h
extern u16 PC_TimerDiv; // time counter divider, port 40h (default: 0.0549255 sec, 18.2065Hz)
extern Bool PC_TimerHigh; // flip/flop - read/write high byte
extern u16 PC_TimerLatch; // latch

// setup timer to default state (should be called on program start)
void PC_TimerDef();

// INT 1Ah service - function
void PC_Int1Afnc();
