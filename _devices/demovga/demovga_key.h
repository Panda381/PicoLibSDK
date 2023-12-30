
// ****************************************************************************
//
//                            Keyboard - DemoVGA
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

#if USE_DEMOVGA

#ifndef _DEMOVGA_KEY_H
#define _DEMOVGA_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

// buttons codes (= button indice + 1) (0 = no key)
#define BTN_A		1		// A
#define BTN_B		2		// B
#define BTN_C		3		// C

#define KEY_A		BTN_A
#define KEY_DOWN	BTN_B
#define KEY_UP		BTN_C

#define KEY_NUM		3	// number of buttons

#define KEY_REL_TIME	50	// delta time of release in [ms]

// initialize keyboard
void KeyInit();

// terminate keyboard
void KeyTerm();

// check if button KEY_* is currently pressed
Bool KeyPressed(u8 key);

// scan keyboard (called from SysTick)
void KeyScan();

// get virtual key code from keyboard buffer (returns NOKEY if no scan code)
u8 KeyGet();

// get character from keyboard buffer (returns NOCHAR if no valid character)
INLINE char KeyChar() { return NOCHAR; }

// flush keyboard buffer
void KeyFlush();

// return key to keyboard buffer (can hold only 1 key)
void KeyRet(u8 key);

// check no pressed key
Bool KeyNoPressed();

// wait for no key pressed
void KeyWaitNoPressed();

#ifdef __cplusplus
}
#endif

#endif // _DEMOVGA_KEY_H

#endif // USE_DEMOVGA
