
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

#define DEB_FPS		0	// 1=debug display FPS
#define DEB_WRITECRAM	0	// 1=debug display write bytes into CRAM (in game menu)

// - Set this value to max. value of free RAM (every cache page is 4 KB)
#define GB_CACHERAM_NUM		3			// number of cache pages in RAM (minimum 1!)

#define GB_CRAMBATCH	3000	// number of bytes to auto save RAM to SAV file

// home path
extern char HomePath[];
extern int HomePathLen;

// GameBoy context
extern struct gb_s gbContext;

#if DEB_WRITECRAM	// 1=debug display write bytes into CRAM (in game menu)
extern u32 CRamWriteNum;	// number of writes to CRAM
#endif

// game title CRC (CRC16A of game title, game code, support code and maker code, address 0x0134..0x0145)
extern u16 TitleCrc;

// ROM cache pages ('cartridge RAM valid' requires 4 KB pages)
#define GB_CACHE_SHIFT		12			// number of shifts of cache page
#define GB_CACHE_SIZE		BIT(GB_CACHE_SHIFT)	// size of cache page (= 4096 = 0x1000)
#define GB_CACHE_MASK		(GB_CACHE_SIZE-1)	// mask of cache page (to keep page bits; = 0x0FFF)
#define GB_CACHE_INVMASK	(~GB_CACHE_MASK)	// inverted mask of cache page (to clear page bits; = 0xFFFFF000)
#define GB_ROMMAX		(8*1024*1024)		// max. ROM file size (8 MB)
#define GB_ROMCACHEMAX		(GB_ROMMAX/GB_CACHE_SIZE) // max. number of ROM cache pages (= 2048)

#define GB_CACHECRAM_NUM	(CRAM_SIZE/GB_CACHE_SIZE) // number of cache pages in CRAM (= 32)
#define GB_CACHE_NUM		(GB_CACHERAM_NUM+GB_CACHECRAM_NUM) // total number of cache pages (= 42)
#define GB_CACHEINX_CRAM	GB_CACHERAM_NUM		// start index of cache page in CRAM
#define GB_CACHEINX_INV		255			// invalid cache page index

// cache page desctriptor
#define GB_CACHETYPE_FREE	0	// cache page is free to use
#define GB_CACHETYPE_VALID	1	// valid data in cache
#define GB_CACHETYPE_FORB	2	// forbidden page (used by CRAM)

typedef struct {
	u32	last;		// time of last access in [us]
	u16	rominx;		// index of page in ROM table
	u8	type;		// type GB_CACHETYE_*
} sGB_Cache;

extern sGB_Cache GB_CacheDesc[GB_CACHE_NUM];	// cache page descriptors
extern u8 GB_CacheROM[GB_ROMCACHEMAX];		// ROM cache pages

extern sFile GB_ROMFile; // open ROM file
extern Bool GB_ReqExit; // request to exit program

// initialize system clock
void GB_InitSysClk();

// restore system clock
void GB_TermSysClk();

// initialize core 1
void GB_InitCore1();

// terminate core 1
void GB_TermCore1();

// initialize emulator
void GB_Setup();

// set palette
void gbSetPal(u8 info);

// program
extern char ProgName[];		// program name (max. 8 characters upper case)
extern int ProgNameLen;		// length of program name (1 to 8 characters)
extern char ProgExt[];		// program name extension (max. 3 characters upper case)
extern int ProgExtLen;		// length of program name extension (1 to 3 characters)
extern int gameRomLen;		// length of compressed ROM image
extern int gameRomOrig;		// original length of ROM image (without compression)
extern int gameRomPages;	// number of ROM pages
extern int gameFlashPages;	// number of Flash pages (ROM in flash)
extern u16 gameRomSizeList[];	// list of sizes of valid data of ROM pages
extern u32 gameRomOffList[];	// list of offsets of ROM pages
extern u8 gameRomStuffList[];	// list of stuff byte of rest of ROM pages
extern u8 gameRomLastList[];	// list of last byte of of ROM pages
extern const u8 gameRom[];	// ROM image (compressed)

#endif // _MAIN_H
