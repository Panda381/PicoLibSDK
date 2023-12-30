
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

// https://tobiasvl.github.io/optable/intel-8080/

#if USE_EMU_I8080		// use I8080 CPU emulator

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

#define I8080_CLOCKMUL	8	// clock multiplier (to achieve lower frequencies and finer timing)

// I8080 CPU descriptor
// - Keep compatibility of the structure with the #define version in machine code!
// - Entries must be aligned
// - thumb1 on RP2040: offset of byte fast access must be <= 31, word fast access <= 62, dword fast access <= 124
typedef struct {

	sEmuSync	sync;		// 0x00: (8) time synchronization
// align
	union { u16 pc; struct { u8 pcl, pch; }; }; // 0x08: program counter PC
	union { u16 sp; struct { u8 spl, sph; }; }; // 0x0A: stack pointer SP
// align
	union { u32 afbc;
		struct {
			union { u16 af; struct { u8 f, a; }; };	// 0x0C: registers A (high) and F (low)
			union { u16 bc; struct { u8 c, b; }; };	// 0x0E: registers B (high) and C (low)
		};
	};
// align
	union { u32 dehl;
		struct {
			union { u16 de; struct { u8 e, d; }; };	// 0x10: registers D (high) and E (low)
			union { u16 hl; struct { u8 l, h; }; };	// 0x12: registers H (high) and L (low)
		};
	};
// align
	u8		ie;		// 0x14: interrupt enable flag (0=disable interrupts, 1=enable interrupts)
	u8		tid;		// 0x15: temporary interrupt disable flag (1 instruction after enabling IE)
	u8		halted;		// 0x16: 1=CPU is halted (HALT instruction)
	volatile u8	stop;		// 0x17: 1=request to stop (pause) program execution
// align
	u8		intreq;		// 0x18: 1=interrupt request, execute instruction RST n
	u8		intins;		// 0x19: instruction RST n to execute during interrupt
	u8		res, res2;	// 0x1A: ... reserved (align)
// align
	pEmu16Read8	readmem;	// 0x1C: read byte from memory
	pEmu16Write8	writemem;	// 0x20: write byte to memory
	pEmu8Read8	readio;		// 0x24: read byte from port
	pEmu8Write8	writeio;	// 0x28: write byte to port

} sI8080;

STATIC_ASSERT(sizeof(sI8080) == 0x2C, "Incorrect sI8080!");

// current CPU descriptor (NULL = not running)
extern volatile sI8080* I8080_Cpu;

// initialize I8080 tables
void I8080_InitTab();

// initialize time synchronization (initialize PWM counter; returns real emulated frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 64 kHz to 12 MHz)
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8008_CLOCKMUL is recommended to maintain.
INLINE u32 I8080_SyncInit(sI8080* cpu, int pwm, u32 freq) { return (EmuSyncInit(&cpu->sync, pwm, freq*I8080_CLOCKMUL) + I8080_CLOCKMUL/2)/I8080_CLOCKMUL; }

// reset processor
void I8080_Reset(sI8080* cpu);

// start time synchronization (= sets "time 0"; should be called before first program execution)
INLINE void I8080_SyncStart(sI8080* cpu) { EmuSyncStart(&cpu->sync); }

// execute program (start or continue, until "stop" request)
//  C code size in RAM: 8708 bytes (optimization -Os)
void NOFLASH(I8080_Exec)(sI8080* cpu);

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
INLINE void I8080_SyncTerm(int pwm) { EmuSyncTerm(pwm); }

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8008_CLOCKMUL is recommended to maintain.
u32 I8080_Start(sI8080* cpu, int pwm, u32 freq);

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I8080_Stop(int pwm);

// continue emulation without restart processor
u32 I8080_Cont(sI8080* cpu, int pwm, u32 freq);

// check if emulation is running
INLINE Bool I8080_IsRunning() { return I8080_Cpu != NULL; }

#endif // USE_EMU_I8080
