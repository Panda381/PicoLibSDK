
// ****************************************************************************
//
//                     Game Boy Emulator - Memory service
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

/*
Memory requirements, GB:
------------------------
working WRAM: 8 KB
video VRAM: 8 KB
cartridge XRAM ... up to 128 KB
ports RAM ... 512 bytes

cartridge ROM ... up to 1 MB

Total RAM: 8+8+128=144 KB

Memory requirements, GBC:
-------------------------
working WRAM: 32 KB
video VRAM: 16 KB
cartridge XRAM ... up to 128 KB
ports RAM ... 512 bytes

cartridge ROM ... up to 8 MB

Total RAM: 32+16+128=176 KB

If using no internal video RAM, remains 24 KB of RAM (6 pages)


Game Boy memory map:
--------------------
0x0000-0x00FF ... RST and interrupt services (256 B)
0x0100-0x014F ... ROM area for storing data such as name of the game (80 B)
0x0150-0x7FFF ... user program area (almost 32 KB)
  0x0000-0x3FFF ... ROM bank 0 (16 KB)
  0x4000-0x7FFF ... ROM switchable 16 KB bank 1..15 (256 KB, MBC2), 1..31 (512 KB, MBC1), 1..127 (2 MB, MBC1 or MBC3), 1...511 (8 MB, MBC5)

0x8000-0x9FFF ... VRAM (GB: 8 KB, GBC: 16 KB with 8 KB bank switching)
  0x8000-0x97FF ... character data (6 KB; 1 byte lower dot data + 1 byte upper dot data;
			GBC: switchable bank 0 and bank 1)
	0x8000-0x87FF ... OBJ character data 0x00-0x7F
	0x8800-0x8FFF ... OBJ and GB character data 0x80-0xFF
	0x9000-0x97FF ... BG character data 0x00-90x7F

  1 character: 8x8 pixels, 1 pixel is 2 bits (4 shadow levels) on GB mono (16 B/char), or 4 bits (16 levels) on GB color
	18h, 18h, 3ch, 3ch, 66h, 66h, 66h, 66h, 7eh, 7eh, 66h, 66h, 24h, 24h, 0, 0

	00011000b, 00011000b,
	00111100b, 00111100b,
	01100110b, 01100110b,
	01100110b, 01100110b,
	01111110b, 01111110b,
	01100110b, 01100110b,
	00100100b, 00100100b,
	00000000b, 00000000b,

  0x9800-0x9BFF ... display data 1 (32x32 characters; 1 KB; 1 byte character, GBC: + 1 byte attributes)
  0x9C00-0x9FFF ... display data 2 (32x32 characters; 1 KB; 1 byte character, GBC: + 1 byte attributes)
			attribute (GBC only): B0-B2: color palette, B3=character bank, B5=1 flip left/right,
			B6=1 flip up/down, B7=1 highest priority to BG (1=BG tile in from of sprites)

0xA000-0xBFFF ... external expansion working XRAM (8 KB area, up to 16 banks)

0xC000-0xDFFF ... main working WRAM (GB: 8 KB, GBC: 32 KB with 4 KB bank switching)
  1) 0xC000-0xCFFF fixed bank 0
  2) 0xD000-0xDFFF switched banks 1 to 7

0xE000-0xFDFF ... echo of main working WRAM (undocumented, should not be used)

0xFE00-0xFFFF ... CPU internal RAM
  0xFE00-0xFE9F ... OAM-RAM (160 bytes, holds display data for 40 objects x 32 bits)
     - it can only be accessed during Vblank; color 0 is transparent
     4 bytes per object: Y coordinate, X coordinate, character code in 0x8000-0x8FFF (GBC: 3 lower bits = color palette),
           attribute (B0..B2 color palette of GBC, B3 character bank GBC, B4 palette GB,
           B5 horiz flip, B6 vertical flip, B7 priority 1=BG to hide sprite)
  0xFF00-0xFF7F and 0xFFFF ... registers and flags (129 bytes)
  0xFF80-0xFFFE ... fast work high HRAM and stack (127 bytes)

*/

// Host must define:
//  #define GB_RAM_PAGE_NUM	58		// number of RAM pages
//  #define GB_RAM_BASE		((u8*)FrameBuf)	// RAM base address (declared as u8*, must be ALIGNED)

#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
// Game Boy Color
#define GB_BOOT_SIZE	(2*1024)	// bootstram ROM size (2 KB)
#define GB_VRAM_SIZE	(16*1024)	// video RAM size (16 KB)
#define GB_WRAM_SIZE	(32*1024)	// main working RAM size (32 KB)
#else
// Game Boy Mono
#define GB_BOOT_SIZE	256		// bootstram ROM size (256 B)
#define GB_VRAM_SIZE	(8*1024)	// video RAM size (8 KB)
#define GB_WRAM_SIZE	(8*1024)	// main working RAM size (8 KB)
#endif

#define GB_XRAM_SIZE	(128*1024)	// external expansion working XRAM (128 KB)
#define GB_OAM_SIZE	0x00A0		// OAM-RAM size (objects, 160 B)
#define GB_OAM_BUFSIZE	0x0100		// OAM-RAM buffer size (objects, 160 B + some RAM)
#define GB_HRAM_SIZE	0x0100		// HRAM size (registers; 256 B)

// memory base addresses
#define GB_ROM0_BASE	0x0000		// ROM base address, bank 0 (16 KB, address 0x0000..0x3F00)
#define GB_ROMN_BASE	0x4000		// ROM base address, switchable bank (16 KB, address 0x4000..0x7FFF)
#define GB_VRAM_BASE	0x8000		// VRAM base address (8 KB, address 0x8000..0x9FFF)
#define GB_XRAM_BASE	0xA000		// external expansion XRAM base address (8 KB, address 0xA000..0xBFFF)
#define GB_WRAM0_BASE	0xC000		// main working RAM base address, bank 0 (4 KB, address 0xC000..0xCFFF)
#define GB_WRAMN_BASE	0xD000		// main working RAM base address, switchable bank 1..7 (4 KB, address 0xD000..0xDFFF)
#define GB_ECHO_BASE	0xE000		// echo of main working RAM (8 KB, address 0xE000..0xFDFF)
#define GB_OAM_BASE	0xFE00		// object OAM-RAM base address (160 bytes + some RAM, address 0xFE00..0xFE9F or 0xFEFF)
#define GB_HRAM_BASE	0xFF00		// high HRAM base address (256 bytes, address 0xFF00..0xFFFF)

// bank size
#define GB_ROM_BANK_SIZE	0x4000	// ROM bank size (16 KB)
#define GB_WRAM_BANK_SIZE	0x1000	// working RAM bank size (4 KB)
#define GB_XRAM_BANK_SIZE	0x2000	// external expansion XRAM bank size (8 KB)
#define GB_VRAM_BANK_SIZE	0x2000	// video VRAM bank size (8 KB)

// frame buffer
#define GB_WIDTH	160	// LCD screen width
#define GB_HEIGHT	144	// LCD screen height
#define GB_VSYNC	10	// LCD screen vertical SYNC lines
#define GB_LINES	(GB_HEIGHT+GB_VSYNC) // LCD screen total height, including VSYNC
#define GB_FRAMESIZE	(GB_WIDTH*GB_HEIGHT) // size of frame buffer

// memory pages
#define GB_PAGE_SHIFT		12			// number of shifts of memory page
#define GB_PAGE_SIZE		BIT(GB_PAGE_SHIFT)	// size of memory page (= 4096 = 0x1000)
#define GB_PAGE_MASK		(GB_PAGE_SIZE-1)	// mask of memory page (to keep page bits; = 0x0FFF)
#define GB_PAGE_INVMASK		(~GB_PAGE_MASK)		// inverted mask of memory page (to clear page bits; = 0xFFFFF000)
#define GB_PAGE_ALIGNUP(n) 	(((n) + GB_PAGE_SIZE - 1) & GB_PAGE_INVMASK) // align size up to memory page
#define GB_PAGE_ALIGNDN(n) 	((n) & GB_PAGE_INVMASK) // align size down to memory page

#define GB_RAM_SIZE		(GB_RAM_PAGE_NUM*GB_PAGE_SIZE) // RAM size in number of bytes

#define GB_CACHE_PAGENUM	(GB_RAM_PAGE_NUM - (GB_VRAM_SIZE+GB_WRAM_SIZE+GB_XRAM_SIZE+GB_FRAMESIZE)/GB_PAGE_SIZE - 1) // ROM cache pages (about 13 pages)
#define GB_CACHE_SIZE		(GB_CACHE_PAGENUM*GB_PAGE_SIZE) // cache RAM size (about 53 KB)
#define GB_ROM_PAGENUM		(0x8000/GB_PAGE_SIZE)	// number of ROM pages (= 8)

// RAM cache descriptor
#define GB_CACHE_INV	((u32)-1)	// flag - cache is invalid
#define GB_ROM_PAGEINV	255		// flag - ROM page is invalid
typedef struct {
	u32	off;			// source offset in ROM image (GB_CACHE_INV = cache is invalid)
	u32	last;			// time of last access in [us]
} sGB_Cache;

// ports in HRAM (offset from 0xFE00)
#define GB_IO_P1	0x00		// (default 0) port mode
					//   B0..B3: input ports P10..P13 with pull-ups (keys P10=Right+A,
					//              P11=Left+B, P12=Up+Select, P13=Down+Start)
					//   B4,B5: output ports P14,P15 (keys P14=Right+Left+Up+Down,
					//              P15=A+B+Select+Start)
					//   Negative edge on P10..P13 raises interrupt request 4 X80_IF_KEYPAD)

#define GB_IO_SB	0x01		// serial cable transfer data (transfer is 8-bit, sync by clock, no start/stop)

#define GB_IO_SC	0x02		// (default 0) serial cable transfer control
					//   B0: 1=use internal clock, 0=use external clock
					//   B1: 1=select 256 kHz or 512 kHz clock (only GBC), 0=select 8 kHz or 16 kHz
					//       (GB: this bit is always 1, but speed is always 8 kHz)
					//   B7: 1=start serial transfer, 0=no serial transfer (cleared by hardware)

#define GB_IO_DIV	0x04		// divide register, upper 8 bits of 16-bit counter, input is 4194304 Hz
					//   B0=f/2^9 (8192 Hz), ... B7=f/2^16 (64 Hz)
					//   Any write will reset counter to 0

#define GB_IO_TIMA	0x05		// (default 0) time counter, lower 8 bits of 16-bit counter,
					//   input is 4194304 Hz, period is 4194304/256 = 16384 Hz

#define GB_IO_TMA	0x06		// timer modulo register, generates interrupt X80_IF_TIMER when TIMA overflows
					//   and loads value of TMA to TIMA counter

#define GB_IO_TAC	0x07		// (default 0) timer control register for TIMA timer
					//    B0,B1: input clock select 0=f/2^10 (4096 Hz), 1=f/2^4 (262144 Hz),
					//           2=f/f^6 (65536 Hz), 3=f/2^8 (16384 Hz)
					//    B2: 1=start timer, 0=stop timer

#define GB_IO_IF	0x0F		// interrupt flag request (1=interrupt occurs) (cleared by hardware)
					//    B0: vertical blanking (jump to 0x0040)
					//    B1: LCDC status (jump to 0x0048)
					//    B2: timer overflow (jump to 0x0050)
					//    B3: serial I/O transfer completion (jump to 0x0058)
					//    B4: P10-P13 terminal negative edge (jump to 0x0060)

#define GB_IO_NR	0x10		// sound base

#define GB_IO_NR10	0x10		// sound 1 mode register (rectangle waveform)
					//    B0-B2: sweep shift number 0..7
					//    B3: sweep 0=increase, 1=decrease
					//    B4-B6: sweep time

#define GB_IO_WAVE	0x30		// waveform RAM base

#define GB_IO_LCDC	0x40		// (default 0x83 = BG/OBJ ON, LCDC operation)
					//    B0: 1=background BG display on (only GB; GBC is always on)
					//    B1: 1=sprites OBJ is on
					//    B2: sprite size 1=8x16 dots, 0=8x8 dots
					//    B3: background code area 1=0x9C00-0x9FFF, 0=0x9800-0x9BFF
					//    B4: character data 1=0x8000-0x8FFF, 0=0x8800-0x97FF
					//    B5: 1=windowing on
					//    B6: window code are selection 1=0x9C00-0x9FFF, 0=0x9800-0x9BFF
					//    B7: LCDC controller operation stop 1=LCDC on, 0=LCDC off

#define GB_IO_STAT	0x41		// LCD status
					//    B0,B1: mode 0=HSYNC, 1=VSYNC, 2=searching OAM RAM, 3=transfering data to LCD
					//    B2: match 1= LYC == LCDC LY, 0= LYC != LCDC LY
					//    B3: HSYNC status interrupt
					//    B4: VSYNC status interrupt
					//    B5: searching OAM interrupt
					//    B6: LYC==LY status interrupt

#define GB_IO_SCY	0x42		// (default 0) scroll background Y 0-255

#define GB_IO_SCX	0x43		// (default 0) scroll background X 0-255

#define GB_IO_LY	0x44		// LCDC Y-coordinate, which line of data is currently being
					//     transferred to LCD, 0-153 (144-153 vertical blanking)

#define GB_IO_LYC	0x45		// (default 0) LY compare, if LYC == LY, sets match flag of STAT register

#define GB_IO_DMA	0x46		// DMA transfer and starting address

#define GB_IO_BGP	0x47		// BG palette data; B0,B1=data for dot data 0, ... B6,B7=data for dot data 3

#define GB_IO_OBP0	0x48		// OBJ palette data 0; B0,B1=data for dot data 0, ... B6,B7=data for dot data 3

#define GB_IO_OBP1	0x49		// OBJ palette data 1; B0,B1=data for dot data 0, ... B6,B7=data for dot data 3

#define GB_IO_WY	0x4A		// (default 0) window Y coordinate 0..143 (from top edge)

#define GB_IO_WX	0x4B		// (default 0) window X coordinate 7..166 (from left edge)

#define GB_IO_KEY1	0x4D		// only GBC, set CPU speed
					//    B0: 1=enable speed switching
					//    B7: (read only) indicating current speed: 0=normal speed, 1=double speed
					//          - set bit B0
					//          - perform STOP instruction, speed changes
					//          - bit B0 is auto cleared by hardware
					//          - read B7 to get current speed

#define GB_IO_VBK	0x4F		// only GBC, 0 or 1: VRAM 0x8000-0x9FFF switch bank 0 or 1

#define GB_IO_BANK	0x50		// 0=select boot ROM

#define GB_IO_HDMA1	0x51		// only GBC, source address HIGH (0x00-0x7F, 0xA0-0xDF)

#define GB_IO_HDMA2	0x52		// only GBC, source address LOW (multiply of 16 bytes)

#define GB_IO_HDMA3	0x53		// only GBC, destination address HIGH (0x00-0x1F)

#define GB_IO_HDMA4	0x54		// only GBC, destination address LOW (multiply of 16 bytes)

#define GB_IO_HDMA5	0x55		// only GBC, transfer start and number of bytes
					//    B0-B6: number of 16-byte lines to transfer = n+1 (number of bytes = 16*(n+1))
					//    B7: 1=horizontal blanking DMA transfer, 0=general purpose DMA transfer

#define GB_IO_RP	0x56		// only GBC, infrared port
					//    B0: write data, 1=LED on (sending light)
					//    B1: read data, 1=LED on (receiving light)
					//    B6,B7: 11=data read enable flag, 00=disable

#define GB_IO_BCPS	0x68		// only GBC, BG write palettes
					//    B0: 1=H, 0=L
					//    B1,B2: palette data
					//    B3-B5: palette
					//    B7: 1=with each write next palette, 0=fixed palette (bit 0-5)

#define GB_IO_BCPD	0x69		// only GBC, BG write data

#define GB_IO_OCPS	0x6A		// only GBC, OBJ write palettes (as BCPS)

#define GB_IO_OCPD	0x6B		// only GBC, OBJ write data

#define GB_IO_SVBK	0x70		// only GBC, (0)1-7: WRAM 0xD000-0xDFFF switch bank 1 to 7

#define GB_IO_IE	0xFF		// (default 0 = disable) interrup enable (1=interrupt enabled)
					//    B0: vertical blanking (jump to 0x0040)
					//    B1: LCDC status (jump to 0x0048)
					//    B2: timer overflow (jump to 0x0050)
					//    B3: serial I/O transfer completion (jump to 0x0058)
					//    B4: P10-P13 terminal negative edge (jump to 0x0060)

// read memory (callback from emulator)
u8 FASTCODE NOFLASH(GB_GetMem)(u16 addr);

// write memory (callback from emulator)
void FASTCODE NOFLASH(GB_SetMem)(u16 addr, u8 data);
