
// ****************************************************************************
//
//                            Z80 CPU Emulator - macro
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
//	N ... reset
//	PV ... set if result is 0x80
//	X ... copy from result
//	H ... set if result is 0xN0
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_INC(var) {					\
		u8 temp = var + 1;			\
		var = temp;				\
		u8 f = cpu->f & Z80_C;			\
		f |= temp & (Z80_S | Z80_X | Z80_Y);	\
		if (temp == 0) f |= Z80_Z;		\
		if (temp == 0x80) f |= Z80_PV;		\
		if ((temp & 0x0f) == 0) f |= Z80_H;	\
		cpu->f = f; }

// DEC var (var = 8-bit variable)
//	C ... not affected
//	N ... set
//	PV ... set if result is 0x7f
//	X ... copy from result
//	H ... set if result is 0xNf
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_DEC(var) {					\
		u8 temp = var - 1;			\
		var = temp;				\
		u8 f = cpu->f & Z80_C;			\
		f |= temp & (Z80_S | Z80_X | Z80_Y);	\
		if (temp == 0) f |= Z80_Z;		\
		f |= Z80_N;				\
		if (temp == 0x7f) f |= Z80_PV;		\
		if ((temp & 0x0f) == 0x0f) f |= Z80_H;	\
		cpu->f = f; }

// ADD var,val (var = 16-bit variable, val = 16-bit value)
//	C ... carry from bit 15
//	N ... reset
//	PV ... not affected
//	X ... copy from result HIGH
//	H ... carry from bit 11
//	Y ... copy from result HIGH
//	Z ... not affected
//	S ... not affected
#define Z80_ADD16(var, val) {					\
		u16 nn = (var);					\
		u16 nn2 = (val);				\
		u32 nnnn = (u32)nn + nn2;			\
		(var) = (u16)nnnn;				\
		u8 f = cpu->f & (Z80_PV | Z80_Z | Z80_S);	\
		f |= ((nnnn ^ nn ^ nn2) >> 8) & Z80_H;		\
		f |= (nnnn >> 8) & (Z80_X | Z80_Y);		\
		f |= (nnnn >> 16) & Z80_C;			\
		cpu->f = f; }

// ADD A,val (val = 8-bit value or register)
//	C ... carry from bit 7
//	N ... reset
//	PV ... set if overflow
//	X ... copy from result
//	H ... carry from bit 3
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_ADD(val) {						\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp + temp2;			\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = (u8)(temp16 >> 8); /* carry */		\
		f |= xor & Z80_H;				\
		f |= ((xor ^ (xor >> 1)) >> 5) & Z80_PV;	\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		if (temp == 0) f |= Z80_Z;			\
		f |= temp & (Z80_S | Z80_X | Z80_Y);		\
		cpu->f = f; }

// ADC A,val (val = 8-bit value or register)
//	C ... carry from bit 7
//	N ... reset
//	PV ... set if overflow
//	X ... copy from result
//	H ... carry from bit 3
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_ADC(val) {						\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp + temp2 + (cpu->f & Z80_C);	\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = (u8)(temp16 >> 8); /* carry */		\
		f |= xor & Z80_H;				\
		f |= ((xor ^ (xor >> 1)) >> 5) & Z80_PV;	\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		if (temp == 0) f |= Z80_Z;			\
		f |= temp & (Z80_S | Z80_X | Z80_Y);		\
		cpu->f = f; }

// SUB val (val = 8-bit value or register)
//	C ... borrow from bit 7
//	N ... set
//	PV ... set if overflow
//	X ... copy from result
//	H ... borrow from bit 3
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_SUB(val) {						\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp2 - temp;			\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = (u8)((temp16 >> 8) & Z80_C); /* borrow */\
		f |= xor & Z80_H;				\
		f |= ((xor ^ (xor >> 1)) >> 5) & Z80_PV;	\
		f |= Z80_N;					\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		if (temp == 0) f |= Z80_Z;			\
		f |= temp & (Z80_S | Z80_X | Z80_Y);		\
		cpu->f = f; }

// SBC val (val = 8-bit value or register)
//	C ... borrow from bit 7
//	N ... set
//	PV ... set if overflow
//	X ... copy from result
//	H ... borrow from bit 3
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_SBC(val) {						\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp2 - temp - (cpu->f & Z80_C);	\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = (u8)((temp16 >> 8) & Z80_C); /* borrow */\
		f |= xor & Z80_H;				\
		f |= ((xor ^ (xor >> 1)) >> 5) & Z80_PV;	\
		f |= Z80_N;					\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		if (temp == 0) f |= Z80_Z;			\
		f |= temp & (Z80_S | Z80_X | Z80_Y);		\
		cpu->f = f; }

// AND val (val = 8-bit value or register)
//	C ... reset
//	N ... reset
//	PV ... parity
//	X ... copy from result
//	H ... set
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_AND(val) {				\
		u8 temp = (val);		\
		temp = temp & cpu->a;		\
		cpu->a = temp;			\
		u8 f = Z80_FlagParTab[temp];	\
		f |= Z80_H;			\
		cpu->f = f; }

// XOR val (val = 8-bit value or register)
//	C ... reset
//	N ... reset
//	PV ... parity
//	X ... copy from result
//	H ... reset
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_XOR(val) {				\
		u8 temp = (val);		\
		temp = temp ^ cpu->a;		\
		cpu->a = temp;			\
		u8 f = Z80_FlagParTab[temp];	\
		cpu->f = f; }

// OR val (val = 8-bit value or register)
//	C ... reset
//	N ... reset
//	PV ... parity
//	X ... copy from result
//	H ... reset
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_OR(val) {				\
		u8 temp = (val);		\
		temp = temp | cpu->a;		\
		cpu->a = temp;			\
		u8 f = Z80_FlagParTab[temp];	\
		cpu->f = f; }

// CP val (val = 8-bit value or register)
//	C ... borrow from bit 7
//	N ... set
//	PV ... set if overflow
//	X ... copy from 2nd operand
//	H ... borrow from bit 3
//	Y ... copy from 2nd operand
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_CP(val) {						\
		u8 temp2 = (val);				\
		u8 temp = cpu->a;				\
		u16 temp16 = temp - temp2;			\
		u16 xor = temp ^ temp2 ^ temp16;		\
		u8 f = (u8)((temp16 >> 8) & Z80_C); /* borrow */\
		f |= xor & Z80_H;				\
		f |= ((xor ^ (xor >> 1)) >> 5) & Z80_PV;	\
		f |= Z80_N;					\
		temp = (u8)temp16;				\
		if (temp == 0) f |= Z80_Z;			\
		f |= temp & Z80_S;				\
		f |= temp2 & (Z80_X | Z80_Y);			\
		cpu->f = f; }

// push 16-bit register to the stack
#define Z80_PUSH(regL, regH)	{ cpu->writemem(--cpu->sp, regH); cpu->writemem(--cpu->sp, regL); }

// pop 16-bit register from the stack
#define Z80_POP(regL, regH)	{ regL = cpu->readmem(cpu->sp++); regH = cpu->readmem(cpu->sp++); }

// RET
#define Z80_RET() { Z80_POP(cpu->pcl, cpu->pch); }

// CALL, RST
#define Z80_CALL(addr) { Z80_PUSH(cpu->pcl, cpu->pch); cpu->pc = addr; }

// RLC r (reg = register or variable)
//	C ... carry from bit 7
//	N ... reset
//	PV ... parity
//	X ... copy from result
//	H ... reset
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_RLC(reg) {					\
		u8 temp = reg;				\
		temp = (temp << 1) | (temp >> 7);	\
		reg = temp;				\
		u8 f = Z80_FlagParTab[temp];		\
		f |= temp & Z80_C;			\
		cpu->f = f; }

// RRC r (reg = register or variable)
//	C ... carry from bit 0
//	N ... reset
//	PV ... parity
//	X ... copy from result
//	H ... reset
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_RRC(reg) {					\
		u8 temp = reg;				\
		u8 f = temp & Z80_C;			\
		temp = (temp >> 1) | (temp << 7);	\
		reg = temp;				\
		f |= Z80_FlagParTab[temp];		\
		cpu->f = f; }

// RL r (reg = register or variable)
//	C ... carry from bit 7
//	N ... reset
//	PV ... parity
//	X ... copy from result
//	H ... reset
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_RL(reg) {					\
		u8 temp = reg;				\
		u8 f = temp >> 7; /* Z80_C */		\
		temp = (temp << 1) | (cpu->f & Z80_C);	\
		reg = temp;				\
		f |= Z80_FlagParTab[temp];		\
		cpu->f = f; }

// RR r (reg = register or variable)
//	C ... carry from bit 0
//	N ... reset
//	PV ... parity
//	X ... copy from result
//	H ... reset
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_RR(reg) {					\
		u8 temp = reg;				\
		u8 f = temp & Z80_C;			\
		temp = (temp >> 1) | (cpu->f << 7);	\
		reg = temp;				\
		f |= Z80_FlagParTab[temp];		\
		cpu->f = f; }

// SLA r (reg = register or variable)
//	C ... carry from bit 7
//	N ... reset
//	PV ... parity
//	X ... copy from result
//	H ... reset
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_SLA(reg) {					\
		u8 temp = reg;				\
		u8 f = temp >> 7; /* Z80_C */		\
		temp = (temp << 1);			\
		reg = temp;				\
		f |= Z80_FlagParTab[temp];		\
		cpu->f = f; }

// SRA r (reg = register or variable)
//	C ... carry from bit 0
//	N ... reset
//	PV ... parity
//	X ... copy from result
//	H ... reset
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_SRA(reg) {					\
		u8 temp = reg;				\
		u8 f = temp & Z80_C;			\
		temp = ((s8)temp >> 1);			\
		reg = temp;				\
		f |= Z80_FlagParTab[temp];		\
		cpu->f = f; }

// SLL r (reg = register or variable)
//	C ... carry from bit 7
//	N ... reset
//	PV ... parity
//	X ... copy from result
//	H ... reset
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_SLL(reg) {					\
		u8 temp = reg;				\
		u8 f = temp >> 7; /* Z80_C */		\
		temp = (temp << 1) | 1;			\
		reg = temp;				\
		f |= Z80_FlagParTab[temp];		\
		cpu->f = f; }

// SRL r (reg = register or variable)
//	C ... carry from bit 0
//	N ... reset
//	PV ... parity
//	X ... copy from result
//	H ... reset
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result (set if result is >= 0x80, i.e. negative)
#define Z80_SRL(reg) {					\
		u8 temp = reg;				\
		u8 f = temp & Z80_C;			\
		temp >>= 1;				\
		reg = temp;				\
		f |= Z80_FlagParTab[temp];		\
		cpu->f = f; }

// BIT r (bit = tested bit, val = value)
//	C ... not affected
//	N ... reset
//	PV ... set if specified bit is 0 (set like Z)
//	X ... copy from "value AND bit"
//	H ... set
//	Y ... copy from "value AND bit"
//	Z ... set if specified bit is 0
//	S ... copy from "value AND bit"
#define Z80_BIT(bit, val) {				\
		u8 temp = (val) & BIT(bit);		\
		u8 f = cpu->f & Z80_C;			\
		if (temp == 0) f |= Z80_Z | Z80_PV;	\
		f |= Z80_H;				\
		f |= temp & (Z80_S | Z80_X | Z80_Y);	\
		cpu->f = f; }

// IN r,(BC) (reg = register or variable)
//	C ... not affected
//	N ... reset
//	PV ... parity
//	X ... copy from result
//	H ... reset
//	Y ... copy from result
//	Z ... set if result is 0
//	S ... copy from result
#define Z80_IN(reg) {					\
		u8 temp = cpu->readport(cpu->bc);	\
		reg = temp;				\
		u8 f = cpu->f & Z80_C;			\
		f |= Z80_FlagParTab[temp];		\
		cpu->f = f; }

// SBC HL,reg (val = 16-bit value)
//	C ... borrow from bit 15
//	N ... set
//	PV ... set if overflow
//	X ... copy from result HIGH
//	H ... borrow from bit 11
//	Y ... copy from result HIGH
//	Z ... set if zero
//	S ... copy from result HIGH
#define Z80_SBC16(val) {					\
		u16 nn = cpu->hl;				\
		u16 nn2 = (val);				\
		u32 nnnn = (u32)nn - nn2 - (cpu->f & Z80_C);	\
		u32 xor = nn ^ nn2 ^ nnnn;			\
		u8 f = (xor >> 8) & Z80_H;			\
		f |= ((xor ^ (xor >> 1)) >> 13) & Z80_PV;	\
		f |= (nnnn >> 16) & Z80_C;			\
		f |= Z80_N;					\
		nn = (u16)nnnn;					\
		cpu->hl = nn;					\
		if (nn == 0) f |= Z80_Z;			\
		f |= (nn >> 8) & (Z80_S | Z80_X | Z80_Y);	\
		cpu->f = f; }

// ADC HL,reg (val = 16-bit value)
//	C ... carry from bit 15
//	N ... reset
//	PV ... set if overflow
//	X ... copy from result HIGH
//	H ... carry from bit 11
//	Y ... copy from result HIGH
//	Z ... set if zero
//	S ... copy from result HIGH
#define Z80_ADC16(val) {					\
		u16 nn = cpu->hl;				\
		u16 nn2 = (val);				\
		u32 nnnn = (u32)nn + nn2 + (cpu->f & Z80_C);	\
		u32 xor = nn ^ nn2 ^ nnnn;			\
		u8 f = (xor >> 8) & Z80_H;			\
		f |= ((xor ^ (xor >> 1)) >> 13) & Z80_PV;	\
		f |= (nnnn >> 16) & Z80_C;			\
		nn = (u16)nnnn;					\
		cpu->hl = nn;					\
		if (nn == 0) f |= Z80_Z;			\
		f |= (nn >> 8) & (Z80_S | Z80_X | Z80_Y);	\
		cpu->f = f; }
