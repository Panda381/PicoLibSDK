
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
		par = (par == 0) ? Z80_PV : 0;

		// parity flags
		Z80_FlagParTab[i] = (u8)(par | (i & (Z80_S | Z80_X | Z80_Y)));
	}

	// add zero flag
	Z80_FlagParTab[0] |= Z80_Z;
}

// reset processor
void Z80_Reset(sZ80* cpu)
{
	// disable interrupts, set interrupt mode to 0
	cpu->iff1 = 0;
	cpu->iff2 = 0;
	cpu->mode = Z80_INTMODE0;
	cpu->halted = 0;
	cpu->tid = 0;

	// clear program counter
	cpu->pc = 0;

	// clear IR registers
	cpu->r = 0;
	cpu->r7 = 0;
	cpu->i = 0;

	// initialize AF and SP (other registers are undefined)
	cpu->sp = 0xffff;
	cpu->af = 0xffff;
}

// execute program (start or continue, until "stop" request)
//  C code size in Flash: 42840 bytes (optimization -Os)
void Z80_Exec(sZ80* cpu)
{
	u8 op;

	// clear stop flag
	cpu->stop = 0;

	// program loop
	while (cpu->stop == 0)
	{
		// interrupt



		// clear temporary disable interrupts flag
		cpu->tid = 0;

		// increment memory refresh address
		cpu->r++;

		// switch base operation code
		switch (PROGBYTE())
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
			cpu->c = PROGBYTE();
			cpu->b = PROGBYTE();
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
			Z80_INC(cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// DEC B
		case 0x05:
			Z80_DEC(cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD B,n
		case 0x06:
			cpu->b = PROGBYTE();
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
				u16 nn = cpu->af;
				cpu->af = cpu->af2;
				cpu->af2 = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADD HL,BC
		case 0x09:
			Z80_ADD16(hl, bc);
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

		// INC C
		case 0x0C:
			Z80_INC(cpu->c);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// DEC C
		case 0x0D:
			Z80_DEC(cpu->c);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD C,n
		case 0x0E:
			cpu->c = PROGBYTE();
			cpu->sync.clock += Z80_CLOCKMUL*7;
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
				s8 jr = (s8)PROGBYTE(); // read offset of relative jump
				cpu->sync.clock += Z80_CLOCKMUL*8;
				if (n != 0)
				{
					cpu->pc += jr;
					cpu->sync.clock += Z80_CLOCKMUL*(13 - 8);
				}
			}
			break;

		// LD DE,nn
		case 0x11:
			cpu->e = PROGBYTE();
			cpu->d = PROGBYTE();
			cpu->sync.clock += Z80_CLOCKMUL*10;
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

		// INC D
		case 0x14:
			Z80_INC(cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// DEC D
		case 0x15:
			Z80_DEC(cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD D,n
		case 0x16:
			cpu->d = PROGBYTE();
			cpu->sync.clock += Z80_CLOCKMUL*7;
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
				s8 jr = (s8)PROGBYTE(); // read offset of relative jump
				cpu->pc += jr;
			}
			cpu->sync.clock += Z80_CLOCKMUL*12;
			break;

		// ADD HL,DE
		case 0x19:
			Z80_ADD16(hl, de);
			cpu->sync.clock += Z80_CLOCKMUL*11;
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

		// INC E
		case 0x1C:
			Z80_INC(cpu->e);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// DEC E
		case 0x1D:
			Z80_DEC(cpu->e);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD E,n
		case 0x1E:
			cpu->e = PROGBYTE();
			cpu->sync.clock += Z80_CLOCKMUL*7;
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
				s8 jr = (s8)PROGBYTE(); // read offset of relative jump
				cpu->sync.clock += Z80_CLOCKMUL*7;
				if ((cpu->f & Z80_Z) == 0)
				{
					cpu->pc += jr;
					cpu->sync.clock += Z80_CLOCKMUL*(12 - 7);
				}
			}
			break;

		// LD HL,nn
		case 0x21:
			cpu->l = PROGBYTE();
			cpu->h = PROGBYTE();
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// LD (a),HL
		case 0x22:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
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

		// INC H
		case 0x24:
			Z80_INC(cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// DEC H
		case 0x25:
			Z80_DEC(cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD H,n
		case 0x26:
			cpu->h = PROGBYTE();
			cpu->sync.clock += Z80_CLOCKMUL*7;
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
				s8 jr = (s8)PROGBYTE(); // read offset of relative jump
				cpu->sync.clock += Z80_CLOCKMUL*7;
				if ((cpu->f & Z80_Z) != 0)
				{
					cpu->pc += jr;
					cpu->sync.clock += Z80_CLOCKMUL*(12 - 7);
				}
			}
			break;

		// ADD HL,HL
		case 0x29:
			Z80_ADD16(hl, hl);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// LD HL,(a)
		case 0x2A:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
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

		// INC L
		case 0x2C:
			Z80_INC(cpu->l);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// DEC L
		case 0x2D:
			Z80_DEC(cpu->l);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD L,n
		case 0x2E:
			cpu->l = PROGBYTE();
			cpu->sync.clock += Z80_CLOCKMUL*7;
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
				s8 jr = (s8)PROGBYTE(); // read offset of relative jump
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
			cpu->spl = PROGBYTE();
			cpu->sph = PROGBYTE();
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// LD (a),A
		case 0x32:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
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
			cpu->writemem(cpu->hl, PROGBYTE());
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
				s8 jr = (s8)PROGBYTE(); // read offset of relative jump
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
			Z80_ADD16(hl, sp);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// LD A,(a)
		case 0x3A:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				cpu->a = cpu->readmem(nn);
			}
			cpu->sync.clock += Z80_CLOCKMUL*13;
			break;

		// DEC SP
		case 0x3B:
			cpu->sp--;
			cpu->sync.clock += Z80_CLOCKMUL*6;
			break;

		// INC A
		case 0x3C:
			Z80_INC(cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// DEC A
		case 0x3D:
			Z80_DEC(cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD A,n
		case 0x3E:
			cpu->a = PROGBYTE();
			cpu->sync.clock += Z80_CLOCKMUL*7;
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
			cpu->b = cpu->c;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD B,D
		case 0x42:
			cpu->b = cpu->d;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD B,E
		case 0x43:
			cpu->b = cpu->e;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD B,H
		case 0x44:
			cpu->b = cpu->h;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD B,L
		case 0x45:
			cpu->b = cpu->l;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD B,(HL)
		case 0x46:
			cpu->b = cpu->readmem(cpu->hl);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD B,A
		case 0x47:
			cpu->b = cpu->a;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD C,B
		case 0x48:
			cpu->c = cpu->b;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD C,C
		// case 0x49: ... see 0x00 NOP

		// LD C,D
		case 0x4A:
			cpu->c = cpu->d;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD C,E
		case 0x4B:
			cpu->c = cpu->e;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD C,H
		case 0x4C:
			cpu->c = cpu->h;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD C,L
		case 0x4D:
			cpu->c = cpu->l;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD C,(HL)
		case 0x4E:
			cpu->c = cpu->readmem(cpu->hl);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD C,A
		case 0x4F:
			cpu->c = cpu->a;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD D,B
		case 0x50:
			cpu->d = cpu->b;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD D,C
		case 0x51:
			cpu->d = cpu->c;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD D,D
		// case 0x52: ... see 0x00 NOP

		// LD D,E
		case 0x53:
			cpu->d = cpu->e;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD D,H
		case 0x54:
			cpu->d = cpu->h;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD D,L
		case 0x55:
			cpu->d = cpu->l;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD D,(HL)
		case 0x56:
			cpu->d = cpu->readmem(cpu->hl);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD D,A
		case 0x57:
			cpu->d = cpu->a;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD E,B
		case 0x58:
			cpu->e = cpu->b;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD E,C
		case 0x59:
			cpu->e = cpu->c;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD E,D
		case 0x5A:
			cpu->e = cpu->d;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD E,E
		// case 0x5B: ... see 0x00 NOP

		// LD E,H
		case 0x5C:
			cpu->e = cpu->h;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD E,L
		case 0x5D:
			cpu->e = cpu->l;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD E,(HL)
		case 0x5E:
			cpu->e = cpu->readmem(cpu->hl);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD E,A
		case 0x5F:
			cpu->e = cpu->a;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD H,B
		case 0x60:
			cpu->h = cpu->b;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD H,C
		case 0x61:
			cpu->h = cpu->c;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD H,D
		case 0x62:
			cpu->h = cpu->d;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD H,E
		case 0x63:
			cpu->h = cpu->e;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD H,H
		// case 0x64: ... see 0x00 NOP

		// LD H,L
		case 0x65:
			cpu->h = cpu->l;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD H,(HL)
		case 0x66:
			cpu->h = cpu->readmem(cpu->hl);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD H,A
		case 0x67:
			cpu->h = cpu->a;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD L,B
		case 0x68:
			cpu->l = cpu->b;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD L,C
		case 0x69:
			cpu->l = cpu->c;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD L,D
		case 0x6A:
			cpu->l = cpu->d;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD L,E
		case 0x6B:
			cpu->l = cpu->e;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD L,H
		case 0x6C:
			cpu->l = cpu->h;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD L,L
		// case 0x6D: ... see 0x00 NOP

		// LD L,(HL)
		case 0x6E:
			cpu->l = cpu->readmem(cpu->hl);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD L,A
		case 0x6F:
			cpu->l = cpu->a;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD (HL),B
		case 0x70:
			cpu->writemem(cpu->hl, cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD (HL),C
		case 0x71:
			cpu->writemem(cpu->hl, cpu->c);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD (HL),D
		case 0x72:
			cpu->writemem(cpu->hl, cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD (HL),E
		case 0x73:
			cpu->writemem(cpu->hl, cpu->e);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD (HL),H
		case 0x74:
			cpu->writemem(cpu->hl, cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD (HL),L
		case 0x75:
			cpu->writemem(cpu->hl, cpu->l);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// HALT
		case 0x76:
			cpu->pc--;
			cpu->halted = 1;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD (HL),A
		case 0x77:
			cpu->writemem(cpu->hl, cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD A,B
		case 0x78:
			cpu->a = cpu->b;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD A,C
		case 0x79:
			cpu->a = cpu->c;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD A,D
		case 0x7A:
			cpu->a = cpu->d;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD A,E
		case 0x7B:
			cpu->a = cpu->e;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD A,H
		case 0x7C:
			cpu->a = cpu->h;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD A,L
		case 0x7D:
			cpu->a = cpu->l;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// LD A,(HL)
		case 0x7E:
			cpu->a = cpu->readmem(cpu->hl);
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// LD A,A
		// case 0x7F: ... see 0x00 NOP

		// ADD A,B
		case 0x80:
			Z80_ADD(cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADD A,C
		case 0x81:
			Z80_ADD(cpu->c);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADD A,D
		case 0x82:
			Z80_ADD(cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADD A,E
		case 0x83:
			Z80_ADD(cpu->e);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADD A,H
		case 0x84:
			Z80_ADD(cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADD A,L
		case 0x85:
			Z80_ADD(cpu->l);
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

		// ADD A,A
		case 0x87:
			Z80_ADD(cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADC A,B
		case 0x88:
			Z80_ADC(cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADC A,C
		case 0x89:
			Z80_ADC(cpu->c);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADC A,D
		case 0x8A:
			Z80_ADC(cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADC A,E
		case 0x8B:
			Z80_ADC(cpu->e);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADC A,H
		case 0x8C:
			Z80_ADC(cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// ADC A,L
		case 0x8D:
			Z80_ADC(cpu->l);
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

		// ADC A,A
		case 0x8F:
			Z80_ADC(cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SUB B
		case 0x90:
			Z80_SUB(cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SUB C
		case 0x91:
			Z80_SUB(cpu->c);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SUB D
		case 0x92:
			Z80_SUB(cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SUB E
		case 0x93:
			Z80_SUB(cpu->e);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SUB H
		case 0x94:
			Z80_SUB(cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SUB L
		case 0x95:
			Z80_SUB(cpu->l);
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

		// SUB A
		case 0x97:
			Z80_SUB(cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SBC A,B
		case 0x98:
			Z80_SBC(cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SBC A,C
		case 0x99:
			Z80_SBC(cpu->c);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SBC A,D
		case 0x9A:
			Z80_SBC(cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SBC A,E
		case 0x9B:
			Z80_SBC(cpu->e);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SBC A,H
		case 0x9C:
			Z80_SBC(cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// SBC A,L
		case 0x9D:
			Z80_SBC(cpu->l);
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

		// SBC A,A
		case 0x9F:
			Z80_SBC(cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// AND B
		case 0xA0:
			Z80_AND(cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// AND C
		case 0xA1:
			Z80_AND(cpu->c);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// AND D
		case 0xA2:
			Z80_AND(cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// AND E
		case 0xA3:
			Z80_AND(cpu->e);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// AND H
		case 0xA4:
			Z80_AND(cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// AND L
		case 0xA5:
			Z80_AND(cpu->l);
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

		// AND A
		case 0xA7:
			Z80_AND(cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// XOR B
		case 0xA8:
			Z80_XOR(cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// XOR C
		case 0xA9:
			Z80_XOR(cpu->c);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// XOR D
		case 0xAA:
			Z80_XOR(cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// XOR E
		case 0xAB:
			Z80_XOR(cpu->e);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// XOR H
		case 0xAC:
			Z80_XOR(cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// XOR L
		case 0xAD:
			Z80_XOR(cpu->l);
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

		// XOR A
		case 0xAF:
			Z80_XOR(cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// OR B
		case 0xB0:
			Z80_OR(cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// OR C
		case 0xB1:
			Z80_OR(cpu->c);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// OR D
		case 0xB2:
			Z80_OR(cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// OR E
		case 0xB3:
			Z80_OR(cpu->e);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// OR H
		case 0xB4:
			Z80_OR(cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// OR L
		case 0xB5:
			Z80_OR(cpu->l);
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

		// OR A
		case 0xB7:
			Z80_OR(cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// CP B
		case 0xB8:
			Z80_CP(cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// CP C
		case 0xB9:
			Z80_CP(cpu->c);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// CP D
		case 0xBA:
			Z80_CP(cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// CP E
		case 0xBB:
			Z80_CP(cpu->e);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// CP H
		case 0xBC:
			Z80_CP(cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// CP L
		case 0xBD:
			Z80_CP(cpu->l);
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

		// CP A
		case 0xBF:
			Z80_CP(cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// RET NZ
		case 0xC0:
			cpu->sync.clock += Z80_CLOCKMUL*5;
			if ((cpu->f & Z80_Z) == 0)
			{
				Z80_RET();
				cpu->sync.clock += Z80_CLOCKMUL*(11 - 5);
			}
			break;

		// POP BC
		case 0xC1:
			Z80_POP(cpu->c, cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// JP NZ,a
		case 0xC2:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_Z) == 0) cpu->pc = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// JP a
		case 0xC3:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				cpu->pc = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// CALL NZ,a
		case 0xC4:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_Z) == 0)
				{
					Z80_CALL(nn);
					cpu->sync.clock += Z80_CLOCKMUL*(17 - 10);
				}
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// PUSH BC
		case 0xC5:
			Z80_PUSH(cpu->c, cpu->b);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// ADD A,d
		case 0xC6:
			{
				u8 n = PROGBYTE();
				Z80_ADD(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// RST 00h
		case 0xC7:
			Z80_CALL(0x00);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// RET Z
		case 0xC8:
			cpu->sync.clock += Z80_CLOCKMUL*5;
			if ((cpu->f & Z80_Z) != 0)
			{
				Z80_RET();
				cpu->sync.clock += Z80_CLOCKMUL*(11 - 5);
			}
			break;

		// RET
		case 0xC9:
			Z80_RET();
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// JP Z,a
		case 0xCA:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_Z) != 0) cpu->pc = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// prefix CB
		case 0xCB:
			cpu->r++; // increment memory refresh address
#include "emu_z80_cb.c"
			break;

		// CALL Z,a
		case 0xCC:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_Z) != 0)
				{
					Z80_CALL(nn);
					cpu->sync.clock += Z80_CLOCKMUL*(17 - 10);
				}
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// CALL a
		case 0xCD:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				Z80_CALL(nn);
			}
			cpu->sync.clock += Z80_CLOCKMUL*17;
			break;

		// ADC A,d
		case 0xCE:
			{
				u8 n = PROGBYTE();
				Z80_ADC(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// RST 08h
		case 0xCF:
			Z80_CALL(0x08);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// RET NC
		case 0xD0:
			cpu->sync.clock += Z80_CLOCKMUL*5;
			if ((cpu->f & Z80_C) == 0)
			{
				Z80_RET();
				cpu->sync.clock += Z80_CLOCKMUL*(11 - 5);
			}
			break;

		// POP DE
		case 0xD1:
			Z80_POP(cpu->e, cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// JP NC,a
		case 0xD2:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_C) == 0) cpu->pc = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// OUT (d),A
		case 0xD3:
			{
				u8 n = PROGBYTE();
				u8 a = cpu->a;
				u16 nn = n + (a << 8);
				cpu->writeio(nn, a);
			}
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// CALL NC,a
		case 0xD4:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_C) == 0)
				{
					Z80_CALL(nn);
					cpu->sync.clock += Z80_CLOCKMUL*(17 - 10);
				}
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// PUSH DE
		case 0xD5:
			Z80_PUSH(cpu->e, cpu->d);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// SUB d
		case 0xD6:
			{
				u8 n = PROGBYTE();
				Z80_SUB(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// RST 10h
		case 0xD7:
			Z80_CALL(0x10);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// RET C
		case 0xD8:
			cpu->sync.clock += Z80_CLOCKMUL*5;
			if ((cpu->f & Z80_C) != 0)
			{
				Z80_RET();
				cpu->sync.clock += Z80_CLOCKMUL*(11 - 5);
			}
			break;

		// EXX
		case 0xD9:
			{
				u16 nn = cpu->bc;
				cpu->bc = cpu->bc2;
				cpu->bc2 = nn;

				u32 nnnn = cpu->dehl;
				cpu->dehl = cpu->dehl2;
				cpu->dehl2 = nnnn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// JP C,a
		case 0xDA:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_C) != 0) cpu->pc = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// IN A,(d)
		case 0xDB:
			{
				u8 n = PROGBYTE();
				u8 a = cpu->a;
				u16 nn = n + (a << 8);
				cpu->a = cpu->readio(nn);
			}
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// CALL C,a
		case 0xDC:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_C) != 0)
				{
					Z80_CALL(nn);
					cpu->sync.clock += Z80_CLOCKMUL*(17 - 10);
				}
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// prefix DD
		case 0xDD:
			cpu->r++; // increment memory refresh address
#define REG_IXY ix
#define REG_IXYL ixl
#define REG_IXYH ixh
#include "emu_z80_ddfd.c"
#undef REG_IXY
#undef REG_IXYL
#undef REG_IXYH
			break;

		// SBC A,d
		case 0xDE:
			{
				u8 n = PROGBYTE();
				Z80_SBC(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// RST 18h
		case 0xDF:
			Z80_CALL(0x18);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// RET PO
		case 0xE0:
			cpu->sync.clock += Z80_CLOCKMUL*5;
			if ((cpu->f & Z80_PV) == 0)
			{
				Z80_RET();
				cpu->sync.clock += Z80_CLOCKMUL*(11 - 5);
			}
			break;

		// POP HL
		case 0xE1:
			Z80_POP(cpu->l, cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// JP PO,a
		case 0xE2:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_PV) == 0) cpu->pc = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
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

		// CALL PO,a
		case 0xE4:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_PV) == 0)
				{
					Z80_CALL(nn);
					cpu->sync.clock += Z80_CLOCKMUL*(17 - 10);
				}
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// PUSH HL
		case 0xE5:
			Z80_PUSH(cpu->l, cpu->h);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// AND d
		case 0xE6:
			{
				u8 n = PROGBYTE();
				Z80_AND(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// RST 20h
		case 0xE7:
			Z80_CALL(0x20);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// RET PE
		case 0xE8:
			cpu->sync.clock += Z80_CLOCKMUL*5;
			if ((cpu->f & Z80_PV) != 0)
			{
				Z80_RET();
				cpu->sync.clock += Z80_CLOCKMUL*(11 - 5);
			}
			break;

		// JP (HL)
		case 0xE9:
			cpu->pc = cpu->hl;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// JP PE,a
		case 0xEA:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_PV) != 0) cpu->pc = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
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

		// CALL PE,a
		case 0xEC:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_PV) != 0)
				{
					Z80_CALL(nn);
					cpu->sync.clock += Z80_CLOCKMUL*(17 - 10);
				}
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// prefix ED
		case 0xED:
			cpu->r++; // increment memory refresh address
#include "emu_z80_ed.c"
			break;

		// XOR d
		case 0xEE:
			{
				u8 n = PROGBYTE();
				Z80_XOR(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// RST 28h
		case 0xEF:
			Z80_CALL(0x28);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// RET P
		case 0xF0:
			cpu->sync.clock += Z80_CLOCKMUL*5;
			if ((cpu->f & Z80_S) == 0)
			{
				Z80_RET();
				cpu->sync.clock += Z80_CLOCKMUL*(11 - 5);
			}
			break;

		// POP AF
		case 0xF1:
			Z80_POP(cpu->f, cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// JP P,a
		case 0xF2:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_S) == 0) cpu->pc = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// DI
		case 0xF3:
			cpu->iff1 = 0;
			cpu->iff2 = 0;
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// CALL P,a
		case 0xF4:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_S) == 0)
				{
					Z80_CALL(nn);
					cpu->sync.clock += Z80_CLOCKMUL*(17 - 10);
				}
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// PUSH AF
		case 0xF5:
			Z80_PUSH(cpu->f, cpu->a);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// OR d
		case 0xF6:
			{
				u8 n = PROGBYTE();
				Z80_OR(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// RST 30h
		case 0xF7:
			Z80_CALL(0x30);
			cpu->sync.clock += Z80_CLOCKMUL*11;
			break;

		// RET M
		case 0xF8:
			cpu->sync.clock += Z80_CLOCKMUL*5;
			if ((cpu->f & Z80_S) != 0)
			{
				Z80_RET();
				cpu->sync.clock += Z80_CLOCKMUL*(11 - 5);
			}
			break;

		// LD SP,HL
		case 0xF9:
			cpu->sp = cpu->hl;
			cpu->sync.clock += Z80_CLOCKMUL*6;
			break;

		// JP M,a
		case 0xFA:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_S) != 0) cpu->pc = nn;
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// EI
		case 0xFB:
			cpu->iff1 = 1;
			cpu->iff2 = 1;
			cpu->tid = 1; // temporary disable interrupt (to disable 1 next instruction)
			cpu->sync.clock += Z80_CLOCKMUL*4;
			break;

		// CALL M,a
		case 0xFC:
			{
				u16 nn;
				PROGWORD(nn);	// read address -> nn
				if ((cpu->f & Z80_S) != 0)
				{
					Z80_CALL(nn);
					cpu->sync.clock += Z80_CLOCKMUL*(17 - 10);
				}
			}
			cpu->sync.clock += Z80_CLOCKMUL*10;
			break;

		// prefix FD
		case 0xFD:
			cpu->r++; // increment memory refresh address
#define REG_IXY iy
#define REG_IXYL iyl
#define REG_IXYH iyh
#include "emu_z80_ddfd.c"
#undef REG_IXY
#undef REG_IXYL
#undef REG_IXYH
			break;

		// CP d
		case 0xFE:
			{
				u8 n = PROGBYTE();
				Z80_CP(n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*7;
			break;

		// RST 38h
		case 0xFF:
			Z80_CALL(0x38);
			cpu->sync.clock += Z80_CLOCKMUL*11;
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
