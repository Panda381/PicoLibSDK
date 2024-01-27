
// ****************************************************************************
//
//                             I8048 CPU Emulator
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

#if USE_EMU_I8048	// use I8048 CPU emulator

#include "emu_i8048_macro.h"

// current CPU descriptor (NULL = not running)
volatile sI8048* I8048_Cpu = NULL;

// reset processor
void I8048_Reset(sI8048* cpu)
{
	cpu->pc = 0;		// program counter
	cpu->a = 0;		// accumulator
	cpu->psw = B3;		// status word
	cpu->f1 = 0;		// F1 state
	cpu->regbase = 0;	// base index of registers
	cpu->a11 = 0;		// last A11 state
	cpu->inirq = 0;		// IRQ is not in progress
	cpu->ie = 0;		// IRQ disabled
	cpu->intreq = 0;	// interrupt request
	cpu->tcntie = 0;	// timer/counter interrupt disabled
	cpu->tcntreq = 0;	// timer/counter interrupt request
	cpu->ent0clk = 0;	// disable test 0 clock output
	//cpu->tcnt = 		// timer/counter is not affected by reset
	cpu->tcntmode = I8048_CNT_STOP; // timer/counter mode
	cpu->lastport[0] = 0xff; // last output to BUS
	cpu->lastport[1] = 0xff; // last output to port 1
	cpu->lastport[2] = 0xff; // last output to port 2
	cpu->lastport[3] = 0xff; // last output to port 3
	cpu->lastt1 = 0;	// last T1 state 

	// setup PROG signal to HIGH state
	cpu->writeprog(1);
}

// shift clock and process timer
void FASTCODE NOFLASH(I8048_Clock)(sI8048* cpu)
{
	// shift clock
	cpu->sync.clock += I8048_CLOCKMUL*15;

	// check if timer/counter is enabled
	u8 mode = cpu->tcntmode;
	if (mode != I8048_CNT_STOP)
	{
		// timer
		if (mode == I8048_CNT_TIMER)
		{
			// increment prescaler
			u8 presc = cpu->prescaler + 1;
			if (presc >= 32) // tick timer?
			{
				// reset prescaler
				presc -= 32;

				// increment timer
				u8 t = cpu->tcnt;
				t++;
				cpu->tcnt = t;

				// overflow
				if (t == 0) cpu->tcntreq = 1;
			}

			// save new prescaler
			cpu->prescaler = presc;
		}

		// counter
		else
		{
			// get new T1 state
			u8 t1 = cpu->readtest(1);

			// 1->0 edge
			if ((t1 == 0) && (cpu->lastt1 != 0))
			{
				// increment counter
				u8 t = cpu->tcnt;
				t++;
				cpu->tcnt = t;

				// overflow
				if (t == 0) cpu->tcntreq = 1;
			}

			// save new T1 state
			cpu->lastt1 = t1;
		}
	}
}

// load byte from program memory
INLINE u8 I8048_ProgByte(sI8048* cpu)
{
	// read program byte
	u8 n = cpu->readrom(cpu->pc);

	// compiler barrier (for the PC to be valid at the time of the callback instruction)
	cb();

	// shift program counter
	u16 pc = cpu->pc;
	cpu->pc = ((pc + 1) & I8048_ADDRMASK) | (pc & I8048_BANKMASK);

	// shift clock
	I8048_Clock(cpu);

	return n;
}

// execute program (start or continue, until "stop" request)
// Size of code of this function: 3344 code + 1024 jump table = 4368 bytes
// CPU loading at 6 MHz on 120 MHz: used 25-30%, max. 27-35%
void FASTCODE NOFLASH(I8048_Exec)(sI8048* cpu)
{
	u8 op;

	// clear stop flag
	cpu->stop = 0;

	// program loop
	while (cpu->stop == 0)
	{
		// interrupt
		if (cpu->inirq != 0) // already in interrupt handler
		{
			// external interrupt
			if ((cpu->intreq != 0) && (cpu->ie != 0))
			{
				// in interrupt
				cpu->inirq = 1;

				// call 0x003
				I8048_PUSH_PC_PSW();
				cpu->pc = 0x003;

				// shift clock
				I8048_Clock(cpu);
				I8048_Clock(cpu);

				// next loop (we ignore time synchronization for now)
				continue;
			}

			// timer/counter overflow
			if ((cpu->tcntreq != 0) && (cpu->tcntie != 0))
			{
				// in interrupt
				cpu->inirq = 1;

				// clear request
				cpu->tcntreq = 0;

				// call 0x007
				I8048_PUSH_PC_PSW();
				cpu->pc = 0x007;

				// shift clock
				I8048_Clock(cpu);
				I8048_Clock(cpu);

				// next loop (we ignore time synchronization for now)
				continue;
			}
		}

		// get next instruction
		op = I8048_ProgByte(cpu);

		// switch base operation code
		switch (op)
		{
		// NOP
		case 0x00: // NOP
		case 0x01: // ...illegal
		case 0x06: // ...illegal
		case 0x22: // ...illegal
		case 0x33: // ...illegal
		case 0x63: // ...illegal
		case 0x66: // ...illegal
		case 0x73: // ...illegal
		case 0x82: // ...illegal
		case 0x87: // ...illegal
		case 0xA2: // ...illegal
		case 0xA6: // ...illegal
		case 0xB7: // ...illegal
		case 0xC0: // ...illegal
		case 0xC1: // ...illegal
		case 0xC2: // ...illegal
		case 0xC3: // ...illegal
		case 0xD6: // ...illegal
		case 0xE0: // ...illegal
		case 0xE1: // ...illegal
		case 0xE2: // ...illegal
		case 0xF3: // ...illegal
			break;

		// OUTL BUS,A (OUTL P0,A)
		case 0x02:
			{
				u8 n = cpu->a;
				cpu->lastport[0] = n;
				cpu->writeport(0, n);
			}
			I8048_Clock(cpu);
			break;

		// ADD A,#n
		case 0x03:
			{
				u8 n = I8048_ProgByte(cpu);
				I8048_ADD(n);
			}
			break;

		// JMP 0xx
		case 0x04:
		// JMP 1xx
		case 0x24:
		// JMP 2xx
		case 0x44:
		// JMP 3xx
		case 0x64:
		// JMP 4xx
		case 0x84:
		// JMP 5xx
		case 0xA4:
		// JMP 6xx
		case 0xC4:
		// JMP 7xx
		case 0xE4:
			{
				// jump to destination address (interrupt always in bank 0)
				u16 pc = ((u16)(op - 0x04) << 3) | I8048_ProgByte(cpu);
				if (cpu->inirq == 0) pc |= (u16)cpu->a11 << 11;
				cpu->pc = pc;
			}
			break;

		// EN I
		case 0x05:
			cpu->ie = 1;
			break;

		// DEC A
		case 0x07:
			cpu->a--;
			break;

		// INS A,BUS (IN A,P0)
		case 0x08:
		// IN A,P1
		case 0x09:
		// IN A,P2
		case 0x0A:
		// IN A,P3
		case 0x0B:
			cpu->a = cpu->readport(op - 0x08);
			I8048_Clock(cpu);
			break;

		// MOVD A,P4
		case 0x0C:
		// MOVD A,P5
		case 0x0D:
		// MOVD A,P6
		case 0x0E:
		// MOVD A,P7
		case 0x0F:
			{
				// put opcode on lower 4 bits of P2
				u8 n = cpu->lastport[2] & 0xf0;
				n += (I8048_EXP_READ << 2) + (op - 0x0c);
				cpu->writeport(2, n);

				// PROG signal 1->0 to write setup
				cpu->writeprog(0);

				// read data to accumulator
				n |= 0x0f;
				cpu->writeport(2, n); // place P20-P23 to input mode
				cpu->lastport[2] = n; // save last output state
				cpu->a = cpu->readport(2) & 0x0f; // read input data

				// PROG signal 0->1 to terminate READ operation
				cpu->writeprog(1);

				// shift clock
				I8048_Clock(cpu);
			}
			break;

		// INC @R0
		case 0x10:
		// INC @R1
		case 0x11:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0x10)];
				cpu->ram[n & I8048_RAMMASK]++;
			}
			break;

		// JB0 a
		case 0x12:
		// JB1 a
		case 0x32:
		// JB2 a
		case 0x52:
		// JB3 a
		case 0x72:
		// JB4 a
		case 0x92:
		// JB5 a
		case 0xB2:
		// JB6 a
		case 0xD2:
		// JB7 a
		case 0xF2:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if ((cpu->a & BIT(op - 0x12)) != 0) cpu->pcl = a;
			}
			break;

		// ADDC A,#n
		case 0x13:
			{
				u8 n = I8048_ProgByte(cpu);
				I8048_ADDC(n);
			}
			break;

		// CALL 0xx
		case 0x14:
		// CALL 1xx
		case 0x34:
		// CALL 2xx
		case 0x54:
		// CALL 3xx
		case 0x74:
		// CALL 4xx
		case 0x94:
		// CALL 5xx
		case 0xB4:
		// CALL 6xx
		case 0xD4:
		// CALL 7xx
		case 0xF4:
			{
				// get destination address
				u8 a = I8048_ProgByte(cpu);

				// push PC and PSW into stack
				I8048_PUSH_PC_PSW();

				// jump to destination address (interrupt always in bank 0)
				u16 pc = ((u16)(op - 0x14) << 3) | a;
				if (cpu->inirq == 0) pc |= (u16)cpu->a11 << 11;
				cpu->pc = pc;
			}
			break;

		// DIS I
		case 0x15:
			cpu->ie = 0;
			break;

		// JTF a
		case 0x16:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if (cpu->tcntreq != 0) cpu->pcl = a;
				cpu->tcntreq = 0; // reset counter overflow flag
			}
			break;

		// INC A
		case 0x17:
			cpu->a++;
			break;

		// INC R0
		case 0x18:
		// INC R1
		case 0x19:
		// INC R2
		case 0x1A:
		// INC R3
		case 0x1B:
		// INC R4
		case 0x1C:
		// INC R5
		case 0x1D:
		// INC R6
		case 0x1E:
		// INC R7
		case 0x1F:
			cpu->ram[cpu->regbase + (op - 0x18)]++;
			break;

		// XCH A,@R0
		case 0x20:
		// XCH A,@R1
		case 0x21:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0x20)];
				u8* r = &cpu->ram[n & I8048_RAMMASK];
				u8 a = cpu->a;
				cpu->a = *r;
				*r = a;
			}
			break;

		// MOV A,#n
		case 0x23:
			cpu->a = I8048_ProgByte(cpu);
			break;

		// EN TCNTI
		case 0x25:
			cpu->tcntie = 1; // enable timer/counter interrupt
			break;

		// JNT0 a
		case 0x26:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if (cpu->readtest(0) == 0) cpu->pcl = a;
			}
			break;

		// CLR A
		case 0x27:
			cpu->a = 0;
			break;

		// XCH A,R0
		case 0x28:
		// XCH A,R1
		case 0x29:
		// XCH A,R2
		case 0x2A:
		// XCH A,R3
		case 0x2B:
		// XCH A,R4
		case 0x2C:
		// XCH A,R5
		case 0x2D:
		// XCH A,R6
		case 0x2E:
		// XCH A,R7
		case 0x2F:
			{
				u8* r = &cpu->ram[cpu->regbase + (op - 0x28)];
				u8 a = cpu->a;
				cpu->a = *r;
				*r = a;
			}
			break;

		// XCHD A,@R0
		case 0x30:
		// XCHD A,@R1
		case 0x31:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0x30)];
				u8* r = &cpu->ram[n & I8048_RAMMASK];
				n = *r;
				u8 a = cpu->a;
				cpu->a = (n & 0x0f) | (a & 0xf0);
				*r = (a & 0x0f) | (n & 0xf0);
			}
			break;

		// DIS TCNTI
		case 0x35:
			cpu->tcntie = 0; // disable timer/counter interrupt
			cpu->tcntreq = 0; // clear timer/counter interrupt request
			break;

		// JT0 a
		case 0x36:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if (cpu->readtest(0) != 0) cpu->pcl = a;
			}
			break;

		// CPL A
		case 0x37:
			cpu->a = ~cpu->a;
			break;

		// OUT BUS,A
		case 0x38:
		// OUTL P1,A
		case 0x39:
		// OUTL P2,A
		case 0x3A:
		// OUTL P3,A
		case 0x3B:
			{
				op -= 0x38;
				u8 n = cpu->a;
				cpu->lastport[op] = n;
				cpu->writeport(op, n);
			}
			I8048_Clock(cpu);
			break;

		// MOVD P4,A
		case 0x3C:
		// MOVD P5,A
		case 0x3D:
		// MOVD P6,A
		case 0x3E:
		// MOVD P7,A
		case 0x3F:
			{
				// put opcode on lower 4 bits of P2
				u8 n = cpu->lastport[2] & 0xf0;
				cpu->writeport(2, n + (I8048_EXP_WRITE << 2) + (op - 0x3c));

				// PROG signal 1->0 to write setup
				cpu->writeprog(0);

				// write data from accumulator
				n |= cpu->a & 0x0f;
				cpu->lastport[2] = n;
				cpu->writeport(2, n);

				// PROG signal 0->1 to confirm output data
				cpu->writeprog(1);

				// shift clock
				I8048_Clock(cpu);
			}
			break;

		// ORL A,@R0
		case 0x40:
		// ORL A,@R1
		case 0x41:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0x40)];
				n = cpu->ram[n & I8048_RAMMASK];
				cpu->a |= n;
			}
			break;

		// MOV A,T
		case 0x42:
			cpu->a = cpu->tcnt;
			break;

		// ORL A,#n
		case 0x43:
			cpu->a |= I8048_ProgByte(cpu);
			break;

		// STRT CNT
		case 0x45:
			cpu->lastt1 = 0; // reset old T1 state
			cpu->tcntmode = I8048_CNT_COUNT; // start counting 1->0 edges on T1
			break;

		// JNT1 a
		case 0x46:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if (cpu->readtest(1) == 0) cpu->pcl = a;
			}
			break;

		// SWAP A
		case 0x47:
			{
				u8 a = cpu->a;
				cpu->a = (a << 4) | (a >> 4);
			}
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
			cpu->a |= cpu->ram[cpu->regbase + (op - 0x48)];
			break;

		// ANL A,@R0
		case 0x50:
		// ANL A,@R1
		case 0x51:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0x50)];
				n = cpu->ram[n & I8048_RAMMASK];
				cpu->a &= n;
			}
			break;

		// ANL A,#n
		case 0x53:
			cpu->a &= I8048_ProgByte(cpu);
			break;

		// STRT T
		case 0x55:
			cpu->prescaler = 0; // clear timer prescaler
			cpu->tcntmode = I8048_CNT_TIMER; // start timer
			break;

		// JT1 a
		case 0x56:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if (cpu->readtest(1) != 0) cpu->pcl = a;
			}
			break;

		// DA A
		case 0x57:
			{
				u8 a = cpu->a;
				u8 psw = cpu->psw;
				u8 psw2 = psw & ~I8048_AC; // carry not reset, will be accumulated over more BCDs

				if (((a & 0x0f) > 9) || ((psw & I8048_AC) != 0))
				{
					if ((a & 0x0f) > 9) psw2 |= I8048_AC;
					if (a > 0xf9) psw |= I8048_C;
					a += 6;
				}

				if ((a > 0x9f) || ((psw & I8048_C) != 0))
				{
					a += 0x60;
					psw2 |= I8048_C;
				}

				cpu->a = a;
				cpu->psw = psw2;
			}
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
			cpu->a &= cpu->ram[cpu->regbase + (op - 0x58)];
			break;

		// ADD A,@R0
		case 0x60:
		// ADD A,@R1
		case 0x61:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0x60)];
				n = cpu->ram[n & I8048_RAMMASK];
				I8048_ADD(n);
			}
			break;

		// MOV T,A
		case 0x62:
			cpu->tcnt = cpu->a;
			break;

		// STOP TCNT
		case 0x65:
			cpu->tcntmode = I8048_CNT_STOP; // stop timer/counter
			break;

		// RRC A
		case 0x67:
			{
				u8 a = cpu->a;
				u8 psw = cpu->psw;
				u8 carry = (a << 7) & I8048_C; // new carry bit
				a = (a >> 1) | (psw & I8048_C);
				cpu->a = a;
				cpu->psw = (psw & ~I8048_C) | carry;
			}
			break;

		// ADD A,R0
		case 0x68:
		// ADD A,R1
		case 0x69:
		// ADD A,R2
		case 0x6A:
		// ADD A,R3
		case 0x6B:
		// ADD A,R4
		case 0x6C:
		// ADD A,R5
		case 0x6D:
		// ADD A,R6
		case 0x6E:
		// ADD A,R7
		case 0x6F:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0x68)];
				I8048_ADD(n);
			}
			break;

		// ADDC A,@R0
		case 0x70:
		// ADDC A,@R1
		case 0x71:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0x70)];
				n = cpu->ram[n & I8048_RAMMASK];
				I8048_ADDC(n);
			}
			break;

		// ENT0 CLK
		case 0x75:
			cpu->ent0clk = 1;	// enable test 0 clock output
			break;

		// JF1 a
		case 0x76:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if (cpu->f1 != 0) cpu->pcl = a;
			}
			break;

		// RR A
		case 0x77:
			{
				u8 a = cpu->a;
				a = (a >> 1) | (a << 7);
				cpu->a = a;
			}
			break;

		// ADDC A,R0
		case 0x78:
		// ADDC A,R1
		case 0x79:
		// ADDC A,R2
		case 0x7A:
		// ADDC A,R3
		case 0x7B:
		// ADDC A,R4
		case 0x7C:
		// ADDC A,R5
		case 0x7D:
		// ADDC A,R6
		case 0x7E:
		// ADDC A,R7
		case 0x7F:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0x78)];
				I8048_ADDC(n);
			}
			break;

		// MOVX A,@R0
		case 0x80:
		// MOVX A,@R1
		case 0x81:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0x80)];
				cpu->a = cpu->readext(n);
				I8048_Clock(cpu);
			}
			break;

		// RET
		case 0x83:
			{
				I8048_POP_PC();
				I8048_Clock(cpu);
			}
			break;

		// CLR F0
		case 0x85:
			cpu->psw &= ~I8048_F0;
			break;

		// JNI a
		case 0x86:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if (cpu->intreq != 0) cpu->pcl = a;
			}
			break;

		// ORL BUS,#n
		case 0x88:
		// ORL P1,#n
		case 0x89:
		// ORL P2,#n
		case 0x8A:
		// ORL P3,#n
		case 0x8B:
			{
				u8 n = I8048_ProgByte(cpu);
				op -= 0x88;
				n |= cpu->lastport[op];
				cpu->lastport[op] = n;
				cpu->writeport(op, n);
			}
			break;

		// ORLD P4,A
		case 0x8C:
		// ORLD P5,A
		case 0x8D:
		// ORLD P6,A
		case 0x8E:
		// ORLD P7,A
		case 0x8F:
			{
				// put opcode on lower 4 bits of P2
				u8 n = cpu->lastport[2] & 0xf0;
				cpu->writeport(2, n + (I8048_EXP_OR << 2) + (op - 0x8c));

				// PROG signal 1->0 to write setup
				cpu->writeprog(0);

				// write data from accumulator
				n |= cpu->a & 0x0f;
				cpu->lastport[2] = n;
				cpu->writeport(2, n);

				// PROG signal 0->1 to confirm output data
				cpu->writeprog(1);

				// shift clock
				I8048_Clock(cpu);
			}
			break;

		// MOVX @R0,A
		case 0x90:
		// MOVX @R1,A
		case 0x91:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0x90)];
				cpu->writeext(n, cpu->a);
				I8048_Clock(cpu);
			}
			break;

		// RETR
		case 0x93:
			{
				cpu->inirq = 0; // end IRQ service
				I8048_POP_PC_PSW();
				I8048_Clock(cpu);
			}
			break;

		// CPL F0
		case 0x95:
			cpu->psw ^= I8048_F0;
			break;

		// JNZ a
		case 0x96:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if (cpu->a != 0) cpu->pcl = a;
			}
			break;

		// CLR C
		case 0x97:
			cpu->psw &= ~I8048_C;
			break;

		// ANL BUS,#n
		case 0x98:
		// ANL P1,#n
		case 0x99:
		// ANL P2,#n
		case 0x9A:
		// ANL P3,#n
		case 0x9B:
			{
				u8 n = I8048_ProgByte(cpu);
				op -= 0x98;
				n &= cpu->lastport[op];
				cpu->lastport[op] = n;
				cpu->writeport(op, n);
			}
			break;

		// ANLD P4,A
		case 0x9C:
		// ANLD P5,A
		case 0x9D:
		// ANLD P6,A
		case 0x9E:
		// ANLD P7,A
		case 0x9F:
			{
				// put opcode on lower 4 bits of P2
				u8 n = cpu->lastport[2] & 0xf0;
				cpu->writeport(2, n + (I8048_EXP_AND << 2) + (op - 0x9c));

				// PROG signal 1->0 to write setup
				cpu->writeprog(0);

				// write data from accumulator
				n |= cpu->a & 0x0f;
				cpu->lastport[2] = n;
				cpu->writeport(2, n);

				// PROG signal 0->1 to confirm output data
				cpu->writeprog(1);

				// shift clock
				I8048_Clock(cpu);
			}
			break;

		// MOV @R0,A
		case 0xA0:
		// MOV @R1,A
		case 0xA1:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0xa0)];
				cpu->ram[n & I8048_RAMMASK] = cpu->a;
			}
			break;

		// MOVP A,@A
		case 0xA3:
			{
				cpu->a = cpu->readrom(((u16)cpu->pch << 8) | cpu->a);
				I8048_Clock(cpu);
			}
			break;

		// CLK F1
		case 0xA5:
			cpu->f1 = 0;
			break;

		// CPL C
		case 0xA7:
			cpu->psw ^= I8048_C;
			break;

		// MOV R0,A
		case 0xA8:
		// MOV R1,A
		case 0xA9:
		// MOV R2,A
		case 0xAA:
		// MOV R3,A
		case 0xAB:
		// MOV R4,A
		case 0xAC:
		// MOV R5,A
		case 0xAD:
		// MOV R6,A
		case 0xAE:
		// MOV R7,A
		case 0xAF:
			cpu->ram[cpu->regbase + (op - 0xa8)] = cpu->a;
			break;

		// MOV @R0,#n
		case 0xB0:
		// MOV @R1,#n
		case 0xB1:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0xb0)];
				u8 d = I8048_ProgByte(cpu); // data
				cpu->ram[n & I8048_RAMMASK] = d;
			}
			break;

		// JMPP @A
		case 0xB3:
			{
				u16 pc = cpu->pc;
				u8 n = cpu->readrom((pc & 0xff00) | cpu->a); // read destination address
				cpu->pcl = n;
				I8048_Clock(cpu);
			}
			break;

		// CPL F1
		case 0xB5:
			cpu->f1 ^= B0;
			break;

		// JF0 a
		case 0xB6:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if ((cpu->psw & I8048_F0) != 0) cpu->pcl = a;
			}
			break;

		// MOV R0,#n
		case 0xB8:
		// MOV R1,#n
		case 0xB9:
		// MOV R2,#n
		case 0xBA:
		// MOV R3,#n
		case 0xBB:
		// MOV R4,#n
		case 0xBC:
		// MOV R5,#n
		case 0xBD:
		// MOV R6,#n
		case 0xBE:
		// MOV R7,#n
		case 0xBF:
			{
				u8 n = I8048_ProgByte(cpu); // destination address
				cpu->ram[cpu->regbase + (op - 0xb8)] = n;
			}
			break;

		// SEL RB0
		case 0xC5:
			cpu->psw &= ~I8048_BS;
			cpu->regbase = 0;
			break;

		// JZ a
		case 0xC6:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if (cpu->a == 0) cpu->pcl = a;
			}
			break;

		// MOV A,PSW
		case 0xC7:
			cpu->a = cpu->psw | B3;
			break;

		// DEC R0
		case 0xC8:
		// DEC R1
		case 0xC9:
		// DEC R2
		case 0xCA:
		// DEC R3
		case 0xCB:
		// DEC R4
		case 0xCC:
		// DEC R5
		case 0xCD:
		// DEC R6
		case 0xCE:
		// DEC R7
		case 0xCF:
			cpu->ram[cpu->regbase + (op - 0xc8)]--;
			break;

		// XRL A,@R0
		case 0xD0:
		// XRL A,@R1
		case 0xD1:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0xD0)];
				n = cpu->ram[n & I8048_RAMMASK];
				cpu->a ^= n;
			}
			break;

		// XRL A,#n
		case 0xD3:
			cpu->a &= I8048_ProgByte(cpu);
			break;

		// SEL RB1
		case 0xD5:
			cpu->psw |= I8048_BS;
			cpu->regbase = 24;
			break;

		// MOV PSW,A
		case 0xD7:
			{
				u8 n = cpu->a & ~B3;
				cpu->psw = n;
				cpu->regbase = ((n & I8048_BS) == 0) ? 0 : 24;
			}
			break;

		// XRL A,R0
		case 0xD8:
		// XRL A,R1
		case 0xD9:
		// XRL A,R2
		case 0xDA:
		// XRL A,R3
		case 0xDB:
		// XRL A,R4
		case 0xDC:
		// XRL A,R5
		case 0xDD:
		// XRL A,R6
		case 0xDE:
		// XRL A,R7
		case 0xDF:
			cpu->a ^= cpu->ram[cpu->regbase + (op - 0xd8)];
			break;

		// MOVP3 A,@A
		case 0xE3:
			{
				cpu->a = cpu->readrom(0x300 | cpu->a);
				I8048_Clock(cpu);
			}
			break;

		// SEL MB0
		case 0xE5:
			cpu->a11 = 0;
			break;

		// JNC a
		case 0xE6:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if ((cpu->psw & I8048_C) == 0) cpu->pcl = a;
			}
			break;

		// RL A
		case 0xE7:
			{
				u8 a = cpu->a;
				a = (a << 1) | (a >> 7);
				cpu->a = a;
			}
			break;

		// DJNZ R0,a
		case 0xE8:
		// DJNZ R1,a
		case 0xE9:
		// DJNZ R2,a
		case 0xEA:
		// DJNZ R3,a
		case 0xEB:
		// DJNZ R4,a
		case 0xEC:
		// DJNZ R5,a
		case 0xED:
		// DJNZ R6,a
		case 0xEE:
		// DJNZ R7,a
		case 0xEF:
			{
				u8 a = I8048_ProgByte(cpu);
				u8* r = &cpu->ram[cpu->regbase + (op - 0xe8)];
				u8 n = *r - 1;
				*r = n;
				if (n != 0) cpu->pcl = a;
			}
			break;

		// MOV A,@R0
		case 0xF0:
		// MOV A,@R1
		case 0xF1:
			{
				u8 n = cpu->ram[cpu->regbase + (op - 0xf0)];
				cpu->a = cpu->ram[n & I8048_RAMMASK];
			}
			break;

		// SEL MB1
		case 0xF5:
			cpu->a11 = 1;
			break;

		// JC a
		case 0xF6:
			{
				u8 a = I8048_ProgByte(cpu); // destination address
				if ((cpu->psw & I8048_C) != 0) cpu->pcl = a;
			}
			break;

		// RLC A
		case 0xF7:
			{
				u8 a = cpu->a;
				u8 psw = cpu->psw;
				u8 carry = a & I8048_C; // new carry bit
				a = (a << 1) | (psw >> 7);
				cpu->a = a;
				cpu->psw = (psw & ~I8048_C) | carry;
			}
			break;

		// MOV A,R0
		case 0xF8:
		// MOV A,R1
		case 0xF9:
		// MOV A,R2
		case 0xFA:
		// MOV A,R3
		case 0xFB:
		// MOV A,R4
		case 0xFC:
		// MOV A,R5
		case 0xFD:
		// MOV A,R6
		case 0xFE:
		// MOV A,R7
		case 0xFF:
			cpu->a = cpu->ram[cpu->regbase + (op - 0xf8)];
			break;
		}

		// time synchronization
		EmuSync(&cpu->sync);
	}
}

// execute I8048 emulator on CPU 1
static void I8048_ExecCore1()
{
	// start time synchronization
	I8048_SyncStart((sI8048*)I8048_Cpu);

	// execute
	I8048_Exec((sI8048*)I8048_Cpu);

	// clear descriptor
	I8048_Cpu = NULL;
	dsb();
}

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8048_CLOCKMUL is recommended to maintain.
u32 I8048_Start(sI8048* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I8048_Cpu != NULL) I8048_Stop(pwm);

	// initialize time synchronization
	freq = I8048_SyncInit(cpu, pwm, freq);

	// reset processor
	I8048_Reset(cpu);

	// current CPU descriptor
	I8048_Cpu = cpu;
	dsb();

	// execute program
#if USE_MINIVGA				// use mini-VGA display
	VgaCore1Exec(I8048_ExecCore1);
#else
	Core1Exec(I8048_ExecCore1);
#endif

	return freq;
}

// continue emulation without reset processor
u32 I8048_Cont(sI8048* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I8048_Cpu != NULL) I8048_Stop(pwm);

	// initialize time synchronization
	freq = I8048_SyncInit(cpu, pwm, freq);

	// current CPU descriptor
	I8048_Cpu = cpu;
	dsb();

	// execute program
#if USE_MINIVGA				// use mini-VGA display
	VgaCore1Exec(I8048_ExecCore1);
#else
	Core1Exec(I8048_ExecCore1);
#endif

	return freq;
}

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I8048_Stop(int pwm)
{
	// get CPU descriptor
	sI8048* cpu = (sI8048*)I8048_Cpu;

	// check if emulation is running
	if (cpu == NULL) return;

	// request to stop emulation
	cpu->stop = 1;

	// wait to complete
	while (I8048_Cpu != NULL) { dsb(); }

	// terminate time synchronization (stop PWM counter)
	I8048_SyncTerm(pwm);

#if !USE_MINIVGA				// use mini-VGA display
	// core 1 reset
	Core1Reset();
#endif
}

#endif // USE_EMU_I8048
