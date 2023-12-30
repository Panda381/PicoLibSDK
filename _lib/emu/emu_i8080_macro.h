
// ****************************************************************************
//
//                           I8080 CPU Emulator - macro
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

// load byte from program memory
#define PROGBYTE()	cpu->readmem(cpu->pc++)

// load word from program memory to result number
#define PROGWORD(res)	{ u8 temp = PROGBYTE(); res = temp | ((u16)PROGBYTE() << 8); }

// INC var (var = 8-bit variable)
//	C ... not affected
//	P ... parity
//	AC ... set if result is 0xN0
//	Z ... set if result is 0
//	S ... copy from result
#define I8080_INC(var) {					\
		u8 temp = var + 1;				\
		var = temp;					\
		u8 f = cpu->f & (I8080_C | I8080_FLAGINV);	\
		f |= I8080_FlagParTab[temp];			\
		if ((temp & 0x0f) == 0) f |= I8080_AC;		\
		cpu->f = f; }

// DEC var (var = 8-bit variable)
//	C ... not affected
//	P ... parity
//	AC ... set if result is not 0xNf (inverted borrow)
//	Z ... set if result is 0
//	S ... copy from result
#define I8080_DEC(var) {					\
		u8 temp = var - 1;				\
		var = temp;					\
		u8 f = cpu->f & (I8080_C | I8080_FLAGINV);	\
		f |= I8080_FlagParTab[temp];			\
		if ((temp & 0x0f) != 0x0f) f |= I8080_AC;	\
		cpu->f = f; }

// ADD HL,reg (reg = double register name)
//	C ... carry from bit 15
//	other ... not affected
#define I8080_ADD16(reg) {				\
		u16 hl = cpu->hl;			\
		u16 nn = cpu->reg;			\
		u32 nnnn = (u32)hl + nn;		\
		cpu->hl = (u16)nnnn;			\
		u8 f = cpu->f & ~I8080_C;		\
		f |= nnnn >> 16; /* I8080_C */;		\
		cpu->f = f; }

// ADD A,val (val = 8-bit value or register)
//	C ... carry from bit 7
//	P ... parity
//	AC ... carry from bit 3
//	Z ... set if result is 0
//	S ... copy from result
#define I8080_ADD(val) {					\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp + temp2;			\
		u8 f = (u8)(temp16 >> 8); /* carry */		\
		f |= (temp ^ temp2 ^ temp16) & I8080_AC;	\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		f |= I8080_FlagParTab[temp];			\
		cpu->f = f; }

// ADC A,val (val = 8-bit value or register)
//	C ... carry from bit 7
//	P ... parity
//	AC ... carry from bit 3
//	Z ... set if result is 0
//	S ... copy from result
#define I8080_ADC(val) {					\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp + temp2 + (cpu->f & I8080_C);	\
		u8 f = (u8)(temp16 >> 8); /* carry */		\
		f |= (temp ^ temp2 ^ temp16) & I8080_AC;	\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		f |= I8080_FlagParTab[temp];			\
		cpu->f = f; }

// SUB val (val = 8-bit value or register)
//	C ... borrow from bit 7
//	P ... parity
//	AC ... borrow from bit 3
//	Z ... set if result is 0
//	S ... copy from result
#define I8080_SUB(val) {					\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp2 - temp;			\
		u8 f = (u8)((temp16 >> 8) & I8080_C); 		\
		f |= (~(temp ^ temp2 ^ temp16)) & I8080_AC;	\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		f |= I8080_FlagParTab[temp];			\
		cpu->f = f; }

// SBC val (val = 8-bit value or register)
//	C ... borrow from bit 7
//	P ... parity
//	AC ... borrow from bit 3
//	Z ... set if result is 0
//	S ... copy from result
#define I8080_SBC(val) {					\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp2 - temp - (cpu->f & I8080_C);	\
		u8 f = (u8)((temp16 >> 8) & I8080_C);		\
		f |= (~(temp ^ temp2 ^ temp16)) & I8080_AC;	\
		temp = (u8)temp16;				\
		cpu->a = temp;					\
		f |= I8080_FlagParTab[temp];			\
		cpu->f = f; }

// AND val (val = 8-bit value or register)
//	C ... reset
//	P ... parity
//	AC ... set if bit 2 of A or reg
//	Z ... set if result is 0
//	S ... copy from result
#define I8080_AND(val) {				\
		u8 temp = (val);			\
		u8 temp2 = cpu->a;			\
		u8 f = ((temp | temp2) << 1) & I8080_AC; \
		temp = temp & temp2;			\
		cpu->a = temp;				\
		f |= I8080_FlagParTab[temp];		\
		cpu->f = f; }

// XOR val (val = 8-bit value or register)
//	C ... reset
//	P ... parity
//	AC ... reset
//	Z ... set if result is 0
//	S ... copy from result
#define I8080_XOR(val) {			\
		u8 temp = (val);		\
		temp = temp ^ cpu->a;		\
		cpu->a = temp;			\
		u8 f = I8080_FlagParTab[temp];	\
		cpu->f = f; }

// OR val (val = 8-bit value or register)
//	C ... reset
//	P ... parity
//	AC ... reset
//	Z ... set if result is 0
//	S ... copy from result
#define I8080_OR(val) {				\
		u8 temp = (val);		\
		temp = temp | cpu->a;		\
		cpu->a = temp;			\
		u8 f = I8080_FlagParTab[temp];	\
		cpu->f = f; }

// CP val (val = 8-bit value or register)
//	C ... borrow from bit 7
//	P ... parity
//	AC ... borrow from bit 3
//	Z ... set if result is 0
//	S ... copy from result
#define I8080_CP(val) {						\
		u8 temp = (val);				\
		u8 temp2 = cpu->a;				\
		u16 temp16 = temp2 - temp;			\
		u8 f = (u8)((temp16 >> 8) & I8080_C); 		\
		f |= (~(temp ^ temp2 ^ temp16)) & I8080_AC;	\
		temp = (u8)temp16;				\
		f |= I8080_FlagParTab[temp];			\
		cpu->f = f; }

// push 16-bit register to the stack
#define I8080_PUSH(regL, regH)	{ cpu->writemem(--cpu->sp, regH); cpu->writemem(--cpu->sp, regL); }

// pop 16-bit register from the stack
#define I8080_POP(regL, regH)	{ regL = cpu->readmem(cpu->sp++); regH = cpu->readmem(cpu->sp++); }

// RET
#define I8080_RET() { I8080_POP(cpu->pcl, cpu->pch); }

// CALL, RST
#define I8080_CALL(addr) { I8080_PUSH(cpu->pcl, cpu->pch); cpu->pc = addr; }
