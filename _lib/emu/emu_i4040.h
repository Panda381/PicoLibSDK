
// ****************************************************************************
//
//                               I4040 CPU Emulator
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

// I4040 CPU speed: 740 kHz, clock period 1.35 us
// 1-word instruction 8 clock periods = 10.8 us
// 2-word instructon 16 clock periods = 21.6 us

#if USE_EMU_I4040	// use I4040 CPU emulator

// constants
#define I4040_CLOCKMUL	4	// clock multiplier (to achieve lower frequencies and finer timing)
#define I4040_STACKNUM	8	// number of entries in the stack (must be power of 2)
#define I4040_REGNUM	24	// number of registers (must be even number)
#define I4040_STACKMASK	(I4040_STACKNUM-1) // stack index mask (= 7)
#define I4040_PCMASK	0xfff	// mask of 12-bit PC register
#define I4040_RPMADDR	0xf00	// ROM high address for RPM/WPM instruction
#define I4040_ROMSIZE	0x2000	// total ROM memory size

//   1 number (1 register) = 16 digit nibbles + 4 status nibbles = 20 nibbles = 80 bits = 10 bytes
//   1 RAM chip = 320 bits = 80 nibbles = 40 bytes = 4 numbers
//   4 RAM chips = 1280 bits = 320 nibbles = 160 bytes = 16 numbers
//   8 RAM banks = 10240 bits = 2560 nibbles = 1280 bytes = 128 numbers

#define I4040_RAMBANK	8	// (3 bits) number of RAM banks (max. 8)
#define I4040_RAMCHIP	4	// (2 bits) number of RAM chips in one RAM bank
#define I4040_RAMREG	4	// (2 bits) number of RAM registers (numbers) in one RAM chip
#define I4040_RAMDATA	16	// (4 bits) number of data nibbles in one RAM register
#define I4040_RAMSTAT	4	// (2 bits) number of status nibbles in one RAM register

#define I4040_RAMDATASIZE (I4040_RAMBANK * I4040_RAMCHIP * I4040_RAMREG * I4040_RAMDATA) // size or RAM data in bytes (= 8 * 4 * 4 * 16 = 2048 bytes)
#define I4040_RAMSTATSIZE (I4040_RAMBANK * I4040_RAMCHIP * I4040_RAMREG * I4040_RAMSTAT) // size or RAM status in bytes (= 8 * 4 * 4 * 4 = 512 bytes)

//   1 ROM chip = 2048 bits = 256 bytes (= 1 page of the program)
//   Max. 32 ROM chips = 65536 bits = 8192 bytes

// function to read byte from ROM (12-bit address, ROM bank 0 or 1; address base I4040_RPMADDR = read ROM constant RPM) 
typedef u8 (*pI4040GetRom)(u16 addr, u8 bank);

// function to write byte to ROM with WPM instruction, ROM bank 0 or 1 (12-bit address >= I4040_RPMADDR; 256-byte ROM page) 
typedef void (*pI4040SetRom)(u16 addr, u8 bank, u8 val);

//   4 RAM output ports in 1 RAM bank, 32 RAM output ports in 8 RAM banks (select with DCL)
//   16 ROM input/output ports

// write nibble to port (RAM or ROM)
//  RAM addr = index of RAM chip = 5 bits = 0..31
//  ROM addr = index of ROM chip (from bank 0) = 4 bits = 0..15
typedef void (*pI4040WritePort)(u8 addr, u8 val);

// read nibble from ROM port
//  ROM addr = index of ROM chip (from bank 0) = 4 bits = 0..15
typedef u8 (*pI4040ReadPort)(u8 addr);

// get TEST signal (0=TEST is active, 1=TEST is not active)
typedef u8 (*pI4040GetTest)(void);

// I4040 CPU descriptor
// - Optimization of thumb1 on RP2040: offset of byte should be <= 31, word <= 62, dword <= 124
// - Entries should be aligned
typedef struct {
	sEmuSync	sync;		// 0x00: time synchronization
	union { u16 pc; struct { u8 pcl, pch; }; }; // 0x08: 12-bit program counter
	union {
		struct {
			u8	src;	// 0x0A: SRC send register control
					//  RAM data:
					//	bit 0..3: nibble 0..15 within the register (total 64 bits = 16 nibbles = 8 bytes)
					//	bit 4..5: register 0..3 within the RAM chip (total 256 bits = 64 nibbles = 32 bytes)
					//	bit 6..7: RAM chip 0..3 (total 1024 bits = 256 nibbles = 128 bytes)
					//  RAM status:
					//	bit 4..5: register 0..3 within the RAM chip (total 64 bits = 16 nibbles = 8 bytes)
					//	bit 6..7: RAM chip 0..3 (total 256 bits = 64 nibbles = 32 bytes)
					//  RAM output port:
					//	bit 6..7: RAM chip 0..3
					//  ROM output or input port:
					//	bit 4..7: ROM chip 0..15
					//  RPM/WPM ROM memory: byte address
			u8	ram_bank; // 0x0B: RAM bank selection (0..7; DCL designate command line, command register)
		};
		u16	src_rambank;
	};
	u16		src_rambank_save; // 0x0C: save RAM bank selection and SRC
	u8		stack_top;	// 0x0E: stack top (= index of current program counter) ... masked with I4040_STACKMASK
	u8		reg_bank;	// 0x0F: register 0..7 bank selection flip-flop (0..1)
	u8		a;		// 0x10: 4-bit accumulator
	u8		carry;		// 0x11: carry flag
	u8		halted;		// 0x12: 1=CPU is halted (HALT instruction)
	u8		ie;		// 0x13: 1=interrupt enable
	u8		rom_bank;	// 0x14: ROM bank selection (0..1)
	s8		rom_delay;	// 0x15: (must be s8!) ROM selection delay if != 0: +2 +1 select ROM bank 1, -2 -1 select ROM bank 0
	u8		firstlast;	// 0x16: 0=first (high) nibble of RPM/WPM, 1=last (low) nibbler of RPM/WPM
	volatile u8	intreq;		// 0x17: 1=interrupt request
	u8		intack;		// 0x18: 1=interrupt acknowledge (interrupt is processing, to prevent nested interrupt)
	u8		reg_bank_save;	// 0x19: save register bank selection during interrupt
	volatile u8	stop;		// 0x1A: 1=request to stop (pause) program execution
	u8		res;		// 0x1B: ... reserved (align)
	u8		reg[I4040_REGNUM]; // 0x1C [24=0x18]: working registers
					//  Register pair R01, R23, R45... first register is HIGH nibble, second register is LOW nibble
					//	0..7 = registers 0..7 of bank 0
					//	8..15 = registers 8..15
					//	16..23 = registers 0..7 of bank 1
	u16		stack[I4040_STACKNUM];	// 0x34 [16=0x10]:  12-bit stack
	pI4040GetRom	readrom;	// 0x44: read byte from ROM memory
	pI4040SetRom	writerom;	// 0x48: write byte to ROM with WPM instruction, ROM bank 0 or 1 (12-bit address >= I4040_RPMADDR; 256-byte ROM page) 
	pI4040WritePort	writeramport;	// 0x4C: write nibble to RAM port (addr = 0..31 = RAM chip)
	pI4040WritePort	writeromport;	// 0x50: write nibble to ROM port (addr = 0..15 = ROM chip from bank 0)
	pI4040ReadPort	readromport;	// 0x54: read nibble from ROM port (addr = 0..15 = ROM chip from bank 0)
	pI4040GetTest	gettest;	// 0x58: get TEST signal (0=TEST is active, 1=TEST is not active)
	u8		data[I4040_RAMDATASIZE]; // 0x5C [0x800]: (RAM data (1 byte = 1 nibble = 1 digit)
	u8		stat[I4040_RAMSTATSIZE]; // 0x85C [0x200]: RAM status (1 byte = 1 nibble = 1 digit)
} sI4040;

STATIC_ASSERT(sizeof(sI4040) == 0xA5C, "Incorrect sI4040!");

// current CPU descriptor (NULL = not running)
extern volatile sI4040* I4040_Cpu;

// initialize time synchronization (initialize PWM counter; returns real emulated frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I4040_CLOCKMUL is recommended to maintain.
INLINE u32 I4040_SyncInit(sI4040* cpu, int pwm, u32 freq) { return (EmuSyncInit(&cpu->sync, pwm, freq*I4040_CLOCKMUL) + I4040_CLOCKMUL/2)/I4040_CLOCKMUL; }

// reset processor
void I4040_Reset(sI4040* cpu);

// start time synchronization (= sets "time 0"; should be called before first program execution)
INLINE void I4040_SyncStart(sI4040* cpu) { EmuSyncStart(&cpu->sync); }

// execute program (start or continue, until "stop" request)
// Size of code of this function: 1460 code + 1016 jump table = 2476 bytes
// CPU loading at 740 kHz on 133.2 MHz: used 4-5%, max. 4-6%
// CPU loading at 5 MHz on 120 MHz: used 43-52%, max. 45-52%
void FASTCODE NOFLASH(I4040_Exec)(sI4040* cpu);

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
INLINE void I4040_SyncTerm(int pwm) { EmuSyncTerm(pwm); }

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: gettest, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I4040_CLOCKMUL is recommended to maintain.
u32 I4040_Start(sI4040* cpu, int pwm, u32 freq);

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I4040_Stop(int pwm);

// continue emulation without restart processor
u32 I4040_Cont(sI4040* cpu, int pwm, u32 freq);

// check if emulation is running
INLINE Bool I4040_IsRunning() { return I4040_Cpu != NULL; }

#endif // USE_EMU_I4040
