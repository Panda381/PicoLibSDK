
// ****************************************************************************
//
//                              I4004 CPU Emulator
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

#if USE_EMU_I4004		// use I4004 CPU emulator

// keyboard table (convert bit to bit index)
//  Keep this table in RAM (without "const") for faster access.
u8 I4004_KBPTab[16] = {
	// only 1=0001b, 2=0010b, 4=0100b and 8=1000b are valid values
	// 0  1  2   3  4   5   6   7  8   9  10  11  12  13  14  15
	   0, 1, 2, 15, 3, 15, 15, 15, 4, 15, 15, 15, 15, 15, 15, 15 };

// current CPU descriptor (NULL = not running)
volatile sI4004* I4004_Cpu = NULL;

// reset processor
void I4004_Reset(sI4004* cpu)
{
	cpu->pc = 0;		// program counter
	cpu->src = 0;		// SRC send register
	cpu->ram_bank = 0;	// RAM bank
	cpu->stack_top = 0;	// stack top
	cpu->a = 0;		// accumulator
	cpu->carry = 0;		// carry flag
	cpu->firstlast = 0;	// first/last nibble of WPM
	cpu->stop = 0;		// stop request

	int i;
	for (i = 0; i < I4004_STACKNUM; i++) cpu->stack[i] = 0;	// stack
	for (i = 0; i < I4004_REGNUM; i++) cpu->reg[i] = 0; // registers
	for (i = 0; i < I4004_RAMDATASIZE; i++) cpu->data[i] = 0; // RAM data
	for (i = 0; i < I4004_RAMSTATSIZE; i++) cpu->stat[i] = 0; // RAM status
}

// get next program byte, increment PC and shift time
u8 NOFLASH(I4004_ProgByte)(sI4004* cpu)
{
	// read program byte
	u8 n = cpu->readrom(cpu->pc);

	// shift program counter
	cpu->pc = (cpu->pc + 1) & I4004_PCMASK;

	// shift clock counter
	cpu->sync.clock += I4004_CLOCKMUL*8;

	return n;
}

// execute program (start or continue, until "stop" request)
// Size of code of this function: 1072 code + 952 jump table = 2024 bytes
// CPU loading at 740 kHz on 133.2 MHz: used 4-5%, max. 4-5%
// CPU loading at 5 MHz on 120 MHz: used 35-45%, max. 43-90%
void FASTCODE NOFLASH(I4004_Exec)(sI4004* cpu)
{
	u8 op;

	// clear stop flag
	cpu->stop = 0;

	// program loop
	while (cpu->stop == 0)
	{
		// switch base operation code
		op = I4004_ProgByte(cpu);
		switch (op)
		{
		// NOP
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0E:
		case 0x0F:
		case 0xFE:
		case 0xFF:
			break;

		// JCN cc,a
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F:
			{
				// prepare condition
				Bool cond = False;
				if ((op & B0) != 0) if (cpu->gettest() == 0) cond = True;
				if ((op & B1) != 0) if (cpu->carry != 0) cond = True;
				if ((op & B2) != 0) if (cpu->a == 0) cond = True;

				// load jump address
				u8 n = I4004_ProgByte(cpu);

				// jump
				if ((op & B3) == 0)
				{
					// positive condition
					if (cond) cpu->pcl = n;
				}
				else
				{
					// negative condition
					if (!cond) cpu->pcl = n;
				}
			}
			break;

		// FIM rr,d ... fetch immediate data byte d into register pair rr (higher nibble into first register)
		case 0x20:
		case 0x22:
		case 0x24:
		case 0x26:
		case 0x28:
		case 0x2A:
		case 0x2C:
		case 0x2E:
			{
				u8 n = I4004_ProgByte(cpu);
				u8 r = op - 0x20;
				cpu->reg[r] = n >> 4;
				cpu->reg[r+1] = n & 0x0f;
			}
			break;

		// SRC rr ... send register control from register pair
		case 0x21:
		case 0x23:
		case 0x25:
		case 0x27:
		case 0x29:
		case 0x2B:
		case 0x2D:
		case 0x2F:
			{
				u8 r = op - 0x21;
				cpu->src = (cpu->reg[r] << 4) | cpu->reg[r+1];
			}
			break;

		// FIN rr ... fetch indirect data byte from ROM address R0R1 into register pair rr
		case 0x30:
		case 0x32:
		case 0x34:
		case 0x36:
		case 0x38:
		case 0x3A:
		case 0x3C:
		case 0x3E:
			{
				u16 a = (cpu->pc & 0xf00) | (cpu->reg[0] << 4) | cpu->reg[1]; // address in ROM
				u8 r = op - 0x30;
				u8 n = cpu->readrom(a); // read program byte
				cpu->sync.clock += I4004_CLOCKMUL*8; // shift clock counter
				cpu->reg[r] = n >> 4; // save byte HIGH into first register
				cpu->reg[r+1] = n & 0x0f; // save byte LOW into second register
			}
			break;

		// JIN rr ... jump indirect to ROM address rr
		case 0x31:
		case 0x33:
		case 0x35:
		case 0x37:
		case 0x39:
		case 0x3B:
		case 0x3D:
		case 0x3F:
			{
				u8 r = op - 0x31;
				cpu->pc = (cpu->pc & 0xf00) | (cpu->reg[r] << 4) | cpu->reg[r+1];
			}
			break;

		// JUN a (JMP a) ... jump unconditional to address a
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B:
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F:
			cpu->pc = ((u16)(op - 0x40) << 8) | I4004_ProgByte(cpu);
			break;

		// JMS a (CALL a) ... call subroutine a
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F:
			{
				u8 n = I4004_ProgByte(cpu); // read destination address LOW
				cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
				cpu->stack_top = (cpu->stack_top + 1) & I4004_STACKMASK; // increase stack pointer
				cpu->pc = ((u16)(op - 0x50) << 8) | n; // new program counter (destination address)
			}
			break;

		// INC r ... increment register r
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0x6A:
		case 0x6B:
		case 0x6C:
		case 0x6D:
		case 0x6E:
		case 0x6F:
			{
				u8 r = op - 0x60;
				u8* pr = &cpu->reg[r]; // pointer to the register
				*pr = (*pr + 1) & 0x0f; // increment register
			}
			break;

		// ISZ r,s (IJNZ r,s) ... increment r and jump if not zero to address s
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7B:
		case 0x7C:
		case 0x7D:
		case 0x7E:
		case 0x7F:
			{
				u8 r = op - 0x70;
				u8* pr = &cpu->reg[r]; // pointer to the register
				u8 d = (*pr + 1) & 0x0f; // increment register
				*pr = d; // save new value of the register
				u8 n = I4004_ProgByte(cpu); // get destination address LOW
				if (d != 0) cpu->pcl = n; // jump if not zero
			}
			break;

		// ADD r ... add register r and carry to A (A = A + r + carry)
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F:
			{
				u8 r = op - 0x80;
				u8* pr = &cpu->reg[r]; // pointer to the register
				u8 n = cpu->a + *pr + cpu->carry; // sum
				cpu->a = n & 0x0f; // save result
				cpu->carry = n >> 4; // new carry
			}
			break;

		// SUB r ... subtract register r and carry from A (A = A + ~r + ~carry)
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F:
			{
				u8 r = op - 0x90;
				u8* pr = &cpu->reg[r]; // pointer to the register
				u8 n = cpu->a + (*pr ^ 0x0f) + (cpu->carry ^ 1); // subtract
				cpu->a = n & 0x0f; // save result
				cpu->carry = n >> 4; // new carry
			}
			break;

		// LD r ... load register r to A
		case 0xA0:
		case 0xA1:
		case 0xA2:
		case 0xA3:
		case 0xA4:
		case 0xA5:
		case 0xA6:
		case 0xA7:
		case 0xA8:
		case 0xA9:
		case 0xAA:
		case 0xAB:
		case 0xAC:
		case 0xAD:
		case 0xAE:
		case 0xAF:
			{
				u8 r = op - 0xa0;
				cpu->a = cpu->reg[r];
			}
			break;

		// XCH r ... exchange register r and A
		case 0xB0:
		case 0xB1:
		case 0xB2:
		case 0xB3:
		case 0xB4:
		case 0xB5:
		case 0xB6:
		case 0xB7:
		case 0xB8:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF:
			{
				u8 r = op - 0xb0;
				u8* pr = &cpu->reg[r]; // pointer to the register
				u8 d = cpu->a;
				cpu->a = *pr;
				*pr = d;
			}
			break;

		// BBL n (RET n) ... return from subroutine with code in A
		case 0xC0:
		case 0xC1:
		case 0xC2:
		case 0xC3:
		case 0xC4:
		case 0xC5:
		case 0xC6:
		case 0xC7:
		case 0xC8:
		case 0xC9:
		case 0xCA:
		case 0xCB:
		case 0xCC:
		case 0xCD:
		case 0xCE:
		case 0xCF:
			{
				cpu->a = op - 0xc0; // return code
				cpu->stack_top = (cpu->stack_top - 1) & I4004_STACKMASK; // decrease stack pointer
				cpu->pc = cpu->stack[cpu->stack_top]; // restore program counter
			}
			break;

		// LDM n (LDI n) ... load immediate to A
		case 0xD0:
		case 0xD1:
		case 0xD2:
		case 0xD3:
		case 0xD4:
		case 0xD5:
		case 0xD6:
		case 0xD7:
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
		case 0xDC:
		case 0xDD:
		case 0xDE:
		case 0xDF:
			cpu->a = op - 0xd0;
			break;

		// WRM ... write A into RAM memory
		case 0xE0:
			cpu->data[cpu->src_rambank] = cpu->a;
			break;

		// WMP ... write A into RAM port
		case 0xE1:
			{
				u8 i = (cpu->src >> 6) | (cpu->ram_bank << 2);
				cpu->writeramport(i, cpu->a);
			}
			break;

		// WRR ... write A into ROM port
		case 0xE2:
			{
				u8 i = (cpu->src >> 4);
				cpu->writeromport(i, cpu->a);
			}
			break;

		// WPM ... write program memory from (I4004_RPMADDR+SRC) address, firstlast nibble
		case 0xE3:
			{
				u8 n = cpu->readrom(I4004_RPMADDR + cpu->src);
				u8 f = cpu->firstlast;
				if (f == 0)
					n = (n & 0x0f) | (cpu->a << 4);
				else
					n = (n & 0xf0) | cpu->a;
				cpu->firstlast = f ^ 1;
				cpu->writerom(I4004_RPMADDR + cpu->src, n);
			}
			break;

		// WR0/WR1/WR2/WR3 ... write A into RAM status 0/1/2/3
		case 0xE4:
		case 0xE5:
		case 0xE6:
		case 0xE7:
			{
				int i = ((cpu->src_rambank >> 2) & ~3) | (op & 3);
				cpu->stat[i] = cpu->a;
			}
			break;

		// SBM ... subtract RAM and carry from A (A = A + ~m + ~carry)
		case 0xE8:
			{
				u8 n = cpu->data[cpu->src_rambank];
				n = cpu->a + (n ^ 0x0f) + (cpu->carry ^ 1); // subtract
				cpu->a = n & 0x0f; // save result
				cpu->carry = n >> 4; // new carry
			}
			break;

		// RDM ... read A from RAM memory
		case 0xE9:
			cpu->a = cpu->data[cpu->src_rambank];
			break;

		// RDR ... read ROM port into A
		case 0xEA:
			{
				u8 i = (cpu->src >> 4);
				cpu->a = cpu->readromport(i);
			}
			break;

		// ADM ... add RAM and carry to A (A = A + m + carry)
		case 0xEB:
			{
				u8 n = cpu->data[cpu->src_rambank];
				n = cpu->a + n + cpu->carry; // add
				cpu->a = n & 0x0f; // save result
				cpu->carry = n >> 4; // new carry
			}
			break;

		// RD0/RD1/RD2/RD3 ... read A from RAM status 0/1/2/3
		case 0xEC:
		case 0xED:
		case 0xEE:
		case 0xEF:
			{
				int i = ((cpu->src_rambank >> 2) & ~3) | (op & 3);
				cpu->a = cpu->stat[i];
			}
			break;

		// CLB ... clear both A and carry
		case 0xF0:
			cpu->a = 0;
		// CLC ... clear carry
		case 0xF1:
			cpu->carry = 0;
			break;

		// IAC ... increment A
		case 0xF2:
			{
				u8 n = cpu->a + 1; // increment
				cpu->a = n & 0x0f; // save result
				cpu->carry = n >> 4; // new carry
			}
			break;

		// CMC ... complement carry
		case 0xF3:
			cpu->carry = cpu->carry ^ 1;
			break;

		// CMA ... complement A
		case 0xF4:
			cpu->a = cpu->a ^ 0x0f;
			break;

		// RAL ... rotate A left through carry (carry<-a3<-a2<-a1<-a0<-carry)
		case 0xF5:
			{
				u8 n = (cpu->a << 1) | cpu->carry;
				cpu->a = n & 0x0f; // save result
				cpu->carry = n >> 4; // new carry
			}
			break;

		// RAR ... rotate A right through carry (carry->a3->a2->a1->a0->carry)
		case 0xF6:
			{
				u8 n = cpu->a;
				n |= (cpu->carry << 4);
				cpu->carry = n & 1;
				cpu->a = n >> 1;
			}
			break;

		// TCC ... transmit carry to A, clear carry
		case 0xF7:
			cpu->a = cpu->carry;
			cpu->carry = 0;
			break;

		// DAC ... decrement A
		case 0xF8:
			{
				u8 n = cpu->a + 15;
				cpu->a = n & 0x0f; // save result
				cpu->carry = n >> 4; // new carry
			}
			break;

		// TCS ... transfer carry subtract to A (0->9 or 1->10), clear carry
		case 0xF9:
			cpu->a = 9 + cpu->carry;
			cpu->carry = 0;
			break;

		// STC ... set carry
		case 0xFA:
			cpu->carry = 1;
			break;

		// DAA ... decimal adjust A (add 6 if A>9 or carry=1 and set carry to 1; or carry not affected otherwise)
		case 0xFB:
			if ((cpu->carry != 0) || (cpu->a > 9))
			{
				u8 n = cpu->a + 6;
				cpu->a = n & 0x0f; // save result
				if (n > 0x0f) cpu->carry = 1; // carry is not reset if d <= 0x0f
			}
			break;

		// KBP ... keyboard process (A change "1 of 4" to bit number, or 15 if invalid)
		case 0xFC:
			cpu->a = I4004_KBPTab[cpu->a];
			break;

		// DCL ... designate command line (high byte of RAM address)
		case 0xFD:
			cpu->ram_bank = cpu->a & 7;
			break;
		}

		// time synchronization
		EmuSync(&cpu->sync);
	}
}

// execute I4004 emulator on CPU 1
static void I4004_ExecCore1()
{
	// start time synchronization
	I4004_SyncStart((sI4004*)I4004_Cpu);

	// execute
	I4004_Exec((sI4004*)I4004_Cpu);

	// clear descriptor
	I4004_Cpu = NULL;
	dsb();
}

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I4004_CLOCKMUL is recommended to maintain.
u32 I4004_Start(sI4004* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I4004_Cpu != NULL) I4004_Stop(pwm);

	// initialize time synchronization
	freq = I4004_SyncInit(cpu, pwm, freq);

	// reset processor
	I4004_Reset(cpu);

	// current CPU descriptor
	I4004_Cpu = cpu;
	dsb();

	// execute program
#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	if (DispHstxRunning)
		DispHstxCore1Exec(I4004_ExecCore1);
	else
		Core1Exec(I4004_ExecCore1);
#elif USE_DISPHSTX			// 1=use HSTX Display driver
	DispHstxCore1Exec(I4004_ExecCore1);
#elif USE_MINIVGA			// use mini-VGA display
	VgaCore1Exec(I4004_ExecCore1);
#else
	Core1Exec(I4004_ExecCore1);
#endif

	return freq;
}

// continue emulation without reset processor
u32 I4004_Cont(sI4004* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I4004_Cpu != NULL) I4004_Stop(pwm);

	// initialize time synchronization
	freq = I4004_SyncInit(cpu, pwm, freq);

	// current CPU descriptor
	I4004_Cpu = cpu;
	dsb();

	// execute program
#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	if (DispHstxRunning)
		DispHstxCore1Exec(I4004_ExecCore1);
	else
		Core1Exec(I4004_ExecCore1);
#elif USE_DISPHSTX			// 1=use HSTX Display driver
	DispHstxCore1Exec(I4004_ExecCore1);
#elif USE_MINIVGA			// use mini-VGA display
	VgaCore1Exec(I4004_ExecCore1);
#else
	Core1Exec(I4004_ExecCore1);
#endif

	return freq;
}

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I4004_Stop(int pwm)
{
	// get CPU descriptor
	sI4004* cpu = (sI4004*)I4004_Cpu;

	// check if emulation is running
	if (cpu == NULL) return;

	// request to stop emulation
	cpu->stop = 1;

	// wait to complete
	while (I4004_Cpu != NULL) { dsb(); }

	// terminate time synchronization (stop PWM counter)
	I4004_SyncTerm(pwm);

#if !USE_MINIVGA && !USE_DISPHSTX	// use mini-VGA display

#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	if (!DispHstxRunning)
#endif

	// core 1 reset
	Core1Reset();
#endif
}

#endif // USE_EMU_I4004
