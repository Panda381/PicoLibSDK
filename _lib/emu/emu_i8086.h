
// ****************************************************************************
//
//                      I8088/I8086/I80186/I80188 CPU Emulator
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

// I8086 CPU speed: 5 to 10 MHz
// I8088 CPU speed: 4.77 to 9 MHz
// I80186 CPU speed: 6 MHz
//    CPU frequency = NTSC subcarrier frequency * 4 / 3 = 3.579545 MHz * 4 / 3 = 4.772727 MHz

/* ==== SYSINFO test detects this CPU as 8088 - 8-bit bus check does not pass:

1) check 8088+8086+V20+V30 / 80186..:
8088+8086+V20+V30 : flags bits 12..15 are set to 1 and cannot be changed

	pushf			; push flags
	xor	ax,ax		; AX <- 0
	push	ax		; 0 -> stack
	popf			; F <- 0
	pushf			; F -> stack
	pop	ax		; AX <- F
	popf			; pop flags
	and	ax,0f000h	; check bits 12 to 15
	cmp	ax,0f000h	; are all bits set to 1?
	jne	testpro2	; bits are not all 1s - CPU 80186 or later

2) check 8088+8086 / V20+V30:
8088+8086 : MUL instruction clears the ZF flag
V20+V30 : MUL instruction leaves the ZF flag unchanged

	xor	al,al		; set ZF flag
	mov	al,40h		; number to multiply
	mul	al		; multiply 40h*40h
	mov	ax,1		; 1 = 8088,8086 CPU
	jnz	testpro1	; ZF was changed to 0, this is 8088, 8086 CPU
	mov	ax,3		; ZF flag remains set to 1, this is V20, V30 CPU
testpro1:

3) check 8-bit bus (8088+V20) / 16-bit bus (8086+V30)
8088+V20 : has 4-byte prefetch queue
8086+V30 : has 6-byte prefetch queue
Emulator does not have prefetch queue - it is detected as 8088

	push	ax		; save CPU code (1=8088+8086, 3=V20+V30)
	EVEN			; align to even address
	mov	di,offset testproa ; prepare pointer to test of check code
	std			; direction down
	mov	al,byte ptr cs:[testpro9]; load instruction code STI -> AL
	mov	cx,3		; number of bytes to test
	cli			; disable interrupts
	rep	stosb		; overwrite 3 instructions (NOP, INX CX, STI)
	cld			; default direction up
	nop			; cache delay
	nop			; cache delay
testpro8:nop			; start of rewrite 3 instructions STI
	inc	cx		; 8086: this instruction INC CX stays active, 8088: here will be STI instruction
testpro9:sti			; last instruction to rewrite
testproa:sti			; enable interrupts again
	pop	ax		; return CPU code
	add	ax,cx		; correction CPU code (16-bit bus CX = 1, 8-bit bus CX = 0)

*/

#if USE_EMU_I8086		// use I8086/I8088/I80186/I80188 CPU emulator

// code modifications
#ifndef I8086_CPU_INTEL
#define I8086_CPU_INTEL		1	// 1=use Intel vendor alternative
#endif

#ifndef I8086_CPU_AMD
#define I8086_CPU_AMD		0	// 1=use AMD (and partially NEC) vendor alternative
#endif

#ifndef I8086_CPU_8088
#define I8086_CPU_8088		0	// 1=use 8088/80188 timings limitations
#endif

#ifndef I8086_CPU_80186
#define I8086_CPU_80186		0	// 1=use 80186 extensions
#endif

#if I8086_CPU_INTEL && I8086_CPU_AMD
#error Do not use Intel and AMD at the same time
#endif

// constants
#define I8086_CLOCKMUL	1	// clock multiplier (to achieve lower frequencies and finer timing)

// flags
//   Unused bits are set to 0, only bits 1,12,13,14,15 are set to 1.
#define I8086_CF_BIT		0	// carry flag
#define I8086_PF_BIT		2	// parity flag (0=odd parity, 1=even parity)
#define I8086_AF_BIT		4	// auxiliary flag, carry from bit 3 to bit 4
#define I8086_ZF_BIT		6	// zero flag
#define I8086_SF_BIT		7	// sign flag
#define I8086_TF_BIT		8	// trap flag (INT 1)
#define I8086_IF_BIT		9	// interrupt flag (enable external interrupt)
#define I8086_DF_BIT		10	// direction flag (0=increment, 1=decrement)
#define I8086_OF_BIT		11	// overflow flag

#define I8086_CF		B0	// (0x001) carry flag
#define I8086_PF		B2	// (0x004) parity flag (0=odd parity, 1=even parity)
#define I8086_AF		B4	// (0x010) auxiliary flag, carry from bit 3 to bit 4
#define I8086_ZF		B6	// (0x040) zero flag
#define I8086_SF		B7	// (0x080) sign flag
#define I8086_TF		B8	// (0x100) trap flag (INT 1, single step)
#define I8086_IF		B9	// (0x200) interrupt flag (enable external interrupt)
#define I8086_DF		B10	// (0x400) direction flag (0=increment, 1=decrement)
#define I8086_OF		B11	// (0x800) overflow flag

#define I8086_FLAGHW		(B15+B14+B13+B12+B1) // hardware flags, cannot be changed
#define I8086_FLAGDEF		I8086_FLAGHW	// default flags
#define I8086_FLAGALL		(I8086_OF|I8086_SF|I8086_ZF|I8086_AF|I8086_PF|I8086_CF) // (0x8D5) all arithmetic flags
#define I8086_FLAGALLCTRL	(I8086_FLAGALL|I8086_TF|I8086_IF|I8086_DF) // (0xFD5) all flags, including controls

// instruction memory addressing mode "mod": bit 7 and 6 of opcode
#define I8086_MODE_0		0	// memory addressing, no offset
#define I8086_MODE_8		1	// memory addressing, offset signed byte (1 byte), signed -128..+127
#define I8086_MODE_16		2	// memory addressing, offset unsigned word (2 bytes)
#define I8086_MODE_REG		3	// register addressing

// register array "reg": bit 3..5 of opcode, w=0 if 8-bit, w=1 if 16-bit
#define I8086_REG_AL_AX		0	// AL or AX
#define I8086_REG_CL_CX		1	// CL or CX
#define I8086_REG_DL_DX		2	// DL or DX
#define I8086_REG_BL_BX		3	// BL or BX
#define I8086_REG_AH_SP		4	// AH or SP
#define I8086_REG_CH_BP		5	// CH or BP
#define I8086_REG_DH_SI		6	// DH or SI
#define I8086_REG_BH_DI		7	// BH or DI

// mod in 2nd byte of instruction (ModR/M byte):
//  bit 7,6: mod
//  bit 3..5: reg
//  bit 0..2: r/m

// mapping array "r/m": bit 0..2 of opcode:
//  	mode 0		mode 1			mode 2			mode 3, w=0	mode 3, w=1
//  0	BX + SI (7)	BX + SI + off8 (11)	BX + SI + off16 (11)	AL		AX
//  1	BX + DI (8)	BX + DI + off8 (12)	BX + DI + off16 (12)	CL		CX
//  2	BP + SI	(8)	BP + SI + off8 (12)	BP + SI + off16 (12)	DL		DX
//  3	BP + DI	(7)	BP + DI + off8 (11)	BP + DI + off16 (11)	BL		BX
//  4	SI (5)		SI + off8 (9)		SI + off16 (9)		AH		SP
//  5	DI (5)		DI + off8 (9)		DI + off16 (9)		CH		BP
//  6	address16 (6)	BP + off8 (9)		BP + off16 (9)		DH		SI
//  7	BX (5)		BX + off8 (9)		BX + off16 (9)		BH		DI

// segment override prefix: +2 cycles
// default segments: CS:IP, DS:BX, SS:BP, DS:SI, DS:DI, string destination ES:DI

// Addressing 16-bit registers: 0 AX, 1 CX, 2 DX, 3 BX, 4 SP, 5 BP, 6 SI, 7 DI
// Addressing 8-bit registers: 0 AL, 1 CL, 2 DL, 3 BL, 4 AH, 5 CH, 6 DH, 7 BH
// Addresing segment registers: 0 ES, 1 CS, 2 SS, 3 DS, ... undocumented: 4 ES, 5 CS, 6 SS, 7 DS

// index of segment register
#define I8086_SEG_ES	0	// ES
#define I8086_SEG_CS	1	// CS
#define I8086_SEG_SS	2	// SS
#define I8086_SEG_DS	3	// DS

#define I8086_SEG_NO	8	// no segment prefix

// repeat flag
#define I8086_REP_NO	0	// no repeat
#define I8086_REP_Z	1	// REP or REPZ
#define I8086_REP_NZ	2	// REPNZ

// some instructions
#define I8086_INS_NOP	0x90	// instruction NOP
#define I8086_INS_IRET	0xCF	// instruction IRET

// IBM PC hardware interrupt control (register in sEmuSync)
// - on change, update interrupt service in I8086_Exec ("find INT")
#define I8086_IRQ_INT08	0	// IRQ0, INT 08h, system clock (18.2 Hz)
#define I8086_IRQ_INT09	1	// IRQ1, INT 09h, keyboard interrupt
#define I8086_IRQ_INT0A	2	// IRQ2, INT 0Ah, EGA vertical retrace, cascade interrupt from interrupt controller #2
#define I8086_IRQ_INT0B	3	// IRQ3, INT 0Bh, COM2
#define I8086_IRQ_INT0C	4	// IRQ4, INT 0Ch, COM1
#define I8086_IRQ_INT0D	5	// IRQ5, INT 0Dh, XT hard disk, AT LPT2
#define I8086_IRQ_INT0E	6	// IRQ6, INT 0Eh, floppy disk controller
#define I8086_IRQ_INT0F	7	// IRQ7, INT 0Fh, LPT1

#define I8086_IRQ_INT1B	8	// INT 1Bh - raised from INT 09h
#define I8086_IRQ_INT1C	9	// INT 1Ch - raised from INT 08h

#define I8086_IRQ_INT23	10	// INT 23h - raised from INT 21h to break program

#define I8086_IRQ_INT70	11	// IRQ8, INT 70h, CMOS timer (1024 Hz)
#define I8086_IRQ_INT71	12	// IRQ9, INT 71h, redirect to INT 0Ah
#define I8086_IRQ_INT72	13	// IRQ10, INT 72h
#define I8086_IRQ_INT73	14	// IRQ11, INT 73h
#define I8086_IRQ_INT74	15	// IRQ12, INT 74h, pointing device PS
#define I8086_IRQ_INT75	16	// IRQ13, INT 75h, mathematic coprocessor exception
#define I8086_IRQ_INT76	17	// IRQ14, INT 76h, AT hard disk controller
#define I8086_IRQ_INT77	18	// IRQ15, INT 77h, secondary IDE controller

#define I8086_IRQ_ALL	0x7ffff	// mask of all IRQs

// I8086 CPU descriptor
// - Optimization of thumb1 on RP2040: offset of byte should be <= 31, word <= 62, dword <= 124
// - Entries should be aligned
typedef struct {
	sEmuSync	sync;		// 0x00: time synchronization
	u8		segpref;	// 0x08: index of segment prefix for current instruction (I8086_SEG_NO if no prefix)
	u8		segpref_next;	// 0x09: index of segment prefix for next instruction (I8086_SEG_NO if no prefix)
	u8		reppref;	// 0x0A REP prefix flag I8086_REP_* (internal flag - REP prefix has been applied ... IMUL and IDIV uses this flag to track sign)
	u8		reppref_next;	// 0x0B: next REP prefix flag
	u8		modrm;		// 0x0C: mode byte r/m
	volatile u8	stop;		// 0x0D: 1=request to stop (pause) program execution
	u8		src8;		// 0x0E: 8-bit source operand
	u8		dst8;		// 0x0F: 8-bit destination operand
	u8		waiting;	// 0x10: waiting if TEST pin is HIGH (WAIT instruction)
	volatile u8	test;		// 0x11: TEST pin state (0=active, WAIT continues, 1=not active, WAIT halts and wait)
	u8		tid;		// 0x12: >0 temporary interrupt disable
	u8		halted;		// 0x13: 1=CPU is halted (HLT instruction)
	u8		res[4];		// 0x14: ... reserved (align)

// optimization: registers AL..BH should be at offset < 0x20 (starting at max. 0x18), registers SP..DI can have offset >= 0x20

	union {
		// 16-bit registers must have same order as in instruction, mode 3, w=1
		struct {
			union { volatile u16 ax; struct { volatile u8 al, ah; }; };	// 0x18: registers AH (high) and AL (low)
			union { volatile u16 cx; struct { volatile u8 cl, ch; }; };	// 0x1A: registers CH (high) and CL (low)
			union { volatile u16 dx; struct { volatile u8 dl, dh; }; };	// 0x1C: registers DH (high) and DL (low)
			union { volatile u16 bx; struct { volatile u8 bl, bh; }; };	// 0x1E: registers BH (high) and BL (low)

			volatile u16	sp;	// 0x20: stack pointer SP
			volatile u16	bp;	// 0x22: base pointer BP
			volatile u16	si;	// 0x24: source index SI
			volatile u16	di;	// 0x26: destination index DI
		};

		volatile u16	reg16[8];	// 0x18: 16-bit registers indexed

		volatile u8	reg8[16];	// 0x18: 8-bit registers indexed
	};

	u16		src16;		// 0x28: 16-bit source operand
	u16		dst16;		// 0x2A: 16-bit destination operand
	volatile u16	ip;		// 0x2C: instruction pointer IP
	volatile u16	flags;		// 0x2E: flags
	u16		eoff;		// 0x30: offset of effective address (undocumented: LEA with mod == 3 returns previous effective address)
	u16		eseg;		// 0x32: segment of effective address
	u16		res2[2];	// 0x34: ... reserved

// optimization: registers ES..DS should be at offset < 0x40 (starting at max. 0x38)

	union {
		// segment registers must have same order as in instruction
		struct {
			volatile u16	es;	// 0x38: extra segment ES
			volatile u16	cs;	// 0x3A: code segment CS
			volatile u16	ss;	// 0x3C: stack segment SS
			volatile u16	ds;	// 0x3E: data segment DS
		};

		volatile u16	regseg[4];	// 0x38: segment registers indexed
	};

	pEmu32Read8	readmem;	// 0x40: read byte from memory
	pEmu32Write8	writemem;	// 0x44: write byte to memory
	pEmu16Read8	readport8;	// 0x48: read byte from port
	pEmu16Write8	writeport8;	// 0x4C: write byte to port
	pEmu16Read16	readport16;	// 0x50: read word from port
	pEmu16Write16	writeport16;	// 0x54: write word to port
} sI8086;

STATIC_ASSERT(sizeof(sI8086) == 0x58, "Incorrect sI8086!");

// current CPU descriptor (NULL = not running)
extern volatile sI8086* I8086_Cpu;

// initialize I8086 tables
void I8086_InitTab();

// initialize time synchronization (initialize PWM counter; returns real emulated frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 64 kHz to 12 MHz)
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8086_CLOCKMUL is recommended to maintain.
INLINE u32 I8086_SyncInit(sI8086* cpu, int pwm, u32 freq) { return (EmuSyncInit(&cpu->sync, pwm, freq*I8086_CLOCKMUL) + I8086_CLOCKMUL/2)/I8086_CLOCKMUL; }

// reset processor
void I8086_Reset(sI8086* cpu);

// start time synchronization (= sets "time 0"; should be called before first program execution)
INLINE void I8086_SyncStart(sI8086* cpu) { EmuSyncStart(&cpu->sync); }

// execute program (start or continue, until "stop" request)
// 8086: Size of code of this function and subfunctions: 12752 code + 1356 jump table = 14108 bytes
// 80186: Size of code of this function and subfunctions: 14472 code + 1412 jump table = 15884 bytes
// CPU loading at 4.77 MHz on 252 MHz: used 17-44%, max. 30-130%
void FASTCODE NOFLASH(I8086_Exec)(sI8086* cpu);

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
INLINE void I8086_SyncTerm(int pwm) { EmuSyncTerm(pwm); }

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8080_CLOCKMUL is recommended to maintain.
u32 I8086_Start(sI8086* cpu, int pwm, u32 freq);

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I8086_Stop(int pwm);

// continue emulation without restart processor
u32 I8086_Cont(sI8086* cpu, int pwm, u32 freq);

// check if emulation is running
INLINE Bool I8086_IsRunning() { return I8086_Cpu != NULL; }

// raise IRQ interrupt request I8086_IRQ*
INLINE void I8086_RaiseIRQ(sI8086* cpu, int irq) { EmuInterSetBit(&cpu->sync, irq); }

#endif // USE_EMU_I8086
