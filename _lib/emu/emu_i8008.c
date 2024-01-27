
// ****************************************************************************
//
//                              I8008 CPU Emulator
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

#if USE_EMU_I8008	// use I8008 CPU emulator

// current CPU descriptor (NULL = not running)
volatile sI8008* I8008_Cpu = NULL;

// flags table with parity (I8008_P, I8008_Z and I8008_S flags)
u8 I8008_FlagParTab[256];

// initialize I8008 tables
void I8008_InitTab()
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
		f = 0;
		if (par == 0) f |= I8008_P; // even parity
		if (i >= 0x80) f |= I8008_S; // negative
		if (i == 0) f |= I8008_Z; // zero
		I8008_FlagParTab[i] = f;
	}
}

// reset processor
void I8008_Reset(sI8008* cpu)
{
	cpu->pc = 0;		// program counter
	cpu->stack_top = 0;	// stack top
	cpu->hl = 0;
	cpu->de = 0;
	cpu->bc = 0;
	cpu->fa = 0;
	cpu->stop = 0;		// stop request
	cpu->halted = 0;	// 1=CPU is halted (HALT instruction)
	cpu->intreq = 0;	// 1=interrupt request
	cpu->intins = 0x0D;	// interrupt instruction RST 0x08

	int i;
	for (i = 0; i < I8008_STACKNUM; i++) cpu->stack[i] = 0;	// stack
}

// load byte from program memory
INLINE u8 I8008_ProgByte(sI8008* cpu)
{
	// read program byte
	u8 n = cpu->readmem(cpu->pc);

	// compiler barrier (for the PC to be valid at the time of the callback instruction)
	cb();

	// shift program counter
	cpu->pc = (cpu->pc + 1) & I8008_ADDRMASK;

	return n;
}

// load 14-bit word from program memory
INLINE u16 I8008_ProgWord(sI8008* cpu)
{
	u8 temp = I8008_ProgByte(cpu);
	return (temp | ((u16)I8008_ProgByte(cpu) << 8)) & I8008_ADDRMASK;
}

// execute program (start or continue, until "stop" request)
// Size of code of this function: 1576 code + 1024 jump table = 2600 bytes
// CPU loading at 500 kHz on 120 MHz: used 2%, max. 2-5%
// CPU loading at 5 MHz on 120 MHz: used 25-29%, max. 25-50%
void FASTCODE NOFLASH(I8008_Exec)(sI8008* cpu)
{
	u8 op;

	// clear stop flag
	cpu->stop = 0;

	// program loop
	while (cpu->stop == 0)
	{
		// interrupt
		if (cpu->intreq != 0)
		{
			// clear interrupt request
			cpu->intreq = 0;

			// continue after HALT
			if (cpu->halted != 0)
			{
				cpu->pc = (cpu->pc + 1) & I8008_ADDRMASK;
				cpu->halted = 0;
			}

			// process interrupt instruction
			op = cpu->intins;
		}
		else
		{
			// get next instruction
			op = I8008_ProgByte(cpu);
		}

		// switch base operation code
		switch (op)
		{
		// HLT ... HALT
		//case 0x00: ... duplicated 0xFF
		//case 0x01: ... duplicated 0xFF

		// RLC
		case 0x02:
			{
				u8 n = cpu->a;
				n = (n << 1) | (n >> 7);
				cpu->a = n;
				u8 f = cpu->f & (I8008_P | I8008_Z | I8008_S);
				f |= n & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// RFC ... RNC
		case 0x03:
		// RFZ ... RNZ
		case 0x0B:
		// RFS ... RP
		case 0x13:
		// RFP ... RPO
		case 0x1B:
		// RTC ... RC
		case 0x23:
		// RTZ ... RZ
		case 0x2B:
		// RTS ... RM
		case 0x33:
		// RTP ... RPE
		case 0x3B:
			{
				// flag bit
				int fb = (op >> 3) & 3;

				// invert flags
				u8 f = cpu->f;
				if ((op & B5) != 0) f = ~f;

				// check condition
				if ((f & BIT(fb)) == 0)
				{
					cpu->stack_top = (cpu->stack_top - 1) & I8008_STACKMASK; // decrease stack pointer
					cpu->pc = cpu->stack[cpu->stack_top]; // restore program counter
					cpu->sync.clock += I8008_CLOCKMUL*(10 - 6);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*6;
			break;

		// ADI d
		case 0x04:
			{
				u16 n = cpu->a + I8008_ProgByte(cpu);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n >> 8; // carry
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RST 0 ... RST 0x00
		case 0x05:
		// RST 1 ... RST 0x08
		case 0x0D:
		// RST 2 ... RST 0x10
		case 0x15:
		// RST 3 ... RST 0x18
		case 0x1D:
		// RST 4 ... RST 0x20
		case 0x25:
		// RST 5 ... RST 0x28
		case 0x2D:
		// RST 6 ... RST 0x30
		case 0x35:
		// RST 7 ... RST 0x38
		case 0x3D:
			cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
			cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
			cpu->pc = op - 5;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LAI d ... MVI A
		case 0x06:
		// LBI d ... MVI B
		case 0x0E:
		// LCI d ... MVI C
		case 0x16:
		// LDI d ... MVI D
		case 0x1E:
		// LEI d ... MVI E
		case 0x26:
		// LHI d ... MVI H
		case 0x2E:
		// LLI d ... MVI L
		case 0x36:
			{
				u8* r = &cpu->reg[6 - (op >> 3)];
				*r = I8008_ProgByte(cpu);
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RET
		case 0x07:
		case 0x0F: // duplicated
		case 0x17: // duplicated
		case 0x1F: // duplicated
		case 0x27: // duplicated
		case 0x2F: // duplicated
		case 0x37: // duplicated
		case 0x3F: // duplicated
			cpu->stack_top = (cpu->stack_top - 1) & I8008_STACKMASK; // decrease stack pointer
			cpu->pc = cpu->stack[cpu->stack_top]; // restore program counter
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// INB ... INR B
		case 0x08:
		// INC ... INR C
		case 0x10:
		// IND ... INR D
		case 0x18:
		// INE ... INR E
		case 0x20:
		// INH ... INR H
		case 0x28:
		// INL ... INR L
		case 0x30:
			{
				u8* r = &cpu->reg[6 - (op >> 3)];
				u8 n = *r + 1;
				*r = n;
				cpu->f = I8008_FlagParTab[n] | (cpu->f & I8008_C);
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// DCB ... DCR B
		case 0x09:
		// DCC ... DCR C
		case 0x11:
		// DCD ... DCR D
		case 0x19:
		// DCE ... DCR E
		case 0x21:
		// DCH ... DCR H
		case 0x29:
		// DCL ... DCR L
		case 0x31:
			{
				u8* r = &cpu->reg[6 - (op >> 3)];
				u8 n = *r - 1;
				*r = n;
				cpu->f = I8008_FlagParTab[n] | (cpu->f & I8008_C);
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// RRC
		case 0x0A:
			{
				u8 n = cpu->a;
				u8 n2 = (n >> 1) | (n << 7);
				cpu->a = n2;
				u8 f = cpu->f & (I8008_P | I8008_Z | I8008_S);
				f |= n & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ACI d
		case 0x0C:
			{
				u16 n = cpu->a + I8008_ProgByte(cpu) + (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n >> 8; // carry
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RET ... duplicated 0x07
		//case 0x0F:

		// RAL
		case 0x12:
			{
				u8 n = cpu->a;
				cpu->a = (n << 1) | (cpu->f & I8008_C);
				u8 f = cpu->f & (I8008_P | I8008_Z | I8008_S);
				f |= n >> 7; // carry;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SUI d
		case 0x14:
			{
				u16 n = cpu->a - I8008_ProgByte(cpu);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RET ... duplicated 0x07
		//case 0x17:

		// RAR
		case 0x1A:
			{
				u8 n = cpu->a;
				cpu->a = (n >> 1) | (cpu->f << 7);
				u8 f = cpu->f & (I8008_P | I8008_Z | I8008_S);
				f |= n & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SBI d
		case 0x1C:
			{
				u16 n = cpu->a - I8008_ProgByte(cpu) - (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RET ... duplicated 0x07
		//case 0x1F:

		// - undefined instruction
		//case 0x22:

		// NDI d ... ANI d
		case 0x24:
			{
				u8 n = cpu->a & I8008_ProgByte(cpu);
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RET ... duplicated 0x07
		//case 0x27:

		// - undefined instruction
		//case 0x2A:

		// XRI d
		case 0x2C:
			{
				u8 n = cpu->a ^ I8008_ProgByte(cpu);
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RET ... duplicated 0x07
		//case 0x2F:

		// - undefined instruction
		//case 0x32:

		// ORI d
		case 0x34:
			{
				u8 n = cpu->a | I8008_ProgByte(cpu);
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RET ... duplicated 0x07
		//case 0x37:

		// -  undefined instruction (INM ... INR M ... instruction is not supported by the CPU)
		//case 0x38:

		// -  undefined instruction (DCM ... DCR M ... instruction is not supported by the CPU)
		//case 0x39:

		// - undefined instruction
		//case 0x3A:

		// CPI d
		case 0x3C: // S Z P
			{
				u16 n = cpu->a - I8008_ProgByte(cpu);
				u8 f = I8008_FlagParTab[(u8)n];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// LMI d ... MVI M
		case 0x3E:
			cpu->writemem(cpu->hl & I8008_ADDRMASK, I8008_ProgByte(cpu));
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// RET ... duplicated 0x07
		//case 0x3F:

		// JFC a ... JNC a
		case 0x40:
		// JFZ a ... JNZ a
		case 0x48:
		// JFS a ... JP a
		case 0x50:
		// JFP a ... JPO a
		case 0x58:
		// JTC a ... JC a
		case 0x60:
		// JTZ a ... JZ a
		case 0x68:
		// JTS a ... JM a
		case 0x70:
		// JTP a ... JPE a
		case 0x78:
			{
				// flag bit
				int fb = (op >> 3) & 3;

				// invert flags
				u8 f = cpu->f;
				if ((op & B5) != 0) f = ~f;

				// read destination address
				u16 n = I8008_ProgWord(cpu);

				// check condition
				if ((f & BIT(fb)) == 0)
				{
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// INP 0 ... IN 0
		case 0x41:
		// INP 1 ... IN 1
		case 0x43:
		// INP 2 ... IN 2
		case 0x45:
		// INP 3 ... IN 3
		case 0x47:
		// INP 4 ... IN 4
		case 0x49:
		// INP 5 ... IN 5
		case 0x4B:
		// INP 6 .. IN 6
		case 0x4D:
		// INP 7 ... IN 7
		case 0x4F:
			cpu->a = cpu->readport((op - 0x41) >> 1);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// CFC a ... CNC a
		case 0x42:
		// CFZ a ... CNZ a
		case 0x4A:
		// CFS a ... CP a
		case 0x52:
		// CFP a ... CPO a
		case 0x5A:
		// CTC a ... CC a
		case 0x62:
		// CTZ a ... CZ a
		case 0x6A:
		// CTS a ... CM a
		case 0x72:
		// CTP a ... CPE a
		case 0x7A:
			{
				// flag bit
				int fb = (op >> 3) & 3;

				// invert flags
				u8 f = cpu->f;
				if ((op & B5) != 0) f = ~f;

				// read destination address
				u16 n = I8008_ProgWord(cpu);

				// check condition
				if ((f & BIT(fb)) == 0)
				{
					cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
					cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// JMP a
		case 0x44:
		case 0x4C: // duplicated
		case 0x54: // duplicated
		case 0x5C: // duplicated
		case 0x64: // duplicated
		case 0x6C: // duplicated
		case 0x74: // duplicated
		case 0x7C: // duplicated
			cpu->pc = I8008_ProgWord(cpu);
			cpu->sync.clock += I8008_CLOCKMUL*22;
			break;

		// CAL a ... CALL a
		case 0x46:
		case 0x4E: // duplicated
		case 0x56: // duplicated
		case 0x5E: // duplicated
		case 0x66: // duplicated
		case 0x6E: // duplicated
		case 0x76: // duplicated
		case 0x7E: // duplicated
			cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
			cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
			cpu->pc = I8008_ProgWord(cpu);
			cpu->sync.clock += I8008_CLOCKMUL*22;
			break;

		// JMP a ... duplicated 0x44
		//case 0x4C:

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x4E:

		// OUT 8
		case 0x51:
		// OUT 9
		case 0x53:
		// OUT 10
		case 0x55:
		// OUT 11
		case 0x57:
		// OUT 12
		case 0x59:
		// OUT 13
		case 0x5B:
		// OUT 14
		case 0x5D:
		// OUT 15
		case 0x5F:
		// OUT 16
		case 0x61:
		// OUT 17
		case 0x63:
		// OUT 18
		case 0x65:
		// OUT 19
		case 0x67:
		// OUT 20
		case 0x69:
		// OUT 21
		case 0x6B:
		// OUT 22
		case 0x6D:
		// OUT 23
		case 0x6F:
		// OUT 24
		case 0x71:
		// OUT 25
		case 0x73:
		// OUT 26
		case 0x75:
		// OUT 27
		case 0x77:
		// OUT 28
		case 0x79:
		// OUT 29
		case 0x7B:
		// OUT 30
		case 0x7D:
		// OUT 31
		case 0x7F:
			cpu->writeport((op - 0x41) >> 1, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// JMP a ... duplicated 0x44
		//case 0x54:

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x56:

		// JMP a ... duplicated 0x44
		//case 0x5C:

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x5E:

		// JMP a ... duplicated 0x44
		//case 0x64:

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x66:

		// JMP a ... duplicated 0x44
		//case 0x6C:

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x6E:

		// JMP a ... duplicated 0x44
		//case 0x74:

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x76:

		// JMP a ... duplicated 0x44
		//case 0x7C:

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x7E:

		// ADA ... ADD A
		case 0x80:
		// ADB ... ADD B
		case 0x81:
		// ADC ... ADD C
		case 0x82:
		// ADD ... ADD D
		case 0x83:
		// ADE ... ADD E
		case 0x84:
		// ADH ... ADD H
		case 0x85:
		// ADL ... ADD L
		case 0x86:
			{
				u8* rs = &cpu->reg[6 - (op - 0x80)]; // source register
				u16 n = cpu->a + *rs;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n >> 8; // carry
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ADM ... ADD M
		case 0x87:
			{
				u16 n = cpu->a + cpu->readmem(cpu->hl & I8008_ADDRMASK);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n >> 8; // carry
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// ACA ... ADC A
		case 0x88:
		// ACB ... ADC B
		case 0x89:
		// ACC ... ADC C
		case 0x8A:
		// ACD ... ADC D
		case 0x8B:
		// ACE ... ADC E
		case 0x8C:
		// ACH ... ADC H
		case 0x8D:
		// ACL ... ADC L
		case 0x8E:
			{
				u8* rs = &cpu->reg[6 - (op - 0x88)]; // source register
				u16 n = cpu->a + *rs + (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n >> 8; // carry
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ACM ... ADC M
		case 0x8F:
			{
				u16 n = cpu->a + cpu->readmem(cpu->hl & I8008_ADDRMASK) + (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n >> 8; // carry
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// SUA ... SUB A
		case 0x90:
			{
				cpu->a = 0;
				cpu->f = I8008_P | I8008_Z;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SUB ... SUB B
		case 0x91:
		// SUC ... SUB C
		case 0x92:
		// SUD ... SUB D
		case 0x93:
		// SUE ... SUB E
		case 0x94:
		// SUH ... SUB H
		case 0x95:
		// SUL ... SUB L
		case 0x96:
			{
				u8* rs = &cpu->reg[6 - (op - 0x90)]; // source register
				u16 n = cpu->a - *rs;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SUM ... SUB M
		case 0x97:
			{
				u16 n = cpu->a - cpu->readmem(cpu->hl & I8008_ADDRMASK);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// SBA ... SBB A
		case 0x98:
			{
				u8 n = 0;
				u8 f = I8008_P | I8008_Z;
				if ((cpu->f & I8008_C) != 0)
				{
					n = 0xff;
					f = I8008_P | I8008_C;
				}
				cpu->a = n;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SBB ... SBB B
		case 0x99:
		// SBC ... SBB C
		case 0x9A:
		// SBD ... SBB D
		case 0x9B:
		// SBE ... SBB E
		case 0x9C:
		// SBH ... SBB H
		case 0x9D:
		// SBL ... SBB L
		case 0x9E:
			{
				u8* rs = &cpu->reg[6 - (op - 0x98)]; // source register
				u16 n = cpu->a - *rs - (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SBM ... SBB M
		case 0x9F:
			{
				u16 n = cpu->a - cpu->readmem(cpu->hl & I8008_ADDRMASK) - (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// NDA ... ANA A
		case 0xA0:
			cpu->f = I8008_FlagParTab[cpu->a];
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// NDB ... ANA B
		case 0xA1:
		// NDC ... ANA C
		case 0xA2:
		// NDD ... ANA D
		case 0xA3:
		// NDE ... ANA E
		case 0xA4:
		// NDH ... ANA H
		case 0xA5:
		// NDL ... ANA L
		case 0xA6:
			{
				u8* rs = &cpu->reg[6 - (op - 0xA0)]; // source register
				u8 n = cpu->a & *rs;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// NDM ... ANA M
		case 0xA7:
			{
				u8 n = cpu->a & cpu->readmem(cpu->hl & I8008_ADDRMASK);
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// XRA ... XRA A
		case 0xA8:
			{
				cpu->a = 0;
				cpu->f = I8008_P | I8008_Z;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// XRB ... XRA B
		case 0xA9:
		// XRC ... XRA C
		case 0xAA:
		// XRD ... XRA D
		case 0xAB:
		// XRE ... XRA E
		case 0xAC:
		// XRH ... XRA H
		case 0xAD:
		// XRL ... XRA L
		case 0xAE:
			{
				u8* rs = &cpu->reg[6 - (op - 0xA8)]; // source register
				u8 n = cpu->a ^ *rs;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// XRM ... XRA M
		case 0xAF:
			{
				u8 n = cpu->a ^ cpu->readmem(cpu->hl & I8008_ADDRMASK);
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// ORA ... ORA A
		case 0xB0:
			cpu->f = I8008_FlagParTab[cpu->a];
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ORB ... ORA B
		case 0xB1:
		// ORC ... ORA C
		case 0xB2:
		// ORD ... ORA D
		case 0xB3:
		// ORE ... ORA E
		case 0xB4:
		// ORH .. ORA H
		case 0xB5:
		// ORL ... ORA L
		case 0xB6:
			{
				u8* rs = &cpu->reg[6 - (op - 0xB0)]; // source register
				u8 n = cpu->a | *rs;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ORM ... ORA M
		case 0xB7:
			{
				u8 n = cpu->a | cpu->readmem(cpu->hl & I8008_ADDRMASK);
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// CPA ... CMP A
		case 0xB8:
			cpu->f = I8008_P | I8008_Z;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// CPB ... CMP B
		case 0xB9:
		// CPC ... CMP C
		case 0xBA:
		// CPD ... CMP D
		case 0xBB:
		// CPE ... CMP E
		case 0xBC:
		// CPH ... CMP H
		case 0xBD:
		// CPL ... CMP L
		case 0xBE:
			{
				u8* rs = &cpu->reg[6 - (op - 0xB8)]; // source register
				u16 n = cpu->a - *rs;
				u8 f = I8008_FlagParTab[(u8)n];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// CPM ... CMP M
		case 0xBF:
			{
				u16 n = cpu->a - cpu->readmem(cpu->hl & I8008_ADDRMASK);
				u8 f = I8008_FlagParTab[(u8)n];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// NOP
		case 0xC0: // LAA ... MOV A,A ... load register A from A ... NOP
		case 0xC9: // LBB ... MOV B,B ... load register B from B
		case 0xD2: // LCC ... MOV C,C ... load register C from C
		case 0xDB: // LDD ... MOV D,D ... load register D from D
		case 0xE4: // LEE ... MOV E,E ... load register E from E
		case 0xED: // LHH ... MOV H,H ... load register H from H
		case 0xF6: // LLL ... MOV L,L ... load register L from L
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LAA ... MOV A,A ... load register A from A
		//case 0xC0: ... see NOP
		// LAB ... MOV A,B ... load register A from B
		case 0xC1:
		// LAC ... MOV A,C ... load register A from C
		case 0xC2:
		// LAD ... MOV A,D ... load register A from D
		case 0xC3:
		// LAE ... MOV A,E ... load register A from E
		case 0xC4:
		// LAH ... MOV A,H ... load register A from H
		case 0xC5:
		// LAL ... MOV A,L ... load register A from L
		case 0xC6:
		// LBA ... MOV B,A ... load register B from A
		case 0xC8:
		// LBB... MOV B,B ... load register B from B
		//case 0xC9: ... see NOP
		// LBC ... MOV B,C ... load register B from C
		case 0xCA:
		// LBD ... MOV B,D ... load register B from D
		case 0xCB:
		// LBE ... MOV B,E ... load register B from E
		case 0xCC:
		// LBH ... MOV B,H ... load register B from H
		case 0xCD:
		// LBL ... MOV B,L ... load register B from L
		case 0xCE:
		// LCA ... MOV C,A ... load register C from A
		case 0xD0:
		// LCB ... MOV C,B ... load register C from B
		case 0xD1:
		// LCC ... MOV C,C ... load register C from C
		//case 0xD2: ... see NOP
		// LCD ... MOV C,D ... load register C from D
		case 0xD3:
		// LCE ... MOV C,E ... load register C from E
		case 0xD4:
		// LCH ... MOV C,H ... load register C from H
		case 0xD5:
		// LCL ... MOV C,L ... load register C from L
		case 0xD6:
		// LDA ... MOV D,A ... load register D from A
		case 0xD8:
		// LDB ... MOV D,B ... load register D from B
		case 0xD9:
		// LDC ... MOV D,C ... load register D from C
		case 0xDA:
		// LDD... MOV D,D ... load register D from D
		//case 0xDB: ... see NOP
		// LDE ... MOV D,E ... load register D from E
		case 0xDC:
		// LDH ... MOV D,H ... load register D from H
		case 0xDD:
		// LDL ... MOV D,L ... load register D from L
		case 0xDE:
		// LEA ... MOV E,A ... load register E from A
		case 0xE0:
		// LEB ... MOV E,B ... load register E from B
		case 0xE1:
		// LEC ... MOV E,C ... load register E from C
		case 0xE2:
		// LED ... MOV E,D ... load register E from D
		case 0xE3:
		// LEE ... MOV E,E ... load register E from E
		//case 0xE4: ... see NOP
		// LEH ... MOV E,H ... load register E from H
		case 0xE5:
		// LEL ... MOV E,L ... load register E from L
		case 0xE6:
		// LHA ... MOV H,A ... load register H from A
		case 0xE8:
		// LHB ... MOV H,B ... load register H from B
		case 0xE9:
		// LHC ... MOV H,C ... load register H from C
		case 0xEA:
		// LHD... MOV H,D ... load register H from D
		case 0xEB:
		// LHE ... MOV H,E ... load register H from E
		case 0xEC:
		// LHH ... MOV H,H ... load register H from H
		//case 0xED: ... see NOP
		// LHL ... MOV H,L ... load register H from L
		case 0xEE:
		// LLA ... MOV L,A ... load register L from A
		case 0xF0:
		// LLB ... MOV L,B ... load register L from B
		case 0xF1:
		// LLC ... MOV L,C ... load register L from C
		case 0xF2:
		// LLD ... MOV L,D ... load register L from D
		case 0xF3:
		// LLE ... MOV L,E ... load register L from E
		case 0xF4:
		// LLH ... MOV L,H ... load register L from H
		case 0xF5:
		// LLL ... MOV L,L ... load register L from L
		//case 0xF6: ... see NOP
			{
				op -= 0xc0;
				u8* rd = &cpu->reg[6 - (op >> 3)]; // destination register
				u8* rs = &cpu->reg[6 - (op & 7)]; // source register
				*rd = *rs;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LAM ... MOV A,M ... load register A from (HL)
		case 0xC7:
		// LBM ... MOV B,M ... load register B from (HL)
		case 0xCF:
		// LCM ... MOV C,M ... load register C from (HL)
		case 0xD7:
		// LDM ... MOV D,M ... load register D from (HL)
		case 0xDF:
		// LEM ... MOV E,M ... load register E from (HL)
		case 0xE7:
		// LHM ... MOV H,M ... load register H from (HL)
		case 0xEF:
		// LLM ... MOV L,M ... load register L from (HL)
		case 0xF7:
			{
				op -= 0xc0;
				u8* rd = &cpu->reg[6 - (op >> 3)]; // destination register
				*rd = cpu->readmem(cpu->hl & I8008_ADDRMASK);
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// LMA ... MOV M,A ... load memory (HL) from A
		case 0xF8:
		// LMB ... MOV M,B ... load memory (HL) from B
		case 0xF9:
		// LMC ... MOV M,C ... load memory (HL) from C
		case 0xFA:
		// LMD... MOV M,D ... load memory (HL) from D
		case 0xFB:
		// LME ... MOV M,E ... load memory (HL) from E
		case 0xFC:
		// LMH ... MOV M,H ... load memory (HL) from H
		case 0xFD:
		// LML ... MOV M,L ... load memory (HL) from L
		case 0xFE:
			{
				u8* rs = &cpu->reg[6 - (op & 7)]; // source register
				cpu->writemem(cpu->hl & I8008_ADDRMASK, *rs);
			}
			cpu->sync.clock += I8008_CLOCKMUL*14;
			break;

		// HLT ... HALT
		case 0x00: // duplicated
		case 0x01: // duplicated
		case 0xFF:
			cpu->pc = (cpu->pc - 1) & I8008_ADDRMASK; // return PC to start of HLT instruction
			cpu->halted = 1;
			cpu->sync.clock += I8008_CLOCKMUL*8;
			break;

		// undefined instructions 0x22, 0x2A, 0x32, 0x38, 0x39, 0x3A
		default:
			cpu->sync.clock += I8008_CLOCKMUL*8;
			break;
		}

		// time synchronization
		EmuSync(&cpu->sync);
	}
}

// execute I8008 emulator on CPU 1
static void I8008_ExecCore1()
{
	// start time synchronization
	I8008_SyncStart((sI8008*)I8008_Cpu);

	// execute
	I8008_Exec((sI8008*)I8008_Cpu);

	// clear descriptor
	I8008_Cpu = NULL;
	dsb();
}

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8008_CLOCKMUL is recommended to maintain.
u32 I8008_Start(sI8008* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I8008_Cpu != NULL) I8008_Stop(pwm);

	// initialize time synchronization
	freq = I8008_SyncInit(cpu, pwm, freq);

	// reset processor
	I8008_Reset(cpu);

	// current CPU descriptor
	I8008_Cpu = cpu;
	dsb();

	// execute program
#if USE_MINIVGA				// use mini-VGA display
	VgaCore1Exec(I8008_ExecCore1);
#else
	Core1Exec(I8008_ExecCore1);
#endif

	return freq;
}

// continue emulation without reset processor
u32 I8008_Cont(sI8008* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I8008_Cpu != NULL) I8008_Stop(pwm);

	// initialize time synchronization
	freq = I8008_SyncInit(cpu, pwm, freq);

	// current CPU descriptor
	I8008_Cpu = cpu;
	dsb();

	// execute program
#if USE_MINIVGA				// use mini-VGA display
	VgaCore1Exec(I8008_ExecCore1);
#else
	Core1Exec(I8008_ExecCore1);
#endif

	return freq;
}

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I8008_Stop(int pwm)
{
	// get CPU descriptor
	sI8008* cpu = (sI8008*)I8008_Cpu;

	// check if emulation is running
	if (cpu == NULL) return;

	// request to stop emulation
	cpu->stop = 1;

	// wait to complete
	while (I8008_Cpu != NULL) { dsb(); }

	// terminate time synchronization (stop PWM counter)
	I8008_SyncTerm(pwm);

#if !USE_MINIVGA				// use mini-VGA display
	// core 1 reset
	Core1Reset();
#endif
}

#endif // USE_EMU_I8008
