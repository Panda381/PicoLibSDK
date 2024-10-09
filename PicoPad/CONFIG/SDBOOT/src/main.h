
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

// text of file list
//#define FONTW		8	// font width
//#define FONTH		16	// font height
#define TEXTW		(WIDTH/FONTW) // number of characters per row (=320/8=40)
#define TEXTH		(HEIGHT/FONTH) // number of rows (=240/16=15)

// files
#define PATHMAX		240	// max. length of path (it must be <= 256 with CRC and magic)
#define MAXFILES	256	// max files in one directory
#define NAMELEN		9	// max. length of filename without extension and with terminating 0
#define FILEROW1	0	// first file
#define FILEROWS	(TEXTH-2-FILEROW1) // rows of files (=11)
#define FILECOLW	20	// width of file panel

// file entry descriptor (14 bytes)
//#define ATTR_RO	B0	// Read only
//#define ATTR_HID	B1	// Hidden
//#define ATTR_SYS	B2	// System
//#define ATTR_VOL	B3	// Volume label
//#define ATTR_DIR	B4	// Directory
//#define ATTR_ARCH	B5	// Archive
#define ATTR_MASK	0x3F	// mask of valid bits

typedef struct { // 14 bytes
	u32	size;		// file size
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
