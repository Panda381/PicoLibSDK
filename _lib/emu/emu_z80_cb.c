
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

	// switch 0xCB operation code
	switch (PROGBYTE())
	{
	// RLC B
	case 0x00:
		Z80_RLC(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RLC C
	case 0x01:
		Z80_RLC(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RLC D
	case 0x02:
		Z80_RLC(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RLC E
	case 0x03:
		Z80_RLC(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RLC H
	case 0x04:
		Z80_RLC(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RLC L
	case 0x05:
		Z80_RLC(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RLC (HL)
	case 0x06:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_RLC(n);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// RLC A
	case 0x07:
		Z80_RLC(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RRC B
	case 0x08:
		Z80_RRC(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RRC C
	case 0x09:
		Z80_RRC(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RRC D
	case 0x0A:
		Z80_RRC(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RRC E
	case 0x0B:
		Z80_RRC(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RRC H
	case 0x0C:
		Z80_RRC(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RRC L
	case 0x0D:
		Z80_RRC(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RRC (HL)
	case 0x0E:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_RRC(n);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// RRC A
	case 0x0F:
		Z80_RRC(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RL B
	case 0x10:
		Z80_RL(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RL C
	case 0x11:
		Z80_RL(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RL D
	case 0x12:
		Z80_RL(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RL E
	case 0x13:
		Z80_RL(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RL H
	case 0x14:
		Z80_RL(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RL L
	case 0x15:
		Z80_RL(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RL (HL)
	case 0x16:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_RL(n);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// RL A
	case 0x17:
		Z80_RL(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RR B
	case 0x18:
		Z80_RR(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RR C
	case 0x19:
		Z80_RR(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RR D
	case 0x1A:
		Z80_RR(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RR E
	case 0x1B:
		Z80_RR(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RR H
	case 0x1C:
		Z80_RR(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RR L
	case 0x1D:
		Z80_RR(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RR (HL)
	case 0x1E:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_RR(n);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// RR A
	case 0x1F:
		Z80_RR(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLA B
	case 0x20:
		Z80_SLA(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLA C
	case 0x21:
		Z80_SLA(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLA D
	case 0x22:
		Z80_SLA(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLA E
	case 0x23:
		Z80_SLA(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLA H
	case 0x24:
		Z80_SLA(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLA L
	case 0x25:
		Z80_SLA(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLA (HL)
	case 0x26:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_SLA(n);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// SLA A
	case 0x27:
		Z80_SLA(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRA B
	case 0x28:
		Z80_SRA(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRA C
	case 0x29:
		Z80_SRA(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRA D
	case 0x2A:
		Z80_SRA(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRA E
	case 0x2B:
		Z80_SRA(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRA H
	case 0x2C:
		Z80_SRA(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRA L
	case 0x2D:
		Z80_SRA(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRA (HL)
	case 0x2E:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_SRA(n);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// SRA A
	case 0x2F:
		Z80_SRA(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLL B
	case 0x30:
		Z80_SLL(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLL C
	case 0x31:
		Z80_SLL(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLL D
	case 0x32:
		Z80_SLL(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLL E
	case 0x33:
		Z80_SLL(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLL H
	case 0x34:
		Z80_SLL(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLL L
	case 0x35:
		Z80_SLL(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SLL (HL)
	case 0x36:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_SLL(n);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// SLL A
	case 0x37:
		Z80_SLL(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRL B
	case 0x38:
		Z80_SRL(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRL C
	case 0x39:
		Z80_SRL(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRL D
	case 0x3A:
		Z80_SRL(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRL E
	case 0x3B:
		Z80_SRL(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRL H
	case 0x3C:
		Z80_SRL(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRL L
	case 0x3D:
		Z80_SRL(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SRL (HL)
	case 0x3E:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_SRL(n);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// SRL A
	case 0x3F:
		Z80_SRL(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 0,B
	case 0x40:
		Z80_BIT(0, cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 0,C
	case 0x41:
		Z80_BIT(0, cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 0,D
	case 0x42:
		Z80_BIT(0, cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 0,E
	case 0x43:
		Z80_BIT(0, cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 0,H
	case 0x44:
		Z80_BIT(0, cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 0,L
	case 0x45:
		Z80_BIT(0, cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 0,(HL)
	case 0x46:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_BIT(0, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*12;
		break;

	// BIT 0,A
	case 0x47:
		Z80_BIT(0, cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 1,B
	case 0x48:
		Z80_BIT(1, cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 1,C
	case 0x49:
		Z80_BIT(1, cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 1,D
	case 0x4A:
		Z80_BIT(1, cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 1,E
	case 0x4B:
		Z80_BIT(1, cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 1,H
	case 0x4C:
		Z80_BIT(1, cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 1,L
	case 0x4D:
		Z80_BIT(1, cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 1,(HL)
	case 0x4E:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_BIT(1, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*12;
		break;

	// BIT 1,A
	case 0x4F:
		Z80_BIT(1, cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 2,B
	case 0x50:
		Z80_BIT(2, cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 2,C
	case 0x51:
		Z80_BIT(2, cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 2,D
	case 0x52:
		Z80_BIT(2, cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 2,E
	case 0x53:
		Z80_BIT(2, cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 2,H
	case 0x54:
		Z80_BIT(2, cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 2,L
	case 0x55:
		Z80_BIT(2, cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 2,(HL)
	case 0x56:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_BIT(2, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*12;
		break;

	// BIT 2,A
	case 0x57:
		Z80_BIT(2, cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 3,B
	case 0x58:
		Z80_BIT(3, cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 3,C
	case 0x59:
		Z80_BIT(3, cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 3,D
	case 0x5A:
		Z80_BIT(3, cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 3,E
	case 0x5B:
		Z80_BIT(3, cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 3,H
	case 0x5C:
		Z80_BIT(3, cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 3,L
	case 0x5D:
		Z80_BIT(3, cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 3,(HL)
	case 0x5E:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_BIT(3, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*12;
		break;

	// BIT 3,A
	case 0x5F:
		Z80_BIT(3, cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 4,B
	case 0x60:
		Z80_BIT(4, cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 4,C
	case 0x61:
		Z80_BIT(4, cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 4,D
	case 0x62:
		Z80_BIT(4, cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 4,E
	case 0x63:
		Z80_BIT(4, cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 4,H
	case 0x64:
		Z80_BIT(4, cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 4,L
	case 0x65:
		Z80_BIT(4, cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 4,(HL)
	case 0x66:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_BIT(4, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*12;
		break;

	// BIT 4,A
	case 0x67:
		Z80_BIT(4, cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 5,B
	case 0x68:
		Z80_BIT(5, cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 5,C
	case 0x69:
		Z80_BIT(5, cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 5,D
	case 0x6A:
		Z80_BIT(5, cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 5,E
	case 0x6B:
		Z80_BIT(5, cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 5,H
	case 0x6C:
		Z80_BIT(5, cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 5,L
	case 0x6D:
		Z80_BIT(5, cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 5,(HL)
	case 0x6E:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_BIT(5, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*12;
		break;

	// BIT 5,A
	case 0x6F:
		Z80_BIT(5, cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 6,B
	case 0x70:
		Z80_BIT(6, cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 6,C
	case 0x71:
		Z80_BIT(6, cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 6,D
	case 0x72:
		Z80_BIT(6, cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 6,E
	case 0x73:
		Z80_BIT(6, cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 6,H
	case 0x74:
		Z80_BIT(6, cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 6,L
	case 0x75:
		Z80_BIT(6, cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 6,(HL)
	case 0x76:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_BIT(6, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*12;
		break;

	// BIT 6,A
	case 0x77:
		Z80_BIT(6, cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 7,B
	case 0x78:
		Z80_BIT(7, cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 7,C
	case 0x79:
		Z80_BIT(7, cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 7,D
	case 0x7A:
		Z80_BIT(7, cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 7,E
	case 0x7B:
		Z80_BIT(7, cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 7,H
	case 0x7C:
		Z80_BIT(7, cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 7,L
	case 0x7D:
		Z80_BIT(7, cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// BIT 7,(HL)
	case 0x7E:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			Z80_BIT(7, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*12;
		break;

	// BIT 7,A
	case 0x7F:
		Z80_BIT(7, cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 0,B
	case 0x80:
		cpu->b &= ~BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 0,C
	case 0x81:
		cpu->c &= ~BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 0,D
	case 0x82:
		cpu->d &= ~BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 0,E
	case 0x83:
		cpu->e &= ~BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 0,H
	case 0x84:
		cpu->h &= ~BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 0,L
	case 0x85:
		cpu->l &= ~BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 0,(HL)
	case 0x86:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n &= ~BIT(0);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// RES 0,A
	case 0x87:
		cpu->a &= ~BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 1,B
	case 0x88:
		cpu->b &= ~BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 1,C
	case 0x89:
		cpu->c &= ~BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 1,D
	case 0x8A:
		cpu->d &= ~BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 1,E
	case 0x8B:
		cpu->e &= ~BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 1,H
	case 0x8C:
		cpu->h &= ~BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 1,L
	case 0x8D:
		cpu->l &= ~BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 1,(HL)
	case 0x8E:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n &= ~BIT(1);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// RES 1,A
	case 0x8F:
		cpu->a &= ~BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 2,B
	case 0x90:
		cpu->b &= ~BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 2,C
	case 0x91:
		cpu->c &= ~BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 2,D
	case 0x92:
		cpu->d &= ~BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 2,E
	case 0x93:
		cpu->e &= ~BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 2,H
	case 0x94:
		cpu->h &= ~BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 2,L
	case 0x95:
		cpu->l &= ~BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 2,(HL)
	case 0x96:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n &= ~BIT(2);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// RES 2,A
	case 0x97:
		cpu->a &= ~BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 3,B
	case 0x98:
		cpu->b &= ~BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 3,C
	case 0x99:
		cpu->c &= ~BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 3,D
	case 0x9A:
		cpu->d &= ~BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 3,E
	case 0x9B:
		cpu->e &= ~BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 3,H
	case 0x9C:
		cpu->h &= ~BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 3,L
	case 0x9D:
		cpu->l &= ~BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 3,(HL)
	case 0x9E:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n &= ~BIT(3);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// RES 3,A
	case 0x9F:
		cpu->a &= ~BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 4,B
	case 0xA0:
		cpu->b &= ~BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 4,C
	case 0xA1:
		cpu->c &= ~BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 4,D
	case 0xA2:
		cpu->d &= ~BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 4,E
	case 0xA3:
		cpu->e &= ~BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 4,H
	case 0xA4:
		cpu->h &= ~BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 4,L
	case 0xA5:
		cpu->l &= ~BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 4,(HL)
	case 0xA6:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n &= ~BIT(4);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// RES 4,A
	case 0xA7:
		cpu->a &= ~BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 5,B
	case 0xA8:
		cpu->b &= ~BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 5,C
	case 0xA9:
		cpu->c &= ~BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 5,D
	case 0xAA:
		cpu->d &= ~BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 5,E
	case 0xAB:
		cpu->e &= ~BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 5,H
	case 0xAC:
		cpu->h &= ~BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 5,L
	case 0xAD:
		cpu->l &= ~BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 5,(HL)
	case 0xAE:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n &= ~BIT(5);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// RES 5,A
	case 0xAF:
		cpu->a &= ~BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 6,B
	case 0xB0:
		cpu->b &= ~BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 6,C
	case 0xB1:
		cpu->c &= ~BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 6,D
	case 0xB2:
		cpu->d &= ~BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 6,E
	case 0xB3:
		cpu->e &= ~BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 6,H
	case 0xB4:
		cpu->h &= ~BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 6,L
	case 0xB5:
		cpu->l &= ~BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 6,(HL)
	case 0xB6:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n &= ~BIT(6);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// RES 6,A
	case 0xB7:
		cpu->a &= ~BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 7,B
	case 0xB8:
		cpu->b &= ~BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 7,C
	case 0xB9:
		cpu->c &= ~BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 7,D
	case 0xBA:
		cpu->d &= ~BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 7,E
	case 0xBB:
		cpu->e &= ~BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 7,H
	case 0xBC:
		cpu->h &= ~BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 7,L
	case 0xBD:
		cpu->l &= ~BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// RES 7,(HL)
	case 0xBE:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n &= ~BIT(7);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// RES 7,A
	case 0xBF:
		cpu->a &= ~BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 0,B
	case 0xC0:
		cpu->b |= BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 0,C
	case 0xC1:
		cpu->c |= BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 0,D
	case 0xC2:
		cpu->d |= BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 0,E
	case 0xC3:
		cpu->e |= BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 0,H
	case 0xC4:
		cpu->h |= BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 0,L
	case 0xC5:
		cpu->l |= BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 0,(HL)
	case 0xC6:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n |= BIT(0);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// SET 0,A
	case 0xC7:
		cpu->a |= BIT(0);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 1,B
	case 0xC8:
		cpu->b |= BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 1,C
	case 0xC9:
		cpu->c |= BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 1,D
	case 0xCA:
		cpu->d |= BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 1,E
	case 0xCB:
		cpu->e |= BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 1,H
	case 0xCC:
		cpu->h |= BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 1,L
	case 0xCD:
		cpu->l |= BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 1,(HL)
	case 0xCE:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n |= BIT(1);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// SET 1,A
	case 0xCF:
		cpu->a |= BIT(1);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 2,B
	case 0xD0:
		cpu->b |= BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 2,C
	case 0xD1:
		cpu->c |= BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 2,D
	case 0xD2:
		cpu->d |= BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 2,E
	case 0xD3:
		cpu->e |= BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 2,H
	case 0xD4:
		cpu->h |= BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 2,L
	case 0xD5:
		cpu->l |= BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 2,(HL)
	case 0xD6:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n |= BIT(2);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// SET 2,A
	case 0xD7:
		cpu->a |= BIT(2);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 3,B
	case 0xD8:
		cpu->b |= BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 3,C
	case 0xD9:
		cpu->c |= BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 3,D
	case 0xDA:
		cpu->d |= BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 3,E
	case 0xDB:
		cpu->e |= BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 3,H
	case 0xDC:
		cpu->h |= BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 3,L
	case 0xDD:
		cpu->l |= BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 3,(HL)
	case 0xDE:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n |= BIT(3);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// SET 3,A
	case 0xDF:
		cpu->a |= BIT(3);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 4,B
	case 0xE0:
		cpu->b |= BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 4,C
	case 0xE1:
		cpu->c |= BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 4,D
	case 0xE2:
		cpu->d |= BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 4,E
	case 0xE3:
		cpu->e |= BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 4,H
	case 0xE4:
		cpu->h |= BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 4,L
	case 0xE5:
		cpu->l |= BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 4,(HL)
	case 0xE6:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n |= BIT(4);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// SET 4,A
	case 0xE7:
		cpu->a |= BIT(4);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 5,B
	case 0xE8:
		cpu->b |= BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 5,C
	case 0xE9:
		cpu->c |= BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 5,D
	case 0xEA:
		cpu->d |= BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 5,E
	case 0xEB:
		cpu->e |= BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 5,H
	case 0xEC:
		cpu->h |= BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 5,L
	case 0xED:
		cpu->l |= BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 5,(HL)
	case 0xEE:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n |= BIT(5);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// SET 5,A
	case 0xEF:
		cpu->a |= BIT(5);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 6,B
	case 0xF0:
		cpu->b |= BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 6,C
	case 0xF1:
		cpu->c |= BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 6,D
	case 0xF2:
		cpu->d |= BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 6,E
	case 0xF3:
		cpu->e |= BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 6,H
	case 0xF4:
		cpu->h |= BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 6,L
	case 0xF5:
		cpu->l |= BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 6,(HL)
	case 0xF6:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n |= BIT(6);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// SET 6,A
	case 0xF7:
		cpu->a |= BIT(6);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 7,B
	case 0xF8:
		cpu->b |= BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 7,C
	case 0xF9:
		cpu->c |= BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 7,D
	case 0xFA:
		cpu->d |= BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 7,E
	case 0xFB:
		cpu->e |= BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 7,H
	case 0xFC:
		cpu->h |= BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 7,L
	case 0xFD:
		cpu->l |= BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;

	// SET 7,(HL)
	case 0xFE:
		{
			u16 nn = cpu->hl;
			u8 n = cpu->readmem(nn);
			n |= BIT(7);
			cpu->writemem(nn, n);
		}
		cpu->sync.clock += Z80_CLOCKMUL*15;
		break;

	// SET 7,A
	case 0xFF:
		cpu->a |= BIT(7);
		cpu->sync.clock += Z80_CLOCKMUL*8;
		break;
	}
