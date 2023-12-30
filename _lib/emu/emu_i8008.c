
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
	for (i = 0; i < 256; i++)
	{
		// initialize parity table
		par = 1;
		j = i;
		while (j != 0)
		{
			par ^= (j & 1);
			j >>= 1;
		}

		// parity flags
		I8008_FlagParTab[i] = (par << I8008_P_BIT) | (i & I8008_S);
	}

	// add zero flag
	I8008_FlagParTab[0] |= I8008_Z;
}

// reset processor
void I8008_Reset(sI8008* cpu)
{
	cpu->pc = 0;		// program counter
	cpu->stack_top = 0;	// stack top

	cpu->afbc = 0;
	cpu->dehl = 0;

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
//  C code size in RAM: 6648 bytes (optimization -Os)
void NOFLASH(I8008_Exec)(sI8008* cpu)
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
			if ((cpu->f & I8008_C) == 0)
			{
				cpu->stack_top = (cpu->stack_top - 1) & I8008_STACKMASK; // decrease stack pointer
				cpu->pc = cpu->stack[cpu->stack_top]; // restore program counter
				cpu->sync.clock += I8008_CLOCKMUL*(10 - 6);
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
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RST 0
		case 0x05:
			cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
			cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
			cpu->pc = 0x0000;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LAI d ... MVI A
		case 0x06:
			cpu->a = I8008_ProgByte(cpu);
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
			{
				u8 n = cpu->b + 1;
				cpu->b = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// DCB ... DCR B
		case 0x09:
			{
				u8 n = cpu->b - 1;
				cpu->b = n;
				cpu->f = I8008_FlagParTab[n];
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

		// RFZ ... RNZ
		case 0x0B:
			if ((cpu->f & I8008_Z) == 0)
			{
				cpu->stack_top = (cpu->stack_top - 1) & I8008_STACKMASK; // decrease stack pointer
				cpu->pc = cpu->stack[cpu->stack_top]; // restore program counter
				cpu->sync.clock += I8008_CLOCKMUL*(10 - 6);
			}
			cpu->sync.clock += I8008_CLOCKMUL*6;
			break;

		// ACI d
		case 0x0C:
			{
				u16 n = cpu->a + I8008_ProgByte(cpu) + (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RST 1 ... RST 0x08
		case 0x0D:
			cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
			cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
			cpu->pc = 0x0008;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LBI d ... MVI B
		case 0x0E:
			cpu->b = I8008_ProgByte(cpu);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RET ... duplicated 0x07
		//case 0x0F:

		// INC ... INR C
		case 0x10:
			{
				u8 n = cpu->c + 1;
				cpu->c = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// DCC ... DCR C
		case 0x11:
			{
				u8 n = cpu->c - 1;
				cpu->c = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// RAL
		case 0x12:
			{
				u8 n = cpu->a;
				cpu->a = (n << 1) | (cpu->f & I8008_C);
				u8 f = cpu->f & (I8008_P | I8008_Z | I8008_S);
				f |= n >> 7; // I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// RFS ... RP
		case 0x13:
			if ((cpu->f & I8008_S) == 0)
			{
				cpu->stack_top = (cpu->stack_top - 1) & I8008_STACKMASK; // decrease stack pointer
				cpu->pc = cpu->stack[cpu->stack_top]; // restore program counter
				cpu->sync.clock += I8008_CLOCKMUL*(10 - 6);
			}
			cpu->sync.clock += I8008_CLOCKMUL*6;
			break;

		// SUI d
		case 0x14:
			{
				u16 n = cpu->a - I8008_ProgByte(cpu);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RST 2 ... RST 0x10
		case 0x15:
			cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
			cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
			cpu->pc = 0x0010;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LCI d ... MVI C
		case 0x16:
			cpu->c = I8008_ProgByte(cpu);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RET ... duplicated 0x07
		//case 0x17:

		// IND ... INR D
		case 0x18:
			{
				u8 n = cpu->d + 1;
				cpu->d = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// DCD ... DCR D
		case 0x19:
			{
				u8 n = cpu->d - 1;
				cpu->d = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

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

		// RFP ... RPO
		case 0x1B:
			if ((cpu->f & I8008_P) == 0)
			{
				cpu->stack_top = (cpu->stack_top - 1) & I8008_STACKMASK; // decrease stack pointer
				cpu->pc = cpu->stack[cpu->stack_top]; // restore program counter
				cpu->sync.clock += I8008_CLOCKMUL*(10 - 6);
			}
			cpu->sync.clock += I8008_CLOCKMUL*6;
			break;

		// SBI d
		case 0x1C:
			{
				u16 n = cpu->a - I8008_ProgByte(cpu) - (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RST 3 ... RST 0x18
		case 0x1D:
			cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
			cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
			cpu->pc = 0x0018;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LDI d ... MVI D
		case 0x1E:
			cpu->d = I8008_ProgByte(cpu);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RET ... duplicated 0x07
		//case 0x1F:

		// INE ... INR E
		case 0x20:
			{
				u8 n = cpu->e + 1;
				cpu->e = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// DCE ... DCR E
		case 0x21:
			{
				u8 n = cpu->e - 1;
				cpu->e = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// - undefined instruction
		//case 0x22:

		// RTC ... RC
		case 0x23:
			if ((cpu->f & I8008_C) != 0)
			{
				cpu->stack_top = (cpu->stack_top - 1) & I8008_STACKMASK; // decrease stack pointer
				cpu->pc = cpu->stack[cpu->stack_top]; // restore program counter
				cpu->sync.clock += I8008_CLOCKMUL*(10 - 6);
			}
			cpu->sync.clock += I8008_CLOCKMUL*6;
			break;

		// NDI d ... ANI d
		case 0x24:
			{
				u8 n = cpu->a & I8008_ProgByte(cpu);
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RST 4 ... RST 0x20
		case 0x25:
			cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
			cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
			cpu->pc = 0x0020;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LEI d ... MVI E
		case 0x26:
			cpu->e = I8008_ProgByte(cpu);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RET ... duplicated 0x07
		//case 0x27:

		// INH ... INR H
		case 0x28:
			{
				u8 n = cpu->h + 1;
				cpu->h = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// DCH ... DCR H
		case 0x29:
			{
				u8 n = cpu->h - 1;
				cpu->h = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// - undefined instruction
		//case 0x2A:

		// RTZ ... RZ
		case 0x2B:
			if ((cpu->f & I8008_Z) != 0)
			{
				cpu->stack_top = (cpu->stack_top - 1) & I8008_STACKMASK; // decrease stack pointer
				cpu->pc = cpu->stack[cpu->stack_top]; // restore program counter
				cpu->sync.clock += I8008_CLOCKMUL*(10 - 6);
			}
			cpu->sync.clock += I8008_CLOCKMUL*6;
			break;

		// XRI d
		case 0x2C:
			{
				u8 n = cpu->a ^ I8008_ProgByte(cpu);
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RST 5 ... RST 0x28
		case 0x2D:
			cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
			cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
			cpu->pc = 0x0028;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LHI d ... MVI H
		case 0x2E:
			cpu->h = I8008_ProgByte(cpu);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RET ... duplicated 0x07
		//case 0x2F:

		// INL ... INR L
		case 0x30:
			{
				u8 n = cpu->l + 1;
				cpu->l = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// DCL ... DCR L
		case 0x31:
			{
				u8 n = cpu->l - 1;
				cpu->l = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// - undefined instruction
		//case 0x32:

		// RTS ... RM
		case 0x33:
			if ((cpu->f & I8008_S) != 0)
			{
				cpu->stack_top = (cpu->stack_top - 1) & I8008_STACKMASK; // decrease stack pointer
				cpu->pc = cpu->stack[cpu->stack_top]; // restore program counter
				cpu->sync.clock += I8008_CLOCKMUL*(10 - 6);
			}
			cpu->sync.clock += I8008_CLOCKMUL*6;
			break;

		// ORI d
		case 0x34:
			{
				u8 n = cpu->a | I8008_ProgByte(cpu);
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// RST 6 ... RST 0x30
		case 0x35:
			cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
			cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
			cpu->pc = 0x0030;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LLI d ... MVI L
		case 0x36:
			cpu->l = I8008_ProgByte(cpu);
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

		// RTP ... RPE
		case 0x3B:
			if ((cpu->f & I8008_P) != 0)
			{
				cpu->stack_top = (cpu->stack_top - 1) & I8008_STACKMASK; // decrease stack pointer
				cpu->pc = cpu->stack[cpu->stack_top]; // restore program counter
				cpu->sync.clock += I8008_CLOCKMUL*(10 - 6);
			}
			cpu->sync.clock += I8008_CLOCKMUL*6;
			break;

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

		// RST 7 ... RST 0x38
		case 0x3D:
			cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
			cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
			cpu->pc = 0x0038;
			cpu->sync.clock += I8008_CLOCKMUL*10;
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
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_C) == 0)
				{
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// INP 0 ... IN 0
		case 0x41:
			cpu->a = cpu->readio(0);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// CFC a ... CNC a
		case 0x42:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_C) == 0)
				{
					cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
					cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// INP 1 ... IN 1
		case 0x43:
			cpu->a = cpu->readio(1);
			cpu->sync.clock += I8008_CLOCKMUL*16;
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

		// INP 2 ... IN 2
		case 0x45:
			cpu->a = cpu->readio(2);
			cpu->sync.clock += I8008_CLOCKMUL*16;
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

		// INP 3 ... IN 3
		case 0x47:
			cpu->a = cpu->readio(3);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// JFZ a ... JNZ a
		case 0x48:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_Z) == 0)
				{
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// INP 4 ... IN 4
		case 0x49:
			cpu->a = cpu->readio(4);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// CFZ a ... CNZ a
		case 0x4A:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_Z) == 0)
				{
					cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
					cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// INP 5 ... IN 5
		case 0x4B:
			cpu->a = cpu->readio(5);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// JMP a ... duplicated 0x44
		//case 0x4C:

		// INP 6 .. IN 6
		case 0x4D:
			cpu->a = cpu->readio(6);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x4E:

		// INP 7 ... IN 7
		case 0x4F:
			cpu->a = cpu->readio(7);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// JFS a ... JP a
		case 0x50:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_S) == 0)
				{
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// OUT 8
		case 0x51:
			cpu->writeio(8, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// CFS a ... CP a
		case 0x52:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_S) == 0)
				{
					cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
					cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// OUT 9
		case 0x53:
			cpu->writeio(9, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// JMP a ... duplicated 0x44
		//case 0x54:

		// OUT 10
		case 0x55:
			cpu->writeio(10, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x56:

		// OUT 11
		case 0x57:
			cpu->writeio(11, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// JFP a ... JPO a
		case 0x58:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_P) == 0)
				{
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// OUT 12
		case 0x59:
			cpu->writeio(12, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// CFP a ... CPO a
		case 0x5A:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_P) == 0)
				{
					cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
					cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// OUT 13
		case 0x5B:
			cpu->writeio(13, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// JMP a ... duplicated 0x44
		//case 0x5C:

		// OUT 14
		case 0x5D:
			cpu->writeio(14, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x5E:

		// OUT 15
		case 0x5F:
			cpu->writeio(15, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// JTC a ... JC a
		case 0x60:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_C) != 0)
				{
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// OUT 16
		case 0x61:
			cpu->writeio(16, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// CTC a ... CC a
		case 0x62:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_C) != 0)
				{
					cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
					cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// OUT 17
		case 0x63:
			cpu->writeio(17, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// JMP a ... duplicated 0x44
		//case 0x64:

		// OUT 18
		case 0x65:
			cpu->writeio(18, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x66:

		// OUT 19
		case 0x67:
			cpu->writeio(19, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// JTZ a ... JZ a
		case 0x68:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_Z) != 0)
				{
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// OUT 20
		case 0x69:
			cpu->writeio(20, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// CTZ a ... CZ a
		case 0x6A:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_Z) != 0)
				{
					cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
					cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// OUT 21
		case 0x6B:
			cpu->writeio(21, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// JMP a ... duplicated 0x44
		//case 0x6C:

		// OUT 22
		case 0x6D:
			cpu->writeio(22, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x6E:

		// OUT 23
		case 0x6F:
			cpu->writeio(23, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// JTS a ... JM a
		case 0x70:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_S) != 0)
				{
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// OUT 24
		case 0x71:
			cpu->writeio(24, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// CTS a ... CM a
		case 0x72:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_S) != 0)
				{
					cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
					cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// OUT 25
		case 0x73:
			cpu->writeio(25, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// JMP a ... duplicated 0x44
		//case 0x74:

		// OUT 26
		case 0x75:
			cpu->writeio(26, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x76:

		// OUT 27
		case 0x77:
			cpu->writeio(27, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// JTP a ... JPE a
		case 0x78:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_P) != 0)
				{
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// OUT 28
		case 0x79:
			cpu->writeio(28, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// CTP a ... CPE a
		case 0x7A:
			{
				u16 n = I8008_ProgWord(cpu);
				if ((cpu->f & I8008_P) != 0)
				{
					cpu->stack[cpu->stack_top] = cpu->pc; // save current program counter
					cpu->stack_top = (cpu->stack_top + 1) & I8008_STACKMASK; // increase stack pointer
					cpu->pc = n;
					cpu->sync.clock += I8008_CLOCKMUL*(22 - 18);
				}
			}
			cpu->sync.clock += I8008_CLOCKMUL*18;
			break;

		// OUT 29
		case 0x7B:
			cpu->writeio(29, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// JMP a ... duplicated 0x44
		//case 0x7C:

		// OUT 30
		case 0x7D:
			cpu->writeio(30, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// CAL a ... CALL a ... duplicated 0x46
		//case 0x7E:

		// OUT 31
		case 0x7F:
			cpu->writeio(31, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*12;
			break;

		// ADA ... ADD A
		case 0x80:
			{
				u16 n = cpu->a + cpu->a;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ADB ... ADD B
		case 0x81:
			{
				u16 n = cpu->a + cpu->b;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ADC ... ADD C
		case 0x82:
			{
				u16 n = cpu->a + cpu->c;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ADD ... ADD D
		case 0x83:
			{
				u16 n = cpu->a + cpu->d;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ADE ... ADD E
		case 0x84:
			{
				u16 n = cpu->a + cpu->e;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ADH ... ADD H
		case 0x85:
			{
				u16 n = cpu->a + cpu->h;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ADL ... ADD L
		case 0x86:
			{
				u16 n = cpu->a + cpu->l;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
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
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// ACA ... ADC A
		case 0x88:
			{
				u16 n = cpu->a + cpu->a + (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ACB ... ADC B
		case 0x89:
			{
				u16 n = cpu->a + cpu->b + (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ACC ... ADC C
		case 0x8A:
			{
				u16 n = cpu->a + cpu->c + (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ACD ... ADC D
		case 0x8B:
			{
				u16 n = cpu->a + cpu->d + (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ACE ... ADC E
		case 0x8C:
			{
				u16 n = cpu->a + cpu->e + (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ACH ... ADC H
		case 0x8D:
			{
				u16 n = cpu->a + cpu->h + (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ACL ... ADC L
		case 0x8E:
			{
				u16 n = cpu->a + cpu->l + (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= n2 >> 8; // carry on bit 0
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
				f |= n2 >> 8; // carry on bit 0
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
			{
				u16 n = cpu->a - cpu->b;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SUC ... SUB C
		case 0x92:
			{
				u16 n = cpu->a - cpu->c;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SUD ... SUB D
		case 0x93:
			{
				u16 n = cpu->a - cpu->d;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SUE ... SUB E
		case 0x94:
			{
				u16 n = cpu->a - cpu->e;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SUH ... SUB H
		case 0x95:
			{
				u16 n = cpu->a - cpu->h;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SUL ... SUB L
		case 0x96:
			{
				u16 n = cpu->a - cpu->l;
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
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
				f |= (n2 >> 8) & I8008_C;
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
			{
				u16 n = cpu->a - cpu->b - (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SBC ... SBB C
		case 0x9A:
			{
				u16 n = cpu->a - cpu->c - (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SBD ... SBB D
		case 0x9B:
			{
				u16 n = cpu->a - cpu->d - (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SBE ... SBB E
		case 0x9C:
			{
				u16 n = cpu->a - cpu->e - (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SBH ... SBB H
		case 0x9D:
			{
				u16 n = cpu->a - cpu->h - (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// SBL ... SBB L
		case 0x9E:
			{
				u16 n = cpu->a - cpu->l - (cpu->f & I8008_C);
				u8 n2 = (u8)n;
				cpu->a = n2;
				u8 f = I8008_FlagParTab[n2];
				f |= (n2 >> 8) & I8008_C;
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
				f |= (n2 >> 8) & I8008_C;
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
			{
				u8 n = cpu->a & cpu->b;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// NDC ... ANA C
		case 0xA2:
			{
				u8 n = cpu->a & cpu->c;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// NDD ... ANA D
		case 0xA3:
			{
				u8 n = cpu->a & cpu->d;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// NDE ... ANA E
		case 0xA4:
			{
				u8 n = cpu->a & cpu->e;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// NDH ... ANA H
		case 0xA5:
			{
				u8 n = cpu->a & cpu->h;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// NDL ... ANA L
		case 0xA6:
			{
				u8 n = cpu->a & cpu->l;
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
			{
				u8 n = cpu->a ^ cpu->b;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// XRC ... XRA C
		case 0xAA:
			{
				u8 n = cpu->a ^ cpu->c;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// XRD ... XRA D
		case 0xAB:
			{
				u8 n = cpu->a ^ cpu->d;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// XRE ... XRA E
		case 0xAC:
			{
				u8 n = cpu->a ^ cpu->e;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// XRH ... XRA H
		case 0xAD:
			{
				u8 n = cpu->a ^ cpu->h;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// XRL ... XRA L
		case 0xAE:
			{
				u8 n = cpu->a ^ cpu->l;
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
			{
				u8 n = cpu->a | cpu->b;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ORC ... ORA C
		case 0xB2:
			{
				u8 n = cpu->a | cpu->c;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ORD ... ORA D
		case 0xB3:
			{
				u8 n = cpu->a | cpu->d;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ORE ... ORA E
		case 0xB4:
			{
				u8 n = cpu->a | cpu->e;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ORH .. ORA H
		case 0xB5:
			{
				u8 n = cpu->a | cpu->h;
				cpu->a = n;
				cpu->f = I8008_FlagParTab[n];
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// ORL ... ORA L
		case 0xB6:
			{
				u8 n = cpu->a | cpu->l;
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
			{
				u16 n = cpu->a - cpu->b;
				u8 f = I8008_FlagParTab[(u8)n];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// CPC ... CMP C
		case 0xBA:
			{
				u16 n = cpu->a - cpu->c;
				u8 f = I8008_FlagParTab[(u8)n];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// CPD ... CMP D
		case 0xBB:
			{
				u16 n = cpu->a - cpu->d;
				u8 f = I8008_FlagParTab[(u8)n];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// CPE ... CMP E
		case 0xBC:
			{
				u16 n = cpu->a - cpu->e;
				u8 f = I8008_FlagParTab[(u8)n];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// CPH ... CMP H
		case 0xBD:
			{
				u16 n = cpu->a - cpu->h;
				u8 f = I8008_FlagParTab[(u8)n];
				f |= (n >> 8) & I8008_C;
				cpu->f = f;
			}
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// CPL ... CMP L
		case 0xBE:
			{
				u16 n = cpu->a - cpu->l;
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
		// case 0xC0: ... see NOP

		// LAB ... MOV A,B ... load register A from B
		case 0xC1:
			cpu->a = cpu->b;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LAC ... MOV A,C ... load register A from C
		case 0xC2:
			cpu->a = cpu->c;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LAD ... MOV A,D ... load register A from D
		case 0xC3:
			cpu->a = cpu->d;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LAE ... MOV A,E ... load register A from E
		case 0xC4:
			cpu->a = cpu->e;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LAH ... MOV A,H ... load register A from H
		case 0xC5:
			cpu->a = cpu->h;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LAL ... MOV A,L ... load register A from L
		case 0xC6:
			cpu->a = cpu->l;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LAM ... MOV A,M ... load register A from (HL)
		case 0xC7:
			cpu->a = cpu->readmem(cpu->hl & I8008_ADDRMASK);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// LBA ... MOV B,A ... load register B from A
		case 0xC8:
			cpu->b = cpu->a;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LBB... MOV B,B ... load register B from B
		//case 0xC9: ... see NOP

		// LBC ... MOV B,C ... load register B from C
		case 0xCA:
			cpu->b = cpu->c;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LBD ... MOV B,D ... load register B from D
		case 0xCB:
			cpu->b = cpu->d;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LBE ... MOV B,E ... load register B from E
		case 0xCC:
			cpu->b = cpu->e;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LBH ... MOV B,H ... load register B from H
		case 0xCD:
			cpu->b = cpu->h;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LBL ... MOV B,L ... load register B from L
		case 0xCE:
			cpu->b = cpu->l;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LBM ... MOV B,M ... load register B from (HL)
		case 0xCF:
			cpu->b = cpu->readmem(cpu->hl & I8008_ADDRMASK);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// LCA ... MOV C,A ... load register C from A
		case 0xD0:
			cpu->c = cpu->a;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LCB ... MOV C,B ... load register C from B
		case 0xD1:
			cpu->c = cpu->b;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LCC ... MOV C,C ... load register C from C
		//case 0xD2: ... see NOP

		// LCD ... MOV C,D ... load register C from D
		case 0xD3:
			cpu->c = cpu->d;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LCE ... MOV C,E ... load register C from E
		case 0xD4:
			cpu->c = cpu->e;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LCH ... MOV C,H ... load register C from H
		case 0xD5:
			cpu->c = cpu->h;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LCL ... MOV C,L ... load register C from L
		case 0xD6:
			cpu->c = cpu->l;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LCM ... MOV C,M ... load register C from (HL)
		case 0xD7:
			cpu->c = cpu->readmem(cpu->hl & I8008_ADDRMASK);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// LDA ... MOV D,A ... load register D from A
		case 0xD8:
			cpu->d = cpu->a;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LDB ... MOV D,B ... load register D from B
		case 0xD9:
			cpu->d = cpu->b;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LDC ... MOV D,C ... load register D from C
		case 0xDA:
			cpu->d = cpu->c;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LDD... MOV D,D ... load register D from D
		//case 0xDB: ... see NOP

		// LDE ... MOV D,E ... load register D from E
		case 0xDC:
			cpu->d = cpu->e;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LDH ... MOV D,H ... load register D from H
		case 0xDD:
			cpu->d = cpu->h;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LDL ... MOV D,L ... load register D from L
		case 0xDE:
			cpu->d = cpu->l;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LDM ... MOV D,M ... load register D from (HL)
		case 0xDF:
			cpu->d = cpu->readmem(cpu->hl & I8008_ADDRMASK);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// LEA ... MOV E,A ... load register E from A
		case 0xE0:
			cpu->e = cpu->a;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LEB ... MOV E,B ... load register E from B
		case 0xE1:
			cpu->e = cpu->b;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LEC ... MOV E,C ... load register E from C
		case 0xE2:
			cpu->e = cpu->c;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LED ... MOV E,D ... load register E from D
		case 0xE3:
			cpu->e = cpu->d;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LEE ... MOV E,E ... load register E from E
		//case 0xE4: ... see NOP

		// LEH ... MOV E,H ... load register E from H
		case 0xE5:
			cpu->e = cpu->h;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LEL ... MOV E,L ... load register E from L
		case 0xE6:
			cpu->e = cpu->l;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LEM ... MOV E,M ... load register E from (HL)
		case 0xE7:
			cpu->e = cpu->readmem(cpu->hl & I8008_ADDRMASK);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// LHA ... MOV H,A ... load register H from A
		case 0xE8:
			cpu->h = cpu->a;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LHB ... MOV H,B ... load register H from B
		case 0xE9:
			cpu->h = cpu->b;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LHC ... MOV H,C ... load register H from C
		case 0xEA:
			cpu->h = cpu->c;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LHD... MOV H,D ... load register H from D
		case 0xEB:
			cpu->h = cpu->d;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LHE ... MOV H,E ... load register H from E
		case 0xEC:
			cpu->h = cpu->e;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LHH ... MOV H,H ... load register H from H
		//case 0xED: ... see NOP

		// LHL ... MOV H,L ... load register H from L
		case 0xEE:
			cpu->h = cpu->l;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LHM ... MOV H,M ... load register H from (HL)
		case 0xEF:
			cpu->h = cpu->readmem(cpu->hl & I8008_ADDRMASK);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// LLA ... MOV L,A ... load register L from A
		case 0xF0:
			cpu->l = cpu->a;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LLB ... MOV L,B ... load register L from B
		case 0xF1:
			cpu->l = cpu->b;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LLC ... MOV L,C ... load register L from C
		case 0xF2:
			cpu->l = cpu->c;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LLD ... MOV L,D ... load register L from D
		case 0xF3:
			cpu->l = cpu->d;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LLE ... MOV L,E ... load register L from E
		case 0xF4:
			cpu->l = cpu->e;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LLH ... MOV L,H ... load register L from H
		case 0xF5:
			cpu->l = cpu->h;
			cpu->sync.clock += I8008_CLOCKMUL*10;
			break;

		// LLL ... MOV L,L ... load register L from L
		//case 0xF6: ... see NOP

		// LLM ... MOV L,M ... load register L from (HL)
		case 0xF7:
			cpu->l = cpu->readmem(cpu->hl & I8008_ADDRMASK);
			cpu->sync.clock += I8008_CLOCKMUL*16;
			break;

		// LMA ... MOV M,A ... load memory (HL) from A
		case 0xF8:
			cpu->writemem(cpu->hl & I8008_ADDRMASK, cpu->a);
			cpu->sync.clock += I8008_CLOCKMUL*14;
			break;

		// LMB ... MOV M,B ... load memory (HL) from B
		case 0xF9:
			cpu->writemem(cpu->hl & I8008_ADDRMASK, cpu->b);
			cpu->sync.clock += I8008_CLOCKMUL*14;
			break;

		// LMC ... MOV M,C ... load memory (HL) from C
		case 0xFA:
			cpu->writemem(cpu->hl & I8008_ADDRMASK, cpu->c);
			cpu->sync.clock += I8008_CLOCKMUL*14;
			break;

		// LMD... MOV M,D ... load memory (HL) from D
		case 0xFB:
			cpu->writemem(cpu->hl & I8008_ADDRMASK, cpu->d);
			cpu->sync.clock += I8008_CLOCKMUL*14;
			break;

		// LME ... MOV M,E ... load memory (HL) from E
		case 0xFC:
			cpu->writemem(cpu->hl & I8008_ADDRMASK, cpu->e);
			cpu->sync.clock += I8008_CLOCKMUL*14;
			break;

		// LMH ... MOV M,H ... load memory (HL) from H
		case 0xFD:
			cpu->writemem(cpu->hl & I8008_ADDRMASK, cpu->h);
			cpu->sync.clock += I8008_CLOCKMUL*14;
			break;

		// LML ... MOV M,L ... load memory (HL) from L
		case 0xFE:
			cpu->writemem(cpu->hl & I8008_ADDRMASK, cpu->l);
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
