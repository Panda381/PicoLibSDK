
// ****************************************************************************
//
//                              I8080 CPU Emulator
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

// I8080 CPU speed: 2 MHz (instructions 4, 5, 7, 10 or 11 clock cycles)
// I808A-1 CPU speed: 3.125 MHz

#if USE_EMU_I8080		// use I8080 CPU emulator

// constants
#define I8080_CLOCKMUL	4	// clock multiplier (to achieve lower frequencies and finer timing)
#define I8080_MEMSIZE	0x10000	// memory size

// flags
#define I8080_C_BIT		0	// carry
//				1	// always 1
#define I8080_P_BIT		2	// parity (0=odd parity, 1=even parity)
//				3	// always 0
#define I8080_AC_BIT		4	// auxiliary carry from bit 3 to bit 4
//				5	// always 0
#define I8080_Z_BIT		6	// zero
#define I8080_S_BIT		7	// sign

#define I8080_C		BIT(I8080_C_BIT)	// 0x01 carry
#define I8080_P		BIT(I8080_P_BIT)	// 0x04 parity
#define I8080_AC	BIT(I8080_AC_BIT)	// 0x10 auxiliary carry
#define I8080_Z		BIT(I8080_Z_BIT)	// 0x40 zero
#define I8080_S		BIT(I8080_S_BIT)	// 0x80 sign

#define I8080_FLAGALL	(B0+B2+B4+B6+B7)	// mask of valid flags
#define I8080_FLAGINV	(B1+B3+B5)		// mask of invalid flags
#define I8080_FLAGDEF	B1			// default flags

// I8080 CPU descriptor
// - Optimization of thumb1 on RP2040: offset of byte should be <= 31, word <= 62, dword <= 124
// - Entries should be aligned
typedef struct {
	sEmuSync	sync;		// 0x00: time synchronization
	union { u16 pc; struct { u8 pcl, pch; }; }; // 0x08: program counter PC
	union { u16 sp; struct { u8 spl, sph; }; }; // 0x0A: stack pointer SP
	union {
		struct {
			union { u16 fa; struct { u8 a, f; }; };	// 0x0C: registers F (high) and A (low)
			union { u16 hl; struct { u8 l, h; }; };	// 0x0E: registers H (high) and L (low)
			union { u16 de; struct { u8 e, d; }; };	// 0x10: registers D (high) and E (low)
			union { u16 bc; struct { u8 c, b; }; };	// 0x12: registers B (high) and C (low)
		};
		u8	reg[8];		// registers accessible via index (0:A, 1:F, 2:L, 3:H, 4:E, 5:D, 6:C, 7:B)
		u16	dreg[4];	// double registers (0:FA, 1:HL, 2:DE, 3:BC)
	};
	u8		ie;		// 0x14: interrupt enable flag (0=disable interrupts, 1=enable interrupts)
	u8		tid;		// 0x15: temporary interrupt disable flag (1 instruction after enabling IE)
	u8		halted;		// 0x16: 1=CPU is halted (HALT instruction)
	volatile u8	stop;		// 0x17: 1=request to stop (pause) program execution
	volatile u8	intreq;		// 0x18: 1=interrupt request, execute instruction RST n
	volatile u8	intins;		// 0x19: instruction RST n to execute during interrupt
	u8		cond[4];	// 0x1A [4]: condition table (0:I8080_Z, 1:I8080_C, 2:I8080_P, 3:I8080_S)
	u16		res;		// 0x1E: ... reserved (align)
	pEmu16Read8	readmem;	// 0x20: read byte from memory
	pEmu16Write8	writemem;	// 0x24: write byte to memory
	pEmu8Read8	readport;	// 0x28: read byte from port
	pEmu8Write8	writeport;	// 0x2C: write byte to port
} sI8080;

STATIC_ASSERT(sizeof(sI8080) == 0x30, "Incorrect sI8080!");

// current CPU descriptor (NULL = not running)
extern volatile sI8080* I8080_Cpu;

// initialize I8080 tables
void I8080_InitTab();

// initialize time synchronization (initialize PWM counter; returns real emulated frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 64 kHz to 12 MHz)
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8080_CLOCKMUL is recommended to maintain.
INLINE u32 I8080_SyncInit(sI8080* cpu, int pwm, u32 freq) { return (EmuSyncInit(&cpu->sync, pwm, freq*I8080_CLOCKMUL) + I8080_CLOCKMUL/2)/I8080_CLOCKMUL; }

// reset processor
void I8080_Reset(sI8080* cpu);

// start time synchronization (= sets "time 0"; should be called before first program execution)
INLINE void I8080_SyncStart(sI8080* cpu) { EmuSyncStart(&cpu->sync); }

// execute program (start or continue, until "stop" request)
// Size of code of this function: 2412 code + 1024 jump table = 3436 bytes
// CPU loading at 2 MHz on 120 MHz: used 17-23%, max. 20-27%
// CPU loading at 5 MHz on 120 MHz: used 46-63%, max. 52-70%
void FASTCODE NOFLASH(I8080_Exec)(sI8080* cpu);

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
INLINE void I8080_SyncTerm(int pwm) { EmuSyncTerm(pwm); }

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8080_CLOCKMUL is recommended to maintain.
u32 I8080_Start(sI8080* cpu, int pwm, u32 freq);

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I8080_Stop(int pwm);

// continue emulation without restart processor
u32 I8080_Cont(sI8080* cpu, int pwm, u32 freq);

// check if emulation is running
INLINE Bool I8080_IsRunning() { return I8080_Cpu != NULL; }

#endif // USE_EMU_I8080
