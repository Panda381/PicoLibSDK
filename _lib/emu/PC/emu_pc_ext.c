
// ****************************************************************************
//
//                       IBM PC Emulator - INT 15h extension
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

// flag - waiting in fuction 86h
Bool PC_Int15Wait = False;
u32 PC_Int15Start = 0; // start time

// INT 15h service - function; returns instruction code I8086_INS_IRET if continue, or I8086_INS_NOP if waiting in loop
u8 PC_Int15fnc()
{
	// return instruction IRET
	u8 res = I8086_INS_IRET;

	// pointer to CPU
	sI8086* cpu = PC_Cpu;

	// get AH function
	u8 ah = cpu->ah;

	// load flags
	u16 f = I8086_GetWord(cpu, cpu->ss, cpu->sp+4);

	// clear error flag CY
	f &= ~I8086_CF;

	switch (ah)
	{
	// wait
	case 0x86:
		{
#define INT15_MAXTIME	10000000	// max. 10 seconds

			// get current time
			u32 t = Time();

			// check elapsed time - if it's more than say 60 seconds, it's some kind of inconsistency error
			if ((u32)(t - PC_Int15Start) > (u32)(6*INT15_MAXTIME)) PC_Int15Wait = False;

			// start waiting
			if (!PC_Int15Wait)
			{
				PC_Int15Start = t;
				PC_Int15Wait = True;
			}

			// get required time
			u32 d = ((u32)cpu->cx << 16) | cpu->dx;
			if (d > INT15_MAXTIME) d = INT15_MAXTIME;

			// check stop time
			if ((u32)(t - PC_Int15Start) >= d)
			{
				// stop
				PC_Int15Wait = False;
			}
			else
			{
				// wait - return instruction NOP
				res = I8086_INS_NOP;
			}
		}
		break;

	// get extended memory (emulator: no extended memory)
	case 0x88:
		cpu->ax = 0;
		break;

	// unsupported function
	default:
		{
			// set error flag CY
			f |= I8086_CF;

			// set error code
			cpu->ah = 1;
		}
		break;
	}

	// write new flags
	I8086_SetWord(cpu, cpu->ss, cpu->sp+4, f);

	return res;
}
