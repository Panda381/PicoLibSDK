
// ****************************************************************************
//
//                            Keyboard - Picotron 1.0
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

#if USE_PICOTRON

#ifndef _PICOTRON_KEY_H
#define _PICOTRON_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_COLNUM	8	// number of columns (output COL1..COL8)
#define KEY_ROWNUM	5	// number of rows (input ROW1..ROW7)
#define KEY_NUM		(KEY_COLNUM*KEY_ROWNUM) // number of keys

#define KEYBUF_SIZE	20	// size of keyboard buffer (= 32)

#define KEY_REL_TIME	50	// delta time of release in [ms]

// Virtual key codes (scanned keyboard) = key scan code + 1
// - use symbol instead exact value, virtual key code can change in future version

// row 1
#define VK_2		1	// 2 @ ;
#define VK_3		2	// 3 # ~
#define VK_4		3	// 4 $ ^
#define VK_5		4	// 5 % '\'
#define VK_6		5	// 6 ^
#define VK_7		6	// 7 &
#define VK_8		7	// 8 *
#define VK_9		8	// 9 (
// row 2
#define VK_1		9	// 1 ! :
#define VK_E		10	// E End
#define VK_R		11	// R PgUp
#define VK_T		12	// T [
#define VK_Y		13	// Y ]
#define VK_U		14	// U {
#define VK_I		15	// I }
#define VK_0		16	// 0 )
// row 3
#define VK_Q		17	// Q Home
#define VK_W		18	// W Up
#define VK_F		19	// F PgDn
#define VK_G		20	// G <
#define VK_H		21	// H >
#define VK_J		22	// J ?
#define VK_O		23	// O '
#define VK_P		24	// P "
// row 4
#define VK_A		25	// A Left
#define VK_S		26	// S Down
#define VK_D		27	// D Right
#define VK_V		28	// V /
#define VK_B		29	// B +
#define VK_K		30	// K _
#define VK_L		31	// L |
#define VK_CR		32	// CR Esc Ins
// row 5
#define VK_SHIFT	33	// Shift Caps
#define VK_Z		34	// Z Tab
#define VK_X		35	// X ,
#define VK_C		36	// C .
#define VK_N		37	// N -
#define VK_M		38	// M =
#define VK_SPACE	39	// Spc Back Del
#define VK_ALT		40	// Alt

// buttons codes
#define KEY_UP		VK_W		// up
#define KEY_LEFT	VK_A		// left
#define KEY_RIGHT	VK_D		// right
#define KEY_DOWN	VK_S		// down
#define KEY_X		VK_L		// X
#define KEY_Y		VK_CR		// Y
#define KEY_A		VK_SPACE	// A
#define KEY_B		VK_M		// B

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

#ifdef __cplusplus
}
#endif

#endif // _PICOTRON_KEY_H

#endif // USE_PICOTRON
