
// ****************************************************************************
//
//                        Z80 CPU Emulator, CB instructions
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

// #define REG_IXY ix
// #define REG_IXYL ixl
// #define REG_IXYH ixh

	// switch 0xDD (=IX) or 0xFD (=IY) operation code
	switch (PROGBYTE())
	{
	// ADD IXY,BC
	case 0x09:
		Z80_ADD16(REG_IXY, bc);
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// ADD IXY,DE
	case 0x19:
		Z80_ADD16(REG_IXY, de);
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// LD IXY,nn
	case 0x21:
		cpu->REG_IXYL = PROGBYTE();
		cpu->REG_IXYH = PROGBYTE();
		cpu->sync.clock += Z80_CLOCKMUL*14;
		break;

	// LD (a),IXY
	case 0x22:
		{
			u16 nn;
			PROGWORD(nn);	// read address -> nn
			cpu->writemem(nn, cpu->REG_IXYL);
			nn++;
			cpu->writemem(nn, cpu->REG_IXYH);
		}
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// INC IXY
	case 0x23:
		cpu->REG_IXY++;
		cpu->sync.clock += Z80_CLOCKMUL*10;
		break;

	// INC IXYH
	case 0x24:
		Z80_INC(cpu->REG_IXYH);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// DEC IXYH
	case 0x25:
		Z80_DEC(cpu->REG_IXYH);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYH,n
	case 0x26:
		cpu->REG_IXYH = PROGBYTE();
		cpu->sync.clock += Z80_CLOCKMUL*11;
		break;

	// ADD IXY,IXY
	case 0x29:
		Z80_ADD16(REG_IXY, REG_IXY);
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// LD IXY,(a)
	case 0x2A:
		{
			u16 nn;
			PROGWORD(nn);	// read address -> nn
			cpu->REG_IXYL = cpu->readmem(nn);
			nn++;
			cpu->REG_IXYH = cpu->readmem(nn);
		}
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// DEC IXY
	case 0x2B:
		cpu->REG_IXY--;
		cpu->sync.clock += Z80_CLOCKMUL*10;
		break;

	// INC IXYL
	case 0x2C:
		Z80_INC(cpu->REG_IXYL);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// DEC IXYL
	case 0x2D:
		Z80_DEC(cpu->REG_IXYL);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYL,n
	case 0x2E:
		cpu->REG_IXYL = PROGBYTE();
		cpu->sync.clock += Z80_CLOCKMUL*11;
		break;

	// INC (IXY+d)
	case 0x34:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			u8 n = cpu->readmem(nn);
			Z80_INC(n);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// DEC (IXY+d)
	case 0x35:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			u8 n = cpu->readmem(nn);
			Z80_DEC(n);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// LD (IXY+d),n
	case 0x36:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->writemem(nn, PROGBYTE());
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// ADD IXY,SP
	case 0x39:
		Z80_ADD16(REG_IXY, sp);
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// LD B,IXYH
	case 0x44:
		cpu->b = cpu->REG_IXYH;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD B,IXYL
	case 0x45:
		cpu->b = cpu->REG_IXYL;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD B,(IXY+d)
	case 0x46:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->b = cpu->readmem(nn);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD C,IXYH
	case 0x4C:
		cpu->c = cpu->REG_IXYH;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD C,IXYL
	case 0x4D:
		cpu->c = cpu->REG_IXYL;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD C,(IXY+d)
	case 0x4E:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->c = cpu->readmem(nn);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD D,IXYH
	case 0x54:
		cpu->d = cpu->REG_IXYH;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD D,IXYL
	case 0x55:
		cpu->d = cpu->REG_IXYL;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD D,(IXY+d)
	case 0x56:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->d = cpu->readmem(nn);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD E,IXYH
	case 0x5C:
		cpu->e = cpu->REG_IXYH;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD E,IXYL
	case 0x5D:
		cpu->e = cpu->REG_IXYL;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD E,(IXY+d)
	case 0x5E:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->e = cpu->readmem(nn);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD IXYH,B
	case 0x60:
		cpu->REG_IXYH = cpu->b;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYH,C
	case 0x61:
		cpu->REG_IXYH = cpu->c;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYH,D
	case 0x62:
		cpu->REG_IXYH = cpu->d;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYH,E
	case 0x63:
		cpu->REG_IXYH = cpu->e;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYH,IXYH
	case 0x64:
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYH,IXYL
	case 0x65:
		cpu->REG_IXYH = cpu->REG_IXYL;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD H,(IXY+d)
	case 0x66:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->h = cpu->readmem(nn);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD IXYH,A
	case 0x67:
		cpu->REG_IXYH = cpu->a;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYL,B
	case 0x68:
		cpu->REG_IXYL = cpu->b;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYL,C
	case 0x69:
		cpu->REG_IXYL = cpu->c;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYL,D
	case 0x6A:
		cpu->REG_IXYL = cpu->d;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYL,E
	case 0x6B:
		cpu->REG_IXYL = cpu->e;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYL,IXYH
	case 0x6C:
		cpu->REG_IXYL = cpu->REG_IXYH;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYL,IXYL
	case 0x6D:
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD L,(IXY+d)
	case 0x6E:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->l = cpu->readmem(nn);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD IXYL,A
	case 0x6F:
		cpu->REG_IXYL = cpu->a;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD (IXY+d),B
	case 0x70:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->writemem(nn, cpu->b);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD (IXY+d),C
	case 0x71:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->writemem(nn, cpu->c);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD (IXY+d),D
	case 0x72:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->writemem(nn, cpu->d);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD (IXY+d),E
	case 0x73:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->writemem(nn, cpu->e);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD (IXY+d),H
	case 0x74:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->writemem(nn, cpu->h);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD (IXY+d),L
	case 0x75:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->writemem(nn, cpu->l);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD (IXY),A
	case 0x77:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->writemem(nn, cpu->a);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD A,IXYH
	case 0x7C:
		cpu->a = cpu->REG_IXYH;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD A,IXYL
	case 0x7D:
		cpu->a = cpu->REG_IXYL;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD A,(IXY+d)
	case 0x7E:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			cpu->a = cpu->readmem(nn);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// ADD A,IXYH
	case 0x84:
		Z80_ADD(cpu->REG_IXYH);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// ADD A,IXYL
	case 0x85:
		Z80_ADD(cpu->REG_IXYL);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// ADD A,(IXY+d)
	case 0x86:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			u8 n = cpu->readmem(nn);
			Z80_ADD(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// ADC A,IXYH
	case 0x8C:
		Z80_ADC(cpu->REG_IXYH);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// ADC A,IXYL
	case 0x8D:
		Z80_ADC(cpu->REG_IXYL);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// ADC A,(IXY+d)
	case 0x8E:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			u8 n = cpu->readmem(nn);
			Z80_ADC(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// SUB IXYH
	case 0x94:
		Z80_SUB(cpu->REG_IXYH);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SUB IXYL
	case 0x95:
		Z80_SUB(cpu->REG_IXYL);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SUB (IXY+d)
	case 0x96:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			u8 n = cpu->readmem(nn);
			Z80_SUB(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// SBC A,IXYH
	case 0x9C:
		Z80_SBC(cpu->REG_IXYH);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SBC A,IXYL
	case 0x9D:
		Z80_SBC(cpu->REG_IXYL);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SBC A,(IXY+d)
	case 0x9E:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			u8 n = cpu->readmem(nn);
			Z80_SBC(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// AND IXYH
	case 0xA4:
		Z80_AND(cpu->REG_IXYH);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// AND IXYL
	case 0xA5:
		Z80_AND(cpu->REG_IXYL);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// AND (IXY+d)
	case 0xA6:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			u8 n = cpu->readmem(nn);
			Z80_AND(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// XOR IXYH
	case 0xAC:
		Z80_XOR(cpu->REG_IXYH);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// XOR IXYL
	case 0xAD:
		Z80_XOR(cpu->REG_IXYL);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// XOR (IXY+d)
	case 0xAE:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			u8 n = cpu->readmem(nn);
			Z80_XOR(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// OR IXYH
	case 0xB4:
		Z80_OR(cpu->REG_IXYH);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// OR IXYL
	case 0xB5:
		Z80_OR(cpu->REG_IXYL);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// OR (IXY+d)
	case 0xB6:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			u8 n = cpu->readmem(nn);
			Z80_OR(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// CP IXYH
	case 0xBC:
		Z80_CP(cpu->REG_IXYH);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// CP IXYL
	case 0xBD:
		Z80_CP(cpu->REG_IXYL);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// CP (IXY+d)
	case 0xBE:
		{
			u16 nn = cpu->REG_IXY + (s8)PROGBYTE();
			u8 n = cpu->readmem(nn);
			Z80_CP(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// prefix DDFDCB
	case 0xCB:
		{
			cpu->r++;
			u16 tempaddr = cpu->REG_IXY + (s8)PROGBYTE();
			u8 n = cpu->readmem(tempaddr);
#include "emu_z80_ddfdcb.c"
		}
		break;

	// POP IXY
	case 0xE1:
		Z80_POP(cpu->REG_IXYL, cpu->REG_IXYH);
		cpu->sync.clock += Z80_CLOCKMUL*14;
		break;

	// EX (SP),IXY
	case 0xE3:
		{
			u16 sp = cpu->sp;
			u8 n = cpu->readmem(sp);
			u8 n2 = cpu->readmem(sp+1);
			cpu->writemem(sp, cpu->REG_IXYL);
			cpu->writemem(sp+1, cpu->REG_IXYH);
			cpu->REG_IXYL = n;
			cpu->REG_IXYH = n2;
		}
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// PUSH IXY
	case 0xE5:
		Z80_PUSH(cpu->REG_IXYL, cpu->REG_IXYH);
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// JP (IXY)
	case 0xE9:
		cpu->pc = cpu->REG_IXY;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD SP,IXY
	case 0xF9:
		cpu->sp = cpu->REG_IXY;
		cpu->sync.clock += Z80_CLOCKMUL*10;
		break;

	// invalid opcode, roll back
	default:
		cpu->pc--;
		cpu->r--;
		break;
	}
