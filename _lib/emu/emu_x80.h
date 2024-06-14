
// ****************************************************************************
//
//                       Sharp X80 (SM83 LR35902) CPU Emulator
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

// https://rylev.github.io/DMG-01/public/book/introduction.html

// X80 CPU speed: 4.194304 MHz (Machine Cycles 1.05 MHz)

/*
GameBoy special registers (memory mapped at X80_PORTBASE port base):

0xFF00 (P1) ... joypad (R/W)
0xFF01 (SB) ... serial transfer data (R/W)
0xFF02 (SC) ... SIO control (R/W)
0xFF03 (DIV) ... divider register (R/W)
0xFF05 (TIMA) ... timer counter (R/W)
0xFF06 (TMA) ... timer modulo (R/W)
0xFF08 (TAC) ... timer control (R/W)
0xFF0F (IF) ... interrupt flag request (R/W)
	bit 0: V-blank
	bit 1: LCDC
	bit 2: timer
	bit 3: serial
	bit 4: keypad
0xFF10 (NR 10) ... sound mode 1 register - sweep register (R/W)
0xFF11 (NR 11) ... sound mode 1 register - sound length (R/W)
0xFF12 (NR 12) ... sound mode 1 register - envelope (R/W)
0xFF13 (NR 13) ... sound mode 1 register - frequency low (W)
0xFF14 (NR 14) ... sound mode 1 register - frequency high (R/W)
0xFF16 (NR 21) ... sound mode 2 register - sound length (R/W)
0xFF17 (NR 22) ... sound mode 2 register - envelope (R/W)
0xFF18 (NR 23) ... sound mode 2 register - frequency low (W)
0xFF19 (NR 24) ... sound mode 2 register - frequency high (R/W)
0xFF1A (NR 30) ... sound mode 3 register - sound on/off (R/W)
0xFF1B (NR 31) ... sound mode 3 register - sound length (R/W)
0xFF1C (NR 32) ... sound mode 3 register - select output level (R/W)
0xFF1D (NR 33) ... sound mode 3 register - frequency low (W)
0xFF1E (NR 34) ... sound mode 3 register - frequency high (R/W)
0xFF20 (NR 41) ... sound mode 4 register - sound length (R/W)
0xFF21 (NR 42) ... sound mode 4 register - envelope (R/W)
0xFF22 (NR 43) ... sound mode 4 register - polynomial counter (R/W)
0xFF23 (NR 44) ... sound mode 4 register - counter (R/W)
0xFF24 (NR 50) ... channel control ON-OFF (R/W)
0xFF25 (NR 51) ... selection of sound output (R/W)
0xFF26 (NR 52) ... sound on/off (R/W)
0xFF30-0xFF3F ... wave pattern RAM
0xFF40 (LCDC) ... LCD control (R/W)
0xFF41 (STAT) ... LCD status (R/W)
0xFF42 (SCY) ... scroll Y (R/W)
0xFF43 (SCX) ... scroll X (R/W)
0xFF44 (LY) ... LCDC Y coordinate (R)
0xFF45 (LYC) ... LY compare (R/W)
0xFF46 (DMA) ... DMA transfer and start address (W)
0xFF47 (BGP) ... BG window palette data (R/W)
0xFF48 (OBP0) ... object palette 0 (R/W)
0xFF49 (OBP1) ... object palette 1 (R/W)
0xFF4A (WY) ... window Y position (R/W)
0xFF4B (WX) ... window X position (R/W)
0xFFFF (IE) ... interrupt enable, 1=enable (R/W)
	bit 0: V-blank
	bit 1: LCDC
	bit 2: timer
	bit 3: serial
	bit 4: keypad
*/

#if USE_EMU_X80			// use Sharp X80 (LR35902) CPU emulator

// constants

	// X80_CLOCKMUL should be set to 1, to update DIV counter correctly
#define X80_CLOCKMUL	1	// clock multiplier (to achieve lower frequencies and finer timing)

#define X80_MEMSIZE	0x10000	// memory size
#define X80_PORTBASE	0xFF00	// port area base address (memory mapped)

// flags (unused bits 0..3 are always 0)
#define X80_C_BIT		4	// carry (or borrow)
#define X80_H_BIT		5	// half carry (or borrow) from bit 3 to bit 4
#define X80_N_BIT		6	// negative (1=last instruction was SUB, 0=last instruction was ADD)
#define X80_Z_BIT		7	// zero result

#define X80_C		BIT(X80_C_BIT)	// 0x10 carry (or borrow)
#define X80_H		BIT(X80_H_BIT)	// 0x20 half carry
#define X80_N		BIT(X80_N_BIT)	// 0x40 negative
#define X80_Z		BIT(X80_Z_BIT)	// 0x80 zero result

#define X80_FLAGALL	(B4+B5+B6+B7)	// mask of valid flags

// interrupt request bits (interrupt flag register IF 0xFF0F)
#define X80_IRQ_VBLANK	0		// V-blank (jump address 0x0040)
#define X80_IRQ_LCDC	1		// LCDC (jump address 0x0048)
#define X80_IRQ_TIMER	2		// timer (jump address 0x0050)
#define X80_IRQ_SERIAL	3		// serial (jump address 0x0058)
#define X80_IRQ_KEYPAD	4		// keypad control (jump address 0x0060)
#define X80_IRQ_ALL	(B0+B1+B2+B3+B4) // mask of all interrupts

// interrupt request flags (interrupt flag register IF 0xFF0F)
#define X80_IF_VBLANK	B0		// V-blank (jump address 0x0040)
#define X80_IF_LCDC	B1		// LCDC (jump address 0x0048)
#define X80_IF_TIMER	B2		// timer (jump address 0x0050)
#define X80_IF_SERIAL	B3		// serial (jump address 0x0058)
#define X80_IF_KEYPAD	B4		// keypad control (jump address 0x0060)
#define X80_IF_ALL	(B0+B1+B2+B3+B4) // mask of all interrupts

// LCD mode state
#define X80_LCD_MODE0	0		// LCD mode 0: start HSYNC (456 cycles)
#define X80_LCD_MODE1	1		// LCD mode 1: start VSYNC (144 lines, total 154 lines)
#define X80_LCD_MODE2	2		// LCD mode 2: start OAM-RAM search (204 cycles)
#define X80_LCD_MODE3	3		// LCD mode 3: start transfer (284 cycles)

// X80 CPU descriptor
// - Optimization of thumb1 on RP2040: offset of byte should be <= 31, word <= 62, dword <= 124
// - Entries should be aligned
typedef struct {
	sEmuSync	sync;		// 0x00: time synchronization
	union { u16 pc; struct { u8 pcl, pch; }; }; // 0x08: program counter PC
	union { u16 sp; struct { u8 spl, sph; }; }; // 0x0A: stack pointer SP
	u8		ime;		// 0x0C: 1=interrupt master enable flag
	u8		ie;		// 0x0D: interrupt enable mask (X80_IF_*)
	u8		halted;		// 0x0E: 1=CPU is halted (HALT instruction)
	volatile u8	stop;		// 0x0F: 1=request to stop (pause) program execution
	u8		tma;		// 0x10: TMA timer modulo
	u8		divshift;	// 0x11: number of shifts to get number of TIMA increments (255 = disabled)
	u8		tima;		// 0x12: TIMA counter
	u8		lcd_mode;	// 0x13: LCD current mode X80_LCD_MODE*
	union {
		struct {
			union { u16 fa; struct { u8 a, f; }; };	// 0x14: registers F (high) and A (low) ... registers FA are in reverse order than hardware!
			union { u16 hl; struct { u8 l, h; }; };	// 0x16: registers H (high) and L (low)
			union { u16 de; struct { u8 e, d; }; };	// 0x18: registers D (high) and E (low)
			union { u16 bc; struct { u8 c, b; }; };	// 0x1A: registers B (high) and C (low)
		};
		u8	reg[8];		// 0x14: registers accessible via index (0:A, 1:F, 2:L, 3:H, 4:E, 5:D, 6:C, 7:B)
		u16	dreg[4];	// 0x14: double registers (0:FA, 1:HL, 2:DE, 3:BC) ... registers FA are in reverse order than hardware!
	};
	u16		div;		// 0x1C: DIV counter * 256
	u16		lcd_count;	// 0x1E: LCD scanline time counter
	u16		lcd_cycles;	// 0x20: LCD cycles per scanline (start mode 0 = start HSYNC, 456 cycles)
	u16		lcd_cycles2;	// 0x22: LCD cycles per start mode 2 (OAM access, 204 cycles)
	u16		lcd_cycles3;	// 0x24: LCD cycles per start mode 3 (LCD transfer, 284 cycles)
	u16		res2;		// 0x26: ... reserved (align)
	pEmu16Read8	readmem;	// 0x28: read byte from memory
	pEmu16Write8	writemem;	// 0x2C: write byte to memory
	pEmuBool	stopins;	// 0x30: execute STOP instruction, return True = continue, False = repeat STOP instruction
	pEmuVoid	lcdline;	// 0x34: process LCD scanline (start HSYNC)
	pEmuVoid	lcdoam;		// 0x38: start LCD OAM-RAM access mode
	pEmuVoid	lcdtrans;	// 0x3C: start LCD transfer
} sX80;

STATIC_ASSERT(sizeof(sX80) == 0x40, "Incorrect sX80!");

// current CPU descriptor (NULL = not running)
extern volatile sX80* X80_Cpu;

// initialize time synchronization (initialize PWM counter; returns real emulated frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// To achieve accurate timing, an integer ratio between system clock and clock frequency*X80_CLOCKMUL is recommended to maintain.
INLINE u32 X80_SyncInit(sX80* cpu, int pwm, u32 freq) { return (EmuSyncInit(&cpu->sync, pwm, freq*X80_CLOCKMUL) + X80_CLOCKMUL/2)/X80_CLOCKMUL; }

// reset processor
void X80_Reset(sX80* cpu);

// start time synchronization (= sets "time 0"; should be called before first program execution)
INLINE void X80_SyncStart(sX80* cpu) { EmuSyncStart(&cpu->sync); }

// execute program (start or continue, until "stop" request)
// Size of code of this function: 5532 code + 1280 jump table = 6812 bytes
// CPU loading at 4.194350 MHz on 154.667 MHz: used 29-40%, max. 37-45%
void FASTCODE NOFLASH(X80_Exec)(sX80* cpu);

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
INLINE void X80_SyncTerm(int pwm) { EmuSyncTerm(pwm); }

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*X80_CLOCKMUL is recommended to maintain.
// Emulation can be stopped by X80_Stop() function or by STOP 0 instruction.
u32 X80_Start(sX80* cpu, int pwm, u32 freq);

// stop emulation (can be also activated by STOP 0 instruction)
//  pwm ... index of used PWM slice (0..7)
void X80_Stop(int pwm);

// continue emulation without restart processor
u32 X80_Cont(sX80* cpu, int pwm, u32 freq);

// check if emulation is running
INLINE Bool X80_IsRunning() { return X80_Cpu != NULL; }

// raise IRQ interrupt request X80_IRQ*
INLINE void X80_RaiseIRQ(sX80* cpu, int irq) { EmuInterSetBit(&cpu->sync, irq); }

#endif // USE_EMU_X80
