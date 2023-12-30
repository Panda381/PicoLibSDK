
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

#include "../../global.h"	// globals

#if USE_PICOINO

#if !USE_PICOINOMINI

#include "../../_sdk/inc/sdk_gpio.h"
#include "../../_sdk/inc/sdk_timer.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_lib/inc/lib_print.h"
#include "picoino_key.h"

// key map, counters of pressed keys - key is pressed if counter > 0 (counter is in [ms]/2)
volatile u8 KeyMap[KEY_NUM];
volatile u8 KeyRelMap[KEY_NUM]; // key release map counter (in [ms]/2)

volatile Bool KeyCapsLock = False; // flag - caps lock is ON
volatile Bool KeyInsert = False; // flag - Insert is ON
Bool ShiftPrefix = False; // shift prefix pressed

// last time of KeyScan function
u16 KeyLastTime; // last time in [ms]
u8 KeyDeltaTime; // delta of new KeyScan in [ms]/2

// keyboard buffer
u8 KeyBuf[KEYBUF_SIZE]; // keyboard buffer
u8 KeyRetBuf = NOKEY;	// returned key to keyboard buffer (NOKEY = no key)
volatile u8 KeyWriteOff = 0; // write offset to keyboard buffer
volatile u8 KeyReadOff = 0;	// read offset from keyboard buffer

// key remap table, shift is OFF (NOCHAR=no key)
const char KeyRemapTabOff[KEY_NUM] = {
	'2',	// #define VK_2		0	// 2/@
	'1',	// #define VK_1		1	// 1/!
	NOCHAR,	// #define VK_CTRL	2	// Ctrl
	's',	// #define VK_S		3	// S
	NOCHAR,	// #define VK_SHIFT	4	// Shift
	' ',	// #define VK_SPACE	5	// Space/Ins
	CH_TAB,	// #define VK_TAB	6	// Tab
	'q',	// #define VK_Q		7	// Q
	'3',	// #define VK_3		8	// 3/#
	'x',	// #define VK_X		9	// X
	'e',	// #define VK_E		10	// E
	'a',	// #define VK_A		11	// A
	'z',	// #define VK_Z		12	// Z
	'w',	// #define VK_W		13	// W
	'4',	// #define VK_4		14	// 4/$
	'5',	// #define VK_5		15	// 5/%
	'v',	// #define VK_V		16	// V
	'f',	// #define VK_F		17	// F
	'd',	// #define VK_D		18	// D
	'c',	// #define VK_C		19	// C
	'r',	// #define VK_R		20	// R
	'6',	// #define VK_6		21	// 6/^
	'7',	// #define VK_7		22	// 7/&
	'n',	// #define VK_N		23	// N
	't',	// #define VK_T		24	// T
	'g',	// #define VK_G		25	// G
	'b',	// #define VK_B		26	// B
	'y',	// #define VK_Y		27	// Y
	'9',	// #define VK_9		28	// 9/(
	'8',	// #define VK_8		29	// 8/*
	'h',	// #define VK_H		30	// H
	'u',	// #define VK_U		31	// U
	'j',	// #define VK_J		32	// J
	'm',	// #define VK_M		33	// M
	'i',	// #define VK_I		34	// I
	'\\',	// #define VK_RSLASH	35	// back slash /|
	'=',	// #define VK_EQU	36	// =/+
	CH_LEFT, // #define VK_LEFT	37	// Left/Home
	'l',	// #define VK_L		38	// L
	';',	// #define VK_SEMI	39	// ;/:
	CH_DOWN, // #define VK_DOWN	40	// Down/PgDn
	'[',	// #define VK_LPAR	41	// [/{
	'-',	// #define VK_MINUS	42	// -/_
	'0',	// #define VK_0		43	// 0/)
	',',	// #define VK_COMMA	44	// ,/<
	'o',	// #define VK_O		45	// O
	'k',	// #define VK_K		46	// K
	'.',	// #define VK_DOT	47	// ./>
	'p',	// #define VK_P		48	// P
	']',	// #define VK_RPAR	49	// ]/}
	CH_BS,	// #define VK_BS	50	// Backspace/Del
	CH_UP,	// #define VK_UP	51	// Up/PgUp
	'/',	// #define VK_SLASH	52	// /?
	CH_CR,	// #define VK_CR	53	// CR/Esc
	CH_RIGHT, // #define VK_RIGHT	54	// Right/End
	'\'',	// #define VK_QUOT	55	// '/"
};

// key remap table, shift is ON (NOCHAR=no key)
const char KeyRemapTabOn[KEY_NUM] = {
	'@',	// #define VK_2		0	// 2/@
	'!',	// #define VK_1		1	// 1/!
	NOCHAR,	// #define VK_CTRL	2	// Ctrl
	's',	// #define VK_S		3	// S
	NOCHAR,	// #define VK_SHIFT	4	// Shift
	CH_INS,	// #define VK_SPACE	5	// Space/Ins
	CH_TAB,	// #define VK_TAB	6	// Tab
	'q',	// #define VK_Q		7	// Q
	'#',	// #define VK_3		8	// 3/#
	'x',	// #define VK_X		9	// X
	'e',	// #define VK_E		10	// E
	'a',	// #define VK_A		11	// A
	'z',	// #define VK_Z		12	// Z
	'w',	// #define VK_W		13	// W
	'$',	// #define VK_4		14	// 4/$
	'%',	// #define VK_5		15	// 5/%
	'v',	// #define VK_V		16	// V
	'f',	// #define VK_F		17	// F
	'd',	// #define VK_D		18	// D
	'c',	// #define VK_C		19	// C
	'r',	// #define VK_R		20	// R
	'^',	// #define VK_6		21	// 6/^
	'&',	// #define VK_7		22	// 7/&
	'n',	// #define VK_N		23	// N
	't',	// #define VK_T		24	// T
	'g',	// #define VK_G		25	// G
	'b',	// #define VK_B		26	// B
	'y',	// #define VK_Y		27	// Y
	'(',	// #define VK_9		28	// 9/(
	'*',	// #define VK_8		29	// 8/*
	'h',	// #define VK_H		30	// H
	'u',	// #define VK_U		31	// U
	'j',	// #define VK_J		32	// J
	'm',	// #define VK_M		33	// M
	'i',	// #define VK_I		34	// I
	'|',	// #define VK_RSLASH	35	// back slash /|
	'+',	// #define VK_EQU	36	// =/+
	CH_HOME, // #define VK_LEFT	37	// Left/Home
	'l',	// #define VK_L		38	// L
	':',	// #define VK_SEMI	39	// ;/:
	CH_PGDN, // #define VK_DOWN	40	// Down/PgDn
	'{',	// #define VK_LPAR	41	// [/{
	'_',	// #define VK_MINUS	42	// -/_
	')',	// #define VK_0		43	// 0/)
	'<',	// #define VK_COMMA	44	// ,/<
	'o',	// #define VK_O		45	// O
	'k',	// #define VK_K		46	// K
	'>',	// #define VK_DOT	47	// ./>
	'p',	// #define VK_P		48	// P
	'}',	// #define VK_RPAR	49	// ]/}
	CH_DEL,	// #define VK_BS	50	// Backspace/Del
	CH_PGUP,// #define VK_UP	51	// Up/PgUp
	'?',	// #define VK_SLASH	52	// /?
	CH_ESC,	// #define VK_CR	53	// CR/Esc
	CH_END, // #define VK_RIGHT	54	// Right/End
	'"',	// #define VK_QUOT	55	// '/"
};

// key short labels (3 characters per key)
const char KeyShortLabel[KEY_NUM*3+1] =
	"2 @"	// #define VK_2		0	// 2/@
	"1 !"	// #define VK_1		1	// 1/!
	"Ctr"	// #define VK_CTRL	2	// Ctrl
	" S "	// #define VK_S		3	// S
	"Shf"	// #define VK_SHIFT	4	// Shift
	"Spc"	// #define VK_SPACE	5	// Space/Ins
	"Tab"	// #define VK_TAB	6	// Tab
	" Q "	// #define VK_Q		7	// Q
	"3 #"	// #define VK_3		8	// 3/#
	" X "	// #define VK_X		9	// X
	" E "	// #define VK_E		10	// E
	" A "	// #define VK_A		11	// A
	" Z "	// #define VK_Z		12	// Z
	" W "	// #define VK_W		13	// W
	"4 $"	// #define VK_4		14	// 4/$
	"5 %"	// #define VK_5		15	// 5/%
	" V "	// #define VK_V		16	// V
	" F "	// #define VK_F		17	// F
	" D "	// #define VK_D		18	// D
	" C "	// #define VK_C		19	// C
	" R "	// #define VK_R		20	// R
	"6 ^"	// #define VK_6		21	// 6/^
	"7 &"	// #define VK_7		22	// 7/&
	" N "	// #define VK_N		23	// N
	" T "	// #define VK_T		24	// T
	" G "	// #define VK_G		25	// G
	" B "	// #define VK_B		26	// B
	" Y "	// #define VK_Y		27	// Y
	"9 ("	// #define VK_9		28	// 9/(
	"8 *"	// #define VK_8		29	// 8/*
	" H "	// #define VK_H		30	// H
	" U "	// #define VK_U		31	// U
	" J "	// #define VK_J		32	// J
	" M "	// #define VK_M		33	// M
	" I "	// #define VK_I		34	// I
	"\\ |"	// #define VK_RSLASH	35	// back slash /|
	"= +"	// #define VK_EQU	36	// =/+
	"Lft"	// #define VK_LEFT	37	// Left/Home
	" L "	// #define VK_L		38	// L
	"; :"	// #define VK_SEMI	39	// ;/:
	"Dwn"	// #define VK_DOWN	40	// Down/PgDn
	"[ {"	// #define VK_LPAR	41	// [/{
	"- _"	// #define VK_MINUS	42	// -/_
	"0 )"	// #define VK_0		43	// 0/)
	", <"	// #define VK_COMMA	44	// ,/<
	" O "	// #define VK_O		45	// O
	" K "	// #define VK_K		46	// K
	". >"	// #define VK_DOT	47	// ./>
	" P "	// #define VK_P		48	// P
	"] }"	// #define VK_RPAR	49	// ]/}
	"Bsp"	// #define VK_BS	50	// Backspace/Del
	"Up "	// #define VK_UP	51	// Up/PgUp
	"/ ?"	// #define VK_SLASH	52	// /?
	"CR "	// #define VK_CR	53	// CR/Esc
	"Rgh"	// #define VK_RIGHT	54	// Right/End
	"' \""	// #define VK_QUOT	55	// '/"
;

// name of virtual key code
const char* const KeyCodeName[KEY_NUM] = {
	"2",		// #define VK_2		0	// 2/@
	"1",		// #define VK_1		1	// 1/!
	"CTRL",		// #define VK_CTRL	2	// Ctrl
	"S",		// #define VK_S		3	// S
	"SHIFT",	// #define VK_SHIFT	4	// Shift
	"SPACE",	// #define VK_SPACE	5	// Space/Ins
	"TAB",		// #define VK_TAB	6	// Tab
	"Q",		// #define VK_Q		7	// Q
	"3",		// #define VK_3		8	// 3/#
	"X",		// #define VK_X		9	// X
	"E",		// #define VK_E		10	// E
	"A",		// #define VK_A		11	// A
	"Z",		// #define VK_Z		12	// Z
	"W",		// #define VK_W		13	// W
	"4",		// #define VK_4		14	// 4/$
	"5",		// #define VK_5		15	// 5/%
	"V",		// #define VK_V		16	// V
	"F",		// #define VK_F		17	// F
	"D",		// #define VK_D		18	// D
	"C",		// #define VK_C		19	// C
	"R",		// #define VK_R		20	// R
	"6",		// #define VK_6		21	// 6/^
	"7",		// #define VK_7		22	// 7/&
	"N",		// #define VK_N		23	// N
	"T",		// #define VK_T		24	// T
	"G",		// #define VK_G		25	// G
	"B",		// #define VK_B		26	// B
	"Y",		// #define VK_Y		27	// Y
	"9",		// #define VK_9		28	// 9/(
	"8",		// #define VK_8		29	// 8/*
	"H",		// #define VK_H		30	// H
	"U",		// #define VK_U		31	// U
	"J",		// #define VK_J		32	// J
	"M",		// #define VK_M		33	// M
	"I",		// #define VK_I		34	// I
	"RSLASH",	// #define VK_RSLASH	35	// back slash /|
	"EQU",		// #define VK_EQU	36	// =/+
	"LEFT",		// #define VK_LEFT	37	// Left/Home
	"L",		// #define VK_L		38	// L
	"SEMI",		// #define VK_SEMI	39	// ;/:
	"DOWN",		// #define VK_DOWN	40	// Down/PgDn
	"LPAR",		// #define VK_LPAR	41	// [/{
	"MINUS",	// #define VK_MINUS	42	// -/_
	"0",		// #define VK_0		43	// 0/)
	"COMMA",	// #define VK_COMMA	44	// ,/<
	"O",		// #define VK_O		45	// O
	"K",		// #define VK_K		46	// K
	"DOT",		// #define VK_DOT	47	// ./>
	"P",		// #define VK_P		48	// P
	"RPAR",		// #define VK_RPAR	49	// ]/}
	"BS",		// #define VK_BS	50	// Backspace/Del
	"UP",		// #define VK_UP	51	// Up/PgUp
	"SLASH",	// #define VK_SLASH	52	// /?
	"CR",		// #define VK_CR	53	// CR/Esc
	"RIGHT",	// #define VK_RIGHT	54	// Right/End
	"QUOT",		// #define VK_QUOT	55	// '/"
};

// name of control character
const char* const CtrlCharName[34] = {
	"NUL",		// #define CH_NUL	0x00	// NUL null ^@
	"ALL",		// #define CH_ALL	0x01	// SOH start of heading ^A	... select all
	"BLOCK",	// #define CH_BLOCK	0x02	// STX start of text ^B		... mark block
	"COPY",		// #define CH_COPY	0x03	// ETX end of text ^C		... copy block, copy file
	"END",		// #define CH_END	0x04	// EOT end of transmission ^D	... end of row, end of files
	"MOVE",		// #define CH_MOVE	0x05	// ENQ enquiry ^E		... rename files, move block
	"FIND",		// #define CH_FIND	0x06	// ACK acknowledge ^F		... find
	"NEXT",		// #define CH_NEXT	0x07	// BEL bell ^G			... repeat find
	"BS",		// #define CH_BS	0x08	// BS backspace ^H		... backspace
	"TAB",		// #define CH_TAB	0x09	// HT horizontal tabulator ^I	... tabulator
	"LF",		// #define CH_LF	0x0A	// LF line feed ^J		... line feed
	"PGUP",		// #define CH_PGUP	0x0B	// VT vertical tabulator ^K	... page up
	"PGDN",		// #define CH_PGDN	0x0C	// FF form feed ^L		... page down
	"CR",		// #define CH_CR	0x0D	// CR carriage return ^M	... enter, next row, run file
	"NEW",		// #define CH_NEW	0x0E	// SO shift-out ^N		... new file
	"OPEN",		// #define CH_OPEN	0x0F	// SI shift-in ^O		... open file, edit file
	"PRINT",	// #define CH_PRINT	0x10	// DLE data link escape ^P	... print file, send file
	"QUERY",	// #define CH_QUERY	0x11	// DC1 device control 1 ^Q	... display help (query)
	"REPLACE",	// #define CH_REPLACE	0x12	// DC2 device control 2 ^R	... find and replace
	"SAVE",		// #define CH_SAVE	0x13	// DC3 device control 3 ^S	... save file
	"INS",		// #define CH_INS	0x14	// DC4 device control 4 ^T	... toggle Insert switch, mark file
	"HOME",		// #define CH_HOME	0x15	// NAK negative acknowledge ^U	... begin of row, begin of files
	"PASTE",	// #define CH_PASTE	0x16	// SYN synchronous idle ^V	... paste from clipboard
	"CLOSE",	// #define CH_CLOSE	0x17	// ETB end of transmission block ^W ... close file
	"CUT",		// #define CH_CUT	0x18	// CAN cancel ^X		... cut selected text
	"REDO",		// #define CH_REDO	0x19	// EM end of medium ^Y		... redo previously undo action
	"UNDO",		// #define CH_UNDO	0x1A	// SUB subtitle character ^Z	... undo action
	"ESC",		// #define CH_ESC	0x1B	// ESC escape ^[		... break, menu
	"RIGHT",	// #define CH_RIGHT	0x1C	// FS file separator		... Right, Shift: End, Ctrl: Word right
	"UP",		// #define CH_UP	0x1D	// GS group separator ^]	... Up, Shift: PageUp, Ctrl: Text start
	"LEFT",		// #define CH_LEFT	0x1E	// RS record separator ^^	... Left, Shift: Home, Ctrl: Word left
	"DOWN",		// #define CH_DOWN	0x1F	// US unit separator ^_		... Down, Shift: PageDn, Ctrl: Text end
	"SPC",		// #define CH_SPC	0x20	// SPC space
	"DEL",		// #define CH_DEL	0x7F	// delete			... delete character on cursor, Ctrl: delete block, delete file
};

// initialize keyboard
void KeyInit()
{
	// initialize GPIO
	GPIO_Init(KEY_DATA_GPIO);
	GPIO_Init(KEY_CLK1_GPIO);
	GPIO_Init(KEY_CLK2_GPIO);

	// set GPIO to output
	GPIO_DirOut(KEY_CLK1_GPIO);
	GPIO_DirOut(KEY_CLK2_GPIO);

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
	GPIO_Reset(KEY_DATA_GPIO);
	GPIO_Reset(KEY_CLK1_GPIO);
	GPIO_Reset(KEY_CLK2_GPIO);
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
	if ((key == VK_SPACE) && (KeyMap[VK_SHIFT-1] > 0)) KeyInsert = !KeyInsert;
}

// short delay
NOINLINE void KeyShortDelay(u8 num)
{
	for (; num > 0; num--) nop2();
}

// scan one column of keyboard
//   m ... pointer to key pressed map
//   r ... pointer to key released map
void KeyColScan(u8* m, u8* r)
{
	int j;
	u8 b, c;
	u8 t = KeyDeltaTime;
	u8 key;

	// set data pin to input
	GPIO_DirIn(KEY_DATA_GPIO);

	// loop rows
	for (j = 0; j < KEY_ROWNUM; j++)
	{
		// key code
		key = m - KeyMap + 1;

		// delay to stabilise keyboard input signal
		KeyShortDelay(5); // short delay

		// load maps
		b = *m; // load key pressed map
		c = *r; // load key released map

		// check if button is pressed
		if (GPIO_In(KEY_DATA_GPIO) != 0)
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
				if (t < b)
					b -= t; // update repeat counter
				else
				{
					// Shift and Ctrl does not repeat
					if ((key != VK_SHIFT) && (key != VK_CTRL)) KeyWriteKey(key);
					b = KEY_REP_TIME2/2; // update repeat timer
				}
			}

			// reset release timer
			c = KEY_REL_TIME/2;
		}

		// button is not pressed
		else
		{
			if (t < c)
			{
				c -= t; // update release counter
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

		// shift row register
		GPIO_Out1(KEY_CLK2_GPIO); // set clock2 HIGH
		KeyShortDelay(2); // short delay
		GPIO_Out0(KEY_CLK2_GPIO); // set clock2 LOW
	}
}

// clock 1 impulse HLH (load keyboard rows and shift keyboard columns)
void KeyClock1()
{
	GPIO_Out0(KEY_CLK1_GPIO); // set clock1 LOW
	KeyShortDelay(2); // short delay
	GPIO_Out1(KEY_CLK1_GPIO); // set clock1 HIGH - shift columns
	KeyShortDelay(1); // short delay
}

// scan keyboard
void KeyScan()
{
	int i, j, n;
	volatile u8* m;
	volatile u8* r;
	u8 b;

	// delta time
	u16 t = TimeMs();
	i = ((u16)(t - KeyLastTime))/2;
	if (i < 3) return;
	if (i > 255) i = 255;
	KeyLastTime = t;
	KeyDeltaTime = (u8)i;

	// prepare 1st column - set QA to HIGH
	GPIO_Out0(KEY_CLK2_GPIO); // set clock2 LOW
	GPIO_DirOut(KEY_DATA_GPIO); // set data pin to output
	GPIO_Out1(KEY_DATA_GPIO); // set data pin to HIGH
	KeyClock1();		// (CLK1 pulse HLH) shift data HIGH to QA

	// loop columns
	m = KeyMap;
	r = KeyRelMap;
	for (i = 0; i < KEY_COLNUM; i++)
	{
		// load keyboard rows and shift to next column
		GPIO_DirOut(KEY_DATA_GPIO); // set data pin to output
		GPIO_Out0(KEY_DATA_GPIO); // set data pin LOW
		KeyClock1(); // (CLK1 pulse HLH) shift columns

		// scan one column
		KeyColScan((u8*)m, (u8*)r);
		m += KEY_ROWNUM;
		r += KEY_ROWNUM;
	}

	// set LEDs
	if (KeyCapsLock) GPIO_Out0(KEY_CLK1_GPIO); // set clock1 LOW
	if (!KeyInsert) GPIO_Out1(KEY_CLK2_GPIO); // set clock2 HIGH
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
	Bool up = (KeyMap[VK_SHIFT-1] > 0);
	char ch = up ? KeyRemapTabOn[key-1] : KeyRemapTabOff[key-1];
	if (ch == NOCHAR) return NOCHAR;

	// Ctrl
	if (KeyMap[VK_CTRL-1] > 0)
	{
		if ((ch >= 'a') && (ch <= 'z')) return ch & 0x1f;
	}

	// upper case
	if (KeyCapsLock) up = !up;
	if (up && (ch >= 'a') && (ch <= 'z')) ch -= 'a' - 'A';
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

#endif // USE_PICOINOMINI

#endif // USE_PICOINO
