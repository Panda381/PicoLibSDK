
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

// RLC (IXY+d),r (reg = register or variable)
#define Z80_RLCX(reg) {					\
		Z80_RLC(n);				\
		reg = n;				\
		cpu->writemem(tempaddr, n); }

// RRC (IXY+d),r (reg = register or variable)
#define Z80_RRCX(reg) {					\
		Z80_RRC(n);				\
		reg = n;				\
		cpu->writemem(tempaddr, n); }

// RL (IXY+d),r (reg = register or variable)
#define Z80_RLX(reg) {					\
		Z80_RL(n);				\
		reg = n;				\
		cpu->writemem(tempaddr, n); }

// RR (IXY+d),r (reg = register or variable)
#define Z80_RRX(reg) {					\
		Z80_RR(n);				\
		reg = n;				\
		cpu->writemem(tempaddr, n); }

// SLA (IXY+d),r (reg = register or variable)
#define Z80_SLAX(reg) {					\
		Z80_SLA(n);				\
		reg = n;				\
		cpu->writemem(tempaddr, n); }

// SRA (IXY+d),r (reg = register or variable)
#define Z80_SRAX(reg) {					\
		Z80_SRA(n);				\
		reg = n;				\
		cpu->writemem(tempaddr, n); }

// SLL (IXY+d),r (reg = register or variable)
#define Z80_SLLX(reg) {					\
		Z80_SLL(n);				\
		reg = n;				\
		cpu->writemem(tempaddr, n); }

// SRL (IXY+d),r (reg = register or variable)
#define Z80_SRLX(reg) {					\
		Z80_SRL(n);				\
		reg = n;				\
		cpu->writemem(tempaddr, n); }


	// switch 0xDDFDCB operation code
	switch (PROGBYTE())
	{
	// RLC (IXY+d),B
	case 0x00:
		Z80_RLCX(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RLC (IXY+d),C
	case 0x01:
		Z80_RLCX(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RLC (IXY+d),D
	case 0x02:
		Z80_RLCX(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RLC (IXY+d),E
	case 0x03:
		Z80_RLCX(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RLC (IXY+d),H
	case 0x04:
		Z80_RLCX(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RLC (IXY+d),L
	case 0x05:
		Z80_RLCX(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RLC (IXY+d)
	case 0x06:
		Z80_RLCX(n);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RLC (IXY+d),A
	case 0x07:
		Z80_RLCX(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RRC (IXY+d),B
	case 0x08:
		Z80_RRCX(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RRC (IXY+d),C
	case 0x09:
		Z80_RRCX(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RRC (IXY+d),D
	case 0x0A:
		Z80_RRCX(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RRC (IXY+d),E
	case 0x0B:
		Z80_RRCX(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RRC (IXY+d),H
	case 0x0C:
		Z80_RRCX(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RRC (IXY+d),L
	case 0x0D:
		Z80_RRCX(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RRC (IXY+d)
	case 0x0E:
		Z80_RRCX(n);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RRC (IXY+d),A
	case 0x0F:
		Z80_RRCX(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RL (IXY+d),B
	case 0x10:
		Z80_RLX(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RL (IXY+d),C
	case 0x11:
		Z80_RLX(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RL (IXY+d),D
	case 0x12:
		Z80_RLX(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RL (IXY+d),E
	case 0x13:
		Z80_RLX(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RL (IXY+d),H
	case 0x14:
		Z80_RLX(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RL (IXY+d),L
	case 0x15:
		Z80_RLX(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RL (IXY+d)
	case 0x16:
		Z80_RLX(n);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RL (IXY+d),A
	case 0x17:
		Z80_RLX(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RR (IXY+d),B
	case 0x18:
		Z80_RRX(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RR (IXY+d),C
	case 0x19:
		Z80_RRX(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RR (IXY+d),D
	case 0x1A:
		Z80_RRX(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RR (IXY+d),E
	case 0x1B:
		Z80_RRX(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RR (IXY+d),H
	case 0x1C:
		Z80_RRX(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RR (IXY+d),L
	case 0x1D:
		Z80_RRX(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RR (IXY+d)
	case 0x1E:
		Z80_RRX(n);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RR (IXY+d),A
	case 0x1F:
		Z80_RRX(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLA (IXY+d),B
	case 0x20:
		Z80_SLAX(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLA (IXY+d),C
	case 0x21:
		Z80_SLAX(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLA (IXY+d),D
	case 0x22:
		Z80_SLAX(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLA (IXY+d),E
	case 0x23:
		Z80_SLAX(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLA (IXY+d),H
	case 0x24:
		Z80_SLAX(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLA (IXY+d),L
	case 0x25:
		Z80_SLAX(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLA (IXY+d)
	case 0x26:
		Z80_SLAX(n);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLA (IXY+d),A
	case 0x27:
		Z80_SLAX(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRA (IXY+d),B
	case 0x28:
		Z80_SRAX(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRA (IXY+d),C
	case 0x29:
		Z80_SRAX(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRA (IXY+d),D
	case 0x2A:
		Z80_SRAX(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRA (IXY+d),E
	case 0x2B:
		Z80_SRAX(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRA (IXY+d),H
	case 0x2C:
		Z80_SRAX(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRA (IXY+d),L
	case 0x2D:
		Z80_SRAX(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRA (IXY+d)
	case 0x2E:
		Z80_SRAX(n);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRA (IXY+d),A
	case 0x2F:
		Z80_SRAX(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLL (IXY+d),B
	case 0x30:
		Z80_SLLX(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLL (IXY+d),C
	case 0x31:
		Z80_SLLX(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLL (IXY+d),D
	case 0x32:
		Z80_SLLX(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLL (IXY+d),E
	case 0x33:
		Z80_SLLX(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLL (IXY+d),H
	case 0x34:
		Z80_SLLX(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLL (IXY+d),L
	case 0x35:
		Z80_SLLX(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLL (IXY+d)
	case 0x36:
		Z80_SLLX(n);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SLL (IXY+d),A
	case 0x37:
		Z80_SLLX(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRL (IXY+d),B
	case 0x38:
		Z80_SRLX(cpu->b);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRL (IXY+d),C
	case 0x39:
		Z80_SRLX(cpu->c);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRL (IXY+d),D
	case 0x3A:
		Z80_SRLX(cpu->d);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRL (IXY+d),E
	case 0x3B:
		Z80_SRLX(cpu->e);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRL (IXY+d),H
	case 0x3C:
		Z80_SRLX(cpu->h);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRL (IXY+d),L
	case 0x3D:
		Z80_SRLX(cpu->l);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRL (IXY+d)
	case 0x3E:
		Z80_SRLX(n);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SRL (IXY+d),A
	case 0x3F:
		Z80_SRLX(cpu->a);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// BIT 0,(IXY+d)
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
		Z80_BIT(0, n);
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// BIT 1,(IXY+d)
	case 0x48:
	case 0x49:
	case 0x4A:
	case 0x4B:
	case 0x4C:
	case 0x4D:
	case 0x4E:
	case 0x4F:
		Z80_BIT(1, n);
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// BIT 2,(IXY+d)
	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
		Z80_BIT(2, n);
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// BIT 3,(IXY+d)
	case 0x58:
	case 0x59:
	case 0x5A:
	case 0x5B:
	case 0x5C:
	case 0x5D:
	case 0x5E:
	case 0x5F:
		Z80_BIT(3, n);
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// BIT 4,(IXY+d)
	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
		Z80_BIT(4, n);
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// BIT 5,(IXY+d)
	case 0x68:
	case 0x69:
	case 0x6A:
	case 0x6B:
	case 0x6C:
	case 0x6D:
	case 0x6E:
	case 0x6F:
		Z80_BIT(5, n);
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// BIT 6,(IXY+d)
	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x77:
		Z80_BIT(6, n);
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// BIT 7,(IXY+d)
	case 0x78:
	case 0x79:
	case 0x7A:
	case 0x7B:
	case 0x7C:
	case 0x7D:
	case 0x7E:
	case 0x7F:
		Z80_BIT(7, n);
		cpu->sync.clock += Z80_CLOCKMUL*20;
		break;

	// RES 0,(IXY+d),B
	case 0x80:
		n &= ~BIT(0);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 0,(IXY+d),C
	case 0x81:
		n &= ~BIT(0);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 0,(IXY+d),D
	case 0x82:
		n &= ~BIT(0);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 0,(IXY+d),E
	case 0x83:
		n &= ~BIT(0);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 0,(IXY+d),H
	case 0x84:
		n &= ~BIT(0);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 0,(IXY+d),L
	case 0x85:
		n &= ~BIT(0);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 0,(IXY+d)
	case 0x86:
		n &= ~BIT(0);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 0,(IXY+d),A
	case 0x87:
		n &= ~BIT(0);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 1,(IXY+d),B
	case 0x88:
		n &= ~BIT(1);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 1,(IXY+d),C
	case 0x89:
		n &= ~BIT(1);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 1,(IXY+d),D
	case 0x8A:
		n &= ~BIT(1);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 1,(IXY+d),E
	case 0x8B:
		n &= ~BIT(1);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 1,(IXY+d),H
	case 0x8C:
		n &= ~BIT(1);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 1,(IXY+d),L
	case 0x8D:
		n &= ~BIT(1);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 1,(IXY+d)
	case 0x8E:
		n &= ~BIT(1);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 1,(IXY+d),A
	case 0x8F:
		n &= ~BIT(1);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 2,(IXY+d),B
	case 0x90:
		n &= ~BIT(2);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 2,(IXY+d),C
	case 0x91:
		n &= ~BIT(2);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 2,(IXY+d),D
	case 0x92:
		n &= ~BIT(2);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 2,(IXY+d),E
	case 0x93:
		n &= ~BIT(2);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 2,(IXY+d),H
	case 0x94:
		n &= ~BIT(2);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 2,(IXY+d),L
	case 0x95:
		n &= ~BIT(2);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 2,(IXY+d)
	case 0x96:
		n &= ~BIT(2);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 2,(IXY+d),A
	case 0x97:
		n &= ~BIT(2);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 3,(IXY+d),B
	case 0x98:
		n &= ~BIT(3);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 3,(IXY+d),C
	case 0x99:
		n &= ~BIT(3);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 3,(IXY+d),D
	case 0x9A:
		n &= ~BIT(3);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 3,(IXY+d),E
	case 0x9B:
		n &= ~BIT(3);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 3,(IXY+d),H
	case 0x9C:
		n &= ~BIT(3);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 3,(IXY+d),L
	case 0x9D:
		n &= ~BIT(3);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 3,(IXY+d)
	case 0x9E:
		n &= ~BIT(3);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 3,(IXY+d),A
	case 0x9F:
		n &= ~BIT(3);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 4,(IXY+d),B
	case 0xA0:
		n &= ~BIT(4);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 4,(IXY+d),C
	case 0xA1:
		n &= ~BIT(4);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 4,(IXY+d),D
	case 0xA2:
		n &= ~BIT(4);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 4,(IXY+d),E
	case 0xA3:
		n &= ~BIT(4);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 4,(IXY+d),H
	case 0xA4:
		n &= ~BIT(4);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 4,(IXY+d),L
	case 0xA5:
		n &= ~BIT(4);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 4,(IXY+d)
	case 0xA6:
		n &= ~BIT(4);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 4,(IXY+d),A
	case 0xA7:
		n &= ~BIT(4);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 5,(IXY+d),B
	case 0xA8:
		n &= ~BIT(5);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 5,(IXY+d),C
	case 0xA9:
		n &= ~BIT(5);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 5,(IXY+d),D
	case 0xAA:
		n &= ~BIT(5);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 5,(IXY+d),E
	case 0xAB:
		n &= ~BIT(5);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 5,(IXY+d),H
	case 0xAC:
		n &= ~BIT(5);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 5,(IXY+d),L
	case 0xAD:
		n &= ~BIT(5);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 5,(IXY+d)
	case 0xAE:
		n &= ~BIT(5);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 5,(IXY+d),A
	case 0xAF:
		n &= ~BIT(5);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 6,(IXY+d),B
	case 0xB0:
		n &= ~BIT(6);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 6,(IXY+d),C
	case 0xB1:
		n &= ~BIT(6);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 6,(IXY+d),D
	case 0xB2:
		n &= ~BIT(6);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 6,(IXY+d),E
	case 0xB3:
		n &= ~BIT(6);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 6,(IXY+d),H
	case 0xB4:
		n &= ~BIT(6);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 6,(IXY+d),L
	case 0xB5:
		n &= ~BIT(6);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 6,(IXY+d)
	case 0xB6:
		n &= ~BIT(6);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 6,(IXY+d),A
	case 0xB7:
		n &= ~BIT(6);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 7,(IXY+d),B
	case 0xB8:
		n &= ~BIT(7);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 7,(IXY+d),C
	case 0xB9:
		n &= ~BIT(7);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 7,(IXY+d),D
	case 0xBA:
		n &= ~BIT(7);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 7,(IXY+d),E
	case 0xBB:
		n &= ~BIT(7);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 7,(IXY+d),H
	case 0xBC:
		n &= ~BIT(7);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 7,(IXY+d),L
	case 0xBD:
		n &= ~BIT(7);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 7,(IXY+d)
	case 0xBE:
		n &= ~BIT(7);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// RES 7,(IXY+d),A
	case 0xBF:
		n &= ~BIT(7);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 0,(IXY+d),B
	case 0xC0:
		n |= BIT(0);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 0,(IXY+d),C
	case 0xC1:
		n |= BIT(0);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 0,(IXY+d),D
	case 0xC2:
		n |= BIT(0);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 0,(IXY+d),E
	case 0xC3:
		n |= BIT(0);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 0,(IXY+d),H
	case 0xC4:
		n |= BIT(0);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 0,(IXY+d),L
	case 0xC5:
		n |= BIT(0);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 0,(IXY+d)
	case 0xC6:
		n |= BIT(0);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 0,(IXY+d),A
	case 0xC7:
		n |= BIT(0);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 1,(IXY+d),B
	case 0xC8:
		n |= BIT(1);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 1,(IXY+d),C
	case 0xC9:
		n |= BIT(1);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 1,(IXY+d),D
	case 0xCA:
		n |= BIT(1);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 1,(IXY+d),E
	case 0xCB:
		n |= BIT(1);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 1,(IXY+d),H
	case 0xCC:
		n |= BIT(1);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 1,(IXY+d),L
	case 0xCD:
		n |= BIT(1);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 1,(IXY+d)
	case 0xCE:
		n |= BIT(1);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 1,(IXY+d),A
	case 0xCF:
		n |= BIT(1);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 2,(IXY+d),B
	case 0xD0:
		n |= BIT(2);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 2,(IXY+d),C
	case 0xD1:
		n |= BIT(2);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 2,(IXY+d),D
	case 0xD2:
		n |= BIT(2);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 2,(IXY+d),E
	case 0xD3:
		n |= BIT(2);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 2,(IXY+d),H
	case 0xD4:
		n |= BIT(2);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 2,(IXY+d),L
	case 0xD5:
		n |= BIT(2);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 2,(IXY+d)
	case 0xD6:
		n |= BIT(2);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 2,(IXY+d),A
	case 0xD7:
		n |= BIT(2);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 3,(IXY+d),B
	case 0xD8:
		n |= BIT(3);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 3,(IXY+d),C
	case 0xD9:
		n |= BIT(3);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 3,(IXY+d),D
	case 0xDA:
		n |= BIT(3);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 3,(IXY+d),E
	case 0xDB:
		n |= BIT(3);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 3,(IXY+d),H
	case 0xDC:
		n |= BIT(3);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 3,(IXY+d),L
	case 0xDD:
		n |= BIT(3);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 3,(IXY+d)
	case 0xDE:
		n |= BIT(3);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 3,(IXY+d),A
	case 0xDF:
		n |= BIT(3);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 4,(IXY+d),B
	case 0xE0:
		n |= BIT(4);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 4,(IXY+d),C
	case 0xE1:
		n |= BIT(4);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 4,(IXY+d),D
	case 0xE2:
		n |= BIT(4);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 4,(IXY+d),E
	case 0xE3:
		n |= BIT(4);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 4,(IXY+d),H
	case 0xE4:
		n |= BIT(4);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 4,(IXY+d),L
	case 0xE5:
		n |= BIT(4);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 4,(IXY+d)
	case 0xE6:
		n |= BIT(4);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 4,(IXY+d),A
	case 0xE7:
		n |= BIT(4);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 5,(IXY+d),B
	case 0xE8:
		n |= BIT(5);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 5,(IXY+d),C
	case 0xE9:
		n |= BIT(5);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 5,(IXY+d),D
	case 0xEA:
		n |= BIT(5);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 5,(IXY+d),E
	case 0xEB:
		n |= BIT(5);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 5,(IXY+d),H
	case 0xEC:
		n |= BIT(5);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 5,(IXY+d),L
	case 0xED:
		n |= BIT(5);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 5,(IXY+d)
	case 0xEE:
		n |= BIT(5);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 5,(IXY+d),A
	case 0xEF:
		n |= BIT(5);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 6,(IXY+d),B
	case 0xF0:
		n |= BIT(6);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 6,(IXY+d),C
	case 0xF1:
		n |= BIT(6);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 6,(IXY+d),D
	case 0xF2:
		n |= BIT(6);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 6,(IXY+d),E
	case 0xF3:
		n |= BIT(6);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 6,(IXY+d),H
	case 0xF4:
		n |= BIT(6);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 6,(IXY+d),L
	case 0xF5:
		n |= BIT(6);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 6,(IXY+d)
	case 0xF6:
		n |= BIT(6);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 6,(IXY+d),A
	case 0xF7:
		n |= BIT(6);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 7,(IXY+d),B
	case 0xF8:
		n |= BIT(7);
		cpu->b = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 7,(IXY+d),C
	case 0xF9:
		n |= BIT(7);
		cpu->c = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 7,(IXY+d),D
	case 0xFA:
		n |= BIT(7);
		cpu->d = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 7,(IXY+d),E
	case 0xFB:
		n |= BIT(7);
		cpu->e = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 7,(IXY+d),H
	case 0xFC:
		n |= BIT(7);
		cpu->h = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 7,(IXY+d),L
	case 0xFD:
		n |= BIT(7);
		cpu->l = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 7,(IXY+d)
	case 0xFE:
		n |= BIT(7);
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;

	// SET 7,(IXY+d),A
	case 0xFF:
		n |= BIT(7);
		cpu->a = n;
		cpu->writemem(tempaddr, n);
		cpu->sync.clock += Z80_CLOCKMUL*23;
		break;
	}
