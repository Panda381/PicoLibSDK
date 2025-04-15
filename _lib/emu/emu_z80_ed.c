
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

	op = Z80_ProgByte(cpu);

	// switch 0xED operation code
	switch (op)
	{
	// IN B,(C)
	case 0x40:
	// IN C,(C)
	case 0x48:
	// IN D,(C)
	case 0x50:
	// IN E,(C)
	case 0x58:
	// IN H,(C)
	case 0x60:
	// IN L,(C)
	case 0x68:
	// IN A,(C)
	case 0x78:
		{
			op -= 0x40;
			u8* rd = &cpu->reg[7 - (op >> 3)]; // destination register
			Z80_IN(*rd);
		}
		cpu->sync.clock += Z80_CLOCKMUL*12;
		break;

	// OUT (C),B
	case 0x41:
	// OUT (C),C
	case 0x49:
	// OUT (C),D
	case 0x51:
	// OUT (C),E
	case 0x59:
	// OUT (C),H
	case 0x61:
	// OUT (C),L
	case 0x69:
	// OUT (C),A
	case 0x79:
		{
			op -= 0x41;
			u8* rd = &cpu->reg[7 - (op >> 3)]; // destination register
			cpu->writeport(cpu->bc, *rd);
		}
		cpu->sync.clock += Z80_CLOCKMUL*12;
		break;

	// SBC HL,BC
	case 0x42:
	// SBC HL,DE
	case 0x52:
	// SBC HL,HL
	case 0x62:
		{
			u16* r = &cpu->dreg[3 - ((op >> 4) - 4)]; // double register
			Z80_SBC16(*r);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// LD (a),BC
	case 0x43:
	// LD (a),DE
	case 0x53:
	// LD (a),HL
	case 0x63:
		{
			u16 nn = Z80_ProgWord(cpu);
			u8* r = &cpu->reg[6 - ((op >> 3) - 8)]; // register low
			cpu->writemem(nn, r[0]);
			nn++;
			cpu->writemem(nn, r[1]);
		}
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// NEG
	case 0x44:
	case 0x4C:
	case 0x54:
	case 0x5C:
	case 0x64:
	case 0x6C:
	case 0x74:
	case 0x7C:
		{
			u8 n = cpu->a;
			u8 n2 = -n;
			cpu->a = n2;
			//	C ... set if result is not 0
			//	N ... set
			//	PV ... set if result is 0x80
			//	X ... copy from result
			//	H ... set if result low nibble is not 0
			//	Y ... copy from result
			//	Z ... set if result is 0
			//	S ... copy from result (set if result is >= 0x80, i.e. negative)
			u8 f = (n2 == 0) ? Z80_Z : Z80_C;
			if ((n2 & 0x0f) != 0) f |= Z80_H;
			if (n2 == 0x80) f |= Z80_PV;
			f |= n2 & (Z80_S | Z80_X | Z80_Y);
			f |= Z80_N;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RETN, RETI
	case 0x45:
	case 0x4D: // RETI (RETI returns IFF1 in the same way as RETN)
	case 0x55:
	case 0x5D:
	case 0x65:
	case 0x6D:
	case 0x75:
	case 0x7D:
		cpu->iff1 = cpu->iff2;
		Z80_RET();
		cpu->sync.clock += Z80_CLOCKMUL*14;
		break;

	// IM 0
	case 0x46:
	case 0x4E:
	case 0x66:
	case 0x6E:
		cpu->mode = Z80_INTMODE0;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD I,A
	case 0x47:
		cpu->i = cpu->a;
		cpu->sync.clock += Z80_CLOCKMUL*9;
		break;

	// ADC HL,BC
	case 0x4A:
	// ADC HL,DE
	case 0x5A:
	// ADC HL,HL
	case 0x6A:
		{
			u16* r = &cpu->dreg[3 - ((op >> 4) - 4)]; // double register
			Z80_ADC16(*r);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// LD BC,(a)
	case 0x4B:
	// LD DE,(a)
	case 0x5B:
	// LD HL,(a)
	case 0x6B:
		{
			u16 nn = Z80_ProgWord(cpu);
			u8* r = &cpu->reg[6 - ((op >> 3) - 9)]; // register low
			r[0] = cpu->readmem(nn);
			nn++;
			r[1] = cpu->readmem(nn);
		}
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// LD R,A
	case 0x4F:
		{
			u8 n = cpu->a;
			cpu->r = n;
			cpu->r7 = n;
		}
		cpu->sync.clock += Z80_CLOCKMUL*9;
		break;

	// IM 1
	case 0x56:
	case 0x76:
		cpu->mode = Z80_INTMODE1;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD A,I
	case 0x57:
		{
			u8 n = cpu->i;
			cpu->a = n;
			//	C ... not affected
			//	N ... reset
			//	PV ... content of IFF2
			//	X ... copy from result
			//	H ... reset
			//	Y ... copy from result
			//	Z ... set if result is 0
			//	S ... copy from result (set if result is >= 0x80, i.e. negative)
			u8 f = cpu->f & Z80_C;
			if (n == 0) f |= Z80_Z;
			f |= n & (Z80_S | Z80_X | Z80_Y);
			if (cpu->iff2 != 0) f |= Z80_PV;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*9;
		break;

	// IM 2
	case 0x5E:
	case 0x7E:
		cpu->mode = Z80_INTMODE2;
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// LD A,R
	case 0x5F:
		{
			u8 n = (cpu->r & 0x7f) | (cpu->r7 & 0x80);
			cpu->a = n;
			//	C ... not affected
			//	N ... reset
			//	PV ... content of IFF2
			//	X ... copy from result
			//	H ... reset
			//	Y ... copy from result
			//	Z ... set if result is 0
			//	S ... copy from result (set if result is >= 0x80, i.e. negative)
			u8 f = cpu->f & Z80_C;
			if (n == 0) f |= Z80_Z;
			f |= n & (Z80_S | Z80_X | Z80_Y);
			if (cpu->iff2 != 0) f |= Z80_PV;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*9;
		break;

	// RRD
	case 0x67:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			u8 n2 = cpu->a;
			cpu->writemem(nn, (u8)((n2 << 4) | (n >> 4)));
			n2 = (n2 & 0xf0) | (n & 0x0f);
			cpu->a = n2;
			//	C ... not affected
			//	N ... reset
			//	PV ... parity
			//	X ... copy from result
			//	H ... reset
			//	Y ... copy from result
			//	Z ... set if result is 0
			//	S ... copy from result (set if result is >= 0x80, i.e. negative)
			u8 f = cpu->f & Z80_C;
			f |= Z80_FlagParTab[n2];
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*18;
		break;

	// RLD
	case 0x6F:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			u8 n2 = cpu->a;
			cpu->writemem(nn, (u8)((n2 & 0x0f) | (n << 4)));
			n2 = (n2 & 0xf0) | (n >> 4);
			cpu->a = n2;
			//	C ... not affected
			//	N ... reset
			//	PV ... parity
			//	X ... copy from result
			//	H ... reset
			//	Y ... copy from result
			//	Z ... set if result is 0
			//	S ... copy from result (set if result is >= 0x80, i.e. negative)
			u8 f = cpu->f & Z80_C;
			f |= Z80_FlagParTab[n2];
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*18;
		break;

	// IN F,(C) ... IN (C)
	case 0x70:
		{
			u8 n;
			Z80_IN(n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*12;
		break;

	// OUT (C),0
	case 0x71:
		cpu->writeport(cpu->bc, 0);
		cpu->sync.clock += Z80_CLOCKMUL*12;
		break;

	// SBC HL,SP
	case 0x72:
		Z80_SBC16(cpu->sp);
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// LD (a),SP
	case 0x73:
		{
			u16 nn = Z80_ProgWord(cpu);
			cpu->writemem(nn, cpu->spl);
			nn++;
			cpu->writemem(nn, cpu->sph);
		}
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// ADC HL,SP
	case 0x7A:
		Z80_ADC16(cpu->sp);
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// LD SP,(a)
	case 0x7B:
		{
			u16 nn = Z80_ProgWord(cpu);
			cpu->spl = cpu->readmem(nn);
			nn++;
			cpu->sph = cpu->readmem(nn);
		}
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// LDI
	case 0xA0:
		{
			u16 hl = cpu->hl;
			u8 n = cpu->readmem(hl);
			hl++;
			cpu->hl = hl;

			u16 de = cpu->de;
			cpu->writemem(de, n);
			de++;
			cpu->de = de;

			u16 bc = cpu->bc - 1;
			cpu->bc = bc;

			//	C ... not affected
			//	N ... reset
			//	PV ... set if result BC not 0
			//	X ... copy from "A + n"
			//	H ... reset
			//	Y ... copy from bit 1 of "A + n"
			//	Z ... not affected
			//	S ... not affected
			n += cpu->a;
			u8 f = cpu->f & (Z80_C | Z80_Z | Z80_S);
			if (bc != 0) f |= Z80_PV;
			f |= n & Z80_X;
			if ((n & B1) != 0) f |= Z80_Y;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// CPI
	case 0xA1:
		{
			u16 hl = cpu->hl;
			u8 n = cpu->readmem(hl);
			hl++;
			cpu->hl = hl;

			u16 bc = cpu->bc - 1;
			cpu->bc = bc;

			//	C ... not affected
			//	N ... set
			//	PV ... set if result BC not 0
			//	X ... copy from "A - n - HF"
			//	H ... set if borrow from bit 4
			//	Y ... copy from bit 1 of "A - n - HF"
			//	Z ... set if "A == n"
			//	S ... set if "A - n" is negative
			u8 a = cpu->a;
			u8 res = a - n;
			u8 f = cpu->f & Z80_C;
			if (bc != 0) f |= Z80_PV;
			f |= Z80_N;
			if (res == 0) f |= Z80_Z;
			f |= res & Z80_S;
			a = (a ^ n ^ res) & Z80_H;
			if (a != 0) res--;
			f |= a;
			f |= res & Z80_X;
			if ((res & B1) != 0) f |= Z80_Y;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// INI
	case 0xA2:
		{
			u8 n = cpu->readport(cpu->bc);

			u16 hl = cpu->hl;
			cpu->writemem(hl, n);
			hl++;
			cpu->hl = hl;

			u8 b = cpu->b - 1;
			cpu->b = b;

			//	C ... set if ((n + (u8)(C+1)) > 255)
			//	N ... bit 7 of n
			//	PV ... parity of (((n + (u8)(C+1)) & 7) ^ B)
			//	X ... copy from result B
			//	H ... set if ((n + (u8)(C+1)) > 255)
			//	Y ... copy from result B
			//	Z ... set if result B is 0
			//	S ... copy from result B
			u8 f = b & (Z80_S | Z80_X | Z80_Y);
			if (b == 0) f |= Z80_Z;
			if (n >= 0x80) f |= Z80_N;
			u8 n2 = n + cpu->c + 1;
			if (n2 < n) f |= Z80_H | Z80_C;
			f |= Z80_FlagParTab[(n2 & 7) ^ b] & Z80_PV;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// OUTI
	case 0xA3:
		{
			u16 hl = cpu->hl;
			u8 n = cpu->readmem(hl);
			hl++;
			cpu->hl = hl;

			u8 b = cpu->b - 1;
			cpu->b = b;

			cpu->writeport(cpu->bc, n);

			//	C ... set if ((n + L) > 255)
			//	N ... bit 7 of n
			//	PV ... parity of (((n + L) & 7) ^ B)
			//	X ... copy from result B
			//	H ... set if ((n + L) > 255)
			//	Y ... copy from result B
			//	Z ... set if result B is 0
			//	S ... copy from result B
			u8 f = b & (Z80_S | Z80_X | Z80_Y);
			if (b == 0) f |= Z80_Z;
			if (n >= 0x80) f |= Z80_N;
			u8 n2 = n + cpu->l;
			if (n2 < n) f |= Z80_H | Z80_C;
			f |= Z80_FlagParTab[(n2 & 7) ^ b] & Z80_PV;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// LDD
	case 0xA8:
		{
			u16 hl = cpu->hl;
			u8 n = cpu->readmem(hl);
			hl--;
			cpu->hl = hl;

			u16 de = cpu->de;
			cpu->writemem(de, n);
			de--;
			cpu->de = de;

			u16 bc = cpu->bc - 1;
			cpu->bc = bc;

			//	C ... not affected
			//	N ... reset
			//	PV ... set if result BC not 0
			//	X ... copy from "A + n"
			//	H ... reset
			//	Y ... copy from bit 1 of "A + n"
			//	Z ... not affected
			//	S ... not affected
			n += cpu->a;
			u8 f = cpu->f & (Z80_C | Z80_Z | Z80_S);
			if (bc != 0) f |= Z80_PV;
			f |= n & Z80_X;
			if ((n & B1) != 0) f |= Z80_Y;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// CPD
	case 0xA9:
		{
			u16 hl = cpu->hl;
			u8 n = cpu->readmem(hl);
			hl--;
			cpu->hl = hl;

			u16 bc = cpu->bc - 1;
			cpu->bc = bc;

			//	C ... not affected
			//	N ... set
			//	PV ... set if result BC not 0
			//	X ... copy from "A - n - HF"
			//	H ... set if borrow from bit 4
			//	Y ... copy from bit 1 of "A - n - HF"
			//	Z ... set if "A == n"
			//	S ... set if "A - n" is negative
			u8 a = cpu->a;
			u8 res = a - n;
			u8 f = cpu->f & Z80_C;
			if (bc != 0) f |= Z80_PV;
			f |= Z80_N;
			if (res == 0) f |= Z80_Z;
			f |= res & Z80_S;
			a = (a ^ n ^ res) & Z80_H;
			if (a != 0) res--;
			f |= a;
			f |= res & Z80_X;
			if ((res & B1) != 0) f |= Z80_Y;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// IND
	case 0xAA:
		{
			u8 n = cpu->readport(cpu->bc);

			u16 hl = cpu->hl;
			cpu->writemem(hl, n);
			hl--;
			cpu->hl = hl;

			u8 b = cpu->b - 1;
			cpu->b = b;

			//	C ... set if ((n + (u8)(C-1)) > 255)
			//	N ... bit 7 of n
			//	PV ... parity of (((n + (u8)(C-1)) & 7) ^ B)
			//	X ... copy from result B
			//	H ... set if ((n + (u8)(C-1)) > 255)
			//	Y ... copy from result B
			//	Z ... set if result B is 0
			//	S ... copy from result B
			u8 f = b & (Z80_S | Z80_X | Z80_Y);
			if (b == 0) f |= Z80_Z;
			if (n >= 0x80) f |= Z80_N;
			u8 n2 = n + cpu->c - 1;
			if (n2 < n) f |= Z80_H | Z80_C;
			f |= Z80_FlagParTab[(n2 & 7) ^ b] & Z80_PV;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// OUTD
	case 0xAB:
		{
			u16 hl = cpu->hl;
			u8 n = cpu->readmem(hl);
			hl--;
			cpu->hl = hl;

			u8 b = cpu->b - 1;
			cpu->b = b;

			cpu->writeport(cpu->bc, n);

			//	C ... set if ((n + L) > 255)
			//	N ... bit 7 of n
			//	PV ... parity of (((n + L) & 7) ^ B)
			//	X ... copy from result B
			//	H ... set if ((n + L) > 255)
			//	Y ... copy from result B
			//	Z ... set if result B is 0
			//	S ... copy from result B
			u8 f = b & (Z80_S | Z80_X | Z80_Y);
			if (b == 0) f |= Z80_Z;
			if (n >= 0x80) f |= Z80_N;
			u8 n2 = n + cpu->l;
			if (n2 < n) f |= Z80_H | Z80_C;
			f |= Z80_FlagParTab[(n2 & 7) ^ b] & Z80_PV;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// LDIR
	case 0xB0:
		{
			u16 hl = cpu->hl;
			u8 n = cpu->readmem(hl);
			hl++;
			cpu->hl = hl;

			u16 de = cpu->de;
			cpu->writemem(de, n);
			de++;
			cpu->de = de;

			u16 bc = cpu->bc - 1;
			cpu->bc = bc;

			//	C ... not affected
			//	N ... reset
			//	PV ... set if result BC not 0
			//	X ... copy from "A + n"
			//	H ... reset
			//	Y ... copy from bit 1 of "A + n"
			//	Z ... not affected
			//	S ... not affected
			n += cpu->a;
			u8 f = cpu->f & (Z80_C | Z80_Z | Z80_S);
			if (bc != 0)
			{
				f |= Z80_PV;
				cpu->pc -= 2;
				cpu->sync.clock += Z80_CLOCKMUL*(21 - 16);
			}
			f |= n & Z80_X;
			if ((n & B1) != 0) f |= Z80_Y;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// CPIR
	case 0xB1:
		{
			u16 hl = cpu->hl;
			u8 n = cpu->readmem(hl);
			hl++;
			cpu->hl = hl;

			u16 bc = cpu->bc - 1;
			cpu->bc = bc;

			//	C ... not affected
			//	N ... set
			//	PV ... set if result BC not 0
			//	X ... copy from "A - n - HF"
			//	H ... set if borrow from bit 4
			//	Y ... copy from bit 1 of "A - n - HF"
			//	Z ... set if "A == n"
			//	S ... set if "A - n" is negative
			u8 a = cpu->a;
			u8 res = a - n;
			u8 f = cpu->f & Z80_C;
			if (bc != 0) f |= Z80_PV;
			f |= Z80_N;
			if (res == 0)
				f |= Z80_Z;
			else
			{
				if (bc != 0)
				{
					cpu->pc -= 2;
					cpu->sync.clock += Z80_CLOCKMUL*(21 - 16);
				}
			}
			f |= res & Z80_S;
			a = (a ^ n ^ res) & Z80_H;
			if (a != 0) res--;
			f |= a;
			f |= res & Z80_X;
			if ((res & B1) != 0) f |= Z80_Y;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// INIR
	case 0xB2:
		{
			u8 n = cpu->readport(cpu->bc);

			u16 hl = cpu->hl;
			cpu->writemem(hl, n);
			hl++;
			cpu->hl = hl;

			u8 b = cpu->b - 1;
			cpu->b = b;

			//	C ... set if ((n + (u8)(C+1)) > 255)
			//	N ... bit 7 of n
			//	PV ... parity of (((n + (u8)(C+1)) & 7) ^ B)
			//	X ... copy from result B
			//	H ... set if ((n + (u8)(C+1)) > 255)
			//	Y ... copy from result B
			//	Z ... set if result B is 0
			//	S ... copy from result B
			u8 f = b & (Z80_S | Z80_X | Z80_Y);
			if (b == 0)
				f |= Z80_Z;
			else
			{
				cpu->pc -= 2;
				cpu->sync.clock += Z80_CLOCKMUL*(21 - 16);
			}
			if (n >= 0x80) f |= Z80_N;
			u8 n2 = n + cpu->c + 1;
			if (n2 < n) f |= Z80_H | Z80_C;
			f |= Z80_FlagParTab[(n2 & 7) ^ b] & Z80_PV;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// OTIR
	case 0xB3:
		{
			u16 hl = cpu->hl;
			u8 n = cpu->readmem(hl);
			hl++;
			cpu->hl = hl;

			u8 b = cpu->b - 1;
			cpu->b = b;

			cpu->writeport(cpu->bc, n);

			//	C ... set if ((n + L) > 255)
			//	N ... bit 7 of n
			//	PV ... parity of (((n + L) & 7) ^ B)
			//	X ... copy from result B
			//	H ... set if ((n + L) > 255)
			//	Y ... copy from result B
			//	Z ... set if result B is 0
			//	S ... copy from result B
			u8 f = b & (Z80_S | Z80_X | Z80_Y);
			if (b == 0)
				f |= Z80_Z;
			else
			{
				cpu->pc -= 2;
				cpu->sync.clock += Z80_CLOCKMUL*(21 - 16);
			}
			if (n >= 0x80) f |= Z80_N;
			u8 n2 = n + cpu->l;
			if (n2 < n) f |= Z80_H | Z80_C;
			f |= Z80_FlagParTab[(n2 & 7) ^ b] & Z80_PV;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// LDDR
	case 0xB8:
		{
			u16 hl = cpu->hl;
			u8 n = cpu->readmem(hl);
			hl--;
			cpu->hl = hl;

			u16 de = cpu->de;
			cpu->writemem(de, n);
			de--;
			cpu->de = de;

			u16 bc = cpu->bc - 1;
			cpu->bc = bc;

			//	C ... not affected
			//	N ... reset
			//	PV ... set if result BC not 0
			//	X ... copy from "A + n"
			//	H ... reset
			//	Y ... copy from bit 1 of "A + n"
			//	Z ... not affected
			//	S ... not affected
			n += cpu->a;
			u8 f = cpu->f & (Z80_C | Z80_Z | Z80_S);
			if (bc != 0)
			{
				f |= Z80_PV;
				cpu->pc -= 2;
				cpu->sync.clock += Z80_CLOCKMUL*(21 - 16);
			}
			f |= n & Z80_X;
			if ((n & B1) != 0) f |= Z80_Y;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// CPDR
	case 0xB9:
		{
			u16 hl = cpu->hl;
			u8 n = cpu->readmem(hl);
			hl--;
			cpu->hl = hl;

			u16 bc = cpu->bc - 1;
			cpu->bc = bc;

			//	C ... not affected
			//	N ... set
			//	PV ... set if result BC not 0
			//	X ... copy from "A - n - HF"
			//	H ... set if borrow from bit 4
			//	Y ... copy from bit 1 of "A - n - HF"
			//	Z ... set if "A == n"
			//	S ... set if "A - n" is negative
			u8 a = cpu->a;
			u8 res = a - n;
			u8 f = cpu->f & Z80_C;
			if (bc != 0) f |= Z80_PV;
			f |= Z80_N;
			if (res == 0)
				f |= Z80_Z;
			else
			{
				if (bc != 0)
				{
					cpu->pc -= 2;
					cpu->sync.clock += Z80_CLOCKMUL*(21 - 16);
				}
			}
			f |= res & Z80_S;
			a = (a ^ n ^ res) & Z80_H;
			if (a != 0) res--;
			f |= a;
			f |= res & Z80_X;
			if ((res & B1) != 0) f |= Z80_Y;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// INDR
	case 0xBA:
		{
			u8 n = cpu->readport(cpu->bc);

			u16 hl = cpu->hl;
			cpu->writemem(hl, n);
			hl--;
			cpu->hl = hl;

			u8 b = cpu->b - 1;
			cpu->b = b;

			//	C ... set if ((n + (u8)(C-1)) > 255)
			//	N ... bit 7 of n
			//	PV ... parity of (((n + (u8)(C-1)) & 7) ^ B)
			//	X ... copy from result B
			//	H ... set if ((n + (u8)(C-1)) > 255)
			//	Y ... copy from result B
			//	Z ... set if result B is 0
			//	S ... copy from result B
			u8 f = b & (Z80_S | Z80_X | Z80_Y);
			if (b == 0)
				f |= Z80_Z;
			else
			{
				cpu->pc -= 2;
				cpu->sync.clock += Z80_CLOCKMUL*(21 - 16);
			}
			if (n >= 0x80) f |= Z80_N;
			u8 n2 = n + cpu->c - 1;
			if (n2 < n) f |= Z80_H | Z80_C;
			f |= Z80_FlagParTab[(n2 & 7) ^ b] & Z80_PV;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// OTDR
	case 0xBB:
		{
			u16 hl = cpu->hl;
			u8 n = cpu->readmem(hl);
			hl--;
			cpu->hl = hl;

			u8 b = cpu->b - 1;
			cpu->b = b;

			cpu->writeport(cpu->bc, n);

			//	C ... set if ((n + L) > 255)
			//	N ... bit 7 of n
			//	PV ... parity of (((n + L) & 7) ^ B)
			//	X ... copy from result B
			//	H ... set if ((n + L) > 255)
			//	Y ... copy from result B
			//	Z ... set if result B is 0
			//	S ... copy from result B
			u8 f = b & (Z80_S | Z80_X | Z80_Y);
			if (b == 0)
				f |= Z80_Z;
			else
			{
				cpu->pc -= 2;
				cpu->sync.clock += Z80_CLOCKMUL*(21 - 16);
			}
			if (n >= 0x80) f |= Z80_N;
			u8 n2 = n + cpu->l;
			if (n2 < n) f |= Z80_H | Z80_C;
			f |= Z80_FlagParTab[(n2 & 7) ^ b] & Z80_PV;
			cpu->f = f;
		}
		cpu->sync.clock += Z80_CLOCKMUL*16;
		break;

	// invalid opcode = NOP
	default:
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;
	}
