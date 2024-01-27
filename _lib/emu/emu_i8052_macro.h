
// ****************************************************************************
//
//                            I8052 CPU Emulator - macro
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

// SFR register
#define I8052_SFR(a) cpu->sfr[(a)-I8052_SFR_BASE]

// stack register
#define I8052_SP I8052_SFR(I8052_ADDR_SP)

// accumulator
#define I8052_ACC I8052_SFR(I8052_ADDR_ACC)

// PSW register
#define I8052_PSW I8052_SFR(I8052_ADDR_PSW)

// B register
#define I8052_B I8052_SFR(I8052_ADDR_B)

// data pointer register
#define I8052_DPTR (*(u16*)&I8052_SFR(I8052_ADDR_DPL))

// register
#define I8052_REG(r) cpu->ram[(r) + (I8052_PSW & I8052_REGBANK_MASK)]

// update parity
#define U8052_PARITY_UPDATE() I8052_PSW = (I8052_PSW & ~I8052_P) | I8052_ParTab[I8052_ACC]
#define U8052_PARITY_UPDATE_A(a) I8052_PSW = (I8052_PSW & ~I8052_P) | I8052_ParTab[(a)]

// push 8-bit value into stack
#define I8052_PUSH8(val) {			\
		u8 sp = I8052_SP + 1;		\
		I8052_SP = sp;			\
		cpu->ram[sp] = (val); }
		
// push 16-bit value into stack
#define I8052_PUSH16(val) {			\
		u8 sp = I8052_SP + 1;		\
		u16 nn = (val);			\
		cpu->ram[sp] = (u8)nn;		\
		sp++;				\
		I8052_SP = sp;			\
		cpu->ram[sp] = nn >> 8; }

// pop 8-bit value from the stack
#define I8052_POP8(reg) {			\
		u8 sp = I8052_SP;		\
		reg = cpu->ram[sp];		\
		I8052_SP = sp - 1; }

// pop 16-bit value from the stack
#define I8052_POP16(reg) {				\
		u8 sp = I8052_SP;			\
		u8 n = cpu->ram[sp];			\
		sp--;					\
		reg = ((u16)n << 8) + cpu->ram[sp];	\
		I8052_SP = sp - 1; }

// ADD A,val (val = 8-bit value)
#define I8052_ADD(val) {						\
		u8 tempn = (val);					\
		u8 tempa = I8052_ACC;					\
		int temp16 = tempa + tempn;				\
		int tempxor = tempa ^ tempn ^ temp16;			\
		u8 f = I8052_PSW;					\
		f &= ~(I8052_C | I8052_AC | I8052_OV | I8052_P);	\
		f |= (temp16 >> 1) & I8052_C;				\
		f |= (tempxor << 2) & I8052_AC;				\
		f |= ((tempxor ^ (tempxor >> 1)) >> 5) & I8052_OV;	\
		u8 n = (u8)temp16;					\
		I8052_ACC = n;						\
		f |= I8052_ParTab[n]; /* I8052_P */			\
		I8052_PSW = f; }

// ADDC A,val (val = 8-bit value)
#define I8052_ADDC(val) {						\
		u8 tempn = (val);					\
		u8 tempa = I8052_ACC;					\
		u8 f = I8052_PSW;					\
		int temp16 = tempa + tempn + (f >> 7);			\
		int tempxor = tempa ^ tempn ^ temp16;			\
		f &= ~(I8052_C | I8052_AC | I8052_OV | I8052_P);	\
		f |= (temp16 >> 1) & I8052_C;				\
		f |= (tempxor << 2) & I8052_AC;				\
		f |= ((tempxor ^ (tempxor >> 1)) >> 5) & I8052_OV;	\
		u8 n = (u8)temp16;					\
		I8052_ACC = n;						\
		f |= I8052_ParTab[n]; /* I8052_P */			\
		I8052_PSW = f; }

// SUBB A,val (val = 8-bit value)
#define I8052_SUBB(val) {						\
		u8 tempn = (val);					\
		u8 tempa = I8052_ACC;					\
		u8 f = I8052_PSW;					\
		int temp16 = tempa - tempn - (f >> 7);			\
		int tempxor = tempa ^ tempn ^ temp16;			\
		f &= ~(I8052_C | I8052_AC | I8052_OV | I8052_P);	\
		f |= (temp16 >> 1) & I8052_C;				\
		f |= (tempxor << 2) & I8052_AC;				\
		f |= ((tempxor ^ (tempxor >> 1)) >> 5) & I8052_OV;	\
		u8 n = (u8)temp16;					\
		I8052_ACC = n;						\
		f |= I8052_ParTab[n]; /* I8052_P */			\
		I8052_PSW = f; }

// ANL A,val (val = 8-bit value)
#define I8052_ANL_A(val) {			\
		u8 temp = (val);		\
		temp &= I8052_ACC;		\
		I8052_ACC = temp;		\
		U8052_PARITY_UPDATE_A(temp); }

// ORL A,val (val = 8-bit value)
#define I8052_ORL_A(val) {			\
		u8 temp = (val);		\
		temp |= I8052_ACC;		\
		I8052_ACC = temp;		\
		U8052_PARITY_UPDATE_A(temp); }

// XRL A,val (val = 8-bit value)
#define I8052_XRL_A(val) {			\
		u8 temp = (val);		\
		temp ^= I8052_ACC;		\
		I8052_ACC = temp;		\
		U8052_PARITY_UPDATE_A(temp); }

// CJNE val1,val2,rel (val = 8-bit values)
#define I8052_CJNE(val1, val2) {				\
		s8 rel = (s8)I8052_ProgByte(cpu);		\
		u8 tmp1 = (val1);				\
		u8 tmp2 = (val2);				\
		u8 psw = I8052_PSW;				\
		psw &= ~I8052_C;				\
		if (tmp1 != tmp2)				\
		{	cpu->pc += (s8)rel;			\
			if (tmp1 < tmp2) psw |= I8052_C; }	\
		I8052_PSW = psw; }

// DJNZ rel (var = 8-bit variable)
#define I8052_DJNZ(var) {				\
		s8 rel = (s8)I8052_ProgByte(cpu);	\
		var--;					\
		if (var != 0) cpu->pc += (s8)rel; }

