
// ****************************************************************************
//
//                               I8052 CPU Emulator
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

// @TODO: interrupts, timers, hardware

#if USE_EMU_I8052	// use I8052 CPU emulator

#include "emu_i8052_macro.h"

// current CPU descriptor (NULL = not running)
volatile sI8052* I8052_Cpu = NULL;

// parity table (I8052_P set if odd parity) ... keep the table in RAM, for higher speed
u8 I8052_ParTab[256];

// initialize I8052 tables
void I8052_InitTab()
{
	int i, j, par;
	u8 f;
	for (i = 0; i < 256; i++)
	{
		par = 0;
		j = i;
		while (j != 0)
		{
			par ^= (j & 1);
			j >>= 1;
		}
		I8052_ParTab[i] = (par == 0) ? 0 : I8052_P; // I8052_P set if odd parity
	}
}

// direct read data memory or SFR registers
u8 FASTCODE NOFLASH(I8052_GetDirect)(sI8052* cpu, u8 addr)
{
	// read base memory
	if (addr < 0x80) return cpu->ram[addr];

	// read port
	if (((addr & 0x0f) == 0) && (addr <= 0xb0))
	{
		addr -= 0x80;
		addr >>= 4;
		return cpu->readport(addr);
	}

	// read SFR register
	return cpu->sfr[addr - I8052_SFR_BASE];
}

// direct read data memory or SFR registers latched
u8 FASTCODE NOFLASH(I8052_GetDirectLatch)(sI8052* cpu, u8 addr)
{
	// read base memory
	if (addr < 0x80) return cpu->ram[addr];

	// latched port
	if (((addr & 0x0f) == 0) && (addr <= 0xb0))
	{
		addr -= 0x80;
		addr >>= 4;
		return cpu->lastport[addr];
	}

	// read SFR register
	return cpu->sfr[addr - I8052_SFR_BASE];
}

// direct write data memory or SFR registers
void FASTCODE NOFLASH(I8052_SetDirect)(sI8052* cpu, u8 addr, u8 data)
{
	// write base memory
	if (addr < 0x80)
		cpu->ram[addr] = data;
	else
	{
		// write port
		if (((addr & 0x0f) == 0) && (addr <= 0xb0))
		{
			addr -= 0x80;
			addr >>= 4;
			cpu->lastport[addr] = data;
			cpu->writeport(addr, data);
		}
		else
		{
			addr -= I8052_SFR_BASE;
			cpu->sfr[addr] = data;

			// parity bit correction
			if ((addr == (I8052_ADDR_ACC - I8052_SFR_BASE)) ||
				(addr == (I8052_ADDR_PSW - I8052_SFR_BASE)))
					U8052_PARITY_UPDATE();
		}
	}
}

// get bit (returns 0 or 1)
u8 FASTCODE NOFLASH(I8052_GetBit)(sI8052* cpu, u8 addr)
{
	int a, b;
	u8 d;

	// bits 0x00..0x7F (RAM address 0x20..0x2F)
	if (addr < 0x80)
	{
		// byte address
		a = (addr >> 3) + 0x20;

		// read RAM
		d = cpu->ram[a];
	}
	else
	{
		// SFR registers - 0x80
		a = addr & 0x78;

		// read port or SFR
		if (((a & 0x0f) == 0) && (a <= 0xb0 - 0x80))
			d = cpu->readport(a >> 4);
		else
			d = cpu->sfr[a];
	}

	// get bit
	b = addr & 7; // bit position
	return (d >> b) & 1;
}

// get bit latched (returns 0 or 1)
u8 FASTCODE NOFLASH(I8052_GetBitLatch)(sI8052* cpu, u8 addr)
{
	int a, b;
	u8 d;

	// bits 0x00..0x7F (RAM address 0x20..0x2F)
	if (addr < 0x80)
	{
		// byte address
		a = (addr >> 3) + 0x20;

		// read RAM
		d = cpu->ram[a];
	}
	else
	{
		// SFR registers - 0x80
		a = addr & 0x78;

		// read port or SFR
		if (((a & 0x0f) == 0) && (a <= 0xb0 - 0x80))
			d = cpu->lastport[a >> 4];
		else
			d = cpu->sfr[a];
	}

	// get bit
	b = addr & 7; // bit position
	return (d >> b) & 1;
}

// set bit (value==0 or value!=0)
void FASTCODE NOFLASH(I8052_SetBit)(sI8052* cpu, u8 addr, u8 val)
{
	int a, b, m;
	u8 d;
	u8* s;

	// bit mask
	b = addr & 7; // bit position
	m = BIT(b); // bit mask

	// bits 0x00..0x7F (RAM address 0x20..0x2F)
	if (addr < 0x80)
	{
		// byte address
		a = (addr >> 3) + 0x20;

		// address in RAM
		s = &cpu->ram[a];

		// set data in RAM
		d = *s;
		d |= m;
		if (val == 0) d ^= m;
		*s = d;
	}
	else
	{
		// SFR registers - 0x80
		a = addr & 0x78;

		// write port
		if (((a & 0x0f) == 0) && (a <= 0xb0 - 0x80))
		{
			a >>= 4;
			s = &cpu->lastport[a];
			d = *s;
			d |= m;
			if (val == 0) d ^= m;
			*s = d;
			cpu->writeport(a, d);
		}
		else
		{
			// address in SFR
			s = &cpu->sfr[a];

			// set data in SFR
			d = *s;
			d |= m;
			if (val == 0) d ^= m;
			*s = d;

			// parity bit correction
			if ((a == (I8052_ADDR_ACC - I8052_SFR_BASE)) ||
				(a == (I8052_ADDR_PSW - I8052_SFR_BASE)))
					U8052_PARITY_UPDATE();
		}
	}
}

// reset processor
void I8052_Reset(sI8052* cpu)
{
	int i;

	// reset program counter
	cpu->pc = 0;

	cpu->inirq = 0; // not in interrupt

	// clear RAM memory (clear Rx registers)
	for (i = 0; i < I8052_RAMSIZE; i++) cpu->ram[i] = 0;

	// initialize SFR registers
	for (i = 0; i < 128; i++) cpu->sfr[i] = 0;
	I8052_SP = 7;
	I8052_SetDirect(cpu, 0, 0xff);
	I8052_SetDirect(cpu, 1, 0xff);
	I8052_SetDirect(cpu, 2, 0xff);
	I8052_SetDirect(cpu, 3, 0xff);
}

// load byte from program memory
INLINE u8 I8052_ProgByte(sI8052* cpu)
{
	// read program byte
	u8 n = cpu->readrom(cpu->pc);

	// compiler barrier (for the PC to be valid at the time of the callback instruction)
	cb();

	// shift program counter
	cpu->pc++;

	return n;
}

// execute program (start or continue, until "stop" request)
// Size of code of this function: 6620 code + 1024 jump table = 7644 bytes
// CPU loading at 12 MHz on 240 MHz: used 20-37%, max. 66-83%
void FASTCODE NOFLASH(I8052_Exec)(sI8052* cpu)
{
	u8 op;

	// clear stop flag
	cpu->stop = 0;

	// program loop
	while (cpu->stop == 0)
	{

// @TODO: interrupts, timers, hardware

		// cpu->inirq = I8052_IRQ_LEVEL_LOW; // current interrupt level


		// get next instruction
		op = I8052_ProgByte(cpu);

		// switch base operation code
		switch (op)
		{
		// NOP
		case 0x00:
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// AJMP 0xx
		case 0x01:
		// AJMP 1xx
		case 0x21:
		// AJMP 2xx
		case 0x41:
		// AJMP 3xx
		case 0x61:
		// AJMP 4xx
		case 0x81:
		// AJMP 5xx
		case 0xA1:
		// AJMP 6xx
		case 0xC1:
		// AJMP 7xx
		case 0xE1:
			{
				// read address LOW
				u8 a = I8052_ProgByte(cpu);

				// jump to destination address
				cpu->pc = (cpu->pc & 0xf800) | ((u16)(op - 0x01) << 3) | a;
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// LJMP a
		case 0x02:
			{
				// read address HIGH
				u8 hi = I8052_ProgByte(cpu);

				// read address LOW
				u8 lo = I8052_ProgByte(cpu);

				// jump to destination address
				cpu->pc = ((u16)hi << 8) | lo;
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// RR A
		case 0x03:
			{
				u8 a = I8052_ACC;
				a = (a >> 1) | (a << 7);
				I8052_ACC = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// INC A
		case 0x04:
			{
				u8 a = I8052_ACC + 1;
				I8052_ACC = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// INC direct
		case 0x05:
			{
				u8 a = I8052_ProgByte(cpu);
				u8 r = I8052_GetDirectLatch(cpu, a) + 1;
				I8052_SetDirect(cpu, a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// INC @R0
		case 0x06:
		// INC @R1
		case 0x07:
			{
				u8 r = I8052_REG(op - 0x06);
				u8* s = &cpu->ram[r];
				*s = *s + 1;
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// INC R0
		case 0x08:
		// INC R1
		case 0x09:
		// INC R2
		case 0x0A:
		// INC R3
		case 0x0B:
		// INC R4
		case 0x0C:
		// INC R5
		case 0x0D:
		// INC R6
		case 0x0E:
		// INC R7
		case 0x0F:
			{
				u8* s = &I8052_REG(op - 0x08);
				*s = *s + 1;
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;


		// JBC bit,rel
		case 0x10:
			{
				u8 a = I8052_ProgByte(cpu);	// get bit address
				a = I8052_GetBitLatch(cpu, a);	// get current value of the bit
				s8 rel = (s8)I8052_ProgByte(cpu); // get relativne jump
				if (a != 0)
				{
					cpu->pc += (s8)rel;	// jump if bit is set
					I8052_SetBit(cpu, a, 0); // clear bit
				}
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// ACALL 0xx
		case 0x11:
		// ACALL 1xx
		case 0x31:
		// ACALL 2xx
		case 0x51:
		// ACALL 3xx
		case 0x71:
		// ACALL 4xx
		case 0x91:
		// ACALL 5xx
		case 0xB1:
		// ACALL 6xx
		case 0xD1:
		// ACALL 7xx
		case 0xF1:
			{
				// read address LOW
				u8 a = I8052_ProgByte(cpu);

				// push current PC into stack
				u16 pc = cpu->pc;
				I8052_PUSH16(pc);

				// jump to destination address
				cpu->pc = (pc & 0xf800) | ((u16)(op - 0x11) << 3) | a;
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// LCALL a
		case 0x12:
			{
				// read address HIGH
				u8 hi = I8052_ProgByte(cpu);

				// read address LOW
				u8 lo = I8052_ProgByte(cpu);

				// push current PC into stack
				I8052_PUSH16(cpu->pc);

				// jump to destination address
				cpu->pc = ((u16)hi << 8) | lo;
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// RRC A
		case 0x13:
			{
				u8 psw = I8052_PSW;
				u8 a = I8052_ACC;
				I8052_PSW = (psw & ~I8052_C) | (a << 7);
				a = (a >> 1) | (psw & B7);
				I8052_ACC = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// DEC A
		case 0x14:
			{
				u8 a = I8052_ACC - 1;
				I8052_ACC = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// DEC direct
		case 0x15:
			{
				u8 a = I8052_ProgByte(cpu);
				u8 r = I8052_GetDirectLatch(cpu, a) - 1;
				I8052_SetDirect(cpu, a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// DEC @R0
		case 0x16:
		// DEC @R1
		case 0x17:
			{
				u8 r = I8052_REG(op - 0x16);
				u8* s = &cpu->ram[r];
				*s = *s - 1;
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// DEC R0
		case 0x18:
		// DEC R1
		case 0x19:
		// DEC R2
		case 0x1A:
		// DEC R3
		case 0x1B:
		// DEC R4
		case 0x1C:
		// DEC R5
		case 0x1D:
		// DEC R6
		case 0x1E:
		// DEC R7
		case 0x1F:
			{
				u8* s = &I8052_REG(op - 0x18);
				*s = *s - 1;
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// JB bit,rel
		case 0x20:
			{
				u8 a = I8052_ProgByte(cpu);	// get bit address
				a = I8052_GetBit(cpu, a);	// get current value of the bit
				s8 rel = (s8)I8052_ProgByte(cpu); // get relativne jump
				if (a != 0) cpu->pc += (s8)rel;	// jump if bit is set
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// RET
		case 0x22:
			I8052_POP16(cpu->pc);
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// RL A
		case 0x23:
			{
				u8 a = I8052_ACC;
				a = (a << 1) | (a >> 7);
				I8052_ACC = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ADD A,#n
		case 0x24:
			{
				u8 r = I8052_ProgByte(cpu);
				I8052_ADD(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ADD A,direct
		case 0x25:
			{
				u8 r = I8052_GetDirect(cpu, I8052_ProgByte(cpu));
				I8052_ADD(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ADD A,@R0
		case 0x26:
		// ADD A,@R1
		case 0x27:
			{
				u8 r = I8052_REG(op - 0x26);
				r = cpu->ram[r];
				I8052_ADD(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ADD A,R0
		case 0x28:
		// ADD A,R1
		case 0x29:
		// ADD A,R2
		case 0x2A:
		// ADD A,R3
		case 0x2B:
		// ADD A,R4
		case 0x2C:
		// ADD A,R5
		case 0x2D:
		// ADD A,R6
		case 0x2E:
		// ADD A,R7
		case 0x2F:
			{
				u8 r = I8052_REG(op - 0x28);
				I8052_ADD(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// JNB bit,rel
		case 0x30:
			{
				u8 a = I8052_ProgByte(cpu);	// get bit address
				a = I8052_GetBit(cpu, a);	// get current value of the bit
				s8 rel = (s8)I8052_ProgByte(cpu); // get relativne jump
				if (a == 0) cpu->pc += (s8)rel;	// jump if bit is not set
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// RETI
		case 0x32:
			I8052_POP16(cpu->pc);
			if (cpu->inirq > 0) cpu->inirq--; // decrease IRQ level
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// RLC A
		case 0x33:
			{
				u8 psw = I8052_PSW;
				u8 a = I8052_ACC;
				I8052_PSW = (psw & ~I8052_C) | (a & B7);
				a = (a << 1) | (psw >> 7);
				I8052_ACC = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ADDC A,#n
		case 0x34:
			{
				u8 r = I8052_ProgByte(cpu);
				I8052_ADDC(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ADDC A,direct
		case 0x35:
			{
				u8 r = I8052_GetDirect(cpu, I8052_ProgByte(cpu));
				I8052_ADDC(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ADDC A,@R0
		case 0x36:
		// ADDC A,@R1
		case 0x37:
			{
				u8 r = I8052_REG(op - 0x36);
				r = cpu->ram[r];
				I8052_ADDC(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ADDC A,R0
		case 0x38:
		// ADDC A,R1
		case 0x39:
		// ADDC A,R2
		case 0x3A:
		// ADDC A,R3
		case 0x3B:
		// ADDC A,R4
		case 0x3C:
		// ADDC A,R5
		case 0x3D:
		// ADDC A,R6
		case 0x3E:
		// ADDC A,R7
		case 0x3F:
			{
				u8 r = I8052_REG(op - 0x38);
				I8052_ADDC(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// JC rel
		case 0x40:
			{
				s8 rel = (s8)I8052_ProgByte(cpu); // get relativne jump
				if ((I8052_PSW & I8052_C) != 0) cpu->pc += (s8)rel; // jump if carry is set
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// ORL direct,A
		case 0x42:
			{
				u8 a = I8052_ProgByte(cpu);
				u8 r = I8052_GetDirectLatch(cpu, a);
				r |= I8052_ACC;
				I8052_SetDirect(cpu, a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ORL direct,#n
		case 0x43:
			{
				u8 a = I8052_ProgByte(cpu); // address
				u8 n = I8052_ProgByte(cpu); // data
				u8 r = I8052_GetDirectLatch(cpu, a);
				r |= n;
				I8052_SetDirect(cpu, a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// ORL A,#n
		case 0x44:
			{
				u8 r = I8052_ProgByte(cpu);
				I8052_ORL_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ORL A,direct
		case 0x45:
			{
				u8 r = I8052_GetDirect(cpu, I8052_ProgByte(cpu));
				I8052_ORL_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ORL A,@R0
		case 0x46:
		// ORL A,@R1
		case 0x47:
			{
				u8 r = I8052_REG(op - 0x46);
				r = cpu->ram[r];
				I8052_ORL_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ORL A,R0
		case 0x48:
		// ORL A,R1
		case 0x49:
		// ORL A,R2
		case 0x4A:
		// ORL A,R3
		case 0x4B:
		// ORL A,R4
		case 0x4C:
		// ORL A,R5
		case 0x4D:
		// ORL A,R6
		case 0x4E:
		// ORL A,R7
		case 0x4F:
			{
				u8 r = I8052_REG(op - 0x48);
				I8052_ORL_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// JNC rel
		case 0x50:
			{
				s8 rel = (s8)I8052_ProgByte(cpu); // get relativne jump
				if ((I8052_PSW & I8052_C) == 0) cpu->pc += (s8)rel; // jump if carry is not set
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// ANL direct,A
		case 0x52:
			{
				u8 a = I8052_ProgByte(cpu);
				u8 r = I8052_GetDirectLatch(cpu, a);
				r &= I8052_ACC;
				I8052_SetDirect(cpu, a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ANL direct,#n
		case 0x53:
			{
				u8 a = I8052_ProgByte(cpu); // address
				u8 n = I8052_ProgByte(cpu); // data
				u8 r = I8052_GetDirectLatch(cpu, a);
				r &= n;
				I8052_SetDirect(cpu, a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// ANL A,#n
		case 0x54:
			{
				u8 r = I8052_ProgByte(cpu);
				I8052_ANL_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ANL A,direct
		case 0x55:
			{
				u8 r = I8052_GetDirect(cpu, I8052_ProgByte(cpu));
				I8052_ANL_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ANL A,@R0
		case 0x56:
		// ANL A,@R1
		case 0x57:
			{
				u8 r = I8052_REG(op - 0x56);
				r = cpu->ram[r];
				I8052_ANL_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ANL A,R0
		case 0x58:
		// ANL A,R1
		case 0x59:
		// ANL A,R2
		case 0x5A:
		// ANL A,R3
		case 0x5B:
		// ANL A,R4
		case 0x5C:
		// ANL A,R5
		case 0x5D:
		// ANL A,R6
		case 0x5E:
		// ANL A,R7
		case 0x5F:
			{
				u8 r = I8052_REG(op - 0x58);
				I8052_ANL_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// JZ rel
		case 0x60:
			{
				s8 rel = (s8)I8052_ProgByte(cpu); // get relativne jump
				if (I8052_ACC == 0) cpu->pc += (s8)rel; // jump if accumulator is zero
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// XRL direct,A
		case 0x62:
			{
				u8 a = I8052_ProgByte(cpu);
				u8 r = I8052_GetDirectLatch(cpu, a);
				r ^= I8052_ACC;
				I8052_SetDirect(cpu, a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// XRL direct,#n
		case 0x63:
			{
				u8 a = I8052_ProgByte(cpu); // address
				u8 n = I8052_ProgByte(cpu); // data
				u8 r = I8052_GetDirectLatch(cpu, a);
				r ^= n;
				I8052_SetDirect(cpu, a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// XRL A,#n
		case 0x64:
			{
				u8 r = I8052_ProgByte(cpu);
				I8052_XRL_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// XRL A,direct
		case 0x65:
			{
				u8 r = I8052_GetDirect(cpu, I8052_ProgByte(cpu));
				I8052_XRL_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// XRL A,@R0
		case 0x66:
		// XRL A,@R1
		case 0x67:
			{
				u8 r = I8052_REG(op - 0x66);
				r = cpu->ram[r];
				I8052_XRL_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// XRL A,R0
		case 0x68:
		// XRL A,R1
		case 0x69:
		// XRL A,R2
		case 0x6A:
		// XRL A,R3
		case 0x6B:
		// XRL A,R4
		case 0x6C:
		// XRL A,R5
		case 0x6D:
		// XRL A,R6
		case 0x6E:
		// XRL A,R7
		case 0x6F:
			{
				u8 r = I8052_REG(op - 0x68);
				I8052_XRL_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// JNZ rel
		case 0x70:
			{
				s8 rel = (s8)I8052_ProgByte(cpu); // get relativne jump
				if (I8052_ACC != 0) cpu->pc += (s8)rel; // jump if accumulator is not zero
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// ORL C,bit
		case 0x72:
			{
				u8 a = I8052_ProgByte(cpu);	// bit address
				a = I8052_GetBit(cpu, a);	// get current value of the bit
				if (a != 0) I8052_PSW |= I8052_C; // set carry
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// JMP @A+DPTR
		case 0x73:
			cpu->pc = I8052_ACC + I8052_DPTR;
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOV A,#n
		case 0x74:
			{
				u8 a = I8052_ProgByte(cpu); // data
				I8052_ACC = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// MOV direct,#n
		case 0x75:
			{
				u8 a = I8052_ProgByte(cpu); // address
				u8 r = I8052_ProgByte(cpu); // data
				I8052_SetDirect(cpu, a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOV @R0,#n
		case 0x76:
		// MOV @R1,#n
		case 0x77:
			{
				u8 a = I8052_REG(op - 0x76); // address
				u8 r = I8052_ProgByte(cpu); // data
				cpu->ram[a] = r;
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// MOV R0,#n
		case 0x78:
		// MOV R1,#n
		case 0x79:
		// MOV R2,#n
		case 0x7A:
		// MOV R3,#n
		case 0x7B:
		// MOV R4,#n
		case 0x7C:
		// MOV R5,#n
		case 0x7D:
		// MOV R6,#n
		case 0x7E:
		// MOV R7,#n
		case 0x7F:
			{
				u8 a = I8052_ProgByte(cpu); // data
				I8052_REG(op - 0x78) = a;
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// SJMP a
		case 0x80:
			{
				s8 rel = (s8)I8052_ProgByte(cpu); // get relativne jump
				cpu->pc += (s8)rel; // jump
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// ANL C,bit
		case 0x82:
			{
				u8 a = I8052_ProgByte(cpu);	// bit address
				a = I8052_GetBit(cpu, a);	// get current value of the bit
				if (a == 0) I8052_PSW &= ~I8052_C; // reset carry
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOVC A,@A+PC
		case 0x83:
			{
				u16 a = I8052_ACC + cpu->pc;	// address
				u8 r = cpu->readrom(a);		// read byte from ROM
				I8052_ACC = r;
				U8052_PARITY_UPDATE_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// DIV AB
		case 0x84:
			{
				u8 psw = I8052_PSW & ~(I8052_C | I8052_OV); // clear carry and overflow
				u8 b = I8052_B;
				if (b == 0)
					psw |= I8052_OV; // set overflow flag on divide by 0
				else
				{
					u8 a = I8052_ACC;
					u8 quot = a / b; // quotient
					u8 rem = a - quot*b; // remainder
					I8052_ACC = quot; // ACC gets quotient
					I8052_B = rem; // B gets remainder
					U8052_PARITY_UPDATE_A(quot);
				}
				I8052_PSW = psw;
			}
			cpu->sync.clock += I8052_CLOCKMUL*48;
			break;

		// MOV direct,direct
		case 0x85:
			{
				u8 src = I8052_ProgByte(cpu);
				u8 dst = I8052_ProgByte(cpu);
				u8 r = I8052_GetDirect(cpu, src);
				I8052_SetDirect(cpu, dst, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOV direct,@R0
		case 0x86:
		// MOV direct,@R1
		case 0x87:
			{
				u8 a = I8052_ProgByte(cpu);
				u8 r = I8052_REG(op - 0x86);
				r = cpu->ram[r];
				I8052_SetDirect(cpu, a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOV direct,R0
		case 0x88:
		// MOV direct,R1
		case 0x89:
		// MOV direct,R2
		case 0x8A:
		// MOV direct,R3
		case 0x8B:
		// MOV direct,R4
		case 0x8C:
		// MOV direct,R5
		case 0x8D:
		// MOV direct,R6
		case 0x8E:
		// MOV direct,R7
		case 0x8F:
			{
				u8 a = I8052_REG(op - 0x88);
				I8052_SetDirect(cpu, I8052_ProgByte(cpu), a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOV DPTR,#n16
		case 0x90:
			I8052_SFR(I8052_ADDR_DPH) = I8052_ProgByte(cpu); // high
			I8052_SFR(I8052_ADDR_DPL) = I8052_ProgByte(cpu); // low
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOV bit,C
		case 0x92:
			{
				u8 a = I8052_ProgByte(cpu);	// bit address
				u8 r = I8052_PSW >> 7;		// carry bit
				I8052_SetBit(cpu, a, r);	// set bit
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOVC A,@A+DPTR
		case 0x93:
			{
				u16 a = I8052_ACC + I8052_DPTR; // address
				u8 r = cpu->readrom(a);		// read byte from ROM
				I8052_ACC = r;
				U8052_PARITY_UPDATE_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// SUBB A,#n
		case 0x94:
			{
				u8 r = I8052_ProgByte(cpu);
				I8052_SUBB(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// SUBB A,direct
		case 0x95:
			{
				u8 r = I8052_GetDirect(cpu, I8052_ProgByte(cpu));
				I8052_SUBB(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// SUBB A,@R0
		case 0x96:
		// SUBB A,@R1
		case 0x97:
			{
				u8 r = I8052_REG(op - 0x96);
				r = cpu->ram[r];
				I8052_SUBB(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// SUBB A,R0
		case 0x98:
		// SUBB A,R1
		case 0x99:
		// SUBB A,R2
		case 0x9A:
		// SUBB A,R3
		case 0x9B:
		// SUBB A,R4
		case 0x9C:
		// SUBB A,R5
		case 0x9D:
		// SUBB A,R6
		case 0x9E:
		// SUBB A,R7
		case 0x9F:
			{
				u8 r = I8052_REG(op - 0x98);
				I8052_SUBB(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// ORL C,/bit
		case 0xA0:
			{
				u8 a = I8052_ProgByte(cpu);	// bit address
				a = I8052_GetBit(cpu, a);	// get current value of the bit
				if (a == 0) I8052_PSW |= I8052_C; // set carry
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOV C,bit
		case 0xA2:
			{
				u8 a = I8052_ProgByte(cpu);	// bit address
				a = I8052_GetBit(cpu, a);	// get current value of the bit
				if (a == 0)
					I8052_PSW &= ~I8052_C;	// reset carry
				else
					I8052_PSW |= I8052_C;	// set carry
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// INC DPTR
		case 0xA3:
			I8052_DPTR = I8052_DPTR + 1;
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MUL AB
		case 0xA4:
			{
				u8 a = I8052_ACC;
				u8 b = I8052_B;
				u16 res = (u16)a*b;
				I8052_ACC = (u8)res;
				I8052_B = res >> 8;
				I8052_PSW &= ~(I8052_C | I8052_OV);
				if (res > 0xff) I8052_PSW |= I8052_OV;
			}
			cpu->sync.clock += I8052_CLOCKMUL*48;
			break;

		// ... reserved
		case 0xA5:
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// MOV @R0,direct
		case 0xA6:
		// MOV @R1,direct
		case 0xA7:
			{
				u8 r = I8052_REG(op - 0xa6);
				u8 a = I8052_GetDirect(cpu, I8052_ProgByte(cpu));
				cpu->ram[r] = a;
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOV R0,direct
		case 0xA8:
		// MOV R1,direct
		case 0xA9:
		// MOV R2,direct
		case 0xAA:
		// MOV R3,direct
		case 0xAB:
		// MOV R4,direct
		case 0xAC:
		// MOV R5,direct
		case 0xAD:
		// MOV R6,direct
		case 0xAE:
		// MOV R7,direct
			{
				u8 a = I8052_GetDirect(cpu, I8052_ProgByte(cpu));
				I8052_REG(op - 0xA8) = a;
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// ANL C,/bit
		case 0xB0:
			{
				u8 a = I8052_ProgByte(cpu);	// bit address
				a = I8052_GetBit(cpu, a);	// get current value of the bit
				if (a != 0) I8052_PSW &= ~I8052_C; // reset carry
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// CPL bit
		case 0xB2:
			{
				u8 a = I8052_ProgByte(cpu);	// bit address
				u8 val = I8052_GetBitLatch(cpu, a); // get current value of the bit
				I8052_SetBit(cpu, a, val ^ B0); // set inverted bit
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// CPL C
		case 0xB3:
			I8052_PSW ^= I8052_C;
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// CJNE A,#n,rel
		case 0xB4:
			{
				u8 a = I8052_ACC;
				u8 r = I8052_ProgByte(cpu);
				I8052_CJNE(a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// CJNE A,direct,rel
		case 0xB5:
			{
				u8 a = I8052_ACC;
				u8 r = I8052_GetDirect(cpu, I8052_ProgByte(cpu));
				I8052_CJNE(a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// CJNE @R0,#n,rel
		case 0xB6:
		// CJNE @R1,#n,rel
		case 0xB7:
			{
				u8 a = I8052_REG(op - 0xb6);
				a = cpu->ram[a];
				u8 r = I8052_ProgByte(cpu);
				I8052_CJNE(a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// CJNE R0,#n,rel
		case 0xB8:
		// CJNE R1,#n,rel
		case 0xB9:
		// CJNE R2,#n,rel
		case 0xBA:
		// CJNE R3,#n,rel
		case 0xBB:
		// CJNE R4,#n,rel
		case 0xBC:
		// CJNE R5,#n,rel
		case 0xBD:
		// CJNE R6,#n,rel
		case 0xBE:
		// CJNE R7,#n,rel
		case 0xBF:
			{
				u8 a = I8052_REG(op - 0xb8);
				u8 r = I8052_ProgByte(cpu);
				I8052_CJNE(a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// PUSH a
		case 0xC0:
			{
				u8 r = I8052_GetDirect(cpu, I8052_ProgByte(cpu));
				I8052_PUSH8(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// CLR bit
		case 0xC2:
			{
				u8 a = I8052_ProgByte(cpu);	// bit address
				I8052_SetBit(cpu, a, 0);	// clear bit
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// CLR C
		case 0xC3:
			I8052_PSW &= ~I8052_C;
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// SWAP A
		case 0xC4:
			{
				u8 a = I8052_ACC;
				a = (a >> 4) | (a << 4);
				I8052_ACC = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// XCH A,direct
		case 0xC5:
			{
				u8 a = I8052_ProgByte(cpu);
				u8 r = I8052_GetDirectLatch(cpu, a);
				I8052_SetDirect(cpu, a, I8052_ACC);
				I8052_ACC = r;
				U8052_PARITY_UPDATE_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// XCH A,@R0
		case 0xC6:
		// XCH A,@R1
		case 0xC7:
			{
				u8 r = I8052_REG(op - 0xc6);
				u8* s = &cpu->ram[r];
				r = *s;
				*s = I8052_ACC;
				I8052_ACC = r;
				U8052_PARITY_UPDATE_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// XCH A,R0
		case 0xC8:
		// XCH A,R1
		case 0xC9:
		// XCH A,R2
		case 0xCA:
		// XCH A,R3
		case 0xCB:
		// XCH A,R4
		case 0xCC:
		// XCH A,R5
		case 0xCD:
		// XCH A,R6
		case 0xCE:
		// XCH A,R7
		case 0xCF:
			{
				u8* s = &I8052_REG(op - 0xc8);
				u8 a = I8052_ACC;
				I8052_ACC = *s;
				*s = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// POP direct
		case 0xD0:
			{
				u8 a = I8052_ProgByte(cpu);
				u8 r;
				I8052_POP8(r);
				I8052_SetDirect(cpu, a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// SETB bit
		case 0xD2:
			{
				u8 a = I8052_ProgByte(cpu);	// bit address
				I8052_SetBit(cpu, a, 1);	// set bit
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// SETB C
		case 0xD3:
			I8052_PSW |= I8052_C;
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// DA A
		case 0xD4:
			{
				u8 a = I8052_ACC;
				u8 psw = I8052_PSW;
				u8 psw2 = psw & ~I8052_AC; // carry not reset, will be accumulated over more BCDs

				if (((a & 0x0f) > 9) || ((psw & I8052_AC) != 0))
				{
					if ((a & 0x0f) > 9) psw2 |= I8052_AC;
					if (a > 0xf9) psw |= I8052_C;
					a += 6; 
				}

				if ((a > 0x9f) || ((psw & I8052_C) != 0))
				{
					a += 0x60;
					psw2 |= I8052_C;
				}

				I8052_PSW = psw2;
				I8052_ACC = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// DJNZ direct,rel
		case 0xD5:
			{
				u8 a = I8052_ProgByte(cpu);
				u8 r = I8052_GetDirectLatch(cpu, a) - 1;
				I8052_DJNZ(r);
				I8052_SetDirect(cpu, a, r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// XCHD A,@R0
		case 0xD6:
		// XCHD A,@R1
		case 0xD7:
			{
				u8 r = I8052_REG(op - 0xd6);
				u8* s = &cpu->ram[r];
				r = *s;
				u8 a = I8052_ACC;
				*s = (r & 0xf0) | (a & 0x0f);
				r = (r & 0x0f) | (a & 0xf0);
				I8052_ACC = r;
				U8052_PARITY_UPDATE_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// DJNZ R0,rel
		case 0xD8:
		// DJNZ R1,rel
		case 0xD9:
		// DJNZ R2,rel
		case 0xDA:
		// DJNZ R3,rel
		case 0xDB:
		// DJNZ R4,rel
		case 0xDC:
		// DJNZ R5,rel
		case 0xDD:
		// DJNZ R6,rel
		case 0xDE:
		// DJNZ R7,rel
		case 0xDF:
			{
				u8* s = &I8052_REG(op - 0xd8);
				u8 r = *s;
				I8052_DJNZ(r);
				*s = r;
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOVX A,@DPTR
		case 0xE0:
			{
				u8 r = cpu->readext(I8052_DPTR);
				I8052_ACC = r;
				U8052_PARITY_UPDATE_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOVX A,@R0
		case 0xE2:
		// MOVX A,@R1
		case 0xE3:
			{
				u8 r = I8052_REG(op - 0xe2);
				r = cpu->ram[r];
				r = cpu->readext(r);
				I8052_ACC = r;
				U8052_PARITY_UPDATE_A(r);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// CLR A
		case 0xE4:
			I8052_ACC = 0;
			I8052_PSW &= ~I8052_P;
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// MOV A,direct
		case 0xE5:
			{
				u8 a = I8052_GetDirect(cpu, I8052_ProgByte(cpu));
				I8052_ACC = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// MOV A,@R0
		case 0xE6:
		// MOV A,@R1
		case 0xE7:
			{
				u8 a = I8052_REG(op - 0xe6);
				a = cpu->ram[a];
				I8052_ACC = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// MOV A,R0
		case 0xE8:
		// MOV A,R1
		case 0xE9:
		// MOV A,R2
		case 0xEA:
		// MOV A,R3
		case 0xEB:
		// MOV A,R4
		case 0xEC:
		// MOV A,R5
		case 0xED:
		// MOV A,R6
		case 0xEE:
		// MOV A,R7
		case 0xEF:
			{
				u8 a = I8052_REG(op - 0xe8);
				I8052_ACC = a;
				U8052_PARITY_UPDATE_A(a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// MOVX @DPTR,A
		case 0xF0:
			cpu->writeext(I8052_DPTR, I8052_ACC);
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// MOVX @R0,A
		case 0xF2:
		// MOVX @R1,A
		case 0xF3:
			{
				u8 r = I8052_REG(op - 0xf2);
				r = cpu->ram[r];
				cpu->writeext(r, I8052_ACC);
			}
			cpu->sync.clock += I8052_CLOCKMUL*24;
			break;

		// CPL A
		case 0xF4:
			I8052_ACC = ~I8052_ACC; // parity stays unchanged
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// MOV direct,A
		case 0xF5:
			{
				u8 a = I8052_ACC;
				I8052_SetDirect(cpu, I8052_ProgByte(cpu), a);
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// MOV @R0,A
		case 0xF6:
		// MOV @R1,A
		case 0xF7:
			{
				u8 r = I8052_REG(op - 0xf6);
				cpu->ram[r] = I8052_ACC;
			}
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;

		// MOV R0,A
		case 0xF8:
		// MOV R1,A
		case 0xF9:
		// MOV R2,A
		case 0xFA:
		// MOV R3,A
		case 0xFB:
		// MOV R4,A
		case 0xFC:
		// MOV R5,A
		case 0xFD:
		// MOV R6,A
		case 0xFE:
		// MOV R7,A
		case 0xFF:
			I8052_REG(op - 0xf8) = I8052_ACC;
			cpu->sync.clock += I8052_CLOCKMUL*12;
			break;
		}

		// time synchronization
		EmuSync(&cpu->sync);
	}
}

// execute I8052 emulator on CPU 1
static void I8052_ExecCore1()
{
	// start time synchronization
	I8052_SyncStart((sI8052*)I8052_Cpu);

	// execute
	I8052_Exec((sI8052*)I8052_Cpu);

	// clear descriptor
	I8052_Cpu = NULL;
	dsb();
}

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8052_CLOCKMUL is recommended to maintain.
u32 I8052_Start(sI8052* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I8052_Cpu != NULL) I8052_Stop(pwm);

	// initialize time synchronization
	freq = I8052_SyncInit(cpu, pwm, freq);

	// reset processor
	I8052_Reset(cpu);

	// current CPU descriptor
	I8052_Cpu = cpu;
	dsb();

	// execute program
#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	if (DispHstxRunning)
		DispHstxCore1Exec(I8052_ExecCore1);
	else
		Core1Exec(I8052_ExecCore1);
#elif USE_DISPHSTX			// 1=use HSTX Display driver
	DispHstxCore1Exec(I8052_ExecCore1);
#elif USE_MINIVGA			// use mini-VGA display
	VgaCore1Exec(I8052_ExecCore1);
#else
	Core1Exec(I8052_ExecCore1);
#endif

	return freq;
}

// continue emulation without reset processor
u32 I8052_Cont(sI8052* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I8052_Cpu != NULL) I8052_Stop(pwm);

	// initialize time synchronization
	freq = I8052_SyncInit(cpu, pwm, freq);

	// current CPU descriptor
	I8052_Cpu = cpu;
	dsb();

	// execute program
#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	if (DispHstxRunning)
		DispHstxCore1Exec(I8052_ExecCore1);
	else
		Core1Exec(I8052_ExecCore1);
#elif USE_DISPHSTX			// 1=use HSTX Display driver
	DispHstxCore1Exec(I8052_ExecCore1);
#elif USE_MINIVGA			// use mini-VGA display
	VgaCore1Exec(I8052_ExecCore1);
#else
	Core1Exec(I8052_ExecCore1);
#endif

	return freq;
}

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I8052_Stop(int pwm)
{
	// get CPU descriptor
	sI8052* cpu = (sI8052*)I8052_Cpu;

	// check if emulation is running
	if (cpu == NULL) return;

	// request to stop emulation
	cpu->stop = 1;

	// wait to complete
	while (I8052_Cpu != NULL) { dsb(); }

	// terminate time synchronization (stop PWM counter)
	I8052_SyncTerm(pwm);

#if !USE_MINIVGA && !USE_DISPHSTX	// use mini-VGA display

#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	if (!DispHstxRunning)
#endif

	// core 1 reset
	Core1Reset();
#endif
}

#endif // USE_EMU_I8052
