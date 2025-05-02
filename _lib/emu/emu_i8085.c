
// ****************************************************************************
//
//                              I8085 CPU Emulator
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

#if USE_EMU_I8085		// use I8085 CPU emulator

#include "emu_i8085_macro.h"

// current CPU descriptor (NULL = not running)
volatile sI8085* I8085_Cpu = NULL;

// flags table with parity (I8085_P, I8085_Z and I8085_S flags) ... keep the table in RAM, for higher speed
u8 I8085_FlagParTab[256];

// initialize I8085 tables
void I8085_InitTab()
{
	int i, j, par;
	u8 f;
	for (i = 0; i < 256; i++)
	{
		// initialize parity table
		par = 0;
		j = i;
		while (j != 0)
		{
			par ^= (j & 1);
			j >>= 1;
		}

		// set flag
		f = I8085_FLAGDEF;
		if (par == 0) f |= I8085_P; // even parity
		if (i >= 0x80) f |= I8085_S; // negative
		if (i == 0) f |= I8085_Z; // zero
		I8085_FlagParTab[i] = f;
	}
}

// reset processor
void I8085_Reset(sI8085* cpu)
{
	cpu->pc = 0;		// program counter
	cpu->sp = 0xffff;	// stack pointer
	cpu->a = 0;
	cpu->f = I8085_FLAGDEF;
	cpu->hl = 0;
	cpu->de = 0;
	cpu->bc = 0;
	cpu->intmask = B0+B1+B2+B7; // interrupt mask (interrupts are disabled, TRAP is always enabled)
	cpu->intreq = 0;	// interrupt request
	cpu->ie = 0;		// interrupt disable
	cpu->tid = 0;		// temporary interrupt disable
	cpu->halted = 0;	// halted
	cpu->stop = 0;		// stop request
	cpu->cond[0] = I8085_Z;	// condition table
	cpu->cond[1] = I8085_C;	// condition table
	cpu->cond[2] = I8085_P; // condition table
	cpu->cond[3] = I8085_S;	// condition table
	cpu->intins = 0x0D;	// interrupt instruction RST 0x08
	cpu->ie_save = 0;	// IE flag saved during TRAP interrupt
}

// load byte from program memory
INLINE u8 I8085_ProgByte(sI8085* cpu)
{
	// read program byte
	u8 n = cpu->readmem(cpu->pc);

	// compiler barrier (for the PC to be valid at the time of the callback instruction)
	cb();

	// shift program counter
	cpu->pc++;

	return n;
}

// load word from program memory
INLINE u16 I8085_ProgWord(sI8085* cpu)
{
	u8 temp = I8085_ProgByte(cpu);
	return (temp | ((u16)I8085_ProgByte(cpu) << 8));
}

// execute program (start or continue, until "stop" request)
// Size of code of this function: 3288 code + 1024 jump table = 4312 bytes
// CPU loading at 2 MHz on 120 MHz: used 15-25%, max. 20-25%
// CPU loading at 5 MHz on 120 MHz: used 45-79%, max. 50-85%
void FASTCODE NOFLASH(I8085_Exec)(sI8085* cpu)
{
	u8 op, m;

	// clear stop flag
	cpu->stop = 0;

	// program loop
	while (cpu->stop == 0)
	{
		// check interrupts
		m = cpu->intmask & cpu->intreq;
		if ((m != 0) && (cpu->tid == 0))
		{
			// continue after HALT
			if (cpu->halted != 0)
			{
				cpu->pc++;
				cpu->halted = 0;
			}

			// save and disable interrupt
			cpu->ie_save = cpu->ie;
			cpu->ie = 0; // disable interrupt
			cpu->intmask &= B0|B1|B2|B7; // disable interrupts, leave only RST mask and TRAP enable

			int addr;

			// TRAP (RST4.5) interrupt (highest priority)
			if ((m & I8085_INT_TRAP) != 0)
			{
				cpu->intreq &= ~I8085_INT_TRAP; // clear request
				addr = 0x24;
			}
			else
			{
				// clear interrupt save flag
				cpu->ie_save = 0;

				// RST7.5
				if ((m & I8085_INT_RST75) != 0)
				{
					cpu->intreq &= ~I8085_INT_RST75; // clear request
					addr = 0x3c;
				}

				// RST6.5
				else if ((m & I8085_INT_RST65) != 0)
					addr = 0x34;

				// RST5.5
				else if ((m & I8085_INT_RST65) != 0)
					addr = 0x2c;

				// INTR
				else
					addr = cpu->intins & 0x38;
			}

			// jump to service
			I8085_CALL(addr);
			cpu->sync.clock += I8085_CLOCKMUL*12;

// At this point we could put a goto to synchronize the time at the end of this function.
// However, this would slow down the code (the compiler would have to turn off optimization).
// Synchronization is not necessary here, so we omit it.
			continue;
		}

		// get next instruction
		op = I8085_ProgByte(cpu);

		// clear temporary disable flag
		cpu->tid = 0;

		// switch base operation code
		switch (op)
		{
		// NOP
		case 0x00:
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// LXI B,nn ... LD BC,nn
		case 0x01:
		// LXI D,nn ... LD DE,nn
		case 0x11:
		// LXI H,nn ... LD HL,nn
		case 0x21:
			{
				u8* r = &cpu->reg[6 - (op >> 3)]; // register low
				r[0] = I8085_ProgByte(cpu); // low
				r[1] = I8085_ProgByte(cpu); // high
			}
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// STAX B ... LD (BC),A
		case 0x02:
			cpu->writemem(cpu->bc, cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// INX B ... INC BC
		case 0x03:
		// INX D ... INC DE
		case 0x13:
		// INX H ... INC HL
		case 0x23:
			{
				u16* r = &cpu->dreg[3 - (op >> 4)]; // double register
				u16 nn = *r + 1;
				*r = nn;
				u8 f = cpu->f & ~I8085_K;
				if (nn == 0) f |= I8085_K;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// INR B ... INC B
		case 0x04:
		// INR C ... INC C
		case 0x0C:
		// INR D ... INC D
		case 0x14:
		// INR E ... INC E
		case 0x1C:
		// INR H ... INC H
		case 0x24:
		// INR L ... INC L
		case 0x2C:
		// INR A ... INC A
		case 0x3C:
			{
				u8* r = &cpu->reg[7 - (op >> 3)]; // pointer to register
				I8085_INC(*r);
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// DCR B ... DEC B
		case 0x05:
		// DCR C ... DEC C
		case 0x0D:
		// DCR D ... DEC D
		case 0x15:
		// DCR E ... DEC E
		case 0x1D:
		// DCR H ... DEC H
		case 0x25:
		// DCR L ... DEC L
		case 0x2D:
		// DCR A ... DEC A
		case 0x3D:
			{
				u8* r = &cpu->reg[7 - (op >> 3)]; // pointer to register
				I8085_DEC(*r);
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MVI B,d ... LD B,n
		case 0x06:
		// MVI C,n ... LD C,n
		case 0x0E:
		// MVI D,n ... LD D,n
		case 0x16:
		// MVI E,n ... LD E,n
		case 0x1E:
		// MVI H,n ... LD H,n
		case 0x26:
		// MVI L,n ... LD L,n
		case 0x2E:
		// MVI A,n ... LD A,n
		case 0x3E:
			{
				u8* r = &cpu->reg[7 - (op >> 3)]; // pointer to register
				*r = I8085_ProgByte(cpu);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RLC ... RLCA
		case 0x07:
			{
				u8 n = cpu->a;
				u16 n2 = ((u16)n << 1) | (n >> 7);
				cpu->a = (u8)n2;
				u8 f = cpu->f & ~(I8085_C | I8085_V);
				f |= ((n2 ^ (n2 >> 1)) >> 6) & I8085_V;
				f |= n2 & I8085_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// DSUB ... SUB HL,BC
		case 0x08:
			{
				u16 n = cpu->l - cpu->c;
				cpu->l = (u8)n;
				u8 f = (n >> 8) & I8085_C;
				u8 h = cpu->h;
				u8 b = cpu->b;
				n = h - b - f;
				u16 xor = h ^ b ^ n;
				f = (n >> 8) & I8085_C;
				f |= (~xor) & I8085_AC;
				f |= ((xor ^ (xor >> 1)) >> 6) & I8085_V;
				u8 n2 = (u8)n;
				cpu->h = n2;
				f |= I8085_FlagParTab[n2];
				f |= ((f >> 2) ^ (f << 4)) & I8085_K;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// DAD B ... ADD HL,BC
		case 0x09:
		// DAD D ... ADD HL,DE
		case 0x19:
		// DAD H ... ADD HL,HL
		case 0x29:
			{
				u16* r = &cpu->dreg[3 - (op >> 4)]; // double register
				I8085_ADD16(*r);
			}
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// LDAX B ... LD A,(BC)
		case 0x0A:
			cpu->a = cpu->readmem(cpu->bc);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// DCX B ... DEC BC
		case 0x0B:
		// DCX D ... DEC DE
		case 0x1B:
		// DCX H ... DEC HL
		case 0x2B:
			{
				u16* r = &cpu->dreg[3 - (op >> 4)]; // double register
				u16 nn = *r - 1;
				*r = nn;
				u8 f = cpu->f & ~I8085_K;
				if (nn == 0xffff) f |= I8085_K;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// RRC ... RRCA
		case 0x0F:
			{
				u8 n = cpu->a;
				u8 n2 = (n >> 1) | (n << 7);
				cpu->a = n2;
				u8 f = cpu->f & ~(I8085_C | I8085_V);
				f |= n & I8085_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ARHL ... shift right HL
		case 0x10:
			{
				s16 nn = cpu->hl;
				cpu->f = (cpu->f & ~I8085_C) | (nn & I8085_C);
				nn >>= 1;
				cpu->hl = nn;
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// STAX D ... LD (DE),A
		case 0x12:
			cpu->writemem(cpu->de, cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RAL ... RLA
		case 0x17:
			{
				u8 n2 = cpu->a;
				u16 n = ((u16)n2 << 1) | (cpu->f & I8085_C);
				cpu->a = (u8)n;
				u8 f = cpu->f & ~(I8085_C | I8085_V);
				f |= n2 >> 7; // I8085_C;
				f |= ((n ^ (n >> 1)) >> 6) & I8085_V;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// RDEL ... rotate left DE
		case 0x18:
			{
				u16 n2 = cpu->de;
				u32 n = ((u32)n2 << 1) | (cpu->f & I8085_C);
				cpu->de = (u16)n;
				u8 f = cpu->f & ~(I8085_C | I8085_V);
				f |= n2 >> 15; // I8085_C;
				f |= ((n ^ (n >> 1)) >> 14) & I8085_V;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// LDAX D ... LD A,(DE)
		case 0x1A:
			cpu->a = cpu->readmem(cpu->de);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RAR ... RRA
		case 0x1F:
			{
				u8 n2 = cpu->a;
				u8 n = (n2 >> 1) | (cpu->f << 7);
				cpu->a = n;
				u8 f = cpu->f & ~(I8085_C | I8085_V);
				f |= n2 & I8085_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// RIM ... RIM Read Interrupt Mask
		case 0x20:
			{
				u8 n = cpu->intmask & (B0|B1|B2); // get RST mask
				// ie_save is ie state saved during TRAP; it is cleared in first RIM instruction
				if ((cpu->ie | cpu->ie_save) != 0) n |= B3; // global interrupt enable flag
				cpu->ie_save = 0;		// reset saved IE flag
				n |= cpu->intreq & (B4|B5|B6); // pending interrupts
				if (cpu->readsid() != 0) n |= B7; // serial input signal
				cpu->a = n;
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SHLD a ... LD (a),HL
		case 0x22:
			{
				u16 nn = I8085_ProgWord(cpu);
				cpu->writemem(nn, cpu->l);
				nn++;
				cpu->writemem(nn, cpu->h);
			}
			cpu->sync.clock += I8085_CLOCKMUL*16;
			break;

		// DAA
		case 0x27:
			{
				u8 f = cpu->f;
				u8 a = cpu->a;
				u8 a0 = a;
				u8 f2 = 0;

				if (((a & 0x0f) > 9) || ((f & I8085_AC) != 0))
				{
					if ((a & 0x0f) > 9) f2 |= I8085_AC;
					if (a > 0xf9) f |= I8085_C;
					a += 6;
				}

				if ((a > 0x9f) || ((f & I8085_C) != 0))
				{
					a += 0x60;
					f2 |= I8085_C;
				}

				if ((a0 < 0x80) && (a >= 0x80)) f2 |= I8085_V;

				cpu->a = a;
				f2 |= I8085_FlagParTab[a];
				f2 |= ((f2 >> 2) ^ (f2 << 4)) & I8085_K;
				cpu->f = f2;
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// LDHI nn ... add HL and nn into DE
		case 0x28:
			cpu->de = cpu->hl + (u8)I8085_ProgByte(cpu);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// LHLD a ... LD HL,(a)
		case 0x2A:
			{
				u16 nn = I8085_ProgWord(cpu);
				cpu->l = cpu->readmem(nn);
				nn++;
				cpu->h = cpu->readmem(nn);
			}
			cpu->sync.clock += I8085_CLOCKMUL*16;
			break;

		// CMA ... CPL
		case 0x2F:
			cpu->a = ~cpu->a;
			cpu->f |= I8085_V;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SIM ... Set Interrupt Mask
		case 0x30:
			{
				u8 n = cpu->a;

				// set RST mask
				if ((n & B3) != 0)
				{
					cpu->intmask = (cpu->intmask & B7) | (n & (B0|B1|B2)); // set new mask, disable interrupts
					if (cpu->ie != 0) cpu->intmask |= B3 | (((~n) << 4) & (B4|B5|B6)); // add inverted RST flags = enable RST interrupts
				}

				// reset RST7.5 pending flip-flop
				if ((n & B4) != 0) cpu->intreq &= ~I8085_INT_RST75;

				// serial output data
				if ((n & B6) != 0) cpu->writesod(n >> 7);
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// LXI SP,nn ... LD SP,nn
		case 0x31:
			cpu->spl = I8085_ProgByte(cpu);
			cpu->sph = I8085_ProgByte(cpu);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// STA a ... LD (a),A
		case 0x32:
			{
				u16 nn = I8085_ProgWord(cpu);
				cpu->writemem(nn, cpu->a);
			}
			cpu->sync.clock += I8085_CLOCKMUL*13;
			break;

		// INX SP ... INC SP
		case 0x33:
			{
				u16 nn = cpu->sp + 1;
				cpu->sp = nn;
				u8 f = cpu->f & ~I8085_K;
				if (nn == 0) f |= I8085_K;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// INR M ... INC (HL)
		case 0x34:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				I8085_INC(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// DCR M ... DEC (HL)
		case 0x35:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				I8085_DEC(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// MVI M,n ... LD (HL),n
		case 0x36:
			cpu->writemem(cpu->hl, I8085_ProgByte(cpu));
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// STC ... SCF
		case 0x37:
			cpu->f |= I8085_C;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// LDSI ... add SP and nn into DE
		case 0x38:
			cpu->de = cpu->sp + (u8)I8085_ProgByte(cpu);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// DAD SP ... ADD HL,SP
		case 0x39:
			I8085_ADD16(cpu->sp);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// LDA a ... LD A,(a)
		case 0x3A:
			{
				u16 nn = I8085_ProgWord(cpu);
				cpu->a = cpu->readmem(nn);
			}
			cpu->sync.clock += I8085_CLOCKMUL*13;
			break;

		// DCX SP ... DEC SP
		case 0x3B:
			{
				u16 nn = cpu->sp - 1;
				cpu->sp = nn;
				u8 f = cpu->f & ~I8085_K;
				if (nn == 0xffff) f |= I8085_K;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// CMC ... CCF
		case 0x3F:
			cpu->f ^= I8085_C;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV B,B ... LD B,B
		case 0x40:
		// MOV C,C ... LD C,C
		case 0x49:
		// MOV D,D ... LD D,D
		case 0x52:
		// MOV E,E ... LD E,E
		case 0x5B:
		// MOV H,H ... LD H,H
		case 0x64:
		// MOV L,L ... LD L,L
		case 0x6D:
		// MOV A,A ... LD A,A
		case 0x7F:
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV B,B ... LD B,B
		//case 0x40:
		// MOV B,C ... LD B,C
		case 0x41:
		// MOV B,D ... LD B,D
		case 0x42:
		// MOV B,E ... LD B,E
		case 0x43:
		// MOV B,H ... LD B,H
		case 0x44:
		// MOV B,L .. LD B,L
		case 0x45:
		// MOV B,A ... LD B,A
		case 0x47:
		// MOV C,B ... LD C,B
		case 0x48:
		// MOV C,C ... LD C,C
		//case 0x49:
		// MOV C,D ... LD C,D
		case 0x4A:
		// MOV C,E ... LD C,E
		case 0x4B:
		// MOV C,H ... LD C,H
		case 0x4C:
		// MOV C,L ... LD C,L
		case 0x4D:
		// MOV C,A ... LD C,A
		case 0x4F:
		// MOV D,B ... LD D,B
		case 0x50:
		// MOV D,C ... LD D,C
		case 0x51:
		// MOV D,D ... LD D,D
		//case 0x52:
		// MOV D,E .. LD D,E
		case 0x53:
		// MOV D,H ... LD D,H
		case 0x54:
		// MOV D,L ... LD D,L
		case 0x55:
		// MOV D,A ... LD D,A
		case 0x57:
		// MOV E,B ... LD E,B
		case 0x58:
		// MOV E,C ... LD E,C
		case 0x59:
		// MOV E,D ... LD E,D
		case 0x5A:
		// MOV E,E ... LD E,E
		//case 0x5B:
		// MOV E,H ... LD E,H
		case 0x5C:
		// MOV E,L ... LD E,L
		case 0x5D:
		// MOV E,A ... LD E,A
		case 0x5F:
		// MOV H,B ... LD H,B
		case 0x60:
		// MOV H,C ... LD H,C
		case 0x61:
		// MOV H,D ... LD H,D
		case 0x62:
		// MOV H,E ... LD H,E
		case 0x63:
		// MOV H,H ... LD H,H
		//case 0x64:
		// MOV H,L ... LD H,L
		case 0x65:
		// MOV H,A ... LD H,A
		case 0x67:
		// MOV L,B ... LD L,B
		case 0x68:
		// MOV L,C ... LD L,C
		case 0x69:
		// MOV L,D ... LD L,D
		case 0x6A:
		// MOV L,E ... LD L,E
		case 0x6B:
		// MOV L,H ... LD L,H
		case 0x6C:
		// MOV L,L ... LD L,L
		//case 0x6D:
		// MOV L,A ... LD L,A
		case 0x6F:
		// MOV A,B ... LD A,B
		case 0x78:
		// MOV A,C ... LD A,C
		case 0x79:
		// MOV A,D ... LD A,D
		case 0x7A:
		// MOV A,E ... LD A,E
		case 0x7B:
		// MOV A,H ... LD A,H
		case 0x7C:
		// MOV A,L ... LD A,L
		case 0x7D:
		// MOV A,A ... LD A,A
		//case 0x7F:
			{
				op -= 0x40;
				u8* rd = &cpu->reg[7 - (op >> 3)]; // destination register
				u8* rs = &cpu->reg[7 - (op & 7)]; // source register
				*rd = *rs;
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV B,M ... LD B,(HL)
		case 0x46:
		// MOV C,M ... LD C,(HL)
		case 0x4E:
		// MOV D,M ... LD D,(HL)
		case 0x56:
		// MOV E,M ... LD E,(HL)
		case 0x5E:
		// MOV H,M ... LD H,(HL)
		case 0x66:
		// MOV L,M ... LD L,(HL)
		case 0x6E:
		// MOV A,M ... LD A,(HL)
		case 0x7E:
			{
				op -= 0x40;
				u8* rd = &cpu->reg[7 - (op >> 3)]; // destination register
				*rd = cpu->readmem(cpu->hl);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV M,B ... LD (HL),B
		case 0x70:
		// MOV M,C ... LD (HL),C
		case 0x71:
		// MOV M,D ... LD (HL),D
		case 0x72:
		// MOV M,E ... LD (HL),E
		case 0x73:
		// MOV M,H ... LD (HL),H
		case 0x74:
		// MOV M,L ... LD (HL),L
		case 0x75:
		// MOV M,A ... LD (HL),A
		case 0x77:
			{
				u8* rs = &cpu->reg[7 - (op - 0x70)]; // source register
				cpu->writemem(cpu->hl, *rs);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// HLT ... HALT
		case 0x76:
			cpu->pc--;
			cpu->halted = 1;
			cpu->sync.clock += I8085_CLOCKMUL*5;
			break;

		// ADD B ... ADD A,B
		case 0x80:
		// ADD C ... ADD A,C
		case 0x81:
		// ADD D ... ADD A,D
		case 0x82:
		// ADD E ... ADD A,E
		case 0x83:
		// ADD H ... ADD A,H
		case 0x84:
		// ADD L ... ADD A,L
		case 0x85:
		// ADD A ... ADD A,A
		case 0x87:
			{
				u8* rs = &cpu->reg[7 - (op - 0x80)]; // source register
				I8085_ADD(*rs);
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADD M ... ADD A,(HL)
		case 0x86:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8085_ADD(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// ADC B ... ADC A,B
		case 0x88:
		// ADC C ... ADC A,C
		case 0x89:
		// ADC D ... ADC A,D
		case 0x8A:
		// ADC E ... ADC A,E
		case 0x8B:
		// ADC H ... ADC A,H
		case 0x8C:
		// ADC L ... ADC A,L
		case 0x8D:
		// ADC A ... ADC A,A
		case 0x8F:
			{
				u8* rs = &cpu->reg[7 - (op - 0x88)]; // source register
				I8085_ADC(*rs);
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADC M ... ADC A,(HL)
		case 0x8E:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8085_ADC(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// SUB B
		case 0x90:
		// SUB C
		case 0x91:
		// SUB D
		case 0x92:
		// SUB E
		case 0x93:
		// SUB H
		case 0x94:
		// SUB L
		case 0x95:
		// SUB A
		case 0x97:
			{
				u8* rs = &cpu->reg[7 - (op - 0x90)]; // source register
				I8085_SUB(*rs);
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SUB M ... SUB (HL)
		case 0x96:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8085_SUB(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// SBB B ... SBC A,B
		case 0x98:
		// SBB C ... SBC A,C
		case 0x99:
		// SBB D ... SBC A,D
		case 0x9A:
		// SBB E ... SBC A,E
		case 0x9B:
		// SBB H ... SBC A,H
		case 0x9C:
		// SBB L ... SBC A,L
		case 0x9D:
		// SBB A ... SBC A,A
		case 0x9F:
			{
				u8* rs = &cpu->reg[7 - (op - 0x98)]; // source register
				I8085_SBC(*rs);
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SBB M ... SBC A,(HL)
		case 0x9E:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8085_SBC(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// ANA B ... AND B
		case 0xA0:
		// ANA C ... AND C
		case 0xA1:
		// ANA D ... AND D
		case 0xA2:
		// ANA E ... AND E
		case 0xA3:
		// ANA H ... AND H
		case 0xA4:
		// ANA L ... AND L
		case 0xA5:
		// ANA A ... AND A
		case 0xA7:
			{
				u8* rs = &cpu->reg[7 - (op - 0xa0)]; // source register
				I8085_AND(*rs);
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ANA M ... AND (HL)
		case 0xA6:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8085_AND(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// XRA B ... XOR B
		case 0xA8:
		// XRA C ... XOR C
		case 0xA9:
		// XRA D ... XOR D
		case 0xAA:
		// XRA E ... XOR E
		case 0xAB:
		// XRA H ... XOR H
		case 0xAC:
		// XRA L ... XOR L
		case 0xAD:
		// XRA A ... XOR A
		case 0xAF:
			{
				u8* rs = &cpu->reg[7 - (op - 0xa8)]; // source register
				I8085_XOR(*rs);
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// XRA M ... XOR (HL)
		case 0xAE:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8085_XOR(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// ORA B ... OR B
		case 0xB0:
		// ORA C ... OR C
		case 0xB1:
		// ORA D ... OR D
		case 0xB2:
		// ORA E ... OR E
		case 0xB3:
		// ORA H ... OR H
		case 0xB4:
		// ORA L ... OR L
		case 0xB5:
		// ORA A ... OR A
		case 0xB7:
			{
				u8* rs = &cpu->reg[7 - (op - 0xb0)]; // source register
				I8085_OR(*rs);
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ORA M ... OR (HL)
		case 0xB6:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8085_OR(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// CMP B ... CP B
		case 0xB8:
		// CMP C ... CP C
		case 0xB9:
		// CMP D ... CP D
		case 0xBA:
		// CMP E ... CP E
		case 0xBB:
		// CMP H ... CP H
		case 0xBC:
		// CMP L ... CP L
		case 0xBD:
		// CMP A ... CP A
		case 0xBF:
			{
				u8* rs = &cpu->reg[7 - (op - 0xb8)]; // source register
				I8085_CP(*rs);
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// CMP M ... CP (HL)
		case 0xBE:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8085_CP(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RNZ ... RET NZ
		case 0xC0:
		// RZ ... RET Z
		case 0xC8:
		// RNC ... RET NC
		case 0xD0:
		// RC ... RET C
		case 0xD8:
		// RPO ... RET PO
		case 0xE0:
		// RPE ... RET PE
		case 0xE8:
		// RP ... RET P
		case 0xF0:
		// RM ... RET M
		case 0xF8:
			{
				// flag bit
				int fb = cpu->cond[(op - 0xc0) >> 4];

				// invert flags
				u8 f = cpu->f;
				if ((op & B3) != 0) f = ~f;

				// check condition
				if ((f & fb) == 0)
				{
					I8085_RET();
					cpu->sync.clock += I8085_CLOCKMUL*(12 - 6);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// POP B ... POP BC
		case 0xC1:
		// POP D ... POP DE
		case 0xD1:
		// POP H ... POP HL
		case 0xE1:
			{
				u8* r = &cpu->reg[6 - ((op - 0xc1) >> 3)]; // register low
				I8085_POP(r[0], r[1]);
			}
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// JNZ a ... JP NZ,a
		case 0xC2:
		// JZ a ... JP Z,a
		case 0xCA:
		// JNC a ... JP NC,a
		case 0xD2:
		// JC a ... JP C,a
		case 0xDA:
		// JPO a ... JP PO,a
		case 0xE2:
		// JPE a ... JP PE,a
		case 0xEA:
		// JP a ... JP P,a
		case 0xF2:
		// JM a ... JP M,a
		case 0xFA:
			{
				// read destination address -> nn
				u16 nn = I8085_ProgWord(cpu);

				// flag bit
				int fb = cpu->cond[(op - 0xc2) >> 4];

				// invert flags
				u8 f = cpu->f;
				if ((op & B3) != 0) f = ~f;

				// check condition
				if ((f & fb) == 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += I8085_CLOCKMUL*(10 - 7);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// JMP a ... JP a
		case 0xC3:
			cpu->pc = I8085_ProgWord(cpu);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// CNZ a ... CALL NZ,a
		case 0xC4:
		// CZ a ... CALL Z,a
		case 0xCC:
		// CNC a ... CALL NC,a
		case 0xD4:
		// CC a ... CALL C,a
		case 0xDC:
		// CPO a ... CALL PO,a
		case 0xE4:
		// CPE a ... CALL PE,a
		case 0xEC:
		// CP a ... CALL P,a
		case 0xF4:
		// CM a ... CALL M,a
		case 0xFC:
			{
				// read destination address -> nn
				u16 nn = I8085_ProgWord(cpu);

				// flag bit
				int fb = cpu->cond[(op - 0xc4) >> 4];

				// invert flags
				u8 f = cpu->f;
				if ((op & B3) != 0) f = ~f;

				// check condition
				if ((f & fb) == 0)
				{
					I8085_CALL(nn);
					cpu->sync.clock += I8085_CLOCKMUL*(18 - 9);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*9;
			break;

		// PUSH B ... PUSH BC
		case 0xC5:
		// PUSH D ... PUSH DE
		case 0xD5:
		// PUSH H ... PUSH HL
		case 0xE5:
			{
				u8* r = &cpu->reg[6 - ((op - 0xc5) >> 3)]; // register low
				I8085_PUSH(r[0], r[1]);
			}
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// ADI d ... ADD A,d
		case 0xC6:
			{
				u8 n = I8085_ProgByte(cpu);
				I8085_ADD(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RST 0 ... RST 00h
		case 0xC7:
		// RST 1 ... RST 08h
		case 0xCF:
		// RST 2 ... RST 10h
		case 0xD7:
		// RST 3 ... RST 18h
		case 0xDF:
		// RST 4 ... RST 20h
		case 0xE7:
		// RST 5 ... RST 28h
		case 0xEF:
		// RST 6 ... RST 30h
		case 0xF7:
		// RST 7 ... RST 38h
		case 0xFF:
			I8085_CALL(op & 0x38);
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// RET
		case 0xC9:
			I8085_RET();
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// RSTV ... ReSTart on Overflow to 0040h
		case 0xCB:
			if ((cpu->f & I8085_V) != 0)
			{
				I8085_CALL(0x40);
				cpu->sync.clock += I8085_CLOCKMUL*(12 - 6);
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// CALL a
		case 0xCD:
			{
				u16 nn = I8085_ProgWord(cpu);
				I8085_CALL(nn);
			}
			cpu->sync.clock += I8085_CLOCKMUL*18;
			break;

		// ACI d ... ADC A,d
		case 0xCE:
			{
				u8 n = I8085_ProgByte(cpu);
				I8085_ADC(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// OUT d ... OUT (d),A
		case 0xD3:
			cpu->writeport(I8085_ProgByte(cpu), cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// SUI d ... SUB d
		case 0xD6:
			{
				u8 n = I8085_ProgByte(cpu);
				I8085_SUB(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// SHLX ... Load [DE] from HL
		case 0xD9:
			cpu->writemem(cpu->de, cpu->l);
			cpu->writemem(cpu->de+1, cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// IN d ... IN A,(d)
		case 0xDB:
			cpu->a = cpu->readport(I8085_ProgByte(cpu));
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// JNK a (JNX5 a) ... Jump on 'Not X5'
		case 0xDD:
			{
				u16 nn = I8085_ProgWord(cpu);
				if ((cpu->f & I8085_K) == 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += I8085_CLOCKMUL*(10 - 7);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*7; // 7 or 10
			break;

		// SBI d ... SBC A,d
		case 0xDE:
			{
				u8 n = I8085_ProgByte(cpu);
				I8085_SBC(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// XTHL ... EX (SP),HL
		case 0xE3:
			{
				u16 sp = cpu->sp;
				u8 n = cpu->readmem(sp);
				u8 n2 = cpu->readmem(sp+1);
				cpu->writemem(sp, cpu->l);
				cpu->writemem(sp+1, cpu->h);
				cpu->l = n;
				cpu->h = n2;
			}
			cpu->sync.clock += I8085_CLOCKMUL*16;
			break;

		// ANI d ... AND d
		case 0xE6:
			{
				u8 n = I8085_ProgByte(cpu);
				I8085_AND(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// PCHL ... JP (HL)
		case 0xE9:
			cpu->pc = cpu->hl;
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// XCHG ... EX DE.HL
		case 0xEB:
			{
				u16 nn = cpu->hl;
				cpu->hl = cpu->de;
				cpu->de = nn;
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// LHLX ... Load Hl from [DE]
		case 0xED:
			cpu->l = cpu->readmem(cpu->de);
			cpu->h = cpu->readmem(cpu->de+1);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// XRI d ... XOR d
		case 0xEE:
			{
				u8 n = I8085_ProgByte(cpu);
				I8085_XOR(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// POP PSW ... POP AF
		case 0xF1:
			I8085_POP(cpu->f, cpu->a);
			cpu->f = (cpu->f & I8085_FLAGALL) | I8085_FLAGDEF;
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// DI
		case 0xF3:
			cpu->ie = 0;
			cpu->intmask &= B0|B1|B2|B7; // disable interrupts, leave only RST mask and TRAP enable
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// PUSH PSW ... PUSH AF
		case 0xF5:
			cpu->f = (cpu->f & I8085_FLAGALL) | I8085_FLAGDEF;
			I8085_PUSH(cpu->f, cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// ORI d ... OR d
		case 0xF6:
			{
				u8 n = I8085_ProgByte(cpu);
				I8085_OR(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// SPHL ... LD SP,HL
		case 0xF9:
			cpu->sp = cpu->hl;
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// EI
		case 0xFB:
			{
				cpu->ie = 1; // enable interrupt
				cpu->tid = 1; // temporary disable interrupt (for next 1 instruction)
				u8 n = cpu->intmask & (B0|B1|B2|B7); // get RST mask and TRAP enable mask
				n |= ((~n) << 4) & (B4|B5|B6); // add inverted RST flags = enable RST interrupts
				n |= B3; // enable INTR interrupt
				cpu->intmask = n;
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// JK a ... Jump on 'X5'
		case 0xFD:
			{
				u16 nn = I8085_ProgWord(cpu);
				if ((cpu->f & I8085_K) != 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += I8085_CLOCKMUL*(10 - 7);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*7; // 7 or 10
			break;

		// CPI d ... CP d
		case 0xFE:
			{
				u8 n = I8085_ProgByte(cpu);
				I8085_CP(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;
		}

		// time synchronization
		EmuSync(&cpu->sync);
	}
}

// execute I8085 emulator on CPU 1
static void I8085_ExecCore1()
{
	// start time synchronization
	I8085_SyncStart((sI8085*)I8085_Cpu);

	// execute
	I8085_Exec((sI8085*)I8085_Cpu);

	// clear descriptor
	I8085_Cpu = NULL;
	dsb();
}

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8085_CLOCKMUL is recommended to maintain.
u32 I8085_Start(sI8085* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I8085_Cpu != NULL) I8085_Stop(pwm);

	// initialize time synchronization
	freq = I8085_SyncInit(cpu, pwm, freq);

	// reset processor
	I8085_Reset(cpu);

	// current CPU descriptor
	I8085_Cpu = cpu;
	dsb();

	// execute program
#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	if (DispHstxRunning)
		DispHstxCore1Exec(I8085_ExecCore1);
	else
		Core1Exec(I8085_ExecCore1);
#elif USE_DISPHSTX			// 1=use HSTX Display driver
	DispHstxCore1Exec(I8085_ExecCore1);
#elif USE_MINIVGA			// use mini-VGA display
	VgaCore1Exec(I8085_ExecCore1);
#else
	Core1Exec(I8085_ExecCore1);
#endif

	return freq;
}

// continue emulation without reset processor
u32 I8085_Cont(sI8085* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I8085_Cpu != NULL) I8085_Stop(pwm);

	// initialize time synchronization
	freq = I8085_SyncInit(cpu, pwm, freq);

	// current CPU descriptor
	I8085_Cpu = cpu;
	dsb();

	// execute program
#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	if (DispHstxRunning)
		DispHstxCore1Exec(I8085_ExecCore1);
	else
		Core1Exec(I8085_ExecCore1);
#elif USE_DISPHSTX			// 1=use HSTX Display driver
	DispHstxCore1Exec(I8085_ExecCore1);
#elif USE_MINIVGA			// use mini-VGA display
	VgaCore1Exec(I8085_ExecCore1);
#else
	Core1Exec(I8085_ExecCore1);
#endif

	return freq;
}

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I8085_Stop(int pwm)
{
	// get CPU descriptor
	sI8085* cpu = (sI8085*)I8085_Cpu;

	// check if emulation is running
	if (cpu == NULL) return;

	// request to stop emulation
	cpu->stop = 1;

	// wait to complete
	while (I8085_Cpu != NULL) { dsb(); }

	// terminate time synchronization (stop PWM counter)
	I8085_SyncTerm(pwm);

#if !USE_MINIVGA && !USE_DISPHSTX	// use mini-VGA display

#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	if (!DispHstxRunning)
#endif

	// core 1 reset
	Core1Reset();
#endif
}

#endif // USE_EMU_I8085
