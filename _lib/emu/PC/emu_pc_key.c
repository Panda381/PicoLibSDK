
// ****************************************************************************
//
//                       IBM PC Emulator - Keyboard
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

// INT 09h keyboard
//   Passing keys through the scan code on the keyboard port is supported only partially,
//   for applications using direct code reading.
volatile u8 PC_Int9KeyNum = 0; // number of scan codes in key buffer
volatile u8 PC_Int9KeyScan[4]; // scan code key buffer (can be read through keyboard port 60h)
volatile Bool PC_KeyMenuReq = False; // requirement for game menu (key Y)
volatile Bool PC_KeyZoomReq = False; // requirement for zoom screen (key X)
volatile u32 PC_Int9KeyTime = 0; // time of last inserted key

// remap keypad codes to scan code (B7: extended scan code with 0xE0 prefix)
// - this table can be modified during program setup
u8 PC_KeypadScan[KEY_NUM+1] = {
	0,			// 0: no key
	B7 + PC_KEYSCAN_UP,	// 1: KEY_UP (up)
	B7 + PC_KEYSCAN_LEFT,	// 2: KEY_LEFT (left)
	B7 + PC_KEYSCAN_RIGHT,	// 3: KEY_RIGHT (right)
	B7 + PC_KEYSCAN_DOWN,	// 4: KEY_DOWN (down)
	PC_KEYSCAN_SCROLL,	// 5: KEY_X ScrollLock (zoom)
	B7 + PC_KEYSCAN_PAUSE,	// 6: KEY_Y Pause (menu)
	PC_KEYSCAN_ENTER,	// 7: KEY_A A (enter)
	PC_KEYSCAN_ESC,		// 8: KEY_B B (Esc)
};

// alternatives with modifier
u8 PC_KeypadScanU[KEY_NUM+1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // key with UP modifier
u8 PC_KeypadScanL[KEY_NUM+1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // key with LEFT modifier
u8 PC_KeypadScanR[KEY_NUM+1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // key with RIGHT modifier
u8 PC_KeypadScanD[KEY_NUM+1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // key with DOWN modifier
u8 PC_KeypadScanX[KEY_NUM+1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // key with X modifier
u8 PC_KeypadScanY[KEY_NUM+1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // key with Y modifier
u8 PC_KeypadScanA[KEY_NUM+1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // key with A modifier
u8 PC_KeypadScanB[KEY_NUM+1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // key with B modifier
u8 PC_KeypadMultiKey1 = 0; // last multi key 1
u8 PC_KeypadMultiKey2 = 0; // last multi key 2
u16 PC_KeypadMultiScan = 0; // last multi scan code

// remap scan code to buffer code, shifts no/shift/ctrl/alt (0x0000 = invalid code)
#define PC_KEYMAP_SCAN1_FIRST	0x01	// first scan code of 1st part
#define PC_KEYMAP_SCAN1_LAST	0x58	// last scan code of 1st part
//   1st part: scan code 0x01..0x58
const u16 PC_KeymapScanBuf1[4*(PC_KEYMAP_SCAN1_LAST+1-PC_KEYMAP_SCAN1_FIRST)] = {
	// no	Shift	Ctrl	Alt
	0x011B,	0x011B,	0x011B,	0x01F0,	// 0x01 PC_KEYSCAN_ESC		// Esc
	0x0231,	0x0221,	0x0000,	0x7800,	// 0x02 PC_KEYSCAN_1		// 1 !
	0x0332,	0x0340,	0x0300,	0x7900,	// 0x03 PC_KEYSCAN_2		// 2 @
	0x0433,	0x0423,	0x0000,	0x7A00,	// 0x04 PC_KEYSCAN_3		// 3 #
	0x0534,	0x0524,	0x0000,	0x7B00,	// 0x05 PC_KEYSCAN_4		// 4 $
	0x0635,	0x0625,	0x0000,	0x7C00,	// 0x06 PC_KEYSCAN_5		// 5 %
	0x0736,	0x075E,	0x071E,	0x7D00,	// 0x07 PC_KEYSCAN_6		// 6 ^
	0x0837,	0x0826,	0x0000,	0x7E00,	// 0x08 PC_KEYSCAN_7		// 7 &
	0x0938,	0x092A,	0x0000,	0x7F00,	// 0x09 PC_KEYSCAN_8		// 8 *
	0x0A39,	0x0A28,	0x0000,	0x8000,	// 0x0A PC_KEYSCAN_9		// 9 ( 
	0x0B30,	0x0B29,	0x0000,	0x8100,	// 0x0B PC_KEYSCAN_0		// 0 )
	0x0C2D,	0x0C5F,	0x0C1F,	0x8200,	// 0x0C PC_KEYSCAN_HYPHEN	// - _
	0x0D3D,	0x0D2B,	0x0000,	0x8300,	// 0x0D PC_KEYSCAN_EQU		// = +
	0x0E08,	0x0E08,	0x0E7F,	0x0EF0,	// 0x0E PC_KEYSCAN_BS		// BackSpace
	0x0F09,	0x0F00,	0x9400,	0xA500,	// 0x0F PC_KEYSCAN_TAB		// Tab
	0x1071,	0x1051,	0x1011,	0x1000,	// 0x10 PC_KEYSCAN_Q		// Q
	0x1177,	0x1157,	0x1117,	0x1100,	// 0x11 PC_KEYSCAN_W		// W
	0x1265,	0x1245,	0x1205,	0x1200,	// 0x12 PC_KEYSCAN_E		// E
	0x1372,	0x1352,	0x1312,	0x1300,	// 0x13 PC_KEYSCAN_R		// R
	0x1474,	0x1454,	0x1414,	0x1400,	// 0x14 PC_KEYSCAN_T		// T
	0x1579,	0x1559,	0x1519,	0x1500,	// 0x15 PC_KEYSCAN_Y		// Y
	0x1675,	0x1655,	0x1615,	0x1600,	// 0x16 PC_KEYSCAN_U		// U
	0x1769,	0x1749,	0x1709,	0x1700,	// 0x17 PC_KEYSCAN_I		// I
	0x186F,	0x184F,	0x180F,	0x1800,	// 0x18 PC_KEYSCAN_O		// O
	0x1970,	0x1950,	0x1910,	0x1900,	// 0x19 PC_KEYSCAN_P		// P
	0x1A5B,	0x1A7B,	0x1A1B,	0x1AF0,	// 0x1A PC_KEYSCAN_LBRACKET	// [ {
	0x1B5D,	0x1B7D,	0x1B1D,	0x1BF0,	// 0x1B PC_KEYSCAN_RBRACKET	// ] }
	0x1C0D,	0x1C0D,	0x1C0A,	0x1CF0,	// 0x1C PC_KEYSCAN_ENTER	// Enter
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x1D PC_KEYSCAN_LCTRL	// Left Ctrl
	0x1E61,	0x1E41,	0x1E01,	0x1E00,	// 0x1E PC_KEYSCAN_A		// A
	0x1F73,	0x1F53,	0x1F13,	0x1F00,	// 0x1F PC_KEYSCAN_S		// S
	0x2064,	0x2044,	0x2004,	0x2000,	// 0x20 PC_KEYSCAN_D		// D
	0x2166,	0x2146,	0x2106,	0x2100,	// 0x21 PC_KEYSCAN_F		// F
	0x2267,	0x2247,	0x2207,	0x2200,	// 0x22 PC_KEYSCAN_G		// G
	0x2368,	0x2348,	0x2308,	0x2300,	// 0x23 PC_KEYSCAN_H		// H
	0x246A,	0x244A,	0x240A,	0x2400,	// 0x24 PC_KEYSCAN_J		// J
	0x256B,	0x254B,	0x250B,	0x2500,	// 0x25 PC_KEYSCAN_K		// K
	0x266C,	0x264C,	0x260C,	0x2600,	// 0x26 PC_KEYSCAN_L		// L
	0x273B,	0x273A,	0x0000,	0x27F0,	// 0x27 PC_KEYSCAN_SEMICOLON	// ; :
	0x2827,	0x2822,	0x0000,	0x28F0,	// 0x28 PC_KEYSCAN_SQUOTE	// ' "
	0x2960,	0x297E,	0x0000,	0x29F0,	// 0x29 PC_KEYSCAN_BACKQUOTE	// ` ~
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x2A PC_KEYSCAN_LSHIFT	// Left Shift
	0x2B5C,	0x2B7C,	0x2B1C,	0x2BF0,	// 0x2B PC_KEYSCAN_BACKSLASH	// \ |
	0x2C7A,	0x2C5A,	0x2C1A,	0x2C00,	// 0x2C PC_KEYSCAN_Z		// Z
	0x2D78,	0x2D58,	0x2D18,	0x2D00,	// 0x2D PC_KEYSCAN_X		// X
	0x2E63,	0x2E43,	0x2E03,	0x2E00,	// 0x2E PC_KEYSCAN_C		// C
	0x2F76,	0x2F56,	0x2F16,	0x2F00,	// 0x2F PC_KEYSCAN_V		// V
	0x3062,	0x3042,	0x3002,	0x3000,	// 0x30 PC_KEYSCAN_B		// B
	0x316E,	0x314E,	0x310E,	0x3100,	// 0x31 PC_KEYSCAN_N		// N
	0x326D,	0x324D,	0x320D,	0x3200,	// 0x32 PC_KEYSCAN_M		// M
	0x332C,	0x333C,	0x0000,	0x33F0,	// 0x33 PC_KEYSCAN_COMMA	// , <
	0x342E,	0x343E,	0x0000,	0x34F0,	// 0x34 PC_KEYSCAN_PERIOD	// . >
	0x352F,	0x353F,	0x0000,	0x35F0,	// 0x35 PC_KEYSCAN_SLASH	// / ?
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x36 PC_KEYSCAN_RSHIFT	// Right Shift
	0x372A,	0x372A,	0x9600,	0x37F0,	// 0x37 PC_KEYSCAN_GREYSTAR	// Grey [*]
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x38 PC_KEYSCAN_LALT		// Left Alt
	0x3920,	0x3920,	0x3920,	0x3920,	// 0x39 PC_KEYSCAN_SPACE	// SpaceBar
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x3A PC_KEYSCAN_CAPSLOCK	// Caps Lock
	0x3B00,	0x5400,	0x5E00,	0x6800,	// 0x3B PC_KEYSCAN_F1		// F1
	0x3C00,	0x5500,	0x5F00,	0x6900,	// 0x3C PC_KEYSCAN_F2		// F2
	0x3D00,	0x5600,	0x6000,	0x6A00,	// 0x3D PC_KEYSCAN_F3		// F3
	0x3E00,	0x5700,	0x6100,	0x6B00,	// 0x3E PC_KEYSCAN_F4		// F4
	0x3F00,	0x5800,	0x6200,	0x6C00,	// 0x3F PC_KEYSCAN_F5		// F5
	0x4000,	0x5900,	0x6300,	0x6D00,	// 0x40 PC_KEYSCAN_F6		// F6
	0x4100,	0x5A00,	0x6400,	0x6E00,	// 0x41 PC_KEYSCAN_F7		// F7
	0x4200,	0x5B00,	0x6500,	0x6F00,	// 0x42 PC_KEYSCAN_F8		// F8
	0x4300,	0x5C00,	0x6600,	0x7000,	// 0x43 PC_KEYSCAN_F9		// F9
	0x4400,	0x5D00,	0x6700,	0x7100,	// 0x44 PC_KEYSCAN_F10		// F10
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x45 PC_KEYSCAN_NUMLOCK	// Num Lock
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x46 PC_KEYSCAN_SCROLL	// Scroll Lock
	0x4700,	0x4737,	0x7700,	0x9700,	// 0x47 PC_KEYSCAN_NUM7		// [7 Home]
	0x4800,	0x4838,	0x8D00,	0x9800,	// 0x48 PC_KEYSCAN_NUM8		// [8 Up]
	0x4900,	0x4939,	0x8400,	0x9900,	// 0x49 PC_KEYSCAN_NUM9		// [9 PgUp]
	0x4A2D,	0x4A2D,	0x8E00,	0x4AF0,	// 0x4A PC_KEYSCAN_GREYMINUS	// Grey [-]
	0x4B00,	0x4B34,	0x7300,	0x9B00,	// 0x4B PC_KEYSCAN_NUM4		// [4 Left]
	0x4CF0,	0x4C35,	0x8F00,	0x0000,	// 0x4C PC_KEYSCAN_NUM5		// [5]
	0x4D00,	0x4D36,	0x7400,	0x9D00,	// 0x4D PC_KEYSCAN_NUM6		// [6 Right]
	0x4E2B,	0x4E2B,	0x9000,	0x4EF0,	// 0x4E PC_KEYSCAN_GREYPLUS	// Grey [+]
	0x4F00,	0x4F31,	0x7500,	0x9F00,	// 0x4F PC_KEYSCAN_NUM1		// [1 End]
	0x5000,	0x5032,	0x9100,	0xA000,	// 0x50 PC_KEYSCAN_NUM2		// [2 Down]
	0x5100,	0x5133,	0x7600,	0xA100,	// 0x51 PC_KEYSCAN_NUM3		// [3 PgDn]
	0x5200,	0x5230,	0x9200,	0xA200,	// 0x52 PC_KEYSCAN_NUM0		// [0 Ins]
	0x5300,	0x532E,	0x9300,	0xA300,	// 0x53 PC_KEYSCAN_DECIMAL	// [. Del]
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x54 PC_KEYSCAN_SYSRQ	// SysRq (PrintScr with Alt)
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x55
	0x2B5C,	0x2B7C,	0x2B1C,	0x2BF0,	// 0x56 PC_KEYSCAN_BACKSLASH2	// \ | alternative
	0x8500,	0x8700,	0x8900,	0x8B00,	// 0x57 PC_KEYSCAN_F11		// F11
	0x8600,	0x8800,	0x8A00,	0x8C00,	// 0x58 PC_KEYSCAN_F12		// F12
};

//   2nd part: scan code 0x1C, with prefix 0xE0
const u16 PC_KeymapScanBuf2[] = {
	// no	Shift	Ctrl	Alt
	0x1C0D,	0x1C0D,	0x1C0A,	0x1CF0,	// 0x1C PC_KEYSCAN_GREYENTER	// grey [Enter]
};

//   3rd part: scan code 0x35..0x37, with prefix 0xE0
const u16 PC_KeymapScanBuf3[] = {
	// no	Shift	Ctrl	Alt
	0x352F,	0x352F,	0x0000,	0x35F0,	// 0x35 PC_KEYSCAN_GREYSLASH	// grey [/]
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x36
	0x7200,	0x7200,	0x7200,	0x0000,	// 0x37 PC_KEYSCAN_PRINTSCR	// Print Scrn
};

//   4th part: scan code 0x45..0x53, with prefix 0xE0
const u16 PC_KeymapScanBuf4[] = {
	// no	Shift	Ctrl	Alt
	0x4500,	0x4500,	0x4503,	0x45F0,	// 0x45 Pause (E1 1D 45 E1 9D C5)
	0x4603,	0x4603,	0x4603,	0x46F0,	// 0x46 Break (Pause with Ctrl)
	0x47E0,	0x47E0,	0x77E0,	0x9700,	// 0x47 PC_KEYSCAN_HOME		// Home
	0x48E0,	0x48E0,	0x8DE0,	0x9800,	// 0x48 PC_KEYSCAN_UP		// Up
	0x49E0,	0x49E0,	0x84E0,	0x9900,	// 0x49 PC_KEYSCAN_PGUP		// PgUp
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x4A
	0x4BE0,	0x4BE0,	0x73E0,	0x9B00,	// 0x4B PC_KEYSCAN_LEFT		// Left
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x4C
	0x4DE0,	0x4DE0,	0x74E0,	0x9D00,	// 0x4D PC_KEYSCAN_RIGHT	// Right
	0x0000,	0x0000,	0x0000,	0x0000,	// 0x4E
	0x4FE0,	0x4FE0,	0x75E0,	0x9F00,	// 0x4F PC_KEYSCAN_END		// End
	0x50E0,	0x50E0,	0x91E0,	0xA000,	// 0x50 PC_KEYSCAN_DOWN		// Down
	0x51E0,	0x51E0,	0x76E0,	0xA100,	// 0x51 PC_KEYSCAN_PGDN		// PgDn
	0x52E0,	0x52E0,	0x92E0,	0xA200,	// 0x52 PC_KEYSCAN_INS		// Ins
	0x53E0,	0x53E0,	0x93E0,	0xA300,	// 0x53 PC_KEYSCAN_DEL		// Del
};

// reset keyboard handler (on start of emulation)
void PC_Int9Reset()
{
	PC_Int9KeyNum = 0; // no key in buffer
	PC_KeyMenuReq = False; // requirement for game menu (key Y)
	PC_KeyZoomReq = False; // requirement for zoom screen (key X)
}

// write 2 scan codes into keyboard port (B7 = release, B8 = extended)
void PC_Int9SendKey(u16 scan, u16 scan2)
{
	int n = 0;

	// time of last key
	PC_Int9KeyTime = Time();

	// 1st scan code
	if ((scan & 0x7f) != 0)
	{
		// extended scan code
		if ((scan & B8) != 0) PC_Int9KeyScan[n++] = PC_KEYSCAN_EXT;

		// normal scan code
		PC_Int9KeyScan[n++] = (u8)scan;
	}

	// 2nd scan code
	if ((scan2 & 0x7f) != 0)
	{
		// extended scan code 2
		if ((scan2 & B8) != 0) PC_Int9KeyScan[n++] = PC_KEYSCAN_EXT;

		// normal scan code 2
		PC_Int9KeyScan[n++] = (u8)scan2;
	}

	// save number of keys
	dmb(); // data memory barrier
	PC_Int9KeyNum = (u8)n;

	// raise INT 09h
	if (n > 0) I8086_RaiseIRQ(PC_Cpu, I8086_IRQ_INT09);
}

// keyboard handler (called every 5 or 1 ms)
void PC_Int9Handler()
{
	u8 key;

	// key is not taken by INT 9 interrupt ... flush last key on timeout
	if (PC_Int9KeyNum != 0)
	{
		// check prepared key
		key = KeyGetRel();
		if (key == NOKEY) return; // no key, no timeout needed
		KeyRet(key); // return key

		// max timeout - if KEY_Y is pressed (=system menu), clear buffer faster
		u32 max = 600000;
		if (KeyPressedFast(KEY_Y)) max = 200000;

		// timeout is not up yet
		if ((u32)(Time() - PC_Int9KeyTime) < max) return;

		// delete unused key
		PC_Int9KeyNum = 0;
		dmb(); // data memory barrier
	}

	// get key from keypad, including key release KEY_RELEASE
	key = KeyGetRel();
	u8 keymask = key & KEY_MASK;
	u16 scan = 0;
	if (keymask != NOKEY)
	{
		// remap multiply key to scan code - press key
		if ((key & KEY_RELEASE) == 0)
		{
			// remap multiply keys
			u8 key2 = 0;
			if (KeyPressedFast(KEY_UP)) { scan = PC_KeypadScanU[keymask]; key2 = KEY_UP; }
			if ((scan == 0) && KeyPressedFast(KEY_LEFT)) { scan = PC_KeypadScanL[keymask]; key2 = KEY_LEFT; }
			if ((scan == 0) && KeyPressedFast(KEY_RIGHT)) { scan = PC_KeypadScanR[keymask]; key2 = KEY_RIGHT; }
			if ((scan == 0) && KeyPressedFast(KEY_DOWN)) { scan = PC_KeypadScanD[keymask]; key2 = KEY_DOWN; }
			if ((scan == 0) && KeyPressedFast(KEY_A)) { scan = PC_KeypadScanA[keymask]; key2 = KEY_A; }
			if ((scan == 0) && KeyPressedFast(KEY_B)) { scan = PC_KeypadScanB[keymask]; key2 = KEY_B; }
			if ((scan == 0) && KeyPressedFast(KEY_X)) { scan = PC_KeypadScanX[keymask]; key2 = KEY_X; }
			if ((scan == 0) && KeyPressedFast(KEY_Y)) { scan = PC_KeypadScanY[keymask]; key2 = KEY_Y; }

			// not for Zoom or Menu
			if ((scan != 0) && (scan != PC_KEYSCAN_SCROLL) && (scan != PC_KEYSCAN_PAUSE))
			{
				// prepare scan code of multiply key
				scan = (scan & 0x7f) | ((scan & B7) << 1); // prepare extended flag

				// save keys
				PC_KeypadMultiKey1 = key2; // 1st key
				PC_KeypadMultiKey2 = keymask; // 2nd key
				PC_KeypadMultiScan = scan; // scan code of multiply key

				// prepare scan code of 1st key - simulate its release
				u16 scan2 = PC_KeypadScan[key2];
				scan2 = (scan2 & 0x7f) | ((scan2 & B7) << 1); // prepare extended flag
				scan2 |= B7; // simulate release key

				// write scan codes into keyboard port
				PC_Int9SendKey(scan2, scan);
				return;
			}
		}

		// release multiply key
		else
		{
			// release multiply key, simulate press key 2
			if (keymask == PC_KeypadMultiKey1) // key 1 is released
			{
				// prepare scan code of 2nd key - simulate its press
				u16 scan2 = PC_KeypadScan[PC_KeypadMultiKey2];
				scan2 = (scan2 & 0x7f) | ((scan2 & B7) << 1); // prepare extended flag

				// write scan codes into keyboard port (release multiply key)
				PC_Int9SendKey(PC_KeypadMultiScan | B7, scan2);

				// destroy multiply keys
				PC_KeypadMultiKey1 = 0;
				PC_KeypadMultiKey2 = 0;
				return;
			}

			// release multiply key, simulate press key 1
			if (keymask == PC_KeypadMultiKey2) // key 2 is released
			{
				// prepare scan code of 1st key - simulate its press
				u16 scan2 = PC_KeypadScan[PC_KeypadMultiKey1];
				scan2 = (scan2 & 0x7f) | ((scan2 & B7) << 1); // prepare extended flag

				// write scan codes into keyboard port (release multiply key)
				PC_Int9SendKey(PC_KeypadMultiScan | B7, scan2);

				// destroy multiply keys
				PC_KeypadMultiKey1 = 0;
				PC_KeypadMultiKey2 = 0;
				return;
			}
		}

		// remap key to scan code (B7 = extended key code)
		if (scan == 0) scan = PC_KeypadScan[keymask];

		// no scan code
		if (scan == 0) return;

		// Zoom
		if (scan == PC_KEYSCAN_SCROLL)
		{
			// zoom request
			if ((key & KEY_RELEASE) == 0) PC_KeyZoomReq = True;
			return;
		}

		// Menu
		if (scan == B7 + PC_KEYSCAN_PAUSE)
		{
			// menu request
			if ((key & KEY_RELEASE) == 0) PC_KeyMenuReq = True;
			return;
		}

		scan = (scan & 0x7f) | ((scan & B7) << 1); // extended flag
		if ((key & KEY_RELEASE) != 0) scan |= B7; // release flag
	}

#if USE_USB_HOST_HID

	// keymask == 0
	else
	{
		// get key from USB keyboard, including key release
		//    bit 0..7: key code HID_KEY_* (NOKEY = no valid key code)
		//    bit 8..15: modifiers USB_KEY_MODI_*
		//    bit 16..23: ASCII character CH_* (NOCHAR = no valid character or release key)
		//    bit 24: 0=press key, 1=release key
		// In case of key release, ASCII character is invalid (= 0).
		u32 usb = UsbGetKeyRel();
		keymask = (u8)usb;

		// Zoom and Menu keys - without modifiers
		if ((usb & 0xf00) == 0)
		{
			// zoom: Scroll Lock
			if (keymask == HID_KEY_SCROLL_LOCK)
			{
				// zoom request
				if ((usb & B24) == 0) PC_KeyZoomReq = True;
				return;
			}

			// menu: Pause
			if (keymask == HID_KEY_PAUSE)
			{
				// menu request
				if ((usb & B24) == 0) PC_KeyMenuReq = True;
				return;
			}
		}

		if (usb != 0)
		{
			// convert USB key to PC scan code
			//    key ... USB key, as returned from UsbGetKeyRel() function (including release flag)
			// Output:
			//    bit 0..6: PC key scan code (0=invalid)
			//    bit 7: 1=release key
			//    bit 8: 1=extended key
			scan = UsbKeyToScan(usb);
		}
	}

#endif // USE_USB_HOST_HID

	// no scan code
	if (scan == 0) return;

	// write 2 scan codes into keyboard port
	PC_Int9SendKey(scan, 0);
}

// write key into keyboard buffer (returns 1 if buffer is full, 0 if success)
u8 PC_Int09Write(u16 key)
{
	u16 end = *(u16*)&PC_RAM_BASE[0x0482]; // end address of keyboard ring buffer + 1
	u16 w = *(u16*)&PC_RAM_BASE[0x041C]; // write pointer

	// shift write pointer
	u16 w2 = w + 2;
	if (w2 >= end) w2 = *(u16*)&PC_RAM_BASE[0x0480]; // start address of keyboard ring buffer

	// check if buffer is full
	u16 r = *(u16*)&PC_RAM_BASE[0x041A]; // read pointer
	if (w2 == r) return 1; // error, buffer is full

	// write key
	*(u16*)&PC_RAM_BASE[0x0400 + w] = key;
	*(u16*)&PC_RAM_BASE[0x041C] = w2; // save new write pointer
	return 0;
}

// INT 09h service - function
void PC_Int09fnc()
{
	// get next scan code
	u8 n = PC_Int9KeyNum;
	if (n == 0) return; // no data
	u8 key = PC_Int9KeyScan[0]; // get key (B7 = released key)

	// check more keys
	if (n > 1)
	{
		// time of last key
		PC_Int9KeyTime = Time();

		// shift buffer
		PC_Int9KeyScan[0] = PC_Int9KeyScan[1];
		PC_Int9KeyScan[1] = PC_Int9KeyScan[2];
		PC_Int9KeyScan[2] = PC_Int9KeyScan[3];

		// raise INT 09h
		I8086_RaiseIRQ(PC_Cpu, I8086_IRQ_INT09);
	}

	// decrease number of keys
	dmb(); // data memory barrier
	PC_Int9KeyNum = n-1;

	// extended flag
	if (key == PC_KEYSCAN_EXT)
	{
		// 0040:0096 (1) keyboard status byte 1
		//	B7: 1=read-ID in progress
		//	B6: 1=last code read was first of two IDs
		//	B5: 1=force Num Lock if read-ID on enhanced
		//	B4: 1=enhanced keyboard installed
		//	B3: 1=R-Alt pressed
		//	B2: 1=R-Ctrl pressed
		//	B1: 1=last code read was E0h
		//	B0: 1=last code read was E1h
		PC_RAM_BASE[0x0496] |= B1; // set E0h flag
		return;
	}

	// check extended flag
	Bool ext = (PC_RAM_BASE[0x0496] & (B1 | B0)) != 0;
	PC_RAM_BASE[0x0496] &= ~(B1 | B0); // clear E0h flag

	// update flags
	Bool rel = ((key & B7) != 0); // release flag
	u8 keymask = key & 0x7f; // raw scan code

	// Ctrl
	if (keymask == PC_KEYSCAN_LCTRL)
	{
		// 0040:0017 (1) keyboard status flag 1
		//	B7: Insert active
		//	B6: Caps Lock active
		//	B5: Num Lock active
		//	B4: Scroll Lock active
		//	B3: L-Alt or R-Alt pressed
		//	B2: L-Ctrl or R-Ctrl pressed
		//	B1: L-Shift pressed
		//	B0: R-Shift pressed
		if (rel)
			PC_RAM_BASE[0x0417] &= ~B2; // Ctrl released
		else
			PC_RAM_BASE[0x0417] |= B2; // Ctrl pressed

		if (ext)
		{
			// 0040:0096 (1) keyboard status byte 1
			//	B7: 1=read-ID in progress
			//	B6: 1=last code read was first of two IDs
			//	B5: 1=force Num Lock if read-ID on enhanced
			//	B4: 1=enhanced keyboard installed
			//	B3: 1=R-Alt pressed
			//	B2: 1=R-Ctrl pressed
			//	B1: 1=last code read was E0h
			//	B0: 1=last code read was E1h
			if (rel)
				PC_RAM_BASE[0x0496] &= ~B2; // Right Ctrl released
			else
				PC_RAM_BASE[0x0496] |= B2; // Right Ctrl pressed
		}
		else
		{
			// 0040:0018 (1) keyboard status flag 2
			//	B7: Insert pressed
			//	B6: Caps Lock pressed
			//	B5: Num Lock pressed
			//	B4: Scroll Lock pressed
			//	B3: Pause active
			//	B2: SysReq pressed
			//	B1: L-Alt pressed
			//	B0: L-Ctrl pressed
			if (rel)
				PC_RAM_BASE[0x0418] &= ~B0; // Left Ctrl released
			else
				PC_RAM_BASE[0x0418] |= B0; // Left Ctrl pressed
		}
		return;
	}

	// Alt
	if (keymask == PC_KEYSCAN_LALT)
	{
		// 0040:0017 (1) keyboard status flag 1
		//	B7: Insert active
		//	B6: Caps Lock active
		//	B5: Num Lock active
		//	B4: Scroll Lock active
		//	B3: L-Alt or R-Alt pressed
		//	B2: L-Ctrl or R-Ctrl pressed
		//	B1: L-Shift pressed
		//	B0: R-Shift pressed
		if (rel)
			PC_RAM_BASE[0x0417] &= ~B3; // Alt released
		else
			PC_RAM_BASE[0x0417] |= B3; // Alt pressed

		if (ext)
		{
			// 0040:0096 (1) keyboard status byte 1
			//	B7: 1=read-ID in progress
			//	B6: 1=last code read was first of two IDs
			//	B5: 1=force Num Lock if read-ID on enhanced
			//	B4: 1=enhanced keyboard installed
			//	B3: 1=R-Alt pressed
			//	B2: 1=R-Ctrl pressed
			//	B1: 1=last code read was E0h
			//	B0: 1=last code read was E1h
			if (rel)
				PC_RAM_BASE[0x0496] &= ~B3; // Right Alt released
			else
				PC_RAM_BASE[0x0496] |= B3; // Right Alt pressed
		}
		else
		{
			// 0040:0018 (1) keyboard status flag 2
			//	B7: Insert pressed
			//	B6: Caps Lock pressed
			//	B5: Num Lock pressed
			//	B4: Scroll Lock pressed
			//	B3: Pause active
			//	B2: SysReq pressed
			//	B1: L-Alt pressed
			//	B0: L-Ctrl pressed
			if (rel)
				PC_RAM_BASE[0x0418] &= ~B1; // Left Alt released
			else
				PC_RAM_BASE[0x0418] |= B1; // Left Alt pressed
		}
		return;
	}

	// Left Shift
	if (keymask == PC_KEYSCAN_LSHIFT)
	{
		// 0040:0017 (1) keyboard status flag 1
		//	B7: Insert active
		//	B6: Caps Lock active
		//	B5: Num Lock active
		//	B4: Scroll Lock active
		//	B3: L-Alt or R-Alt pressed
		//	B2: L-Ctrl or R-Ctrl pressed
		//	B1: L-Shift pressed
		//	B0: R-Shift pressed
		if (rel)
			PC_RAM_BASE[0x0417] &= ~B1; // Left Shift released
		else
			PC_RAM_BASE[0x0417] |= B1; // Left Shift pressed
		return;
	}

	// Right Shift
	if (keymask == PC_KEYSCAN_RSHIFT)
	{
		// 0040:0017 (1) keyboard status flag 1
		//	B7: Insert active
		//	B6: Caps Lock active
		//	B5: Num Lock active
		//	B4: Scroll Lock active
		//	B3: L-Alt or R-Alt pressed
		//	B2: L-Ctrl or R-Ctrl pressed
		//	B1: L-Shift pressed
		//	B0: R-Shift pressed
		if (rel)
			PC_RAM_BASE[0x0417] &= ~B0; // Right Shift released
		else
			PC_RAM_BASE[0x0417] |= B0; // Right Shift pressed
		return;
	}

	// Caps Lock
	if (keymask == PC_KEYSCAN_CAPSLOCK)
	{
		// 0040:0018 (1) keyboard status flag 2
		//	B7: Insert pressed
		//	B6: Caps Lock pressed
		//	B5: Num Lock pressed
		//	B4: Scroll Lock pressed
		//	B3: Pause active
		//	B2: SysReq pressed
		//	B1: L-Alt pressed
		//	B0: L-Ctrl pressed
		if (rel)
			PC_RAM_BASE[0x0418] &= ~B6; // Caps Lock released
		else
		{
			PC_RAM_BASE[0x0418] |= B6; // Caps Lock pressed

			// 0040:0017 (1) keyboard status flag 1
			//	B7: Insert active
			//	B6: Caps Lock active
			//	B5: Num Lock active
			//	B4: Scroll Lock active
			//	B3: L-Alt or R-Alt pressed
			//	B2: L-Ctrl or R-Ctrl pressed
			//	B1: L-Shift pressed
			//	B0: R-Shift pressed
			PC_RAM_BASE[0x0417] ^= B6; // Caps Lock invert
		}
		return;
	}

	// Num Lock
	if ((keymask == PC_KEYSCAN_NUMLOCK) && !ext) // Num Lock with ext is Pause
	{
		// 0040:0018 (1) keyboard status flag 2
		//	B7: Insert pressed
		//	B6: Caps Lock pressed
		//	B5: Num Lock pressed
		//	B4: Scroll Lock pressed
		//	B3: Pause active
		//	B2: SysReq pressed
		//	B1: L-Alt pressed
		//	B0: L-Ctrl pressed
		if (rel)
			PC_RAM_BASE[0x0418] &= ~B5; // Num Lock released
		else
		{
			PC_RAM_BASE[0x0418] |= B5; // Num Lock pressed

			// 0040:0017 (1) keyboard status flag 1
			//	B7: Insert active
			//	B6: Caps Lock active
			//	B5: Num Lock active
			//	B4: Scroll Lock active
			//	B3: L-Alt or R-Alt pressed
			//	B2: L-Ctrl or R-Ctrl pressed
			//	B1: L-Shift pressed
			//	B0: R-Shift pressed
			PC_RAM_BASE[0x0417] ^= B5; // Num Lock invert
		}
		return;
	}

	// Scroll Lock
	if ((keymask == PC_KEYSCAN_SCROLL) && !ext) // Scroll Lock with ext is Break
	{
		// 0040:0018 (1) keyboard status flag 2
		//	B7: Insert pressed
		//	B6: Caps Lock pressed
		//	B5: Num Lock pressed
		//	B4: Scroll Lock pressed
		//	B3: Pause active
		//	B2: SysReq pressed
		//	B1: L-Alt pressed
		//	B0: L-Ctrl pressed
		if (rel)
			PC_RAM_BASE[0x0418] &= ~B4; // Scroll Lock released
		else
		{
			PC_RAM_BASE[0x0418] |= B4; // Scroll Lock pressed

			// 0040:0017 (1) keyboard status flag 1
			//	B7: Insert active
			//	B6: Caps Lock active
			//	B5: Num Lock active
			//	B4: Scroll Lock active
			//	B3: L-Alt or R-Alt pressed
			//	B2: L-Ctrl or R-Ctrl pressed
			//	B1: L-Shift pressed
			//	B0: R-Shift pressed
			PC_RAM_BASE[0x0417] ^= B4; // Scroll Lock invert
		}
		return;
	}

	// Insert
	if (ext && (keymask == PC_KEYSCAN_INS))
	{
		// 0040:0018 (1) keyboard status flag 2
		//	B7: Insert pressed
		//	B6: Caps Lock pressed
		//	B5: Num Lock pressed
		//	B4: Scroll Lock pressed
		//	B3: Pause active
		//	B2: SysReq pressed
		//	B1: L-Alt pressed
		//	B0: L-Ctrl pressed
		if (rel)
			PC_RAM_BASE[0x0418] &= ~B7; // Insert released
		else
		{
			PC_RAM_BASE[0x0418] |= B7; // Insert pressed

			// 0040:0017 (1) keyboard status flag 1
			//	B7: Insert active
			//	B6: Caps Lock active
			//	B5: Num Lock active
			//	B4: Scroll Lock active
			//	B3: L-Alt or R-Alt pressed
			//	B2: L-Ctrl or R-Ctrl pressed
			//	B1: L-Shift pressed
			//	B0: R-Shift pressed
			PC_RAM_BASE[0x0417] ^= B7; // Insert invert
		}
	}

	// Pause
	if (ext && (keymask == PC_KEYSCAN_PAUSE) && !rel)
	{
		// Break (Ctrl+Pause)
		if ((PC_RAM_BASE[0x0417] & B2) != 0) // check Ctrl flag
		{
			// 0040:0071 (1) bit 7 = Ctrl+Break pressed flag
			PC_RAM_BASE[0x0471] = B7;

			// raise INT 1Bh interrupt
			I8086_RaiseIRQ(PC_Cpu, I8086_IRQ_INT1B);
		}
		else
		{
			// 0040:0018 (1) keyboard status flag 2
			//	B7: Insert pressed
			//	B6: Caps Lock pressed
			//	B5: Num Lock pressed
			//	B4: Scroll Lock pressed
			//	B3: Pause active
			//	B2: SysReq pressed
			//	B1: L-Alt pressed
			//	B0: L-Ctrl pressed
			PC_RAM_BASE[0x0418] ^= B3; // Pause invert
		}
	}

	// Break (Pause with Ctrl)
	if (ext && (keymask == PC_KEYSCAN_BREAK) && !rel)
	{
		// 0040:0071 (1) bit 7 = Ctrl+Break pressed flag
		PC_RAM_BASE[0x0471] = B7;

		// raise INT 1Bh interrupt
		I8086_RaiseIRQ(PC_Cpu, I8086_IRQ_INT1B);
	}

	// SysReq (PrintScr with Alt)
	if ((keymask == PC_KEYSCAN_SYSRQ) ||
		(ext && (keymask == PC_KEYSCAN_PRINTSCR) && ((PC_RAM_BASE[0x0417] & B3) != 0)))
	{
		// 0040:0018 (1) keyboard status flag 2
		//	B7: Insert pressed
		//	B6: Caps Lock pressed
		//	B5: Num Lock pressed
		//	B4: Scroll Lock pressed
		//	B3: Pause active
		//	B2: SysReq pressed
		//	B1: L-Alt pressed
		//	B0: L-Ctrl pressed
		if (rel)
			PC_RAM_BASE[0x0418] &= ~B2; // SysReq released
		else
			PC_RAM_BASE[0x0418] |= B2; // SysReq pressed
	}

	// release
	if (rel) return;

	// Num Lock
	Bool shift = ((PC_RAM_BASE[0x0417] & (B0 | B1)) != 0);
	if (!ext && ((PC_RAM_BASE[0x0417] & B5) != 0) && (keymask >= 0x47) && (keymask <= 0x53))
		shift = !shift;

	// remap key
	u8 off = 0;
	if (shift) off = 1; // Shift
	if ((PC_RAM_BASE[0x0417] & B2) != 0) off = 2; // Ctrl
	if ((PC_RAM_BASE[0x0417] & B3) != 0) off = 3; // Alt

	u16 scan = 0;

	// normal key
	if (!ext)
	{
		if ((keymask >= PC_KEYMAP_SCAN1_FIRST) && (keymask <= PC_KEYMAP_SCAN1_LAST))
			scan = PC_KeymapScanBuf1[4*(keymask - PC_KEYMAP_SCAN1_FIRST) + off];
	}

	// extended key
	else
	{
		if (keymask == 0x1c)
			scan = PC_KeymapScanBuf2[off];
		else if ((keymask >= 0x35) && (keymask <= 0x37))
			scan = PC_KeymapScanBuf3[4*(keymask - 0x35) + off];
		else if ((keymask >= 0x45) && (keymask <= 0x53))
			scan = PC_KeymapScanBuf4[4*(keymask - 0x45) + off];

#if USE_EMUSCREENSHOT		// use emulator screen shots
		// PrintScreen
		if (scan == 0x7200)
		{
			DoEmuScreenShot = True;	// request to do emulator screenshot
			scan = 0;
		}
#endif
	}

	// Caps Lock
	if (((PC_RAM_BASE[0x0417] & B6) != 0) &&
		((((u8)scan >= (u8)'A') && ((u8)scan <= (u8)'Z')) ||
		(((u8)scan >= (u8)'a') && ((u8)scan <= (u8)'z'))))
	{
		scan ^= 0x20;
	}

	// save key
	if (scan != 0)
	{
		// Ctrl+Break is written asi 0x0000 into keyboard buffer
		if ((scan == 0x4503) || (scan == 0x4603))
			PC_Int09Write(0);
		else
			// write key into keyboard buffer (returns False if buffer is full)
			PC_Int09Write(scan);
	}
}

// input key into cpu->AX
//   ext ... use extended code
// Returns: I8086_INS_NOP if no key is ready (registers stay unchanged), or I8086_INS_IRET if key was loaded into cpu->AX
u8 PC_InCh(Bool ext)
{
	// return instruction IRET
	u8 res = I8086_INS_IRET;

	// get pointers
	u16 r = *(u16*)&PC_RAM_BASE[0x041A]; // read pointer
	u16 w = *(u16*)&PC_RAM_BASE[0x041C]; // write pointer

	// no key available, waiting loop
	if (r == w)
	{
		// return instruction NOP
		res = I8086_INS_NOP;
	}
	else
	{
		// load key
		u16 key = *(u16*)&PC_RAM_BASE[0x0400 + r];

		// shift read pointer
		r += 2;
		u16 end = *(u16*)&PC_RAM_BASE[0x0482]; // end address of keyboard ring buffer + 1
		if (r >= end) r = *(u16*)&PC_RAM_BASE[0x0480]; // rewind to start address of keyboard ring buffer
		*(u16*)&PC_RAM_BASE[0x041A] = r; // save new read pointer

		// discard extended code
		if (!ext)
		{
			u8 ch = (u8)key;
			if ((ch == 0xe0) || (ch == 0xe1) || (ch == 0xf0)) key &= 0xff00;
		}

		PC_Cpu->ax = key;
	}

	return res;
}

// DOS Ctrl+Break or Ctrl+C
void PC_DosBreak()
{
	// get pointers
	u16 r = *(u16*)&PC_RAM_BASE[0x041A]; // read pointer
	u16 w = *(u16*)&PC_RAM_BASE[0x041C]; // write pointer

	// check if some key is ready
	if (r != w)
	{
		// check break key (Ctrl+Break or Ctrl+C)
		u16 key = *(u16*)&PC_RAM_BASE[0x0400 + r];
		if ((key == 0x0000) || ((u8)key == 3)) I8086_RaiseIRQ(PC_Cpu, I8086_IRQ_INT23);
	}
}

// INT 16h service - function; returns instruction code I8086_INS_IRET if continue, or I8086_INS_NOP if waiting in loop
u8 PC_Int16fnc()
{
	// return instruction IRET
	u8 res = I8086_INS_IRET;

	// pointer to CPU
	sI8086* cpu = PC_Cpu;

	// get AH function
	u8 ah = cpu->ah;

	switch (ah)
	{
	// get keystroke
	case 0x00:
	// get extended keystroke
	case 0x10:
		// input key into cpu->AX
		//   ext ... use extended code
		// Returns: I8086_INS_NOP if no key is ready, or I8086_INS_IRET if key was loaded into cpu->AX
		res = PC_InCh(ah != 0x00);
		break;

	// check keystroke
	case 0x01:
	// check extended keystroke
	case 0x11:
		{
			// get pointers
			u16 r = *(u16*)&PC_RAM_BASE[0x041A]; // read pointer
			u16 w = *(u16*)&PC_RAM_BASE[0x041C]; // write pointer

			// load flags
			u16 f = I8086_GetWord(cpu, cpu->ss, cpu->sp+4);

			// no key available, set zero flag
			if (r == w)
			{
				// set zero flag
				f |= I8086_ZF;
			}

			else
			{
				// clear zero flag
				f &= ~I8086_ZF;

				// load key
				u16 key = *(u16*)&PC_RAM_BASE[0x0400 + r];

				// discard extended code
				if (ah == 0x01)
				{
					u8 ch = (u8)key;
					if ((ch == 0xe0) || (ch == 0xe1) || (ch == 0xf0)) key &= 0xff00;
				}

				cpu->ax = key;
			}

			// write new flags
			I8086_SetWord(cpu, cpu->ss, cpu->sp+4, f);
		}
		break;

	// get shift flags
	case 0x02:
	// get extended shift flags
	case 0x12:
		{
			// 0040:0017 (1) keyboard status flag 1
			//	B7: Insert active
			//	B6: Caps Lock active
			//	B5: Num Lock active
			//	B4: Scroll Lock active
			//	B3: L-Alt or R-Alt pressed
			//	B2: L-Ctrl or R-Ctrl pressed
			//	B1: L-Shift pressed
			//	B0: R-Shift pressed
			cpu->al = PC_RAM_BASE[0x0417];

			// 0040:0018 (1) keyboard status flag 2
			//	B7: Insert pressed
			//	B6: Caps Lock pressed
			//	B5: Num Lock pressed
			//	B4: Scroll Lock pressed
			//	B3: Pause active
			//	B2: SysReq pressed
			//	B1: L-Alt pressed
			//	B0: L-Ctrl pressed
			u8 a = PC_RAM_BASE[0x0418];

			// 0040:0096 (1) keyboard status byte 1
			//	B7: 1=read-ID in progress
			//	B6: 1=last code read was first of two IDs
			//	B5: 1=force Num Lock if read-ID on enhanced
			//	B4: 1=enhanced keyboard installed
			//	B3: 1=R-Alt pressed
			//	B2: 1=R-Ctrl pressed
			//	B1: 1=last code read was E0h
			//	B0: 1=last code read was E1h
			u8 b = PC_RAM_BASE[0x0496];

			// result AH
			//	B7: SysReq pressed
			//	B6: Caps Lock pressed
			//	B5: Num Lock pressed
			//	B4: Scroll Lock pressed
			//	B3: 1=R-Alt pressed
			//	B2: 1=R-Ctrl pressed
			//	B1: L-Alt pressed
			//	B0: L-Ctrl pressed
			cpu->ah = (a & (B6|B5|B4|B1|B0)) | (b & (B3|B2)) | ((a & B2) << 5);
		}
		break;

	// flush keyboard buffer (Tandy 2000)
	case 0x04:
		// write pointer -> read pointer
		*(u16*)&PC_RAM_BASE[0x041A] = *(u16*)&PC_RAM_BASE[0x041C];
		break;

	// write key into keyboard buffer
	case 0x05:
		// write key into keyboard buffer (returns 1 if buffer is full, 0 if success)
		cpu->al = PC_Int09Write(cpu->cx);
		break;

	// get keyboard functionality
	case 0x09:
		// functions 10h-12h supported
		cpu->al = B5;
		break;
	}

	return res;
}
