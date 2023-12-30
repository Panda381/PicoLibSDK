
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

#include "../../global.h"	// globals

#if USE_PICOTRON

#include "../../_sdk/inc/sdk_gpio.h"
#include "../../_sdk/inc/sdk_timer.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_lib/inc/lib_print.h"
#include "picotron_key.h"

// key map, counters of pressed keys - key is pressed if counter > 0 (counter is in [ms]/2)
volatile u8 KeyMap[KEY_NUM]; // key map
volatile u8 KeyRelMap[KEY_NUM]; // key release map counter (in [ms]/2)

volatile Bool KeyCapsLock = False; // flag - caps lock is ON
volatile Bool KeyInsert = False; // flag - Insert is ON
Bool ShiftPrefix = False; // shift prefix pressed

// last time of KeyScan function
u16 KeyLastTime; // last time in [ms]

// keyboard buffer
u8 KeyBuf[KEYBUF_SIZE]; // keyboard buffer
u8 KeyRetBuf = NOKEY;	// returned key to keyboard buffer (NOKEY = no key)
volatile u8 KeyWriteOff = 0; // write offset to keyboard buffer
volatile u8 KeyReadOff = 0;	// read offset from keyboard buffer

// GPIO pins
u8 KeyRowGpio[KEY_ROWNUM] = {
	KEY_ROW1_GPIO,
	KEY_ROW2_GPIO,
	KEY_ROW3_GPIO,
	KEY_ROW4_GPIO,
	KEY_ROW5_GPIO,
};

u8 KeyColGpio[KEY_COLNUM] = {
	KEY_COL1_GPIO,
	KEY_COL2_GPIO,
	KEY_COL3_GPIO,
	KEY_COL4_GPIO,
	KEY_COL5_GPIO,
	KEY_COL6_GPIO,
	KEY_COL7_GPIO,
	KEY_COL8_GPIO,
};

// key remap table, shift is OFF, alt is OFF (NOCHAR=no key)
const char KeyRemapTabOff[KEY_NUM] = {
	'2',	// #define VK_2		1	// 2 @ ;
	'3',	// #define VK_3		2	// 3 # ~
	'4',	// #define VK_4		3	// 4 $ ^
	'5',	// #define VK_5		4	// 5 % '\'
	'6',	// #define VK_6		5	// 6 ^
	'7',	// #define VK_7		6	// 7 &
	'8',	// #define VK_8		7	// 8 *
	'9',	// #define VK_9		8	// 9 (
	'1',	// #define VK_1		9	// 1 ! :
	'e',	// #define VK_E		10	// E End
	'r',	// #define VK_R		11	// R PgUp
	't',	// #define VK_T		12	// T [
	'y',	// #define VK_Y		13	// Y ]
	'u',	// #define VK_U		14	// U {
	'i',	// #define VK_I		15	// I }
	'0',	// #define VK_0		16	// 0 )
	'q',	// #define VK_Q		17	// Q Home
	'w',	// #define VK_W		18	// W Up
	'f',	// #define VK_F		19	// F PgDn
	'g',	// #define VK_G		20	// G <
	'h',	// #define VK_H		21	// H >
	'j',	// #define VK_J		22	// J ?
	'o',	// #define VK_O		23	// O '
	'p',	// #define VK_P		24	// P "
	'a',	// #define VK_A		25	// A Left
	's',	// #define VK_S		26	// S Down
	'd',	// #define VK_D		27	// D Right
	'v',	// #define VK_V		28	// V /
	'b',	// #define VK_B		29	// B +
	'k',	// #define VK_K		30	// K _
	'l',	// #define VK_L		31	// L |
	CH_CR,	// #define VK_CR	32	// CR Esc Ins
	NOCHAR,	// #define VK_SHIFT	33	// Shift Caps
	'z',	// #define VK_Z		34	// Z Tab
	'x',	// #define VK_X		35	// X ,
	'c',	// #define VK_C		36	// C .
	'n',	// #define VK_N		37	// N -
	'm',	// #define VK_M		38	// M =
	' ',	// #define VK_SPACE	39	// Spc Back Del
	NOCHAR,	// #define VK_ALT	40	// Alt
};

// key remap table, shift is ON (NOCHAR=no key)
const char KeyRemapTabOn[KEY_NUM] = {
	'@',	// #define VK_2		1	// 2 @ ;
	'#',	// #define VK_3		2	// 3 # ~
	'$',	// #define VK_4		3	// 4 $ ^
	'%',	// #define VK_5		4	// 5 % '\'
	'^',	// #define VK_6		5	// 6 ^
	'&',	// #define VK_7		6	// 7 &
	'*',	// #define VK_8		7	// 8 *
	'(',	// #define VK_9		8	// 9 (
	'!',	// #define VK_1		9	// 1 ! :
	'E',	// #define VK_E		10	// E End
	'R',	// #define VK_R		11	// R PgUp
	'T',	// #define VK_T		12	// T [
	'Y',	// #define VK_Y		13	// Y ]
	'U',	// #define VK_U		14	// U {
	'I',	// #define VK_I		15	// I }
	')',	// #define VK_0		16	// 0 )
	'Q',	// #define VK_Q		17	// Q Home
	'W',	// #define VK_W		18	// W Up
	'F',	// #define VK_F		19	// F PgDn
	'G',	// #define VK_G		20	// G <
	'H',	// #define VK_H		21	// H >
	'J',	// #define VK_J		22	// J ?
	'O',	// #define VK_O		23	// O '
	'P',	// #define VK_P		24	// P "
	'A',	// #define VK_A		25	// A Left
	'S',	// #define VK_S		26	// S Down
	'D',	// #define VK_D		27	// D Right
	'V',	// #define VK_V		28	// V /
	'B',	// #define VK_B		29	// B +
	'K',	// #define VK_K		30	// K _
	'L',	// #define VK_L		31	// L |
	CH_ESC,	// #define VK_CR	32	// CR Esc Ins
	NOCHAR,	// #define VK_SHIFT	33	// Shift Caps
	'Z',	// #define VK_Z		34	// Z Tab
	'X',	// #define VK_X		35	// X ,
	'C',	// #define VK_C		36	// C .
	'N',	// #define VK_N		37	// N -
	'M',	// #define VK_M		38	// M =
	CH_BS,	// #define VK_SPACE	39	// Spc Back Del
	NOCHAR,	// #define VK_ALT	40	// Alt
};

// key remap table, alt is ON (NOCHAR=no key)
const char KeyRemapTabAlt[KEY_NUM] = {
	';',	// #define VK_2		1	// 2 @ ;
	'~',	// #define VK_3		2	// 3 # ~
	'\\',	// #define VK_4		3	// 4 $ ^
	NOCHAR,	// #define VK_5		4	// 5 % '\'
	NOCHAR,	// #define VK_6		5	// 6 ^
	NOCHAR,	// #define VK_7		6	// 7 &
	NOCHAR,	// #define VK_8		7	// 8 *
	NOCHAR,	// #define VK_9		8	// 9 (
	':',	// #define VK_1		9	// 1 ! :
	CH_END,	// #define VK_E		10	// E End
	CH_PGUP,// #define VK_R		11	// R PgUp
	'[',	// #define VK_T		12	// T [
	']',	// #define VK_Y		13	// Y ]
	'{',	// #define VK_U		14	// U {
	'}',	// #define VK_I		15	// I }
	NOCHAR,	// #define VK_0		16	// 0 )
	CH_HOME,// #define VK_Q		17	// Q Home
	CH_UP,	// #define VK_W		18	// W Up
	CH_PGDN,// #define VK_F		19	// F PgDn
	'<',	// #define VK_G		20	// G <
	'>',	// #define VK_H		21	// H >
	'?',	// #define VK_J		22	// J ?
	'\'',	// #define VK_O		23	// O '
	'"',	// #define VK_P		24	// P "
	CH_LEFT,// #define VK_A		25	// A Left
	CH_DOWN,// #define VK_S		26	// S Down
	CH_RIGHT,// #define VK_D	27	// D Right
	'/',	// #define VK_V		28	// V /
	'+',	// #define VK_B		29	// B +
	'_',	// #define VK_K		30	// K _
	'|',	// #define VK_L		31	// L |
	CH_INS,	// #define VK_CR	32	// CR Esc Ins
	NOCHAR,	// #define VK_SHIFT	33	// Shift Caps
	CH_TAB,	// #define VK_Z		34	// Z Tab
	',',	// #define VK_X		35	// X ,
	'.',	// #define VK_C		36	// C .
	'-',	// #define VK_N		37	// N -
	'=',	// #define VK_M		38	// M =
	CH_DEL,	// #define VK_SPACE	39	// Spc Back Del
	NOCHAR,	// #define VK_ALT	40	// Alt
};

// initialize keyboard
void KeyInit()
{
	int i;
	u8 gpio;

	// setup rows, set temporary direction to input, level HIGH
	for (i = 0; i < KEY_ROWNUM; i++)
	{
		// get GPIO
		gpio = KeyRowGpio[i];

		// initialize GPIO, set to input
		GPIO_Init(gpio);

		// set output value to 1
		GPIO_Out1(gpio);
	}
	
	// setup columns, set direction to input, pull-down
	for (i = 0; i < KEY_COLNUM; i++)
	{
		// get GPIO
		gpio = KeyColGpio[i];

		// initialize GPIO, set to input
		GPIO_Init(gpio);

		// set pull-down
		GPIO_PullDown(gpio);
	}

	// clear key map
	memset((void*)KeyMap, 0, sizeof(KeyMap));
	memset((void*)KeyRelMap, 0, sizeof(KeyRelMap));

	KeyCapsLock = False;
	KeyInsert = False;
	ShiftPrefix = False;

	KeyRetBuf = NOKEY;
	KeyReadOff = 0;
	KeyWriteOff = 0;

	KeyLastTime = TimeMs();
}

// terminate keys
void KeyTerm()
{
	int i;
	for (i = 0; i < KEY_ROWNUM; i++) GPIO_Reset(KeyRowGpio[i]);
	for (i = 0; i < KEY_COLNUM; i++) GPIO_Reset(KeyColGpio[i]);
}

// write key to keyboard buffer
void KeyWriteKey(u8 key)
{
	u8 w = KeyWriteOff;
	u8 w2 = w + 1;
	if (w2 >= KEYBUF_SIZE) w2 = 0;
	dmb();
	if (w2 != KeyReadOff)
	{
		// write key
		KeyBuf[w] = key;
		dmb();
		KeyWriteOff = w2;
	}

	// update shift prefix
	ShiftPrefix = (key == VK_SHIFT);

	// update Insert
	if ((key == VK_CR) && (KeyMap[VK_ALT-1] > 0)) KeyInsert = !KeyInsert;
}

// scan keyboard
void KeyScan()
{
	u8 col, row, b, c, key;
	volatile u8* m = KeyMap; // key map
	volatile u8* r = KeyRelMap; // key release map counter (in [ms]/2)

	// delta time
	u16 t = TimeMs(); // current time in [ms]
	int i = ((u16)(t - KeyLastTime))/2;
	if (i < 2) return; // too short delta time < 4 ms
	if (i > 255) i = 255; // maximal delta time 510 ms
	KeyLastTime = t;
	u8 deltatime = (u8)i;

	// rows
	key = 1;
	for (row = 0; row < KEY_ROWNUM; row++)
	{
		// set row to output
		GPIO_DirOut(KeyRowGpio[row]);

		// short delay to stabilise signals
		for (i = 10; i > 0; i--) nop2();

		// columns
		for (col = 0; col < KEY_COLNUM; col++)
		{
			// load maps
			b = *m; // load key pressed map
			c = *r; // load key released map

			// check if button is pressed
			if (GPIO_In(KeyColGpio[col]) == 1)
			{
				// first press
				if (b == 0)
				{
					KeyWriteKey(key);
					b = KEY_REP_TIME1/2;
				}

				// key is already pressed - check repeat interval
				else
				{
					if (deltatime < b)
						b -= deltatime; // update repeat counter
					else
					{
						// Shift and Ctrl does not repeat
						if ((key != VK_SHIFT) && (key != VK_ALT)) KeyWriteKey(key);
						b = KEY_REP_TIME2/2; // update repeat timer
					}
				}

				// reset release timer
				c = KEY_REL_TIME/2;
			}

			// button is not pressed
			else
			{
				if (deltatime < c)
				{
					c -= deltatime; // update release counter
				}
				else
				{
					c = 0; // reset release counter

					// release
					if (b > 0)
					{
						b = 0; // reset pressed counter

						// caps lock
						if ((key == VK_SHIFT) && ShiftPrefix)
						{
							KeyCapsLock = !KeyCapsLock;
							ShiftPrefix = False;
						}
					}
				}
			}

			// update map
			*m = b;
			*r = c;
			m++;
			r++;
			key++;
		}

		// set row to input
		GPIO_DirIn(KeyRowGpio[row]);
	}
}

// check if key is pressed
Bool KeyPressed(u8 key)
{
	if ((key == 0) || (key > KEY_NUM)) return False;
	return KeyMap[key-1] > 0;
}

// remap scan code to ASCII character (returns NOCHAR on invalid key)
char KeyToChar(u8 key)
{
	// invalid key
	if ((key == 0) || (key > KEY_NUM)) return NOCHAR;

	// remap to character with shift
	Bool shift = (KeyMap[VK_SHIFT-1] > 0);
	Bool alt = (KeyMap[VK_ALT-1] > 0);
	char ch = alt ? KeyRemapTabAlt[key-1] : (shift ? KeyRemapTabOn[key-1] : KeyRemapTabOff[key-1]);
	if (ch == NOCHAR) return NOCHAR;

	// upper case
	if (KeyCapsLock) shift = !shift;
	if (shift && (ch >= 'a') && (ch <= 'z')) ch -= 'a' - 'A';
	return ch;
}

// get scan code from keyboard buffer (returns NOKEY if no scan code)
u8 KeyGet()
{
#if !SYSTICK_KEYSCAN	// call KeyScan() function from SysTick system timer
	// scan keyboard
	KeyScan();
#endif

	// get key from temporary 1-key buffer
	u8 ch = KeyRetBuf;
	if (ch != NOKEY)
	{
		KeyRetBuf = NOKEY;
		return ch;
	}

	// no key, buffer is empty
	u8 r = KeyReadOff;
	dmb();
	if (r == KeyWriteOff) return NOKEY;

	// get key
	ch = KeyBuf[r];

	// increase read offset
	r++;
	if (r >= KEYBUF_SIZE) r = 0;
	KeyReadOff = r;

	return ch;
}

// get character from keyboard buffer (returns NOCHAR if no valid character)
char KeyChar()
{
	char ch;
	u8 key;
	while (True)
	{
		// get key scan code
		key = KeyGet();
		if (key == NOKEY) return NOCHAR; // no key

		// remap scan code to ASCII character
		ch = KeyToChar(key);
		if (ch != NOCHAR) return ch;
	}
}

// flush keyboard buffer
void KeyFlush()
{
	KeyReadOff = KeyWriteOff;
	KeyRetBuf = NOKEY;
}

// return key to keyboard buffer (can hold only 1 key)
void KeyRet(char key)
{
	KeyRetBuf = key;
}

// check no pressed key
Bool KeyNoPressed()
{
	int i;
	for (i = 0; i < KEY_NUM; i++) if (KeyMap[i] > 0) return False;
	return True;
}

// wait for no key pressed
void KeyWaitNoPressed()
{
	while (!KeyNoPressed()) {}
}

#endif // USE_PICOTRON
