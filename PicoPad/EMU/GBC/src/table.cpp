
// ****************************************************************************
//
//                                 Tables
//
// ****************************************************************************

#include "../include.h"

// Game Boy Mono palette definition

// black & white
#define GB_GRAY3	COLOR(255, 255, 255)
#define GB_GRAY2	COLOR(170, 170, 170)
#define GB_GRAY1	COLOR( 85,  85,  85)
#define GB_GRAY0	COLOR(  0,   0,   0)

// current palette (format RGB565)
u16 gbPalette[64] = {
	// OBJ0
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	// OBJ1
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	// BG
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	// invalid
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
	GB_GRAY3, GB_GRAY2, GB_GRAY1, GB_GRAY0,
};

// Title checksums that are treated specially by the CGB boot ROM
// 14 indexes above 0x40 (64) have to be disambiguated.
const u8 gbColorizationChecksums[79] =
{
    0x00, 0x88, 0x16, 0x36, 0xD1, 0xDB, 0xF2, 0x3C, 0x8C, 0x92, 0x3D, 0x5C,
    0x58, 0xC9, 0x3E, 0x70, 0x1D, 0x59, 0x69, 0x19, 0x35, 0xA8, 0x14, 0xAA,
    0x75, 0x95, 0x99, 0x34, 0x6F, 0x15, 0xFF, 0x97, 0x4B, 0x90, 0x17, 0x10,
    0x39, 0xF7, 0xF6, 0xA2, 0x49, 0x4E, 0x43, 0x68, 0xE0, 0x8B, 0xF0, 0xCE,
    0x0C, 0x29, 0xE8, 0xB7, 0x86, 0x9A, 0x52, 0x01, 0x9D, 0x71, 0x9C, 0xBD,
    0x5D, 0x6D, 0x67, 0x3F, 0x6B,
// For these games, the 4th letter is taken into account
    0xB3, 0x46, 0x28, 0xA5, 0xC6, 0xD3, 0x27,
    0x61, 0x18, 0x66, 0x6A, 0xBF, 0x0D, 0xF4
};

// For titles with colliding checksums, the fourth character of the game title
// for disambiguation.
const u8 gbColorizationDisambigChars[29] =
{
    'B', 'E', 'F', 'A', 'A', 'R', 'B', 'E',
    'K', 'E', 'K', ' ', 'R', '-', 'U', 'R',
    'A', 'R', ' ', 'I', 'N', 'A', 'I', 'L',
    'I', 'C', 'E', ' ', 'R'
};

// Palette ID | (Flags << 5)
const u8 gbColorizationPaletteInfo[94] =
{
    0x7C, 0x08, 0x12, 0xA3, 0xA2, 0x07, 0x87, 0x4B, 0x20, 0x12, 0x65, 0xA8,
    0x16, 0xA9, 0x86, 0xB1, 0x68, 0xA0, 0x87, 0x66, 0x12, 0xA1, 0x30, 0x3C,
    0x12, 0x85, 0x12, 0x64, 0x1B, 0x07, 0x06, 0x6F, 0x6E, 0x6E, 0xAE, 0xAF,
    0x6F, 0xB2, 0xAF, 0xB2, 0xA8, 0xAB, 0x6F, 0xAF, 0x86, 0xAE, 0xA2, 0xA2,
    0x12, 0xAF, 0x13, 0x12, 0xA1, 0x6E, 0xAF, 0xAF, 0xAD, 0x06, 0x4C, 0x6E,
    0xAF, 0xAF, 0x12, 0x7C, 0xAC, 0xA8, 0x6A, 0x6E, 0x13, 0xA0, 0x2D, 0xA8,
    0x2B, 0xAC, 0x64, 0xAC, 0x6D, 0x87, 0xBC, 0x60, 0xB4, 0x13, 0x72, 0x7C,
    0xB5, 0xAE, 0xAE, 0x7C, 0x7C, 0x65, 0xA2, 0x6C, 0x64, 0x85
};

// Uncompressed palette data from the CGB boot ROM (format BGR555)
#define COL(b,g,r) (((b)<<10)|((g)<<5)|(r)) // value = 0..31
const u16 gbColorizationPaletteData[PALDATA_NUM][3][4] =
{
	// 0x00
	{
		{ COL(31,31,31), COL( 0,14,31), COL( 0, 8,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,23,11), COL( 0, 0,31), COL(31, 0, 0), },
		{ COL(31,31,31), COL(16,21,21), COL(15,14, 8), COL( 0, 0, 0), },
	},
	// 0x01
	{
		{ COL( 8,24,31), COL( 0,26,31), COL( 0, 7,18), COL( 0, 0, 9), },
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(19,31,31), COL(31,22,18), COL(14,18,12), COL( 7, 7, 0), },
	},
	// 0x02
	{
		{ COL(31,31,31), COL(31,31,31), COL(31,20,12), COL(31, 0, 0), },
		{ COL(31,31,31), COL(12,21,31), COL( 0, 6,16), COL( 0, 0, 0), },
		{ COL( 0,31,13), COL(31,31,31), COL( 9,10,31), COL( 0, 0, 0), },
	},
	// 0x03
	{
		{ COL(31,31,31), COL(31,31,31), COL(31,20,12), COL(31, 0, 0), },
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL( 0,27,10), COL( 0,16,31), COL( 0,31,31), COL(31,31,31), },
	},
	// 0x04
	{
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 0,31,15), COL( 0,14,22), COL( 0, 0, 0), },
	},
	// 0x05
	{
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,23,11), COL( 0, 0,31), COL(31, 0, 0), },
		{ COL(31,31,31), COL( 0,26, 8), COL( 0, 8,31), COL( 0, 0, 0), },
	},
	// 0x06
	{
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,23,11), COL( 0, 0,31), COL(31, 0, 0), },
		{ COL(31,31,31), COL( 0,19,31), COL( 0, 0,31), COL( 0, 0, 0), },
	},
	// 0x07
	{
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,23,11), COL( 0, 0,31), COL(31, 0, 0), },
		{ COL(31,31,31), COL( 0,31,31), COL( 0, 0,31), COL( 0, 0, 0), },
	},
	// 0x08
	{
		{ COL(10,12,31), COL( 0, 0,26), COL( 0, 0,12), COL( 0, 0, 0), },
		{ COL(31, 0, 0), COL(31,31,31), COL(15,31,31), COL(31,16, 0), },
		{ COL(31,19,20), COL( 0,31,31), COL( 0,12, 0), COL( 0, 0, 0), },
	},
	// 0x09
	{
		{ COL(31,31,31), COL( 0,14,31), COL( 0, 8,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(25,31,31), COL(29,29,12), COL( 6,16,19), COL(11,11,11), },
	},
	// 0x0A
	{
		{ COL( 0, 0, 0), COL(31,31,31), COL(16,16,31), COL( 7, 7,18), },
		{ COL( 0, 0, 0), COL(31,31,31), COL(16,16,31), COL( 7, 7,18), },
		{ COL(31,22,22), COL(18,31,31), COL( 8,11,21), COL( 0, 0, 0), },
	},
	// 0x0B
	{
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(15,31,31), COL(31,16, 0), COL( 0, 0,31), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
	},
	// 0x0C
	{
		{ COL( 8,24,31), COL( 0,26,31), COL( 0, 7,18), COL( 0, 0, 9), },
		{ COL(31,31,31), COL(31,23,11), COL( 0, 0,31), COL(31, 0, 0), },
		{ COL(31,31,31), COL(27,17,17), COL(17,10,10), COL( 0, 0, 0), },
	},
	// 0x0D
	{
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(12,21,31), COL( 0, 6,16), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(27,17,17), COL(17,10,10), COL( 0, 0, 0), },
	},
	// 0x0E
	{
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 6,31,15), COL( 0,16, 0), COL( 0, 0, 0), },
	},
	// 0x0F
	{
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 6,31,15), COL( 0,16, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(12,21,31), COL( 0, 6,16), COL( 0, 0, 0), },
	},
	// 0x10
	{
		{ COL(31,31,31), COL( 6,24,15), COL( 0,16, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
	},
	// 0x11
	{
		{ COL(31,31,31), COL( 0,31, 0), COL( 0,16, 6), COL( 0, 9, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
	},
	// 0x12
	{
		{ COL(31,31,31), COL( 6,31,15), COL( 0,16, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(12,21,31), COL( 0, 6,16), COL( 0, 0, 0), },
	},
	// 0x13
	{
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 6,31,15), COL( 0,16, 0), COL( 0, 0, 0), },
		{ COL( 0, 0, 0), COL(16,16, 0), COL( 0,27,31), COL(31,31,31), },
	},
	// 0x14
	{
		{ COL( 0,31,31), COL( 0, 0,31), COL( 0, 0,12), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 6,31,15), COL( 0,16, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
	},
	// 0x15
	{
		{ COL(31,31,31), COL(12,21,31), COL( 0, 6,16), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(16,21,21), COL(15,14, 8), COL( 0, 0, 0), },
	},
	// 0x16
	{
		{ COL(31,31,31), COL(20,20,20), COL(10,10,10), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(20,20,20), COL(10,10,10), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(20,20,20), COL(10,10,10), COL( 0, 0, 0), },
	},
	// 0x17
	{
		{ COL(31,31,31), COL( 6,31,15), COL( 0,16, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(20,31,31), COL(18,18,31), COL(31,18,18), COL( 0, 0, 0), },
	},
	// 0x18
	{
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 6,31,15), COL( 0,16, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
	},
	// 0x19
	{
		{ COL(31,31,31), COL(12,21,31), COL( 0, 6,16), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(12,21,31), COL( 0, 6,16), COL( 0, 0, 0), },
		{ COL(24,28,31), COL(16,19,25), COL( 5,13,16), COL( 1, 6,11), },
	},
	// 0x1A
	{
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 6,31,15), COL( 0,16, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 0,31,31), COL( 0, 9,15), COL( 0, 0, 0), },
	},
	// 0x1B
	{
		{ COL(31,31,31), COL( 0,25,31), COL( 0,12,19), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 0,25,31), COL( 0,12,19), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 0,25,31), COL( 0,12,19), COL( 0, 0, 0), },
	},
	// 0x1C
	{
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 6,24,15), COL(24,12, 0), COL( 0, 0, 0), },
	},
	// 0x1D
	{
		{ COL( 8, 9, 0), COL( 0, 1, 2), COL( 0,20, 1), COL( 4, 8,16), },
		{ COL(31,27,31), COL(23,27,31), COL( 4,16, 1), COL( 0, 0, 1), },
		{ COL(31,24, 0), COL(31,27,31), COL( 7,24, 3), COL( 0, 0, 0), },
	},
	// 0x1E
	{
		{ COL(31,31,31), COL(12,21,31), COL( 0, 6,16), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(12,21,31), COL( 0, 6,16), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(12,21,31), COL( 0, 6,16), COL( 0, 0, 0), },
	},
	// 0x1F
	{
		{ COL(31,31,31), COL(21,21,21), COL(10,10,10), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(21,21,21), COL(10,10,10), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(21,21,21), COL(10,10,10), COL( 0, 0, 0), },
	},
	// 0x20 (use flag 7)
	{
		{ COL(23,31,29), COL(15,21,19), COL( 7,10, 9), COL( 0, 0, 0), },
		{ COL(23,31,29), COL(15,21,19), COL( 7,10, 9), COL( 0, 0, 0), },
		{ COL(23,31,29), COL(15,21,19), COL( 7,10, 9), COL( 0, 0, 0), },
	},
	// 0x21 (use flag 7)
	{
		{ COL(31,26,29), COL(21,17,20), COL(10, 9,10), COL( 0, 0, 0), },
		{ COL(31,26,29), COL(21,17,20), COL(10, 9,10), COL( 0, 0, 0), },
		{ COL(31,26,29), COL(21,17,20), COL(10, 9,10), COL( 0, 0, 0), },
	},
	// 0x22 (use flag 7)
	{
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(24,12, 0), COL( 6,24,15), COL( 0, 0, 0), },
	},
	// 0x23 (use flag 7)
	{
		{ COL(31,31,31), COL(16,20,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(26,28,31), COL(24,12, 0), COL( 6,24,15), COL( 0, 0, 0), },
	},
	// 0x24 (use flag 7)
	{
		{ COL(31,31,31), COL(31,31,31), COL( 0,28, 0), COL(31, 0, 0), },
		{ COL(31,31,31), COL(12,21,31), COL( 0, 6,16), COL( 0, 0, 0), },
		{ COL(31,24,12), COL( 8,31,31), COL( 8, 8,31), COL( 0, 0, 0), },
	},
	// 0x25 (use flag 7)
	{
		{ COL(15,31,31), COL( 0,20,31), COL( 0,12,20), COL( 0, 0, 0), },
		{ COL(20,31,31), COL( 8,15,31), COL( 4, 8,20), COL( 0, 0, 0), },
		{ COL( 8,28, 8), COL(28,22,18), COL( 8,20, 8), COL( 0, 4, 0), },
	},
	// 0x26 (use flag 7)
	{
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 0,31,31), COL(24,12, 0), COL( 0, 0, 0), },
	},
	// 0x27 (use flag 7)
	{
		{ COL(31,31,31), COL(16,16,31), COL( 7, 7,18), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 0,26,31), COL(24,12, 0), COL( 0, 0, 0), },
	},
	// 0x28 (use flag 7)
	{
		{ COL(10,22,30), COL(16,16,31), COL( 0, 4,14), COL( 0, 0, 0), },
		{ COL(31,31,31), COL(31,20,12), COL(31, 0, 0), COL( 0, 0, 0), },
		{ COL(31,31,31), COL( 0,26,31), COL( 0, 0,26), COL( 4, 0, 0), },
	},
	// 0x29 (use flag 7)
	{
		{ COL( 0,31,31), COL( 0, 0,31), COL( 0, 8,16), COL( 0, 0, 0), },
		{ COL(20,31,31), COL( 8,15,31), COL( 4, 8,20), COL( 0, 0, 0), },
		{ COL( 6,24,15), COL(24,12, 0), COL( 4,18, 4), COL( 0, 2, 4), },
	},
	// 0x2A (use flag 7)
	{
		{ COL(26,31,31), COL( 0,14,31), COL( 0, 8,18), COL( 0, 2, 6), },
		{ COL(16,31,31), COL(31,23,11), COL( 0, 0,31), COL( 8, 3, 0), },
		{ COL(31,31,31), COL(16,21,21), COL(15,14, 8), COL( 0, 0, 0), },
	},
	// 0x2B (use flag 7)
	{
		{ COL( 3,28,12), COL( 1,20, 7), COL( 0,12, 4), COL( 0, 0, 0), },
		{ COL( 8,26,31), COL( 0,18,31), COL( 0, 7,18), COL( 0, 0, 9), },
		{ COL(31,31,31), COL(27,17,17), COL(17,10,10), COL( 0, 0, 0), },
	},
};

// palette manual selection
//  flag 5 = normal state, flag 7 = use index 0x20..
#define PAL(inx, flag) ((inx) | ((flag)<<5))
const u8 gbColorPalSel[PAL_NUM] =
{
	PAL(0x1C, 3),	// 1	... default palette
	PAL(0x1F, 5),	// 2	... black & white
	PAL(0x20, 7),	// 3	... soft green
	PAL(0x21, 7),	// 4	... soft blue
	PAL(0x00, 1),	// 5
	PAL(0x00, 3),	// 6
	PAL(0x00, 5),	// 7
	PAL(0x01, 5),	// 8
	PAL(0x02, 5),	// 9
	PAL(0x03, 5),	// 10
	PAL(0x04, 3),	// 11
	PAL(0x05, 3),	// 12
	PAL(0x05, 4),	// 13
	PAL(0x06, 0),	// 14
	PAL(0x06, 3),	// 15
	PAL(0x06, 4),	// 16
	PAL(0x07, 0),	// 17
	PAL(0x07, 4),	// 18
	PAL(0x08, 0),	// 19
	PAL(0x08, 3),	// 20
	PAL(0x08, 5),	// 21
	PAL(0x09, 5),	// 22
	PAL(0x0A, 3),	// 23
	PAL(0x0B, 1),	// 24
	PAL(0x0B, 2),	// 25
	PAL(0x0B, 5),	// 26
	PAL(0x0C, 2),	// 27
	PAL(0x0C, 3),	// 28
	PAL(0x0C, 5),	// 29
	PAL(0x0D, 1),	// 30
	PAL(0x0D, 3),	// 31
	PAL(0x0D, 5),	// 32
	PAL(0x0E, 3),	// 33
	PAL(0x0E, 5),	// 34
	PAL(0x0F, 3),	// 35
	PAL(0x0F, 5),	// 36
	PAL(0x10, 1),	// 37
	PAL(0x11, 5),	// 38
	PAL(0x12, 0),	// 39
	PAL(0x12, 3),	// 40
	PAL(0x12, 5),	// 41
	PAL(0x13, 0),	// 42
	PAL(0x14, 5),	// 43
	PAL(0x15, 5),	// 44
	PAL(0x16, 0),	// 45
	PAL(0x1B, 0),	// 46
	PAL(0x1C, 1),	// 47
	PAL(0x1C, 5),	// 48
	PAL(0x1D, 5),	// 49
	PAL(0x1E, 5),	// 50
// additions (use flag 7)
	PAL(0x22, 7),	// 51
	PAL(0x23, 7),	// 52
	PAL(0x24, 7),	// 53
	PAL(0x25, 7),	// 54
	PAL(0x26, 7),	// 55
	PAL(0x27, 7),	// 56
	PAL(0x28, 7),	// 57
	PAL(0x29, 7),	// 58
	PAL(0x2A, 7),	// 59
	PAL(0x2B, 7),	// 60
};

// extra palette setup - DMG games (* = original palette set by GBC)
// - If the CRCs of the two programs are identical, add non-zero checksum
//   from ROM address 0x14d to the high byte of the second number.
const u16 gbPalExt[] = {
	0x13FB, 41,	// 4IN1    : 4-in-1 FunPak
	0x22A0, 50,	// 4IN1_2  : 4-in-1 FunPak Vol.II
	0x2E7E, 8,	// ADAMS2  : Addams Family - Pugsley's Scavenger Hunt
	0x8FFC, 32,	// ADVISL  : Adventure Island
	0x8DE6, 32,	// ADVISL2 : Adventure Island II
	0xED7E, 5,	// ADVROCK : Adventures of Rocky and Bullwinkle and Friends
	0x3E6F, 35,	// ADVSTAR : Adventures of Star Saver
	0xB9B7, 32,	// AEROSTAR: Aerostar
	0xE14A, 5,	// ALADDIN : Aladdin
	0x13E4, 8,	// ALIEN3  : Alien 3
	0x1DCE, 19,	//*ALLEY   : Alley Way
	0xC208, 44,	// ALPRED  : Alien vs Predator
	0x9D18, 44,	// AMAZING : Amazing Tater
	0xE05A, 5,	// AMSPIDER: Amazing Spider-Man
	0xA374, 20,	// ANIMAN  : Animaniacs
	0xB3B7, 33,	//*ARCADE1 : Arcade Classic No. 1 - Asteroids & Missile Command
	0xCAAD, 1,	//*ARCADE2 : Arcade Classic No. 2 - Centipede & Millipede
	0x3CF2, 42,	//*ARCADE3 : Arcade Classic No. 3 - Galaga & Galaxian
	0x2ED5, 29,	// ASTEROID: Asteroids
	0xDF1D, 32,	// ATOMIC  : Atomic Punk
	0xA250, 9,	// ATTACK  : Attack of the Killer Tomatoes
	0x027B, 22,	// AVENG   : Avenging Spirit
	0x0CCA, 14,	//*BALLOON : Balloon Kid
	0x2B9E, 24,	// BARBIE  : Barbie - Game Girl
	0x66D8, 43,	// BART    : Bart Simpson's Escape from Camp Deadly
	0xC057, 10,	// BASEBALL: Baseball
	0xBED5, 8,	// BATANIM : Batman - The Animated Series
	0x7F56, 36,	//*BATARENA: Battle Arena Toshinden
	0x9BC5, 5,	// BATDRAG : Battletoads - Double Dragon
	0xDB58, 35,	// BATFOR  : Batman Forever
	0xC4DD, 30,	// BATRAGN : Battletoads in Ragnarok's World
	0xF44D, 32,	// BATRET  : Batman - Return of the Joker
	0xB88D, 1,	// BATSHIP : Battleship
	0x2465, 41,	// BATTOAD : Battletoads
	0xD10A, 8,	// BATUNIT : Battle Unit Zeoth
	0xBF5C, 24,	// BATVID  : Batman - The Video Game
	0x539A, 28,	// BEAVIS  : Beavis and Butt-head
	0xDA9A, 40,	// BEETLE  : Beetlejuice
	0x7BB9, 28,	// BILLTED : Bill & Teds Excellent Game
	0xCE05, 44,	// BIONIC  : Bionic Commando
	0x5C39, 23,	// BLADES  : Blades of Steel
	0x478A, 29,	// BLASTER : Blaster Master Boy
	0xB51C, 8,	// BLUES   : Blues Brothers
	0x95F4, 44,	// BOMBER  : Bomberman
	0x1A52, 32,	// BONKADV : Bonk's Adventure
	0x6DBE, 32,	// BONKREV : Bonk's Revenge
	0x19C3, 44,	// BOXXLE  : Boxxle
	0x46D9, 44,	// BOXXLE2 : Boxxle II
	0x05EC, 33,	// BRAM    : Bram Stoker's Dracula
	0x3164, 43,	// BUBBLE2 : Bubble Bobble Part 2
	0x23B2, 35,	// BUBGHOST: Bubble Ghost
	0x23DB, 5,	// BUGS    : Bugs Bunny Crazy Castle
	0x1234, 35,	// BUGS2   : Bugs Bunny Crazy Castle 2
	0xB7C3, 44,	// BURAI   : Burai Fighter Deluxe
	0x6B8B, 32,	// BURGER  : BurgerTime Deluxe
	0x9A8D, 36,	// BUST2   : Bust-A-Move 2 - Arcade Edition
	0xC1DD, 44,	// BUSTER  : Buster Brothers
	0x48E7, 30,	// CAPTAIN : Captain America and the Avengers
	0x2E1C, 36,	// CASTL   : Castlevania - The Adventure
			// CASTLB  : Castlevania - The Adventure
	0xB053, 44,	// CASTL2  : Castlevania II - Belmont's Revenge
	0x1BE1, 32,	// CASTLLEG: Castlevania Legends
	0xCCB0, 29,	// CATRAP  : Catrap
	0x95E2, 29,	// CONTRA  : Contra
	0x19F7, 46,	// CONTWAR : Contra - The Alien Wars
	0x1543, 30,	// DAFFY   : Daffy Duck
	0xB322, 30,	// DARKMAN : Darkman
	0x1430, 30,	// DARKWING: Darkwing Duck
	0x7FF2, 22,	// DBLDRAG : Double Dragon
	0xB6C7, 8,	// DBLDRAG2: Double Dragon II
	0xD77D, 22,	// DIGDUG  : Dig Dug
	0x355B, 15,	//*DONK    : Donkey Kong (Jp, USA)
			//*DONKW   : Donkey Kong (World)
	0x5466, 8,	//*DONKEY  : Donkey Kong Land
	0x3926, 29,	//*DONKEY2 : Donkey Kong Land 2
	0x0E16, 29,	//*DONKEY3 : Donkey Kong Land 3
	0xD5C4, 29,	// DRAGON  : DragonHeart
	0x3A7D, 25,	// DRMARIO : Dr. Mario
	0xDDD6, 1,	// DUCK    : DuckTales
	0x9FD0, 22|(0xc9<<8), // DUCK2   : DuckTales 2
	0xE60E, 11,	// EARTHJIM: Earthworm Jim
	0x7041, 44,	// ELEVATOR: Elevator Action
	0x5B8D, 39,	//*F1RACE  : F-1 Race
	0x9173, 17,	//*FACEBALL: Faceball 2000
	0x3B2E, 1,	// FELIX   : Felix the Cat
	0x1BD1, 47,	// FIFA    : FIFA International Soccer
	0xB1F4, 1,	// FIFA96  : FIFA Soccer 96
	0x1BA5, 1,	// FIFA97  : FIFA Soccer 97
	0x186B, 1,	// FIFA98  : FIFA-Road to World Cup 98
	0x441D, 48,	// FINAL   : Final Fantasy Legend
	0xE9E7, 1,	// FINAL2  : Final Fantasy Legend II
	0x9286, 1,	// FINAL3  : Final Fantasy Legend III
	0xCBAA, 1,	// FINALA  : Final Fantasy Adventure (USA)
			// FINALAW : Final Fantasy Adventure (world)
	0x47FC, 44,	// FIST    : Fist of the North Star
	0x813B, 8,	// FLASH   : The Flash
	0x13C2, 48,	// FLINT   : The Flintstones
	0x3706, 8,	// FLINTK  : The Flintstones - King Rock Treasure Island
	0x31A3, 40,	// FROGGER : Frogger
	0x2B2E, 1,	// GARGOYL : Gargoyle's Quest
	0xA6A1, 1,	// GAUNT2  : Gauntlet II
	0x8AB5, 1,	// GHOST2  : Ghostbusters II
	0x798C, 1,	// GODZIL  : Godzilla
	0x2246, 33,	//*GOLF    : Golf
	0xC407, 1,	// GRADIUS : Gradius - The Interstellar
	0x13C8, 23,	// GREM2   : Gremlins 2 - The New Batch
	0xB98C, 11,	// GWATCH  :*Game & Watch Gallery
	0x8DA0, 1,	// HALONE  : Home Alone
	0x44DA, 1,	// HALONE2 : Home Alone 2 - Lost in New York
	0x5A43, 1,	// HARVEST : Harvest Moon
	0xEC53, 52,	// HERCULES: Hercules
	0xCBCA, 51,	// HUNT    : Hunt for Red October
	0xCC1E, 1,	// HYPER   : Hyper Lode Runner
	0x6DA2, 51,	// CHASE   : Chase H.Q.
	0x9B21, 32,	// INDIANA : Indiana Jones and the Last Crusade
	0x2961, 43,	// IRONMAN : Iron Man X-O Manowar in Heavy Metal
	0x5334, 47,	// JBOND   : James Bond 007
	0xD378, 48,	// JETSONS : The Jetsons - Robot Panic
	0x8FC7, 1,	// JOEMAC  : Joe & Mac
	0x633C, 1,	// JUDGE   : Judge Dredd
	0xEB0F, 1,	// JUNGLE  : The Jungle Book
	0x5CBE, 1,	// JURPARK : Jurassic Park
	0xA35A, 1,	// JURPARK2: Jurassic Park Part 2 - The Chaos Continues
	0XBCA5, 1,	// KIDDRAC : Kid Dracula
	0x7500, 31,	//*KIDICAR : Kid Icarus - Of Myths and Monsters
	0xFF6B, 32,	//*KILLER  : Killer Instinct
	0x9FD0, 36|(0xDD<<8), //*KING95  : The King of Fighters '95
	0x6824, 21,	//*KIRBYB  : Kirby's Block Ball
	0x0412, 21,	//*KIRBYD  : Kirby's Dream Land
	0xC7FB, 21,	//*KIRBYD2 : Kirby's Dream Land 2
	0xCD89, 20,	//*KIRBYP  : Kirby's Pinball Land
	0x8E7E, 39,	//*KIRBYS  : Kirby's Star Stacker
	0xBFD3, 1,	// KNIGHT  : Knight Quest
	0x1A2A, 43,	// KRUSTY  : Krusty's Fun House
	0xE3DF, 22,	// KUNGFU  : Kung-Fu Master
	0x0A26, 32,	// KWIRK   : Kwirk - He's A-maze-ing!
	0x6845, 8,	// LEGRIV  : Legend of the River King
	0x9A93, 47,	// LEGZELDA: The Legend of Zelda - Link's Awakening (original palette = 38)
	0xC086, 53,	// LEMMINGS: Lemmings
	0x83E4, 54,	// LION    : The Lion King
	0xA3DE, 52,	// LITTLE  : The Little Mermaid
	0x1953, 1,	// LOONEY  : Looney Tunes
	0x114B, 1,	// LOONEY2 : Looney Tunes 2 - Tasmanian Devil in Island Chase
	0x9348, 51,	// LOSTW   : The Lost World - Jurassic Park
	0xAC2A, 1,	// MADDEN95: Madden 95
	0x7F33, 1,	// MADDEN96: Madden 96
	0x6A6C, 51,	// MADDEN97: Madden 97
	0xDE5D, 22,	// MARBLE  : Marble Madness
	0x2F3C, 39,	//*MARIOP  : Mario's Picross
	0x4A98, 1,	// MEGAMAN2: Mega Man II
	0x1729, 1,	// MEGAMAN3: Mega Man III
	0x3E94, 1,	// MEGAMAN4: Mega Man IV
	0x5413, 1,	// MEGAMAN5: Mega Man V
	0xCDE7, 1,	// MERCEN  : Mercenary Force
	0x4952, 43,	//*METR2   : Metroid II - Return of Samus
	0xC8FE, 41,	// MICKEY  : Mickey Mouse - Magic Wands!
	0xDF10, 1,	// MICKEYD : Mickey's Dangerous Chase
	0x082B, 1,	// MICKEYU : Mickey's Ultimate Challenge
	0x03D4, 1,	// MICRO   : Micro Machines
	0x0D42, 43,	// MIGHTY  : Mighty Morphin Power Rangers
	0xEB75, 6,	//*MOLE    : Mole Mania
	0x3C63, 1,	// MONOPOLY: Monopoly
	0x697A, 35,	// MORTAL  : Mortal Kombat
	0xA30B, 35,	// MORTAL2 : Mortal Kombat II
	0xA21D, 32,	// MORTAL3 : Mortal Kombat 3
	0x3C88, 44,	// MOTO    : Motocross Maniacs
	0x0BE1, 29,	// MSPACMAN: Ms. Pac-Man
	0x0B92, 1,	// MYST    : Mystical Ninja Starring Goemon
	0x9299, 23,	// NBAJAM  : NBA Jam
	0x5B4F, 23,	// NBAJAMT : NBA Jam - Tournament Edition
	0x5540, 36,	// NEMESIS : Nemesis
	0xF7CC, 41,	// NHL95   : NHL Hockey 95
	0xEC82, 41,	// NHL96   : NHL Hockey 96
	0x17B2, 1,	// NINJA   : Ninja Boy
	0x7934, 51,	// NINJA2  : Ninja Boy 2
	0x15D6, 32,	// NINT    : Nintendo World Cup (original palette = 33)
	0xDFBB, 1,	// ODDW    : Oddworld Adventures
	0x95D4, 32,	// OPERC   : Operation C
	0x2350, 46,	// PACAT   : Pac-Attack
	0x6F6A, 8,	// PACIN   : Pac-In-Time
	0x5B22, 18,	// PACMAN  : Pac-Man (USA)
			// PACMANJ : Pac-Man (JAPAN)
	0xFA58, 30,	// PAPER   : Paperboy
	0xF130, 43,	// PAPER2  : Paperboy 2
	0x563E, 1,	// PINBALL : Pinball - Revenge of the 'Gator
	0x5CF3, 21,	// PINBALLD: Pinball Dreams
	0x23AC, 45,	//*PLAGUE  : Plaguemon Lost Diaries v0.666
	0x6835, 1,	// POCAH   : Pocahontas
	0x53C7, 24,	//*POKBLUE : Pokemon - Blue Version
					//*POKBABY : Pokemon Baby Blue v19-08-23
					//*POKBLUB : Pokemon Blueberry v1.0.18
					//*POKBLUEC: Pokemon Blue Celebrations v1.8
					//*POKBLUEK: Pokemon - Blue Kaizo
					//*POKLET  : Pokemon Let's Go Meltan v1
					//*POKREGB : Pokemon Regulation Blue
					//*POKSIS1 : Pokemon Sister Version v1.1
					//*POKSIS2 : Pokemon Sister Version v1.2
					//*POKSIS3 : Pokemon Sister Version v1.3
//	0x11A9, 1,			// POKBEAST: Pokemon - Star Beasts - Meteor Version
	0xDBA1, 37,	//*POKRED  : Pokemon - Red Version
					//*POKBLOOD: Pokemon Blood and Tears v0.1
					//*POKCARM : Pokemon Carmine Red (v1.0.1)
					//*POKCARM1: Pokemon Carmine Red (v1.1)
					//*POKCHARI: Pokemon Chari Red
					//*POKINT  : Pokemon - Intense Indigo Version
					//*POKPERF : Pokemon Perfect Red
					//*POKLIT  : Pokemon Little Cup Red
					//*POKMAIZ : Pokemon Maize v3.2
					//*POKREDC : Pokemon Red Celebrations v1.8
					//*POKREDCU: Pokemon Redcurrant v1.0.18
					//*POKREDE : Pokemon Red Expert v1
					//*POKREDI : Pokemon Red Ignited Beta
					//*POKREDP : Pokemon Red Patched
					//*POKREGR : Pokemon Regulation Red
					//*POKREMIX: Pokemon Remixed Red v1.1
					//*POKRGHT0: Pokemon Righteous Red v1.1.2
					//*POKRIGHT: Pokemon Righteous Red v1.2.1
					//*POKROAM : Pokemon Roaming Red
					//*POKUNOV : Pokemon Unova Red
//	0x62AD, 36,			// POKBROWN: Pokemon Brown
//	0xEA4A, 1,			// POKFAC  : Pokemon Factory Adventure
	0x6639, 47,	//*POKGREEN: Pokemon Green
//	0x3E9F, 1,			// POKKANT : Pokemon Kanto Expansion v1.0
					// POKKANT2: Pokemon Kanto Expansion v1.2
//	0x7EE6, 1,			// POKNEO  : Pokemon Neo Adventure Revised V1.0
//	0xB163, 37,			// POKRRUM : Pokemon - Red Rumor
	0x271C, 1,	// POPEYE2 : Popeye 2
	0xA03E, 27,	// PRIMAL  : Primal Rage
	0x9D63, 1, 	// PRINCE  : Prince of Persia
	0x7DEB, 1,	// PUNISHER: The Punisher - The Ultimate Payback
	0xA0CF, 26,	// QBERT   : Q-bert for Game Boy
	0xC5A4, 36,	// QUARTH  : Quarth
	0xCCF7, 44,	// REALG   : The Real Ghostbusters
	0x315B, 56,	// RENSTIMS: The Ren & Stimpy Show - Space Cadet Adventures
	0xDCC5, 56,	// RENSTIMV: The Ren & Stimpy Show - Veediots!
	0xFBA3, 1,	// ROLAN   : Rolan's Curse
	0xEF53, 1,	// ROLAN2  : Rolan's Curse II
	0x467E, 41,	// RTYPE   : R-Type
	0x9A83, 57,	// RUGRATS : The Rugrats Movie
	0xD030, 32,	// SAMURAI : Samurai Shodown
	0x316E, 58,	// SIMPSB  : The Simpsons - Bart & The Beanstalk
	0xCA1E, 1,	// SIMPSI  : The Simpsons - Itchy & Scratchy - Miniature Golf
	0x2652, 1,	// SIMPSJ  : The Simpsons - Bart vs. The Juggernauts
	0xEE52, 52,	// SKATE   : Skate or Die - Bad 'N Rad
	0x14E8, 56,	// SMALLS  : Small Soldiers
	0xFB0F, 23,	//*SMARIO  : Super Mario Land
	0xABF5, 22,	//*SMARIO2 : Super Mario Land 2 - 6 Golden Coins
	0xBE1D, 53,	// SNOOPY  : Snoopy's Magic Show
	0xAE31, 42,	//*SOLAR   : SolarStriker
	0xCA51, 42,	//*SPACEINV: Space Invaders
	0x4E9C, 1,	// SPANKY  : Spanky's Quest
	0x6819, 1,	// SPIDER  : Spider-Man - X-Men - Arcade's Revenge
	0xA25C, 59,	// SPIDER2 : Spider-Man 2
	0x787C, 59,	// SPIDER3 : Spider-Man 3 - Invasion of The Spider-Slayers
	0xEB00, 1,	// SPY     : Spy vs Spy - Operation Boobytrap
	0x18DB, 44,	// STARNEXT: Star Trek - The Next Generation
			// STARTREK: Star Trek - 25th Anniversary
	0xAE57, 41,	//*STARWAR : Star Wars
	0x727A, 52,	// STARWAR2: Star Wars - The Empire Strikes Back
	0x9387, 1,	// SUPERMAN: Superman
	0x8693, 59,	// SUPOFF  : Super Off Road
	0x9E01, 36,	// SUPRC   : Super R.C. Pro-Am
	0xFA3A, 29,	// SUPSW   : Super Star Wars - Return of the Jedi
	0xCFD1, 57,	// SWAMP   : Swamp Thing
	0x5860, 57,	// SWORD   : The Sword of Hope
	0xDA00, 57,	// SWORD2  : The Sword of Hope II
	0x3E41, 44,	// T2      : T2 - The Arcade Game
	0x7E35, 57,	// TAMAGOT : Tamagotchi
	0xBBE1, 1,	// TARZAN  : Tarzan - Lord of the Jungle
	0xEE21, 29,	// TAZMAN2 : Taz-Mania 2
	0x5713, 60,	// TEENAGE : Teenage Mutant Ninja Turtles - Fall of The Foot Clan
	0x602A, 60,	// TEENAGE2: Teenage Mutant Ninja Turtles II - Back from Sewers
	0x64AE, 60,	// TEENAGE3: Teenage Mutant Ninja Turtles III - Radical Rescue
	0x2947, 9,	//*TENNIS  : Tennis
	0x1768, 8,	// TERMIN2 : Terminator 2 - Judgment Day
	0xA9DC, 17,	//*TETRIS  : Tetris
	0xF6C6, 18,	//*TETRIS2 : Tetris 2
	0xD521, 18,	//*TETRIS2B: Tetris 2 (Rev 1)
	0xBEA9, 13,	//*TETRISA : Tetris Attack
	0x0923, 14,	//*TETRISB : Tetris Blast
	0x18ED, 22,	// TETRISP : Tetris Plus
	0x3E30, 57,	// TINY2M  : Tiny Toon Adventures 2 - Montana's Movie Madness
	0xF64C, 1,	// TINYB   : Tiny Toon Adventures - Babs' Big Break
	0x2417, 57,	// TINYW   : Tiny Toon Adventures - Wacky Sports
	0x6C7A, 56,	// TOMJER  : Tom & Jerry
	0xD546, 56,	// TOMJERF : Tom and Jerry - Frantic Antics!
	0x8CDE, 56,	// TOYSTORY: Toy Story
	0x9C67, 1,	// TRAX    : Trax
	0x7869, 1,	// TUROK   : Turok - Battle of the Bionosaurs
	0xA848, 1,	// TURRIC  : Turrican
	0x2531, 1,	// ULTIMA  : Ultima - Runes of Virtue
	0x8EF4, 1,	// ULTIMA2 : Ultima - Runes of Virtue 2
	0x6DEE, 1,	// UNIV    : Universal Soldier
	0x8823, 28,	// URBSTRI : Urban Strike
	0x7B88, 7,	//*WARIO   : Wario Land - Super Mario Land 3
	0x7BC1, 44,	//*WARIO2  : Wario Land II
	0xB2DD, 1,	// WARIOB  : Wario Blast Featuring Bomberman!
	0xB51B, 26,	// WAVERACE: Wave Race
	0x3DE0, 1,	// WHO     : Who Framed Roger Rabbit
	0x7C3F, 32,	// WWFKING : WWF King of the Ring
	0x33A5, 6,	// WWFRAW  : WWF Raw
	0x8A6D, 5,	// WWFSUP  : WWF Superstars
	0x032F, 5,	// WWFSUP2 : WWF Superstars 2
	0x8CB1, 8,	// WWFWAR  : WWF War Zone
	0x07C6, 29,	// XENON2  : Xenon 2
	0x3E68, 16,	// YOSHIC  : Yoshi's Cookie
	0, 0,
};
