
// ****************************************************************************
//
//                           M6502/M65C02 CPU Emulator
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

// MOS-6502 CPU speed: 1 MHz to 3 MHz (Atari PAL: 1.77 MHz, Atari NTSC: 1.79 MHz)
// Memory mapping (Atari):
//  0x0000..0x00FF: page zero
//	0x0000..0x007F: system page zero
//	0x0080..0x00FF: BASIC and user page zero
//  0x0100..0x01FF: stack
//  0x0200..0x047F: system variables
//  0x0480..0x06FF: user RAM
//  0x2000..0x9FFF: free RAM
//  0xA000..0xBFFF: BASIC ROM
//  0xE000..0xEFFF: hardware I/O
//  0xF000..0xFFFF: system ROM

#if USE_EMU_M6502			// use M6502 CPU emulator

// code modifications
#ifndef M6502_CPU_65C02
#define M6502_CPU_65C02		1	// 1=use modifications of 65C02 and later
#endif

// constants
#define M6502_CLOCKMUL	2	// clock multiplier (to achieve lower frequencies and finer timing)
#define M6502_MEMSIZE	0x10000	// memory size
#define M6502_STACKBASE	0x0100	// stack base address (stack is located at addresses 0x0100 to 0x01FF)

// flags
#define M6502_N		B7		// (0x80) Negative, arithmetic operation was negative (bit 7 was set)
#define M6502_V		B6		// (0x40) Overflow, arithmetic operation overflow
#define M6502_E		B5		// (0x20) bit 5 is always "1"
#define M6502_B		B4		// (0x10) Break, 1="BRK" was called, 0=status register was transferred by hardware
#define M6502_D		B3		// (0x08) Decimal, use BCD binary coded decimal arithmetic
#define M6502_I		B2		// (0x04) IRQ disable
#define M6502_Z		B1		// (0x02) Zero, arithmetic operation was zero
#define M6502_C		B0		// (0x01) Carry, carry over

#define M6502_FLAGDEF		(B5+B4+B2+B1) // default flags
#define M6502_FLAGALL		(B0+B1+B6+B7) // all arithmetic flags

// system vectors
#define M6502_VECT_NMI	0xFFFA		// non-maskable interrupt
#define M6502_VECT_RES	0xFFFC		// reset
#define M6502_VECT_IRQ	0xFFFE		// interrupt request

// M6502 CPU descriptor
// - Optimization of thumb1 on RP2040: offset of byte should be <= 31, word <= 62, dword <= 124
// - Entries should be aligned
typedef struct {
	sEmuSync	sync;		// 0x00: time synchronization
	union { u16 pc; struct { u8 pcl, pch; }; }; // 0x08: program counter PC
	u8		sp;		// 0x0A: stack pointer (stack is on page one 0x0100-0x01FF, top-down)
	u8		a;		// 0x0B: accumulator
	u8		f;		// 0x0C: flags (processor status register)
	u8		x;		// 0x0D: index register X
	u8		y;		// 0x0E: index register Y
	volatile u8	stop;		// 0x0F: 1=request to stop (pause) program execution
	volatile u8	intreq;		// 0x10: 1=interrupt request
	volatile u8	nmireq;		// 0x11: 1=NMI request
	volatile u8	resreq;		// 0x12: 1=RESET request
	u8		res;		// 0x13: ... reserved (align)
	pEmu16Read8	readmem;	// 0x14: read byte from memory
	pEmu16Write8	writemem;	// 0x18: write byte to memory
} sM6502;

STATIC_ASSERT(sizeof(sM6502) == 0x1C, "Incorrect sM6502!");

// current CPU descriptor (NULL = not running)
extern volatile sM6502* M6502_Cpu;

// initialize time synchronization (initialize PWM counter; returns real emulated frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// To achieve accurate timing, an integer ratio between system clock and clock frequency*M6502_CLOCKMUL is recommended to maintain.
INLINE u32 M6502_SyncInit(sM6502* cpu, int pwm, u32 freq) { return (EmuSyncInit(&cpu->sync, pwm, freq*M6502_CLOCKMUL) + M6502_CLOCKMUL/2)/M6502_CLOCKMUL; }

// reset processor
void M6502_Reset(sM6502* cpu);

// start time synchronization (= sets "time 0"; should be called before first program execution)
INLINE void M6502_SyncStart(sM6502* cpu) { EmuSyncStart(&cpu->sync); }

// execute program (start or continue, until "stop" request)
// Size of code of this function: 7320 code + 1024 jump table = 8344 bytes
// CPU loading at 1.77 MHz on 177 MHz: used 42-49%, max. 42-62%
void FASTCODE NOFLASH(M6502_Exec)(sM6502* cpu);

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
INLINE void M6502_SyncTerm(int pwm) { EmuSyncTerm(pwm); }

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*M6502_CLOCKMUL is recommended to maintain.
// Emulation can be stopped by M6502_Stop() function or by STOP 0 instruction.
u32 M6502_Start(sM6502* cpu, int pwm, u32 freq);

// stop emulation (can be also activated by STOP 0 instruction)
//  pwm ... index of used PWM slice (0..7)
void M6502_Stop(int pwm);

// continue emulation without restart processor
u32 M6502_Cont(sM6502* cpu, int pwm, u32 freq);

// check if emulation is running
INLINE Bool M6502_IsRunning() { return M6502_Cpu != NULL; }

#endif // USE_EMU_M6502
