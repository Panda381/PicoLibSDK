
// ****************************************************************************
//
//                             PicoPad Buttons
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

#if USE_PICOPAD

#ifndef _PICOPAD_KEY_H
#define _PICOPAD_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

// buttons codes (= button indice + 1) (0 = no key)
#define KEY_UP		1	// up
#define KEY_LEFT	2	// left
#define KEY_RIGHT	3	// right
#define KEY_DOWN	4	// down
#define KEY_X		5	// X
#define KEY_Y		6	// Y
#define KEY_A		7	// A
#define KEY_B		8	// B

#define KEY_NUM		8	// number of buttons

#define KEY_MASK	0x7f	// key code mask (to clear key release flag)
#define KEY_RELEASE	B7	// key release flag

// timings
#ifndef KEY_REL_TIME
#define KEY_REL_TIME	50	// delta time of release in [ms]
#endif

// keys are currently pressed (index = button code - 1)
extern volatile Bool KeyPressMap[KEY_NUM];

// initialize keys
void KeyInit();

// terminate keys
void KeyTerm();

// check if button KEY_* is currently pressed
Bool KeyPressed(u8 key);
INLINE Bool KeyPressedFast(u8 key) { return KeyPressMap[key-1]; }

// scan keyboard (called from SysTick)
void KeyScan();

// get button from keyboard buffer, including release keys (returns NOKEY if no scan code; B7 = KEY_RELEASE = release flag)
u8 KeyGetRel();

// get button from keyboard buffer (returns NOKEY if no scan code)
u8 KeyGet();

// get character from local keyboard (returns NOCHAR if no character)
char KeyChar();

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

#endif // _PICOPAD_KEY_H

#endif // USE_PICOPAD
