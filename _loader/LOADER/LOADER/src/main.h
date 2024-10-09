
// ****************************************************************************
//
//                                 Main code
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

#ifndef _MAIN_H
#define _MAIN_H

// custom frame buffer - used as buffer to load UF2 program into RAM
#if RP2040
#define CUSTOM_FRAMEBUF_SIZE (220000/sizeof(FRAMETYPE))
#else
#define CUSTOM_FRAMEBUF_SIZE (450000/sizeof(FRAMETYPE))
#endif

// text of file list
//#define FONTW		8	// font width
//#define FONTH		16	// font height
#define TEXTW		(WIDTH/FONTW) // number of characters per row (=320/8=40)
#define TEXTH		(HEIGHT/FONTH) // number of rows (=240/16=15)

// text of preview (preview field width = 20 characters, height = 15 rows)
#define FONTW2		6	// font width
#define FONTH2		8	// font height
#define TEXTW2		(WIDTH/FONTW2) // number of characters per row (=320/6=53)
#define TEXTH2		(HEIGHT/FONTH2) // number of rows (=240/8=30)

// files
#define PATHMAX		240	// max. length of path (it must be <= 256 with CRC and magic)
#define MAXFILES	1024	// max files in one directory
#define NAMELEN		9	// max. length of filename without extension and with terminating 0
#define FILEROWS	(TEXTH-2) // rows of files (=13)
#define FILECOLW	20	// width of file panel

// file entry descriptor (10 bytes)
//#define ATTR_RO	B0	// Read only
//#define ATTR_HID	B1	// Hidden
//#define ATTR_SYS	B2	// System
//#define ATTR_VOL	B3	// Volume label
//#define ATTR_DIR	B4	// Directory
//#define ATTR_ARCH	B5	// Archive
#define ATTR_TXT	B6	// internal: text file TXT present
#define ATTR_BMP	B7	// internal: bitmap indicator BMP present
#define ATTR_MASK	0x3F	// mask of valid bits

typedef struct { // 10 bytes
	u8	attr;		// attributes ATTR_*
	u8	len;		// file name length
	char	name[8];	// file name (without extension and without terminating zero)
} sFileDesc;

// colors
#define COL_TITLEFG	COL_BLACK	// title foreground color
#define COL_TITLEBG	COL_WHITE	// title background color
#define COL_FILEFG	COL_CYAN	// file foreground color
#define COL_FILEBG	COL_BLUE	// file background color
#define COL_DIRFG	COL_WHITE	// directory foreground color
#define COL_DIRBG	COL_BLUE	// directory background color
#define COL_CURFG	COL_BLACK	// cursor coreground color
#define COL_CURBG	COL_CYAN	// cursor background color
#define COL_INFOFG	COL_GREEN	// info text foreground color
#define COL_INFOBG	COL_BLUE	// info text background color
#define COL_TEXTBG	COL_BLACK	// text background color
#define COL_TEXTFG	COL_GRAY	// text foreground color
#define COL_BIGINFOFG	COL_YELLOW	// big info text foreground color
#define COL_BIGINFOBG	COL_BLACK	// big info text background color
#define COL_BIGERRFG	COL_YELLOW	// big error foreground color
#define COL_BIGERRBG	COL_RED		// big error background color

// BMP file header (size 70 bytes)
#pragma pack(push,1)
typedef struct {
	// BMP file header (size 14 bytes)
	u16	bfType;			// 0x00: magic, 'B' 'M' = 0x4D42
	u32	bfSize;			// 0x02: file size, aligned to DWORD = 70 + 320*240*2 + 2 = 153672 = 0x25848
	u16	bfReserved1;		// 0x06: = 0
	u16	bfReserved2;		// 0x08: = 0
	u32	bfOffBits;		// 0x0A: offset of data bits after file header = 70 (0x46)
	// BMP info header (size 40 bytes)
	u32	biSize;			// 0x0E: size of this info header = 56 (0x38)
	s32	biWidth;		// 0x12: width = 320 (0x140)
	s32	biHeight;		// 0x16: height, negate if flip row order = -240 (0xFFFFFF10)
	u16	biPlanes;		// 0x1A: planes = 1
	u16	biBitCount;		// 0x1C: number of bits per pixel = 16
	u32	biCompression;		// 0x1E: compression = 3 (BI_BITFIELDS)
	u32	biSizeImage;		// 0x22: size of data of image + aligned file = 320*240*2 + 2 = 153602 (0x25802)
	s32	biXPelsPerMeter;	// 0x26: X pels per meter = 2834 (= 0xB12)
	s32	biYPelsPerMeter;	// 0x2A: Y pels per meter = 2834 (= 0xB12)
	u32	biClrUsed;		// 0x2E: number of user colors (0 = all)
	u32	biClrImportant;		// 0x32: number of important colors (0 = all)
	// BMP color bit mask (size 16 bytes)
	u32	biRedMask;		// 0x36: red mask = 0x0000F800
	u32	biGreenMask;		// 0x3A: green mask = 0x000007E0
	u32	biBlueMask;		// 0x3E: blue mask = 0x0000001F
	u32	biAlphaMask;		// 0x42: alpha mask = 0x00000000
					// 0x46
} sBmp;
#pragma pack(pop)

// UF2 sector (size 512 bytes)
#define UF2_MAGIC_START0		0x0A324655
#define UF2_MAGIC_START1		0x9E5D5157
#define UF2_MAGIC_END			0x0AB16F30

#define UF2_FLAG_NOT_MAIN_FLASH		0x00000001
#define UF2_FLAG_FILE_CONTAINER		0x00001000
#define UF2_FLAG_FAMILY_ID_PRESENT	0x00002000
#define UF2_FLAG_MD5_PRESENT		0x00004000

#define RP2040_FAMILY_ID		0xe48bff56
#define RP2350_ARM_S_FAMILY_ID		0xe48bff59
#define RP2350_RISCV_FAMILY_ID		0xe48bff5a
#define RP2350_ARM_NS_FAMILY_ID		0xe48bff5b

typedef struct {
	// header size 32 bytes
	u32	magic_start0;		// 0x000: UF2_MAGIC_START0 (0x0A324655)
	u32	magic_start1;		// 0x004: UF2_MAGIC_START1 (0x9E5D5157)
	u32	flags;			// 0x008: UF2_FLAG_FAMILY_ID_PRESENT (0x00002000)
	u32	target_addr;		// 0x00C: address flash 0x10000000, 0x10000100,... or RAM 0x20000000, 0x20000100,...
	u32	payload_size;		// 0x010: data size 0x00000100 (256)
	u32	block_no;		// 0x014: block number 0, 1,...
	u32	num_blocks;		// 0x018: total number of blocks
	u32	file_size;		// 0x01C: familyID *FAMILY_ID

	// sector data
	u8	data[476];

	// sector end magic
	u32	magic_end;		// 0x1FC: UF2_MAGIC_END (0x0AB16F30)
} sUf2;

STATIC_ASSERT(sizeof(sUf2) == 512, "sUf2 not sector sized!");

#endif // _MAIN_H
