
// ****************************************************************************
//
//                            Keyboard - Picoino 1.0
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

#ifndef _PICOINO_KEY_H
#define _PICOINO_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#if !USE_PICOINOMINI

#define KEY_COLNUM	8	// number of columns (output COL1..COL8)
#define KEY_ROWNUM	7	// number of rows (input ROW1..ROW7)
#define KEY_NUM		(KEY_COLNUM*KEY_ROWNUM) // number of keys

#define KEYBUF_SIZE	20	// size of keyboard buffer (= 32)

#define KEY_REL_TIME	50	// delta time of release in [ms]

// Virtual key codes (scanned keyboard) = key scan code + 1
// - use symbol instead exact value, virtual key code can change in future version
#define VK_2		1	// 2/@
#define VK_1		2	// 1/!
#define VK_CTRL		3	// Ctrl
#define VK_S		4	// S
#define VK_SHIFT	5	// Shift
#define VK_SPACE	6	// Space/Ins
#define VK_TAB		7	// Tab
#define VK_Q		8	// Q
#define VK_3		9	// 3/#
#define VK_X		10	// X
#define VK_E		11	// E
#define VK_A		12	// A
#define VK_Z		13	// Z
#define VK_W		14	// W
#define VK_4		15	// 4/$
#define VK_5		16	// 5/%
#define VK_V		17	// V
#define VK_F		18	// F
#define VK_D		19	// D
#define VK_C		20	// C
#define VK_R		21	// R
#define VK_6		22	// 6/^
#define VK_7		23	// 7/&
#define VK_N		24	// N
#define VK_T		25	// T
#define VK_G		26	// G
#define VK_B		27	// B
#define VK_Y		28	// Y
#define VK_9		29	// 9/(
#define VK_8		30	// 8/*
#define VK_H		31	// H
#define VK_U		32	// U
#define VK_J		33	// J
#define VK_M		34	// M
#define VK_I		35	// I
#define VK_RSLASH	36	// '\'/|
#define VK_EQU		37	// =/+
#define VK_LEFT		38	// Left/Home
#define VK_L		39	// L
#define VK_SEMI		40	// ;/:
#define VK_DOWN		41	// Down/PgDn
#define VK_LPAR		42	// [/{
#define VK_MINUS	43	// -/_
#define VK_0		44	// 0/)
#define VK_COMMA	45	// ,/<
#define VK_O		46	// O
#define VK_K		47	// K
#define VK_DOT		48	// ./>
#define VK_P		49	// P
#define VK_RPAR		50	// ]/}
#define VK_BS		51	// Backspace/Del
#define VK_UP		52	// Up/PgUp
#define VK_SLASH	53	// /?
#define VK_CR		54	// CR/Esc
#define VK_RIGHT	55	// Right/End
#define VK_QUOT		56	// '/"

// buttons codes
#define KEY_UP		VK_UP		// up
#define KEY_LEFT	VK_LEFT		// left
#define KEY_RIGHT	VK_RIGHT	// right
#define KEY_DOWN	VK_DOWN		// down
#define KEY_X		VK_CTRL		// X
#define KEY_Y		VK_BS		// Y
#define KEY_A		VK_SPACE	// A
#define KEY_B		VK_Z		// B

// key map, counters of pressed keys - key is pressed if counter > 0 (counter is in [ms]/2)
extern volatile u8 KeyMap[KEY_NUM];
extern volatile u8 KeyRelMap[KEY_NUM]; // key release map counter (in [ms]/2)

extern volatile Bool KeyCapsLock; // flag - caps lock is ON
extern volatile Bool KeyInsert; // flag - Insert is ON

// initialize keyboard
void KeyInit();

// terminate keyboard
void KeyTerm();

// check if key is pressed
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

#endif // _PICOINO_KEY_H

#endif // USE_PICOINO
