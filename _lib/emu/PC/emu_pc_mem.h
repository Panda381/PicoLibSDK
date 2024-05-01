
// ****************************************************************************
//
//                       IBM PC Emulator - Memory service
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

// >>> Keep tables and functions in RAM (without "const") for faster access.

// Memory:
//  0x00000 - 0x003FF: 1 KB vector table (FAR jump addresses) - at default pointing to interrupt traps, addresses 0x0060:0x00nn
//  0x00400 - 0x005FF: 512 bytes BIOS data
//  0x00600 - 0x006FF: 256 bytes Interrupt traps (instructions IRET) ... 0x0060:0x0000 is MS-DOS 1.x load address
//  0x00700 - 0x00AFF: 600 bytes emulator menu screen 20 x 15 characters with attributes (font 8x8), 1024 B memory  ... 0x0070:0x0000 is MS-DOS 2+ load address
//  0x00B00 - 0x00EFF: 600 bytes user debug screen 20 x 15 characters with attributes (font 8x8), 1024 B memory
//  0x00F00 - 0x00FFF: 256 bytes PSP Program Segment Prefix of user COM program (0x00F0:0x0000 - 0x00F0:0x00FF)
//  0x01000: start of user COM program, 0x00F0:0x0100 (CS=DS=SS=ES=0x00F0, IP=0x0100, SP=0xFFFE), size of COM max. 0xFE00 = 65024 bytes)
//  0x01000 - 0x9FFFF: space for EXE program, size 636 KB
//  0xA0000 - 0xC7FFF: video memory, size 160 KB
//  0xC8000 - 0xFBFFF: ... unused (can be used for additional data for user program, size 208 KB, max. program total size = 848 KB)
//  0xFC000 - 0xFCFFF: ROM font 8x8
//  0xFD000 - 0xFDFFF: ROM font 8x14
//  0xFE000 - 0xFEFFF: ROM font 8x16
//  0xFF000 - 0xFFFFF: ROM BIOS ID

// Host must define:
//  #define PC_RAM_PAGE_NUM	50		// number of RAM pages
//  #define PC_RAM_BASE		((u8*)FrameBuf)	// RAM base address (declared as u8*, must be ALIGNED)

#define PC_PAGE_SHIFT	12			// number of shifts of memory page
#define PC_PAGE_SIZE	BIT(PC_PAGE_SHIFT)	// size of memory page (= 4096 = 0x1000)
#define PC_PAGE_MASK	(PC_PAGE_SIZE-1)	// mask of memory page (to keep page bits; = 0x0FFF)
#define PC_PAGE_INVMASK	(~PC_PAGE_MASK)		// inverted mask of memory page (to clear page bits; = 0xFFFFF000)
#define PC_PAGE_ALIGNUP(n) (((n) + PC_PAGE_SIZE - 1) & PC_PAGE_INVMASK) // align size up to memory page
#define PC_PAGE_ALIGNDN(n) ((n) & PC_PAGE_INVMASK) // align size down to memory page

#define PC_RAM_SIZE		(PC_RAM_PAGE_NUM*PC_PAGE_SIZE) // RAM size in number of bytes = about 205 KB (PC_RAM_PAGE_NUM = about 50)
#define PC_RAM_PAGE_MAX 	64		// max. number of RAM pages (= 256 KB)

#define PC_ROM_SIZE_MAX		0x9F000					// max. ROM size (= 651264 = 636 KB = max. DOS memory without first 4 KB page)
#define PC_ROM_PAGE_MAX		(PC_ROM_SIZE_MAX/PC_PAGE_SIZE)		// max. number of ROM pages (= 159)

#define PC_EMUL_VRAM_SEG	0x0070					// segment with emulator menu screen
#define PC_EMUL_VRAM		(&PC_RAM_BASE[PC_EMUL_VRAM_SEG << 4])	// emulator menu screen

#define PC_DEBUG_VRAM_SEG	0x00B0					// segment with user debug screen
#define PC_DEBUG_VRAM		(&PC_RAM_BASE[PC_DEBUG_VRAM_SEG << 4])	// user debug screen

#define PC_TRAP_ADDR		0x00600					// start of interrupt traps

// pages
#define PC_PAGE_NUM		256					// total number of memory pages

#define PC_BIOS_PAGE_NUM	4					// number of BIOS ROM pages (= Font8, Font14, Font16, BIOS ID)
#define PC_BIOS_PAGE_LAST	255					// last BIOS ROM page
#define PC_BIOS_PAGE_FIRST	(PC_BIOS_PAGE_LAST-PC_BIOS_PAGE_NUM+1)	// first BIOS ROM page (= 252)

// ... BIOS pages must be equal to their address
#define PC_BIOS_PAGE_ID		255					// BIOS ROM page with ID (virtual address 0xFF000-0xFFFFF)
#define PC_BIOS_PAGE_FONT16	254					// BIOS ROM page with font16 (virtual address 0xFE000-0xFEFFF)
#define PC_BIOS_FONT16_SIZE	(256*16)				// size of font16
#define PC_BIOS_PAGE_FONT14	253					// BIOS ROM page with font14 (virtual address 0xFD000-0xFDFFF)
#define PC_BIOS_FONT14_SIZE	(256*14)				// size of font14
#define PC_BIOS_PAGE_FONT8	252					// BIOS ROM page with font8 (virtual address 0xFC000-0xFCFFF)
#define PC_BIOS_FONT8_SIZE	(256*8)					// size of font8

#define PC_ROM_PAGE_LAST	250					// last ROM page
#define PC_ROM_PAGE_FIRST	(PC_ROM_PAGE_LAST-PC_ROM_PAGE_MAX+1)	// first ROM page (= 92)

#define PC_RAM_PAGE_FIRST	0					// first RAM page (= vectors and traps)
#define PC_RAM_PAGE_LAST	(PC_RAM_PAGE_FIRST+PC_RAM_PAGE_NUM-1) 	// last RAM page (max. 63)

#define PC_INV_PAGE		80					// invalid page (not allocated)

#define PC_PROG_ADDR	0x01000			// virtual address to load program (start of 2nd memory page), virtual address 0x00F0:0x0100 (0x00F0:0x0000 contains PSP)
#define PC_PROG_SEG	(PC_PROG_ADDR >> 4)	// segment to load program
#define PC_PSP_SEG	(PC_PROG_SEG - 16)	// segment with PSP
#define PC_COM_SIZE_MAX	0xFE00			// max. size of COM program (without PSP and without stack)
#define PC_PROG_MAX	PC_ROM_SIZE_MAX		// max. size of loaded program

// BIOS ID header (16 bytes at address 0xFFFF0, page 255)
extern const u8 PC_BiosID[16];

// base address of program in flash memory (must be ALIGNED)
extern int PC_RomSize;

// map of memory pages (PC_INV_PAGE = invalid page)
extern u8 PC_MemMap[PC_PAGE_NUM];

// index of next free RAM page
extern u8 PC_NextPageRAM;

// segment of end of DOS allocated memory
extern u16 PC_AllocEnd;

// bottom video-RAM allocated page (PC_RAM_PAGE_NUM = no video-RAM allocated)
extern u8 PC_VmemPageRAM;

// add program ROM to memory map
//  size ... size of program (will be aligned up to memory page; must not exceed end of virtual space)
void PC_MemMapSetupRom(int size);

// read memory (callback from emulator)
u8 FASTCODE NOFLASH(PC_GetMem)(u32 addr);

// write memory (callback from emulator)
void FASTCODE NOFLASH(PC_SetMem)(u32 addr, u8 data);
