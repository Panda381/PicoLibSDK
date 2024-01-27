
// ****************************************************************************
//
//                     Sharp X80 (LR35902) CPU Emulator - macro
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
//	H ... set if result is 0xN0
//	N ... reset
//	Z ... set if result is 0
#define X80_INC(var) {					\
		u8 temp = var + 1;			\
		var = temp;				\
		u8 f = cpu->f & X80_C;			\
		if (temp == 0) f |= X80_Z;		\
		if ((temp & 0x0f) == 0) f |= X80_H;	\
		cpu->f = f; }

// DEC var (var = 8-bit variable)
//	C ... not affected
//	H ... set if result is 0xNf
//	N ... set
//	Z ... set if result is 0
#define X80_DEC(var) {					\
		u8 temp = var - 1;			\
		var = temp;				\
		u8 f = cpu->f & X80_C;			\
		if (temp == 0) f |= X80_Z;		\
		f |= X80_N;				\
		if ((temp & 0x0f) == 0x0f) f |= X80_H;	\
		cpu->f = f; }

// ADD var,val (var = 16-bit variable, val = 16-bit value)
//	C ... carry from bit 15
//	H ... carry from bit 11
//	N ... reset
//	Z ... not affected
#define X80_ADD16(var, val) {					\
		u16 nn = (var);					\
		u16 nn2 = (val);				\
		u32 nnnn = (u32)nn + nn2;			\
		(var) = (u16)nnnn;				\
		u8 f = cpu->f & X80_Z;				\
		if (((nnnn ^ nn ^ nn2) & B12) != 0) f |= X80_H;	\
		if (nnnn > 0xffff) f |= X80_C;			\
		cpu->f = f; }

// ADD A,val (val = 8-bit value or register)
//	C ... carry from bit 7
//	H ... carry from bit 3
//	N ... reset
//	Z ... set if result is 0
#define X80_ADD(val) {						\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp + temp2;			\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = 0;					\
		if ((temp16 & 0x100) != 0) f = X80_C;		\
		if ((xor & 0x10) != 0) f |= X80_H;		\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		if (temp == 0) f |= X80_Z;			\
		cpu->f = f; }

// ADC A,val (val = 8-bit value or register)
//	C ... carry from bit 7
//	H ... carry from bit 3
//	N ... reset
//	Z ... set if result is 0
#define X80_ADC(val) {						\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp + temp2;			\
		if ((cpu->f & X80_C) != 0) temp16++;		\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = 0;					\
		if ((temp16 & 0x100) != 0) f = X80_C;		\
		if ((xor & 0x10) != 0) f |= X80_H;		\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		if (temp == 0) f |= X80_Z;			\
		cpu->f = f; }

// SUB val (val = 8-bit value or register)
//	C ... borrow from bit 7
//	H ... borrow from bit 3
//	N ... set
//	Z ... set if result is 0
#define X80_SUB(val) {						\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp2 - temp;			\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = X80_N;					\
		if ((temp16 & 0x100) != 0) f |= X80_C;		\
		if ((xor & 0x10) != 0) f |= X80_H;		\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		if (temp == 0) f |= X80_Z;			\
		cpu->f = f; }

// SBC val (val = 8-bit value or register)
//	C ... borrow from bit 7
//	H ... borrow from bit 3
//	N ... set
//	Z ... set if result is 0
#define X80_SBC(val) {						\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp2 - temp;			\
		if ((cpu->f & X80_C) != 0) temp16--;		\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = X80_N;					\
		if ((temp16 & 0x100) != 0) f |= X80_C;		\
		if ((xor & 0x10) != 0) f |= X80_H;		\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		if (temp == 0) f |= X80_Z;			\
		cpu->f = f; }

// AND val (val = 8-bit value or register)
//	C ... reset
//	H ... set
//	N ... reset
//	Z ... set if result is 0
#define X80_AND(val) {				\
		u8 temp = (val);		\
		temp = temp & cpu->a;		\
		cpu->a = temp;			\
		u8 f = X80_H;			\
		if (temp == 0) f |= X80_Z;	\
		cpu->f = f; }

// XOR val (val = 8-bit value or register)
//	C ... reset
//	H ... reset
//	N ... reset
//	Z ... set if result is 0
#define X80_XOR(val) {				\
		u8 temp = (val);		\
		temp = temp ^ cpu->a;		\
		cpu->a = temp;			\
		u8 f = 0;			\
		if (temp == 0) f |= X80_Z;	\
		cpu->f = f; }

// OR val (val = 8-bit value or register)
//	C ... reset
//	H ... reset
//	N ... reset
//	Z ... set if result is 0
#define X80_OR(val) {				\
		u8 temp = (val);		\
		temp = temp | cpu->a;		\
		cpu->a = temp;			\
		u8 f = 0;			\
		if (temp == 0) f |= X80_Z;	\
		cpu->f = f; }

// CP val (val = 8-bit value or register)
//	C ... borrow from bit 7
//	H ... borrow from bit 3
//	N ... set
//	Z ... set if result is 0
#define X80_CP(val) {						\
		u8 temp2 = (val);				\
		u8 temp = cpu->a;				\
		u16 temp16 = temp - temp2;			\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = X80_N;					\
		if ((temp16 & 0x100) != 0) f |= X80_C;		\
		if ((xor & 0x10) != 0) f |= X80_H;		\
		temp = (u8)temp16;				\
		if (temp == 0) f |= X80_Z;			\
		cpu->f = f; }

// push 16-bit register to the stack
#define X80_PUSH(regL, regH)	{ cpu->writemem(--cpu->sp, regH); cpu->writemem(--cpu->sp, regL); }

// pop 16-bit register from the stack
#define X80_POP(regL, regH)	{ regL = cpu->readmem(cpu->sp++); regH = cpu->readmem(cpu->sp++); }

// RET
#define X80_RET() { X80_POP(cpu->pcl, cpu->pch); }

// CALL, RST
#define X80_CALL(addr) { X80_PUSH(cpu->pcl, cpu->pch); cpu->pc = addr; }

// RLC r (reg = register or variable)
//	C ... carry from bit 7
//	H ... reset
//	N ... reset
//	Z ... set if result is 0
#define X80_RLC(reg) {					\
		u8 temp = reg;				\
		temp = (temp << 1) | (temp >> 7);	\
		reg = temp;				\
		u8 f = 0;				\
		if ((temp & B0) != 0) f = X80_C;	\
		if (temp == 0) f |= X80_Z;		\
		cpu->f = f; }

// RRC r (reg = register or variable)
//	C ... carry from bit 0
//	H ... reset
//	N ... reset
//	Z ... set if result is 0
#define X80_RRC(reg) {					\
		u8 temp = reg;				\
		temp = (temp >> 1) | (temp << 7);	\
		reg = temp;				\
		u8 f = 0;				\
		if ((temp & B7) != 0) f = X80_C;	\
		if (temp == 0) f |= X80_Z;		\
		cpu->f = f; }

// RL r (reg = register or variable)
//	C ... carry from bit 7
//	H ... reset
//	N ... reset
//	Z ... set if result is 0
#define X80_RL(reg) {					\
		u8 temp = reg;				\
		u8 f = ((temp & B7) != 0) ? X80_C : 0;	\
		temp <<= 1;				\
		if ((cpu->f & X80_C) != 0) temp |= B0;	\
		reg = temp;				\
		if (temp == 0) f |= X80_Z;		\
		cpu->f = f; }

// RR r (reg = register or variable)
//	C ... carry from bit 0
//	H ... reset
//	N ... reset
//	Z ... set if result is 0
#define X80_RR(reg) {					\
		u8 temp = reg;				\
		u8 f = ((temp & B0) != 0) ? X80_C : 0;	\
		temp >>= 1;				\
		if ((cpu->f & X80_C) != 0) temp |= B7;	\
		reg = temp;				\
		if (temp == 0) f |= X80_Z;		\
		cpu->f = f; }

// SLA r (reg = register or variable)
//	C ... carry from bit 7
//	H ... reset
//	N ... reset
//	Z ... set if result is 0
#define X80_SLA(reg) {					\
		u8 temp = reg;				\
		u8 f = ((temp & B7) != 0) ? X80_C : 0;	\
		temp = (temp << 1);			\
		reg = temp;				\
		if (temp == 0) f |= X80_Z;		\
		cpu->f = f; }

// SRA r (reg = register or variable)
//	C ... carry from bit 0
//	H ... reset
//	N ... reset
//	Z ... set if result is 0
#define X80_SRA(reg) {					\
		u8 temp = reg;				\
		u8 f = ((temp & B0) != 0) ? X80_C : 0;	\
		temp = ((s8)temp >> 1);			\
		reg = temp;				\
		if (temp == 0) f |= X80_Z;		\
		cpu->f = f; }

// SWAP r (reg = register or variable)
//	C ... reset
//	H ... reset
//	N ... reset
//	Z ... set if result is 0
#define X80_SWAP(reg) {					\
		u8 temp = reg;				\
		temp = (temp >> 4) | (temp << 4);	\
		reg = temp;				\
		u8 f = 0;				\
		if (temp == 0) f = X80_Z;		\
		cpu->f = f; }

// SRL r (reg = register or variable)
//	C ... carry from bit 0
//	H ... reset
//	N ... reset
//	Z ... set if result is 0
#define X80_SRL(reg) {					\
		u8 temp = reg;				\
		u8 f = ((temp & B0) != 0) ? X80_C : 0;	\
		temp >>= 1;				\
		reg = temp;				\
		if (temp == 0) f |= X80_Z;		\
		cpu->f = f; }

// BIT r (bit = tested bit, val = value)
//	C ... not affected
//	H ... set
//	N ... reset
//	Z ... set if specified bit is 0
#define X80_BIT(bit, val) {				\
		u8 temp = (val) & BIT(bit);		\
		u8 f = cpu->f & X80_C;			\
		if (temp == 0) f |= X80_Z;		\
		f |= X80_H;				\
		cpu->f = f; }
