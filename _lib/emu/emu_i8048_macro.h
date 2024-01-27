
// ****************************************************************************
//
//                           I8048 CPU Emulator - macro
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

// ADD A,val (val = 8-bit value)
#define I8048_ADD(val) {					\
		u8 tempn = (val);				\
		u8 tempa = cpu->a;				\
		int temp16 = tempa + tempn;			\
		int tempxor = tempa ^ tempn ^ temp16;		\
		u8 f = cpu->psw & ~(I8048_C | I8048_AC);	\
		f |= (u8)(temp16 >> 1); /* I8048_C */		\
		cpu->a = (u8)temp16;				\
		f |= (u8)(tempxor << 2) & I8048_AC;		\
		cpu->psw = f; }

// ADDC A,val (val = 8-bit value)
#define I8048_ADDC(val) {					\
		u8 tempn = (val);				\
		u8 tempa = cpu->a;				\
		u8 f = cpu->psw;				\
		int temp16 = tempa + tempn + (f >> 7);		\
		int tempxor = tempa ^ tempn ^ temp16;		\
		f &= ~(I8048_C | I8048_AC);			\
		f |= (u8)(temp16 >> 1); /* I8048_C */		\
		cpu->a = (u8)temp16;				\
		f |= (u8)(tempxor << 2) & I8048_AC;		\
		cpu->psw = f; }

// push PC and PSW into stack
#define I8048_PUSH_PC_PSW() {					\
		u8 psw = cpu->psw;				\
		u8 inx = psw & I8048_STACKMASK;			\
		psw &= 0xf0;					\
		cpu->ram[8 + 2*inx] = cpu->pcl;			\
		cpu->ram[8 + 2*inx + 1] = cpu->pch | psw;	\
		inx = (inx + 1) & I8048_STACKMASK;		\
		cpu->psw = psw | inx; }

// pop PC and PSW from stack
#define I8048_POP_PC_PSW() {					\
		u8 inx = (cpu->psw - 1) & I8048_STACKMASK;	\
		cpu->pcl = cpu->ram[8 + 2*inx];			\
		u8 n = cpu->ram[8 + 2*inx + 1];			\
		cpu->pch = n & 0x0f;				\
		cpu->psw = inx | (n & 0xf0);			\
		cpu->regbase = ((n & I8048_BS) == 0) ? 0 : 24; }

// pop PC from stack (without PSW)
#define I8048_POP_PC() {					\
		u8 psw = cpu->psw;				\
		u8 inx = (psw - 1) & I8048_STACKMASK;		\
		cpu->pcl = cpu->ram[8 + 2*inx];			\
		cpu->pch = cpu->ram[8 + 2*inx + 1] & 0x0f;	\
		cpu->psw = (psw & 0xf0) | inx; }
