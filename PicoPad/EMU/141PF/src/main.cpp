
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// used PWMs:
//   LCD backlight GPIO16 ... PWM 0
// * unused PWM 1: GPIO2 (btn RIGHT), GPIO3 (btn LEFT), GPIO18 (LCD SCK), GPIO19 (LCD MOSI)
// * unused PWM 2: GPIO4 (btn UP), GPIO5 (btn DOWN), GPIO20 (LCD RES), GPIO21 (LCD CS)
//   user LED GPIO22 ... PWM 3
//   board LED GPIO25 .. PWM 4
//   ext GPIO26, GPIO27 ... PWM 5
//   ext GPIO28 ... PWM 6
//   sound (GPIO15) ... PWM 7

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
#define EMU_PWM		1	// index of PWM used to synchronize emulations
#else
#define EMU_PWM		2	// index of PWM used to synchronize emulations
#endif

#define EMU_FREQ	740000	// emulation frequency (nominal frequency = 740 kHz)
#define EMU_CLKSYS_MIN	120	// minimal system clock in MHz
#define EMU_CLKSYS_MAX	180	// maximal system clock in MHz

#include "../include.h"

// ============================================================================
//                              Data and tables
// ============================================================================

#define ROUND_FL	0	// round float
#define ROUND_54	1	// round
#define ROUND_N		8	// truncate

sI4004 i4004cpu;

int I4_KeyShift;	// keyboard shifter (10 bits, 0=row is selected)
int I4_PrintShift;	// printer shifter (20 bits, 1=hammer is selected)
volatile u8 I4_DP;	// decimal point switch (0..8 except 7, default 8)
volatile u8 I4_Round;	// rounding mode switch ROUND_* (default ROUND_FL)
volatile u8 I4_Sector;	// current drum sector (0..12, 0=index)
u8 I4_OldROM0;		// old state of shifter clocks
u8 I4_OldRAM0;		// old state of print hammers
volatile Bool I4_PrintAdv; // request to advance paper
volatile u8 I4_Lamp;	// lamp status
			//	bit0 = memory lamp M
			//	bit1 = overflow lamp OVF
			//	bit2 = minus sign lamp NEG
u8 I4_LampOld;		// lamp status old
int I4_KeySel = 21;	// selected key
int I4_PrefKeySel = 21;	// last preffered key
Bool I4_KeySelOn = False; // selected key indicator is ON
Bool I4_DispHelp = False; // display key help
u32 I4_KeySelTime;	// selection time
u32 I4_TestTime;	// TEST time counter
u8 I4_Test;		// TEST signal
volatile Bool I4_AdvPressed; // ADV button is pressed
u8 I4_PrintColor;	// print color: 0 black, 0x80 red
Bool I4_SoundDone;	// sound was done

// characters in column 1-15
const char I4_CharCol1[13] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '.', '-', };

// characters in column 17
const char I4_CharCol17[13] = { 2 /* diamond */, '+', '-', 'x', ':', 3 /* M+ */, 4 /* M- */, '^', '=', 127 /* sqrt */, '%', 'C', 'R', };

// characters in column 18
const char I4_CharCol18[13] = { '#', '*', 'I', 6 /* II */, 7 /* III */, 3 /* M+ */, 4 /* M- */, 'T', 'K', 'E', 5 /* Ex */, 'C', 'M', };

// print buffer (0 = no character to print; bit 7 = use red color)
u8 volatile I4_PrintBuf[DIG_NUM];

/* Assign USB keyboard to buttons:

buttons:
	C ... C
	D ... decimal point switch
	E ... CE
	F ... diamond 2 (not on calculator keyboard) ... same function as "diamond"
	G ... 00
	H ... 000 (not on calculator keyboard)
	I ... M=+
	J ... M=-
	K ... CM
	M ... M-
	N ... %
	O ... diamond (print sub-total)
	P ... M+
	Q ... SQRT
	R ... RM
	S ... Sign
	W ... rounding switch
	0..9 ... 0...9 (key [] or digit row)
	. ... . (key [. Del] or .>)
	+ ... + (key [+] or +=)
	- ... - (key [-] or -_)
	* or U ... x (key [*] or U)
	/ or : ... : (key [/] or ;:)
	Enter ... = (key [Enter] or Enter)
	Tab ... EX
	Space ... ADV

keypad:
	Arrows ... move cursor on keyboard
	A ... select button
	B
	X ... display help
	Y or Esc ... exit
*/

// key indices
enum {
	// shifter bit 0
	KEYMAP_CM = 0,		//  0, 0x81: CM ... [K]
	KEYMAP_RM,		//  1, 0x82: RM ... [R]
	KEYMAP_MM,		//  2, 0x83: M- (memory minus) ... [M]
	KEYMAP_MP,		//  3, 0x84: M+ (memory plus) ... [P]
	// shifter bit 1
	KEYMAP_SQRT,		//  4, 0x85: SQRT ... [Q]
	KEYMAP_PERC,		//  5, 0x86: % (percentage) ... [N]
	KEYMAP_MEM,		//  6, 0x87: M=- (memory equals minus) ... [J]
	KEYMAP_MEP,		//  7, 0x88: M=+ (memory equals plus) ... [I]
	// shifter bit 2
	KEYMAP_DIA,		//  8, 0x89: diamond ... [O]
	KEYMAP_DIV,		//  9, 0x8A: / (divide) ... [/ or :]
	KEYMAP_MUL,		// 10, 0x8B: * (multiply) ... [* or U]
	KEYMAP_EQU,		// 11, 0x8C: = (equals) ... [Enter]
	// shifter bit 3
	KEYMAP_MINUS,		// 12, 0x8D: - (minus) ... [-]
	KEYMAP_PLUS,		// 13, 0x8E: + (plus) ... [+]
	KEYMAP_DIA2,		// 14, 0x8F: diamond 2 ... [F]
	KEYMAP_000,		// 15, 0x90: 000 ... [H]
	// shifter bit 4
	KEYMAP_9,		// 16, 0x91: 9 ... [9]
	KEYMAP_6,		// 17, 0x92: 6 ... [6]
	KEYMAP_3,		// 18, 0x93: 3 ... [3]
	KEYMAP_DOT,		// 19, 0x94: . ... [.]
	// shifter bit 5
	KEYMAP_8,		// 20, 0x95: 8 ... [8]
	KEYMAP_5,		// 21, 0x96: 5 ... [5]
	KEYMAP_2,		// 22, 0x97: 2 ... [2]
	KEYMAP_00,		// 23, 0x98: 00 ... [G]
	// shifter bit 6
	KEYMAP_7,		// 24, 0x99: 7 ... [7]
	KEYMAP_4,		// 25, 0x9A: 4 ... [4]
	KEYMAP_1,		// 26, 0x9B: 1 ... [1]
	KEYMAP_0,		// 27, 0x9C: 0 ... [0]
	// shifter bit 7
	KEYMAP_S,		// 28, 0x9D: S (sign) ... [S]
	KEYMAP_EX,		// 29, 0x9E: EX (exchange) ... [Tab]
	KEYMAP_CE,		// 30, 0x9F: CE (clear error) ... [E]
	KEYMAP_C,		// 31, 0xA0: C (clear) ... [C]
	// shifter bit 8
	KEYMAP_DP0,		// 32, 0xA1: DP0 (decimal point bit 0) ... [D]
	KEYMAP_DP1,		// 33, 0xA2: DP1 (decimal point bit 1)
	KEYMAP_DP2,		// 34, 0xA3: DP2 (decimal point bit 2)
	KEYMAP_DP3,		// 35, 0xA4: DP3 (decimal point bit 3)
	// shifter bit 9
	KEYMAP_SW1,		// 36, 0xA5: SW1 (rounding switch bit 0) ... [W]
	KEYMAP_NONE1,		// 37, 0xA6: ... not used
	KEYMAP_NONE2,		// 38, 0xA7: ... not used
	KEYMAP_SW2,		// 39, 0xA8: SW2 (rounding switch bit 1)

	KEYMAP_NUM		// 40, number of keys
};

#define KEYMAP_ADV	KEYMAP_NUM // 40, paper advance ... [Space]
#define KEYMAP_NUM2	(KEYMAP_NUM+1) // number of keys, including advance button

#define KEYMAP_BTNNUM	32	// number of base buttons

/*
 column (= bit of the shifter):
	9	8	7	6	5	4	3	2	1	0
/row (= ROM1 bit):
0	SW1	DP0	S	7	8	9	-	DIA	SQRT	CM
1	(none)	DP1	EX	4	5	6	+	/	%	RM
2	(none)	DP2	CE	1	2	3	DIA2	*	M=-	M-
3	SW2	DP3	C	0	00	.	000	=	M=+	M+
*/

#define KEYMAP_COLNUM	10	// number of columns (0..9)
#define KEYMAP_ROWNUM	4	// number of rows (0..3)

// map of pressed keys
Bool KeyMapPress[KEYMAP_NUM2];

// bit map of keys (32 bits, as fetched from ROM1 input port, 1=key is pressed)
u32 KeyMapBit;

// column X coordinate
const int KeyMapX[KEYMAP_COLNUM] = {
	8+7*KEYB_W+3*16, 8+6*KEYB_W+3*16, 8+5*KEYB_W+2*16, 8+4*KEYB_W+2*16, 8+3*KEYB_W+16, 8+2*KEYB_W+16, 8+KEYB_W+16, 8, 0, 0,	
};

// row Y coordinate
const int KeyMapY[KEYMAP_ROWNUM] = { KEYB_Y, KEYB_Y+KEYB_H, KEYB_Y+2*KEYB_H, KEYB_Y+3*KEYB_H, };

// key descriptor
typedef struct {
	u8	w;		// width in pixels (0 = invalid key)
	u8	h;		// height in pixels
	u8	key;		// USB key HID code
	u8	key2;		// USB key HID code alternative (0 = not used)
	const char* hlp;	// USB key help
	COLTYPE	col;		// help color
} sKeyMap;

// key list (lower 2 bits = row 0..3, higher 4 bits = column 0..9)
const sKeyMap KeyMap[KEYMAP_NUM2] = {
	// shifter bit 0
	{ KEYB_W, KEYB_H, HID_KEY_K, 0, "K", KEYB_HELP },		// KEYMAP_CM,		//  0, 0x81: CM
	{ KEYB_W, KEYB_H, HID_KEY_R, 0, "R", KEYB_HELP },		// KEYMAP_RM,		//  1, 0x82: RM
	{ KEYB_W, KEYB_H, HID_KEY_M, 0, "M", KEYB_HELP },		// KEYMAP_MM,		//  2, 0x83: M- (memory minus)
	{ KEYB_W, KEYB_H, HID_KEY_P, 0, "P", KEYB_HELP },		// KEYMAP_MP,		//  3, 0x84: M+ (memory plus)
	// shifter bit 1
	{ KEYB_W, KEYB_H, HID_KEY_Q, 0, "Q", KEYB_HELP },		// KEYMAP_SQRT,		//  4, 0x85: SQRT
	{ KEYB_W, KEYB_H, HID_KEY_N, 0, "N", KEYB_HELP },		// KEYMAP_PERC,		//  5, 0x86: % (percentage)
	{ KEYB_W, KEYB_H, HID_KEY_J, 0, "J", KEYB_HELP },		// KEYMAP_MEM,		//  6, 0x87: M=- (memory equals minus)
	{ KEYB_W, KEYB_H, HID_KEY_I, 0, "I", KEYB_HELP },		// KEYMAP_MEP,		//  7, 0x88: M=+ (memory equals plus)
	// shifter bit 2
	{ KEYB_W, KEYB_H, HID_KEY_O, 0, "O", KEYB_HELP },		// KEYMAP_DIA,		//  8, 0x89: diamond
	{ KEYB_W, KEYB_H, HID_KEY_KEYPAD_DIVIDE, HID_KEY_SEMICOLON, "/:", KEYB_HELP },	// KEYMAP_DIV,	//  9, 0x8A: / (divide)
	{ KEYB_W, KEYB_H, HID_KEY_KEYPAD_MULTIPLY, HID_KEY_U, "*U", KEYB_HELP },	// KEYMAP_MUL,	// 10, 0x8B: * (multiply)
	{ 2*KEYB_W, KEYB_H, HID_KEY_ENTER, HID_KEY_KEYPAD_ENTER, "Enter", KEYB_HELP2 },	// KEYMAP_EQU,	// 11, 0x8C: = (equals)
	// shifter bit 3
	{ KEYB_W, KEYB_H, HID_KEY_KEYPAD_SUBTRACT, HID_KEY_MINUS, "-", KEYB_HELP2 },	// KEYMAP_MINUS,	// 12, 0x8D: - (minus)
	{ KEYB_W, 2*KEYB_H, HID_KEY_KEYPAD_ADD, HID_KEY_EQUAL, "+", KEYB_HELP2 },	// KEYMAP_PLUS,		// 13, 0x8E: + (plus)
	{ 0, 0, HID_KEY_F, 0, "F", KEYB_HELP2 },			// KEYMAP_DIA2,		// 14, 0x8F: diamond 2
	{ 0, 0, HID_KEY_H, 0, "H", KEYB_HELP2 },			// KEYMAP_000,		// 15, 0x90: 000
	// shifter bit 4
	{ KEYB_W, KEYB_H, HID_KEY_9, HID_KEY_KEYPAD_9, "9", KEYB_HELP2 }, // KEYMAP_9,		// 16, 0x91: 9
	{ KEYB_W, KEYB_H, HID_KEY_6, HID_KEY_KEYPAD_6, "6", KEYB_HELP2 }, // KEYMAP_6,		// 17, 0x92: 6
	{ KEYB_W, KEYB_H, HID_KEY_3, HID_KEY_KEYPAD_3, "3", KEYB_HELP2 }, // KEYMAP_3,		// 18, 0x93: 3
	{ KEYB_W, KEYB_H, HID_KEY_KEYPAD_DECIMAL, HID_KEY_PERIOD, ".", KEYB_HELP2 },	// KEYMAP_DOT,	// 19, 0x94: .
	// shifter bit 5
	{ KEYB_W, KEYB_H, HID_KEY_8, HID_KEY_KEYPAD_8, "8", KEYB_HELP2 }, // KEYMAP_8,		// 20, 0x95: 8
	{ KEYB_W, KEYB_H, HID_KEY_5, HID_KEY_KEYPAD_5, "5", KEYB_HELP2 }, // KEYMAP_5,		// 21, 0x96: 5
	{ KEYB_W, KEYB_H, HID_KEY_2, HID_KEY_KEYPAD_2, "2", KEYB_HELP2 }, // KEYMAP_2,		// 22, 0x97: 2
	{ KEYB_W, KEYB_H, HID_KEY_G, 0, "G", KEYB_HELP2 },		// KEYMAP_00,		// 23, 0x98: 00
	// shifter bit 6
	{ KEYB_W, KEYB_H, HID_KEY_7, HID_KEY_KEYPAD_7, "7", KEYB_HELP2 }, // KEYMAP_7,		// 24, 0x99: 7
	{ KEYB_W, KEYB_H, HID_KEY_4, HID_KEY_KEYPAD_4, "4", KEYB_HELP2 }, // KEYMAP_4,		// 25, 0x9A: 4
	{ KEYB_W, KEYB_H, HID_KEY_1, HID_KEY_KEYPAD_1, "1", KEYB_HELP2 }, // KEYMAP_1,		// 26, 0x9B: 1
	{ KEYB_W, KEYB_H, HID_KEY_0, HID_KEY_KEYPAD_0, "0", KEYB_HELP2 }, // KEYMAP_0,		// 27, 0x9C: 0
	// shifter bit 7
	{ KEYB_W, KEYB_H, HID_KEY_S, 0, "S", KEYB_HELP },		// KEYMAP_S,		// 28, 0x9D: S (sign)
	{ KEYB_W, KEYB_H, HID_KEY_TAB, 0, "Tab", KEYB_HELP },		// KEYMAP_EX,		// 29, 0x9E: EX (exchange)
	{ KEYB_W, KEYB_H, HID_KEY_E, 0, "E", KEYB_HELP },		// KEYMAP_CE,		// 30, 0x9F: CE (clear error)
	{ KEYB_W, KEYB_H, HID_KEY_C, 0, "C", KEYB_HELP },		// KEYMAP_C,		// 31, 0xA0: C (clear)
	// shifter bit 8
	{ 0, 0, HID_KEY_D, 0, "D", KEYB_HELP },				// KEYMAP_DP0,		// 32, 0xA1: DP0 (decimal point bit 0)
	{ 0, 0, 0, 0, "", KEYB_HELP },					// KEYMAP_DP1,		// 33, 0xA2: DP1 (decimal point bit 1)
	{ 0, 0, 0, 0, "", KEYB_HELP },					// KEYMAP_DP2,		// 34, 0xA3: DP2 (decimal point bit 2)
	{ 0, 0, 0, 0, "", KEYB_HELP },					// KEYMAP_DP3,		// 35, 0xA4: DP3 (decimal point bit 3)
	// shifter bit 9
	{ 0, 0, HID_KEY_W, 0, "W", KEYB_HELP },				// KEYMAP_SW1,		// 36, 0xA5: SW1 (rounding switch bit 0)
	{ 0, 0, 0, 0, "", KEYB_HELP },					// KEYMAP_NONE1,	// 37, 0xA6: ... not used
	{ 0, 0, 0, 0, "", KEYB_HELP },					// KEYMAP_NONE2,	// 38, 0xA7: ... not used
	{ 0, 0, 0, 0, "", KEYB_HELP },					// KEYMAP_SW2,		// 39, 0xA8: SW2 (rounding switch bit 1)

	{ ADV_W, ADV_H, HID_KEY_SPACE, 0, "Spc", KEYB_HELP2 },		// KEYMAP_ADV,		// 40, paper advance
};

// ============================================================================
//                 Callbacks from CPU emulator from core 1
// ============================================================================

// read byte from ROM (only bank 0 used)
u8 FASTCODE NOFLASH(GetRomMem)(u16 addr)
{
	return busicom_Prog[addr];
}

// write byte to ROM (not used)
void FASTCODE NOFLASH(SetRomMem)(u16 addr, u8 val) { }

// write nibble to RAM port
void FASTCODE NOFLASH(SetRamPort)(u8 addr, u8 val)
{
	int i;
	u8 ch, col;

	// RAM0 port
	//	bit0 printing color (0=black, 1=red)
	//	bit1 fire print hammers (1=fire hammers)
	//	bit3 advance printer paper (1=paper is advancing)
	if (addr == 0)
	{
		// set red color
		if ((val & B0) != 0) I4_PrintColor = 0x80;

		// printing color
		col = I4_PrintColor;

		// check fire print hammers (change bit 1 state 0 -> 1)
		u8 old = I4_OldRAM0;	// get old state of RAM0
		I4_OldRAM0 = val;	// save new state of RAM0
		if (((old & B1) == 0) && ((val & B1) != 0))
		{
			// prepare printer shifter
			int s = I4_PrintShift;	// printer shifter (20 bits, 1=hammer is selected)

			// fire column 17 (bit 0 of printer shifter)
			if ((s & B0) != 0)
			{
				I4_PrintBuf[16] = (u8)I4_CharCol17[I4_Sector] | col;
			}
			s >>= 1; // shift printer shifter

			// fire column 18 (bit 1 of printer shifter)
			if ((s & B0) != 0)
			{
				I4_PrintBuf[17] = (u8)I4_CharCol18[I4_Sector] | col;
			}
			s >>= 1; // shift printer shifter

			// skip bit 2 (unused column 16)
			s >>= 1;

			// fire columns 1..15
			ch = (u8)I4_CharCol1[I4_Sector] | col;
			for (i = 0; i < 15; i++)
			{
				if ((s & B0) != 0) I4_PrintBuf[i] = ch;
				s >>= 1;
			}
		}

		// advance paper and set back to black color (change bit 3 state 0 -> 1)
		if (((old & B3) == 0) && ((val & B3) != 0))
		{
			I4_PrintAdv = True; // request to advance paper
			I4_PrintColor = 0; // set to default black color
		}
	}

	// RAM1 port
	//	bit0 = memory lamp M, emulator OUT0 output, LED1 yellow
	//	bit1 = overflow lamp OVF, emulator OUT1 output, LED2 red
	//	bit2 = minus sign lamp NEG, emulator OUT2 output, LED3 green
	else if (addr == 1)
	{
		I4_Lamp = val;	// lamp status
	}
}

// write nibble to ROM port
void FASTCODE NOFLASH(SetRomPort)(u8 addr, u8 val)
{
	int k;

	// ROM0 port
	//	bit0 = keyboard matrix column shifter clock (10 bits; 1=start write pulse, 0=stop write pulse)
	//	bit1 = shifter data (for printer and keyboard shifter; keyboard 0=row selected)
	//	bit2 = printer shifter clock (20 bits; 1=start write pulse, 0=stop write pulse)
	if (addr == 0)
	{
		// check keyboard shift (change bit 0 state 0 -> 1)
		u8 old = I4_OldROM0;	// get old state of ROM0
		I4_OldROM0 = val;	// save new state of ROM0
		if (((old & B0) == 0) && ((val & B0) != 0))
		{
			// shift keyboard shifter (get data from bit 1)
			k = I4_KeyShift << 1;
			if ((val & B1) != 0) k |= B0;
			I4_KeyShift = k;
		}

		// check printer shift (change bit 2 state 0 -> 1)
		if (((old & B2) == 0) && ((val & B2) != 0))
		{
			// shift printer shifter (get data from bit 1)
			k = I4_PrintShift << 1;
			if ((val & B1) != 0) k |= B0;
			I4_PrintShift = k;
		}
	}
}

// read nibble from ROM port
u8 FASTCODE NOFLASH(GetRomPort)(u8 addr)
{
	int i;

	// ROM1: bit 0 .. bit 3 = keyboard matrix rows input from col0..col3
	if (addr == 1)
	{
		// get keyboard shifter (10 bits, 0=row is selected)
		int shift = I4_KeyShift;

		// get bit map of keys (32 bits, 1=key is pressed)
		u32 map = KeyMapBit;

		// process map, rows 0..7
		u8 res = 0;
		for (i = 8; i > 0; i--)
		{
			if ((shift & B0) == 0) res |= map & 0x0f;
			shift >>= 1;
			map >>= 4;
		}

		// process decimal point switch
		if ((shift & B0) == 0) res |= I4_DP;
		shift >>= 1;

		// process rounding switch
		if ((shift & B0) == 0) res |= I4_Round;

		return res;
	}

	// ROM2:
	//	bit0 = printer drum index 0 signal input (1=active, 0=inactive)
	//	bit3 = printer paper advancing button input (1=button is pressed)
	if (addr == 2)
	{
		u8 res = I4_AdvPressed ? B3 : 0; // advancing button pressed
		if (I4_Sector == 0) res |= B0; // sector 0 active
		return res;
	}

	return 0;
}

// get TEST signal (0=TEST is active, 1=TEST is not active)
// - increase sector every 28 ms when TEST is HIGH, index every 364 ms, flip TEST every 14 ms)
u8 FASTCODE NOFLASH(GetTest)()
{
	// get current TEST signal
	u8 test = I4_Test;

	// check flip time 14 ms
	u32 t = Time();
	if ((int)(t - I4_TestTime) >= 14000)
	{
		I4_TestTime = t;

		// flip TEST state
		test ^= 1;
		I4_Test = test;

		// shift sector, if TEST is not active
		if (test != 0)
		{
			u8 s = I4_Sector + 1;
			if (s > 12) s = 0;
			I4_Sector = s;
		}
	}
	return test;
}

// ============================================================================
//                      Control interface on core 0
// ============================================================================

// display decimal point switch
void DPSwitch141PF()
{
	// display background
	DrawImgPal(LightOffImg, LightOffImg_Pal, DP_X, DP_Y-LIGHT_Y, DP_X, DP_Y, DP_W, DP_H, LIGHT_WALL);

	// prepare index
	int i = I4_DP;
	if (i >= 7) i = 7;

	// display switch
	DrawImgPal(LightOnImg, LightOnImg_Pal, SLIDE_X, SLIDE_Y, DP_X+8+i*8, DP_Y, SLIDE_W, SLIDE_H, LIGHT_WALL);

	// display help
	if (I4_DispHelp)
	{
		DrawChar('D', (I4_DP > 3) ? DP_X : (DP_X+DP_W-8), DP_Y, KEYB_HELP);
	}
}

// display rounding switch
void RoundSwitch141PF()
{
	// display background
	DrawImgPal(LightOffImg, LightOffImg_Pal, SW_X, SW_Y-LIGHT_Y, SW_X, SW_Y, SW_W, SW_H, LIGHT_WALL);

	// prepare index
	int i = (I4_Round == ROUND_FL) ? 1 : ((I4_Round == ROUND_54) ? 2 : 0);

	// display switch
	DrawImgPal(LightOnImg, LightOnImg_Pal, SLIDE_X, SLIDE_Y, SW_X+8+i*16, SW_Y, SLIDE_W, SLIDE_H, LIGHT_WALL);

	// display help
	if (I4_DispHelp)
	{
		DrawChar('W', (I4_Round == ROUND_54) ? SW_X : (SW_X+SW_W-8), SW_Y, KEYB_HELP);
	}
}

// check if button is pressed
Bool PressedBtn(int btn)
{
	Bool press = False;

	// selected button
	if ((btn == I4_KeySel) && !I4_DispHelp)
	{
		press = KeyPressed(KEY_A) || UsbKeyIsPressed(HID_KEY_A);
	}

	// USB key
	const sKeyMap* k = &KeyMap[btn];
	u8 key = k->key;
	if (key != 0) press |= UsbKeyIsPressed(key);
	key = k->key2;
	if (key != 0) press |= UsbKeyIsPressed(key); 

	return press;
}

// display button
void DispBtn(int btn, Bool down)
{
	int x, y, w, h;

	// update bit map
	if (btn < 32)
	{
		if (down)
			KeyMapBit |= BIT(btn);
		else
			KeyMapBit &= ~BIT(btn);
	}

	// save pressed flag
	Bool oldpress = KeyMapPress[btn];
	KeyMapPress[btn] = down;

	// sound
	if (down && !oldpress) PLAYSOUND(ClickSnd);

	// ADV button is pressed
	if (btn == KEYMAP_ADV) I4_AdvPressed = down;
	
	// display decimal point switch
	if (btn == KEYMAP_DP0)
	{
		if (down && !oldpress)
		{
			u8 dp = I4_DP + 1;
			if (dp == 7) dp = 8;
			if (dp > 8) dp = 0;
			I4_DP = dp;
		}
		DPSwitch141PF();
		return;
	}

	// display rounding switch
	if (btn == KEYMAP_SW1)
	{
		if (down && !oldpress)
		{
			u8 r = I4_Round;
			if (r == ROUND_N)
				r = ROUND_FL;
			else if (r == ROUND_FL)
				r = ROUND_54;
			else
				r = ROUND_N;
			I4_Round = r;
		}
		RoundSwitch141PF();
		return;
	}

	// get key descriptor
	const sKeyMap* k = &KeyMap[btn];
	w = k->w;
	h = k->h;

	// key has no visible button
	if (w == 0) return;

	// coordinates
	if (btn == KEYMAP_ADV)
	{
		x = ADV_X;
		y = ADV_Y;

		// draw ADV button
		if (down)
			DrawImgPal(LightOnImg, LightOnImg_Pal, x, y-LIGHT_Y, x, y, w, h, LIGHT_WALL);
		else
			DrawImgPal(LightOffImg, LightOffImg_Pal, x, y-LIGHT_Y, x, y, w, h, LIGHT_WALL);
	}
	else
	{
		x = KeyMapX[btn >> 2];
		y = KeyMapY[btn & 3];

		// correction of the [=] button
		if (btn == KEYMAP_EQU) x -= KEYB_W;

		// draw button
		if (down)
			DrawImgPal(KeybDnImg, KeybDnImg_Pal, x, y-KEYB_Y, x, y, w, h, KEYB_WALL);
		else
			DrawImgPal(KeybUpImg, KeybUpImg_Pal, x, y-KEYB_Y, x, y, w, h, KEYB_WALL);
	}

	// display help
	if (I4_DispHelp)
	{
		const char* hlp = k->hlp;
		if (hlp != NULL) DrawText(hlp, x + 1, y + h - DIG_FONTH, k->col);
	}
}

// button selection indicator
void DispKeySel(int btn)
{
	int x, y, w, h;

	// prepare decimal point switch
	if (btn == KEYMAP_DP0)
	{
		x = DP_X;
		y = DP_Y;
		w = DP_W;
		h = DP_H;
	}

	// prepare rounding switch
	else if (btn == KEYMAP_SW1)
	{
		x = SW_X;
		y = SW_Y;
		w = SW_W;
		h = SW_H;
	}

	// get key descriptor
	else
	{
		const sKeyMap* k = &KeyMap[btn];
		w = k->w;
		h = k->h;

		// key has no visible button
		if (w == 0) return;

		// coordinates
		if (btn == KEYMAP_ADV)
		{
			x = ADV_X;
			y = ADV_Y;
		}
		else
		{
			x = KeyMapX[btn >> 2];
			y = KeyMapY[btn & 3];
		}

		// correction of the [=] button
		if (btn == KEYMAP_EQU) x -= KEYB_W;
	}

	// draw frame
	DrawFrameW(x, y, w, h, KEYB_FRAMECOL, 3);

	// selection visible
	I4_KeySelOn = True;
	I4_KeySelTime = Time(); // start time counter
}

// control button selection (key = direction key KEY_* or NOKEY)
void CtrlKeySel(u8 key)
{
	// display help
	if (I4_DispHelp) return;

	// selected button
	int btn = I4_KeySel;

	// save last preffered selected "common" key
	const sKeyMap* k = &KeyMap[btn];
	if ((k->w == KEYB_W) && (k->h == KEYB_W)) I4_PrefKeySel = btn;

	// check if button is pressed
	Bool press = PressedBtn(btn);

	// display button selection
	Bool on = (((Time() - I4_KeySelTime) >> 18) & 1) == 0;
	if ((on != I4_KeySelOn) || (press != KeyMapPress[btn]))
	{
		I4_KeySelOn = on;
		if (on)
		{
			if (press != KeyMapPress[btn]) DispBtn(btn, press);
			DispKeySel(btn);
		}
		else
			DispBtn(btn, press);
		DispUpdate();
	}

	// cursor keys
	switch (key)
	{
	case KEY_LEFT:

		// set cursor OFF
		if (I4_KeySelOn)
		{
			DispBtn(btn, press);
			I4_KeySelOn = False;
		}

		// shift cursor
		if (btn == KEYMAP_ADV)
			btn = KEYMAP_SW1;
		else if (btn == KEYMAP_DP0)
			btn = KEYMAP_ADV;
		else if (btn == KEYMAP_SW1)
			btn = KEYMAP_DP0;
		else
		{
			if (btn == KEYMAP_PLUS)
			{
				if ((I4_PrefKeySel & 3) >= 2)
					btn = KEYMAP_3;
				else
					btn = KEYMAP_6;
			}
			else
			{
				btn += 4;
				if (btn == KEYMAP_DIA2) btn = KEYMAP_PLUS;
				if (btn == KEYMAP_000) btn = KEYMAP_DOT;
				if (btn >= KEYMAP_BTNNUM) btn -= KEYMAP_BTNNUM;
			}
		}
		I4_KeySel = btn;

		// display cursor
		DispKeySel(btn);
		DispUpdate();
		break;

	case KEY_RIGHT:

		// set cursor OFF
		if (I4_KeySelOn)
		{
			DispBtn(btn, press);
			I4_KeySelOn = False;
		}

		// shift cursor
		if (btn == KEYMAP_ADV)
			btn = KEYMAP_DP0;
		else if (btn == KEYMAP_DP0)
			btn = KEYMAP_SW1;
		else if (btn == KEYMAP_SW1)
			btn = KEYMAP_ADV;
		else
		{
			if (btn == KEYMAP_PLUS)
			{
				if ((I4_PrefKeySel & 3) >= 2)
					btn = KEYMAP_MUL;
				else
					btn = KEYMAP_DIV;
			}
			else
			{
				btn -= 4;
				if (btn == KEYMAP_DIA2) btn = KEYMAP_PLUS;
				if (btn == KEYMAP_000) btn = KEYMAP_EQU;
				if (btn < 0) btn += KEYMAP_BTNNUM;
			}
		}
		I4_KeySel = btn;

		// display cursor
		DispKeySel(btn);
		DispUpdate();
		break;

	case KEY_UP:

		// set cursor OFF
		if (I4_KeySelOn)
		{
			DispBtn(btn, press);
			I4_KeySelOn = False;
		}

		// shift cursor
		if (btn == KEYMAP_ADV)
			btn = KEYMAP_C;
		else if (btn == KEYMAP_DP0)
		{
			if (I4_PrefKeySel >= KEYMAP_7)
				btn = KEYMAP_0;
			else if (I4_PrefKeySel <= KEYMAP_DOT)
				btn = KEYMAP_DOT;
			else
				btn = KEYMAP_00;
		}
		else if (btn == KEYMAP_SW1)
			btn = KEYMAP_EQU;
		else if ((btn & 3) == 0)
		{
			if (btn >= KEYMAP_S)
				btn = KEYMAP_ADV;
			else if (btn >= KEYMAP_9)
				btn = KEYMAP_DP0;
			else if (btn >= KEYMAP_DIA)
				btn = KEYMAP_SW1;
			else
				btn |= 3;
		}
		else if (btn == KEYMAP_EQU)
		{
			if (I4_PrefKeySel >= KEYMAP_MINUS)
				btn = KEYMAP_PLUS;
			else
				btn = KEYMAP_MUL;
		}
		else
			btn--;

		I4_KeySel = btn;

		// display cursor
		DispKeySel(btn);
		DispUpdate();
		break;

	case KEY_DOWN:

		// set cursor OFF
		if (I4_KeySelOn)
		{
			DispBtn(btn, press);
			I4_KeySelOn = False;
		}

		// shift cursor
		if (btn == KEYMAP_ADV)
			btn = KEYMAP_S;
		else if (btn == KEYMAP_DP0)
		{
			if (I4_PrefKeySel >= KEYMAP_7)
				btn = KEYMAP_7;
			else if (I4_PrefKeySel <= KEYMAP_DOT)
				btn = KEYMAP_9;
			else
				btn = KEYMAP_8;
		}
		else if (btn == KEYMAP_SW1)
		{
			if (I4_PrefKeySel >= KEYMAP_MINUS)
				btn = KEYMAP_MINUS;
			else
				btn = KEYMAP_DIA;
		}
		else if (btn == KEYMAP_PLUS)
			btn = KEYMAP_EQU;
		else if ((btn & 3) == 3)
		{
			if (btn >= KEYMAP_S)
				btn = KEYMAP_ADV;
			else if (btn >= KEYMAP_9)
				btn = KEYMAP_DP0;
			else if (btn >= KEYMAP_DIA)
				btn = KEYMAP_SW1;
			else
				btn &= ~3;
		}
		else
			btn++;

		I4_KeySel = btn;

		// display cursor
		DispKeySel(btn);
		DispUpdate();
		break;
	}
}

// display print buffer
void Print141PF()
{
	int i, x;
	u8 ch;
	COLTYPE col;
	for (i = 0; i < DIG_NUM; i++)
	{
		// get character
		ch = I4_PrintBuf[i];

		// valid character to be printed:
		if (ch != 0)
		{
			// clear print buffer
			I4_PrintBuf[i] = 0;

			// prepare color
			col = ((ch & B7) == 0) ? PRINT_COLOR : PRINT_COLOR2;

			// print character
			DrawChar((char)(ch & 0x7f), PRINT_X + i*DIG_FONTW, PRINT_Y, col);

			// play sound
			if (!I4_SoundDone)
			{
				I4_SoundDone = True;	// sound was done
				PLAYSOUND(PrintSnd);
			}
		}
	}

	// display update
	DispUpdate();
}

// display lamps
void Lamp141PF()
{
	// get lamp status
	u8 lamp = I4_Lamp;
	u8 old = lamp ^ I4_LampOld;
	I4_LampOld = lamp;

	// memory lamp
	if ((old & B0) != 0)
	{
		if ((lamp & B0) == 0)
			DrawImgPal(LightOffImg, LightOffImg_Pal, LAMPM_X, LAMP_Y-LIGHT_Y, LAMPM_X, LAMP_Y, LAMP_W, LAMP_H, LIGHT_WALL);
		else			
			DrawImgPal(LightOnImg, LightOnImg_Pal, LAMPM_X, LAMP_Y-LIGHT_Y, LAMPM_X, LAMP_Y, LAMP_W, LAMP_H, LIGHT_WALL);
	}

	// overflow lamp
	if ((old & B1) != 0)
	{
		if ((lamp & B1) == 0)
			DrawImgPal(LightOffImg, LightOffImg_Pal, LAMPOVF_X, LAMP_Y-LIGHT_Y, LAMPOVF_X, LAMP_Y, LAMP_W, LAMP_H, LIGHT_WALL);
		else			
			DrawImgPal(LightOnImg, LightOnImg_Pal, LAMPOVF_X, LAMP_Y-LIGHT_Y, LAMPOVF_X, LAMP_Y, LAMP_W, LAMP_H, LIGHT_WALL);
	}

	// minus lamp
	if ((old & B2) != 0)
	{
		if ((lamp & B2) == 0)
			DrawImgPal(LightOffImg, LightOffImg_Pal, LAMPNEG_X, LAMP_Y-LIGHT_Y, LAMPNEG_X, LAMP_Y, LAMP_W, LAMP_H, LIGHT_WALL);
		else			
			DrawImgPal(LightOnImg, LightOnImg_Pal, LAMPNEG_X, LAMP_Y-LIGHT_Y, LAMPNEG_X, LAMP_Y, LAMP_W, LAMP_H, LIGHT_WALL);
	}

	// display update
	DispUpdate();
}

// advance paper
void Adv141PF()
{
#define ADV_DY	4	// number of lines to scroll in one step
	int i;
	for (i = DIG_FONTH; i > 0; i -= ADV_DY)
	{
		DrawScrollRect(PAPER_X, PAPER_Y, PAPER_WIDTH, PAPER_HEIGHT, (i > ADV_DY) ? ADV_DY : i, PAPER_COLOR);
		DispUpdate();
		WaitMs(10);
	}
	I4_SoundDone = False;	// sound was done
}

// update pressed buttons (except selected key)
void UpdateBtn()
{
	int k;
	Bool press;
	for (k = 0; k < KEYMAP_NUM2; k++)
	{
		if ((k != I4_KeySel) || I4_DispHelp)
		{
			press = PressedBtn(k);
			if (press != KeyMapPress[k]) DispBtn(k, press);
		}
	}
}

// display all keyboard
void DispKeyAll()
{
	// display all buttons
	int k;
	for (k = 0; k < KEYMAP_NUM2; k++) DispBtn(k, PressedBtn(k));

	// display lamps
	I4_LampOld = ~I4_Lamp;
	Lamp141PF();

	// display decimal point switch
	DPSwitch141PF();

	// dispay round switch
	RoundSwitch141PF();

	// display update
	DispUpdate();

	// selection not visible
	I4_KeySelOn = False;
	I4_KeySelTime = Time();
}

// initialize device
void Init141PF()
{
	// initialize variables
	I4_KeyShift = -1;	// keyboard shifter (10 bits, 0=row is selected)
	I4_PrintShift = 0;	// printer shifter (20 bits, 1=hammer is selected)
	I4_DP = 8;		// decimal point switch (0..8 except 7, default 8)
	I4_Round = ROUND_FL;	// rounding mode switch ROUND_* (default ROUND_FL)
	I4_Sector = 0;		// current drum sector (0..12, 0=index)
	I4_OldROM0 = 0;		// old state of shifter clocks
	I4_OldRAM0 = 0;		// old state of print hammers
	I4_PrintAdv = False;	// request to advance paper
	I4_Lamp = 0;		// lamp status
	I4_LampOld = 0;		// lamp status old
	I4_AdvPressed = False;	// ADV button is pressed
	I4_TestTime = Time();	// TEST time counter
	I4_Test = 1;		// TEST not active
	I4_PrintColor = 0;	// print color: 0 black, 0x80 red
	I4_SoundDone = False;	// sound was done

	// clear map of pressed keys
	memset(KeyMapPress, 0, sizeof(KeyMapPress));
	KeyMapBit = 0;

	// clear print buffer
	memset((void*)I4_PrintBuf, 0, sizeof(I4_PrintBuf));

	// setup callback functions
	i4004cpu.readrom = GetRomMem;
	i4004cpu.writerom = SetRomMem;
	i4004cpu.writeramport = SetRamPort;
	i4004cpu.writeromport = SetRomPort;
	i4004cpu.readromport = GetRomPort;
	i4004cpu.gettest = GetTest;
}

// ============================================================================
//                             Main function
// ============================================================================

// main function
int main()
{
	u32 k;
	u8 ch;

	// initialize font
	pDrawFont = Font141PF8x13;
	DrawFontHeight = 13;
	DrawFontWidth = 8;
	SelFontUpdate(); // update size of text screen

#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	DispHstxAllTerm();
	DispHstxStart(0);
#endif

#if USE_PICOPADHSTX && USE_DISPHSTX	// use PicoPadHSTX device configuration
	u32 sysclk = ClockGetHz(CLK_SYS);
#else
	// Find system clock in Hz that sets the most accurate PWM clock frequency.
	u32 sysclk = PWM_FindSysClk(EMU_CLKSYS_MIN*MHZ, EMU_CLKSYS_MAX*MHZ, EMU_FREQ*I4004_CLOCKMUL);

	// setup system clock to get precise frequency of emulation
	if (sysclk > 160*MHZ) VregSetVoltage(VREG_VOLTAGE_1_20);
	ClockPllSysFreq((sysclk+500)/1000);
#endif

	// initialize USB
	UsbHostInit();

	// initialize device
	Init141PF();

	// start emulation
	u32 freq = I4004_Start(&i4004cpu, EMU_PWM, EMU_FREQ);

	// idle sound
	PlaySoundChan(1, IdleSnd, count_of(IdleSnd), True, 1, 1, SNDFORM_PCM, 0);

	// debug display frequency (freq = 740 kHz, syclk = 133200 kHz, div = 45)
//	printf("freq=%d on sysclk=%d, div=%#f\n", freq, sysclk, (double)sysclk/(freq*I4004_CLOCKMUL));
//	WaitMs(5000);

	// display all
	DrawClear();
	DrawImgPal(KeybUpImg, KeybUpImg_Pal, 0, 0, KEYB_X, KEYB_Y, KEYB_WALL, KEYB_HALL, KEYB_WALL);
	DrawImgPal(LightOffImg, LightOffImg_Pal, 0, 0, LIGHT_X, LIGHT_Y, LIGHT_WALL, LIGHT_HALL, LIGHT_WALL);
	DrawImg4Pal(Logo1Img, Logo1Img_Pal, 0, 0, LOGO1_X, LOGO1_Y, LOGO1_W, LOGO1_H, LOGO1_W);
	DrawImg4Pal(Logo2Img, Logo2Img_Pal, 0, 0, LOGO2_X, LOGO2_Y, LOGO2_W, LOGO2_H, LOGO2_W);
	DrawRect(PAPER_X, PAPER_Y, PAPER_WIDTH, PAPER_HEIGHT, PAPER_COLOR);
	DispKeyAll();

#if EMU_DEBUG_SYNC
	u32 t0 = Time();
#endif
	// main loop
	while (True)
	{

#if EMU_DEBUG_SYNC	// 1 = debug measure time synchronization ... set this flag in the emu.h file (and rebuild all)

		u32 t = Time();
		if ((t - t0) >= 1000000)
		{
			t0 = t;
			sEmuDebClock* ec = &EmuDebClock;
			int used = (int)((u64)ec->used*100/ec->total);
			if (used > 999) used = 999;
			int mx = ec->maxused*100/ec->maxtot;
			if (mx < used) mx = used;
			if (mx > 999) mx = 999;
			printf("\rPC=%04X used=%d%% max=%d%%   ", i4004cpu.pc, used, mx);
			EmuDebClockRes = True;
			dsb();
			ec->reset = True;
		}

#endif // EMU_DEBUG_SYNC

		// display print buffer
		Print141PF();

		// display lamps
		Lamp141PF();

		// paper advance
		if (I4_PrintAdv)
		{
			I4_PrintAdv = False;
			Adv141PF();
		}

		// update pressed buttons (except selected key)
		UpdateBtn();

		// get USB key
		k = UsbGetKey();
		ch = (u8)(k & 0xff);

		switch (ch)
		{
		case HID_KEY_ARROW_RIGHT:
			CtrlKeySel(KEY_RIGHT);
			break;

		case HID_KEY_ARROW_LEFT:
			CtrlKeySel(KEY_LEFT);
			break;

		case HID_KEY_ARROW_UP:
			CtrlKeySel(KEY_UP);
			break;

		case HID_KEY_ARROW_DOWN:
			CtrlKeySel(KEY_DOWN);
			break;

		case HID_KEY_X:
			I4_DispHelp = !I4_DispHelp;
			DispKeyAll();
			break;

		// Esc: quit
		case HID_KEY_Y:
		case HID_KEY_ESCAPE:
			ResetToBootLoader();
			break;
		}

		// keypad
		ch = KeyGet();

		// control button selection (key = direction key KEY_* or NOKEY)
		CtrlKeySel(ch);

		switch (ch)
		{
		// X: display help
		case KEY_X:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			I4_DispHelp = !I4_DispHelp;

			// display all keyboard
			DispKeyAll();
			break;

		// reset to boot loader
		case KEY_Y:
			ResetToBootLoader();
			break;
		}
	}
}
