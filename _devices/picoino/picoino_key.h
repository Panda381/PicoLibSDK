
// ****************************************************************************
//
//                                 Keyboard
//
// ****************************************************************************

#ifndef _PICOINO_KEY_H
#define _PICOINO_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_COLNUM	8	// number of columns (output COL1..COL8)
#define KEY_ROWNUM	7	// number of rows (input ROW1..ROW7)
#define KEY_NUM		(KEY_COLNUM*KEY_ROWNUM) // number of keys
//#define KEY_EXTNUM	2	// number of extra bits - DIP configuration

//#define KEYMAXCNT	70	// max. counter of pressed key in [ms]
//#define KEYDELCNT	500	// first delay before repeat in [ms]
//#define KEYREPCNT	100	// repeat delay in [ms]

#define KEYBUF_SIZE	20	// size of keyboard buffer (= 32)

//#define KEY_MASK	0x3f	// key code mask
//#define KEY_REP		B6	// flag - repeated key
//#define KEY_REL		B7	// flag - release key

// timings
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

// key map, counters of pressed keys - key is pressed if counter > 0
//extern volatile u8 KeyMap[KEY_NUM];
//extern volatile u8 KeyRepMap[KEY_NUM]; // key repeat map

// key coordinates on board, high nibble = row 0..4, low nibble = column 0..14
//extern const u8 KeyCoord[KEY_NUM];

// key short labels (3 characters per key)
//extern const char KeyShortLabel[KEY_NUM*3+1];

// name of key scan code
//extern const char* const KeyCodeName[KEY_NUM];

// name of control character
//extern const char* const CtrlCharName[34];

// flag - scan first or second half of keyboard
//extern volatile Bool KeyHalf;

//extern Bool ShiftIsOn; // shift is pressed
//extern Bool CtrlIsOn; // ctrl is pressed

// keyboard timings setup
//extern u8 KeyMaxCnt; // max. counter of pressed key - release interval
//extern u8 KeyDelCnt; // first delay before repeat
//extern u8 KeyRepCnt; // repeat delay

// DIP config DEV_* (0=VGA, DIP1=PAL, DIP2=NTSC, DIP1|DIP2=OLDVGA)
// - Use GetDipCfg() function to get config value
//extern u8 DipCfg;

// initialize keyboard
void KeyInit();

// terminate keyboard
void KeyTerm();

// check if key is pressed (release flag is ignored)
Bool KeyPressed(u8 key);

// setup keyboard timings after changing display timings
//  vlines = number of vertical SYNC video lines (how many is KeyScan function called per frame)
//  vfreq = vertical frequency in Hz
//void KeySetup(int vlines, int vfreq);

// short delay
//void NOFLASH(KeyShortDelay)(u8 num);

// scan one column of keyboard
//void KeyColScan(u8* m, u8* r);

// scan keyboard (one call of function scans half of keyboard and takes about 13 us)
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

#endif // _PICOINO_KEY_H
