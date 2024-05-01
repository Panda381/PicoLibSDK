
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

// PC graphics cards:
// ------------------
//   MDA (Monochrome Display Adapter): mono text mode only, 4 KB video RAM
//		mode 7: text 80 x 25 mono, characters 8x16 (400 lines) or 9x14 (350 lines) (4 KB) https://en.wikipedia.org/wiki/IBM_Monochrome_Display_Adapter
//   CGA (Color Graphics Adapter): 16 KB video RAM, text mode, graphics 640 x 200, 16 colors https://en.wikipedia.org/wiki/Color_Graphics_Adapter
//		16 KB of video memory at 0xB8000 is aliased at 0xBC000
//		mode 0,1: text 40 x 25 with 8x8 font (resolution 320x200) (2 KB)
//		mode 2,3: text 80 x 25 with 8x8 font (resolution 640x200) (4 KB)
//		modified text mode 3: graphics 160 x 100 / 16 colors (16 KB as modified attribute text mode)
//		mode 4,5: graphics 320 x 200 / 4 colors (chosen from 3 fixed palettes, color 1 chosen from 16-color palette) (16 KB)
//		mode 6: graphics 640 x 200 / 2 colors, black + 1 color from 16-color palette (16 KB)
//   PCjr: video RAM is shared with CPU at 128 KB system RAM, using 16 KB banks.
//		One 16 KB bank can be mapped to 0xB8000 (alias 0xBC000) to be compatible with CGA.
//   Tandy 1000: video RAM is shared with CPU at 128 KB system RAM, using 16 KB banks.
//		Two 16 KB banks (32 KB) can be mapped to 0xB8000. Test programs http://www.oldskool.org/guides/tvdog/graphics.html
//   Hercules (Hercules Graphics Card): 64 KB video RAM
//		mode 7: text 80 x 25 mono, characters 9x14 (720 x 350 resolution)
//		graphics 720 x 348 / mono
//   EGA (Enhanced Graphic Adapter): 64 KB or 256 KB video RAM
//		graphics 640 x 350 / 16 colors (112 KB)
//		graphics 640 x 350 / 2 colors (56 KB)
//		graphics 640 x 200 / 16 colors (64 KB)
//		graphics 320 x 200 / 16 colors (32 KB)
//   VGA (Video Graphics Array): 256 KB video RAM, 16 or 256 colors
//		graphics 640 x 480 / 16 colors
//		graphics 640 x 350 or 640 x 200 / 16 colors
//		graphics 320 x 200 / 16 colors
//		mode 13h (19): graphics 320 x 200 / 256 colors
//   MCGA (Multicolor Graphics Adapter): compatible with VGA, 64 KB video RAM, max. resolution 640x480 / 2 colors
//   PGC (Professional Graphics Controller): graphics 640 x 480 / 256 colors

// >>> Keep tables and functions in RAM (without "const") for faster access.

// graphics card
#define PC_CARD_MDA	0	// MDA (4 KB VRAM) ... videomode 7 (emulator 0..3, 7)
#define PC_CARD_CGA	1	// CGA (16 KB VRAM) ... videomodes 0-6 (emulator 0..7)
#define PC_CARD_PCJR	2	// PCjr (32 KB VRAM) ... videomodes 0-6, 8-10 (emulator 0-10)
#define PC_CARD_TANDY	3	// Tandy 1000 (64 KB VRAM) ... videomodes 0-6, 8-11 (emulator 0-11)
#define PC_CARD_EGA64	4	// EGA64 (64 KB VRAM) ... videomodes 0-15, 16 limited (4 colors)
#define PC_CARD_EGA128	5	// EGA128 (128 KB VRAM) ... videomodes 0-16
#define PC_CARD_MCGA	6	// MCGA (64 KB VRAM) ... videomodes 0-15, 16 limited (4 colors), 17, 19
#define PC_CARD_VGA	7	// VGA (256 KB VRAM, real 160 KB) ... videomodes 0-19

#define PC_CARD_NUM	8	// number of graphics cards

// order of video memory size
#define PC_VMEM_1K	10	// 1 KB video RAM
#define PC_VMEM_2K	11	// 2 KB video RAM
#define PC_VMEM_4K	12	// 4 KB video RAM (MDA)
#define PC_VMEM_8K	13	// 8 KB video RAM
#define PC_VMEM_16K	14	// 16 KB video RAM (CGA)
#define PC_VMEM_32K	15	// 32 KB video RAM (PCjr)
#define PC_VMEM_64K	16	// 64 KB video RAM (Tandy, EGA64)
#define PC_VMEM_128K	17	// 128 KB video RAM (EGA)
#define PC_VMEM_256K	18	// 256 KB video RAM (VGA) - real limited to 160 KB

// videomodes
#define PC_VMODE_0	0	// (CGA, EGA, VGA) text 40x25, 16 colors (4 + 4 bits), memory 2000 bytes at 0xB8000, 1 byte character + 1 byte attribute,
				//		font 8x8, resolution 320x200 ... emulator 16 KB: 0xB8000 - 0xBBFFF
#define PC_VMODE_1	1	// (CGA, EGA, VGA) text 40x25, 16 colors (4 + 4 bits), memory 2000 bytes at 0xB8000, 1 byte character + 1 byte attribute,
				//		font 8x8, resolution 320x200 ... emulator 16 KB: 0xB8000 - 0xBBFFF
#define PC_VMODE_2	2	// (CGA, EGA, VGA) text 80x25, 16 colors (4 + 4 bits), memory 4000 bytes at 0xB8000, 1 byte character + 1 byte attribute,
				//		font 8x8, resolution 640x200 ... emulator 16 KB: 0xB8000 - 0xBBFFF
#define PC_VMODE_3	3	// (CGA, EGA, VGA) text 80x25, 16 colors (4 + 4 bits), memory 4000 bytes at 0xB8000, 1 byte character + 1 byte attribute,
				//		font 8x8, resolution 640x200 ... emulator 16 KB: 0xB8000 - 0xBBFFF
#define PC_VMODE_4	4	// (CGA, EGA, VGA) graphics 320x200, 4 colors (2 bits), memory 16000 bytes at 0xB8000
				//		interlaced 8 KB even lines + 8 KB odd lines (each line is 80 bytes) ... emulator 16 KB: 0xB8000 - 0xBBFFF
#define PC_VMODE_5	5	// (CGA, EGA, VGA) graphics 320x200, 4 colors (2 bits), memory 16000 bytes at 0xB8000
				//		interlaced 8 KB even lines + 8 KB odd lines (each line is 80 bytes) ... emulator 16 KB: 0xB8000 - 0xBBFFF
#define PC_VMODE_6	6	// (CGA, EGA, VGA) graphics 640x200, 2 colors (1 bit), memory 16000 bytes at 0xB8000
				//		interlaced 8 KB even lines + 8 KB odd lines (each line is 80 bytes) ... emulator 16 KB: 0xB8000 - 0xBBFFF
#define PC_VMODE_7	7	// (MDA, EGA, VGA) text 80x25, 2 colors (4 + 4 bits), memory 4000 bytes at 0xB0000, 1 byte character + 1 byte attribute,
				//		font 8x8, resolution 640x200 ... emulator 4 KB: 0xB0000 - 0xB0FFF
#define PC_VMODE_8	8	// (PCjr, Tandy) graphics 160x200, 16 colors (4 bits), memory 16000 bytes at 0xB8000, packed format, 1 byte contains 2 pixels
				//		interlaced 8 KB even lines + 8 KB odd lines (each line is 80 bytes) ... emulator 16 KB: 0xB8000 - 0xBBFFF
#define PC_VMODE_9	9	// (PCjr, Tandy) graphics 320x200, 16 colors (4 bits), memory 32000 bytes at 0xB8000, packed format, 1 byte contains 2 pixels
				//		interlaced 8 KB line 0 4 8... + 8 KB line 1 5 9... + 8 KB line 2 6 10... + 8 KB line 3 7 11..
				//		(each line is 160 bytes) ... emulator 32 KB: 0xB8000 - 0xBFFFF
				//		PCjr maps only lower 16 KB to 0xB8000. All 32 KB are mapped to 0x18000. Tandy maps whole 32K to 0xB8000.
#define PC_VMODE_10	10	// (PCjr, Tandy) graphics 640x200, 4 colors (2 bits), memory 32000 bytes at 0xB8000, packed format, 1 byte contains 4 pixels
				//		interlaced 8 KB line 0 4 8... + 8 KB line 1 5 9... + 8 KB line 2 6 10... + 8 KB line 3 7 11..
				//		(each line is 160 bytes) ... emulator 32 KB: 0xB8000 - 0xBFFFF
				//		PCjr maps only lower 16 KB to 0xB8000. All 32 KB are mapped to 0x18000. Tandy maps whole 32K to 0xB8000.
#define PC_VMODE_11	11	// (Tandy) graphics 640x200, 16 colors (4 bits), memory 64000 bytes at 0xB0000, packed format, 1 byte contains 2 pixels
				//		interlaced on 8 segments 8 KB ... emulator 64 KB: 0xB0000 - 0xBFFFF
#define PC_VMODE_12	12	// (Emulator) text 20x15, 16 colors (4 + 4 bits), memory 600 bytes at 0xB80000, 1 byte character + 1 byte attribute,
				//		font 8x8, resolution 160x120 ... emulator 1 KB: 0xB8000 - 0xB83FF
#define PC_VMODE_13	13	// (EGA, VGA) graphics 320x200, 16 colors (4 bits), memory 32 KB at 0xA0000, 4 memory planes of size 8000 bytes
				//		emulator 32 KB: 0xA0000 - 0xA7FFF
#define PC_VMODE_14	14	// (EGA, VGA) graphics 640x200, 16 colors (4 bits), memory 64 KB at 0xA0000, 4 memory planes of size 16000 bytes
				//		emulator 64 KB: 0xA0000 - 0xAFFFF
#define PC_VMODE_15	15	// (EGA, VGA) graphics 640x350, 2 colors (1 bits), memory 32 KB at 0xA0000,
				//		emulator 32 KB: 0xA0000 - 0xA7FFF
#define PC_VMODE_16	16	// (EGA, VGA) graphics 640x350, 16 colors (4 bits) (or 4 colors/2 bits on EGA64 - planes 0 and 2),
				//		memory 128 KB (or 64 KB on EGA64) at 0xA0000, 4 memory planes of size 28000 bytes
				//		emulator 128 KB: 0xA0000 - 0xBFFFF
#define PC_VMODE_17	17	// (VGA) graphics 640x480, 2 colors (1 bits), memory 38400 bytes at 0xA0000
				//		emulator 40960 bytes: 0xA0000 - 0xA9FFF
#define PC_VMODE_18	18	// (VGA) graphics 640x480, 16 colors (4 bits), memory 153600 bytes at 0xA0000, 4 memory planes of size 38400 bytes
				//		emulator 160 KB: 0xA0000 - 0xC7FFF
#define PC_VMODE_19	19	// (VGA) graphics 320x200, 256 colors (8 bits), memory 64000 bytes at 0xA0000
				//		emulator 64 KB: 0xA0000 - 0xAFFFF
#define PC_VMODE_NUM	20	// number of videomodes (= max. videomode + 1)

#define PC_PLANE18_SIZE	40960	// size of plane at videomode 18 (VGA 640x480/16 col; = 0xA000)
#define PC_VMEM_256K_REAL (4*PC_PLANE18_SIZE) // real size of memory of videmode 18 (= 163840 = 0x28000 = 160 KB)

#define PC_VRAM_PAGEMIN	0xA0	// minimal video memory page address (= 0xA0000)
#define PC_VRAM_PAGEMAX	0xC8	// maximal video memory page address (= 0xC8000), video RAM max. size is 0x28000 = 160 KB

#define PC_VMODE_NOCLS	B7	// flag - do not clear the screen
#define PC_VMODE_EMU	PC_VMODE_12	// emulator videomode 20x15

// class of videomode
#define PC_VCLASS_TEXT	0	// text videomode (0-3, 7, 12)
#define PC_VCLASS_CGA2	1	// 2-color CGA graphics (6)
#define PC_VCLASS_CGA4	2	// 4-color CGA graphics (4, 5, 10)
#define PC_VCLASS_CGA16	3	// 16-color CGA graphics (8, 9, 11)
#define PC_VCLASS_EGA2	4	// 2-color EGA graphics (15, 17)
#define PC_VCLASS_EGA16	5	// 16-color EGA graphics (13, 14, 16, 18)
#define PC_VCLASS_VGA	6	// 256-color VGA graphics (19)

// EGA color palette components
#define PC_EGA_BLUE	COLOR(0, 0, 170)	// (0x01) blue 2/3
#define PC_EGA_GREEN	COLOR(0, 170, 0)	// (0x02) green 2/3
#define PC_EGA_RED	COLOR(170, 0, 0)	// (0x04) red 2/3
#define PC_EGA_BLUE2	COLOR(0, 0, 85)		// (0x08) blue 1/3
#define PC_EGA_GREEN2	COLOR(0, 85, 0)		// (0x10) green 1/3
#define PC_EGA_RED2	COLOR(85, 0, 0)		// (0x20) red 1/3

#define PC_EGA_GREY	COLOR(85, 85, 85)	// (0x38) grey 1/3

// PC colors (attribute color)
// - This definition is duplicated to lib_tprint.h file
#define PC_BLACK	0
#define PC_BLUE		1
#define PC_GREEN	2
#define PC_CYAN		3
#define PC_RED		4
#define PC_MAGENTA	5
#define PC_BROWN	6
#define PC_LTGRAY	7
#define PC_GRAY		8
#define PC_LTBLUE	9
#define PC_LTGREEN	10
#define PC_LTCYAN	11
#define PC_LTRED	12
#define PC_LTMAGENTA	13
#define PC_YELLOW	14
#define PC_WHITE	15

// compose PC color
#define PC_COLOR(bg,fg) (((bg)<<4)|(fg))

#define PC_VMODE_PAGE_MAX	8	// max. number of video pages

#define PC_DEF_COL	PC_WHITE	// default console color

// EGA/VGA setup registers 0x3C4/0x3C5
#define PC_EGA_SET_RESET	0	// 0: sequencer reset
#define PC_EGA_SET_CLOCK	1	// 1: sequencer clocking mode
#define PC_EGA_SET_WMASK	2	// 2: sequencer write mask
#define PC_EGA_SET_CHAR		3	// 3: sequencer character map select
#define PC_EGA_SET_MEM		4	// 4: sequencer memory mode

#define PC_EGA_SET_MAX		4	// max. EGA/VGA setup register

// EGA/VGA graphics registers 0x3CE/0x3CF
#define PC_EGA_GR_COLOR		0	// 0: graphics color to set
#define PC_EGA_GR_MASK		1	// 1: graphics mask of color planes to set
#define PC_EGA_GR_COMP		2	// 2: graphics compare color
#define PC_EGA_GR_SHIFT		3	// 3: graphics data rotate
#define PC_EGA_GR_READ		4	// 4: graphics read plane
#define PC_EGA_GR_MODE		5	// 5: graphics mode
#define PC_EGA_GR_MEM		6	// 6: graphics memory mapping
#define PC_EGA_GR_CARE		7	// 7: graphics color don't care
#define PC_EGA_GR_BIT		8	// 8: graphics bit mask

#define PC_EGA_GR_MAX		8	// max. EGA/VGA graphics register

// graphics card order of memory size
extern const u8 PC_CardMem[PC_CARD_NUM];

// video page cursor
typedef struct {
	u8	curx;		// text cursor position X
	u8	cury;		// text cursor position Y
} sPC_VmodeCur;

// current videomode descriptor
typedef struct {
	u8	card;		// graphics card PC_CARD_*
	u8	vmode;		// current videomode PC_VMODE_* (PC_VMODE_NUM = no videmode set yet)
	u8	vclass;		// videomode classs PC_VCLASS_*
	u8	cols;		// number of text columns
	u8 	rows;		// number of text rows
	u8	pages;		// number of pages (must be power of 2, max. 8)
	u8	page;		// current visible page
	u8	pagemask;	// mask of page number
	Bool	blink;		// blinking attributes (or intensity background otherwise)
	u8	fonth;		// font height (8, 14 or 16)
	u8	fontpage;	// memory page with default font
	u8	curstart;	// cursor start line (0..7)
	u8	curend;		// cursor end line (0..7)

	u8	egasetsel;	// EGA/VGA select setup register 0x3C4
	u8	egagrsel;	// EGA/VGA select graphics register 0x3CE
	u8	egaset[PC_EGA_SET_MAX+1]; // EGA/VGA setup registers 0x3C5
	u8	egagr[PC_EGA_GR_MAX+1];	// EGA/VGA graphics registers 0x3CF 0..PC_EGA_GR_MAX

	u16	segm;		// base segment address
	u16	resx;		// resolution X
	u16	resy;		// resolution Y
	sPC_VmodeCur cur[PC_VMODE_PAGE_MAX]; // cursors on pages

	int	cardmem;	// video memory on graphics card
	int	usedmem;	// used video memory
	int	planesize;	// plane/page size in bytes
	int	off;		// offset in video-RAM of current visible page 'page'
	int	mask;		// mask of offset in video-RAM
	int	wb;		// width of line in bytes
	int	vfont;		// virtual address of font
	u8*	base;		// pointer to current video-RAM base address
	const u8* font;		// real address of font
	u16*	pal;		// real address of palettes
	u32	latch;		// old value read from videomemory
} sPC_Vmode;

extern sPC_Vmode PC_Vmode;	// current videomode
extern u8 PC_EgaSelReg;		// EGA select setup register 0x3C4
extern u8 PC_EgaWPlanes;	// EGA write planes 0x3C5/0x02
extern u8 PC_EgaCtrlReg;	// EGA select control register 0x3CE
extern u8 PC_EgaCtrlReg0;	// EGA control register 0x3CF/0x00 - color
extern u8 PC_EgaCtrlReg1;	// EGA control register 0x3CF/0x01 - select planes to write color instead data
extern u8 PC_EgaCtrlReg8;	// EGA control register 0x3CF/0x08 - select bits to write

extern u8 PC_CgaSelReg;		// CGA select setup register 0x3B4, 0x3D4

extern u16 PC_CurPal2[2];		// current 2-color CGA palettes
extern u16 PC_CurPal4[4];		// current 4-color CGA palettes
extern u16 PC_CurPal16[16];		// current 16-color EGA palettes
extern u16 PC_CurPal256[256];		// current 256-color VGA palette

// select CGA 4-color palette by CRT palette register
//	B5: palette 0 (red-green-brown) or 1 (cyan-magenta-white)
//	B4: intense colors in graphics, intense background in text mode
extern const u16* PC_Pal4_CgaCrt[4];

extern const u16 PC_Pal2[2];		// 2-color CGA palettes (black/white)
extern const u16 PC_Pal2Inv[2];		// 2-color CGA palettes inverted (white/black)
extern const u16 PC_Pal4_0_low[4];	// 4-color CGA palettes 0, low intensity (green/red/brown)
extern const u16 PC_Pal4_0_high[4];	// 4-color CGA palettes 0, high intensity (green/red/yellow)
extern const u16 PC_Pal4_1_low[4];	// 4-color CGA palettes 1, low intensity (cyan/magenta/white)
extern const u16 PC_Pal4_1_high[4];	// 4-color CGA palettes 1, high intensity (cyan/magenta/white)
extern const u16 PC_Pal4_2_low[4];	// 4-color CGA palettes 2, low intensity (cyan/red/white)
extern const u16 PC_Pal4_2_high[4];	// 4-color CGA palettes 2, high intensity (cyan/red/white)
extern const u16 PC_Pal16[16];		// 16-color EGA palettes
extern const u16 PC_PalMDAWhite[16];	// 16-color MDA palettes (to simulate MDA using CGA text mode) - white
extern const u16 PC_PalMDAGreen[16];	// 16-color MDA palettes (to simulate MDA using CGA text mode) - green
extern const u16 PC_PalMDAPlasma[16];	// 16-color MDA palettes (to simulate MDA using CGA text mode) - orange
extern const u16 PC_Pal16Ega64[16];	// 16-color EGA64 palettes
extern const u16 PC_Pal256[256];	// VGA 256-color default palette

// set videomode PC_VMODE_* (set bit 7 to not clear screen; returns False if videmode cannot be set)
//  "card" entry must be set
// Debug videomode 12 does not deallocate current video memory.
Bool PC_SetVmode(int vmode);

// scroll window (does not affect cursor position)
//  page ... page
//  shift ... number of rows to scroll (0 = clear entire window, > 0 up, < 0 down)
//  attr ... color attribute of new rows
//  x1 ... left column
//  y1 ... top row
//  x2 ... right column
//  y2 ... bottom row
void PC_Scroll(u8 page, int shift, u8 attr, u8 x1, u8 y1, u8 x2, u8 y2);

// scroll screen up (does not affect cursor position)
//  page ... page
void PC_ScrollUp(u8 page);

// clear screen
void PC_ClearScr(u8 page, u8 attr);

// write character to video memory
//  ch ... character to display
//  page ... page (or background color in 256-color mode)
//  attr ... attribute in text mode, or color in graphics (bit 7: XOR with image at mode < 256 colors)
//  num ... number of characters
//  noattr ... True=do not use attributes at text mode
void PC_WriteCh(u8 ch, u8 page, u8 attr, int num, Bool noattr);

// write character in TTY mode
//  ch ... character
//  page ... page (or background color in 256-color mode)
//  attr ... color (only in graphics mode; bit 7: XOR with image at mode < 256 colors)
//  noattr ... True=do not use attributes at text mode
void PC_WriteTty(u8 ch, u8 page, u8 attr, Bool noattr);

// INT 10h service - function
void PC_Int10fnc();
