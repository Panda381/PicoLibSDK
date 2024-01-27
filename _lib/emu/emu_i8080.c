
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

#if USE_EMU_I8080		// use I8080 CPU emulator

#include "emu_i8080_macro.h"

// current CPU descriptor (NULL = not running)
volatile sI8080* I8080_Cpu = NULL;

// flags table with parity (I8080_P, I8080_Z and I8080_S flags) ... keep the table in RAM, for higher speed
u8 I8080_FlagParTab[256];

// initialize I8080 tables
void I8080_InitTab()
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
		f = I8080_FLAGDEF;
		if (par == 0) f |= I8080_P; // even parity
		if (i >= 0x80) f |= I8080_S; // negative
		if (i == 0) f |= I8080_Z; // zero
		I8080_FlagParTab[i] = f;
	}
}

// reset processor
void I8080_Reset(sI8080* cpu)
{
	cpu->pc = 0;		// program counter
	cpu->sp = 0xffff;	// stack pointer
	cpu->a = 0;
	cpu->f = I8080_FLAGDEF;
	cpu->hl = 0;
	cpu->de = 0;
	cpu->bc = 0;
	cpu->ie = 0;		// interrupt enable
	cpu->tid = 0;		// temporary interrupt disable
	cpu->halted = 0;	// halted
	cpu->stop = 0;		// stop request
	cpu->intreq = 0;	// interrupt request
	cpu->intins = 0x0D;	// interrupt instruction RST 0x08
	cpu->cond[0] = I8080_Z;	// condition table
	cpu->cond[1] = I8080_C;	// condition table
	cpu->cond[2] = I8080_P;	// condition table
	cpu->cond[3] = I8080_S;	// condition table
}

// load byte from program memory
INLINE u8 I8080_ProgByte(sI8080* cpu)
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
INLINE u16 I8080_ProgWord(sI8080* cpu)
{
	u8 temp = I8080_ProgByte(cpu);
	return (temp | ((u16)I8080_ProgByte(cpu) << 8));
}

// execute program (start or continue, until "stop" request)
// Size of code of this function: 2412 code + 1024 jump table = 3436 bytes
// CPU loading at 2 MHz on 120 MHz: used 17-23%, max. 20-27%
// CPU loading at 5 MHz on 120 MHz: used 46-63%, max. 52-70%
void FASTCODE NOFLASH(I8080_Exec)(sI8080* cpu)
{
	u8 op;

	// clear stop flag
	cpu->stop = 0;

	// program loop
	while (cpu->stop == 0)
	{
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
			op = I8080_ProgByte(cpu);
		}

		// clear temporary disable flag
		cpu->tid = 0;

		// switch base operation code
		switch (op)
		{
		// NOP
		case 0x00:
		case 0x08: // NOP ... undocumented on 8080
		case 0x10: // NOP ... undocumented on 8080
		case 0x18: // NOP ... undocumented on 8080
		case 0x20: // NOP ... undocumented on 8080
		case 0x28: // NOP ... undocumented on 8080
		case 0x30: // NOP ... undocumented on 8080
		case 0x38: // NOP ... undocumented on 8080
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// LXI B,nn ... LD BC,nn
		case 0x01:
		// LXI D,nn ... LD DE,nn
		case 0x11:
		// LXI H,nn ... LD HL,nn
		case 0x21:
			{
				u8* r = &cpu->reg[6 - (op >> 3)]; // register low
				r[0] = I8080_ProgByte(cpu); // low
				r[1] = I8080_ProgByte(cpu); // high
			}
			cpu->sync.clock += I8080_CLOCKMUL*10;
			break;

		// STAX B ... LD (BC),A
		case 0x02:
			cpu->writemem(cpu->bc, cpu->a);
			cpu->sync.clock += I8080_CLOCKMUL*7;
			break;

		// INX B ... INC BC
		case 0x03:
			cpu->bc++;
			cpu->sync.clock += I8080_CLOCKMUL*5;
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
				I8080_INC(*r);
			}
			cpu->sync.clock += I8080_CLOCKMUL*5;
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
				I8080_DEC(*r);
			}
			cpu->sync.clock += I8080_CLOCKMUL*5;
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
				*r = I8080_ProgByte(cpu);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
			break;

		// RLC ... RLCA
		case 0x07:
			{
				u8 n = cpu->a;
				n = (n << 1) | (n >> 7);
				cpu->a = n;
				u8 f = cpu->f & ~I8080_C;
				f |= n & I8080_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// NOP ... undocumented on 8080
		//case 0x08:

		// DAD B ... ADD HL,BC
		case 0x09:
		// DAD D ... ADD HL,DE
		case 0x19:
		// DAD H ... ADD HL,HL
		case 0x29:
			{
				u16* r = &cpu->dreg[3 - (op >> 4)]; // double register
				I8080_ADD16(*r);
			}
			cpu->sync.clock += I8080_CLOCKMUL*10;
			break;

		// LDAX B ... LD A,(BC)
		case 0x0A:
			cpu->a = cpu->readmem(cpu->bc);
			cpu->sync.clock += I8080_CLOCKMUL*7;
			break;

		// DCX B ... DEC BC
		case 0x0B:
			cpu->bc--;
			cpu->sync.clock += I8080_CLOCKMUL*5;
			break;

		// RRC ... RRCA
		case 0x0F:
			{
				u8 n = cpu->a;
				u8 n2 = (n >> 1) | (n << 7);
				cpu->a = n2;
				u8 f = cpu->f & ~I8080_C;
				f |= n & I8080_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// NOP ... undocumented on 8080
		//case 0x10:

		// STAX D ... LD (DE),A
		case 0x12:
			cpu->writemem(cpu->de, cpu->a);
			cpu->sync.clock += I8080_CLOCKMUL*7;
			break;

		// INX D ... INC DE
		case 0x13:
			cpu->de++;
			cpu->sync.clock += I8080_CLOCKMUL*5;
			break;

		// RAL ... RLA
		case 0x17:
			{
				u8 n2 = cpu->a;
				u8 n = (n2 << 1) | (cpu->f & I8080_C);
				cpu->a = n;
				u8 f = cpu->f & ~I8080_C;
				f |= n2 >> 7; // I8080_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// NOP ... undocumented on 8080
		//case 0x18:

		// LDAX D ... LD A,(DE)
		case 0x1A:
			cpu->a = cpu->readmem(cpu->de);
			cpu->sync.clock += I8080_CLOCKMUL*7;
			break;

		// DCX D ... DEC DE
		case 0x1B:
			cpu->de--;
			cpu->sync.clock += I8080_CLOCKMUL*5;
			break;

		// RAR ... RRA
		case 0x1F:
			{
				u8 n2 = cpu->a;
				u8 n = (n2 >> 1) | (cpu->f << 7);
				cpu->a = n;
				u8 f = cpu->f & ~I8080_C;
				f |= n2 & I8080_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// NOP ... undocumented on 8080
		//case 0x20:

		// SHLD a ... LD (a),HL
		case 0x22:
			{
				u16 nn = I8080_ProgWord(cpu);
				cpu->writemem(nn, cpu->l);
				nn++;
				cpu->writemem(nn, cpu->h);
			}
			cpu->sync.clock += I8080_CLOCKMUL*16;
			break;

		// INX H ... INC HL
		case 0x23:
			cpu->hl++;
			cpu->sync.clock += I8080_CLOCKMUL*5;
			break;

		// DAA
		case 0x27:
			{
				u8 f = cpu->f;
				u8 a = cpu->a;
				u8 f2 = I8080_FLAGDEF;

				if (((a & 0x0f) > 9) || ((f & I8080_AC) != 0))
				{
					if ((a & 0x0f) > 9) f2 |= I8080_AC;
					if (a > 0xf9) f |= I8080_C;
					a += 6;
				}

				if ((a > 0x9f) || ((f & I8080_C) != 0))
				{
					a += 0x60;
					f2 |= I8080_C;
				}

				cpu->a = a;
				f2 |= I8080_FlagParTab[a];
				cpu->f = f2;
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// NOP ... undocumented on 8080
		//case 0x28:

		// LHLD a ... LD HL,(a)
		case 0x2A:
			{
				u16 nn = I8080_ProgWord(cpu);
				cpu->l = cpu->readmem(nn);
				nn++;
				cpu->h = cpu->readmem(nn);
			}
			cpu->sync.clock += I8080_CLOCKMUL*16;
			break;

		// DCX H ... DEC HL
		case 0x2B:
			cpu->hl--;
			cpu->sync.clock += I8080_CLOCKMUL*5;
			break;

		// CMA ... CPL
		case 0x2F:
			cpu->a = ~cpu->a;
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// NOP ... undocumented on 8080
		//case 0x30:

		// LXI SP,nn ... LD SP,nn
		case 0x31:
			cpu->spl = I8080_ProgByte(cpu);
			cpu->sph = I8080_ProgByte(cpu);
			cpu->sync.clock += I8080_CLOCKMUL*10;
			break;

		// STA a ... LD (a),A
		case 0x32:
			{
				u16 nn = I8080_ProgWord(cpu);
				cpu->writemem(nn, cpu->a);
			}
			cpu->sync.clock += I8080_CLOCKMUL*13;
			break;

		// INX SP ... INC SP
		case 0x33:
			cpu->sp++;
			cpu->sync.clock += I8080_CLOCKMUL*5;
			break;

		// INR M ... INC (HL)
		case 0x34:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				I8080_INC(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*10;
			break;

		// DCR M ... DEC (HL)
		case 0x35:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				I8080_DEC(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*10;
			break;

		// MVI M,n ... LD (HL),n
		case 0x36:
			cpu->writemem(cpu->hl, I8080_ProgByte(cpu));
			cpu->sync.clock += I8080_CLOCKMUL*10;
			break;

		// STC ... SCF
		case 0x37:
			cpu->f |= I8080_C;
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// NOP ... undocumented on 8080
		//case 0x38:

		// DAD SP ... ADD HL,SP
		case 0x39:
			I8080_ADD16(cpu->sp);
			cpu->sync.clock += I8080_CLOCKMUL*10;
			break;

		// LDA a ... LD A,(a)
		case 0x3A:
			{
				u16 nn = I8080_ProgWord(cpu);
				cpu->a = cpu->readmem(nn);
			}
			cpu->sync.clock += I8080_CLOCKMUL*13;
			break;

		// DCX SP ... DEC SP
		case 0x3B:
			cpu->sp--;
			cpu->sync.clock += I8080_CLOCKMUL*5;
			break;

		// CMC ... CCF
		case 0x3F:
			cpu->f ^= I8080_C;
			cpu->sync.clock += I8080_CLOCKMUL*4;
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
			cpu->sync.clock += I8080_CLOCKMUL*5;
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
			cpu->sync.clock += I8080_CLOCKMUL*5;
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
			cpu->sync.clock += I8080_CLOCKMUL*7;
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
			cpu->sync.clock += I8080_CLOCKMUL*7;
			break;

		// HLT ... HALT
		case 0x76:
			cpu->pc--;
			cpu->halted = 1;
			cpu->sync.clock += I8080_CLOCKMUL*7;
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
				I8080_ADD(*rs);
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// ADD M ... ADD A,(HL)
		case 0x86:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8080_ADD(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
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
				I8080_ADC(*rs);
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// ADC M ... ADC A,(HL)
		case 0x8E:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8080_ADC(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
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
				I8080_SUB(*rs);
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// SUB M ... SUB (HL)
		case 0x96:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8080_SUB(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
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
				I8080_SBC(*rs);
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// SBB M ... SBC A,(HL)
		case 0x9E:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8080_SBC(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
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
				I8080_AND(*rs);
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// ANA M ... AND (HL)
		case 0xA6:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8080_AND(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
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
				I8080_XOR(*rs);
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// XRA M ... XOR (HL)
		case 0xAE:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8080_XOR(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
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
				I8080_OR(*rs);
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// ORA M ... OR (HL)
		case 0xB6:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8080_OR(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
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
				I8080_CP(*rs);
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// CMP M ... CP (HL)
		case 0xBE:
			{
				u8 n = cpu->readmem(cpu->hl);
				I8080_CP(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
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
					I8080_RET();
					cpu->sync.clock += I8080_CLOCKMUL*(11 - 5);
				}
			}
			cpu->sync.clock += I8080_CLOCKMUL*5;
			break;

		// POP B ... POP BC
		case 0xC1:
		// POP D ... POP DE
		case 0xD1:
		// POP H ... POP HL
		case 0xE1:
			{
				u8* r = &cpu->reg[6 - ((op - 0xc1) >> 3)]; // register low
				I8080_POP(r[0], r[1]);
			}
			cpu->sync.clock += I8080_CLOCKMUL*10;
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
				u16 nn = I8080_ProgWord(cpu);

				// flag bit
				int fb = cpu->cond[(op - 0xc2) >> 4];

				// invert flags
				u8 f = cpu->f;
				if ((op & B3) != 0) f = ~f;

				// check condition
				if ((f & fb) == 0) cpu->pc = nn;
			}
			cpu->sync.clock += I8080_CLOCKMUL*10;
			break;

		// JMP a ... JP a
		case 0xC3:
		case 0xCB: // JMP ... undocumented on 8080
			cpu->pc = I8080_ProgWord(cpu);
			cpu->sync.clock += I8080_CLOCKMUL*10;
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
				u16 nn = I8080_ProgWord(cpu);

				// flag bit
				int fb = cpu->cond[(op - 0xc4) >> 4];

				// invert flags
				u8 f = cpu->f;
				if ((op & B3) != 0) f = ~f;

				// check condition
				if ((f & fb) == 0)
				{
					I8080_CALL(nn);
					cpu->sync.clock += I8080_CLOCKMUL*(17 - 11);
				}
			}
			cpu->sync.clock += I8080_CLOCKMUL*11;
			break;

		// PUSH B ... PUSH BC
		case 0xC5:
		// PUSH D ... PUSH DE
		case 0xD5:
		// PUSH H ... PUSH HL
		case 0xE5:
			{
				u8* r = &cpu->reg[6 - ((op - 0xc5) >> 3)]; // register low
				I8080_PUSH(r[0], r[1]);
			}
			cpu->sync.clock += I8080_CLOCKMUL*11;
			break;

		// ADI d ... ADD A,d
		case 0xC6:
			{
				u8 n = I8080_ProgByte(cpu);
				I8080_ADD(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
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
			I8080_CALL(op & 0x38);
			cpu->sync.clock += I8080_CLOCKMUL*11;
			break;

		// RET
		case 0xC9:
		case 0xD9: // RET ... undocumented on 8080
			I8080_RET();
			cpu->sync.clock += I8080_CLOCKMUL*10;
			break;

		// JMP ... undocumented on 8080
		//case 0xCB:

		// CALL a
		case 0xCD:
		case 0xDD: // CALL a ... undocumented on 8080
		case 0xED: // CALL a ... undocumented on 8080
		case 0xFD: // CALL a ... undocumented on 8080
			{
				u16 nn = I8080_ProgWord(cpu);
				I8080_CALL(nn);
			}
			cpu->sync.clock += I8080_CLOCKMUL*17;
			break;

		// ACI d ... ADC A,d
		case 0xCE:
			{
				u8 n = I8080_ProgByte(cpu);
				I8080_ADC(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
			break;

		// OUT d ... OUT (d),A
		case 0xD3:
			cpu->writeport(I8080_ProgByte(cpu), cpu->a);
			cpu->sync.clock += I8080_CLOCKMUL*10;
			break;

		// SUI d ... SUB d
		case 0xD6:
			{
				u8 n = I8080_ProgByte(cpu);
				I8080_SUB(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
			break;

		// RET ... undocumented on 8080
		//case 0xD9:

		// IN d ... IN A,(d)
		case 0xDB:
			cpu->a = cpu->readport(I8080_ProgByte(cpu));
			cpu->sync.clock += I8080_CLOCKMUL*10;
			break;

		// CALL a ... undocumented on 8080
		//case 0xDD:

		// SBI d ... SBC A,d
		case 0xDE:
			{
				u8 n = I8080_ProgByte(cpu);
				I8080_SBC(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
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
			cpu->sync.clock += I8080_CLOCKMUL*18;
			break;

		// ANI d ... AND d
		case 0xE6:
			{
				u8 n = I8080_ProgByte(cpu);
				I8080_AND(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
			break;

		// PCHL ... JP (HL)
		case 0xE9:
			cpu->pc = cpu->hl;
			cpu->sync.clock += I8080_CLOCKMUL*5;
			break;

		// XCHG ... EX DE.HL
		case 0xEB:
			{
				u16 nn = cpu->hl;
				cpu->hl = cpu->de;
				cpu->de = nn;
			}
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// CALL a ... undocumented on 8080
		//case 0xED:

		// XRI d ... XOR d
		case 0xEE:
			{
				u8 n = I8080_ProgByte(cpu);
				I8080_XOR(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
			break;

		// POP PSW ... POP AF
		case 0xF1:
			I8080_POP(cpu->f, cpu->a);
			cpu->f = (cpu->f & I8080_FLAGALL) | I8080_FLAGDEF;
			cpu->sync.clock += I8080_CLOCKMUL*10;
			break;

		// DI
		case 0xF3:
			cpu->ie = 0;
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// PUSH PSW ... PUSH AF
		case 0xF5:
			cpu->f = (cpu->f & I8080_FLAGALL) | I8080_FLAGDEF;
			I8080_PUSH(cpu->f, cpu->a);
			cpu->sync.clock += I8080_CLOCKMUL*11;
			break;

		// ORI d ... OR d
		case 0xF6:
			{
				u8 n = I8080_ProgByte(cpu);
				I8080_OR(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
			break;

		// SPHL ... LD SP,HL
		case 0xF9:
			cpu->sp = cpu->hl;
			cpu->sync.clock += I8080_CLOCKMUL*5;
			break;

		// EI
		case 0xFB:
			cpu->ie = 1; // enable interrupt
			cpu->tid = 1; // temporary disable interrupt (for next 1 instruction)
			cpu->sync.clock += I8080_CLOCKMUL*4;
			break;

		// CALL a ... undocumented on 8080
		//case 0xFD:

		// CPI d ... CP d
		case 0xFE:
			{
				u8 n = I8080_ProgByte(cpu);
				I8080_CP(n);
			}
			cpu->sync.clock += I8080_CLOCKMUL*7;
			break;
		}

		// time synchronization
		EmuSync(&cpu->sync);
	}
}

// execute I8080 emulator on CPU 1
static void I8080_ExecCore1()
{
	// start time synchronization
	I8080_SyncStart((sI8080*)I8080_Cpu);

	// execute
	I8080_Exec((sI8080*)I8080_Cpu);

	// clear descriptor
	I8080_Cpu = NULL;
	dsb();
}

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8080_CLOCKMUL is recommended to maintain.
u32 I8080_Start(sI8080* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I8080_Cpu != NULL) I8080_Stop(pwm);

	// initialize time synchronization
	freq = I8080_SyncInit(cpu, pwm, freq);

	// reset processor
	I8080_Reset(cpu);

	// current CPU descriptor
	I8080_Cpu = cpu;
	dsb();

	// execute program
#if USE_MINIVGA				// use mini-VGA display
	VgaCore1Exec(I8080_ExecCore1);
#else
	Core1Exec(I8080_ExecCore1);
#endif

	return freq;
}

// continue emulation without reset processor
u32 I8080_Cont(sI8080* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I8080_Cpu != NULL) I8080_Stop(pwm);

	// initialize time synchronization
	freq = I8080_SyncInit(cpu, pwm, freq);

	// current CPU descriptor
	I8080_Cpu = cpu;
	dsb();

	// execute program
#if USE_MINIVGA				// use mini-VGA display
	VgaCore1Exec(I8080_ExecCore1);
#else
	Core1Exec(I8080_ExecCore1);
#endif

	return freq;
}

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I8080_Stop(int pwm)
{
	// get CPU descriptor
	sI8080* cpu = (sI8080*)I8080_Cpu;

	// check if emulation is running
	if (cpu == NULL) return;

	// request to stop emulation
	cpu->stop = 1;

	// wait to complete
	while (I8080_Cpu != NULL) { dsb(); }

	// terminate time synchronization (stop PWM counter)
	I8080_SyncTerm(pwm);

#if !USE_MINIVGA				// use mini-VGA display
	// core 1 reset
	Core1Reset();
#endif
}

#endif // USE_EMU_I8080
