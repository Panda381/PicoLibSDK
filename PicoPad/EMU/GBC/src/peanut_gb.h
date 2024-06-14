/**
 * MIT License
 *
 * Copyright (c) 2018-2023 Mahyar Koshkouei
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Please note that at least three parts of source code within this project was
 * taken from the SameBoy project at https://github.com/LIJI32/SameBoy/ which at
 * the time of this writing is released under the MIT License. Occurrences of
 * this code is marked as being taken from SameBoy with a comment.
 * SameBoy, and code marked as being taken from SameBoy,
 * is Copyright (c) 2015-2019 Lior Halphon.
 */

// Picopad modification: (2024) Miroslav Nemecek

#define GB_SAVEFILE_VER	'1'		// save file version magic

#define PEANUT_GB_HIGH_LCD_ACCURACY 0	// 1=higher rendering accuracy (sort sprites)

// Interrupt masks
#define VBLANK_INTR	0x01		// vertical blanking (VSYNC)
#define LCDC_INTR	0x02		// LCD controller status
#define TIME_INTR	0x04		// timer
#define SERIAL_INTR	0x08		// serial port
#define CONTROL_INTR	0x10		// user control
#define ANY_INTR	0x1F		// mask of all interrupts

// Memory section sizes
#define WRAM_SIZE	0x8000		// working RAM (WRAM) = 32 KB for GBC
#define VRAM_SIZE	0x4000		// video RAM (VRAM) = 16 KB for GBC
#define HRAM_IO_SIZE	0x0100		// high RAM (HRAM) = 256 B
#define OAM_SIZE	0x00A0		// object RAM (ORAM) = 160 B
#define CRAM_SIZE	0x20000		// cartridge RAM max. size (128 KB)

// Memory addresses
#define ROM_0_ADDR      0x0000		// ROM base bank 0 (ROM0)
#define ROM_N_ADDR      0x4000		// ROM banks 1.. (ROMN)
#define VRAM_ADDR       0x8000		// video RAM (VRAM)
#define CART_RAM_ADDR   0xA000		// cartridge RAM (CRAM)
#define WRAM_0_ADDR     0xC000		// working RAM base bank (WRAM0)
#define WRAM_1_ADDR     0xD000		// working RAM bank 1 (WRAM1)
#define ECHO_ADDR       0xE000		// echo copy of working RAM
#define OAM_ADDR        0xFE00		// object RAM (ORAM)
#define UNUSED_ADDR     0xFEA0		// unused RAM
#define IO_ADDR         0xFF00		// ports (HRAM)
#define HRAM_ADDR       0xFF80		// high RAM user data (HRAM)
#define INTR_EN_ADDR    0xFFFF		// interupt enable register (EN)

// Cart section sizes
#define ROM_BANK_SIZE   0x4000		// ROM bank size
#define WRAM_BANK_SIZE  0x1000		// working RAM bank size
#define CRAM_BANK_SIZE  0x2000		// cartridge RAM bank size
#define VRAM_BANK_SIZE  0x2000		// video RAM bank size

// Game Boy Memory Controller
#define MBC0		0		// no memory controller
#define MBC1		1		// ROM 512 KB (32 banks) with RAM 32 KB (4 banks), or ROM 2 MB (128 banks) with RAM 8 KB (1 bank)
#define MBC2		2		// ROM 256 KB (16 banks)
#define MBC3		3		// ROM 2 MB (128 banks)
#define MBC5		5		// ROM 8 MB (512 banks)

// DIV Register is incremented at rate of 16384Hz.
// * 4194304 / 16384 = 256 clock cycles for one increment.
#define DIV_CYCLES	256

// Serial clock locked to 8192Hz on DMG.
// * 4194304 / (8192 / 8) = 4096 clock cycles for sending 1 byte.
#define SERIAL_CYCLES       4096
#define SERIAL_CYCLES_1KB   (SERIAL_CYCLES/1ul)
#define SERIAL_CYCLES_2KB   (SERIAL_CYCLES/2ul)
#define SERIAL_CYCLES_32KB  (SERIAL_CYCLES/32ul)
#define SERIAL_CYCLES_64KB  (SERIAL_CYCLES/64ul)

// Calculating VSYNC.
#define DMG_CLOCK_FREQ      4194304.0	// base clock
#define SCREEN_REFRESH_CYCLES 70224.0	// screen refresh
#define VERTICAL_SYNC       (DMG_CLOCK_FREQ/SCREEN_REFRESH_CYCLES) // vertical sync = 59.73 Hz

// SERIAL SC register masks.
#define SERIAL_SC_TX_START  0x80
#define SERIAL_SC_CLOCK_SRC 0x01

// STAT register masks
#define STAT_LYC_INTR       0x40	// LYC = LY, vertical line equal
#define STAT_MODE_2_INTR    0x20
#define STAT_MODE_1_INTR    0x10
#define STAT_MODE_0_INTR    0x08
#define STAT_LYC_COINC      0x04
#define STAT_MODE           0x03
#define STAT_USER_BITS      0xF8

// LCDC control masks
#define LCDC_ENABLE         0x80
#define LCDC_WINDOW_MAP     0x40
#define LCDC_WINDOW_ENABLE  0x20
#define LCDC_TILE_SELECT    0x10
#define LCDC_BG_MAP         0x08
#define LCDC_OBJ_SIZE       0x04
#define LCDC_OBJ_ENABLE     0x02
#define LCDC_BG_ENABLE      0x01

// LCD characteristics
#define LCD_LINE_CYCLES     456		// PPU cycles through modes every 456 cycles.
#define LCD_MODE_0_CYCLES   372		// Mode 0 starts on cycle 372.
// Games Frogger 1 and 2 GBC requires LCD_MODE_2_CYCLES = 80
#define LCD_MODE_2_CYCLES   204		// Mode 2 starts on cycle 204.
#define LCD_MODE_3_CYCLES   284		// Mode 3 starts on cycle 284.
#define LCD_VERT_LINES      154		// There are 154 scanlines. LY < 154.
#define LCD_WIDTH           160		// GB LCD width
#define LCD_HEIGHT          144		// GB LCD height
#define LCD_FRAMEHEIGHT     80		// height of ring frame buffer
#define LCD_FRAMESIZE       (LCD_WIDTH*LCD_FRAMEHEIGHT) // size of frame buffer (in number of entries)

// VRAM Locations
#define VRAM_TILES_1        (0x8000 - VRAM_ADDR)
#define VRAM_TILES_2        (0x8800 - VRAM_ADDR)
#define VRAM_BMAP_1         (0x9800 - VRAM_ADDR)
#define VRAM_BMAP_2         (0x9C00 - VRAM_ADDR)
#define VRAM_TILES_3        (0x8000 - VRAM_ADDR + VRAM_BANK_SIZE)
#define VRAM_TILES_4        (0x8800 - VRAM_ADDR + VRAM_BANK_SIZE)

// Interrupt jump addresses
#define VBLANK_INTR_ADDR    0x0040
#define LCDC_INTR_ADDR      0x0048
#define TIMER_INTR_ADDR     0x0050
#define SERIAL_INTR_ADDR    0x0058
#define CONTROL_INTR_ADDR   0x0060

// SPRITE controls
#define NUM_SPRITES         0x28
#define MAX_SPRITES_LINE    0x0A
#define OBJ_PRIORITY        0x80
#define OBJ_FLIP_Y          0x40
#define OBJ_FLIP_X          0x20
#define OBJ_PALETTE         0x10
#define OBJ_BANK            0x08
#define OBJ_CGB_PALETTE     0x07

#define ROM_HEADER_CHECKSUM_LOC	0x014D

// Local macros.
#define PEANUT_GB_ARRAYSIZE(array)    (sizeof(array)/sizeof(array[0]))

# define PGB_INSTR_SBC_R8(r,cin)						\
	{									\
		u16 temp = gb->cpu_reg.a - (r + cin);				\
		gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;			\
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.a ^ r ^ temp) & 0x10) > 0; \
		gb->cpu_reg.f_bits.n = 1;					\
		gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);			\
		gb->cpu_reg.a = (temp & 0xFF);					\
	}

# define PGB_INSTR_CP_R8(r)							\
	{									\
		u16 temp = gb->cpu_reg.a - r;					\
		gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;			\
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.a ^ r ^ temp) & 0x10) > 0; \
		gb->cpu_reg.f_bits.n = 1;					\
		gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);			\
	}

# define PGB_INSTR_ADC_R8(r,cin)						\
	{									\
		u16 temp = gb->cpu_reg.a + r + cin;				\
		gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;			\
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.a ^ r ^ temp) & 0x10) > 0; \
		gb->cpu_reg.f_bits.n = 0;					\
		gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);			\
		gb->cpu_reg.a = (temp & 0xFF);					\
	}

#define PGB_INSTR_DEC_R8(r)							\
	r--;									\
	gb->cpu_reg.f_bits.h = ((r & 0x0F) == 0x0F);				\
	gb->cpu_reg.f_bits.n = 1;						\
	gb->cpu_reg.f_bits.z = (r == 0x00);

#define PGB_INSTR_XOR_R8(r)							\
	gb->cpu_reg.a ^= r;							\
	gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);				\
	gb->cpu_reg.f_bits.n = 0;						\
	gb->cpu_reg.f_bits.h = 0;						\
	gb->cpu_reg.f_bits.c = 0;

#define PGB_INSTR_OR_R8(r)							\
	gb->cpu_reg.a |= r;							\
	gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);				\
	gb->cpu_reg.f_bits.n = 0;						\
	gb->cpu_reg.f_bits.h = 0;						\
	gb->cpu_reg.f_bits.c = 0;

#define PGB_INSTR_AND_R8(r)							\
	gb->cpu_reg.a &= r;							\
	gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);				\
	gb->cpu_reg.f_bits.n = 0;						\
	gb->cpu_reg.f_bits.h = 1;						\
	gb->cpu_reg.f_bits.c = 0;

# define PEANUT_GB_GET_LSB16(x) (x & 0xFF)
# define PEANUT_GB_GET_MSB16(x) (x >> 8)
# define PEANUT_GB_GET_MSN16(x) (x >> 12)
# define PEANUT_GB_U8_TO_U16(h,l) ((l) | ((h) << 8))

// CPU registers (12 bytes)
struct cpu_registers_s
{
	// 0x00: AF (Define specific bits of Flag register.)
	struct
	{
		u8 c : 1;	// Carry flag.
		u8 h : 1;	// Half carry flag.
		u8 n : 1;	// Add/sub flag.
		u8 z : 1;	// Zero flag.
	} f_bits;
	u8 a;

	// 0x02: BC
	union
	{
		struct
		{
			u8 c,b;
		} bytes;
		u16 reg;
	} bc;

	// 0x04: DE
	union
	{
		struct
		{
			u8 e,d;
		} bytes;
		u16 reg;
	} de;

	// 0x06: HL
	union
	{
		struct
		{
			u8 l,h;
		} bytes;
		u16 reg;
	} hl;

	// 0x08: SP (Stack pointer)
	union
	{
		struct
		{
			u8 p, s;
		} bytes;
		u16 reg;
	} sp;

	// 0x0A: PC (Program counter)
	union
	{
		struct
		{
			u8 c, p;
		} bytes;
		u16 reg;
	} pc;
};
STATIC_ASSERT(sizeof(struct cpu_registers_s) == 0x0C, "Incorrect struct cpu_registers_s!");

// Counters (8 bytes)
struct count_s
{
	u16 lcd_count;		// 0x00: LCD Timing
	u16 div_count;		// 0x02: Divider Register Counter
	u16 tima_count;		// 0x04: Timer Counter
	u16 serial_count;	// 0x06: Serial Counter
};
STATIC_ASSERT(sizeof(struct count_s) == 0x08, "Incorrect struct count_s!");

// Bit mask for the shade of pixel to display
#define LCD_COLOUR	0x03

// Bit mask for whether a pixel is OBJ0, OBJ1, or BG. Each may have a different
// palette when playing a DMG game on CGB.
#define LCD_PALETTE_OBJ	0x10
#define LCD_PALETTE_BG	0x20

// Bit mask for the two bits listed above.
//  LCD_PALETTE_ALL == 0b00 --> OBJ0
//  LCD_PALETTE_ALL == 0b01 --> OBJ1
//  LCD_PALETTE_ALL == 0b10 --> BG
//  LCD_PALETTE_ALL == 0b11 --> NOT POSSIBLE
#define LCD_PALETTE_ALL 0x30

// Errors that may occur during emulation.
enum gb_error_e
{
	GB_UNKNOWN_ERROR = 0,
	GB_INVALID_OPCODE,
	GB_INVALID_READ,
	GB_INVALID_WRITE,
	GB_HALT_FOREVER,

	GB_INVALID_MAX
};

// Errors that may occur during library initialisation.
enum gb_init_error_e
{
	GB_INIT_NO_ERROR = 0,
	GB_INIT_CARTRIDGE_UNSUPPORTED,
	GB_INIT_INVALID_CHECKSUM
};

// Return codes for serial receive function, mainly for clarity.
enum gb_serial_rx_ret_e
{
	GB_SERIAL_RX_SUCCESS = 0,
	GB_SERIAL_RX_NO_CONNECTION = 1
};

// Emulator context.
//   Only values within the 'direct' struct may be modified directly by the
//   front-end implementation. Other variables must not be modified.
struct gb_s
{
// interface (local data, not save to the file)
#define GBS_LOCAL	(7*4 + LCD_FRAMESIZE*2 + 4)	// size of local data

	// Return byte from ROM at given address.
	// 	gb_s ... emulator context
	//	addr ... address
	// returns byte at address in ROM
	u8 (*gb_rom_read)(struct gb_s*, const uint_fast32_t addr);

	// Return byte from cart RAM at given address.
	//	gb_s ... emulator context
	//	addr ... address
	// returns byte at address in RAM
	u8 (*gb_cart_ram_read)(struct gb_s*, const uint_fast32_t addr);

	// Write byte to cart RAM at given address.
	//	gb_s ... emulator context
	//	addr ... address
	//	val ... value to write to address in RAM
	void (*gb_cart_ram_write)(struct gb_s*, const uint_fast32_t addr, const u8 val);

	// Notify front-end of error.
	//	gb_s ... emulator context
	//	gb_error_e ... error code
	//	addr ... address of where error occurred
	void (*gb_error)(struct gb_s*, const enum gb_error_e, const u16 addr);

	// Transmit one byte and return the received byte.
	void (*gb_serial_tx)(struct gb_s*, const u8 tx);
	enum gb_serial_rx_ret_e (*gb_serial_rx)(struct gb_s*, u8* rx);

	// Read byte from boot ROM at given address.
	u8 (*gb_bootrom_read)(struct gb_s*, const uint_fast16_t addr);

	// LCD display frame buffer
	// Scan lines in the frame buffer are always in the correct order for display.
	u16 framebuf[LCD_FRAMESIZE];		// LCD frame buffer
	volatile u8 frame_write;		// next relative line to write to the frame buffer
	volatile u8 frame_wline;		// next absolute line to write to the frame buffer
	volatile u8 frame_read;			// next relative line to read from the frame buffer
	volatile u8 frame_rline;		// next absolute line to read from the frame buffer

// ==== here starts save content

	// 0x00
	u8 savestart; // save file version magic

	// 0x01
	struct
	{
		u8 gb_halt		: 1;
		u8 gb_ime		: 1;
		u8 gb_frame		: 1; // New frame drawn.
		u8 lcd_blank		: 1;
		u8 battery		: 1; // cartridge contains backup battery
		u8 gb_pause		: 1; // program is paused (similar to halt, but cannot be interrupted)
	};

	// 0x02: Cartridge information
	s8 mbc;			// 0x02: Memory Bank Controller (MBC) type.
	u8 cart_ram;		// 0x03: Whether the MBC has internal RAM.
	u16 num_rom_banks_mask;	// 0x04: Number of ROM banks in cartridge.
	u8 num_ram_banks;	// 0x06: Number of RAM banks in cartridge. Ignore for MBC2.
	u8 cart_ram_bank;	// 0x07:
	u16 selected_rom_bank;	// 0x08: WRAM and VRAM bank selection not available.
	u8 enable_cart_ram;	// 0x0A:
	u8 cart_mode_select;	// 0x0B: Cartridge ROM/RAM mode select.

	// 0x0C: (size 8 bytes) RTC
	union
	{
		struct
		{
			u8 sec;
			u8 min;
			u8 hour;
			u8 yday;
			u8 high;
			u8 res[3]; // align
		} rtc_bits;
		u8 cart_rtc[5+3];
	};

	// 0x14: CPU registers (12 bytes)
	struct cpu_registers_s cpu_reg;

	// 0x20: counters (8 bytes)
	struct count_s counter;

	// 0x28: buffers (size 0x2C1A4)
	u8 wram[WRAM_SIZE];		// working RAM (0x8000)
	u8 vram[VRAM_SIZE];		// video RAM (0x4000)
	u8 oam[OAM_SIZE];		// object RAM (0x00A0)
	u8 hram_io[HRAM_IO_SIZE];	// high RAM (0x0100)
	u8 cram[CRAM_SIZE];		// cartridge RAM (0x20000)
	u32 cram_valid;			// cartridge RAM valid mask (bit '1' = 4 KB page is valid)

// here is offset 0x2C1CC

	// 0x2C1CC: display interface (size 16 bytes)
	struct
	{
		// Palettes
		u8 bg_palette[4];
		u8 sp_palette[8];

		u8 window_clear;
		u8 WY;

		// Only support 30fps frame skip.
		u8 frame_skip_count : 1;	// Picopad: not used
		u8 interlace_count : 1;		// Picopad: not used

		u8 palinx; // GBC palette index
	} display;

// here is offset 0x2C1DC

	// Game Boy Color Mode (size 0x114)
	struct {
		u8 cgbMode;		// 0x00
		u8 doubleSpeed;		// 0x01
		u8 doubleSpeedPrep;	// 0x02
		u8 wramBank;		// 0x03

		u16 wramBankOffset;	// 0x04
		u8 vramBank;		// 0x06
		u8 OAMPaletteID;	// 0x07

		u8 BGPaletteID;		// 0x08
		u8 OAMPaletteInc;	// 0x09
		u16 vramBankOffset;	// 0x0A

		u16 fixPalette[0x40];	// 0x0C: BG then OAM palettes fixed for the screen
		u8 OAMPalette[0x40];	// 0x8C
		u8 BGPalette[0x40];	// 0xCC

		u8 BGPaletteInc;	// 0x10C
		u8 dmaRunning;		// 0x10D
		u8 dmaMode;		// 0x10E
		u8 dmaSize;		// 0x10F

		u16 dmaSource;		// 0x110
		u16 dmaDest;		// 0x112
	} cgb;

// here is offset 0x2C2F0

	// (size 8 bytes)
	// Variables that may be modified directly by the front-end.
	// This method seems to be easier and possibly less overhead than
	// calling a function to modify these variables each time.
	// None of this is thread-safe.
	struct
	{
		// Set to enable interlacing. Interlacing will start immediately
		// (at the next line drawing).
		u8 interlace : 1;	// Picopad: not used
		u8 frame_skip : 1;	// Picopad: not used

		union
		{
			struct
			{
				u8 a		: 1;
				u8 b		: 1;
				u8 select	: 1;
				u8 start	: 1;
				u8 right	: 1;
				u8 left		: 1;
				u8 up		: 1;
				u8 down		: 1;
			} joypad_bits;
			u8 joypad;
		};

		u16	res;		// align

		// Implementation defined data. Set to NULL if not required.
		void *priv;
	} direct;

// here is offset 0x2C2F8

	// audio (size 4*0x3C + 8 = 0xF8)
	struct snd_chan chans[4];
	s32 vol_l, vol_r;

// total 0x2C3F0
};

STATIC_ASSERT(sizeof(struct gb_s) == GBS_LOCAL + 0x2C3F0, "Incorrect struct gb_s!");
#define SAVEFILE_SIZE	(sizeof(struct gb_s) - GBS_LOCAL)

// Ports
#define IO_JOYP	0x00
#define IO_SB	0x01
#define IO_SC	0x02
#define IO_DIV	0x04
#define IO_TIMA	0x05
#define IO_TMA	0x06
#define IO_TAC	0x07
#define IO_IF	0x0F
#define IO_SND	0x10
#define IO_BOOT	0x50
#define IO_LCDC	0x40
#define IO_STAT	0x41
#define IO_SCY	0x42
#define IO_SCX	0x43
#define IO_LY	0x44
#define IO_LYC	0x45
#define	IO_DMA	0x46
#define	IO_BGP	0x47
#define	IO_OBP0	0x48
#define IO_OBP1	0x49
#define IO_WY	0x4A
#define IO_WX	0x4B
#define IO_BANK	0x50
#define IO_IE	0xFF

#define IO_TAC_RATE_MASK	0x3
#define IO_TAC_ENABLE_MASK	0x4

// LCD Mode defines.
#define IO_STAT_MODE_HBLANK		0
#define IO_STAT_MODE_VBLANK		1
#define IO_STAT_MODE_SEARCH_OAM		2
#define IO_STAT_MODE_SEARCH_TRANSFER	3
#define IO_STAT_MODE_VBLANK_OR_TRANSFER_MASK 0x1

struct sprite_data
{
	u8 sprite_number;
	u8 x;
};

// Internal function used to read bytes.
u8 FASTCODE NOFLASH(__gb_read)(struct gb_s *gb, u16 addr);

// Internal function used to write bytes.
void FASTCODE NOFLASH(__gb_write)(struct gb_s *gb, uint_fast16_t addr, u8 val);

// Internal function used to step the CPU.
void FASTCODE NOFLASH(__gb_step_cpu)(struct gb_s *gb);

// draw line to output LCD buffer
void FASTCODE NOFLASH(__gb_draw_line)(struct gb_s *gb);

// Function prototypes: Required functions

// Initialises the emulator context to a known state. Call this before calling
// any other peanut-gb function.
// To reset the emulator, you can call gb_reset() instead.
//
//	gb ... Allocated emulator context. Must not be NULL.
//	gb_rom_read ... Pointer to function that reads ROM data. ROM banking is
//			already handled by Peanut-GB. Must not be NULL.
//	gb_cart_ram_read ... Pointer to function that reads Cart RAM. Must not be NULL.
//	gb_cart_ram_write ... Pointer to function to writes to Cart RAM. Must not be NULL.
//	gb_error ... Pointer to function that is called when an unrecoverable
//			error occurs. Must not be NULL. Returning from this
//			function is undefined and will result in SIGABRT.
//	priv ... Private data that is stored within the emulator context. Set to NULL if unused.
// returns 0 on success or an enum that describes the error.
enum gb_init_error_e gb_init(struct gb_s *gb,
			     u8 (*gb_rom_read)(struct gb_s*, const uint_fast32_t),
			     u8 (*gb_cart_ram_read)(struct gb_s*, const uint_fast32_t),
			     void (*gb_cart_ram_write)(struct gb_s*, const uint_fast32_t, const u8),
			     void (*gb_error)(struct gb_s*, const enum gb_error_e, const u16),
			     void *priv);

// Executes the emulator and runs for one frame.
//	gb ... An initialised emulator context. Must not be NULL.
void FASTCODE NOFLASH(gb_run_frame)(struct gb_s *gb);

// Internal function used to step the CPU. Used mainly for testing.
// Use gb_run_frame() instead.
//	gb ... An initialised emulator context. Must not be NULL.
void FASTCODE NOFLASH(__gb_step_cpu)(struct gb_s *gb);

// Function prototypes: Optional Functions **/

// Reset the emulator, like turning the Game Boy off and on again.
// This function can be called at any time.
//	gb ... An initialised emulator context. Must not be NULL.
void gb_reset(struct gb_s *gb);

// Initialises the display context of the emulator.
// The pixel data sent to lcd_draw_line comes with both shade and layer data.
// The first two least significant bits are the shade data (black, dark, light,
// white). Bits 4 and 5 are layer data (OBJ0, OBJ1, BG), which can be used to
// add more colours to the game in the same way that the Game Boy Color does to
// older Game Boy games.
// This function can be called at any time.
//	gb ... An initialised emulator context. Must not be NULL.
void gb_init_lcd(struct gb_s *gb);

// Initialises the serial connection of the emulator. This function is optional,
// and if not called, the emulator will assume that no link cable is connected
// to the game.
//	gb ... An initialised emulator context. Must not be NULL.
//	gb_serial_tx ... Pointer to function that transmits a byte of data over
//		the serial connection. Must not be NULL.
//	gb_serial_rx ... Pointer to function that receives a byte of data over the
//		serial connection. If no byte is received,
//		return GB_SERIAL_RX_NO_CONNECTION. Must not be NULL.
void gb_init_serial(struct gb_s *gb,
		    void (*gb_serial_tx)(struct gb_s*, const u8),
		    enum gb_serial_rx_ret_e (*gb_serial_rx)(struct gb_s*, u8*));

// Obtains the save size of the game (size of the Cart RAM). Required by the
// frontend to allocate enough memory for the Cart RAM.
//	gb ... An initialised emulator context. Must not be NULL.
// returns size of the Cart RAM in bytes. 0 if Cartridge has not battery backed RAM.
uint_fast32_t gb_get_save_size(struct gb_s *gb);

// Calculates and returns a hash of the game header in the same way the Game
// Boy Color does for colourising old Game Boy games. The frontend can use this
// hash to automatically set a colour palette.
//	gb ... An initialised emulator context. Must not be NULL.
// returns Hash of the game header.
u8 gb_colour_hash(struct gb_s *gb);

// Returns the title of ROM.
//	gb ... An initialised emulator context. Must not be NULL.
//	title_str ... Allocated string at least 16 characters.
// returns pointer to start of string, null terminated.
const char* gb_get_rom_name(struct gb_s* gb, char *title_str);

// Tick the internal RTC by one second. This does not affect games with no RTC support.
//	gb ... An initialised emulator context. Must not be NULL.
void gb_tick_rtc(struct gb_s *gb);

// Set initial values in RTC.
// Should be called after gb_init().
//	gb ... An initialised emulator context. Must not be NULL.
//	time ... Time structure with date and time.
void gb_set_rtc(struct gb_s *gb, const struct tm * const time);

// Use boot ROM on reset. gb_reset() must be called for this to take affect.
//	gb ... 	An initialised emulator context. Must not be NULL.
//	gb_bootrom_read ... Function pointer to read boot ROM binary.
void gb_set_bootrom(struct gb_s *gb, u8 (*gb_bootrom_read)(struct gb_s*, const uint_fast16_t));
