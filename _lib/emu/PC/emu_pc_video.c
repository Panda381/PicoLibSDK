
// ****************************************************************************
//
//                    IBM PC Emulator - Video service, INT 10h
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

sPC_Vmode PC_Vmode;		// current videomode
u8 PC_EgaSelReg = 0;		// EGA select setup register 0x3C4
u8 PC_EgaWPlanes = 0x0F;	// EGA write planes 0x3C5/0x02
u8 PC_EgaCtrlReg = 0;		// EGA select control register 0x3CE
u8 PC_EgaCtrlReg0 = 0;		// EGA control register 0x3CF/0x00 - color
u8 PC_EgaCtrlReg1 = 0;		// EGA control register 0x3CF/0x01 - select planes to write color instead data
u8 PC_EgaCtrlReg8 = 0xff;	// EGA control register 0x3CF/0x08 - select bits to write

u8 PC_CgaSelReg = 12;		// CGA select setup register 0x3B4, 0x3D4

// current 2-color CGA palettes
u16 PC_CurPal2[2];

// current 4-color CGA palettes
u16 PC_CurPal4[4];

// current 16-color EGA palettes
u16 PC_CurPal16[16];

// current 256-color VGA palette
u16 PC_CurPal256[256];

// graphics card order of memory size
const u8 PC_CardMem[PC_CARD_NUM] = {
	PC_VMEM_4K,	// 0 MDA (4 KB VRAM)
	PC_VMEM_16K,	// 1 CGA (16 KB VRAM)
	PC_VMEM_32K,	// 2 PCjr (32 KB VRAM)
	PC_VMEM_64K,	// 3 Tandy 1000 (64 KB VRAM)
	PC_VMEM_64K,	// 4 EGA64 (64 KB VRAM)
	PC_VMEM_128K,	// 5 EGA128 (128 KB VRAM)
	PC_VMEM_64K,	// 6 MCGA (64 KB VRAM)
	PC_VMEM_256K,	// 7 VGA (256 KB VRAM, real 160 KB)
};

// graphics card max. videomode
const u8 PC_CardMaxMode[PC_CARD_NUM] = {
	7,	// 0 MDA (4 KB VRAM)
	7,	// 1 CGA (16 KB VRAM)
	10,	// 2 PCjr (32 KB VRAM)
	11,	// 3 Tandy 1000 (64 KB VRAM)
	16,	// 4 EGA64 (64 KB VRAM)
	16,	// 5 EGA128 (128 KB VRAM)
	19,	// 6 MCGA (64 KB VRAM)
	19,	// 7 VGA (256 KB VRAM, real 160 KB)
};

// 2-color CGA palettes (black/white)
const u16 PC_Pal2[2] = {
	0,							// 0: black
	COL_WHITE,						// 1: light white
};

// 2-color CGA palettes inverted (white/black)
const u16 PC_Pal2Inv[2] = {
	COL_WHITE,						// 0: light white
	0,							// 1: black
};

// 4-color CGA palettes 0, low intensity (green/red/brown)
const u16 PC_Pal4_0_low[4] = {
	0,							// 0: black
	PC_EGA_GREEN,						// 1: dark green
	PC_EGA_RED,						// 2: dark red
	PC_EGA_GREEN2+PC_EGA_RED,				// 3: brown
};

// 4-color CGA palettes 0, high intensity (green/red/yellow)
const u16 PC_Pal4_0_high[4] = {
	0,							// 0: black
	PC_EGA_GREY+PC_EGA_GREEN,				// 1: light green
	PC_EGA_GREY+PC_EGA_RED,					// 2: light red
	PC_EGA_GREY+PC_EGA_GREEN+PC_EGA_RED,			// 3: light yellow
};

// 4-color CGA palettes 1, low intensity (cyan/magenta/white)
const u16 PC_Pal4_1_low[4] = {
	0,							// 0: black
	PC_EGA_BLUE+PC_EGA_GREEN,				// 1: dark cyan
	PC_EGA_BLUE+PC_EGA_RED,					// 2: dark magenta
	PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,			// 3: dark white
};

// 4-color CGA palettes 1, high intensity (cyan/magenta/white)
const u16 PC_Pal4_1_high[4] = {
	0,							// 0: black
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN,			// 1: light cyan
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_RED,			// 2: light magenta
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,	// 3: light white
};

// 4-color CGA palettes 2, low intensity (cyan/red/white)
const u16 PC_Pal4_2_low[4] = {
	0,							// 0: black
	PC_EGA_BLUE+PC_EGA_GREEN,				// 1: dark cyan
	PC_EGA_RED,						// 2: dark red
	PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,			// 3: dark white
};

// 4-color CGA palettes 2, high intensity (cyan/red/white)
const u16 PC_Pal4_2_high[4] = {
	0,							// 0: black
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN,			// 1: light cyan
	PC_EGA_GREY+PC_EGA_RED,					// 2: light red
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,	// 3: light white
};

// select CGA 4-color palette by CRT palette register
//	B5: palette 0 (red-green-brown) or 1 (cyan-magenta-white)
//	B4: intense colors in graphics, intense background in text mode
const u16* PC_Pal4_CgaCrt[4] = {
	PC_Pal4_0_low,
	PC_Pal4_0_high,
	PC_Pal4_1_low,
	PC_Pal4_1_high
};

// 16-color EGA palettes
const u16 PC_Pal16[16] = {
	0,							// 0: (0x00) black
	PC_EGA_BLUE,						// 1: (0x01) dark blue
	PC_EGA_GREEN,						// 2: (0x02) dark green
	PC_EGA_BLUE+PC_EGA_GREEN,				// 3: (0x03) dark cyan
	PC_EGA_RED,						// 4: (0x04) dark red
	PC_EGA_BLUE+PC_EGA_RED,					// 5: (0x05) dark magenta
	PC_EGA_GREEN2+PC_EGA_RED,				// 6: (0x14) brown
	PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,			// 7: (0x07) dark white

	PC_EGA_GREY,						// 0: (0x38) black
	PC_EGA_GREY+PC_EGA_BLUE,				// 1: (0x39) light blue
	PC_EGA_GREY+PC_EGA_GREEN,				// 2: (0x3A) light green
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN,			// 3: (0x3B) light cyan
	PC_EGA_GREY+PC_EGA_RED,					// 4: (0x3C) light red
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_RED,			// 5: (0x3D) light magenta
	PC_EGA_GREY+PC_EGA_GREEN+PC_EGA_RED,			// 6: (0x3E) light yellow
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,	// 7: (0x3F) light white
};

// 16-color MDA palettes (to simulate MDA using CGA text mode)
const u16 PC_PalMDAWhite[16] = {
	0,							// 0: black
	COLOR(170,170,170),					// 1: grey
	COLOR(170,170,170),					// 2: grey
	COLOR(170,170,170),					// 3: grey
	COLOR(170,170,170),					// 4: grey
	COLOR(170,170,170),					// 5: grey
	COLOR(170,170,170),					// 6: grey
	COLOR(170,170,170),					// 7: grey

	0,							// 8: black
	COL_WHITE,						// 9: white
	COL_WHITE,						// 10: white
	COL_WHITE,						// 11: white
	COL_WHITE,						// 12: white
	COL_WHITE,						// 13: white
	COL_WHITE,						// 14: white
	COL_WHITE,						// 15: white
};

const u16 PC_PalMDAGreen[16] = {
	0,							// 0: black
	PC_EGA_GREEN,						// 1: dark green
	PC_EGA_GREEN,						// 2: dark green
	PC_EGA_GREEN,						// 3: dark green
	PC_EGA_GREEN,						// 4: dark green
	PC_EGA_GREEN,						// 5: dark green
	PC_EGA_GREEN,						// 6: dark green
	PC_EGA_GREEN,						// 7: dark green

	0,							// 8: black
	COL_GREEN,						// 9: green
	COL_GREEN,						// 10: green
	COL_GREEN,						// 11: green
	COL_GREEN,						// 12: green
	COL_GREEN,						// 13: green
	COL_GREEN,						// 14: green
	COL_GREEN,						// 15: green
};

const u16 PC_PalMDAPlasma[16] = {
	0,							// 0: black
	COLOR(170,85,0),					// 1: dark orange
	COLOR(170,85,0),					// 2: dark orange
	COLOR(170,85,0),					// 3: dark orange
	COLOR(170,85,0),					// 4: dark orange
	COLOR(170,85,0),					// 5: dark orange
	COLOR(170,85,0),					// 6: dark orange
	COLOR(170,85,0),					// 7: dark orange

	0,							// 8: black
	COLOR(255,170,0),					// 9: orange
	COLOR(255,170,0),					// 10: orange
	COLOR(255,170,0),					// 11: orange
	COLOR(255,170,0),					// 12: orange
	COLOR(255,170,0),					// 13: orange
	COLOR(255,170,0),					// 14: orange
	COLOR(255,170,0),					// 15: orange
};

// 16-color EGA64 palettes
const u16 PC_Pal16Ega64[16] = {
	0,							// 0: black
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN,			// 1: light cyan
	PC_EGA_GREY+PC_EGA_RED,					// 2: light red
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,	// 3: light white

	PC_EGA_GREY+PC_EGA_RED,					// 4: light red
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,	// 5: light white
	PC_EGA_GREY+PC_EGA_RED,					// 6: light red
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,	// 7: light white

	PC_EGA_GREY+PC_EGA_RED,					// 8: light red
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,	// 9: light white
	PC_EGA_GREY+PC_EGA_RED,					// 10: light red
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,	// 11: light white

	PC_EGA_GREY+PC_EGA_RED,					// 12: light red
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,	// 13: light white
	PC_EGA_GREY+PC_EGA_RED,					// 14: light red
	PC_EGA_GREY+PC_EGA_BLUE+PC_EGA_GREEN+PC_EGA_RED,	// 15: light white
};

// VGA 256-color default palette
const u16 PC_Pal256[256] =
{
	COLOR(0x00, 0x00, 0x00),	// 0
	COLOR(0x00, 0x00, 0xaa),	// 1
	COLOR(0x00, 0xaa, 0x00),	// 2
	COLOR(0x00, 0xaa, 0xaa),	// 3
	COLOR(0xaa, 0x00, 0x00),	// 4
	COLOR(0xaa, 0x00, 0xaa),	// 5
	COLOR(0xaa, 0x55, 0x00),	// 6
	COLOR(0xaa, 0xaa, 0xaa),	// 7
	COLOR(0x55, 0x55, 0x55),	// 8
	COLOR(0x55, 0x55, 0xff),	// 9
	COLOR(0x55, 0xff, 0x55),	// 10
	COLOR(0x55, 0xff, 0xff),	// 11
	COLOR(0xff, 0x55, 0x55),	// 12
	COLOR(0xff, 0x55, 0xff),	// 13
	COLOR(0xff, 0xff, 0x55),	// 14
	COLOR(0xff, 0xff, 0xff),	// 15
	COLOR(0x00, 0x00, 0x00),	// 16
	COLOR(0x14, 0x14, 0x14),	// 17
	COLOR(0x20, 0x20, 0x20),	// 18
	COLOR(0x2c, 0x2c, 0x2c),	// 19
	COLOR(0x38, 0x38, 0x38),	// 20
	COLOR(0x45, 0x45, 0x45),	// 21
	COLOR(0x51, 0x51, 0x51),	// 22
	COLOR(0x61, 0x61, 0x61),	// 23
	COLOR(0x71, 0x71, 0x71),	// 24
	COLOR(0x82, 0x82, 0x82),	// 25
	COLOR(0x92, 0x92, 0x92),	// 26
	COLOR(0xa2, 0xa2, 0xa2),	// 27
	COLOR(0xb6, 0xb6, 0xb6),	// 28
	COLOR(0xcb, 0xcb, 0xcb),	// 29
	COLOR(0xe3, 0xe3, 0xe3),	// 30
	COLOR(0xff, 0xff, 0xff),	// 31
	COLOR(0x00, 0x00, 0xff),	// 32
	COLOR(0x41, 0x00, 0xff),	// 33
	COLOR(0x7d, 0x00, 0xff),	// 34
	COLOR(0xbe, 0x00, 0xff),	// 35
	COLOR(0xff, 0x00, 0xff),	// 36
	COLOR(0xff, 0x00, 0xbe),	// 37
	COLOR(0xff, 0x00, 0x7d),	// 38
	COLOR(0xff, 0x00, 0x41),	// 39
	COLOR(0xff, 0x00, 0x00),	// 40
	COLOR(0xff, 0x41, 0x00),	// 41
	COLOR(0xff, 0x7d, 0x00),	// 42
	COLOR(0xff, 0xbe, 0x00),	// 43
	COLOR(0xff, 0xff, 0x00),	// 44
	COLOR(0xbe, 0xff, 0x00),	// 45
	COLOR(0x7d, 0xff, 0x00),	// 46
	COLOR(0x41, 0xff, 0x00),	// 47
	COLOR(0x00, 0xff, 0x00),	// 48
	COLOR(0x00, 0xff, 0x41),	// 49
	COLOR(0x00, 0xff, 0x7d),	// 50
	COLOR(0x00, 0xff, 0xbe),	// 51
	COLOR(0x00, 0xff, 0xff),	// 52
	COLOR(0x00, 0xbe, 0xff),	// 53
	COLOR(0x00, 0x7d, 0xff),	// 54
	COLOR(0x00, 0x41, 0xff),	// 55
	COLOR(0x7d, 0x7d, 0xff),	// 56
	COLOR(0x9e, 0x7d, 0xff),	// 57
	COLOR(0xbe, 0x7d, 0xff),	// 58
	COLOR(0xdf, 0x7d, 0xff),	// 59
	COLOR(0xff, 0x7d, 0xff),	// 60
	COLOR(0xff, 0x7d, 0xdf),	// 61
	COLOR(0xff, 0x7d, 0xbe),	// 62
	COLOR(0xff, 0x7d, 0x9e),	// 63
	COLOR(0xff, 0x7d, 0x7d),	// 64
	COLOR(0xff, 0x9e, 0x7d),	// 65
	COLOR(0xff, 0xbe, 0x7d),	// 66
	COLOR(0xff, 0xdf, 0x7d),	// 67
	COLOR(0xff, 0xff, 0x7d),	// 68
	COLOR(0xdf, 0xff, 0x7d),	// 69
	COLOR(0xbe, 0xff, 0x7d),	// 70
	COLOR(0x9e, 0xff, 0x7d),	// 71
	COLOR(0x7d, 0xff, 0x7d),	// 72
	COLOR(0x7d, 0xff, 0x9e),	// 73
	COLOR(0x7d, 0xff, 0xbe),	// 74
	COLOR(0x7d, 0xff, 0xdf),	// 75
	COLOR(0x7d, 0xff, 0xff),	// 76
	COLOR(0x7d, 0xdf, 0xff),	// 77
	COLOR(0x7d, 0xbe, 0xff),	// 78
	COLOR(0x7d, 0x9e, 0xff),	// 79
	COLOR(0xb6, 0xb6, 0xff),	// 80
	COLOR(0xc7, 0xb6, 0xff),	// 81
	COLOR(0xdb, 0xb6, 0xff),	// 82
	COLOR(0xeb, 0xb6, 0xff),	// 83
	COLOR(0xff, 0xb6, 0xff),	// 84
	COLOR(0xff, 0xb6, 0xeb),	// 85
	COLOR(0xff, 0xb6, 0xdb),	// 86
	COLOR(0xff, 0xb6, 0xc7),	// 87
	COLOR(0xff, 0xb6, 0xb6),	// 88
	COLOR(0xff, 0xc7, 0xb6),	// 89
	COLOR(0xff, 0xdb, 0xb6),	// 90
	COLOR(0xff, 0xeb, 0xb6),	// 91
	COLOR(0xff, 0xff, 0xb6),	// 92
	COLOR(0xeb, 0xff, 0xb6),	// 93
	COLOR(0xdb, 0xff, 0xb6),	// 94
	COLOR(0xc7, 0xff, 0xb6),	// 95
	COLOR(0xb6, 0xff, 0xb6),	// 96
	COLOR(0xb6, 0xff, 0xc7),	// 97
	COLOR(0xb6, 0xff, 0xdb),	// 98
	COLOR(0xb6, 0xff, 0xeb),	// 99
	COLOR(0xb6, 0xff, 0xff),	// 100
	COLOR(0xb6, 0xeb, 0xff),	// 101
	COLOR(0xb6, 0xdb, 0xff),	// 102
	COLOR(0xb6, 0xc7, 0xff),	// 103
	COLOR(0x00, 0x00, 0x71),	// 104
	COLOR(0x1c, 0x00, 0x71),	// 105
	COLOR(0x38, 0x00, 0x71),	// 106
	COLOR(0x55, 0x00, 0x71),	// 107
	COLOR(0x71, 0x00, 0x71),	// 108
	COLOR(0x71, 0x00, 0x55),	// 109
	COLOR(0x71, 0x00, 0x38),	// 110
	COLOR(0x71, 0x00, 0x1c),	// 111
	COLOR(0x71, 0x00, 0x00),	// 112
	COLOR(0x71, 0x1c, 0x00),	// 113
	COLOR(0x71, 0x38, 0x00),	// 114
	COLOR(0x71, 0x55, 0x00),	// 115
	COLOR(0x71, 0x71, 0x00),	// 116
	COLOR(0x55, 0x71, 0x00),	// 117
	COLOR(0x38, 0x71, 0x00),	// 118
	COLOR(0x1c, 0x71, 0x00),	// 119
	COLOR(0x00, 0x71, 0x00),	// 120
	COLOR(0x00, 0x71, 0x1c),	// 121
	COLOR(0x00, 0x71, 0x38),	// 122
	COLOR(0x00, 0x71, 0x55),	// 123
	COLOR(0x00, 0x71, 0x71),	// 124
	COLOR(0x00, 0x55, 0x71),	// 125
	COLOR(0x00, 0x38, 0x71),	// 126
	COLOR(0x00, 0x1c, 0x71),	// 127
	COLOR(0x38, 0x38, 0x71),	// 128
	COLOR(0x45, 0x38, 0x71),	// 129
	COLOR(0x55, 0x38, 0x71),	// 130
	COLOR(0x61, 0x38, 0x71),	// 131
	COLOR(0x71, 0x38, 0x71),	// 132
	COLOR(0x71, 0x38, 0x61),	// 133
	COLOR(0x71, 0x38, 0x55),	// 134
	COLOR(0x71, 0x38, 0x45),	// 135
	COLOR(0x71, 0x38, 0x38),	// 136
	COLOR(0x71, 0x45, 0x38),	// 137
	COLOR(0x71, 0x55, 0x38),	// 138
	COLOR(0x71, 0x61, 0x38),	// 139
	COLOR(0x71, 0x71, 0x38),	// 140
	COLOR(0x61, 0x71, 0x38),	// 141
	COLOR(0x55, 0x71, 0x38),	// 142
	COLOR(0x45, 0x71, 0x38),	// 143
	COLOR(0x38, 0x71, 0x38),	// 144
	COLOR(0x38, 0x71, 0x45),	// 145
	COLOR(0x38, 0x71, 0x55),	// 146
	COLOR(0x38, 0x71, 0x61),	// 147
	COLOR(0x38, 0x71, 0x71),	// 148
	COLOR(0x38, 0x61, 0x71),	// 149
	COLOR(0x38, 0x55, 0x71),	// 150
	COLOR(0x38, 0x45, 0x71),	// 151
	COLOR(0x51, 0x51, 0x71),	// 152
	COLOR(0x59, 0x51, 0x71),	// 153
	COLOR(0x61, 0x51, 0x71),	// 154
	COLOR(0x69, 0x51, 0x71),	// 155
	COLOR(0x71, 0x51, 0x71),	// 156
	COLOR(0x71, 0x51, 0x69),	// 157
	COLOR(0x71, 0x51, 0x61),	// 158
	COLOR(0x71, 0x51, 0x59),	// 159
	COLOR(0x71, 0x51, 0x51),	// 160
	COLOR(0x71, 0x59, 0x51),	// 161
	COLOR(0x71, 0x61, 0x51),	// 162
	COLOR(0x71, 0x69, 0x51),	// 163
	COLOR(0x71, 0x71, 0x51),	// 164
	COLOR(0x69, 0x71, 0x51),	// 165
	COLOR(0x61, 0x71, 0x51),	// 166
	COLOR(0x59, 0x71, 0x51),	// 167
	COLOR(0x51, 0x71, 0x51),	// 168
	COLOR(0x51, 0x71, 0x59),	// 169
	COLOR(0x51, 0x71, 0x61),	// 170
	COLOR(0x51, 0x71, 0x69),	// 171
	COLOR(0x51, 0x71, 0x71),	// 172
	COLOR(0x51, 0x69, 0x71),	// 173
	COLOR(0x51, 0x61, 0x71),	// 174
	COLOR(0x51, 0x59, 0x71),	// 175
	COLOR(0x00, 0x00, 0x41),	// 176
	COLOR(0x10, 0x00, 0x41),	// 177
	COLOR(0x20, 0x00, 0x41),	// 178
	COLOR(0x30, 0x00, 0x41),	// 179
	COLOR(0x41, 0x00, 0x41),	// 180
	COLOR(0x41, 0x00, 0x30),	// 181
	COLOR(0x41, 0x00, 0x20),	// 182
	COLOR(0x41, 0x00, 0x10),	// 183
	COLOR(0x41, 0x00, 0x00),	// 184
	COLOR(0x41, 0x10, 0x00),	// 185
	COLOR(0x41, 0x20, 0x00),	// 186
	COLOR(0x41, 0x30, 0x00),	// 187
	COLOR(0x41, 0x41, 0x00),	// 188
	COLOR(0x30, 0x41, 0x00),	// 189
	COLOR(0x20, 0x41, 0x00),	// 190
	COLOR(0x10, 0x41, 0x00),	// 191
	COLOR(0x00, 0x41, 0x00),	// 192
	COLOR(0x00, 0x41, 0x10),	// 193
	COLOR(0x00, 0x41, 0x20),	// 194
	COLOR(0x00, 0x41, 0x30),	// 195
	COLOR(0x00, 0x41, 0x41),	// 196
	COLOR(0x00, 0x30, 0x41),	// 197
	COLOR(0x00, 0x20, 0x41),	// 198
	COLOR(0x00, 0x10, 0x41),	// 199
	COLOR(0x20, 0x20, 0x41),	// 200 
	COLOR(0x28, 0x20, 0x41),	// 201 
	COLOR(0x30, 0x20, 0x41),	// 202 
	COLOR(0x38, 0x20, 0x41),	// 203 
	COLOR(0x41, 0x20, 0x41),	// 204 
	COLOR(0x41, 0x20, 0x38),	// 205 
	COLOR(0x41, 0x20, 0x30),	// 206 
	COLOR(0x41, 0x20, 0x28),	// 207 
	COLOR(0x41, 0x20, 0x20),	// 208 
	COLOR(0x41, 0x28, 0x20),	// 209 
	COLOR(0x41, 0x30, 0x20),	// 210
	COLOR(0x41, 0x38, 0x20),	// 211
	COLOR(0x41, 0x41, 0x20),	// 212
	COLOR(0x38, 0x41, 0x20),	// 213
	COLOR(0x30, 0x41, 0x20),	// 214
	COLOR(0x28, 0x41, 0x20),	// 215
	COLOR(0x20, 0x41, 0x20),	// 216
	COLOR(0x20, 0x41, 0x28),	// 217
	COLOR(0x20, 0x41, 0x30),	// 218
	COLOR(0x20, 0x41, 0x38),	// 219
	COLOR(0x20, 0x41, 0x41),	// 220
	COLOR(0x20, 0x38, 0x41),	// 221
	COLOR(0x20, 0x30, 0x41),	// 222
	COLOR(0x20, 0x28, 0x41),	// 223
	COLOR(0x2c, 0x2c, 0x41),	// 224
	COLOR(0x30, 0x2c, 0x41),	// 225
	COLOR(0x34, 0x2c, 0x41),	// 226
	COLOR(0x3c, 0x2c, 0x41),	// 227
	COLOR(0x41, 0x2c, 0x41),	// 228
	COLOR(0x41, 0x2c, 0x3c),	// 229
	COLOR(0x41, 0x2c, 0x34),	// 230
	COLOR(0x41, 0x2c, 0x30),	// 231
	COLOR(0x41, 0x2c, 0x2c),	// 232
	COLOR(0x41, 0x30, 0x2c),	// 233
	COLOR(0x41, 0x34, 0x2c),	// 234
	COLOR(0x41, 0x3c, 0x2c),	// 235
	COLOR(0x41, 0x41, 0x2c),	// 236
	COLOR(0x3c, 0x41, 0x2c),	// 237
	COLOR(0x34, 0x41, 0x2c),	// 238
	COLOR(0x30, 0x41, 0x2c),	// 239
	COLOR(0x2c, 0x41, 0x2c),	// 240
	COLOR(0x2c, 0x41, 0x30),	// 241
	COLOR(0x2c, 0x41, 0x34),	// 242
	COLOR(0x2c, 0x41, 0x3c),	// 243
	COLOR(0x2c, 0x41, 0x41),	// 244
	COLOR(0x2c, 0x3c, 0x41),	// 245
	COLOR(0x2c, 0x34, 0x41),	// 246
	COLOR(0x2c, 0x30, 0x41),	// 247
	COLOR(0x00, 0x00, 0x00),	// 248
	COLOR(0x00, 0x00, 0x00),	// 249
	COLOR(0x00, 0x00, 0x00),	// 250
	COLOR(0x00, 0x00, 0x00),	// 251
	COLOR(0x00, 0x00, 0x00),	// 252
	COLOR(0x00, 0x00, 0x00),	// 253
	COLOR(0x00, 0x00, 0x00),	// 254
	COLOR(0x00, 0x00, 0x00),	// 255
};

// videomode template
typedef struct {
	u8	vclass;		// videmode class PC_VMODECLASS_*
	u8	cols;		// number of text columns
	u8 	rows;		// number of text rows
	u8	pages;		// number of pages (must be power of 2, max. 8)
	u8	blink;		// blinking
	u8	segm;		// base segment, high byte (0 = debug screen segment 0x00B0)
	u8	plane;		// order of plane/page size in bytes ... PC_VMEM_*
	u8	mask;		// order of mask of offset in video-RAM + 1 ... PC_VMEM_* (must contain all valid pages of first plane)
	u8	wb;		// width of line in bytes / 2
	u8	mem;		// order of total required memory ... PC_VMEM_*
	u8	fonth;		// font height (8, 14 or 16)
	u16	resx;		// resolution X
	u16	resy;		// resolution Y
	u16*	pal;		// default palettes
} sPC_VmodeTemp;

const sPC_VmodeTemp PC_VmodeTemp[PC_VMODE_NUM] = {
	//-------vclass------cols---rows---pages---blink---segm-----plane-----------mask-------wb--------mem-----font--resx--resy-----pal----

	// 0 (CGA, EGA, VGA) text 40x25, 16 colors, resolution 320x200 (8 * 2 KB = 16 KB)
	{   PC_VCLASS_TEXT,   40,   25,    8,     False,  0xB8,  PC_VMEM_2K,   PC_VMEM_16K,  80/2,  PC_VMEM_16K,  8,  320,  200,  PC_CurPal16 },

	// 1 (CGA, EGA, VGA) text 40x25, 16 colors, resolution 320x200 (8 * 2 KB = 16 KB)
	{   PC_VCLASS_TEXT,   40,   25,    8,     True,   0xB8,  PC_VMEM_2K,   PC_VMEM_16K,  80/2,  PC_VMEM_16K,  8,  320,  200,  PC_CurPal16 },

	// 2 (CGA, EGA, VGA) text 80x25, 16 colors, resolution 640x200 (4 * 4 KB = 16 KB)
	{   PC_VCLASS_TEXT,   80,   25,    4,     False,  0xB8,  PC_VMEM_4K,   PC_VMEM_16K, 160/2,  PC_VMEM_16K,  8,  640,  200,  PC_CurPal16 },

	// 3 (CGA, EGA, VGA) text 80x25, 16 colors, resolution 640x200 (4 * 4 KB = 16 KB)
	{   PC_VCLASS_TEXT,   80,   25,    4,     True,   0xB8,  PC_VMEM_4K,   PC_VMEM_16K, 160/2,  PC_VMEM_16K,  8,  640,  200,  PC_CurPal16 },

	// 4 (CGA, EGA, VGA) graphics 320x200, 4 colors (16 KB)
	{   PC_VCLASS_CGA4,   40,   25,    1,     False,  0xB8,  PC_VMEM_16K,  PC_VMEM_8K,   80/2,  PC_VMEM_16K,  8,  320,  200,  PC_CurPal4 },

	// 5 (CGA, EGA, VGA) graphics 320x200, 4 colors (16 KB)
	{   PC_VCLASS_CGA4,   40,   25,    1,     False,  0xB8,  PC_VMEM_16K,  PC_VMEM_8K,   80/2,  PC_VMEM_16K,  8,  320,  200,  PC_CurPal4 },

	// 6 (CGA, EGA, VGA) graphics 640x200, 2 colors (16 KB)
	{   PC_VCLASS_CGA2,   80,   25,    1,     False,  0xB8,  PC_VMEM_16K,  PC_VMEM_8K,   80/2,  PC_VMEM_16K,  8,  640,  200,  PC_CurPal2 },

	// 7 (MDA, EGA, VGA) text 80x25, 2 colors, resolution 640x200 (4 KB) ... resolution refers to the emulator, not the real graphics card 
	{   PC_VCLASS_TEXT,   80,   25,    1,     True,   0xB0,  PC_VMEM_4K,   PC_VMEM_4K,  160/2,  PC_VMEM_4K,   8,  640,  200,  PC_CurPal16 },

	// 8 (PCjr, Tandy) graphics 160x200, 16 colors (2 * 16 KB = 32 KB)
	{   PC_VCLASS_CGA16,  20,   25,    2,     False,  0xB8,  PC_VMEM_16K,  PC_VMEM_8K,   80/2,  PC_VMEM_32K,  8,  160,  200,  PC_CurPal16 },

	// 9 (PCjr, Tandy) graphics 320x200, 16 colors (32 KB)
	{   PC_VCLASS_CGA16,  40,   25,    1,     False,  0xB8,  PC_VMEM_32K,  PC_VMEM_8K,  160/2,  PC_VMEM_32K,  8,  320,  200,  PC_CurPal16 },

	// 10 (PCjr, Tandy) graphics 640x200, 4 colors (32 KB)
	{   PC_VCLASS_CGA4,   80,   25,    1,     False,  0xB8,  PC_VMEM_32K,  PC_VMEM_8K,  160/2,  PC_VMEM_32K,  8,  640,  200,  PC_CurPal4 },

	// 11 (Tandy) graphics 640x200, 16 colors (64 KB)
	{   PC_VCLASS_CGA16,  80,   25,    1,     False,  0xB0,  PC_VMEM_64K,  PC_VMEM_8K,  320/2,  PC_VMEM_64K,  8,  640,  200,  PC_CurPal16 },

	// 12 (Emulator) text 20x15, 16 colors, resolution 160x120 (1 KB)
	{   PC_VCLASS_TEXT,   20,   15,    1,     False,  0x00,  PC_VMEM_1K,   PC_VMEM_1K,   40/2,  PC_VMEM_1K,   8,  160,  120,  PC_CurPal16 },

	// 13 (EGA, VGA) graphics 320x200, 16 colors .. EGA128 4 pages, EGA64 2 pages
	//		(1 plane 1 page: 8 KB, 1 plane 4 pages: 32 KB, 4 planes 4 pages: 128 KB)
	{   PC_VCLASS_EGA16,  40,   25,    4,     False,  0xA0,  PC_VMEM_8K,   PC_VMEM_32K,  40/2,  PC_VMEM_128K, 8,  320,  200,  PC_CurPal16 },

	// 14 (EGA, VGA) graphics 640x200, 16 colors .. EGA128 2 pages, EGA64 1 pages
	//		(1 plane 1 page: 16 KB, 1 plane 2 pages: 32 KB, 4 planes 2 pages: 128 KB)
	{   PC_VCLASS_EGA16,  80,   25,    2,     False,  0xA0,  PC_VMEM_16K,  PC_VMEM_32K,  80/2,  PC_VMEM_128K, 8,  640,  200,  PC_CurPal16 },

	// 15 (EGA, VGA) graphics 640x350, 2 colors (font 14 lines)
	//		(1 plane 1 page: 32 KB, 1 plane 2 pages: 64 KB)
	{   PC_VCLASS_EGA2,   80,   25,    2,     False,  0xA0,  PC_VMEM_32K,  PC_VMEM_64K,  80/2,  PC_VMEM_64K,  14, 640,  350,  PC_CurPal2 },

	// 16 (EGA, VGA) graphics 640x350, 16 colors (font 14 lines) ... or 4 colors on EGA64 (planes 0 and 2)
	//		(1 plane 1 page: 32 KB, 2 plane 1 page: 64 KB, 4 plane 1 page: 128 KB)
	{   PC_VCLASS_EGA16,  80,   25,    1,     False,  0xA0,  PC_VMEM_32K,  PC_VMEM_32K,  80/2,  PC_VMEM_128K, 14, 640,  350,  PC_CurPal16 },

	// 17 (VGA) graphics 640x480, 2 colors (font 16 lines) (real required memory 40 KB)
	{   PC_VCLASS_EGA2,   80,   25,    1,     False,  0xA0,  PC_VMEM_64K,  PC_VMEM_64K,  80/2,  PC_VMEM_64K,  16, 640,  480,  PC_CurPal2 },

	// 18 (VGA) graphics 640x480, 16 colors (font 16 lines) (real required memory 160 KB)
	{   PC_VCLASS_EGA16,  80,   25,    1,     False,  0xA0,  PC_VMEM_64K,  PC_VMEM_64K,  80/2,  PC_VMEM_256K, 16, 640,  480,  PC_CurPal16 },

	// 19 (VGA) graphics 320x200, 256 colors
	{   PC_VCLASS_VGA,    40,   25,    1,     False,  0xA0,  PC_VMEM_64K,  PC_VMEM_64K, 320/2,  PC_VMEM_64K,  8,  320,  200,  PC_CurPal256 },
};

// set videomode PC_VMODE_* (set bit 7 to not clear screen; returns False if videmode cannot be set)
//  "card" entry must be set
// Debug videomode 12 does not deallocate current video memory.
Bool PC_SetVmode(int vmode)
{
	// check videomode number
	Bool cls = (vmode & B7) == 0;	// "clear screen" flag
	vmode &= 0x7f;			// reset "clear screen" flag
	if (vmode >= PC_VMODE_NUM) return False; // invalid videomode

	// prepare default palettes 2 and 4 colors
	memcpy(PC_CurPal2, PC_Pal2, sizeof(PC_Pal2));
//	memcpy(PC_CurPal4, PC_Pal4_0_low, sizeof(PC_Pal4_0_low));
	memcpy(PC_CurPal4, PC_Pal4_1_high, sizeof(PC_Pal4_1_high));
	memcpy(PC_CurPal16, (vmode == 7) ? PC_PalMDAWhite : PC_Pal16, sizeof(PC_Pal16));
	memcpy(PC_CurPal256, PC_Pal256, sizeof(PC_Pal256));

	// pointer to current videomode descriptor
	sPC_Vmode* m = &PC_Vmode;

	// get card
	u8 card = m->card;
	if (card >= PC_CARD_NUM) return False; // invalid graphics card

	// check if this card supports required videomode (emulator videomode is always supported)
	if ((vmode != PC_VMODE_12) && (vmode > (int)PC_CardMaxMode[card])) return False; // invalid videomode

	// get card memory
	int cardmem = PC_CardMem[card];
	if (cardmem == PC_VMEM_256K)
		cardmem = PC_VMEM_256K_REAL;
	else
		cardmem = BIT(cardmem);

	// videomode template
	const sPC_VmodeTemp* t = &PC_VmodeTemp[vmode];
	
	// get required memory
	int reqmem = t->mem;
	if (reqmem == PC_VMEM_256K)
		reqmem = PC_VMEM_256K_REAL;
	else
		reqmem = BIT(reqmem);
	if (vmode == PC_VMODE_17) reqmem = PC_PLANE18_SIZE; // only 40 KB is sufficient

	// get number of pages
	u8 pages = t->pages;	// number of pages

	// MDA simulates CGA
	u16* pal = t->pal;
	if (card == PC_CARD_MDA)
	{
		if (t->vclass == PC_VCLASS_TEXT) // text videomode
		{
			memcpy(PC_CurPal16, PC_PalMDAPlasma, sizeof(PC_Pal16));
			if (vmode != PC_VMODE_12)
			{
				pages = (vmode <= 1) ? 2 : 1;
				reqmem = BIT(PC_VMEM_4K);
			}
		}
	}

	// special case - EGA64 on 128KB videomode
	if (((card == PC_CARD_EGA64) || (card == PC_CARD_MCGA)) && (reqmem == BIT(PC_VMEM_128K)))
	{
		if (vmode == PC_VMODE_16) memcpy(PC_CurPal16, PC_Pal16Ega64, sizeof(PC_Pal16));
		if (pages > 1) pages >>= 1; // half pages (or half number of planes)
		reqmem = BIT(PC_VMEM_64K); // half required memory
	}

	// check required memory
	if (reqmem > cardmem) return False; // not enough video memory

	// page of base segment (0 = debug segment)
	u8 basepage = t->segm; // >> (PC_PAGE_SHIFT - 8 - 4);
	if (basepage != 0) // 0 = debug screen (need no allocation, left video memory allocated)
	{
		// try to allocate memory pages
		int pagenum = (reqmem + PC_PAGE_SIZE-1) >> PC_PAGE_SHIFT; // required number of pages
		int freepages = PC_RAM_PAGE_NUM - PC_NextPageRAM; // number of free pages
		if (pagenum > freepages)
		{
			// memory error
			PC_Fatal(PC_ERROR_VRAM, 0, 0);
			return False;
		}

		// deallocate old video memory - memory range 0xA0000..0xC8000
		int page = PC_VRAM_PAGEMIN;
		while (page < PC_VRAM_PAGEMAX) PC_MemMap[page++] = PC_INV_PAGE;

		// allocate new video memory
		page = PC_RAM_PAGE_NUM - pagenum; // first used page
		PC_VmemPageRAM = page; // bottom video-RAM allocated page
		while (page < PC_RAM_PAGE_NUM)
		{
			PC_MemMap[basepage] = page;
			page++;
			basepage++;
		}
	}

	// videomode is OK, get parameters
	m->vmode = (u8)vmode;	// save new videomode index
	m->vclass = t->vclass;	// videomode class
	m->cols = t->cols;	// number of text columns
	m->rows = t->rows;	// number of text rows
	m->pages = pages;	// number of pages
	m->page = 0;		// current page
	m->pagemask = pages - 1; // mask of page number
	m->blink = t->blink;	// blinking

	u8 fonth = t->fonth;	// font height
	m->fonth = fonth;
	if (fonth == 16)
	{
		m->fontpage = PC_BIOS_PAGE_FONT16;
		m->font = PC_Font16;
	}
	else if (fonth == 14)
	{
		m->fontpage = PC_BIOS_PAGE_FONT14;
		m->font = PC_Font14;
	}
	else // if (fonth == 8)
	{
		m->fontpage = PC_BIOS_PAGE_FONT8;
		m->font = PC_Font8;
	}
	m->vfont = m->fontpage*PC_PAGE_SIZE; // font virtual address

	m->curstart = 6;	// cursor start line
	m->curend = 7;		// cursor end line
	m->segm = (u16)(u8)t->segm << 8; // base segment
	if (m->segm == 0) m->segm = PC_DEBUG_VRAM_SEG; // segment of user debug screen
	m->resx = t->resx;	// resolution X
	m->resy = t->resy;	// resolution Y
	m->cardmem = cardmem;	// card video memory
	m->usedmem = reqmem;	// used video memory
	m->planesize = BIT(t->plane); // plane/page size
	if ((vmode == PC_VMODE_18) || (vmode == PC_VMODE_17)) m->planesize = PC_PLANE18_SIZE; // size of plane at videomode 18 and 17
	m->off = 0;		// offset of current page in video-RAM
	m->mask = BIT(t->mask) - 1; // mask of offset
	m->wb = t->wb * 2;	// width of line
	//m->vpal = 0;		// virtual address of palettes (0=default)
	m->pal = pal;		// palettes
	m->base = (t->segm == 0) ? PC_DEBUG_VRAM : &PC_RAM_BASE[PC_VmemPageRAM * PC_PAGE_SIZE]; // pointer to video memory

	int i;
	for (i = 0; i < PC_VMODE_PAGE_MAX; i++)
	{
		m->cur[i].curx = 0;
		m->cur[i].cury = 0;
	}		

	// --- update BIOS variables

	// 0040:0049 (1) current video mode (0 = text 40x25, 16 colors)
	PC_RAM_BASE[0x0449] = (u8)vmode; // current videomode

	// 0040:004A (2) number of columns on screen
	*(u16*)&PC_RAM_BASE[0x044A] = m->cols; // number of columns

	// 0040:004C (2) page size in bytes, aligned
	*(u16*)&PC_RAM_BASE[0x044C] = m->planesize; // page size

	// 0040:004E (2) page start address
	*(u16*)&PC_RAM_BASE[0x044E] = 0; // page start address

	// 0040:0050 (16) cursor positions in 8 pages (column and row)
	memset(&PC_RAM_BASE[0x0450], 0, 16);

	// 0040:0060 (2) cursor mode setting (end scan line / start scan line)
	PC_RAM_BASE[0x0460] = 7; // cursor end scanline
	PC_RAM_BASE[0x0461] = 6; // cursor start scanline

	// 0040:0062 (1) current page number
	PC_RAM_BASE[0x0462] = 0; // current page number

	// 0040:0063 (2) CRT controller base I/O port address (03B4h MDA, 03D4h color)
	*(u16*)&PC_RAM_BASE[0x0463] = (vmode == PC_VMODE_7) ? 0x03B4 : 0x03D4; // CRT controller port address

	// 0040:0065 (1) current CRT mode selector register (last value on 03D8h)
	//	B5: bit 7 blinks
	//	B4: mode 6 monochrome
	//	B3: video signal enabled
	//	B2: monochrome
	//	B1: graphics
	//	B0: 80x25 text
	u8 r = B3;
	if (m->cols > 60) r |= B0;
	if (m->vclass != PC_VCLASS_TEXT) r |= B1; // graphics mode
	if ((m->vclass == PC_VCLASS_CGA2) || (m->vclass == PC_VCLASS_EGA2)) r |= B4; // mono
	if (vmode == PC_VMODE_7) r |= B2;
	if (m->blink) r |= B5;
	PC_RAM_BASE[0x0465] = r;

	// 0040:0066 (1) current CGA palette register (last value on 03D9h)
	//	B5: palette 0 (red-green-brown) or 1 (cyan-magenta-white)
	//	B4: intense colors in graphics, intense background in text mode
	//	B3: intense border color in 40x25, intense background in 320x200, intense foreground in 640x200
	//	B2: red border in 40x25, red background in 320x200, red foreground in 640x200
	//	B1: green border in 40x25, red background in 320x200, red foreground in 640x200
	//	B0: blue border in 40x25, red background in 320x200, red foreground in 640x200
	PC_RAM_BASE[0x0466] = 0;

	// 0040:0084 (1) last row on screen (= rows - 1)
	PC_RAM_BASE[0x0484] = m->rows - 1; // last row

	// 0040:0085 (2) bytes per character (character height in scan-lines)
	*(u16*)&PC_RAM_BASE[0x0485] = fonth; // character height

	// --- update videocard registers

	m->egasetsel = 0;	// EGA/VGA select setup register 0x3C4
	m->egagrsel = 0;	// EGA/VGA select graphics register 0x3CE
	memset(m->egaset, 0, PC_EGA_SET_MAX+1);
	memset(m->egagr, 0, PC_EGA_GR_MAX+1);
	m->egaset[PC_EGA_SET_WMASK] = 0x0f; // write mask
	m->egagr[PC_EGA_GR_BIT] = 0xff; // bit mask

	// no zoom
	PC_LCDZoom = 0;

	// clear video memory
	if (cls)
	{
		u8* d = m->base;

		// text mode
		if (m->vclass == PC_VCLASS_TEXT)
		{
			int i;
			for (i = reqmem/2; i > 0; i--)
			{
				*d++ = ' ';
				*d++ = PC_DEF_COL;
			}
		}

		// graphics mode
		else
			memset(d, 0, reqmem);
	}

	return True;
}

// scroll window (does not affect cursor position)
//  page ... page
//  shift ... number of rows to scroll (0 = clear entire window, > 0 up, < 0 down)
//  attr ... color attribute of new rows
//  x1 ... left column
//  y1 ... top row
//  x2 ... right column
//  y2 ... bottom row
void PC_Scroll(u8 page, int shift, u8 attr, u8 x1, u8 y1, u8 x2, u8 y2)
{
	// pointer to current videomode descriptor
	sPC_Vmode* m = &PC_Vmode;
	page &= m->pagemask;

	// limit coordinates
	if (x2 >= m->cols) x2 = m->cols - 1;
	if (x1 > x2) x1 = x2;
	if (y2 >= m->rows) y2 = m->rows - 1;
	if (y1 >= y2) y1 = y2;
	int w = x2 - x1 + 1;
	int h = y2 - y1 + 1;

	// adjust number of rows to scroll (0 = clear whole window)
	Bool up = shift >= 0;
	if (shift < 0) shift = -shift;
	if ((shift >= h) || (shift == 0)) shift = h;

	// prepare
	u8* base = m->base;
	int mask = m->mask;
	int off = (page*m->planesize) & mask; // offset of required video page
	int wb = m->wb;
	int strips = 1;
	int stripsize = 8192;
	int rowh = m->fonth;
	u8 attr1, attr2, attr3, attr4;

	// switch by videomode
	switch (m->vmode)
	{

// ---- text

	// (CGA, EGA, VGA) text 40x25
	case PC_VMODE_0:
	case PC_VMODE_1:
	// (CGA, EGA, VGA) text 80x25
	// (MDA) text 80x25
	case PC_VMODE_2:
	case PC_VMODE_3:
	case PC_VMODE_7:
	// (Emulator) text 20x15
	case PC_VMODE_12:
		{
			// get start offset
			int offsrc, offdst, offshift;
			if (up)
			{
				// scroll up
				offdst = (off + x1*2 + y1*wb) & mask;
				offsrc = (offdst + shift*wb) & mask;
				offshift = wb;
			}
			else
			{
				// scroll down
				offdst = (off + x1*2 + y2*wb) & mask;
				offsrc = (offdst - shift*wb) & mask;
				offshift = -wb;
			}

			// scroll
			int i = w * 2;
			int n = h - shift;
			for (; n > 0; n--)
			{
				memcpy(&base[offdst], &base[offsrc], i);
				offdst = (offdst + offshift) & mask;
				offsrc = (offsrc + offshift) & mask;
			}

			// clear new rows
			u8* d;
			for (; shift > 0; shift--)
			{
				d = &base[offdst];
				for (i = w; i > 0; i--)
				{
					*d++ = ' ';
					*d++ = attr;
				}
				offdst = (offdst + offshift) & mask;
			}
		}
		break;

// ---- graphics, 2 strips x 8 KB, 4 colors

	// (CGA, EGA, VGA) graphics 320x200, 4 colors
	case PC_VMODE_4:
	case PC_VMODE_5:
		strips = 2;
		rowh = 4;
		goto GSCROLL5;

// ---- graphics, 2 strips x 8 KB, 2 colors

	// (CGA, EGA, VGA) graphics 640x200, 2 colors
	case PC_VMODE_6:
		strips = 2;
		rowh = 4;
		attr1 = (attr & B0) ? 0xff : 0;
		goto GSCROLL3;

// ---- graphics, 2 strips x 8 KB, 16 colors

	// (PCjr, Tandy) graphics 160x200, 16 colors
	case PC_VMODE_8:
		strips = 2;
		rowh = 4;
		goto GSCROLL4;

// ---- graphics, 4 strips x 8 KB, 16 colors

	// (PCjr, Tandy) graphics 320x200, 16 colors
	case PC_VMODE_9:
		strips = 4;
		rowh = 2;
		goto GSCROLL4;

// ---- graphics, 4 strips x 8 KB, 4 colors

	// (PCjr, Tandy) graphics 640x200, 4 colors
	case PC_VMODE_10:
		strips = 4;
		rowh = 2;
GSCROLL5:	attr &= 0x03;
		attr1 = attr * 0x55; // duplicate colors
		goto GSCROLL3;

// ---- graphics, 8 strips x 8 KB, 16 colors

	// (Tandy) graphics 640x200, 16 colors
	case PC_VMODE_11:
		strips = 8;
		rowh = 1;
GSCROLL4:	attr &= 0x0f;
		attr1 = attr * 0x11; // duplicate colors
GSCROLL3:	attr2 = attr1;
		attr3 = attr1;
		attr4 = attr1;
		goto GSCROLL2;

// ---- graphics, 2 or 4 planes, 4 or 16 colors

	// (EGA, VGA) graphics 640x350, 4 or 16 colors
	case PC_VMODE_16:
		if (PC_Vmode.usedmem < BIT(PC_VMEM_128K)) // EGA card 64 KB
		{
			strips = 2;
			attr1 = (attr & B0) ? 0xff : 0;
			attr2 = (attr & B1) ? 0xff : 0;
			goto GSCROLL;
		}

// ---- graphics, 4 planes, 16 colors

	// (EGA, VGA) graphics 320x200, 16 colors
	case PC_VMODE_13:
	// (EGA, VGA) graphics 640x200, 16 colors
	case PC_VMODE_14:
	// (VGA) graphics 640x480, 16 colors
	case PC_VMODE_18:
		strips = 4;
		attr1 = (attr & B0) ? 0xff : 0;
		attr2 = (attr & B1) ? 0xff : 0;
		attr3 = (attr & B2) ? 0xff : 0;
		attr4 = (attr & B3) ? 0xff : 0;
		goto GSCROLL;

// ---- graphics, 1 plane, 2 colors

	// (EGA, VGA) graphics 640x350, 2 colors
	case PC_VMODE_15:
	// (VGA) graphics 640x480, 2 colors
	case PC_VMODE_17:
		attr1 = (attr & B0) ? 0xff : 0;
		goto GSCROLL;

// ---- graphics, 1 plane, 256 colors

	// (VGA) graphics 320x200, 256 colors
	case PC_VMODE_19:
		attr1 = attr;
GSCROLL:
		stripsize = m->planesize;
GSCROLL2:
		{
			// is stripsize power of 2? ... mask offset only at this strip
			if ((stripsize & (stripsize-1)) == 0) mask = stripsize-1;

			// number of bytes per character
			int chw = wb / m->cols;

			// recalc shift to number of lines
			shift *= rowh;
			h *= rowh;

			// process strips
			int offsrc, offdst, offshift, i, n;
			w *= chw;
			u8* d;
			for (; strips > 0; strips--)
			{
				// get start offset
				if (up)
				{
					// scroll up
					offdst = (off + x1*chw + y1*rowh*wb) & mask;
					offsrc = (offdst + shift*wb) & mask;
					offshift = wb;
				}
				else
				{
					// scroll down
					offdst = (off + x1*chw + (y2*rowh+rowh-1)*wb) & mask;
					offsrc = (offdst - shift*wb) & mask;
					offshift = -wb;
				}

				// scroll
				for (n = h - shift; n > 0; n--)
				{
					memcpy(&base[offdst], &base[offsrc], w);
					offdst = (offdst + offshift) & mask;
					offsrc = (offsrc + offshift) & mask;
				}

				// clear new rows
				for (n = shift; n > 0; n--)
				{
					d = &base[offdst];
					for (i = w; i > 0; i--) *d++ = attr1;
					offdst = (offdst + offshift) & mask;
				}

				// shift base to next strip
				base += stripsize;

				// shift color attributes
				attr1 = attr2;
				attr2 = attr3;
				attr3 = attr4;
			}
		}
		break;

	// unsupported videomode
	default:
		break;
	}
}

// scroll screen up (does not affect cursor position)
//  page ... page
void PC_ScrollUp(u8 page)
{
	// pointer to current videomode descriptor
	sPC_Vmode* m = &PC_Vmode;
	page &= m->pagemask;

	// get color of last row - only in text modes, graphics mode will use background color 0
	u8 col = 0;
	if (m->vclass == PC_VCLASS_TEXT)
	{
		u8 row = m->rows - 1; // last row
		int wb = m->wb; // width of line
		int off = (page*m->planesize + row*wb + 1) & m->mask; // offset in videomemory
		col = m->base[off]; // read color attribute
	}

	// scroll screen
	PC_Scroll(page, 1, col, 0, 0, m->cols - 1, m->rows - 1);
}

// clear screen
void PC_ClearScr(u8 page, u8 attr)
{
	// pointer to current videomode descriptor
	sPC_Vmode* m = &PC_Vmode;
	page &= m->pagemask;

	// scroll window (does not affect cursor position)
	//  page ... page
	//  shift ... number of rows to scroll (0 = clear entire window, > 0 up, < 0 down)
	//  attr ... color attribute of new rows
	//  x1 ... left column
	//  y1 ... top row
	//  x2 ... right column
	//  y2 ... bottom row
	PC_Scroll(page, 0, attr, 0, 0, m->cols - 1, m->rows - 1);

	m->cur[page].curx = 0;
	m->cur[page].cury = 0;
}

/*
// bit duplication
u16 PC_BitDupl(u8 ch)
{
	// 7 6 5 4 3 2 1 0

	// - - - - 7 6 5 4 / - - - - 3 2 1 0
	u16 chw = (((u16)ch << 4) | ch) & 0x0f0f;
	// - - 7 6 - - 5 4 / - - 3 2 - - 1 0
	chw = ((chw << 2) | chw) & 0x3333;
	// - 7 - 6 - 5 - 4 / - 3 - 2 - 1 - 0
	chw = ((chw << 1) | chw) & 0x5555;
	// 7 7 6 6 5 5 4 4 / 3 3 2 2 1 1 0 0
	chw = ((chw << 1) | chw);
	return chw;
}
*/

// bit duplication, reverse byte order
u16 PC_BitDuplRev(u8 ch)
{
	// 7 6 5 4 3 2 1 0

	// 3 2 1 0 - - - - / 7 6 5 4 - - - -
	u16 chw = (((u16)ch << 12) | ch) & 0xf0f0;
	// 3 2 - - 1 0 - - / 7 6 - - 5 4 - -
	chw = ((chw >> 2) | chw) & 0xcccc;
	// 3 - 2 - 1 - 0 - / 7 - 6 - 5 - 4 -
	chw = ((chw >> 1) | chw) & 0xaaaa;
	// 3 3 2 2 1 1 0 0 / 7 7 6 6 5 5 4 4
	chw = ((chw >> 1) | chw);
	return chw;
}

/*
// bit quadrupling
u32 PC_BitQuad(u8 ch)
{
	// 7 6 5 4 3 2 1 0

	// - - - - - - - - / - - - - 7 6 5 4 / - - - - - - - - / - - - - 3 2 1 0
	u32 chd = (((u32)ch << 12) | ch) & 0x000f000f;
	// - - - - - - 7 6 / - - - - - - 5 4 / - - - - - - 3 2 / - - - - - - 1 0
	chd = ((chd << 6) | chd) & 0x03030303;
	// - - - 7 - - - 6 / - - - 5 - - - 4 / - - - 3 - - - 2 / - - - 1 - - - 0
	chd = ((chd << 3) | chd) & 0x11111111;
	// - 7 - 7 - 6 - 6 / - 5 - 5 - 4 - 4 / - 3 - 3 - 2 - 2 / - 1 - 1 - 0 - 0
	chd = ((chd << 2) | chd);
	// 7 7 7 7 6 6 6 6 / 5 5 5 5 4 4 4 4 / 3 3 3 3 2 2 2 2 / 1 1 1 1 0 0 0 0
	chd = ((chd << 1) | chd);
	return chd;
}
*/

// bit quadrupling, reverse byte order
u32 PC_BitQuadRev(u8 ch)
{
	// 7 6 5 4 3 2 1 0
	// - - - - - - - - / 3 2 1 0 - - - - / - - - - - - - - / 7 6 5 4 - - - -
	u32 chd = (((u32)ch << 20) | ch) & 0x00f000f0;
	// 1 0 - - - - - - / 3 2 - - - - - - / 5 4 - - - - - - / 7 6 - - - - - -
	chd = ((chd << 10) | chd) & 0xc0c0c0c0;
	// 1 - - - 0 - - - / 3 - - - 2 - - - / 5 - - - 4 - - - / 7 - - - 6 - - -
	chd = ((chd >> 3) | chd) & 0x88888888;
	// 1 - 1 - 0 - 0 - / 3 - 3 - 2 - 2 - / 5 - 5 - 4 - 4 - / 7 - 7 - 6 - 6 -
	chd = ((chd >> 2) | chd);
	// 1 1 1 1 0 0 0 0 / 3 3 3 3 2 2 2 2 / 5 5 5 5 4 4 4 4 / 7 7 7 7 6 6 6 6
	chd = ((chd >> 1) | chd);
	return chd;
}

// prepare 4-color
u16 PC_Color4(u8 attr)
{
	u16 col = attr & 3;
	col = (col << 8) | col;
	col = (col << 4) | col;
	col = (col << 2) | col;
	return col;
}

// prepare 16-color
u32 PC_Color16(u8 attr)
{
	u32 col = attr & 0x0f;
	col = (col << 16) | col;
	col = (col << 8) | col;
	col = (col << 4) | col;
	return col;
}

// write character to video memory
//  ch ... character to display
//  page ... page (or background color in 256-color mode)
//  attr ... attribute in text mode, or color in graphics (bit 7: XOR with image at mode < 256 colors)
//  num ... number of characters
//  noattr ... True=do not use attributes at text mode
void PC_WriteCh(u8 ch, u8 page, u8 attr, int num, Bool noattr)
{
	// pointer to current videomode descriptor
	sPC_Vmode* m = &PC_Vmode;
	u8 page2 = page & m->pagemask;
	if (m->vmode == PC_VMODE_19) page2 = 0;

	// pointer to font
	int fonth = m->fonth; // font height
	const u8* s = m->font + ch*fonth;

	// cursor
	u8 x = m->cur[page2].curx;
	u8 y = m->cur[page2].cury;
	u8 cols = m->cols; // number of columns
	u8 rows = m->rows; // number of rows
	if ((x >= cols) || (y >= rows)) return;

	// prepare
	u8* base = m->base; // base
	int mask = m->mask; // offset mask
	int off = (page2*m->planesize) & mask; // offset of the page
	int wb = m->wb; // width in bytes
	int stripnum = 1; // number of strips
	int planes = 4; // number of color planes
	u8 attr2 = attr & 0x7f;

	// switch by videomode
	switch (m->vmode)
	{

// ---- text

	// (CGA, EGA, VGA) text 40x25
	case PC_VMODE_0:
	case PC_VMODE_1:
	// (CGA, EGA, VGA) text 80x25
	// (MDA) text 80x25
	case PC_VMODE_2:
	case PC_VMODE_3:
	case PC_VMODE_7:
	// (Emulator) text 20x15
	case PC_VMODE_12:
		{
			// offset (2 bytes per character)
			off = (off + x*2 + y*wb) & mask;

			// write characters (continue to next row)
			for (; num > 0; num--)
			{
				base[off] = ch;
				if (!noattr) base[off+1] = attr;
				off = (off + 2) & mask;
			}
		}
		break;

// ---- graphics, 4 strips x 8 KB, 4 colors

	// (PCjr, Tandy) graphics 640x200, 4 colors
	case PC_VMODE_10:
		stripnum *= 2; // 4 strips
		fonth >>= 1; // font height / 2

// ---- graphics, 2 strips x 8 KB, 4 colors

	// >>> offset must be 16-bit aligned!

	// (CGA, EGA, VGA) graphics 320x200, 4 colors
	case PC_VMODE_4:
	case PC_VMODE_5:
		{
			stripnum *= 2; // number of strips
			stripnum--; // last strip
			fonth >>= 1; // font height / 2

			// offset (2 bytes per character, 'fonth' lines per character in every strip)
			off = (off + x*2 + y*wb*fonth) & mask;

			// prepare colors
			u16 colf = PC_Color4(attr);
			u16 colb = PC_Color4(attr >> 4);

			// loop through lines in strips
			int line;
			int off0 = off;
			for (line = 0; line < 8; line++)
			{
				// return offset
				off = off0;

				// load next character font mask
				ch = *s++;

				// font bit duplication
				u16 chw = PC_BitDuplRev(ch);

				// add colors
				chw = (colf & chw) | (colb & ~chw);

				// write font
				int k;
				u8 x2 = x;
				for (k = num; k > 0; k--)
				{
					if (attr >= 0x80)
					{
						// XOR
						*(u16*)&base[off] ^= chw;
					}
					else
					{
						// normal write
						*(u16*)&base[off] = chw;
					}

					off = (off + 2) & mask;
					x2++;
					if (x2 >= cols) break;
				}

				// shift strip
				if ((line & stripnum) == stripnum)
				{
					// shift offset
					off0 = (off0 + wb) & mask;

					// shift base
					base -= stripnum*0x2000;
				}
				else
				{
					// shift base
					base += 0x2000;
				}
			}
		}
		break;

// ---- graphics, 2 strips x 8 KB, 2 colors

	// (CGA, EGA, VGA) graphics 640x200, 2 colors
	case PC_VMODE_6:
		{
			stripnum *= 2; // number of strips
			stripnum--; // last strip
			fonth >>= 1; // font height / 2

			// offset (1 byte per character, 'fonth' lines per character in every strip)
			off = (off + x + y*wb*fonth) & mask;

			// loop through lines in strips
			int line;
			int off0 = off;
			for (line = 0; line < 8; line++)
			{
				// return offset
				off = off0;

				// load next character font mask
				ch = *s++;

				// write font
				int k;
				u8 x2 = x;
				for (k = num; k > 0; k--)
				{
					if (attr >= 0x80)
					{
						// XOR
						base[off] ^= ch;
					}
					else
					{
						// normal write
						base[off] = ch;
					}

					off = (off + 1) & mask;
					x2++;
					if (x2 >= cols) break;
				}

				// shift strip
				if ((line & stripnum) == stripnum)
				{
					// shift offset
					off0 = (off0 + wb) & mask;

					// shift base
					base -= stripnum*0x2000;
				}
				else
				{
					// shift base
					base += 0x2000;
				}
			}
		}
		break;

// ---- graphics, 8 strips x 8 KB, 16 colors

	// (Tandy) graphics 640x200, 16 colors
	case PC_VMODE_11:
		stripnum *= 2; // 8 strips
		fonth >>= 1; // font height / 4

// ---- graphics, 4 strips x 8 KB, 16 colors

	// (PCjr, Tandy) graphics 320x200, 16 colors
	case PC_VMODE_9:
		stripnum *= 2; // 4 strips
		fonth >>= 1; // font height / 2

// ---- graphics, 2 strips x 8 KB, 16 colors

	// (PCjr, Tandy) graphics 160x200, 16 colors
	case PC_VMODE_8:
		{

	// >>> offset must be 32-bit aligned!

			stripnum *= 2; // number of strips
			stripnum--; // last strip
			fonth >>= 1; // font height / 2

			// offset (4 bytes per character, 'fonth' lines per character in every strip)
			off = (off + x*4 + y*wb*fonth) & mask;

			// prepare colors
			u32 colf = PC_Color16(attr2);
			u32 colb = PC_Color16(attr2 >> 4);

			// loop through lines in strips
			int line;
			int off0 = off;
			for (line = 0; line < 8; line++)
			{
				// return offset
				off = off0;

				// load next character font mask
				ch = *s++;

				// font bit quadrupling
				u32 chd = PC_BitQuadRev(ch);

				// add colors
				chd = (colf & chd) | (colb & ~chd);

				// write font
				int k;
				u8 x2 = x;
				for (k = num; k > 0; k--)
				{
					if (attr >= 0x80)
					{
						// XOR
						*(u32*)&base[off] ^= chd;
					}
					else
					{
						// normal write
						*(u32*)&base[off] = chd;
					}

					off = (off + 4) & mask;
					x2++;
					if (x2 >= cols) break;
				}

				// shift strip
				if ((line & stripnum) == stripnum)
				{
					// shift offset
					off0 = (off0 + wb) & mask;

					// shift base
					base -= stripnum*0x2000;
				}
				else
				{
					// shift base
					base += 0x2000;
				}
			}
		}
		break;

// ---- graphics, 2 or 4 planes, 4 or 16 colors

	// (EGA, VGA) graphics 640x350, 4 or 16 colors
	case PC_VMODE_16:
		if (PC_Vmode.usedmem < BIT(PC_VMEM_128K)) // EGA card 64 KB
		{
			planes = 2; // number of color planes
			attr2 = (attr2 & 0x11) | ((attr2 & 0x44) >> 1);
		}

// ---- graphics, 4 planes, 16 colors

	// (EGA, VGA) graphics 320x200, 16 colors
	case PC_VMODE_13:
	// (EGA, VGA) graphics 640x200, 16 colors
	case PC_VMODE_14:
	// (VGA) graphics 640x480, 16 colors
	case PC_VMODE_18:
		{
			// loop planes
			for (; planes > 0; planes--)
			{
				// offset (1 byte per character, 'fonth' lines per character)
				int off0 = (m->off + x + y*wb*fonth) & mask;

				// character font
				const u8* s2 = s;

				// loop through lines
				int line;
				for (line = fonth; line > 0; line--)
				{
					// return offset
					off = off0;

					// load next character font mask
					ch = *s2++;
					u8 col = 0;
					if ((attr2 & B0) != 0) col = ch;
					if ((attr2 & B4) != 0) col |= ~ch;

					// write font
					int k;
					u8 x2 = x;
					for (k = num; k > 0; k--)
					{
						if (attr >= 0x80)
						{
							// XOR
							base[off] ^= col;
						}
						else
						{
							// normal write
							base[off] = col;
						}

						off = (off + 1) & mask;
						x2++;
						if (x2 >= cols) break;
					}

					// shift offset
					off0 = (off0 + wb) & mask;
				}

				// shift attribute
				attr2 >>= 1;

				// shift base
				base += m->planesize;
			}
		}
		break;

// ---- graphics, 1 plane, 2 colors

	// (EGA, VGA) graphics 640x350, 2 colors
	case PC_VMODE_15:
	// (VGA) graphics 640x480, 2 colors
	case PC_VMODE_17:
		{
			// offset (1 byte per character, 'fonth' lines per character)
			off = (off + x + y*wb*fonth) & mask;

			// loop through lines
			int line;
			int off0 = off;
			for (line = fonth; line > 0; line--)
			{
				// return offset
				off = off0;

				// load next character font mask
				ch = *s++;

				// write font
				int k;
				u8 x2 = x;
				for (k = num; k > 0; k--)
				{
					if (attr >= 0x80)
					{
						// XOR
						base[off] ^= ch;
					}
					else
					{
						// normal write
						base[off] = ch;
					}

					off = (off + 1) & mask;
					x2++;
					if (x2 >= cols) break;
				}

				// shift offset
				off0 = (off0 + wb) & mask;
			}
		}
		break;

// ---- graphics, 1 plane, 256 colors

	// (VGA) graphics 320x200, 256 colors
	case PC_VMODE_19:
		{
			// offset (8 bytes per character, 'fonth' lines per character)
			off = (off + x*8 + y*wb*fonth) & mask;

			// loop through lines in strips
			int line;
			int off0 = off;
			for (line = fonth; line > 0; line--)
			{
				// return offset
				off = off0;

				// load next character font mask
				ch = *s++;

				// write font
				int k, bits;
				u8 x2 = x;
				for (k = num; k > 0; k--)
				{
					u8 ch2 = ch;
					for (bits = 8; bits > 0; bits--)
					{
						base[off] = ((ch2 & B7) == 0) ? page : attr;
						ch2 <<= 1;
						off++;
					}
					off &= mask;
					x2++;
					if (x2 >= cols) break;
				}

				// shift offset
				off0 = (off0 + wb) & mask;
			}
		}
		break;

	// unsupported videomode
	default:
		break;
	}
}

// write character in TTY mode
//  ch ... character
//  page ... page (or background color in 256-color mode)
//  attr ... color (only in graphics mode; bit 7: XOR with image at mode < 256 colors)
//  noattr ... True=do not use attributes at text mode
void PC_WriteTty(u8 ch, u8 page, u8 attr, Bool noattr)
{
	// pointer to current videomode descriptor
	sPC_Vmode* m = &PC_Vmode;
	u8 page2 = page & m->pagemask;
	if (m->vmode == PC_VMODE_19) page2 = 0;

	// control characters
	if (ch == CH_BS) // 0x08 back space
	{
		if (m->cur[page2].curx > 0)
		{
			m->cur[page2].curx--;
			PC_WriteCh(' ', page, PC_DEF_COL, 1, noattr);
		}
	}
	else if (ch == CH_CR) // 0x0D CR
	{
		m->cur[page2].curx = 0;
	}
	else if (ch == CH_LF) // 0x0A LF
	{
		m->cur[page2].cury++;
		if (m->cur[page2].cury >= m->rows)
		{
			PC_ScrollUp(page2);
			m->cur[page2].cury = m->rows - 1;
		}
	}
	else if (ch == 0x0C) // 0x0C FF clear screen
	{
		attr = 0;
		if (m->vclass == PC_VCLASS_TEXT) attr = PC_DEF_COL;

		// clear screen
		PC_ClearScr(page, attr);
	}
	else if (ch == 7) // 0x07 BELL
	{

	}
	else
	{
		// write character
		PC_WriteCh(ch, page, PC_DEF_COL, 1, noattr);

		// shift cursor
		m->cur[page2].curx++;
		if (m->cur[page2].curx >= m->cols)
		{
			// new line
			m->cur[page2].curx = 0;
			m->cur[page2].cury++;
			if (m->cur[page2].cury >= m->rows)
			{
				// scroll screen
				PC_ScrollUp(page2);
				m->cur[page2].cury = m->rows - 1;
			}
		}
	}
}

// set pixel
//  col ... color
//  x ... X coordinate
//  y ... Y coordinate
//  page ... page
void PC_SetPixel(u8 col, u16 x, u16 y, u8 page)
{
	// pointer to current videomode descriptor
	sPC_Vmode* m = &PC_Vmode;
	page &= m->pagemask;

	// check valid coordinate
	if ((x >= m->resx) || (y >= m->resy)) return;

	// prepare
	u8* base = m->base; // base
	int mask = m->mask; // offset mask
	int off = (page*m->planesize) & mask; // offset of the page
	int wb = m->wb; // width in bytes
	u8 mm, col2;
	int planes = 4; // number of color planes
	u8 col3 = col & 0x7f;

	// switch by videomode
	switch (m->vmode)
	{
	// (CGA, EGA, VGA) graphics 320x200, 4 colors, 2 strips
	case PC_VMODE_4:
	case PC_VMODE_5:
		base += (y & 1) * 0x2000;
		y >>= 1;
SETPIXEL_4:
		off = (off + (x >> 2) + y*wb) & mask;
		x = (3 - (x & 3)) << 1;
		col2 = (col & 3) << x;
		if (col >= 0x80)
			base[off] ^= col2;
		else
		{
			mm = 3 << x;
			base[off] = (base[off] & ~mm) | col2;
		}
		break;

	// (CGA, EGA, VGA) graphics 640x200, 2 colors, 2 strips
	case PC_VMODE_6:
		base += (y & 1) * 0x2000;
		y >>= 1;
		off = (off + (x >> 3) + y*wb) & mask;
		x = 7 - (x & 7);
		col2 = (col & 1) << x;
		if (col >= 0x80)
			base[off] ^= col2;
		else
		{
			mm = 1 << x;
			base[off] = (base[off] & ~mm) | col2;
		}
		break;

	// (PCjr, Tandy) graphics 160x200, 16 colors, 2 strips
	case PC_VMODE_8:
		base += (y & 1) * 0x2000;
		y >>= 1;
SETPIXEL_16:
		off = (off + (x >> 1) + y*wb) & mask;
		x = (1 - (x & 1)) << 2;
		col2 = (col & 0x0f) << x;
		if (col >= 0x80)
			base[off] ^= col2;
		else
		{
			mm = 0x0f << x;
			base[off] = (base[off] & ~mm) | col2;
		}
		break;

	// (PCjr, Tandy) graphics 320x200, 16 colors, 4 strips
	case PC_VMODE_9:
		base += (y & 3) * 0x2000;
		y >>= 2;
		goto SETPIXEL_16;

	// (PCjr, Tandy) graphics 640x200, 4 colors, 4 strips
	case PC_VMODE_10:
		base += (y & 3) * 0x2000;
		y >>= 2;
		goto SETPIXEL_4;

	// (Tandy) graphics 640x200, 16 colors, 8 strips
	case PC_VMODE_11:
		base += (y & 7) * 0x2000;
		y >>= 3;
		goto SETPIXEL_16;

	// (EGA, VGA) graphics 640x350, 4 or 16 colors, 2 or 4 planes
	case PC_VMODE_16:
		if (PC_Vmode.usedmem < BIT(PC_VMEM_128K)) // EGA card 64 KB
		{
			planes = 2; // number of color planes
			col3 = (col3 & 1) | ((col3 & 4) >> 1);
		}
	// (EGA, VGA) graphics 320x200, 16 colors, 4 planes
	case PC_VMODE_13:
	// (EGA, VGA) graphics 640x200, 16 colors, 4 planes
	case PC_VMODE_14:
	// (VGA) graphics 640x480, 16 colors, 4 planes
	case PC_VMODE_18:
SETPIXEL_PLANE:
		// offset
		off = (off + (x >> 3) + y*wb) & mask;
		x = 7 - (x & 7);

		// loop planes
		for (; planes > 0; planes--)
		{
			col2 = (col3 & 1) << x;

			if (col >= 0x80)
				base[off] ^= col2;
			else
			{
				mm = 1 << x;
				base[off] = (base[off] & ~mm) | col2;
			}

			// shift attribute
			col3 >>= 1;

			// shift base
			base += m->planesize;
		}
		break;

	// (EGA, VGA) graphics 640x350, 2 colors, 1 plane
	case PC_VMODE_15:
	// (VGA) graphics 640x480, 2 colors, 1 plane
	case PC_VMODE_17:
		planes = 1;
		goto SETPIXEL_PLANE;

	// (VGA) graphics 320x200, 256 colors
	case PC_VMODE_19:
		off = (off + x + y*wb) & mask;
		base[off] = col;
		break;

	// unsupported videomode
	default:
		break;
	}
}

// get pixel
//  x ... X coordinate
//  y ... Y coordinate
//  page ... page
u8 PC_GetPixel(u16 x, u16 y, u8 page)
{
	// pointer to current videomode descriptor
	sPC_Vmode* m = &PC_Vmode;
	page &= m->pagemask;

	// check valid coordinate
	if ((x >= m->resx) || (y >= m->resy)) return 0;

	// prepare
	u8* base = m->base; // base
	int mask = m->mask; // offset mask
	int off = (page*m->planesize) & mask; // offset of the page
	int wb = m->wb; // width in bytes
	u8 col, mm;
	int planes = 4; // number of color planes

	// switch by videomode
	switch (m->vmode)
	{
	// (CGA, EGA, VGA) graphics 320x200, 4 colors, 2 strips
	case PC_VMODE_4:
	case PC_VMODE_5:
		base += (y & 1) * 0x2000;
		y >>= 1;
GETPIXEL_4:
		off = (off + (x >> 2) + y*wb) & mask;
		x = (3 - (x & 3)) << 1;
		return (base[off] >> x) & 3;

	// (CGA, EGA, VGA) graphics 640x200, 2 colors, 2 strips
	case PC_VMODE_6:
		base += (y & 1) * 0x2000;
		y >>= 1;
		off = (off + (x >> 3) + y*wb) & mask;
		x = 7 - (x & 7);
		return (base[off] >> x) & 1;

	// (PCjr, Tandy) graphics 160x200, 16 colors, 2 strips
	case PC_VMODE_8:
		base += (y & 1) * 0x2000;
		y >>= 1;
GETPIXEL_16:
		off = (off + (x >> 1) + y*wb) & mask;
		x = (1 - (x & 1)) << 2;
		return (base[off] >> x) & 0x0f;

	// (PCjr, Tandy) graphics 320x200, 16 colors, 4 strips
	case PC_VMODE_9:
		base += (y & 3) * 0x2000;
		y >>= 2;
		goto GETPIXEL_16;

	// (PCjr, Tandy) graphics 640x200, 4 colors, 4 strips
	case PC_VMODE_10:
		base += (y & 3) * 0x2000;
		y >>= 2;
		goto GETPIXEL_4;

	// (Tandy) graphics 640x200, 16 colors, 8 strips
	case PC_VMODE_11:
		base += (y & 7) * 0x2000;
		y >>= 3;
		goto GETPIXEL_16;

	// (EGA, VGA) graphics 640x350, 4 or 16 colors, 2 or 4 planes
	case PC_VMODE_16:
		if (PC_Vmode.usedmem < BIT(PC_VMEM_128K)) // EGA card 64 KB
		{
			planes = 2; // number of color planes
		}
	// (EGA, VGA) graphics 320x200, 16 colors, 4 planes
	case PC_VMODE_13:
	// (EGA, VGA) graphics 640x200, 16 colors, 4 planes
	case PC_VMODE_14:
	// (VGA) graphics 640x480, 16 colors, 4 planes
	case PC_VMODE_18:
GETPIXEL_PLANE:
		// offset
		off = (off + (x >> 3) + y*wb) & mask;
		x = 7 - (x & 7);
		col = 0;
		mm = 1;

		// loop planes
		for (; planes > 0; planes--)
		{
			if (((base[off] >> x) & 1) != 0) col |= mm;

			// shift base
			base += m->planesize;

			// shift color mask
			mm <<= 1;
		}

		if ((m->vmode == PC_VMODE_16) && (PC_Vmode.usedmem < BIT(PC_VMEM_128K))) // EGA card 64 KB
		{
			col = (col & 1) | ((col & 2) << 1);
		}
		return col;

	// (EGA, VGA) graphics 640x350, 2 colors, 1 plane
	case PC_VMODE_15:
	// (VGA) graphics 640x480, 2 colors, 1 plane
	case PC_VMODE_17:
		planes = 1;
		goto GETPIXEL_PLANE;

	// (VGA) graphics 320x200, 256 colors
	case PC_VMODE_19:
		off = (off + x + y*wb) & mask;
		return base[off];

	// unsupported videomode
	default:
		return 0;
	}
}

// get color of EGA palette
u16 PC_EGAPal64(u8 pal)
{
	u16 col = 0;
	if ((pal & B0) != 0) col |= PC_EGA_BLUE;	// (0x01) blue 2/3
	if ((pal & B1) != 0) col |= PC_EGA_GREEN;	// (0x02) green 2/3
	if ((pal & B2) != 0) col |= PC_EGA_RED;		// (0x04) red 2/3
	if ((pal & B3) != 0) col |= PC_EGA_BLUE2;	// (0x08) blue 1/3
	if ((pal & B4) != 0) col |= PC_EGA_GREEN2;	// (0x10) green 1/3
	if ((pal & B5) != 0) col |= PC_EGA_RED2;	// (0x20) red 1/3
	return col;
}

// get color of VGA palette (color components 0..63)
u16 PC_VGAPal256(u8 r, u8 g, u8 b)
{
	return (u16)((((u16)r & 0x3e) << 10) | (((u16)g & 0x3f) << 5) | ((b & 0x3e) >> 1));
}

// INT 10h service - function
void PC_Int10fnc()
{
	// pointer to CPU
	sI8086* cpu = PC_Cpu;

	// pointer to current videomode descriptor
	sPC_Vmode* m = &PC_Vmode;

	// get AH and AL function
	u8 ah = cpu->ah;
	u8 al = cpu->al;

	switch (ah)
	{
	// set videomode
	case 0x00:
		PC_SetVmode(al);
		break;	

	// set cursor shape
	case 0x01:
		{
			u8 ch = cpu->ch & 0x1f; // start scanline
			u8 cl = cpu->cl & 0x1f; // end scanline
			if ((ch > 7) || (cl > 7)) // user want use higher fonts
			{
				ch = (u8)(ch * 8 / 14); // approx. recalculate 14 lines to 8 lines
				cl = (u8)((cl + 1) * 8 / 14);
				if (ch > 7) ch = 7;
				if (cl > 7) cl = 7;
			}
			m->curstart = ch; // start scanline
			m->curend = cl; // end scanline

			// 0040:0060 (2) cursor mode setting (end scan line / start scan line)
			PC_RAM_BASE[0x0460] = cl; // cursor end scanline
			PC_RAM_BASE[0x0461] = ch; // cursor start scanline
		}
		break;

	// set cursor position
	case 0x02:
		{
			u8 bh = cpu->bh & m->pagemask; // page
			u8 dh = cpu->dh; // row
			u8 dl = cpu->dl; // columns
			m->cur[bh].curx = dl; // column
			m->cur[bh].cury = dh; // row

			// 0040:0050 (16) cursor positions in 8 pages (column and row)
			PC_RAM_BASE[bh*2 + 0x0450] = dl; // column
			PC_RAM_BASE[bh*2 + 0x0451] = dh; // row
		}
		break;

	// get cursor info
	case 0x03:
		{
			u8 bh = cpu->bh & m->pagemask; // page
			cpu->ch = m->curstart; // start scanline
			cpu->cl = m->curend; // end scanline
			cpu->dh = m->cur[bh].cury; // row
			cpu->dl = m->cur[bh].curx; // column
		}
		break;

	// read light pen
	case 0x04:
		cpu->ah = 0; // pen not active
		break;

	// set active page
	case 0x05:
		{
			u8 page = cpu->al & m->pagemask; // new page
			m->page = page; // set current page
			m->off = page*m->planesize; // set offset

			// 0040:004E (2) page start address
			*(u16*)&PC_RAM_BASE[0x044E] = (u16)m->off; // page start address

			// 0040:0062 (1) current page number
			PC_RAM_BASE[0x0462] = page; // current page number
		}
		break;

	// scroll window up
	case 0x06:
		{
			int shift = cpu->al; // number of rows to scroll (0 = clear entire window)
			u8 attr = cpu->bh; // color attribute of new rows
			u8 x1 = cpu->cl; // left column
			u8 y1 = cpu->ch; // top row
			u8 x2 = cpu->dl; // right column
			u8 y2 = cpu->dh; // bottom row
			PC_Scroll(m->page, shift, attr, x1, y1, x2, y2);
		}
		break;

	// scroll window down
	case 0x07:
		{
			int shift = cpu->al; // number of rows to scroll (0 = clear entire window)
			u8 attr = cpu->bh; // color attribute of new rows
			u8 x1 = cpu->cl; // left column
			u8 y1 = cpu->ch; // top row
			u8 x2 = cpu->dl; // right column
			u8 y2 = cpu->dh; // bottom row
			PC_Scroll(m->page, -shift, attr, x1, y1, x2, y2);
		}
		break;

	// read character and attribute at cursor position
	//  Emulator supports only text mode (this feature is not used in games in graphics modes).
	case 0x08:
		{
			u8 vmode = m->vmode;
			if ((vmode <= 3) || (vmode == 7) || (vmode == 12))
			{
				u8 page = cpu->bh & m->pagemask; // page
				u8 row = m->cur[page].cury; // row
				u8 col = m->cur[page].curx; // column
				int wb = m->wb;
				int off = (page*m->planesize + row*wb + col*2) & m->mask; // offset in videomemory
				u8* s = m->base + off;
				cpu->al = s[0]; // character
				cpu->ah = s[1]; // attribute
			}
			else
			{
				// graphics mode is not supported by emulator
				cpu->al = ' ';
				cpu->ah = PC_DEF_COL;
			}
		}
		break;

	// write character and attribute at cursor position (without cursor update)
	case 0x09:
		{
			u8 ch = cpu->al; // character to display
			u8 page = cpu->bh; // page (or background color in 256-color mode)
			u8 attr = cpu->bl; // attribute in text mode, or color in graphics (bit 7: XOR with image at mode < 256 colors)
			int num = cpu->cx; // number of characters
			PC_WriteCh(ch, page, attr, num, False);
		}
		break;

	// write character at cursor position (without cursor update)
	case 0x0A:
		{
			u8 ch = cpu->al; // character to display
			u8 page = cpu->bh; // page (or background color in 256-color mode)
			u8 attr = cpu->bl; // color in graphics (bit 7: XOR with image at mode < 256 colors)
			int num = cpu->cx; // number of characters
			PC_WriteCh(ch, page, attr, num, True);
		}
		break;

	// set CGA palette
	case 0x0B:
		{
			u8 bh = cpu->bh;	// 0=background 320x200 or foreground 640x200 (EGA background 640x200),
						// 1=select palette 0 green-red-brown or 1 cyan-magenta-white
			u8 bl = cpu->bl;	// color 0-15 or palette 0-1

			// 0040:0066 (1) current CGA palette register (last value on 03D9h)
			//	B5: palette 0 (red-green-brown) or 1 (cyan-magenta-white)
			//	B4: intense colors in graphics, intense background in text mode
			//	B3: intense border color in 40x25, intense background in 320x200, intense foreground in 640x200
			//	B2: red border in 40x25, red background in 320x200, red foreground in 640x200
			//	B1: green border in 40x25, red background in 320x200, red foreground in 640x200
			//	B0: blue border in 40x25, red background in 320x200, red foreground in 640x200
			u8 crt = PC_RAM_BASE[0x0466];
			if (bh == 0)
			{
				// set background graphics color
				crt = (crt & 0xe0) | (bl & 0x1f);

				// background color in 2-color mode (EGA)
				PC_CurPal2[0] = PC_Pal16[(bl >> 4) & 0x0f];
			}
			else
			{
				// set background graphics color
				crt = (crt & ~B5) | ((bl << 5) & B5);
			}
			PC_RAM_BASE[0x0466] = crt;

			// set default CGA 4-color palette
			memcpy(PC_CurPal4, PC_Pal4_CgaCrt[(crt >> 4) & 3], 4*sizeof(u16));

			// blinking in text mode
			m->blink = ((crt & B4) == 0);

			// graphics color
			u16 cf = PC_Pal16[crt & 0x0f]; // color
			PC_CurPal4[0] = cf; // background color of 4-color palette
			PC_CurPal2[1] = cf; // foreground color of 2-color palette
		}
		break;

	// set pixel
	case 0x0C:
		{
			u8 col = cpu->al;		// color
			u16 x = cpu->cx;		// X coordinate
			u16 y = cpu->dx;		// Y coordinate
			u8 page = cpu->bh;		// page
			PC_SetPixel(col, x, y, page);	// set pixel
		}
		break;

	// get pixel
	case 0x0D:
		{
			u16 x = cpu->cx;		// X coordinate
			u16 y = cpu->dx;		// Y coordinate
			u8 page = cpu->bh;		// page
			cpu->al = PC_GetPixel(x, y, page); // get pixel
		}
		break;

	// write TTY chracter
	case 0x0E:
		{
			u8 ch = cpu->al;	// character
			u8 col = cpu->bl;	// color (only graphics modes)
			u8 page = cpu->bh;	// page
			PC_WriteTty(ch, page, col, True); // write character
		}
		break;

	// get current video mode
	case 0x0F:
		cpu->al = m->vmode; // current videomode (bit 7 should include "do not delete screen" flag - not provided)
		cpu->ah = m->cols; // number of text columns
		cpu->bh = m->page; // current page
		break;

	// setup EGA palette register
	case 0x10:
		{
			u8 mode = cpu->al; // mode (0=set register BL to BH, 1=set border BH, 2=set palettes ED:DX, 3=set blink BL)

			// set single palette register (emulator: only 16-color palette is affected)
			if (mode == 0)
			{
				u8 reg = cpu->bl; // palette register number
				if (reg < 16)
				{
					PC_CurPal16[reg] = PC_EGAPal64(cpu->bh);
				}
			}

			// set all palette registers (emulator: only 16-color palette is affected)
			else if (mode == 2)
			{
				// get table virtual address
				u32 a = I8086_ADDR(cpu->es, cpu->dx);

				// read table
				u8 reg;
				for (reg = 0; reg < 16; reg++)
				{
					PC_CurPal16[reg] = PC_EGAPal64(PC_GetMem(a));
					a++;
				}
			}

			// set blinking text
			else if (mode == 3)
			{
				m->blink = (cpu->bl != 0);
			}

			// set VGA DAC register (emulator: only 256-color palette is affected)
			else if (mode == 0x10)
			{
				u16 reg = cpu->bx; // register number
				if (reg < 256)
				{
					PC_CurPal256[reg] = PC_VGAPal256(cpu->dh, cpu->ch, cpu->cl);
				}
			}

			// set block of DAC registers (emulator: only 256-color palette is affected)
			else if (mode == 0x12)
			{
				u16 reg = cpu->bx; // starting register number
				u16 num = cpu->cx; // number of registers to set
				u32 a = I8086_ADDR(cpu->es, cpu->dx); // table virtual address
				for (; num > 0; num--)
				{
					if (reg >= 256) break;

					u8 r = PC_GetMem(a); a++;
					u8 g = PC_GetMem(a); a++;
					u8 b = PC_GetMem(a); a++;

					PC_CurPal256[reg] = PC_VGAPal256(r, g, b);
					reg++;
				}
			}
		}
		break;

	// get EGA info
	case 0x12:
		{
			// get EGA info
			if (cpu->bl == 0x10)
			{
				// color mode (0x03d4 -> 0 color, 0x03b4 -> 1 mono)
				cpu->bh = (PC_RAM_BASE[0x0463] == 0xB4) ? 1 : 0;

				// installed memory size (64 KB, 128 KB or 192 KB)
				cpu->bl = (m->cardmem <= 0x10000) ? 0 : ((m->cardmem <= 0x20000) ? 1 : 2);

				// adapter bits (primary EGA 40x25)
				cpu->cx = 6;
			}
		}
		break;

	// print text
	case 0x13:
		{
			// mode
			u8 mode = cpu->al;

			// page
			u8 page = cpu->bh & m->pagemask; // page

			// save current cursor
			u8 row = m->cur[page].cury; // row
			u8 col = m->cur[page].curx; // column

			// set new cursor position
			m->cur[page].cury = cpu->dh; // row
			m->cur[page].curx = cpu->dl; // column

			// length
			u16 len = cpu->cx;

			// get text virtual address
			u32 a = I8086_ADDR(cpu->es, cpu->bp);

			// print text with attribute BL
			if (mode <= 1)
			{
				// color attributes
				u8 attr = cpu->bl;

				for (; len > 0; len--)
				{
					// load character
					u8 ch = PC_GetMem(a); a++;

					// write character in TTY mode
					PC_WriteTty(ch, page, attr, True);
				}
			}

			// print text with color
			else if (mode <= 3)
			{
				for (; len > 0; len--)
				{
					// load character and attribute
					u8 ch = PC_GetMem(a); a++;
					u8 attr = PC_GetMem(a); a++;

					// write character in TTY mode
					PC_WriteTty(ch, page, attr, True);
				}
			}

			// return cursor position
			if ((mode != 1) && (mode != 3))
			{
				m->cur[page].cury = row; // row
				m->cur[page].curx = col; // column
			}
		}
		break;
	}
}
