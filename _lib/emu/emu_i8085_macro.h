
// ****************************************************************************
//
//                           I8085 CPU Emulator - macro
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

// INC var (var = 8-bit variable)
//	C ... not affected
//	V ... set if result is 0x80
//	P ... parity
//	AC ... set if result is 0xN0
//	K ... V xor S
//	Z ... set if result is 0
//	S ... copy from result
#define I8085_INC(var) {					\
		u8 temp = var + 1;				\
		var = temp;					\
		u8 f = cpu->f & I8085_C;			\
		f |= I8085_FlagParTab[temp];			\
		if (temp == 0x80) f |= I8085_V;			\
		if ((temp & 0x0f) == 0) f |= I8085_AC;		\
		f |= ((f >> 2) ^ (f << 4)) & I8085_K;		\
		cpu->f = f; }

// DEC var (var = 8-bit variable)
//	C ... not affected
//	V ... set if result is 0x7f
//	P ... parity
//	AC ... set if result is not 0xNf (inverted borrow)
//	K ... V xor S
//	Z ... set if result is 0
//	S ... copy from result
#define I8085_DEC(var) {					\
		u8 temp = var - 1;				\
		var = temp;					\
		u8 f = cpu->f & I8085_C;			\
		f |= I8085_FlagParTab[temp];			\
		if (temp == 0x7f) f |= I8085_V;			\
		if ((temp & 0x0f) != 0x0f) f |= I8085_AC;	\
		f |= ((f >> 2) ^ (f << 4)) & I8085_K;		\
		cpu->f = f; }

// ADD HL,val (val = 16-bit value)
//	C ... carry from bit 15
//	V ... set if overflow
//	other ... not affected
#define I8085_ADD16(val) {					\
		u16 hl = cpu->hl;				\
		u16 nn = (val);					\
		u32 nnnn = (u32)hl + nn;			\
		cpu->hl = (u16)nnnn;				\
		u8 f = cpu->f & ~(I8085_C | I8085_V);		\
		f |= nnnn >> 16; /* I8085_C */;			\
		u32 xor = hl ^ nn ^ nnnn;			\
		f |= ((xor ^ (xor >> 1)) >> 14) & I8085_V;	\
		cpu->f = f; }

// ADD A,val (val = 8-bit value or register)
//	C ... carry from bit 7
//	V ... set if overflow
//	P ... parity
//	AC ... carry from bit 3
//	K ... V xor S
//	Z ... set if result is 0
//	S ... copy from result
#define I8085_ADD(val) {					\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp + temp2;			\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = (u8)(temp16 >> 8); /* carry */		\
		f |= xor & I8085_AC;				\
		f |= ((xor ^ (xor >> 1)) >> 6) & I8085_V;	\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		f |= I8085_FlagParTab[temp];			\
		f |= ((f >> 2) ^ (f << 4)) & I8085_K;		\
		cpu->f = f; }

// ADC A,val (val = 8-bit value or register)
//	C ... carry from bit 7
//	V ... set if overflow
//	P ... parity
//	AC ... carry from bit 3
//	K ... V xor S
//	Z ... set if result is 0
//	S ... copy from result
#define I8085_ADC(val) {					\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp + temp2 + (cpu->f & I8085_C);	\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = (u8)(temp16 >> 8); /* carry */		\
		f |= xor & I8085_AC;				\
		f |= ((xor ^ (xor >> 1)) >> 6) & I8085_V;	\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		f |= I8085_FlagParTab[temp];			\
		f |= ((f >> 2) ^ (f << 4)) & I8085_K;		\
		cpu->f = f; }

// SUB val (val = 8-bit value or register)
//	C ... borrow from bit 7
//	V ... set if overflow
//	P ... parity
//	AC ... borrow from bit 3
//	K ... V xor S
//	Z ... set if result is 0
//	S ... copy from result
#define I8085_SUB(val) {					\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp2 - temp;			\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = (u8)((temp16 >> 8) & I8085_C); 		\
		f |= (~xor) & I8085_AC;				\
		f |= ((xor ^ (xor >> 1)) >> 6) & I8085_V;	\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		f |= I8085_FlagParTab[temp];			\
		f |= ((f >> 2) ^ (f << 4)) & I8085_K;		\
		cpu->f = f; }

// SBC val (val = 8-bit value or register)
//	C ... borrow from bit 7
//	V ... set if overflow
//	P ... parity
//	AC ... borrow from bit 3
//	K ... V xor S
//	Z ... set if result is 0
//	S ... copy from result
#define I8085_SBC(val) {					\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp2 - temp - (cpu->f & I8085_C);	\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = (u8)((temp16 >> 8) & I8085_C);		\
		f |= (~xor) & I8085_AC;				\
		f |= ((xor ^ (xor >> 1)) >> 6) & I8085_V;	\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		f |= I8085_FlagParTab[temp];			\
		f |= ((f >> 2) ^ (f << 4)) & I8085_K;		\
		cpu->f = f; }

// AND val (val = 8-bit value or register)
//	C ... reset
//	V ... reset
//	P ... parity
//	AC ... set
//	K ... as S
//	Z ... set if result is 0
//	S ... copy from result
#define I8085_AND(val) {					\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		temp = temp & temp2;				\
		cpu->a = temp;					\
		u8 f = I8085_FlagParTab[temp] | I8085_AC;	\
		f |= (f >> 2) & I8085_K;			\
		cpu->f = f; }

// XOR val (val = 8-bit value or register)
//	C ... reset
//	V ... reset
//	P ... parity
//	AC ... reset
//	K ... as S
//	Z ... set if result is 0
//	S ... copy from result
#define I8085_XOR(val) {			\
		u8 temp = (val);		\
		temp = temp ^ cpu->a;		\
		cpu->a = temp;			\
		u8 f = I8085_FlagParTab[temp];	\
		f |= (f >> 2) & I8085_K;	\
		cpu->f = f; }

// OR val (val = 8-bit value or register)
//	C ... reset
//	V ... reset
//	P ... parity
//	AC ... reset
//	K ... as S
//	Z ... set if result is 0
//	S ... copy from result
#define I8085_OR(val) {				\
		u8 temp = (val);		\
		temp = temp | cpu->a;		\
		cpu->a = temp;			\
		u8 f = I8085_FlagParTab[temp];	\
		f |= (f >> 2) & I8085_K;	\
		cpu->f = f; }

// CP val (val = 8-bit value or register)
//	C ... borrow from bit 7
//	V ... set if overflow
//	P ... parity
//	AC ... borrow from bit 3
//	K ... V xor S
//	Z ... set if result is 0
//	S ... copy from result
#define I8085_CP(val) {						\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp2 - temp;			\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = (u8)((temp16 >> 8) & I8085_C); 		\
		f |= (~xor) & I8085_AC;				\
		f |= ((xor ^ (xor >> 1)) >> 6) & I8085_V;	\
		temp = (u8)temp16;				\
		f |= I8085_FlagParTab[temp];			\
		f |= ((f >> 2) ^ (f << 4)) & I8085_K;		\
		cpu->f = f; }

// push 16-bit register to the stack
#define I8085_PUSH(regL, regH)	{ cpu->writemem(--cpu->sp, regH); cpu->writemem(--cpu->sp, regL); }

// pop 16-bit register from the stack
#define I8085_POP(regL, regH)	{ regL = cpu->readmem(cpu->sp++); regH = cpu->readmem(cpu->sp++); }

// RET
#define I8085_RET() { I8085_POP(cpu->pcl, cpu->pch); }

// CALL, RST
#define I8085_CALL(addr) { I8085_PUSH(cpu->pcl, cpu->pch); cpu->pc = addr; }
