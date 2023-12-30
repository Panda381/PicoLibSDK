
// ****************************************************************************
//
//                            Keyboard - PicoinoMini
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

#if USE_PICOINO

#ifndef _PICOINO_KEYMINI_H
#define _PICOINO_KEYMINI_H

#ifdef __cplusplus
extern "C" {
#endif

#if USE_PICOINOMINI

// buttons codes (= button indice + 1) (0 = no key)
#define KEY_UP		1		// up
#define KEY_LEFT	2		// left
#define KEY_RIGHT	3		// right
#define KEY_DOWN	4		// down
#define KEY_X		5		// X
#define KEY_Y		6		// Y
#define KEY_A		7		// A
#define KEY_B		8		// B

#define KEY_NUM		8	// number of buttons

#define KEY_REL_TIME	50	// delta time of release in [ms]

// state of User LED
extern Bool UserLedState;

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
char KeyChar();

// flush keyboard buffer
void KeyFlush();

// return key to keyboard buffer (can hold only 1 key)
void KeyRet(char key);

// check no pressed key
Bool KeyNoPressed();

// wait for no key pressed
void KeyWaitNoPressed();

#endif // USE_PICOINOMINI

#ifdef __cplusplus
}
#endif

#endif // _PICOINO_KEYMINI_H

#endif // USE_PICOINO
