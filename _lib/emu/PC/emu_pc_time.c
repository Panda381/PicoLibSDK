
// ****************************************************************************
//
//                       IBM PC Emulator - System time
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

u8 PC_TimerMode = 0x36; // time counter 0 setup, port 40h
u16 PC_TimerDiv = 65535; // time counter divider, port 40h (default: 0.0549255 sec, 18.2065Hz)
Bool PC_TimerHigh = False; // flip/flop - read/write high byte
u16 PC_TimerLatch = 65535; // latch

// setup timer to default state (should be called on program start)
void PC_TimerDef()
{
	PC_TimerMode = 0x36; // time counter 0 setup, port 40h
	PC_TimerDiv = 65535; // time counter divider, port 40h (default: 0.0549255 sec, 18.2065Hz)
	PC_TimerHigh = False; // flip/flop - read/write high byte
	PC_TimerLatch = 65535; // latch
}

// INT 1Ah service - function
void PC_Int1Afnc()
{
	// pointer to CPU
	sI8086* cpu = PC_Cpu;

	// get AH function
	u8 ah = cpu->ah;

	switch (ah)
	{
	// read system time
	case 0x00:
		// 0040:006C (4) timer ticks (55 ms) since midnight (INT 08h, 18.2065 per second, 54.9255 ms) ... crystal 14.31818/12 = 1.193182 MHz, /65536 = 18.20651 Hz
		cpu->dx = *(u16*)&PC_RAM_BASE[0x046c]; // time LOW
		cpu->cx = *(u16*)&PC_RAM_BASE[0x046e]; // time HIGH

		// 0040:0070 (1) timer overflow (midnight) ... overflow on value 1800B0H = 1573040
		cpu->al = PC_RAM_BASE[0x0470]; // midnight flag;
		break;

	// write system time
	case 0x01:
		// 0040:006C (4) timer ticks (55 ms) since midnight (INT 08h, 18.2065 per second, 54.9255 ms) ... crystal 14.31818/12 = 1.193182 MHz, /65536 = 18.20651 Hz
		*(u16*)&PC_RAM_BASE[0x046c] = cpu->dx; // time LOW
		*(u16*)&PC_RAM_BASE[0x046e] = cpu->cx; // time HIGH

		// 0040:0070 (1) timer overflow (midnight) ... overflow on value 1800B0H = 1573040
		PC_RAM_BASE[0x0470] = 0; // midnight flag;
		break;

	// unsupported
	default:
		{
			// load flags
			u16 f = I8086_GetWord(cpu, cpu->ss, cpu->sp+4);

			// set error flag CY
			f |= I8086_CF;

			// write new flags
			I8086_SetWord(cpu, cpu->ss, cpu->sp+4, f);
		}
		break;
	}
}
