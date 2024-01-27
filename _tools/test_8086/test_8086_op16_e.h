
// 16-bit addition (AX+BX) by Emulator
u32 ADD16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = 0;
	u16 dst = (u16)reg_ax;
	u16 src = (u16)reg_bx;

	u32 res = dst + src;		// result
	u32 xor = dst ^ src ^ res;	// XOR
	f |= res >> 16;			// I8086_CF carry flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) >> 4) & I8086_OF; // overflow flag
	dst = (u16)res;
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | dst;
}

// 16-bit C-addition (AX+BX+C) by Emulator
u32 ADC16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = 0;
	u16 dst = (u16)reg_ax;
	u16 src = (u16)reg_bx;
	u16 c = (u16)(reg_flags & 1);

	u32 res = dst + src + c;	// result
	u32 xor = dst ^ src ^ res;	// XOR
	f |= res >> 16;			// I8086_CF carry flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) >> 4) & I8086_OF; // overflow flag
	dst = (u16)res;
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | dst;
}

// 16-bit subtraction (AX-BX) by Emulator
u32 SUB16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = 0;
	u16 dst = (u16)reg_ax;
	u16 src = (u16)reg_bx;

	u32 res = dst - src;		// result
	u32 xor = dst ^ src ^ res;	// XOR
	f |= (res >> 16) & I8086_CF;	// I8086_CF borrow flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) >> 4) & I8086_OF; // overflow flag
	dst = (u16)res;
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | dst;
}

// 16-bit C-subtraction (AX-BX-C) by Emulator
u32 SBB16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = 0;
	u16 dst = (u16)reg_ax;
	u16 src = (u16)reg_bx;
	u16 c = (u16)(reg_flags & 1);

	u32 res = dst - src - c;	// result
	u32 xor = dst ^ src ^ res;	// XOR
	f |= (res >> 16) & I8086_CF;	// I8086_CF borrow flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) >> 4) & I8086_OF; // overflow flag
	dst = (u16)res;
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | dst;
}

// 16-bit AND (AX & BX) by Emulator
u32 AND16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = 0;
	u16 dst = (u16)reg_ax;
	u16 src = (u16)reg_bx;

	u16 res = dst & src;		// result
	dst = res;		// save result of operation
	f |= I8086_FlagParTab[(u8)res] & I8086_PF; // add parity flag
	if (res == 0) f |= I8086_ZF;	// add zero flag
	f |= (res >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | dst;
}

// 16-bit OR (AX | BX) by Emulator
u32 OR16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = 0;
	u16 dst = (u16)reg_ax;
	u16 src = (u16)reg_bx;

	u16 res = dst | src;		// result
	dst = res;		// save result of operation
	f |= I8086_FlagParTab[(u8)res] & I8086_PF; // add parity flag
	if (res == 0) f |= I8086_ZF;	// add zero flag
	f |= (res >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | dst;
}

// 16-bit XOR (AX ^ BX) by Emulator
u32 XOR16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = 0;
	u16 dst = (u16)reg_ax;
	u16 src = (u16)reg_bx;

	u16 res = dst ^ src;		// result
	dst = res;		// save result of operation
	f |= I8086_FlagParTab[(u8)res] & I8086_PF; // add parity flag
	if (res == 0) f |= I8086_ZF;	// add zero flag
	f |= (res >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | dst;
}

// 16-bit compare (AX ? BX) by Emulator
u32 CMP16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = 0;
	u16 dst = (u16)reg_ax;
	u16 src = (u16)reg_bx;

	u32 res = dst - src;		// result
	u32 xor = dst ^ src ^ res;	// XOR
	f |= (res >> 16) & I8086_CF;	// I8086_CF borrow flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) >> 4) & I8086_OF; // overflow flag
	dst = (u16)res;
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | (u16)reg_ax;
}

// 16-bit TEST (AX &? BX) by Emulator
u32 TEST16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = 0;
	u16 dst = (u16)reg_ax;
	u16 src = (u16)reg_bx;

	u16 res = dst & src;		// result
	dst = res;		// save result of operation
	f |= I8086_FlagParTab[(u8)res] & I8086_PF; // add parity flag
	if (res == 0) f |= I8086_ZF;	// add zero flag
	f |= (res >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | (u16)reg_ax;
}

// 16-bit INC (AX++) by Emulator
u32 INC16_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)(reg_flags & I8086_CF);
	u16 dst = (u16)reg_ax + 1;

	if ((dst & 0x0f) == 0) f |= I8086_AF; // half-carry flag
	if (dst == 0x8000) f |= I8086_OF; // overflow flag
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | dst;
}

// 16-bit DEC (AX--) by Emulator
u32 DEC16_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)(reg_flags & I8086_CF);
	u16 dst = (u16)reg_ax - 1;

	if ((dst & 0x0f) == 0x0f) f |= I8086_AF; // half-carry flag
	if (dst == 0x7fff) f |= I8086_OF; // overflow flag
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | dst;
}

// 16-bit ROL,1 by Emulator
u32 ROL16_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u16 n = (u16)reg_ax;

	f &= ~(I8086_CF | I8086_OF); // clear carry and overflow
	u16 n2 = n >> 15;		// carry
	f |= n2;			// add carry flag
	n2 |= n << 1;			// ROL shift
	n = (n ^ n2) >> 15;		// bit 0 = overflow flag (difference of the bits)
	f |= (n << I8086_OF_BIT);	// set overflow flag

	return (f << 16) | n2;
}

// 16-bit ROR,1 by Emulator
u32 ROR16_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u16 n = (u16)reg_ax;

	f &= ~(I8086_CF | I8086_OF); // clear carry and overflow
	f |= n & I8086_CF;		// add carry flag
	u16 n2 = (n >> 1) | (n << 15);	// ROR shift
	n = (n ^ n2) >> 15;		// bit 0 = overflow flag (difference of the bits)
	f |= (n << I8086_OF_BIT);	// set overflow flag

	return (f << 16) | n2;
}

// 16-bit RCL,1 by Emulator
u32 RCL16_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u16 n = (u16)reg_ax;

	u16 n2 = f & I8086_CF;		// old carry flag
	f &= ~(I8086_CF | I8086_OF);	// clear carry and overflow
	f |= n >> 15;			// add new carry flag
	n2 |= n << 1;			// RCL shift
	n = (n ^ n2) >> 15;		// bit 0 = overflow flag (difference of the bits)
	f |= (n << I8086_OF_BIT);	// set overflow flag

	return (f << 16) | n2;
}

// 16-bit RCR,1 by Emulator
u32 RCR16_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u16 n = (u16)reg_ax;

	u16 n2 = (f & I8086_CF) << 15;	// old carry flag
	f &= ~(I8086_CF | I8086_OF);	// clear carry and overflow
	u16 n3 = (n ^ n2) >> 15;	// bit 0 = overflow flag (difference of the bits)
	f |= (n3 << I8086_OF_BIT);	// set overflow flag (in case of RCR instruction, OF flag is checked BEFORE rotation)
	f |= n & I8086_CF;		// add new carry flag
	n2 |= n >> 1;			// RCR shift

	return (f << 16) | n2;
}

// 16-bit SHL,1 by Emulator
u32 SHL16_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u16 n = (u16)reg_ax;

	f &= ~I8086_FLAGALL;	// clear flags
	f |= n >> 15;			// I8086_CF carry flag
	f |= ((n ^ (n >> 1)) >> 3) & I8086_OF; // overflow flag
	n <<= 1;			// result

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
#endif

	f |= I8086_FlagParTab[(u8)n] & I8086_PF; // add parity flag
	if (n == 0) f |= I8086_ZF;	// add zero flag
	f |= (n >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | n;
}

// 16-bit SHR,1 by Emulator
u32 SHR16_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u16 n = (u16)reg_ax;

	f &= ~I8086_FLAGALL;	// clear flags
	f |= n & I8086_CF;		// I8086_CF carry flag
	f |= (n >> 4) & I8086_OF;	// overflow flag
	n >>= 1;			// result

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
#endif

	f |= I8086_FlagParTab[(u8)n] & I8086_PF; // add parity flag
	if (n == 0) f |= I8086_ZF;	// add zero flag
	f |= (n >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | n;
}

// 16-bit SAR,1 by Emulator
u32 SAR16_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u16 n = (u16)reg_ax;

	f &= ~I8086_FLAGALL;	// clear flags
	f |= n & I8086_CF;		// I8086_CF carry flag
	n = (u16)(((s16)n)>>1);		// result
	
#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
#endif

	f |= I8086_FlagParTab[(u8)n] & I8086_PF; // add parity flag
	if (n == 0) f |= I8086_ZF;	// add zero flag
	f |= (n >> 8) & I8086_SF;	// add sign flag

	return (f << 16) | n;
}

// 16-bit multiplication (AX*DX) by Emulator
u32 MUL16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = (u16)reg_flags;

	u32 res = (u32)reg_ax * (u32)reg_bx; // DX:AX = AX * operand, unsigned

#if I8086_CPU_INTEL
	f &= ~I8086_FLAGALL; // clear flags
	f |= I8086_FlagParTab[(u8)res] & ~(I8086_ZF | I8086_SF);	// add parity and sign, zero stays cleared
	if ((s16)(u16)res < 0) f |= I8086_SF;
#else
	f &= ~(I8086_CF | I8086_OF); // clear flags
#endif

	u16 dx = (u16)(res >> 16); // HIGH part of the result
	if (dx != 0) f |= I8086_CF | I8086_OF; // add carry and overflow

	return (f << 16) | (u16)res;
}

// 16-bit signed multiplication (AX*DX) by Emulator
u32 IMUL16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = (u16)reg_flags;

	s32 ax0 = (s32)(s16)(u16)reg_ax;
	s32 res = (s32)(s16)(u16)reg_bx * ax0; // DX:AX = AX * operand, signed
	u16 ax = (u16)res; // result LOW
	u16 dx = (u16)(res >> 16); // result HIGHT

#if I8086_CPU_INTEL
	f &= ~I8086_FLAGALL; // clear flags
	f |= I8086_FlagParTab[(u8)ax] & ~(I8086_ZF | I8086_SF); // add parity
	if ((s16)ax < 0) f |= I8086_SF;
#else
	f &= ~(I8086_CF | I8086_OF); // clear flags
#endif

	if ((res < -32768) || (res > 32767)) f |= I8086_CF | I8086_OF; // add carry and overflow if sign extension of AX has been changed

	return (f << 16) | (u16)res;
}

// 16-bit division (DX:AX/BX) by Emulator
u32 DIV16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = (u16)reg_flags;
	u32 ax = reg_ax;
	u16 n = (u16)reg_bx;

	// divide
	u16 al = (u16)(ax / n); // get quotient (1st operand = numerator, 2nd operand = denominator)
	u16 ah = (u16)(ax - al*n); // get remainder

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
	f &= ~(I8086_PF | I8086_ZF | I8086_SF | I8086_CF | I8086_OF);
#endif

#if I8086_CPU_INTEL
#if I8086_CPU_80286_UP
	f &= ~(I8086_CF | I8086_OF | I8086_AF | I8086_SF); // clear flags
	f |= (I8086_PF | I8086_ZF);
#else
	f &= ~(I8086_CF | I8086_OF); // clear flags
#endif
#endif

	return (f << 16) | al;
}

// 16-bit signed division (DX:AX/BX) by Emulator
u32 IDIV16_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = (u16)reg_flags;
	s32 ax = reg_ax;
	s16 nn = (s16)(u16)reg_bx;

	// divide
	s32 res = ax / nn; // get quotient (1st operand = numerator, 2nd operand = denominator)
	s32 rem = ax - res*nn; // get remainder

#if I8086_CPU_AMD // carry undefined
	f |= I8086_AF;			// half-carry flag is always set
	f &= ~(I8086_PF | I8086_ZF | I8086_SF | I8086_OF);
#endif

#if I8086_CPU_INTEL && !I8086_CPU_80286_UP
	f &= ~(I8086_CF | I8086_OF); // clear flags
#endif

	return (f << 16) | (u16)res;
}
