
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
	for (i = 0; i < 256; i++)
	{
		// initialize parity table
		par = 1;
		j = i;
		while (j != 0)
		{
			par ^= (j & 1);
			j >>= 1;
		}

		// parity flags
		I8085_FlagParTab[i] = (par << I8085_P_BIT) | (i & I8085_S) | I8085_FLAGDEF;
	}

	// add zero flag
	I8085_FlagParTab[0] |= I8085_Z;
}

// reset processor
void I8085_Reset(sI8085* cpu)
{
	cpu->pc = 0;		// program counter
	cpu->sp = 0xffff;	// stack pointer

	cpu->a = 0;
	cpu->f = I8085_FLAGDEF;
	cpu->bc = 0;

	cpu->dehl = 0;

	cpu->intmask = 0;	// interrupt mask
	cpu->ie = 0;		// interrupt enable
	cpu->tid = 0;		// temporary interrupt disable
	cpu->halted = 0;	// halted

	cpu->intreq = 0;	// interrupt request
	cpu->intins = 0x0D;	// interrupt instruction RST 0x08
	cpu->stop = 0;		// stop request
}

// execute program (start or continue, until "stop" request)
//  C code size in RAM: 8708 bytes (optimization -Os)
void NOFLASH(I8085_Exec)(sI8085* cpu)
{
	u8 op;

	// clear stop flag
	cpu->stop = 0;

	// program loop
	while (cpu->stop == 0)
	{
/*
Special RST signals:
	TRAP ... restart address 24h
	RST5.5 ... restart address 2Ch
	RST6.5 ... restart address 34h
	RST7.5 ... restart address 3Ch
*/

		// interrupt
		if ((cpu->intreq != 0) && (cpu->ie != 0) && (cpu->tid == 0))
		{
			// clear interrupt request
			cpu->intreq = 0;

			// continue after HALT
			if (cpu->halted != 0)
			{
				cpu->pc++;
				cpu->halted = 0;
			}

			// process interrupt instruction
			op = cpu->intins;
		}
		else
		{
			// get next instruction
			op = PROGBYTE();
		}

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
			cpu->c = PROGBYTE();
			cpu->b = PROGBYTE();
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// STAX B ... LD (BC),A
		case 0x02:
			cpu->writemem(cpu->bc, cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// INX B ... INC BC
		case 0x03:
			{
				u16 nn = cpu->bc + 1;
				cpu->bc = nn;
				u8 f = cpu->f & ~I8085_K;
				if (nn == 0) f |= I8085_K;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// INR B ... INC B
		case 0x04:
			I8085_INC(cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// DCR B ... DEC B
		case 0x05:
			I8085_DEC(cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MVI B,d ... LD B,n
		case 0x06:
			cpu->b = PROGBYTE();
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RLC ... RLCA
		case 0x07:
			{
				u8 n = cpu->a;
				u16 n2 = ((u16)n << 1) | (n >> 7);
				cpu->a = (u8)n2;
				u8 f = cpu->f & ~I8085_C;
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
			I8085_ADD16(bc);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// LDAX B ... LD A,(BC)
		case 0x0A:
			cpu->a = cpu->readmem(cpu->bc);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// DCX B ... DEC BC
		case 0x0B:
			{
				u16 nn = cpu->bc - 1;
				cpu->bc = nn;
				u8 f = cpu->f & ~I8085_K;
				if (nn == 0xffff) f |= I8085_K;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// INR C ... INC C
		case 0x0C:
			I8085_INC(cpu->c);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// DCR C ... DEC C
		case 0x0D:
			I8085_DEC(cpu->c);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MVI C,n ... LD C,n
		case 0x0E:
			cpu->c = PROGBYTE();
			cpu->sync.clock += I8085_CLOCKMUL*7;
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

		// ARHL ... (shift right HL)
		case 0x10:
			{
				s16 nn = cpu->hl;
				cpu->f = (cpu->f & ~I8085_C) | (nn & I8085_C);
				nn >>= 1;
				cpu->hl = nn;
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

!!!!!!!!!!!!!!!!!

		// LXI D,nn ... LD DE,nn
		case 0x11:
			cpu->e = PROGBYTE();
			cpu->d = PROGBYTE();
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// STAX D ... LD (DE),A
		case 0x12:
			cpu->writemem(cpu->de, cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// INX D ... INC DE
		case 0x13:
			{
				u16 nn = cpu->de + 1;
				cpu->de = nn;
				u8 f = cpu->f & ~I8085_K;
				if (nn == 0) f |= I8085_K;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// INR D ... INC D
		case 0x14:
			I8085_INC(cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// DCR D ... DEC D
		case 0x15:
			I8085_DEC(cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MVI D,n ... LD D,n
		case 0x16:
			cpu->d = PROGBYTE();
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RAL ... RLA
		case 0x17:
			{
				u8 n2 = cpu->a;
				u16 n = ((u16)n2 << 1) | (cpu->f & I8085_C);
				cpu->a = (u8)n;
				u8 f = cpu->f & ~I8085_C;
				f |= n2 >> 7; // I8085_C;
				f |= ((n ^ (n >> 1)) >> 6) & I8085_V;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// RDEL ... (rotate right DE)
		case 0x18:
// !!!!!!!!!!!!!!!
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// DAD D ... ADD HL,DE
		case 0x19:
			I8085_ADD16(de);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// LDAX D ... LD A,(DE)
		case 0x1A:
			cpu->a = cpu->readmem(cpu->de);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// DCX D ... DEC DE
		case 0x1B:
			{
				u16 nn = cpu->de - 1;
				cpu->de = nn;
				u8 f = cpu->f & ~I8085_K;
				if (nn == 0xffff) f |= I8085_K;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// INR E ... INC E
		case 0x1C:
			I8085_INC(cpu->e);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// DCR E ... DEC E
		case 0x1D:
			I8085_DEC(cpu->e);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MVI E,n ... LD E,n
		case 0x1E:
			cpu->e = PROGBYTE();
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
				u8 n = cpu->intmask & 0x7f;
				if (cpu->readsid() != 0) n |= B7;
				cpu->a = n;
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// LXI H,nn ... LD HL,nn
		case 0x21:
			cpu->l = PROGBYTE();
			cpu->h = PROGBYTE();
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// SHLD a ... LD (a),HL
		case 0x22:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				cpu->writemem(nn, cpu->l);
				nn++;
				cpu->writemem(nn, cpu->h);
			}
			cpu->sync.clock += I8085_CLOCKMUL*16;
			break;

		// INX H ... INC HL
		case 0x23:
			{
				u16 nn = cpu->hl + 1;
				cpu->hl = nn;
				u8 f = cpu->f & ~I8085_K;
				if (nn == 0) f |= I8085_K;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// INR H ... INC H
		case 0x24:
			I8085_INC(cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// DCR H ... DEC H
		case 0x25:
			I8085_DEC(cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MVI H,n ... LD H,n
		case 0x26:
			cpu->h = PROGBYTE();
			cpu->sync.clock += I8085_CLOCKMUL*7;
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

		// LDHI ... (add HL and Immidiate nnnn into DE)
		case 0x28:
// !!!!!!!!!!
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// DAD H ... ADD HL,HL
		case 0x29:
			I8085_ADD16(hl);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// LHLD a ... LD HL,(a)
		case 0x2A:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				cpu->l = cpu->readmem(nn);
				nn++;
				cpu->h = cpu->readmem(nn);
			}
			cpu->sync.clock += I8085_CLOCKMUL*16;
			break;

		// DCX H ... DEC HL
		case 0x2B:
			{
				u16 nn = cpu->hl - 1;
				cpu->hl = nn;
				u8 f = cpu->f & ~I8085_K;
				if (nn == 0xffff) f |= I8085_K;
				cpu->f = f;
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// INR L ... INC L
		case 0x2C:
			I8085_INC(cpu->l);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// DCR L ... DEC L
		case 0x2D:
			I8085_DEC(cpu->l);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MVI L,n ... LD L,n
		case 0x2E:
			cpu->l = PROGBYTE();
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// CMA ... CPL
		case 0x2F:
			cpu->a = ~cpu->a;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SIM ... Set Interrupt Mask
		case 0x30:
			{
				u8 n = cpu->a;

				// set RST mask
				if ((n & B3) != 0) cpu->intmask = (cpu->intmask & ~(B0|B1|B2)) | (n & (B0|B1|B2));

				// reset RST7.5 pending flip-flop
				if ((n & B4) != 0) cpi->intmask &= ~B6;

				// serial output data
				if ((n & B6) != 0) cpu->writesod(n >> 7);
			}
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// LXI SP,nn ... LD SP,nn
		case 0x31:
			cpu->spl = PROGBYTE();
			cpu->sph = PROGBYTE();
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// STA a ... LD (a),A
		case 0x32:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
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
			cpu->writemem(cpu->hl, PROGBYTE());
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// STC ... SCF
		case 0x37:
			cpu->f |= I8085_C;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// LDSI ... (Add SP and Immidiate nnnn into DE)
		case 0x38:
// !!!!!!!!
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// DAD SP ... ADD HL,SP
		case 0x39:
			I8085_ADD16(sp);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// LDA a ... LD A,(a)
		case 0x3A:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
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

		// INR A ... INC A
		case 0x3C:
			I8085_INC(cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// DCR A ... DEC A
		case 0x3D:
			I8085_DEC(cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MVI A,n ... LD A,n
		case 0x3E:
			cpu->a = PROGBYTE();
			cpu->sync.clock += I8085_CLOCKMUL*7;
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

		// MOV B,C ... LD B,C
		case 0x41:
			cpu->b = cpu->c;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV B,D ... LD B,D
		case 0x42:
			cpu->b = cpu->d;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV B,E ... LD B,E
		case 0x43:
			cpu->b = cpu->e;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV B,H ... LD B,H
		case 0x44:
			cpu->b = cpu->h;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV B,L .. LD B,L
		case 0x45:
			cpu->b = cpu->l;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV B,M ... LD B,(HL)
		case 0x46:
			cpu->b = cpu->readmem(cpu->hl);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV B,A ... LD B,A
		case 0x47:
			cpu->b = cpu->a;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV C,B ... LD C,B
		case 0x48:
			cpu->c = cpu->b;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV C,C ... LD C,C
		//case 0x49:

		// MOV C,D ... LD C,D
		case 0x4A:
			cpu->c = cpu->d;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV C,E ... LD C,E
		case 0x4B:
			cpu->c = cpu->e;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV C,H ... LD C,H
		case 0x4C:
			cpu->c = cpu->h;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV C,L ... LD C,L
		case 0x4D:
			cpu->c = cpu->l;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV C,M ... LD C,(HL)
		case 0x4E:
			cpu->c = cpu->readmem(cpu->hl);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV C,A ... LD C,A
		case 0x4F:
			cpu->c = cpu->a;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV D,B ... LD D,B
		case 0x50:
			cpu->c = cpu->b;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV D,C ... LD D,C
		case 0x51:
			cpu->d = cpu->c;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV D,D ... LD D,D
		//case 0x52:

		// MOV D,E .. LD D,E
		case 0x53:
			cpu->d = cpu->e;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV D,H ... LD D,H
		case 0x54:
			cpu->d = cpu->h;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV D,L ... LD D,L
		case 0x55:
			cpu->d = cpu->l;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV D,M ... LD D,(HL)
		case 0x56:
			cpu->d = cpu->readmem(cpu->hl);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV D,A ... LD D,A
		case 0x57:
			cpu->d = cpu->a;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV E,B ... LD E,B
		case 0x58:
			cpu->e = cpu->b;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV E,C ... LD E,C
		case 0x59:
			cpu->e = cpu->c;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV E,D ... LD E,D
		case 0x5A:
			cpu->e = cpu->d;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV E,E ... LD E,E
		//case 0x5B:

		// MOV E,H ... LD E,H
		case 0x5C:
			cpu->e = cpu->h;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV E,L ... LD E,L
		case 0x5D:
			cpu->e = cpu->l;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV E,M ... LD E,(HL)
		case 0x5E:
			cpu->e = cpu->readmem(cpu->hl);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV E,A ... LD E,A
		case 0x5F:
			cpu->e = cpu->a;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV H,B ... LD H,B
		case 0x60:
			cpu->h = cpu->b;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV H,C ... LD H,C
		case 0x61:
			cpu->h = cpu->c;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV H,D ... LD H,D
		case 0x62:
			cpu->h = cpu->d;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV H,E ... LD H,E
		case 0x63:
			cpu->h = cpu->e;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV H,H ... LD H,H
		//case 0x64:

		// MOV H,L ... LD H,L
		case 0x65:
			cpu->h = cpu->l;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV H,M ... LD H,(HL)
		case 0x66:
			cpu->h = cpu->readmem(cpu->hl);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV H,A ... LD H,A
		case 0x67:
			cpu->h = cpu->a;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV L,B ... LD L,B
		case 0x68:
			cpu->l = cpu->b;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV L,C ... LD L,C
		case 0x69:
			cpu->l = cpu->c;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV L,D ... LD L,D
		case 0x6A:
			cpu->l = cpu->d;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV L,E ... LD L,E
		case 0x6B:
			cpu->l = cpu->e;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV L,H ... LD L,H
		case 0x6C:
			cpu->l = cpu->h;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV L,L ... LD L,L
		//case 0x6D:

		// MOV L,M ... LD L,(HL)
		case 0x6E:
			cpu->l = cpu->readmem(cpu->hl);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV L,A ... LD L,A
		case 0x6F:
			cpu->l = cpu->a;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV M,B ... LD (HL),B
		case 0x70:
			cpu->writemem(cpu->hl, cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV M,C ... LD (HL),C
		case 0x71:
			cpu->writemem(cpu->hl, cpu->c);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV M,D ... LD (HL),D
		case 0x72:
			cpu->writemem(cpu->hl, cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV M,E ... LD (HL),E
		case 0x73:
			cpu->writemem(cpu->hl, cpu->e);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV M,H ... LD (HL),H
		case 0x74:
			cpu->writemem(cpu->hl, cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV M,L ... LD (HL),L
		case 0x75:
			cpu->writemem(cpu->hl, cpu->l);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// HLT ... HALT
		case 0x76:
			cpu->pc--;
			cpu->halted = 1;
			cpu->sync.clock += I8085_CLOCKMUL*5;
			break;

		// MOV M,A ... LD (HL),A
		case 0x77:
			cpu->writemem(cpu->hl, cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV A,B ... LD A,B
		case 0x78:
			cpu->a = cpu->b;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV A,C ... LD A,C
		case 0x79:
			cpu->a = cpu->c;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV A,D ... LD A,D
		case 0x7A:
			cpu->a = cpu->d;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV A,E ... LD A,E
		case 0x7B:
			cpu->a = cpu->e;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV A,H ... LD A,H
		case 0x7C:
			cpu->a = cpu->h;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV A,L ... LD A,L
		case 0x7D:
			cpu->a = cpu->l;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// MOV A,M ... LD A,(HL)
		case 0x7E:
			cpu->a = cpu->readmem(cpu->hl);
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// MOV A,A ... LD A,A
		//case 0x7F:

		// ADD B ... ADD A,B
		case 0x80:
			I8085_ADD(cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADD C ... ADD A,C
		case 0x81:
			I8085_ADD(cpu->c);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADD D ... ADD A,D
		case 0x82:
			I8085_ADD(cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADD E ... ADD A,E
		case 0x83:
			I8085_ADD(cpu->e);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADD H ... ADD A,H
		case 0x84:
			I8085_ADD(cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADD L ... ADD A,L
		case 0x85:
			I8085_ADD(cpu->l);
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

		// ADD A ... ADD A,A
		case 0x87:
			I8085_ADD(cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADC B ... ADC A,B
		case 0x88:
			I8085_ADC(cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADC C ... ADC A,C
		case 0x89:
			I8085_ADC(cpu->c);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADC D ... ADC A,D
		case 0x8A:
			I8085_ADC(cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADC E ... ADC A,E
		case 0x8B:
			I8085_ADC(cpu->e);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADC H ... ADC A,H
		case 0x8C:
			I8085_ADC(cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ADC L ... ADC A,L
		case 0x8D:
			I8085_ADC(cpu->l);
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

		// ADC A ... ADC A,A
		case 0x8F:
			I8085_ADC(cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SUB B
		case 0x90:
			I8085_SUB(cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SUB C
		case 0x91:
			I8085_SUB(cpu->c);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SUB D
		case 0x92:
			I8085_SUB(cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SUB E
		case 0x93:
			I8085_SUB(cpu->e);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SUB H
		case 0x94:
			I8085_SUB(cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SUB L
		case 0x95:
			I8085_SUB(cpu->l);
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

		// SUB A
		case 0x97:
			I8085_SUB(cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SBB B ... SBC A,B
		case 0x98:
			I8085_SBC(cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SBB C ... SBC A,C
		case 0x99:
			I8085_SBC(cpu->c);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SBB D ... SBC A,D
		case 0x9A:
			I8085_SBC(cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SBB E ... SBC A,E
		case 0x9B:
			I8085_SBC(cpu->e);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SBB H ... SBC A,H
		case 0x9C:
			I8085_SBC(cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// SBB L ... SBC A,L
		case 0x9D:
			I8085_SBC(cpu->l);
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

		// SBB A ... SBC A,A
		case 0x9F:
			I8085_SBC(cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ANA B ... AND B
		case 0xA0:
			I8085_AND(cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ANA C ... AND C
		case 0xA1:
			I8085_AND(cpu->c);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ANA D ... AND D
		case 0xA2:
			I8085_AND(cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ANA E ... AND E
		case 0xA3:
			I8085_AND(cpu->e);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ANA H ... AND H
		case 0xA4:
			I8085_AND(cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ANA L ... AND L
		case 0xA5:
			I8085_AND(cpu->l);
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

		// ANA A ... AND A
		case 0xA7:
			I8085_AND(cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// XRA B ... XOR B
		case 0xA8:
			I8085_XOR(cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// XRA C ... XOR C
		case 0xA9:
			I8085_XOR(cpu->c);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// XRA D ... XOR D
		case 0xAA:
			I8085_XOR(cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// XRA E ... XOR E
		case 0xAB:
			I8085_XOR(cpu->e);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// XRA H ... XOR H
		case 0xAC:
			I8085_XOR(cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// XRA L ... XOR L
		case 0xAD:
			I8085_XOR(cpu->l);
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

		// XRA A ... XOR A
		case 0xAF:
			I8085_XOR(cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ORA B ... OR B
		case 0xB0:
			I8085_OR(cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ORA C ... OR C
		case 0xB1:
			I8085_OR(cpu->c);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ORA D ... OR D
		case 0xB2:
			I8085_OR(cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ORA E ... OR E
		case 0xB3:
			I8085_OR(cpu->e);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ORA H ... OR H
		case 0xB4:
			I8085_OR(cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// ORA L ... OR L
		case 0xB5:
			I8085_OR(cpu->l);
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

		// ORA A ... OR A
		case 0xB7:
			I8085_OR(cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// CMP B ... CP B
		case 0xB8:
			I8085_CP(cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// CMP C ... CP C
		case 0xB9:
			I8085_CP(cpu->c);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// CMP D ... CP D
		case 0xBA:
			I8085_CP(cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// CMP E ... CP E
		case 0xBB:
			I8085_CP(cpu->e);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// CMP H ... CP H
		case 0xBC:
			I8085_CP(cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// CMP L ... CP L
		case 0xBD:
			I8085_CP(cpu->l);
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

		// CMP A ... CP A
		case 0xBF:
			I8085_CP(cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// RNZ ... RET NZ
		case 0xC0:
			if ((cpu->f & I8085_Z) == 0)
			{
				I8085_RET();
				cpu->sync.clock += I8085_CLOCKMUL*(12 - 6);
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// POP B ... POP BC
		case 0xC1:
			I8085_POP(cpu->c, cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// JNZ a ... JP NZ,a
		case 0xC2:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_Z) == 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += I8085_CLOCKMUL*(10 - 7);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// JMP a ... JP a
		case 0xC3:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				cpu->pc = nn;
			}
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// CNZ a ... CALL NZ,a
		case 0xC4:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_Z) == 0)
				{
					I8085_CALL(nn);
					cpu->sync.clock += I8085_CLOCKMUL*(18 - 9);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*9;
			break;

		// PUSH B ... PUSH BC
		case 0xC5:
			I8085_PUSH(cpu->c, cpu->b);
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// ADI d ... ADD A,d
		case 0xC6:
			{
				u8 n = PROGBYTE();
				I8085_ADD(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RST 0 ... RST 00h
		case 0xC7:
			I8085_CALL(0x00);
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// RZ ... RET Z
		case 0xC8:
			if ((cpu->f & I8085_Z) != 0)
			{
				I8085_RET();
				cpu->sync.clock += I8085_CLOCKMUL*(12 - 6);
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// RET
		case 0xC9:
			I8085_RET();
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// JZ a ... JP Z,a
		case 0xCA:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_Z) != 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += I8085_CLOCKMUL*(10 - 7);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RSTV ... ( ReSTart on Overflow to 0040h)
		case 0xCB:
// !!!!!!!!!!!!
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// CZ a ... CALL Z,a
		case 0xCC:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_Z) != 0)
				{
					I8085_CALL(nn);
					cpu->sync.clock += I8085_CLOCKMUL*(18 - 9);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*9;
			break;

		// CALL a
		case 0xCD:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				I8085_CALL(nn);
			}
			cpu->sync.clock += I8085_CLOCKMUL*18;
			break;

		// ACI d ... ADC A,d
		case 0xCE:
			{
				u8 n = PROGBYTE();
				I8085_ADC(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RST 1 ... RST 08h
		case 0xCF:
			I8085_CALL(0x08);
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// RNC ... RET NC
		case 0xD0:
			if ((cpu->f & I8085_C) == 0)
			{
				I8085_RET();
				cpu->sync.clock += I8085_CLOCKMUL*(12 - 6);
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// POP D ... POP DE
		case 0xD1:
			I8085_POP(cpu->e, cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// JNC a ... JP NC,a
		case 0xD2:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_C) == 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += I8085_CLOCKMUL*(10 - 7);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// OUT d ... OUT (d),A
		case 0xD3:
			cpu->writeio(PROGBYTE(), cpu->a);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// CNC a ... CALL NC,a
		case 0xD4:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_C) == 0)
				{
					I8085_CALL(nn);
					cpu->sync.clock += I8085_CLOCKMUL*(18 - 9);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*9;
			break;

		// PUSH D ... PUSH DE
		case 0xD5:
			I8085_PUSH(cpu->e, cpu->d);
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// SUI d ... SUB d
		case 0xD6:
			{
				u8 n = PROGBYTE();
				I8085_SUB(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RST 2 ... RST 10h
		case 0xD7:
			I8085_CALL(0x10);
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// RC ... RET C
		case 0xD8:
			if ((cpu->f & I8085_C) != 0)
			{
				I8085_RET();
				cpu->sync.clock += I8085_CLOCKMUL*(12 - 6);
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// SHLX ... (Load [DE] from HL)
		case 0xD9:
// !!!!!!!!!
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// JC a ... JP C,a
		case 0xDA:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_C) != 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += I8085_CLOCKMUL*(10 - 7);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// IN d ... IN A,(d)
		case 0xDB:
			cpu->a = cpu->readio(PROGBYTE());
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// CC a ... CALL C,a
		case 0xDC:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_C) != 0)
				{
					I8085_CALL(nn);
					cpu->sync.clock += I8085_CLOCKMUL*(18 - 9);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*9;
			break;

		// JNK a (JNX5 a) ... (Jump on 'Not X5')
		case 0xDD:
// !!!!!!!!!!
			cpu->sync.clock += I8085_CLOCKMUL*7; // 7 or 10
			break;

		// SBI d ... SBC A,d
		case 0xDE:
			{
				u8 n = PROGBYTE();
				I8085_SBC(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RST 3 ... RST 18h
		case 0xDF:
			I8085_CALL(0x18);
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// RPO ... RET PO
		case 0xE0:
			if ((cpu->f & I8085_P) == 0)
			{
				I8085_RET();
				cpu->sync.clock += I8085_CLOCKMUL*(12 - 6);
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// POP H ... POP HL
		case 0xE1:
			I8085_POP(cpu->l, cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// JPO a ... JP PO,a
		case 0xE2:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_P) == 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += I8085_CLOCKMUL*(10 - 7);
				}
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

		// CPO a ... CALL PO,a
		case 0xE4:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_P) == 0)
				{
					I8085_CALL(nn);
					cpu->sync.clock += I8085_CLOCKMUL*(18 - 9);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*9;
			break;

		// PUSH H ... PUSH HL
		case 0xE5:
			I8085_PUSH(cpu->l, cpu->h);
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// ANI d ... AND d
		case 0xE6:
			{
				u8 n = PROGBYTE();
				I8085_AND(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RST 4 ... RST 20h
		case 0xE7:
			I8085_CALL(0x20);
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// RPE ... RET PE
		case 0xE8:
			if ((cpu->f & I8085_P) != 0)
			{
				I8085_RET();
				cpu->sync.clock += I8085_CLOCKMUL*(12 - 6);
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// PCHL ... JP (HL)
		case 0xE9:
			cpu->pc = cpu->hl;
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// JPE a ... JP PE,a
		case 0xEA:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_P) != 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += I8085_CLOCKMUL*(10 - 7);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
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

		// CPE a ... CALL PE,a
		case 0xEC:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_P) != 0)
				{
					I8085_CALL(nn);
					cpu->sync.clock += I8085_CLOCKMUL*(18 - 9);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*9;
			break;

		// LHLX ... (Load Hl from [DE])
		case 0xED:
// !!!!!!!
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// XRI d ... XOR d
		case 0xEE:
			{
				u8 n = PROGBYTE();
				I8085_XOR(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RST 5 ... RST 28h
		case 0xEF:
			I8085_CALL(0x28);
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// RP ... RET P
		case 0xF0:
			if ((cpu->f & I8085_S) == 0)
			{
				I8085_RET();
				cpu->sync.clock += I8085_CLOCKMUL*(12 - 6);
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// POP PSW ... POP AF
		case 0xF1:
			I8085_POP(cpu->f, cpu->a);
			cpu->f = (cpu->f & I8085_FLAGALL) | I8085_FLAGDEF;
			cpu->sync.clock += I8085_CLOCKMUL*10;
			break;

		// JP a ... JP P,a
		case 0xF2:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_S) == 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += I8085_CLOCKMUL*(10 - 7);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// DI
		case 0xF3:
			cpu->ie = 0;
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// CP a ... CALL P,a
		case 0xF4:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_S) == 0)
				{
					I8085_CALL(nn);
					cpu->sync.clock += I8085_CLOCKMUL*(18 - 9);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*9;
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
				u8 n = PROGBYTE();
				I8085_OR(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RST 6 ... RST 30h
		case 0xF7:
			I8085_CALL(0x30);
			cpu->sync.clock += I8085_CLOCKMUL*12;
			break;

		// RM ... RET M
		case 0xF8:
			if ((cpu->f & I8085_S) != 0)
			{
				I8085_RET();
				cpu->sync.clock += I8085_CLOCKMUL*(12 - 6);
			}
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// SPHL ... LD SP,HL
		case 0xF9:
			cpu->sp = cpu->hl;
			cpu->sync.clock += I8085_CLOCKMUL*6;
			break;

		// JM a ... JP M,a
		case 0xFA:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_S) != 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += I8085_CLOCKMUL*(10 - 7);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// EI
		case 0xFB:
			cpu->ie = 1; // enable interrupt
			cpu->tid = 1; // temporary disable interrupt (for next 1 instruction)
			cpu->sync.clock += I8085_CLOCKMUL*4;
			break;

		// CM a ... CALL M,a
		case 0xFC:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & I8085_S) != 0)
				{
					I8085_CALL(nn);
					cpu->sync.clock += I8085_CLOCKMUL*(18 - 9);
				}
			}
			cpu->sync.clock += I8085_CLOCKMUL*9;
			break;

		// JNK a ... (Jump on 'X5')
		case 0xFD:
// !!!!!!!
			cpu->sync.clock += I8085_CLOCKMUL*7; // 7 or 10
			break;

		// CPI d ... CP d
		case 0xFE:
			{
				u8 n = PROGBYTE();
				I8085_CP(n);
			}
			cpu->sync.clock += I8085_CLOCKMUL*7;
			break;

		// RST 7 ... RST 38h
		case 0xFF:
			I8085_CALL(0x38);
			cpu->sync.clock += I8085_CLOCKMUL*12;
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
#if USE_MINIVGA				// use mini-VGA display
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
#if USE_MINIVGA				// use mini-VGA display
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

#if !USE_MINIVGA				// use mini-VGA display
	// core 1 reset
	Core1Reset();
#endif
}

#endif // USE_EMU_I8085
