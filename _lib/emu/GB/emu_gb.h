
// ****************************************************************************
//
//                             Game Boy Emulator
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

// CPU frequency:
//   Game Boy (GB): 4.194304 MHz
//   Super Game Boy (SGB): 4.295454 MHz (adapter to play Game Boy cartridges on Super Nintendo console)
//   Game Boy Color (GBC): 4.194304 MHz (default) or 8.388608 MHz (dual-speed mode)
// 1 machine cycle: 1.048576 MHz (NOP is 1 machine cycle)
// 1 clock cycle: 4.194304 MHz (NOP is 4 clock cycles)

// DIV register is incremented at rate 16384 Hz
//   4194304 / 16384 = 256 clock cycles for one increment
#define GB_DIV_CYCLES	256		// pre-divider of DIV register

// serial clock is 8192 Hz
//   4194304 / (8192 / 8) = 4096 clock cycles for sending 1 byte
#define GB_SERIAL_CYCLES	4096	// clock cycles for sending 1 byte

#include "../../../_devices/key.h"
#include "../../inc/lib_fat.h"

// current CPU
extern sX80* GB_Cpu; // used CPU
extern int GB_Pwm; // used PWM controller
extern sFile* GB_ROMFile; // open ROM file
extern int GB_ROMSize; // size of ROM file
extern char* GB_ROMName; // pointer to buffer with ROM file name (can modify extension)
extern int GB_ROMNameLen; // ROM file name length without extension (= index of '.')
extern char GB_ROMNameExt[5]; // saved original ROM name extension

extern volatile u32 GB_Freq; // used frequency
extern volatile u32 GB_RealFreq; // real frequency

#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
extern u32 GB_FreqH, GB_FreqL; // used frequency high and low
extern u32 GB_RealFreqH, GB_RealFreqL; // real frequency high and low
extern u16 GB_ClkDivH, GB_ClkDivL; // clock divider high and low
#endif

// keypad handler
#define GB_KEY14_RIGHT		B0
#define GB_KEY14_LEFT		B1
#define GB_KEY14_UP		B2
#define GB_KEY14_DOWN		B3

#define GB_KEY15_A		B0
#define GB_KEY15_B		B1
#define GB_KEY15_SELECT		B2
#define GB_KEY15_START		B3

extern Bool GB_KeyIsReg;			// key handler is registered
extern volatile u8 GB_Key14;			// pressed keys on port P14 (0=key is pressed)
extern volatile u8 GB_Key15;			// pressed keys on port P15 (0=key is pressed)
extern volatile Bool GB_KeyX;			// key X
extern volatile Bool GB_KeyY;			// key Y
extern volatile Bool GB_KeyA;			// key A
extern volatile Bool GB_KeyB;			// key B

extern Bool GB_KeyMenuReq;			// requirement for game menu (keys Y + UP)

#include "emu_gb_disp.h"	// display
#include "emu_gb_mem.h"		// memory service
#include "emu_gb_menu.h"	// game menu
#include "emu_gb_msg.h"		// message

#define GB_KEY_DELTA	50		// time of key handler, in [ms]

// flush keys
void GB_KeyFlush();

// get key KEY_X, KEY_Y, KEY_A or KEY_B (return NOKEY if no key)
u8 GB_KeyGet();

#define ROM_CRC_ADDR	0x014D

#define GB_EMU_VER	0x100	// emulator version, major and minor byte

// Game Boy support mode
#define GB_MODE_GB	0x00	// GB exclusive
#define GB_MODE_MIX	0x80	// GB/GBC compatible
#define GB_MODE_GBC	0xC0	// GBC exclusive

#define GB_PAL_NUM	4	// GB number of palettes
#define GBC_PAL_NUM	64	// GBC number of palettes

// mode 0: cycle 0..203 (204 cycles): HBLANK
// mode 2: cycle 204..283 (80 cycles): OAM transfer
// mode 3: cycle 284..453 (172 cycles): rendering and transfer
#define GB_LCD_CYCLES	456	// LCD cycles per scanline normal speed (456/4.194304 = 108.72 us)
//#define GB_LCD_CYCLES0 372	// LCD cycles per start mode 0 (start of transfer)
#define GB_LCD_CYCLES2	204	// LCD cycles per start mode 2 (OAM access, 80 cycles)
#define GB_LCD_CYCLES3	284	// LCD cycles per start mode 3 (LCD transfer)

// frame buffer in RGB 5-6-5 pixel format
extern ALIGNED FRAMETYPE FrameBuf[GB_RAM_SIZE/sizeof(FRAMETYPE)];

// Emulator context (this structure should be allocated in FrameBuf - use GB_RAM_PAGE_NUM to specify enough size)
typedef struct {

	// CPU
	sX80	cpu;
// aligned

	// memory mapping
	u8*		vram_map;		// pointer to VRAM bank (8 KB, 1 or 2 banks, address 0x8000-0x9FFF)
	u8*		xram_map;		// pointer to XRAM bank (8 KB, 16 banks, address 0xA000-0xBFFF)
	u8*		wram_map;		// pointer to WRAM bank (4 KB, 1 or 7 banks, address 0xD000-0xDFFF)
	u32		rom_base;		// base offset of ROM switchable bank
// aligned
	u8		mbc;			// type of memory controller (1..5)
	u8		mbc_xram;		// ROM has extended XRAM
	u8		mbc_xram_num;		// number of XRAM banks
	u8		mbc_xram_en;		// 1=extended XRAM is enabled
// aligned
	u8		gbmode;			// GB mode GB_MODE_*
	u8		xram_sel;		// selected extended XRAM bank 8 KB (0..15)
	u8		bank_sel_mode;		// MBC1: bank selection mode 0=ROM, 1=ROM+RAM
	u8		vram_sel;		// selected video VRAM bank 8 KB (0 or 1)
// aligned
	u8		wram_sel;		// selected working WRAM bank 4 KB (1..7)
	u8		frame;			// display frame counter, to skip some frames
	u16		res2;			// ... reserved (set to 0)
// aligned
	u16		mbc_rom_num;		// number of ROM banks 16 KB (2..512)
	u16		rom_sel;		// selected ROM bank 16 KB (1..511)
// aligned
	u8		bg_pal[GB_PAL_NUM];	// GB background palettes
// aligned
	u8		sp_pal[2*GB_PAL_NUM];	// GB sprite palettes
// aligned
	u8		bgpal_id;		// GBC background palette index
	u8		sppal_id;		// GBC sprite palette index
	u8		bgpal_inc;		// GBC background palette increment
	u8		sppal_inc;		// GBC sprite palette increment
// align
	u8		bg_pal2[GBC_PAL_NUM];	// GBC background palettes
	u8		sp_pal2[GBC_PAL_NUM];	// GBC sprite palettes
	u16		fixpal[GBC_PAL_NUM];	// fixed palettes in RGB565 format to draw on screen
// aligned
	u8		dmanotrun;		// 1=DMA not running
	u8		dmamode;		// DMA mode; 1=horizontal blanking DMA transfer, 0=general purpose DMA transfer
	u8		dmasize;		// DMA size (= number of 16-byte lines to transfer)
	u8		freqdbl;		// using double frequency
// aligned
	u16		dmasrc;			// DMA source
	u16		dmadst;			// DMA destination
// aligned
	u32		res3;			// ... reserved (set to 0)
// aligned
	u32		res4;			// ... reserved (set to 0)
// aligned
	u32		res5;			// ... reserved (set to 0)
// aligned

	// drawing frame buffer (160*144 = 23 KB = 6 pages)
	u8		framebuf[GB_FRAMESIZE]; // drawing frame buffer

	// RAM (total up to 176 KB = 44 pages)
	ALIGNED u8	hram[GB_HRAM_SIZE];	// internal HRAM (256 B), base address 0xFF00 (256 B)
	ALIGNED u8	vram[GB_VRAM_SIZE];	// video RAM (8 or 16 KB), base address 0x8000 (8 KB) ... 4 pages
	ALIGNED u8	oam[GB_OAM_BUFSIZE];	// OAM-RAM (256 B), base address 0xFE00 (256 B)
	ALIGNED u8	wram[GB_WRAM_SIZE];	// main working RAM (8 or 32 KB), base address 0xC000 (8 KB), mirror at 0xE000 ... 8 pages
	ALIGNED u8	xram[GB_XRAM_SIZE];	// external expansion working XRAM (128 KB), base address 0xA000 (8 KB) ... 32 pages

	// context save info
	u16		emuver;			// emulator version GB_EMU_VER
	u16		crc;			// context save CRC-16A CCITT (use Crc16AFast())

// ==== end of save context

	// ROM cache RAM (32 KB = 8 pages)
	u8		rom[GB_ROM_PAGENUM];	// ROM cache pages (GB_ROM_PAGEINV = invalid)
	ALIGNED u8	cache[GB_CACHE_SIZE];	// ROM cache RAM
	sGB_Cache	cache_list[GB_CACHE_PAGENUM]; // list of cache pages
} sGBC;

STATIC_ASSERT(sizeof(sGBC) <= sizeof(FrameBuf), "Overflow sGBC size - check GB_CACHE_NUM!");

#define GBC ((sGBC*)FrameBuf)	// emulator context

// start emulation (emulator context GBC will be used; returns 0 on error)
//  file ... open ROM file
//  size ... size of ROM file
//  name ... pointer to buffer with ROM file name (can modify extension)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (GB: use 4 MHz, GBC: use 8 MHz)
u32 GB_Start(sFile* file, int size, char* name, int pwm, u32 freq);

// stop/pause emulation
void GB_Stop();

// continue emulation
void GB_Cont();

// check if emulation is running
INLINE Bool GB_IsRunning() { return X80_IsRunning(); }
