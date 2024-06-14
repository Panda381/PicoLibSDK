
// ****************************************************************************
//
//                         Sharp X80 (LR35902) CPU Emulator
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

#if USE_EMU_X80			// use Sharp X80 (LR35902) CPU emulator

#include "emu_x80_macro.h"

// current CPU descriptor (NULL = not running)
volatile sX80* X80_Cpu = NULL;

// reset processor
void X80_Reset(sX80* cpu)
{
	cpu->pc = 0;		// program counter
	cpu->sp = 0xfefe;	// stack pointer (under port area starting at 0xFF00 = X80_PORTBASE port base)
	cpu->ime = 0;		// disable interrupts
	cpu->ie = 0;		// interrupt enable mask
	cpu->halted = 0;	// halted
	cpu->stop = 0;		// stop request
	cpu->tma = 0;		// timer modulo
	cpu->divshift = 255;	// number of shifts (255 = disabled)
	cpu->tima = 0;		// TIMA counter
	cpu->lcd_mode = X80_LCD_MODE0; // LCD mode
	cpu->fa = 0;		// registers A and F
	cpu->hl = 0;		// registers HL
	cpu->de = 0;		// registers DE
	cpu->bc = 0;		// registers BC
	cpu->div = 0;		// DIV counter
	cpu->lcd_count = 0;	// LCD scanline time counter
	cpu->lcd_cycles = 456;	// LCD cycles per scanline (456/4.194304 = 912/8.388608 = 108.72 us)
	cpu->lcd_cycles2 = 204;	// LCD cycles per start mode 2 (OAM access, 204 cycles)
	cpu->lcd_cycles3 = 284;	// LCD cycles per start mode 3 (LCD transfer, 284 cycles)
}

// load byte from program memory
INLINE u8 X80_ProgByte(sX80* cpu)
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
INLINE u16 X80_ProgWord(sX80* cpu)
{
	u8 temp = X80_ProgByte(cpu);
	return (temp | ((u16)X80_ProgByte(cpu) << 8));
}

// execute program (start or continue, until "stop" request)
// Size of code of this function: 5532 code + 1280 jump table = 6812 bytes
// CPU loading at 4.194350 MHz on 154.667 MHz: used 29-40%, max. 37-45%
void FASTCODE NOFLASH(X80_Exec)(sX80* cpu)
{
	u8 op, divshift;
	u16 addr, newdiv, olddiv;
	u32 irq, oldclock;

	// clear stop flag
	cpu->stop = 0;

	// program loop
	while (cpu->stop == 0)
	{
		// save old clock (to update DIV divider)
		oldclock = cpu->sync.clock;

		// check interrupts
		irq = EmuInterGet(&cpu->sync) & cpu->ie;
		if (((irq & X80_IRQ_ALL) != 0) && (cpu->ime != 0))
		{
			// disable interrupts
			cpu->ime = 0;

			// continue after HALT
			if (cpu->halted != 0)
			{
				cpu->pc++;
				cpu->halted = 0;
			}

			// prepare service address
			op = 0;
			addr = 0x0040;
			while ((irq & 1) == 0)
			{
				irq >>= 1;
				op++;
				addr += 8;
			}

			// clear interrupt request
			EmuInterClrBit(&cpu->sync, op);

			// jump to service
			X80_CALL(addr);
			cpu->sync.clock += X80_CLOCKMUL*16;
		}

		// get next instruction
		op = X80_ProgByte(cpu);

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
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// LD BC,nn
		case 0x01:
		// LD DE,nn
		case 0x11:
		// LD HL,nn
		case 0x21:
			{
				u8* r = &cpu->reg[6 - (op >> 3)]; // register low
				r[0] = X80_ProgByte(cpu); // low
				r[1] = X80_ProgByte(cpu); // high
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// LD (BC),A
		case 0x02:
			cpu->writemem(cpu->bc, cpu->a);
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// INC BC
		case 0x03:
			cpu->bc++;
			cpu->sync.clock += X80_CLOCKMUL*8;
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
				X80_INC(*r);
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
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
				X80_DEC(*r);
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
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
				*r = X80_ProgByte(cpu);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// RLCA
		case 0x07:
			{
				u8 n = cpu->a;
				n = (n << 1) | (n >> 7);
				cpu->a = n;
				//	C ... carry from bit 7
				//	H ... reset
				//	N ... reset
				//	Z ... reset
				u8 f = 0;
				if ((n & B0) != 0) f = X80_C;
				cpu->f = f;
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// LD (a),SP
		case 0x08:
			{
				u16 nn = X80_ProgWord(cpu);
				cpu->writemem(nn, cpu->spl);
				nn++;
				cpu->writemem(nn, cpu->sph);
			}
			cpu->sync.clock += X80_CLOCKMUL*20;
			break;

		// ADD HL,BC
		case 0x09:
		// ADD HL,DE
		case 0x19:
		// ADD HL,HL
		case 0x29:
			{
				u16* r = &cpu->dreg[3 - (op >> 4)]; // double register
				X80_ADD16(cpu->hl, *r);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// LD A,(BC)
		case 0x0A:
			cpu->a = cpu->readmem(cpu->bc);
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// DEC BC
		case 0x0B:
			cpu->bc--;
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// RRCA
		case 0x0F:
			{
				u8 n = cpu->a;
				n = (n >> 1) | (n << 7);
				cpu->a = n;
				//	C ... carry from bit 0
				//	H ... reset
				//	N ... reset
				//	Z ... reset
				u8 f = 0;
				if ((n & B7) != 0) f = X80_C;
				cpu->f = f;
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// STOP 0
		case 0x10:
			X80_ProgByte(cpu); // read argument
			cpu->sync.clock += X80_CLOCKMUL*4;
			if (!cpu->stopins()) // callback STOP instruction
			{
				// repeat STOP instruction
				cpu->pc -= 2; // repeat
			}
			break;

		// LD (DE),A
		case 0x12:
			cpu->writemem(cpu->de, cpu->a);
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// INC DE
		case 0x13:
			cpu->de++;
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// RLA
		case 0x17:
			{
				u8 n2 = cpu->a;
				u8 n = (n2 << 1);
				if ((cpu->f & X80_C) != 0) n |= B0;
				cpu->a = n;
				//	C ... carry from bit 7
				//	H ... reset
				//	N ... reset
				//	Z ... reset
				u8 f = 0;
				if ((n2 & B7) != 0) f = X80_C;
				cpu->f = f;
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// JR e
		case 0x18:
			{
				s8 jr = (s8)X80_ProgByte(cpu); // read offset of relative jump
				cpu->pc += jr;
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// LD A,(DE)
		case 0x1A:
			cpu->a = cpu->readmem(cpu->de);
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// DEC DE
		case 0x1B:
			cpu->de--;
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// RRA
		case 0x1F:
			{
				u8 n2 = cpu->a;
				u8 n = (n2 >> 1);
				if ((cpu->f & X80_C) != 0) n |= B7;
				cpu->a = n;
				//	C ... carry from bit 0
				//	H ... reset
				//	N ... reset
				//	Z ... reset
				u8 f = 0;
				if ((n2 & B0) != 0) f = X80_C;
				cpu->f = f;
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// JR NZ,e
		case 0x20:
			{
				s8 jr = (s8)X80_ProgByte(cpu); // read offset of relative jump
				cpu->sync.clock += X80_CLOCKMUL*8;
				if ((cpu->f & X80_Z) == 0)
				{
					cpu->pc += jr;
					cpu->sync.clock += X80_CLOCKMUL*(12 - 8);
				}
			}
			break;

		// LD (HL+),A ... alternate mnemonic LD (HLI),A or LDI (HL),A
		case 0x22:
			cpu->writemem(cpu->hl, cpu->a);
			cpu->hl++;
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// INC HL
		case 0x23:
			cpu->hl++;
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// DAA
		case 0x27:
			{
				u8 f = cpu->f;
				u8 a = cpu->a;

				if ((f & X80_N) != 0) // subtraction (carry not affected)
				{
					if ((f & X80_H) != 0) a -= 6;
					if ((f & X80_C) != 0) a -= 0x60;
				}
				else // adddition (carry set or not affected)
				{
					if (((f & X80_H) != 0) || ((a & 0x0f) > 9)) a += 6;
					if (((f & X80_C) != 0) || (a > 0x9f))
					{
						a += 0x60;
						f |= X80_C;
					}
				}

				//	C ... carry (only set, never cleared)
				//	N ... not affected
				//	H ... reset
				//	Z ... set if result is 0
				f &= X80_C | X80_N;
				if (a == 0) f |= X80_Z;
				cpu->f = f;
				cpu->a = a;
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// JR Z,e
		case 0x28:
			{
				s8 jr = (s8)X80_ProgByte(cpu); // read offset of relative jump
				cpu->sync.clock += X80_CLOCKMUL*8;
				if ((cpu->f & X80_Z) != 0)
				{
					cpu->pc += jr;
					cpu->sync.clock += X80_CLOCKMUL*(12 - 8);
				}
			}
			break;

		// LD A,(HL+) ... alternate mnemonic LD A,(HLI) or LDI A,(HL)
		case 0x2A:
			cpu->a = cpu->readmem(cpu->hl);
			cpu->hl++;
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// DEC HL
		case 0x2B:
			cpu->hl--;
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// CPL
		case 0x2F:
			{
				cpu->a = ~cpu->a;
				//	C ... not affected
				//	H ... set
				//	N ... set
				//	Z ... not affected
				u8 f = cpu->f & (X80_C | X80_Z);
				f |= X80_N | X80_H;
				cpu->f = f;
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// JR NC,e
		case 0x30:
			{
				s8 jr = (s8)X80_ProgByte(cpu); // read offset of relative jump
				cpu->sync.clock += X80_CLOCKMUL*8;
				if ((cpu->f & X80_C) == 0)
				{
					cpu->pc += jr;
					cpu->sync.clock += X80_CLOCKMUL*(12 - 8);
				}
			}
			break;

		// LD SP,nn
		case 0x31:
			cpu->spl = X80_ProgByte(cpu);
			cpu->sph = X80_ProgByte(cpu);
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// LD (HL-),A ... alternate mnemonic LD (HLD),A or LDD (HL),A
		case 0x32:
			cpu->writemem(cpu->hl, cpu->a);
			cpu->hl--;
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// INC SP
		case 0x33:
			cpu->sp++;
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// INC (HL)
		case 0x34:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				X80_INC(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// DEC (HL)
		case 0x35:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				X80_DEC(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// LD (HL),n
		case 0x36:
			cpu->writemem(cpu->hl, X80_ProgByte(cpu));
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// SCF
		case 0x37:
			{
				//	C ... set
				//	H ... reset
				//	N ... reset
				//	Z ... not affected
				u8 f = cpu->f & X80_Z;
				f |= X80_C;
				cpu->f = f;
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// JR C,e
		case 0x38:
			{
				s8 jr = (s8)X80_ProgByte(cpu); // read offset of relative jump
				cpu->sync.clock += X80_CLOCKMUL*8;
				if ((cpu->f & X80_C) != 0)
				{
					cpu->pc += jr;
					cpu->sync.clock += X80_CLOCKMUL*(12 - 8);
				}
			}
			break;

		// ADD HL,SP
		case 0x39:
			X80_ADD16(cpu->hl, cpu->sp);
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// LD A,(HL-) ... alternate mnemonic LD A,(HLD) or LDD A,(HL)
		case 0x3A:
			cpu->a = cpu->readmem(cpu->hl);
			cpu->hl--;
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// DEC SP
		case 0x3B:
			cpu->sp--;
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// CCF
		case 0x3F:
			{
				//	C ... inverted
				//	H ... reset
				//	N ... reset
				//	Z ... not affected
				u8 f = cpu->f ^ X80_C;
				f &= X80_Z | X80_C;
				cpu->f = f;
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
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
			cpu->sync.clock += X80_CLOCKMUL*4;
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
			cpu->sync.clock += X80_CLOCKMUL*8;
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
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// HALT
		case 0x76:
			cpu->pc--;
			cpu->halted = 1;
			cpu->sync.clock += X80_CLOCKMUL*4;
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
				X80_ADD(*rs);
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// ADD A,(HL)
		case 0x86:
			{
				u8 n = cpu->readmem(cpu->hl);
				X80_ADD(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
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
				X80_ADC(*rs);
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// ADC A,(HL)
		case 0x8E:
			{
				u8 n = cpu->readmem(cpu->hl);
				X80_ADC(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
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
				X80_SUB(*rs);
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// SUB (HL)
		case 0x96:
			{
				u8 n = cpu->readmem(cpu->hl);
				X80_SUB(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
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
				X80_SBC(*rs);
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// SBC A,(HL)
		case 0x9E:
			{
				u8 n = cpu->readmem(cpu->hl);
				X80_SBC(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
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
				X80_AND(*rs);
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// AND (HL)
		case 0xA6:
			{
				u8 n = cpu->readmem(cpu->hl);
				X80_AND(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
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
				X80_XOR(*rs);
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// XOR (HL)
		case 0xAE:
			{
				u8 n = cpu->readmem(cpu->hl);
				X80_XOR(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
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
				X80_OR(*rs);
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// OR (HL)
		case 0xB6:
			{
				u8 n = cpu->readmem(cpu->hl);
				X80_OR(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
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
				X80_CP(*rs);
			}
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// CP (HL)
		case 0xBE:
			{
				u8 n = cpu->readmem(cpu->hl);
				X80_CP(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// RET NZ
		case 0xC0:
			if ((cpu->f & X80_Z) == 0)
			{
				X80_RET();
				cpu->sync.clock += X80_CLOCKMUL*(20 - 8);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// POP BC
		case 0xC1:
		// POP DE
		case 0xD1:
		// POP HL
		case 0xE1:
			{
				u8* r = &cpu->reg[6 - ((op - 0xc1) >> 3)]; // register low
				X80_POP(r[0], r[1]);
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// JP NZ,a
		case 0xC2:
			{
				// read destination address -> nn
				u16 nn = X80_ProgWord(cpu);

				// check condition
				if ((cpu->f & X80_Z) == 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += X80_CLOCKMUL*(16-12);
				}
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// JP a
		case 0xC3:
			cpu->pc = X80_ProgWord(cpu);
			cpu->sync.clock += X80_CLOCKMUL*16;
			break;

		// CALL NZ,a
		case 0xC4:
			{
				// read destination address -> nn
				u16 nn = X80_ProgWord(cpu);

				// check condition
				if ((cpu->f & X80_Z) == 0)
				{
					X80_CALL(nn);
					cpu->sync.clock += X80_CLOCKMUL*(24 - 12);
				}
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// PUSH BC
		case 0xC5:
		// PUSH DE
		case 0xD5:
		// PUSH HL
		case 0xE5:
			{
				u8* r = &cpu->reg[6 - ((op - 0xc5) >> 3)]; // register low
				X80_PUSH(r[0], r[1]);
			}
			cpu->sync.clock += X80_CLOCKMUL*16;
			break;

		// ADD A,d
		case 0xC6:
			{
				u8 n = X80_ProgByte(cpu);
				X80_ADD(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
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
			X80_CALL(op & 0x38);
			cpu->sync.clock += X80_CLOCKMUL*16;
			break;

		// RET Z
		case 0xC8:
			if ((cpu->f & X80_Z) != 0)
			{
				X80_RET();
				cpu->sync.clock += X80_CLOCKMUL*(20 - 8);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// RET
		case 0xC9:
			X80_RET();
			cpu->sync.clock += X80_CLOCKMUL*16;
			break;

		// JP Z,a
		case 0xCA:
			{
				// read destination address -> nn
				u16 nn = X80_ProgWord(cpu);

				// check condition
				if ((cpu->f & X80_Z) != 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += X80_CLOCKMUL*(16-12);
				}
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// prefix CB
		case 0xCB:
			{
				op = X80_ProgByte(cpu); // read next program byte
				u8 low = op & 0x07; // low 3 bits of the program byte
				op >>= 3; // high 5 bits of the program byte
#include "emu_x80_cb.c"
			}
			break;

		// CALL Z,a
		case 0xCC:
			{
				// read destination address -> nn
				u16 nn = X80_ProgWord(cpu);

				// check condition
				if ((cpu->f & X80_Z) != 0)
				{
					X80_CALL(nn);
					cpu->sync.clock += X80_CLOCKMUL*(24 - 12);
				}
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// CALL a
		case 0xCD:
			{
				u16 nn = X80_ProgWord(cpu);
				X80_CALL(nn);
			}
			cpu->sync.clock += X80_CLOCKMUL*24;
			break;

		// ADC A,d
		case 0xCE:
			{
				u8 n = X80_ProgByte(cpu);
				X80_ADC(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// RET NC
		case 0xD0:
			if ((cpu->f & X80_C) == 0)
			{
				X80_RET();
				cpu->sync.clock += X80_CLOCKMUL*(20 - 8);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// JP NC,a
		case 0xD2:
			{
				// read destination address -> nn
				u16 nn = X80_ProgWord(cpu);

				// check condition
				if ((cpu->f & X80_C) == 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += X80_CLOCKMUL*(16-12);
				}
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// CALL NC,a
		case 0xD4:
			{
				// read destination address -> nn
				u16 nn = X80_ProgWord(cpu);

				// check condition
				if ((cpu->f & X80_C) == 0)
				{
					X80_CALL(nn);
					cpu->sync.clock += X80_CLOCKMUL*(24 - 12);
				}
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// SUB d
		case 0xD6:
			{
				u8 n = X80_ProgByte(cpu);
				X80_SUB(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// RET C
		case 0xD8:
			if ((cpu->f & X80_C) != 0)
			{
				X80_RET();
				cpu->sync.clock += X80_CLOCKMUL*(20 - 8);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// RETI
		case 0xD9:
			X80_RET();
			cpu->ime = 1; // enable interrupts
			cpu->sync.clock += X80_CLOCKMUL*16;
			break;

		// JP C,a
		case 0xDA:
			{
				// read destination address -> nn
				u16 nn = X80_ProgWord(cpu);

				// check condition
				if ((cpu->f & X80_C) != 0)
				{
					cpu->pc = nn;
					cpu->sync.clock += X80_CLOCKMUL*(16-12);
				}
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// CALL C,a
		case 0xDC:
			{
				// read destination address -> nn
				u16 nn = X80_ProgWord(cpu);

				// check condition
				if ((cpu->f & X80_C) != 0)
				{
					X80_CALL(nn);
					cpu->sync.clock += X80_CLOCKMUL*(24 - 12);
				}
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// SBC A,d
		case 0xDE:
			{
				u8 n = X80_ProgByte(cpu);
				X80_SBC(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;


		// LDH (a8),A ... alternate mnemonic LD ($FF00+a8),A
		case 0xE0:
			{
				u8 n = X80_ProgByte(cpu);
				cpu->writemem(n + X80_PORTBASE, cpu->a);
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// LD (C),A ... alternate mnemonic LD ($FF00+C),A
		case 0xE2:
			cpu->writemem(cpu->c + X80_PORTBASE, cpu->a);
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// AND d
		case 0xE6:
			{
				u8 n = X80_ProgByte(cpu);
				X80_AND(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// ADD SP,r8
		case 0xE8:
			{
				u16 n = (u16)(s16)(s8)X80_ProgByte(cpu);
				u16 sp = cpu->sp;
				//	C ... carry from bit 7
				//	H ... carry from bit 3
				//	N ... reset
				//	Z ... reset
				u16 res = sp + n;
				cpu->sp = res;
				res = res ^ sp ^ n;
				u8 f = 0;
				if ((res & B4) != 0) f = X80_H;
				if ((res & B8) != 0) f |= X80_C;
				cpu->f = f;
			}
			cpu->sync.clock += X80_CLOCKMUL*16;
			break;

		// JP (HL)
		case 0xE9:
			cpu->pc = cpu->hl;
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// LD (a16),A
		case 0xEA:
			{
				u16 n = X80_ProgWord(cpu);
				cpu->writemem(n, cpu->a);
			}
			cpu->sync.clock += X80_CLOCKMUL*16;
			break;

		// XOR d
		case 0xEE:
			{
				u8 n = X80_ProgByte(cpu);
				X80_XOR(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// LDH A,(a8) ... alternate mnemonic LD A,($FF00+a8)
		case 0xF0:
			{
				u8 n = X80_ProgByte(cpu);
				cpu->a = cpu->readmem(n + X80_PORTBASE);
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// POP AF
		case 0xF1:
			X80_POP(cpu->f, cpu->a);
			cpu->f &= X80_FLAGALL;
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// LD A,(C) ... alternate mnemonic LD A,($FF00+C)
		case 0xF2:
			cpu->a = cpu->readmem(cpu->c + X80_PORTBASE);
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// DI
		case 0xF3:
			cpu->ime = 0;
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// PUSH AF
		case 0xF5:
			cpu->f &= X80_FLAGALL;
			X80_PUSH(cpu->f, cpu->a);
			cpu->sync.clock += X80_CLOCKMUL*16;
			break;

		// OR d
		case 0xF6:
			{
				u8 n = X80_ProgByte(cpu);
				X80_OR(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// LD HL,SP+r8 ... alternative mnemonic LDHL SP,r8
		case 0xF8:
			{
				u16 n = (u16)(s16)(s8)X80_ProgByte(cpu);
				u16 sp = cpu->sp;
				//	C ... carry from bit 7
				//	H ... carry from bit 3
				//	N ... reset
				//	Z ... reset
				u16 res = sp + n;
				cpu->hl = res;
				res = res ^ sp ^ n;
				u8 f = 0;
				if ((res & B4) != 0) f = X80_H;
				if ((res & B8) != 0) f |= X80_C;
				cpu->f = f;
			}
			cpu->sync.clock += X80_CLOCKMUL*12;
			break;

		// LD SP,HL
		case 0xF9:
			cpu->sp = cpu->hl;
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;

		// LD A,(a16)
		case 0xFA:
			{
				u16 n = X80_ProgWord(cpu);
				cpu->a = cpu->readmem(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*16;
			break;

		// EI
		case 0xFB:
			cpu->ime = 1;
			cpu->sync.clock += X80_CLOCKMUL*4;
			break;

		// CP d
		case 0xFE:
			{
				u8 n = X80_ProgByte(cpu);
				X80_CP(n);
			}
			cpu->sync.clock += X80_CLOCKMUL*8;
			break;
		}

		// update DIV counter
		olddiv = cpu->div; // old DIV counter
		newdiv = (u16)((cpu->sync.clock - oldclock)/X80_CLOCKMUL); // clock increment
		cpu->div = olddiv + newdiv; // new divider

		// update LCD scanline time counter
		cpu->lcd_count += newdiv; // shift LCD scanline time counter
		if (cpu->lcd_count >= cpu->lcd_cycles)
		{
			cpu->lcd_count -= cpu->lcd_cycles;
			cpu->lcd_mode = X80_LCD_MODE0;
			cpu->lcdline();
		}
		else if ((cpu->lcd_mode == X80_LCD_MODE0) && (cpu->lcd_count >= cpu->lcd_cycles2))
		{
			cpu->lcd_mode = X80_LCD_MODE2;
			cpu->lcdoam(); // start OAM-RAM access
		}
		else if ((cpu->lcd_mode == X80_LCD_MODE2) && (cpu->lcd_count >= cpu->lcd_cycles3))
		{
			cpu->lcd_mode = X80_LCD_MODE3;
			cpu->lcdtrans(); // start transfer
		}

		// update timer
		divshift = cpu->divshift; // shift of mask
		if (divshift != 255)	// is TIMA counting?
		{
			olddiv &= BIT(divshift) - 1;	// mask lower bits of divider
			newdiv += olddiv;	// new divider
			newdiv >>= divshift; // get TIMA increment
			for (; newdiv > 0; newdiv--)
			{
				cpu->tima++;
				if (cpu->tima == 0)
				{
					cpu->tima = cpu->tma; // setup new modulo timer
					X80_RaiseIRQ(cpu, X80_IRQ_TIMER); // raise interrupt
				}
			}
		}

		// time synchronization
		EmuSync(&cpu->sync);
	}
}

// execute X80 emulator on CPU 1
static void X80_ExecCore1()
{
	// start time synchronization
	X80_SyncStart((sX80*)X80_Cpu);

	// execute
	X80_Exec((sX80*)X80_Cpu);

	// clear descriptor
	X80_Cpu = NULL;
	dsb();
}

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*X80_CLOCKMUL is recommended to maintain.
// Emulation can be stopped by X80_Stop() function or by STOP 0 instruction.
u32 X80_Start(sX80* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (X80_Cpu != NULL) X80_Stop(pwm);

	// initialize time synchronization
	freq = X80_SyncInit(cpu, pwm, freq);

	// reset processor
	X80_Reset(cpu);

	// current CPU descriptor
	X80_Cpu = cpu;
	dsb();

	// execute program
#if USE_MINIVGA				// use mini-VGA display
	VgaCore1Exec(X80_ExecCore1);
#else
	Core1Exec(X80_ExecCore1);
#endif

	return freq;
}

// continue emulation without reset processor
u32 X80_Cont(sX80* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (X80_Cpu != NULL) X80_Stop(pwm);

	// initialize time synchronization
	freq = X80_SyncInit(cpu, pwm, freq);

	// current CPU descriptor
	X80_Cpu = cpu;
	dsb();

	// execute program
#if USE_MINIVGA				// use mini-VGA display
	VgaCore1Exec(X80_ExecCore1);
#else
	Core1Exec(X80_ExecCore1);
#endif

	return freq;
}

// stop emulation (can be also activated by STOP 0 instruction)
//  pwm ... index of used PWM slice (0..7)
void X80_Stop(int pwm)
{
	// get CPU descriptor
	sX80* cpu = (sX80*)X80_Cpu;

	// check if emulation is running
	if (cpu == NULL) return;

	// request to stop emulation
	cpu->stop = 1;

	// wait to complete
	while (X80_Cpu != NULL) { dsb(); }

	// terminate time synchronization (stop PWM counter)
	X80_SyncTerm(pwm);

#if !USE_MINIVGA				// use mini-VGA display
	// core 1 reset
	Core1Reset();
#endif
}

#endif // USE_EMU_X80
