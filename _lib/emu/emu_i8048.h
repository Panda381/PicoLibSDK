
// ****************************************************************************
//
//                             I8048 CPU Emulator
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

// I8048 ... 6 MHz, 1 KB ROM, 3 KB external ROM, 64 B RAM (or 256 B external RAM), 27-bit I/O, 1x 8-bit counter
// Clock is divided by 15 steps of one instruction cycle - it gives 2.5 us per 1-byte instruction on 6 MHz
//    1-byte instruction (1 cycle, 15 ticks): 2.5 us, 2-byte instruction (2 cycles, 30 ticks): 5.0 us

// Memory:
//   All locations are indirectly addressable through R1 and R0 (or R1' and R0')
// ROM 2 KB bank 0 + 2 KB bank 1

// RAM 64 bytes:
//  address 0..7: working registers R0..R7 of register bank 0
//  address 8..23: 8-level stack or user RAM (16 bytes)
//  address 24..31: working registers R0'..R7' of register bank 1

// During CALL or interrupt:
//   PC bits 0..7 are stored into R8
//   PC bits 8..11 are stored into bits 0..3 of R9
//   PSW bits 4..7 are stored into bits 4..7 of R9

// Expander I/O 8243:
//  - 4 directly addressable 4-bit ports P4..P7
//  - PROG pin provides clock: 1->0 address and control on P20-P23
//  - P20-P23 (pins 0 to 3 of P2) address and data

#if USE_EMU_I8048	// use I8048 CPU emulator

// constants
#define I8048_CLOCKMUL		8		// clock multiplier (to achieve lower frequencies and finer timing)
#define I8048_ADDRMASK		0x7ff		// address mask (11 bits in PC)
#define I8048_BANKMASK		0x800		// bank mask (bit 11 in PC)
#define I8048_ROMSIZE		0x1000		// ROM memory size (4 KB)
#define I8048_STACKNUM		8		// number of entries in the stack
#define I8048_STACKMASK		(I8048_STACKNUM-1) // stack index mask (= 7)
#define I8048_RAMSIZE		64		// RAM memory size
#define I8048_RAMMASK		(I8048_RAMSIZE-1) // RAM address mask (= 0x3F)

// flags
#define I8048_C			B7	// carry
#define I8048_AC		B6	// auxiliary carry
#define I8048_F0		B5	// user flag
#define I8048_BS		B4	// bank select
#define I8048_1			B3	// always 1

#define I8080_FLAGMASK	(B4+B5+B6+B7)	// mask of valid flags

// 8243 expander operations
#define I8048_EXP_READ		0
#define I8048_EXP_WRITE		1
#define I8048_EXP_OR		2
#define I8048_EXP_AND		3

// counter mode
#define I8048_CNT_STOP		0	// stop
#define I8048_CNT_TIMER		1	// timer
#define I8048_CNT_COUNT		2	// counter (count 1->0 edges on T1 pin)

// I8048 CPU descriptor
// - Optimization of thumb1 on RP2040: offset of byte should be <= 31, word <= 62, dword <= 124
// - Entries should be aligned
typedef struct {
	sEmuSync	sync;		// 0x00: time synchronization
	union { u16 pc; struct { u8 pcl, pch; }; }; // 0x08: 11-bit program counter PC, bit 11: select memory bank 0 or 1
	volatile u8	stop;		// 0x0A: 1=request to stop (pause) program execution
	u8		a;		// 0x0B: accumulator
	u8		psw;		// 0x0C: program status word
					//   B0..B2: S0..S2 stack pointer
					//   B3: always 1 (set on read, 0 internally in emulator)
					//   B4: BS register bank select (selects registers 0: R0..R7 or 1: R0'..R7')
					//   B5: F0 user flag 0 (auto saved and restored during CALL)
					//   B6: AC auxilliary carry
					//   B7: CY carry (modified by ADD, ADDC, RLC, CLR C, CPL C, RRC and DAA instructions)
	u8		f1;		// 0x0D: F1 state (0 or 1)
	u8		regbase;	// 0x0E: base index of registers R0..R7 (=0 for bank 0, 24 for bank 1)
	u8		a11;		// 0x0F: last A11 state (0 or 1) selected by SEL MB instruction, to pass to CALL or JMP ... not during IRQ
	u8		inirq;		// 0x10: 1=IRQ is in progress
	u8		ie;		// 0x11: 1=interrupt enabled
	volatile u8	intreq;		// 0x12: 1=interrupt request (external interrupt signal is LOW = 0)
	u8		tcntie;		// 0x13: 1=timer/counter interrupt enabled
	u8		tcntreq;	// 0x14: 1=timer/counter interrupt request (= overflow)
	u8		ent0clk;	// 0x15: 1=enable test 0 clock output
	u8		tcnt;		// 0x16: timer/counter
	u8		tcntmode;	// 0x17: timer/counter mode I8048_CNT_*
	u8		lastport[4];	// 0x18: last output to the port
	u8		lastt1;		// 0x1C: last T1 state 
	u8		prescaler;	// 0x1D: timer prescaler (timer is incremented every 32 instruction cycles)
	u8		res[2];		// 0x1E: ... reserved (align)
	u8		ram[I8048_RAMSIZE]; // 0x20 [64=0x40]: RAM memory (with registers)
	pEmu16Read8	readrom;	// 0x60: read byte from ROM memory
	pEmu8Read8	readext;	// 0x64: read byte from extended data memory
	pEmu8Write8	writeext;	// 0x68: write byte to extended data memory
	pEmu8Read8	readport;	// 0x6C: read byte from port (0=BUS, 1=P1, 2=P2, 3=P3 emulator internal)
	pEmu8Write8	writeport;	// 0x70: write byte to port (0=BUS, 1=P1, 2=P2, 3=P3 emulator internal)
	pEmuWrite8	writeprog;	// 0x74: output PROG signal (0 or 1)
	pEmu8Read8	readtest;	// 0x78: read test signal 0 or 1 (returns 0 or 1)
} sI8048;

STATIC_ASSERT(sizeof(sI8048) == 0x7C, "Incorrect sI8048!");

// current CPU descriptor (NULL = not running)
extern volatile sI8048* I8048_Cpu;

// initialize time synchronization (initialize PWM counter; returns real emulated frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8048_CLOCKMUL is recommended to maintain.
INLINE u32 I8048_SyncInit(sI8048* cpu, int pwm, u32 freq) { return (EmuSyncInit(&cpu->sync, pwm, freq*I8048_CLOCKMUL) + I8048_CLOCKMUL/2)/I8048_CLOCKMUL; }

// reset processor
void I8048_Reset(sI8048* cpu);

// start time synchronization (= sets "time 0"; should be called before first program execution)
INLINE void I8048_SyncStart(sI8048* cpu) { EmuSyncStart(&cpu->sync); }

// execute program (start or continue, until "stop" request)
// Size of code of this function: 3344 code + 1024 jump table = 4368 bytes
// CPU loading at 6 MHz on 120 MHz: used 25-30%, max. 27-35%
void FASTCODE NOFLASH(I8048_Exec)(sI8048* cpu);

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
INLINE void I8048_SyncTerm(int pwm) { EmuSyncTerm(pwm); }

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8048_CLOCKMUL is recommended to maintain.
u32 I8048_Start(sI8048* cpu, int pwm, u32 freq);

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I8048_Stop(int pwm);

// continue emulation without restart processor
u32 I8048_Cont(sI8048* cpu, int pwm, u32 freq);

// check if emulation is running
INLINE Bool I8048_IsRunning() { return I8048_Cpu != NULL; }

#endif // USE_EMU_I8048
