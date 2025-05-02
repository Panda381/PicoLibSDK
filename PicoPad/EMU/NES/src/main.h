
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

//#define NES_CLKSYS	200000		// system clock in kHz
#define EMU_PWM		2		// index of PWM used to synchronize emulations
#define EMU_FREQ	NES_FREQ	// emulated frequency (NTSC = 21477272/12 = 1789773 Hz)
#define NES_FPS		(60.0*NES_FREQ/1789773) // frames per second (60 Hz default)

//#define DEB_REMAP			// Debug - uncomment this to enable mapper number incrementing using B key in menu

//#define NES_PAL	3		// Use NES palettes: 1=GBAR4444-main, 2=RGB555-Win,
					//    3=RGB888-Matthew, 4=RGB888-Piotr, 5=RGB888-NesDev,
					//    6=RGB888-Wiki, 7=RGB888-Smooth

// Scanlines:
//   0-7: (8 scanlines) cut-off start of visible image
//   8-231: (224 scanlines) visible image
//   232-239: (8 scanlines) cut-off end of visible image
//   240: (1 scanline) post-render blanking before V-blank
//   241: (20 scanline) V-blank, send NMI IRQ on start of scanline 241
//   261: (1 scanline) pre-render blanking after V-blank
//#define NES_NMI_SHIFT	0			// shift start of V-blank (try to raise if some lines flicker)
#define SCAN_TOP_OFF_SCREEN_START 0		// disable visible image on top (8 scanlines)
#define SCAN_ON_SCREEN_START 8			// enable visible image on top
#define SCAN_BOTTOM_OFF_SCREEN_START 232	// disable visible image on bottom (8 scanlines)
#define SCAN_UNKNOWN_START 240			// 1 "post-render" blanking line between end of picuter and NMI
#define SCAN_VBLANK_START (241+NES_NMI_SHIFT)	// start of V-blank, send NMI IRQ
#define SCAN_VBLANK_END 261			// end of V-blank, 1 "pre-render" line between V-blank and next picture
#define SCAN_VLINES 262				// total number of vertical lines

//#define STEP_PER_SCANLINE 114			// (= NES_FREQ/NES_FPS/SCAN_VLINES = 1789773/60/262 = 113.8532) CPU clocks per scanline
#define STEP_PER_SCANLINE_H 116586		// CPU clocks per scanline HighRes (* 1024)
#define STEP_PER_SCANLINE_SHIFT 10		// shifts of CPU clocks per scanline HighRes

// zoom, clip image
#if !NES_CLIP_UP && !NES_CLIP_DOWN && !NES_CLIP_LEFT && !NES_CLIP_RIGHT
#undef NES_CLIP_UP
#undef NES_CLIP_DOWN
#undef NES_CLIP_LEFT
#undef NES_CLIP_RIGHT
#define NES_CLIP_UP	8	// clip image up
#define NES_CLIP_DOWN	8	// clip image down
#define NES_CLIP_LEFT	8	// clip image left
#define NES_CLIP_RIGHT	8	// clip image right
#define NES_CLIP_ZOOM	False	// zoom default on
#else
#define NES_CLIP_ZOOM	True	// zoom default off
#endif

// memory
#define WRAM_SIZE	0x0800		// size of working internal WRAM
#define SRAM_SIZE	0x2000		// size of external cartridge CRAM
#define PPURAM_SIZE	0x4000		// size of video PPU RAM
#define SPRRAM_SIZE	256		// size of sprite RAM

#define MAPDATA_SIZE	0x100		// size of mapper data

#define XRAM_SIZE	(67*1024)	// cartridge extra RAM (67 KB)

#define EMU_PWMTOP		1023	// PWM sound top (period = EMU_PWMTOP + 1 = 1024)
#define AUDIO_SAMPLE_RATE  	31440	// sample rate - 2 samples per scanline (sample delta 1000000/31440 = 31.8 us)
#define EMU_PWMCLOCK	(AUDIO_SAMPLE_RATE*(EMU_PWMTOP+1)) // PWM clock (= 32768*1024 = 32 194 560 Hz)
#define AUDIO_LOCK() IRQ_LOCK		// audio lock (this alternative works only on the same core)
#define AUDIO_UNLOCK() IRQ_UNLOCK	// audio unlock

#define NES_CPUCLOCK_PER_SAMPLE	 ((EMU_FREQ + AUDIO_SAMPLE_RATE/2) / AUDIO_SAMPLE_RATE) // CPU clock per audio sample ((1789773 + 31440/2)/31440 = 57, precise 1789773/31440 = 56.9266)

// NES display size
#define NES_DISP_WIDTH	256		// NES display width
#define NES_DISP_HEIGHT	240		// NES display height
#define NES_FRAMESIZE	(NES_DISP_WIDTH*NES_DISP_HEIGHT) // NES display size

#define NES_HSYNCTIME	((u32)(1000000.0/(NES_FPS*SCAN_VLINES)+0.5)) // h-sync delta time in [us] (= 1000000/(60*262)+0.5 = 64 us

#define NES_BLACK	0x0d		// black color
#define NES_BG		PalTable[0]	// background color
#define NES_COLOR(c)	(((c) >= 0) ? (c) : PalTable[-(c)-1]) // clip color 0..53: palette color, -1..-32: display color, 13: black, -1: background

// keyboard
#define GPA  (1 << 0)
#define GPB  (1 << 1)
#define GPY  (1 << 2)
#define GPX  (1 << 3)
#define GPUP  (1 << 4)
#define GPDOWN  (1 << 5)
#define GPLEFT  (1 << 6)
#define GPRIGHT  (1 << 7)

// home path
extern char HomePath[];
extern int HomePathLen;
extern Bool NES_ReqExit; // request to exit program

#ifdef DEB_REMAP			// Debug - enable mapper number incrementing using B key in menu
extern int ReMapperNo;
#endif

// ROM CRC
extern u32 NES_RomCrc;
extern int NES_VROMSize; // VROM size (bytes)

// NES emulator context
typedef struct {

// ==== 8-bit variables (offset should be <= 0x1F)

// aligned
	u8	savestart; 	// 0x00: save file version magic
	u8	reg_a;		// 0x01: register A (accumulator)
	u8	reg_f;		// 0x02: register F (flags)
	u8	reg_x;		// 0x03: register X (index)
// aligned
	u8	reg_y;		// 0x04: register Y (index)
	u8	reg_sp;		// 0x05: register SP (stack pointer)
	u16	reg_pc;		// 0x06: register PC (program counter)
// aligned
	u8	irq_state;	// 0x08: state of IRQ pin
	u8	irq_wiring;	// 0x09: wiring of IRQ pin
	u8	nmi_state;	// 0x0A: state of NMI pin
	u8	nmi_wiring;	// 0x0B: wiring of NMI pin
// aligned
	u8	vram_write_en;	// 0x0C: VRAM write enable
	u8	ppu_scr_h_byte;	// 0x0D: PPU h-byte ... horizontal byte index X of PPU address (0..31)
	u8	ppu_scr_h_bit;	// 0x0E: PPU h-bit
	u8	frameirq_en;	// 0x0F: frame IRQ enabled
// aligned
	u8	ppu_latch_flag;	// 0x10: PPU latch flag
	u8	ppu_updown;	// 0x11: PPU up-down clip
	u8	sram_written;	// 0x12: SRAM written
	u8	ppu_nametab;	// 0x13: index of name table bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)
// aligned
	u8	ppu_r[8];	// 0x14 (size 8): PPU registers
	u8	apu_r[0x18];	// 0x1C (size 0x18): APU registers

// offset 0x34

// ==== 16-bit variables (offset should be <= 0x3E)

// aligned
	u16	ppu_addr;	// 0x34: current address in PPU video memory (0x0000-0x3FFF)
	u16	ppu_temp;	// 0x36: PPU temporary
// aligned
	u16	ppu_inc;	// 0x38: PPU increment
	u16	sprite_hit;	// 0x3A: sprite #0 scanline hit position
// aligned
	u16	ppu_sp_height;	// 0x3C: sprite height	
	u16	frame_step;	// 0x3E: frame step

// offset 0x40

// ==== 32-bit variables (offset should be <= 0x7C)

// aligned
	u8*	srambank;	// 0x40: pointer to SRAM bank
	u8*	vrom;		// 0x44: pointer to video VROM
	const u8* rombank[4];	// 0x48 (size 0x10): pointers to current ROM banks
// aligned
	s32	passed_clocks;	// 0x58: number of passed clocks
	s32	current_clocks;	// 0x5C: number of current clocks
// aligned
	u8*	ppu_bg_base;	// 0x60: background base (relative to NULL)
	u8*	ppu_sp_base;	// 0x64: sprite base (relative to NULL)

	u8	res[0x18];	// 0x68: ... reserved

// offset 0x80

// ==== Sound chip

	sApuSnd	apu;		// 0x80: (size 0xA0) sound chip

// ==== buffers

	u8	paltab[32];		// 0x120 (size 0x20): palette table
	u8*	ppubank[16];		// 0x140 (size 0x40): pointers to PPU banks (...0x2000, 0x2400, 0x2800, 0x2C00...)

	// --- wram[0] = start address of VRAM_WCHECK check
	u8	wram[WRAM_SIZE];	// 0x180 (size 0x0800): working WRAM
	u8	vram[PPURAM_SIZE];	// 0x980 (size 0x4000): video PPU RAM
	u8	sprram[SPRRAM_SIZE];	// 0x4980 (size 0x0100): sprite RAM

// ==== mapper data

	u8	mapdata[MAPDATA_SIZE];	// 0x4A80 (size 0x0100): mapper data

// ==== cartridge extra RAM (must be last entry in this structure!)
// XRAM Requirements:
//  Mapper 5: 0x2000*8 + 0x400 + 0x400 + 0x400 = 0x10C00 = 67 KB
//  Mapper 6: 0x2000*4 = 0x8000 = 32 KB
//  Mapper 19: 0x2000 = 8 KB
//  Mapper 69: 0x2000 = 8 KB
//  Mapper 85: 0x100 * 0x400 = 0x40000 = 256 KB ... not used
//  Mapper 185: 0x0400 = 1 KB
//  Mapper 188: 0x2000 = 8 KB
//  Mapper 235: 0x2000 = 8 KB

	union {
		struct {
			u8	sram[SRAM_SIZE];	// 0x4B80 (size 0x2000): cartridge SRAM
			u8	xram[XRAM_SIZE];	// 0x6B80: cartridge extra RAM
		};

		u8	cram[SRAM_SIZE + XRAM_SIZE];	// 0x4B800: all cartridge SRAM
	};

// ==== frame buffer

	// --- framebuf[0] = end address of VRAM_WCHECK check
	u8	framebuf[NES_FRAMESIZE]; // frame buffer

#define XX 0 //(1*1024)
//	u8	xx[XX];		// .... unused reserve to check free RAM

} sNES;

STATIC_ASSERT(sizeof(sNES) == 0x6B80 + XRAM_SIZE + NES_FRAMESIZE + XX, "Incorrect sNES!");

extern int NES_XRamSize;		// size of extra RAM

#define SAVEFILE_SIZE	(sizeof(sNES) - XRAM_SIZE - NES_FRAMESIZE - XX + NES_XRamSize)	// size of save file

// frame buffer, with emulator RAM
#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
extern ALIGNED FRAMETYPE FrameBuf2[];
#define NES ((sNES*)FrameBuf2)
#else
#define NES ((sNES*)FrameBuf)
#endif

#define APU (&NES->apu)		// sound chip

#define A		NES->reg_a		// register A (accumulator)
#define F		NES->reg_f		// register F (flags)
#define X		NES->reg_x		// register X (index)

#define Y		NES->reg_y		// register Y (index)
#define SP		NES->reg_sp		// register SP (stack pointer)
#define PC		NES->reg_pc		// register PC (program counter)

#define IRQ_State	NES->irq_state		// state of IRQ pin
#define IRQ_Wiring	NES->irq_wiring		// wiring of IRQ pin
#define NMI_State	NES->nmi_state		// state of NMI pin
#define NMI_Wiring	NES->nmi_wiring		// wiring of NMI pin

#define byVramWriteEnable NES->vram_write_en	// VRAM write enable
#define PPU_Scr_H_Byte	NES->ppu_scr_h_byte	// PPU h-byte ... PPU_Scr_H_Byte horizontal byte index X of PPU address (0..31)
#define PPU_Scr_H_Bit	NES->ppu_scr_h_bit	// PPU h-bit
#define FrameIRQ_Enable	NES->frameirq_en	// frame IRQ enabled

#define PPU_Latch_Flag	NES->ppu_latch_flag	// PPU latch flag
#define PPU_UpDown_Clip NES->ppu_updown		// PPU up-down clip
#define SRAMwritten	NES->sram_written	// SRAM written
#define PPU_NameTableBank NES->ppu_nametab	// index of name table bank (8=0x2000, 9=0x2400, 10=0x2800, 11=0x2C00)

#define PPU_R		NES->ppu_r		// PPU registers
#define APU_Reg		NES->apu_r		// APU registers

#define PPU_Addr	NES->ppu_addr		// current address in PPU video memory (0x0000-0x3FFF)
#define PPU_Temp	NES->ppu_temp		// PPU temporary

#define PPU_Increment	NES->ppu_inc		// PPU increment
#define SpriteJustHit	NES->sprite_hit		// sprite #0 scanline hit position

#define PPU_SP_Height	NES->ppu_sp_height	// sprite height	
#define	FrameStep	NES->frame_step		// frame step

#define SRAMBANK	NES->srambank		// pointer to SRAM bank
#define VROM		NES->vrom		// pointer to video VROM
#define ROMBANK		NES->rombank		// pointers to ROM banks

#define g_wPassedClocks	NES->passed_clocks	// number of elapsed clock ticks left in the previous "step" pass
#define g_wCurrentClocks NES->current_clocks	// number of current clocks

#define PPU_BG_Base	NES->ppu_bg_base	// background base (relative to NULL)
#define PPU_SP_Base	NES->ppu_sp_base	// sprite base (relative to NULL)

#define PalTable	NES->paltab		// palette table
#define PPUBANK		NES->ppubank		// pointers to PPU banks (0x2000, 0x2400, 0x2800, 0x2C00)
#define	RAM		NES->wram		// working RAM
#define SRAM		NES->sram		// cartridge RAM
#define PPURAM		NES->vram		// video PPU RAM
#define SPRRAM		NES->sprram		// sprite RAM

#define MAPDATA		NES->mapdata		// mapper data

#define ROMBANK0	ROMBANK[0]
#define ROMBANK1	ROMBANK[1]
#define ROMBANK2	ROMBANK[2]
#define ROMBANK3	ROMBANK[3]

#define PPU_R0		PPU_R[0]
#define PPU_R1		PPU_R[1]
#define PPU_R2		PPU_R[2]
#define PPU_R3		PPU_R[3]	// PPU_R3 = OAMADDR address in OAM memory
#define PPU_R4		PPU_R[4]
#define PPU_R5		PPU_R[5]
#define PPU_R6		PPU_R[6]
#define PPU_R7		PPU_R[7]	// PPU_R7 = temporary register during read from PPU memory

// NES palettes in RGB565 format
extern u16 NesPalette[64];

// NES initialize
Bool NES_Init();

// NES terminate
void NES_Term();

// unlink NES before save
void NES_Unlink();

// link NES back after save or load
void NES_Link();

// initialize system clock
void NES_InitSysClk();

// restore system clock
void NES_TermSysClk();

// initialize core 1
void NES_InitCore1();

// terminate core 1
void NES_TermCore1();

// draw on screen
#define NES_DISPMODE_MSG		0	// display message text window
#define NES_DISPMODE_EMU		1	// display emulated window
extern volatile u8 NES_DispMode;		// display mode NES_DISPMODE_*
extern volatile u8 NES_DispMap[NES_DISP_HEIGHT]; // map of rendered scanlines

// program ROM
extern char ProgName[];		// program name (max. 8 characters upper case)
extern int ProgNameLen;		// length of program name (1 to 8 characters)
extern int builtinrom_len;
extern const u8 builtinrom[];

//#define ROM		((u8*)builtinrom)	// pointer to ROM (we need not 'const' - in some places it is used instead of reading from RAM)

#endif // _MAIN_H
