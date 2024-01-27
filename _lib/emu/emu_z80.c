
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

#if USE_EMU_Z80			// use Z80 CPU emulator

#include "emu_z80_macro.h"

// current CPU descriptor (NULL = not running)
volatile sZ80* Z80_Cpu = NULL;

// flags table with parity (Z80_PV, Z80_Z, Z80_S, Z80_X and Z80_Y flags)
u8 Z80_FlagParTab[256];

// initialize Z80 tables
void Z80_InitTab()
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
		f = i & (Z80_S | Z80_X | Z80_Y);
		if (par == 0) f |= Z80_PV; // even parity
		if (i == 0) f |= Z80_Z; // zero
		Z80_FlagParTab[i] = f;
	}
}

// reset processor
void Z80_Reset(sZ80* cpu)
{
	cpu->pc = 0;
	cpu->sp = 0xffff;
	cpu->r = 0;
	cpu->i = 0;
	cpu->iff1 = 0;		// disable interrupts
	cpu->iff2 = 0;		// disable saved interrupts
	cpu->mode = Z80_INTMODE0;
	cpu->halted = 0;
	cpu->tid = 0;
	cpu->stop = 0;		// stop request
	cpu->fa = 0xffff;
	// ...other registers have random content
	cpu->intreq = 0;	// interrupt request
	cpu->nmi = 0;		// NMI interrupt request
	cpu->cond[0] = Z80_Z;	// condition table
	cpu->cond[1] = Z80_C;	// condition table
	cpu->cond[2] = Z80_PV;	// condition table
	cpu->cond[3] = Z80_S;	// condition table
	cpu->r7 = 0;
	cpu->intins = 0x0D;	// interrupt instruction RST 0x08
}

// load byte from program memory
INLINE u8 Z80_ProgByte(sZ80* cpu)
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
INLINE u16 Z80_ProgWord(sZ80* cpu)
{
	u8 temp = Z80_ProgByte(cpu);
	return (temp | ((u16)Z80_ProgByte(cpu) << 8));
}

#include "emu_z80_ddfd.c"

// execute program (start or continue, until "stop" request)
// Size of code of this function: 10320 code + 2996 jump table = 13316 bytes
// CPU loading at 4 MHz on 120 MHz: used 29-55%, max. 40-55%
void FASTCODE NOFLASH(Z80_Exec)(sZ80* cpu)
{
	u8 op;

	// clear stop flag
	cpu->stop = 0;

	// program loop
	while (cpu->stop == 0)
	{
		// nonmaskable interrupt
		if (cpu->nmi != 0)
		{
			// clear request
			cpu->nmi = 0;

			// save and disable interrupt
			cpu->iff2 = cpu->iff1;
			cpu->iff1 = 0;

			// continue after HALT
			if (cpu->halted != 0)
			{
				cpu->pc++;
				cpu->halted = 0;
			}

			// jump to service
			Z80_CALL(0x66);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			continue;
		}

		// maskable interrupt
		if ((cpu->intreq != 0) && (cpu->iff1 != 0) && (cpu->tid == 0))
		{
			// clear request
			cpu->intreq = 0;

			// disable interrupts during maskable interrupt
			cpu->iff1 = 0;
			cpu->iff2 = 0;

			// continue after HALT
			if (cpu->halted != 0)
			{
				cpu->pc++;
				cpu->halted = 0;
			}

			// prepare service address
			u16 addr;

			// mode 0: execute RST instruction
			if (cpu->mode == Z80_INTMODE0)
				addr = cpu->intins & 0x38;

			// mode 1: jump to address 0x38
			else if (cpu->mode == Z80_INTMODE1)
				addr = 0x38;

			// mode 2: jump to vector
			else
				addr = (cpu->i << 8) | cpu->intins;

			// jump to service
			Z80_CALL(addr);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			continue;
		}

		// clear temporary disable interrupts flag
		cpu->tid = 0;

		// increment memory refresh address
		cpu->r++;

		// get next instruction
		op = Z80_ProgByte(cpu);

		// switch base operation code
		switch (op)
		{
		// NOP
		// case 0x00:
		// LD B,B
		// case 0x40:
		// LD C,C
		// case 0x49:
		// LD D,D
		// case 0x52:
		// LD E,E
		// case 0x5B:
		// LD H,H
		// case 0x64:
		// LD L,L
		// case 0x6D:
		// LD A,A
		// case 0x7F:
		default:
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD BC,nn
		case 0x01:
		// LD DE,nn
		case 0x11:
		// LD HL,nn
		case 0x21:
			{
				u8* r = &cpu->reg[6 - (op >> 3)]; // register low
				r[0] = Z80_ProgByte(cpu); // low
				r[1] = Z80_ProgByte(cpu); // high
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// LD (BC),A
		case 0x02:
			cpu->writemem(cpu->bc, cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// INC BC
		case 0x03:
			cpu->bc++;
			cpu->sync.clock += Z80_CLOCKMUL*6;
			break;

		// INC B
		case 0x04:
		// INC C
		case 0x0C:
		// INC D
		case 0x14:
		// INC E
		case 0x1C:
		// INC H
		case 0x24:
		// INC L
		case 0x2C:
		// INC A
		case 0x3C:
			{
				u8* r = &cpu->reg[7 - (op >> 3)]; // pointer to register
				Z80_INC(*r);
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// DEC B
		case 0x05:
		// DEC C
		case 0x0D:
		// DEC D
		case 0x15:
		// DEC E
		case 0x1D:
		// DEC H
		case 0x25:
		// DEC L
		case 0x2D:
		// DEC A
		case 0x3D:
			{
				u8* r = &cpu->reg[7 - (op >> 3)]; // pointer to register
				Z80_DEC(*r);
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD B,n
		case 0x06:
		// LD C,n
		case 0x0E:
		// LD D,n
		case 0x16:
		// LD E,n
		case 0x1E:
		// LD H,n
		case 0x26:
		// LD L,n
		case 0x2E:
		// LD A,n
		case 0x3E:
			{
				u8* r = &cpu->reg[7 - (op >> 3)]; // pointer to register
				*r = Z80_ProgByte(cpu);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// RLCA
		case 0x07:
			{
				u8 n = cpu->a;
				n = (n << 1) | (n >> 7);
				cpu->a = n;
				//	C ... carry from bit 7
				//	N ... reset
				//	P/V ... not affected
				//	X ... copy from result
				//	H ... reset
				//	Y ... copy from result
				//	Z ... not affected
				//	S ... not affected
				u8 f = cpu->f & (Z80_PV | Z80_Z | Z80_S);
				f |= n & (Z80_C | Z80_X | Z80_Y);
				cpu->f = f;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// EX AF,AF'
		case 0x08:
			{
				u16 nn = cpu->fa;
				cpu->fa = cpu->fa2;
				cpu->fa2 = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADD HL,BC
		case 0x09:
		// ADD HL,DE
		case 0x19:
		// ADD HL,HL
		case 0x29:
			{
				u16* r = &cpu->dreg[3 - (op >> 4)]; // double register
				Z80_ADD16(cpu->hl, *r);
			}
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// LD A,(BC)
		case 0x0A:
			cpu->a = cpu->readmem(cpu->bc);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// DEC BC
		case 0x0B:
			cpu->bc--;
			cpu->sync.clock += Z80_CLOCKMUL*6;
			break;

		// RRCA
		case 0x0F:
			{
				u8 n = cpu->a;
				u8 n2 = (n >> 1) | (n << 7);
				cpu->a = n2;
				//	C ... carry from bit 0
				//	N ... reset
				//	P/V ... not affected
				//	X ... copy from result
				//	H ... reset
				//	Y ... copy from result
				//	Z ... not affected
				//	S ... not affected
				u8 f = cpu->f & (Z80_PV | Z80_Z | Z80_S);
				f |= n2 & (Z80_X | Z80_Y);
				f |= n & Z80_C;
				cpu->f = f;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// DJNZ e
		case 0x10:
			{
				u8 n = cpu->b - 1;
				cpu->b = n;
				s8 jr = (s8)Z80_ProgByte(cpu); // read offset of relative jump
				cpu->sync.clock += Z80_CLOCKMUL*8;
				if (n != 0)
				{
					cpu->pc += jr;
					cpu->sync.clock += Z80_CLOCKMUL*(13 - 8);
				}
			}
			break;

		// LD (DE),A
		case 0x12:
			cpu->writemem(cpu->de, cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// INC DE
		case 0x13:
			cpu->de++;
			cpu->sync.clock += Z80_CLOCKMUL*6;
			break;

		// RLA
		case 0x17:
			{
				u8 n2 = cpu->a;
				u8 n = (n2 << 1) | (cpu->f & Z80_C);
				cpu->a = n;
				//	C ... carry from bit 7
				//	N ... reset
				//	P/V ... not affected
				//	X ... copy from result
				//	H ... reset
				//	Y ... copy from result
				//	Z ... not affected
				//	S ... not affected
				u8 f = cpu->f & (Z80_PV | Z80_Z | Z80_S);
				f |= n & (Z80_X | Z80_Y);
				f |= n2 >> 7;
				cpu->f = f;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// JR e
		case 0x18:
			{
				s8 jr = (s8)Z80_ProgByte(cpu); // read offset of relative jump
				cpu->pc += jr;
			}
			cpu->sync.clock += Z80_CLOCKMUL*12;
			break;

		// LD A,(DE)
		case 0x1A:
			cpu->a = cpu->readmem(cpu->de);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// DEC DE
		case 0x1B:
			cpu->de--;
			cpu->sync.clock += Z80_CLOCKMUL*6;
			break;

		// RRA
		case 0x1F:
			{
				u8 n2 = cpu->a;
				u8 n = (n2 >> 1) | (cpu->f << 7);
				cpu->a = n;
				//	C ... carry from bit 0
				//	N ... reset
				//	P/V ... not affected
				//	X ... copy from result
				//	H ... reset
				//	Y ... copy from result
				//	Z ... not affected
				//	S ... not affected
				u8 f = cpu->f & (Z80_PV | Z80_Z | Z80_S);
				f |= n & (Z80_X | Z80_Y);
				f |= n2 & Z80_C;
				cpu->f = f;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// JR NZ,e
		case 0x20:
			{
				s8 jr = (s8)Z80_ProgByte(cpu); // read offset of relative jump
				cpu->sync.clock += Z80_CLOCKMUL*7;
				if ((cpu->f & Z80_Z) == 0)
				{
					cpu->pc += jr;
					cpu->sync.clock += Z80_CLOCKMUL*(12 - 7);
				}
			}
			break;

		// LD (a),HL
		case 0x22:
			{
				u16 nn = Z80_ProgWord(cpu);
				cpu->writemem(nn, cpu->l);
				nn++;
				cpu->writemem(nn, cpu->h);
			}
			cpu->sync.clock += Z80_CLOCKMUL*16;
			break;

		// INC HL
		case 0x23:
			cpu->hl++;
			cpu->sync.clock += Z80_CLOCKMUL*6;
			break;

		// DAA
		case 0x27:
			{
				u8 f = cpu->f;
				u8 a = cpu->a;

				u8 add = 0;
				u8 carry = 0;
				if ((a > 0x99) || ((f & Z80_C) != 0))
				{
					carry = Z80_C;
					add = 0x60;
				}

				if (((a & 0x0f) > 0x09) || ((f & Z80_H) != 0)) add += 0x06;
				
				u8 a2;
				u8 f2 = f;
				f &= Z80_N;
				if (f != 0) // negative
				{
					// subtraction
					if (((a & 0x0f) < 6) && ((f2 & Z80_H) != 0)) f |= Z80_H;
					a2 = a - add;
				}
				else
				{
					// addition
					if ((a & 0x0f) > 0x09) f |= Z80_H;
					a2 = a + add;
				}
				cpu->a = a2;

				//	C ... carry if add = 0x60 or 0x66
				//	N ... not affected
				//	P/V ... set if even parity
				//	X ... copy from result
				//	H ... half carry
				//	Y ... copy from result
				//	Z ... set if result is 0
				//	S ... copy from result
				f |= carry;
				f |= Z80_FlagParTab[a2];
				cpu->f = f;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// JR Z,e
		case 0x28:
			{
				s8 jr = (s8)Z80_ProgByte(cpu); // read offset of relative jump
				cpu->sync.clock += Z80_CLOCKMUL*7;
				if ((cpu->f & Z80_Z) != 0)
				{
					cpu->pc += jr;
					cpu->sync.clock += Z80_CLOCKMUL*(12 - 7);
				}
			}
			break;

		// LD HL,(a)
		case 0x2A:
			{
				u16 nn = Z80_ProgWord(cpu);
				cpu->l = cpu->readmem(nn);
				nn++;
				cpu->h = cpu->readmem(nn);
			}
			cpu->sync.clock += Z80_CLOCKMUL*16;
			break;

		// DEC HL
		case 0x2B:
			cpu->hl--;
			cpu->sync.clock += Z80_CLOCKMUL*6;
			break;

		// CPL
		case 0x2F:
			{
				u8 n = ~cpu->a;
				cpu->a = n;
				//	C ... not affected
				//	N ... set
				//	P/V ... not affected
				//	X ... copy from result
				//	H ... set
				//	Y ... copy from result
				//	Z ... not affected
				//	S ... not affected
				u8 f = cpu->f & (Z80_C | Z80_PV | Z80_Z | Z80_S);
				f |= n & (Z80_X | Z80_Y);
				f |= Z80_N | Z80_H;
				cpu->f = f;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// JR NC,e
		case 0x30:
			{
				s8 jr = (s8)Z80_ProgByte(cpu); // read offset of relative jump
				cpu->sync.clock += Z80_CLOCKMUL*7;
				if ((cpu->f & Z80_C) == 0)
				{
					cpu->pc += jr;
					cpu->sync.clock += Z80_CLOCKMUL*(12 - 7);
				}
			}
			break;

		// LD SP,nn
		case 0x31:
			cpu->spl = Z80_ProgByte(cpu);
			cpu->sph = Z80_ProgByte(cpu);
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// LD (a),A
		case 0x32:
			{
				u16 nn = Z80_ProgWord(cpu);
				cpu->writemem(nn, cpu->a);
			}
			cpu->sync.clock += Z80_CLOCKMUL*13;
			break;

		// INC SP
		case 0x33:
			cpu->sp++;
			cpu->sync.clock += Z80_CLOCKMUL*6;
			break;

		// INC (HL)
		case 0x34:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				Z80_INC(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// DEC (HL)
		case 0x35:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				Z80_DEC(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// LD (HL),n
		case 0x36:
			cpu->writemem(cpu->hl, Z80_ProgByte(cpu));
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// SCF
		case 0x37:
			{
				//	C ... set
				//	N ... reset
				//	P/V ... not affected
				//	X ... copy from A
				//	H ... reset
				//	Y ... copy from A
				//	Z ... not affected
				//	S ... not affected
				u8 f = cpu->f & (Z80_PV | Z80_Z | Z80_S);
				f |= cpu->a & (Z80_X | Z80_Y);
				f |= Z80_C;
				cpu->f = f;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// JR C,e
		case 0x38:
			{
				s8 jr = (s8)Z80_ProgByte(cpu); // read offset of relative jump
				cpu->sync.clock += Z80_CLOCKMUL*7;
				if ((cpu->f & Z80_C) != 0)
				{
					cpu->pc += jr;
					cpu->sync.clock += Z80_CLOCKMUL*(12 - 7);
				}
			}
			break;

		// ADD HL,SP
		case 0x39:
			Z80_ADD16(cpu->hl, cpu->sp);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// LD A,(a)
		case 0x3A:
			{
				u16 nn = Z80_ProgWord(cpu);
				cpu->a = cpu->readmem(nn);
			}
			cpu->sync.clock += Z80_CLOCKMUL*13;
			break;

		// DEC SP
		case 0x3B:
			cpu->sp--;
			cpu->sync.clock += Z80_CLOCKMUL*6;
			break;

		// CCF
		case 0x3F:
			{
				//	C ... inverted
				//	N ... reset
				//	P/V ... not affected
				//	X ... copy from A
				//	H ... previous carry
				//	Y ... copy from A
				//	Z ... not affected
				//	S ... not affected
				u8 f0 = cpu->f;
				u8 f = f0 & (Z80_PV | Z80_Z | Z80_S);
				f |= ((f0 & Z80_C) == 0) ? Z80_C : Z80_H;
				f |= cpu->a & (Z80_X | Z80_Y);
				cpu->f = f;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD B,B
		// case 0x40: ... see 0x00 NOP
		// LD B,C
		case 0x41:
		// LD B,D
		case 0x42:
		// LD B,E
		case 0x43:
		// LD B,H
		case 0x44:
		// LD B,L
		case 0x45:
		// LD B,A
		case 0x47:
		// LD C,B
		case 0x48:
		// LD C,C
		// case 0x49: ... see 0x00 NOP
		// LD C,D
		case 0x4A:
		// LD C,E
		case 0x4B:
		// LD C,H
		case 0x4C:
		// LD C,L
		case 0x4D:
		// LD C,A
		case 0x4F:
		// LD D,B
		case 0x50:
		// LD D,C
		case 0x51:
		// LD D,D
		// case 0x52: ... see 0x00 NOP
		// LD D,E
		case 0x53:
		// LD D,H
		case 0x54:
		// LD D,L
		case 0x55:
		// LD D,A
		case 0x57:
		// LD E,B
		case 0x58:
		// LD E,C
		case 0x59:
		// LD E,D
		case 0x5A:
		// LD E,E
		// case 0x5B: ... see 0x00 NOP
		// LD E,H
		case 0x5C:
		// LD E,L
		case 0x5D:
		// LD E,A
		case 0x5F:
		// LD H,B
		case 0x60:
		// LD H,C
		case 0x61:
		// LD H,D
		case 0x62:
		// LD H,E
		case 0x63:
		// LD H,H
		// case 0x64: ... see 0x00 NOP
		// LD H,L
		case 0x65:
		// LD H,A
		case 0x67:
		// LD L,B
		case 0x68:
		// LD L,C
		case 0x69:
		// LD L,D
		case 0x6A:
		// LD L,E
		case 0x6B:
		// LD L,H
		case 0x6C:
		// LD L,L
		// case 0x6D: ... see 0x00 NOP
		// LD L,A
		case 0x6F:
		// LD A,B
		case 0x78:
		// LD A,C
		case 0x79:
		// LD A,D
		case 0x7A:
		// LD A,E
		case 0x7B:
		// LD A,H
		case 0x7C:
		// LD A,L
		case 0x7D:
		// LD A,A
		// case 0x7F: ... see 0x00 NOP
			{
				op -= 0x40;
				u8* rd = &cpu->reg[7 - (op >> 3)]; // destination register
				u8* rs = &cpu->reg[7 - (op & 7)]; // source register
				*rd = *rs;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD B,(HL)
		case 0x46:
		// LD C,(HL)
		case 0x4E:
		// LD D,(HL)
		case 0x56:
		// LD E,(HL)
		case 0x5E:
		// LD H,(HL)
		case 0x66:
		// LD L,(HL)
		case 0x6E:
		// LD A,(HL)
		case 0x7E:
			{
				op -= 0x40;
				u8* rd = &cpu->reg[7 - (op >> 3)]; // destination register
				*rd = cpu->readmem(cpu->hl);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD (HL),B
		case 0x70:
		// LD (HL),C
		case 0x71:
		// LD (HL),D
		case 0x72:
		// LD (HL),E
		case 0x73:
		// LD (HL),H
		case 0x74:
		// LD (HL),L
		case 0x75:
		// LD (HL),A
		case 0x77:
			{
				u8* rs = &cpu->reg[7 - (op - 0x70)]; // source register
				cpu->writemem(cpu->hl, *rs);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// HALT
		case 0x76:
			cpu->pc--;
			cpu->halted = 1;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADD A,B
		case 0x80:
		// ADD A,C
		case 0x81:
		// ADD A,D
		case 0x82:
		// ADD A,E
		case 0x83:
		// ADD A,H
		case 0x84:
		// ADD A,L
		case 0x85:
		// ADD A,A
		case 0x87:
			{
				u8* rs = &cpu->reg[7 - (op - 0x80)]; // source register
				Z80_ADD(*rs);
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADD A,(HL)
		case 0x86:
			{
				u8 n = cpu->readmem(cpu->hl);
				Z80_ADD(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// ADC A,B
		case 0x88:
		// ADC A,C
		case 0x89:
		// ADC A,D
		case 0x8A:
		// ADC A,E
		case 0x8B:
		// ADC A,H
		case 0x8C:
		// ADC A,L
		case 0x8D:
		// ADC A,A
		case 0x8F:
			{
				u8* rs = &cpu->reg[7 - (op - 0x88)]; // source register
				Z80_ADC(*rs);
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADC A,(HL)
		case 0x8E:
			{
				u8 n = cpu->readmem(cpu->hl);
				Z80_ADC(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
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
				Z80_SUB(*rs);
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SUB (HL)
		case 0x96:
			{
				u8 n = cpu->readmem(cpu->hl);
				Z80_SUB(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// SBC A,B
		case 0x98:
		// SBC A,C
		case 0x99:
		// SBC A,D
		case 0x9A:
		// SBC A,E
		case 0x9B:
		// SBC A,H
		case 0x9C:
		// SBC A,L
		case 0x9D:
		// SBC A,A
		case 0x9F:
			{
				u8* rs = &cpu->reg[7 - (op - 0x98)]; // source register
				Z80_SBC(*rs);
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SBC A,(HL)
		case 0x9E:
			{
				u8 n = cpu->readmem(cpu->hl);
				Z80_SBC(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// AND B
		case 0xA0:
		// AND C
		case 0xA1:
		// AND D
		case 0xA2:
		// AND E
		case 0xA3:
		// AND H
		case 0xA4:
		// AND L
		case 0xA5:
		// AND A
		case 0xA7:
			{
				u8* rs = &cpu->reg[7 - (op - 0xa0)]; // source register
				Z80_AND(*rs);
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// AND (HL)
		case 0xA6:
			{
				u8 n = cpu->readmem(cpu->hl);
				Z80_AND(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// XOR B
		case 0xA8:
		// XOR C
		case 0xA9:
		// XOR D
		case 0xAA:
		// XOR E
		case 0xAB:
		// XOR H
		case 0xAC:
		// XOR L
		case 0xAD:
		// XOR A
		case 0xAF:
			{
				u8* rs = &cpu->reg[7 - (op - 0xa8)]; // source register
				Z80_XOR(*rs);
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// XOR (HL)
		case 0xAE:
			{
				u8 n = cpu->readmem(cpu->hl);
				Z80_XOR(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// OR B
		case 0xB0:
		// OR C
		case 0xB1:
		// OR D
		case 0xB2:
		// OR E
		case 0xB3:
		// OR H
		case 0xB4:
		// OR L
		case 0xB5:
		// OR A
		case 0xB7:
			{
				u8* rs = &cpu->reg[7 - (op - 0xb0)]; // source register
				Z80_OR(*rs);
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// OR (HL)
		case 0xB6:
			{
				u8 n = cpu->readmem(cpu->hl);
				Z80_OR(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// CP B
		case 0xB8:
		// CP C
		case 0xB9:
		// CP D
		case 0xBA:
		// CP E
		case 0xBB:
		// CP H
		case 0xBC:
		// CP L
		case 0xBD:
		// CP A
		case 0xBF:
			{
				u8* rs = &cpu->reg[7 - (op - 0xb8)]; // source register
				Z80_CP(*rs);
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// CP (HL)
		case 0xBE:
			{
				u8 n = cpu->readmem(cpu->hl);
				Z80_CP(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// RET NZ
		case 0xC0:
		// RET Z
		case 0xC8:
		// RET NC
		case 0xD0:
		// RET C
		case 0xD8:
		// RET PO
		case 0xE0:
		// RET PE
		case 0xE8:
		// RET P
		case 0xF0:
		// RET M
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
					Z80_RET();
					cpu->sync.clock += Z80_CLOCKMUL*(11 - 5);
				}
			}
			cpu->sync.clock += Z80_CLOCKMUL*5;
			break;

		// POP BC
		case 0xC1:
		// POP DE
		case 0xD1:
		// POP HL
		case 0xE1:
			{
				u8* r = &cpu->reg[6 - ((op - 0xc1) >> 3)]; // register low
				Z80_POP(r[0], r[1]);
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// JP NZ,a
		case 0xC2:
		// JP Z,a
		case 0xCA:
		// JP NC,a
		case 0xD2:
		// JP C,a
		case 0xDA:
		// JP PO,a
		case 0xE2:
		// JP PE,a
		case 0xEA:
		// JP P,a
		case 0xF2:
		// JP M,a
		case 0xFA:
			{
				// read destination address -> nn
				u16 nn = Z80_ProgWord(cpu);

				// flag bit
				int fb = cpu->cond[(op - 0xc2) >> 4];

				// invert flags
				u8 f = cpu->f;
				if ((op & B3) != 0) f = ~f;

				// check condition
				if ((f & fb) == 0) cpu->pc = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// JP a
		case 0xC3:
			cpu->pc = Z80_ProgWord(cpu);
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// CALL NZ,a
		case 0xC4:
		// CALL Z,a
		case 0xCC:
		// CALL NC,a
		case 0xD4:
		// CALL C,a
		case 0xDC:
		// CALL PO,a
		case 0xE4:
		// CALL PE,a
		case 0xEC:
		// CALL P,a
		case 0xF4:
		// CALL M,a
		case 0xFC:
			{
				// read destination address -> nn
				u16 nn = Z80_ProgWord(cpu);

				// flag bit
				int fb = cpu->cond[(op - 0xc4) >> 4];

				// invert flags
				u8 f = cpu->f;
				if ((op & B3) != 0) f = ~f;

				// check condition
				if ((f & fb) == 0)
				{
					Z80_CALL(nn);
					cpu->sync.clock += Z80_CLOCKMUL*(17 - 10);
				}
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// PUSH BC
		case 0xC5:
		// PUSH DE
		case 0xD5:
		// PUSH HL
		case 0xE5:
			{
				u8* r = &cpu->reg[6 - ((op - 0xc5) >> 3)]; // register low
				Z80_PUSH(r[0], r[1]);
			}
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// ADD A,d
		case 0xC6:
			{
				u8 n = Z80_ProgByte(cpu);
				Z80_ADD(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// RST 00h
		case 0xC7:
		// RST 08h
		case 0xCF:
		// RST 10h
		case 0xD7:
		// RST 18h
		case 0xDF:
		// RST 20h
		case 0xE7:
		// RST 28h
		case 0xEF:
		// RST 30h
		case 0xF7:
		// RST 38h
		case 0xFF:
			Z80_CALL(op & 0x38);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// RET
		case 0xC9:
			Z80_RET();
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// prefix CB
		case 0xCB:
			{
				cpu->r++; // increment memory refresh address
				op = Z80_ProgByte(cpu); // read next program byte
				u8 low = op & 0x07; // low 3 bits of the program byte
				op >>= 3; // high 5 bits of the program byte
#include "emu_z80_cb.c"
			}
			break;

		// CALL a
		case 0xCD:
			{
				u16 nn = Z80_ProgWord(cpu);
				Z80_CALL(nn);
			}
			cpu->sync.clock += Z80_CLOCKMUL*17;
			break;

		// ADC A,d
		case 0xCE:
			{
				u8 n = Z80_ProgByte(cpu);
				Z80_ADC(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// OUT (d),A
		case 0xD3:
			{
				u8 n = Z80_ProgByte(cpu);
				u8 a = cpu->a;
				u16 nn = n + (a << 8);
				cpu->writeport(nn, a);
			}
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// SUB d
		case 0xD6:
			{
				u8 n = Z80_ProgByte(cpu);
				Z80_SUB(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// EXX
		case 0xD9:
			{
				u16 nn = cpu->bc;
				cpu->bc = cpu->bc2;
				cpu->bc2 = nn;

				nn = cpu->de;
				cpu->de = cpu->de2;
				cpu->de2 = nn;

				nn = cpu->hl;
				cpu->hl = cpu->hl2;
				cpu->hl2 = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// IN A,(d)
		case 0xDB:
			{
				u8 n = Z80_ProgByte(cpu);
				u8 a = cpu->a;
				u16 nn = n + (a << 8);
				cpu->a = cpu->readport(nn);
			}
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// prefix DD
		case 0xDD:
			cpu->r++; // increment memory refresh address
			Z80_ExecDDFD(cpu, &cpu->ixl);
			break;

		// SBC A,d
		case 0xDE:
			{
				u8 n = Z80_ProgByte(cpu);
				Z80_SBC(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// EX (SP),HL
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
			cpu->sync.clock += Z80_CLOCKMUL*19;
			break;

		// AND d
		case 0xE6:
			{
				u8 n = Z80_ProgByte(cpu);
				Z80_AND(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// JP (HL)
		case 0xE9:
			cpu->pc = cpu->hl;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// EX DE.HL
		case 0xEB:
			{
				u16 nn = cpu->hl;
				cpu->hl = cpu->de;
				cpu->de = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// prefix ED
		case 0xED:
			cpu->r++; // increment memory refresh address
#include "emu_z80_ed.c"
			break;

		// XOR d
		case 0xEE:
			{
				u8 n = Z80_ProgByte(cpu);
				Z80_XOR(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// POP AF
		case 0xF1:
			Z80_POP(cpu->f, cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// DI
		case 0xF3:
			cpu->iff1 = 0;
			cpu->iff2 = 0;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// PUSH AF
		case 0xF5:
			Z80_PUSH(cpu->f, cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// OR d
		case 0xF6:
			{
				u8 n = Z80_ProgByte(cpu);
				Z80_OR(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD SP,HL
		case 0xF9:
			cpu->sp = cpu->hl;
			cpu->sync.clock += Z80_CLOCKMUL*6;
			break;

		// EI
		case 0xFB:
			cpu->iff1 = 1;
			cpu->iff2 = 1;
			cpu->tid = 1; // temporary disable interrupt (to disable 1 next instruction)
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// prefix FD
		case 0xFD:
			cpu->r++; // increment memory refresh address
			Z80_ExecDDFD(cpu, &cpu->iyl);
			break;

		// CP d
		case 0xFE:
			{
				u8 n = Z80_ProgByte(cpu);
				Z80_CP(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;
		}

		// time synchronization
		EmuSync(&cpu->sync);
	}
}

// execute Z80 emulator on CPU 1
static void Z80_ExecCore1()
{
	// start time synchronization
	Z80_SyncStart((sZ80*)Z80_Cpu);

	// execute
	Z80_Exec((sZ80*)Z80_Cpu);

	// clear descriptor
	Z80_Cpu = NULL;
	dsb();
}

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*Z80_CLOCKMUL is recommended to maintain.
u32 Z80_Start(sZ80* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (Z80_Cpu != NULL) Z80_Stop(pwm);

	// initialize time synchronization
	freq = Z80_SyncInit(cpu, pwm, freq);

	// reset processor
	Z80_Reset(cpu);

	// current CPU descriptor
	Z80_Cpu = cpu;
	dsb();

	// execute program
#if USE_MINIVGA				// use mini-VGA display
	VgaCore1Exec(Z80_ExecCore1);
#else
	Core1Exec(Z80_ExecCore1);
#endif

	return freq;
}

// continue emulation without reset processor
u32 Z80_Cont(sZ80* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (Z80_Cpu != NULL) Z80_Stop(pwm);

	// initialize time synchronization
	freq = Z80_SyncInit(cpu, pwm, freq);

	// current CPU descriptor
	Z80_Cpu = cpu;
	dsb();

	// execute program
#if USE_MINIVGA				// use mini-VGA display
	VgaCore1Exec(Z80_ExecCore1);
#else
	Core1Exec(Z80_ExecCore1);
#endif

	return freq;
}

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void Z80_Stop(int pwm)
{
	// get CPU descriptor
	sZ80* cpu = (sZ80*)Z80_Cpu;

	// check if emulation is running
	if (cpu == NULL) return;

	// request to stop emulation
	cpu->stop = 1;

	// wait to complete
	while (Z80_Cpu != NULL) { dsb(); }

	// terminate time synchronization (stop PWM counter)
	Z80_SyncTerm(pwm);

#if !USE_MINIVGA				// use mini-VGA display
	// core 1 reset
	Core1Reset();
#endif
}

#endif // USE_EMU_Z80
