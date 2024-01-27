
// ****************************************************************************
//
//                              I8008 CPU Emulator
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

// I8008 CPU speed: 500 kHz, one T-state is 2 clock period, base instruction (5 T-states) takes 20 us
// I8008-1 CPU speed: 800 kHz, one T-state is 2 clock period, base instruction (5 T-states) takes 12.5 us

#if USE_EMU_I8008	// use I8008 CPU emulator

// constants
#define I8008_CLOCKMUL	8	// clock multiplier (to achieve lower frequencies and finer timing)
#define I8008_STACKNUM	8	// number of entries in the stack ... must be power of 2
#define I8008_STACKMASK	(I8008_STACKNUM-1) // stack index mask
#define I8008_MEMSIZE	0x4000	// memory size
#define I8008_ADDRMASK	0x3fff	// address mask

// flags (order of flags must match order of conditions in instructions)
#define I8008_C_BIT		0	// carry (hardcoded - must be bit 0)
#define I8008_Z_BIT		1	// zero
#define I8008_S_BIT		2	// sign
#define I8008_P_BIT		3	// parity (0=odd parity, 1=even parity)

#define I8008_C		BIT(I8008_C_BIT)	// 0x01 carry
#define I8008_Z		BIT(I8008_Z_BIT)	// 0x02 zero
#define I8008_S		BIT(I8008_S_BIT)	// 0x04 sign
#define I8008_P		BIT(I8008_P_BIT)	// 0x08 parity

// I8008 CPU descriptor
// - Optimization of thumb1 on RP2040: offset of byte should be <= 31, word <= 62, dword <= 124
// - Entries should be aligned
typedef struct {
	sEmuSync	sync;		// 0x00: time synchronization
	union { u16 pc; struct { u8 pcl, pch; }; }; // 0x08: 14-bit program counter PC
	u8		stack_top;	// 0x0A: stack top (= index of current program counter) ... masked with I8008_STACKMASK
	volatile u8	stop;		// 0x0B: 1=request to stop (pause) program execution
	union {
		struct {
			union { u16 hl; struct { u8 l, h; }; };	// 0x0C: registers H (high) and L (low)
			union { u16 de; struct { u8 e, d; }; };	// 0x0E: registers D (high) and E (low)
			union { u16 bc; struct { u8 c, b; }; };	// 0x10: registers B (high) and C (low)
			union { u16 fa; struct { u8 a, f; }; };	// 0x12: registers F (high) and A (low)
		};
		u8	reg[8];		// registers accessible via index (0:L, 1:H, 2:E, 3:D, 4:C, 5:B, 6:A, 7:F)
	};
	u8		halted;		// 0x14: 1=CPU is halted (HALT instruction)
	volatile u8	intreq;		// 0x15: 1=interrupt request, execute instruction RST n
	volatile u8	intins;		// 0x16: instruction RST n to execute during interrupt
	u8		res;		// 0x17: ... reserved (align)
	u16		stack[I8008_STACKNUM];	// 0x18 [16=0x10]: 14-bit stack
	pEmu16Read8	readmem;	// 0x28: read byte from memory
	pEmu16Write8	writemem;	// 0x2C: write byte to memory
	pEmu8Read8	readport;	// 0x30: read byte from port 0..7
	pEmu8Write8	writeport;	// 0x34: write byte to port 8..31
} sI8008;

STATIC_ASSERT(sizeof(sI8008) == 0x38, "Incorrect sI8008!");

// current CPU descriptor (NULL = not running)
extern volatile sI8008* I8008_Cpu;

// initialize I8008 tables
void I8008_InitTab();

// initialize time synchronization (initialize PWM counter; returns real emulated frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8008_CLOCKMUL is recommended to maintain.
INLINE u32 I8008_SyncInit(sI8008* cpu, int pwm, u32 freq) { return (EmuSyncInit(&cpu->sync, pwm, freq*I8008_CLOCKMUL) + I8008_CLOCKMUL/2)/I8008_CLOCKMUL; }

// reset processor
void I8008_Reset(sI8008* cpu);

// start time synchronization (= sets "time 0"; should be called before first program execution)
INLINE void I8008_SyncStart(sI8008* cpu) { EmuSyncStart(&cpu->sync); }

// execute program (start or continue, until "stop" request)
// Size of code of this function: 1576 code + 1024 jump table = 2600 bytes
// CPU loading at 500 kHz on 120 MHz: used 2%, max. 2-5%
// CPU loading at 5 MHz on 120 MHz: used 25-29%, max. 25-50%
void FASTCODE NOFLASH(I8008_Exec)(sI8008* cpu);

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
INLINE void I8008_SyncTerm(int pwm) { EmuSyncTerm(pwm); }

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8008_CLOCKMUL is recommended to maintain.
u32 I8008_Start(sI8008* cpu, int pwm, u32 freq);

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I8008_Stop(int pwm);

// continue emulation without restart processor
u32 I8008_Cont(sI8008* cpu, int pwm, u32 freq);

// check if emulation is running
INLINE Bool I8008_IsRunning() { return I8008_Cpu != NULL; }

#endif // USE_EMU_I8008
