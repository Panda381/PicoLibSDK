
// ****************************************************************************
//
//                              Z80 CPU Emulator
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

// Z80 CPU speed:
//  cpu Z80: max. 2.5 MHz
//  cpu Z80A: max. 4 MHz
//  cpu Z80B: max. 6 MHz
//  cpu Z80H: max. 8 MHz
//  TRS-80: 1.774 MHz
//  TRS-80 II: 4 MHz
//  TRS-80 III: 2 MHz
//  TRS-80 IV: 4 MHz or 6 MHz
//  Rainbow 100: 4.012 MHz
//  Video Genie: 1.76 MHz
//  Micro-Professor MPF-I: 1.79 MHz
//  Sharp Hotbit, Gradiente Expert: 3.58 MHz
//  ZX-80, ZX-81: 3.25 MHz (or 3.55 MHz with 
//  ZX Spectrum: 3.5 MHz
//  Sharp MZ-800: 3.55 MHz
//  Ondra: 2 MHz
//  Sord M5: 3.58 MHz

#if USE_EMU_Z80			// use Z80 CPU emulator

// flags
#define Z80_C_BIT		0	// carry (or borrow)
#define Z80_N_BIT		1	// negative (1=last instruction was SUB, 0=last instruction was ADD)
#define Z80_PV_BIT		2	// parity/overflow (0=odd parity, 1=even parity)
#define Z80_X_BIT		3	// (undocumented) X flag, copy of bit 3 of result
#define Z80_H_BIT		4	// half carry (or borrow) from bit 3 to bit 4
#define Z80_Y_BIT		5	// (undocumented) Y flag, copy of bit 5 of result
#define Z80_Z_BIT		6	// zero
#define Z80_S_BIT		7	// sign

#define Z80_C		BIT(Z80_C_BIT)	// 0x01 carry (or borrow)
#define Z80_N		BIT(Z80_N_BIT)	// 0x02 negative
#define Z80_PV		BIT(Z80_PV_BIT)	// 0x04 parity/overflow
#define Z80_X		BIT(Z80_X_BIT)	// 0x08 (undocumented) X flag, copy of bit 3 of result
#define Z80_H		BIT(Z80_H_BIT)	// 0x10 half carry
#define Z80_Y		BIT(Z80_Y_BIT)	// 0x20 (undocumented) Y flag, copy of bit 5 of result
#define Z80_Z		BIT(Z80_Z_BIT)	// 0x40 zero
#define Z80_S		BIT(Z80_S_BIT)	// 0x80 sign

// interrupt modes
#define Z80_INTMODE0		0	// device can place any instruction on the data bus (as 8080A)
#define Z80_INTMODE1		1	// restart at address 0x0038
#define Z80_INTMODE2		2	// jump to address at vector table (high byte is in I register, low 7 bits come from the device)

#define Z80_CLOCKMUL	8	// clock multiplier (to achieve lower frequencies and finer timing)

// Z80 CPU descriptor
// - Keep compatibility of the structure with the #define version in machine code!
// - Entries must be aligned
// - thumb1 on RP2040: offset of byte fast access must be <= 31, word fast access <= 62, dword fast access <= 124
typedef struct {

	sEmuSync	sync;		// 0x00: (8) time synchronization
// align
	union { u16 pc; struct { u8 pcl, pch; }; }; // 0x08: program counter PC
	union { u16 sp; struct { u8 spl, sph; }; }; // 0x0A: stack pointer SP
// align
	union { u16 ix; struct { u8 ixl, ixh; }; }; // 0x0C: index register IX
	union { u16 iy; struct { u8 iyl, iyh; }; }; // 0x0E: index register IY
// align
	u8		r;		// 0x10: memory refresh address, lower 7 bits are auto incremented
	u8		r7;		// 0x11: bit 7 of R register, as programmed by user
	u8		i;		// 0x12: high byte of address of interrupt service
	u8		iff1;		// 0x13: interrupt flag IFF1 (0=disable interrupts, 1=enable interrupts)
// align
	u8		iff2;		// 0x14: interrupt flag IFF2 (temporary storage of IFF1 during nonmaskable interrupt)
	u8		mode;		// 0x15: interrupt mode Z80_INTMODE*
	u8		halted;		// 0x16: 1=CPU is halted (HALT instruction)
	u8		tid;		// 0X17: 1=temporary disable interrupt after EI instruction
// align
	volatile u8	stop;		// 0x18: 1=request to stop (pause) program execution
	u8		res, res2, res3; // 0x19: ... reserved (align)
// align
	union { u32 afbc;
		struct {
			union { u16 af; struct { u8 f, a; }; };		// 0x1C: registers A (high) and F (low)
			union { u16 bc; struct { u8 c, b; }; };		// 0x1E: registers B (high) and C (low)
		};
	};
// align
	union { u32 dehl;
		struct {
			union { u16 de; struct { u8 e, d; }; };		// 0x20: registers D (high) and E (low)
			union { u16 hl; struct { u8 l, h; }; };		// 0x22: registers H (high) and L (low)
		};
	};
// align
	union { u32 afbc2;
		struct {
			union { u16 af2; struct { u8 f2, a2; }; };	// 0x24: registers A' (high) and F' (low)
			union { u16 bc2; struct { u8 c2, b2; }; };	// 0x26: registers B' (high) and C' (low)
		};
	};
// align
	union { u32 dehl2;
		struct {
			union { u16 de2; struct { u8 e2, d2; }; };	// 0x28: registers D' (high) and E' (low)
			union { u16 hl2; struct { u8 l2, h2; }; };	// 0x2A: registers H' (high) and L' (low)
		};
	};
// align
	pEmu16Read8	readmem;	// 0x2C: read byte from memory
	pEmu16Write8	writemem;	// 0x30: write byte to memory
	pEmu16Read8	readio;		// 0x34: read byte from port
	pEmu16Write8	writeio;	// 0x38: write byte to port

} sZ80;

STATIC_ASSERT(sizeof(sZ80) == 0x3C, "Incorrect sZ80!");

// current CPU descriptor (NULL = not running)
extern volatile sZ80* Z80_Cpu;

// initialize Z80 tables
void Z80_InitTab();

// initialize time synchronization (initialize PWM counter; returns real emulated frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// To achieve accurate timing, an integer ratio between system clock and clock frequency*Z80_CLOCKMUL is recommended to maintain.
INLINE u32 Z80_SyncInit(sZ80* cpu, int pwm, u32 freq) { return (EmuSyncInit(&cpu->sync, pwm, freq*Z80_CLOCKMUL) + Z80_CLOCKMUL/2)/Z80_CLOCKMUL; }

// reset processor
void Z80_Reset(sZ80* cpu);

// start time synchronization (= sets "time 0"; should be called before first program execution)
INLINE void Z80_SyncStart(sZ80* cpu) { EmuSyncStart(&cpu->sync); }

// execute program (start or continue, until "stop" request)
//  C code size in Flash: 42840 bytes (optimization -Os)
void Z80_Exec(sZ80* cpu);

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
INLINE void Z80_SyncTerm(int pwm) { EmuSyncTerm(pwm); }

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*Z80_CLOCKMUL is recommended to maintain.
u32 Z80_Start(sZ80* cpu, int pwm, u32 freq);

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void Z80_Stop(int pwm);

// continue emulation without restart processor
u32 Z80_Cont(sZ80* cpu, int pwm, u32 freq);

// check if emulation is running
INLINE Bool Z80_IsRunning() { return Z80_Cpu != NULL; }

#endif // USE_EMU_Z80
