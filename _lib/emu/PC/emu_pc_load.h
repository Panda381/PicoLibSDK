
// ****************************************************************************
//
//                       IBM PC Emulator - Load program
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

// EXE file format
//  - loading from address PSP + 256
//  - on start DS and ES points to PSP
typedef struct {
	char	sign[2];	// 0x00 (2): signature - characters 'M', 'Z'
	u16	lastpage;	// 0x02 (2): number of bytes in last page (0=no partial page)
	u16	pages;		// 0x04 (2): number of 512-byte pages (including last partial page if not 0) (EXE datasize = pages*512; if (lastpage) datasize -= (512 - lastpage))
	u16	relnum;		// 0x06 (2): number of entries in relocation table
	u16	header;		// 0x08 (2): header size in number of 16-byte paragraphs (data_start = header*16)
	u16	minmem;		// 0x0A (2): minimum required memory in number of 16-byte paragraphs (excluding PSP and program image)
	u16	maxmem;		// 0x0C (2): maximum required memory in number of 16-byte paragraphs (required or smaller block will be allocated)
	u16	init_ss;	// 0x0E (2): relocatable segment address for SS
	u16	init_sp;	// 0x10 (2): initial value for SP
	u16	checksum;	// 0x12 (2): checksum - when added to sum of all other words in the file, result should be zero
	u16	init_ip;	// 0x14 (2): initial value for IP
	u16	init_cs;	// 0x16 (2): relocatable segment address for CS
	u16	reloff;		// 0x18 (2): offset to relocation table
	u16	overlay;	// 0x1A (2): overlay index (0=main executable)

// Entry of relocation table:
//	u16	off;		// offset of relocation within segment
//	u16	seg;		// segment of relocation, relative to load segment

} sExeFile;

// EXE file header
extern sExeFile PC_ExeFile;
extern Bool PC_ExeFileOK; // program is in EXE format (or COM otherwise)

// load program to Flash memory (loads program and adds program to memory pages; returns False on error)
//  file ... open file
//  size ... file size in bytes
//  PC_ROM_BASE ... base address to load program to the flash (must be aligned to 4 KB)
//  PC_RAM_BASE ... base RAM address to load temporary data
//  PC_RAM_PAGE_NUM*PC_PAGE_SIZE ... max. size of RAM
Bool PC_LoadProg(sFile* file, int size);

// load program configuration into RAM memory (returns False on error - can use parser, but will return default values)
//  filename ... filename can be in the same RAM as later file content
//  PC_RAM_BASE ... base RAM address to load temporary data
//  PC_RAM_PAGE_NUM*PC_PAGE_SIZE ... max. size of RAM
Bool PC_LoadCfg(const char* filename);

// get config parameter
int PC_GetCfg(const char* name, int def);
