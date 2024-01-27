
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

// r ... pointer to ixl or iyl
void FASTCODE NOFLASH(Z80_ExecDDFD)(sZ80* cpu, u8* r)
{
	u8 op = Z80_ProgByte(cpu);

	// switch 0xDD (=IX) or 0xFD (=IY) operation code
	switch (op)
	{
	// ADD IXY,BC
	case 0x09:
	// ADD IXY,DE
	case 0x19:
		{
			u16* dr = &cpu->dreg[3 - (op >> 4)]; // double register
			Z80_ADD16(*(u16*)r, *dr);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// LD IXY,nn
	case 0x21:
		r[0] = Z80_ProgByte(cpu);
		r[1] = Z80_ProgByte(cpu);
		cpu->sync.clock += Z80_CLOCKMUL*14;
		break;

	// LD (a),IXY
	case 0x22:
		{
			u16 nn = Z80_ProgWord(cpu);
			cpu->writemem(nn, r[0]);
			nn++;
			cpu->writemem(nn, r[1]);
		}
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// INC IXY
	case 0x23:
		*(u16*)r = *(u16*)r + 1;
		cpu->sync.clock += Z80_CLOCKMUL*10;
		break;

	// INC IXYH
	case 0x24: r++;
	// INC IXYL
	case 0x2C:
		Z80_INC(r[0]);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// DEC IXYH
	case 0x25: r++;
	// DEC IXYL
	case 0x2D:
		Z80_DEC(r[0]);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYH,n
	case 0x26: r++;
	// LD IXYL,n
	case 0x2E:
		r[0] = Z80_ProgByte(cpu);
		cpu->sync.clock += Z80_CLOCKMUL*11;
		break;

	// ADD IXY,IXY
	case 0x29:
		Z80_ADD16(*(u16*)r, *(u16*)r);
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// LD IXY,(a)
	case 0x2A:
		{
			u16 nn = Z80_ProgWord(cpu);
			r[0] = cpu->readmem(nn);
			nn++;
			r[1] = cpu->readmem(nn);
		}
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// DEC IXY
	case 0x2B:
		*(u16*)r = *(u16*)r - 1;
		cpu->sync.clock += Z80_CLOCKMUL*10;
		break;

	// INC (IXY+d)
	case 0x34:
		{
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			u8 n = cpu->readmem(nn);
			Z80_INC(n);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// DEC (IXY+d)
	case 0x35:
		{
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			u8 n = cpu->readmem(nn);
			Z80_DEC(n);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// LD (IXY+d),n
	case 0x36:
		{
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			cpu->writemem(nn, Z80_ProgByte(cpu));
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// ADD IXY,SP
	case 0x39:
		Z80_ADD16(*(u16*)r, cpu->sp);
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// LD B,IXYH
	case 0x44:
	// LD C,IXYH
	case 0x4C:
	// LD D,IXYH
	case 0x54:
	// LD E,IXYH
	case 0x5C:
	// LD A,IXYH
	case 0x7C:
		{
			u8* rd = &cpu->reg[7 - ((op - 0x40) >> 3)]; // destination register
			*rd = r[1];
		}
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD B,IXYL
	case 0x45:
	// LD C,IXYL
	case 0x4D:
	// LD D,IXYL
	case 0x55:
	// LD E,IXYL
	case 0x5D:
	// LD A,IXYL
	case 0x7D:
		{
			u8* rd = &cpu->reg[7 - ((op - 0x40) >> 3)]; // destination register
			*rd = r[0];
		}
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD B,(IXY+d)
	case 0x46:
	// LD C,(IXY+d)
	case 0x4E:
	// LD D,(IXY+d)
	case 0x56:
	// LD E,(IXY+d)
	case 0x5E:
	// LD H,(IXY+d)
	case 0x66:
	// LD L,(IXY+d)
	case 0x6E:
	// LD A,(IXY+d)
	case 0x7E:
		{
			u8* rd = &cpu->reg[7 - ((op - 0x40) >> 3)]; // destination register
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			*rd = cpu->readmem(nn);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// LD IXYH,B
	case 0x60:
	// LD IXYH,C
	case 0x61:
	// LD IXYH,D
	case 0x62:
	// LD IXYH,E
	case 0x63:
	// LD IXYH,A
	case 0x67:
		{
			u8* rs = &cpu->reg[7 - (op - 0x60)]; // source register
			r[1] = *rs;
		}
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYH,IXYL
	case 0x65: r[1] = r[0];
	// LD IXYH,IXYH
	case 0x64:
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYL,B
	case 0x68:
	// LD IXYL,C
	case 0x69:
	// LD IXYL,D
	case 0x6A:
	// LD IXYL,E
	case 0x6B:
	// LD IXYL,A
	case 0x6F:
		{
			u8* rs = &cpu->reg[7 - (op - 0x60)]; // source register
			r[0] = *rs;
		}
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD IXYL,IXYH
	case 0x6C: r[0] = r[1];
	// LD IXYL,IXYL
	case 0x6D:
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD (IXY+d),B
	case 0x70:
	// LD (IXY+d),C
	case 0x71:
	// LD (IXY+d),D
	case 0x72:
	// LD (IXY+d),E
	case 0x73:
	// LD (IXY+d),H
	case 0x74:
	// LD (IXY+d),L
	case 0x75:
	// LD (IXY),A
	case 0x77:
		{
			u8* rs = &cpu->reg[7 - (op - 0x70)]; // source register
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			cpu->writemem(nn, *rs);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// ADD A,IXYH
	case 0x84: r++;
	// ADD A,IXYL
	case 0x85:
		Z80_ADD(r[0]);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// ADD A,(IXY+d)
	case 0x86:
		{
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			u8 n = cpu->readmem(nn);
			Z80_ADD(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// ADC A,IXYH
	case 0x8C: r++;
	// ADC A,IXYL
	case 0x8D:
		Z80_ADC(r[0]);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// ADC A,(IXY+d)
	case 0x8E:
		{
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			u8 n = cpu->readmem(nn);
			Z80_ADC(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// SUB IXYH
	case 0x94: r++;
	// SUB IXYL
	case 0x95:
		Z80_SUB(r[0]);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SUB (IXY+d)
	case 0x96:
		{
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			u8 n = cpu->readmem(nn);
			Z80_SUB(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// SBC A,IXYH
	case 0x9C: r++;
	// SBC A,IXYL
	case 0x9D:
		Z80_SBC(r[0]);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SBC A,(IXY+d)
	case 0x9E:
		{
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			u8 n = cpu->readmem(nn);
			Z80_SBC(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// AND IXYH
	case 0xA4: r++;
	// AND IXYL
	case 0xA5:
		Z80_AND(r[0]);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// AND (IXY+d)
	case 0xA6:
		{
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			u8 n = cpu->readmem(nn);
			Z80_AND(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// XOR IXYH
	case 0xAC: r++;
	// XOR IXYL
	case 0xAD:
		Z80_XOR(r[0]);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// XOR (IXY+d)
	case 0xAE:
		{
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			u8 n = cpu->readmem(nn);
			Z80_XOR(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// OR IXYH
	case 0xB4: r++;
	// OR IXYL
	case 0xB5:
		Z80_OR(r[0]);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// OR (IXY+d)
	case 0xB6:
		{
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			u8 n = cpu->readmem(nn);
			Z80_OR(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// CP IXYH
	case 0xBC: r++;
	// CP IXYL
	case 0xBD:
		Z80_CP(r[0]);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// CP (IXY+d)
	case 0xBE:
		{
			u16 nn = *(u16*)r + (s8)Z80_ProgByte(cpu);
			u8 n = cpu->readmem(nn);
			Z80_CP(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*19;
		break;

	// prefix DDFDCB
	case 0xCB:
		{
			cpu->r++; // increment memory refresh address
			u16 tempaddr = *(u16*)r + (s8)Z80_ProgByte(cpu);
			u8 n = cpu->readmem(tempaddr);
			op = Z80_ProgByte(cpu); // read next program byte
			u8 low = op & 0x07; // low 3 bits of the program byte
			op >>= 3; // high 5 bits of the program byte
#include "emu_z80_ddfdcb.c"
		}
		break;

	// POP IXY
	case 0xE1:
		Z80_POP(r[0], r[1]);
		cpu->sync.clock += Z80_CLOCKMUL*14;
		break;

	// EX (SP),IXY
	case 0xE3:
		{
			u16 sp = cpu->sp;
			u8 n = cpu->readmem(sp);
			u8 n2 = cpu->readmem(sp+1);
			cpu->writemem(sp, r[0]);
			cpu->writemem(sp+1, r[1]);
			r[0] = n;
			r[1] = n2;
		}
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// PUSH IXY
	case 0xE5:
		Z80_PUSH(r[0], r[1]);
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// JP (IXY)
	case 0xE9:
		cpu->pc = *(u16*)r;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD SP,IXY
	case 0xF9:
		cpu->sp = *(u16*)r;
		cpu->sync.clock += Z80_CLOCKMUL*10;
		break;

	// invalid opcode, roll back
	default:
		cpu->pc--;
		cpu->r--;
		break;
	}
}
