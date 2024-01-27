
// 8-bit addition (AL+AH) by Emulator
u32 ADD8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = 0;
	u8 dst = (u8)reg_ax;
	u8 src = (u8)(reg_ax >> 8);

	u16 res = dst + src;		// result
	u16 xor = dst ^ src ^ res;	// XOR
	f |= res >> 8;			// I8086_CF carry flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) << 4) & I8086_OF; // overflow flag
	dst = (u8)res;
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag

	return (f << 16) | (reg_ax & 0xff00) | dst;
}

// 8-bit C-addition (AL+AH+C) by Emulator
u32 ADC8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = 0;
	u8 dst = (u8)reg_ax;
	u8 src = (u8)(reg_ax >> 8);
	u8 c = (u8)(reg_flags & 1);

	u16 res = dst + src + c;	// result
	u16 xor = dst ^ src ^ res;	// XOR
	f |= res >> 8;			// I8086_CF carry flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) << 4) & I8086_OF; // overflow flag
	dst = (u8)res;
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag

	return (f << 16) | (reg_ax & 0xff00) | dst;
}

// 8-bit subtraction (AL-AH) by Emulator
u32 SUB8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = 0;
	u8 dst = (u8)reg_ax;
	u8 src = (u8)(reg_ax >> 8);

	u16 res = dst - src;		// result
	u16 xor = dst ^ src ^ res;	// XOR
	f |= (res >> 8) & I8086_CF;	// I8086_CF borrow flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) << 4) & I8086_OF; // overflow flag
	dst = (u8)res;
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag

	return (f << 16) | (reg_ax & 0xff00) | dst;
}

// 8-bit C-subtraction (AL-AH-C) by Emulator
u32 SBB8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = 0;
	u8 dst = (u8)reg_ax;
	u8 src = (u8)(reg_ax >> 8);
	u8 c = (u8)(reg_flags & 1);

	u16 res = dst - src - c;	// result
	u16 xor = dst ^ src ^ res;	// XOR
	f |= (res >> 8) & I8086_CF;	// I8086_CF borrow flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) << 4) & I8086_OF; // overflow flag
	dst = (u8)res;
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag

	return (f << 16) | (reg_ax & 0xff00) | dst;
}

// 8-bit AND (AL & AH) by Emulator
u32 AND8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = 0;
	u8 dst = (u8)reg_ax;
	u8 src = (u8)(reg_ax >> 8);

	u8 res = dst & src;		// result
	dst = res;			// save result of operation
	f |= I8086_FlagParTab[res];	// add parity, sign and zero flag

	return (f << 16) | (reg_ax & 0xff00) | dst;
}

// 8-bit OR (AL | AH) by Emulator
u32 OR8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = 0;
	u8 dst = (u8)reg_ax;
	u8 src = (u8)(reg_ax >> 8);

	u8 res = dst | src;		// result
	dst = res;			// save result of operation
	f |= I8086_FlagParTab[res];	// add parity, sign and zero flag

	return (f << 16) | (reg_ax & 0xff00) | dst;
}

// 8-bit XOR (AL ^ AH) by Emulator
u32 XOR8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = 0;
	u8 dst = (u8)reg_ax;
	u8 src = (u8)(reg_ax >> 8);

	u8 res = dst ^ src;		// result
	dst = res;			// save result of operation
	f |= I8086_FlagParTab[res];	// add parity, sign and zero flag

	return (f << 16) | (reg_ax & 0xff00) | dst;
}

// 8-bit compare (AL ? AH) by Emulator
u32 CMP8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = 0;
	u8 dst = (u8)reg_ax;
	u8 src = (u8)(reg_ax >> 8);

	u16 res = dst - src;		// result
	u16 xor = dst ^ src ^ res;	// XOR
	f |= (res >> 8) & I8086_CF;	// I8086_CF borrow flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) << 4) & I8086_OF; // overflow flag
	dst = (u8)res;
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag

	return (f << 16) | (u16)reg_ax;
}

// 8-bit TEST (AL &? AH) by Emulator
u32 TEST8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = 0;
	u8 dst = (u8)reg_ax;
	u8 src = (u8)(reg_ax >> 8);

	u8 res = dst & src;		// result
	dst = res;			// save result of operation
	f |= I8086_FlagParTab[res];	// add parity, sign and zero flag

	return (f << 16) | (u16)reg_ax;
}

// 8-bit INC (AL++) by Emulator
u32 INC8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)(reg_flags & I8086_CF);
	u8 dst = (u8)reg_ax + 1;

	if ((dst & 0x0f) == 0) f |= I8086_AF; // half-carry flag
	if (dst == 0x80) f |= I8086_OF;	// overflow flag
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag

	return (f << 16) | (reg_ax & 0xff00) | dst;
}

// 8-bit DEC (AL--) by Emulator
u32 DEC8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)(reg_flags & I8086_CF);
	u8 dst = (u8)reg_ax - 1;

	if ((dst & 0x0f) == 0x0f) f |= I8086_AF; // half-carry flag
	if (dst == 0x7f) f |= I8086_OF;	// overflow flag
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag

	return (f << 16) | (reg_ax & 0xff00) | dst;
}

// JO by Emulator
u32 JO_E(u32 reg_flags)
{
	return (reg_flags & I8086_OF) != 0;
}

// JNO by Emulator
u32 JNO_E(u32 reg_flags)
{
	return (reg_flags & I8086_OF) == 0;
}

// JB by Emulator
u32 JB_E(u32 reg_flags)
{
	return (reg_flags & I8086_CF) != 0;
}

// JNB by Emulator
u32 JNB_E(u32 reg_flags)
{
	return (reg_flags & I8086_CF) == 0;
}

// JE by Emulator
u32 JE_E(u32 reg_flags)
{
	return (reg_flags & I8086_ZF) != 0;
}

// JNE by Emulator
u32 JNE_E(u32 reg_flags)
{
	return (reg_flags & I8086_ZF) == 0;
}

// JBE by Emulator
u32 JBE_E(u32 reg_flags)
{
	return (reg_flags & (I8086_CF | I8086_ZF)) != 0;
}

// JNBE by Emulator
u32 JNBE_E(u32 reg_flags)
{
	return (reg_flags & (I8086_CF | I8086_ZF)) == 0;
}

// JS by Emulator
u32 JS_E(u32 reg_flags)
{
	return (reg_flags & I8086_SF) != 0;
}

// JNS by Emulator
u32 JNS_E(u32 reg_flags)
{
	return (reg_flags & I8086_SF) == 0;
}

// JP by Emulator
u32 JP_E(u32 reg_flags)
{
	return (reg_flags & I8086_PF) != 0;
}

// JNP by Emulator
u32 JNP_E(u32 reg_flags)
{
	return (reg_flags & I8086_PF) == 0;
}

// JL by Emulator
u32 JL_E(u32 reg_flags)
{
	u16 f = (u16)reg_flags;
	f = f ^ (f << (I8086_OF_BIT - I8086_SF_BIT));
	return (f & I8086_OF) != 0;
}

// JNL by Emulator
u32 JNL_E(u32 reg_flags)
{
	u16 f = (u16)reg_flags;
	f = f ^ (f << (I8086_OF_BIT - I8086_SF_BIT));
	return (f & I8086_OF) == 0;
}

// JLE by Emulator
u32 JLE_E(u32 reg_flags)
{
	u16 f = (u16)reg_flags;
	u16 f2 = f ^ (f << (I8086_OF_BIT - I8086_SF_BIT));
	return ((f2 & I8086_OF) != 0) || ((f & I8086_ZF) != 0);
}

// JNLE by Emulator
u32 JNLE_E(u32 reg_flags)
{
	u16 f = (u16)reg_flags;
	u16 f2 = f ^ (f << (I8086_OF_BIT - I8086_SF_BIT));
	return ((f2 & I8086_OF) == 0) && ((f & I8086_ZF) == 0);
}

// DAA instruction by Emulator
u32 DAA_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)(reg_flags & ~(I8086_PF | I8086_ZF | I8086_SF | I8086_OF));
	u8 a = (u8)reg_ax;
#if I8086_CPU_AMD
	u8 a0 = a;
#endif

	if ((a > 0x99) || ((f & I8086_CF) != 0))
	{
		a += 0x60;
		f |= I8086_CF;
	}

	if (((a & 0x0f) > 9) || ((f & I8086_AF) != 0))
	{
		a += 6;
		f |= I8086_AF;
	}

#if I8086_CPU_AMD
	if ((a0 < 0x80) && (a >= 0x80)) f |= I8086_OF;
#endif

	f |= I8086_FlagParTab[a];

	return (f << 16) | (reg_ax & 0xff00) | a;
}

// DAS instruction by Emulator
u32 DAS_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)(reg_flags & ~(I8086_PF | I8086_ZF | I8086_SF | I8086_OF));
	u8 a = (u8)reg_ax;
#if I8086_CPU_AMD
	u8 a0 = a;
#endif

	if ((a > 0x99) || ((f & I8086_CF) != 0))
	{
		a -= 0x60;
		f |= I8086_CF;
	}

	if (((a & 0x0f) > 9) || ((f & I8086_AF) != 0))
	{
		if (a < 6) f |= I8086_CF;
		a -= 6;
		f |= I8086_AF;
	}

#if I8086_CPU_AMD
	if ((a0 >= 0x80) && (a < 0x80)) f |= I8086_OF;
#endif

	f |= I8086_FlagParTab[a];

	return (f << 16) | (reg_ax & 0xff00) | a;
}

// AAA instruction by Emulator
u32 AAA_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)(reg_flags & ~(I8086_PF | I8086_ZF | I8086_SF | I8086_OF));
	u8 al = (u8)reg_ax;
	u8 ah = (u8)(reg_ax >> 8);

	if (((al & 0x0f) > 9) || ((f & I8086_AF) != 0))
	{
		al += 6;

		// === Implementation difference:
// On 8086/8088, correction "6" is added to AL register only.
// On 80286 and later, correction "6" is added do AX register (so add carry +1 to AH).
#if I8086_CPU_80286_UP || I8086_CPU_AMD	// use 80286 or later, or use AMD
		if (al < 6)
		{
			ah++;
#if I8086_CPU_AMD
			if (ah == 0x80) f |= I8086_OF;
#endif // AMD
		}
#endif // 80286 || AMD

		ah++;

#if I8086_CPU_AMD
		if (ah == 0x80) f |= I8086_OF;
#endif

		f |= I8086_AF | I8086_CF;
	}
	else
		f &= ~(I8086_AF | I8086_CF);

#if I8086_CPU_AMD
	if ((al == 0) && (ah == 0)) f |= I8086_ZF;
	f |= I8086_FlagParTab[al] & I8086_PF;
	if (ah >= 0x80) f |= I8086_SF;
#endif

	al &= 0x0f;

#if I8086_CPU_INTEL
	if (al == 0) f |= I8086_ZF;
	f |= I8086_FlagParTab[al] & I8086_PF;
#endif

	return (f << 16) | (ah << 8) | al;
}

// AAS instruction by Emulator
u32 AAS_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)(reg_flags & ~(I8086_PF | I8086_ZF | I8086_SF | I8086_OF));
	u8 al = (u8)reg_ax;
	u8 ah = (u8)(reg_ax >> 8);

	if (((al & 0x0f) > 9) || ((f & I8086_AF) != 0))
	{
		al -= 6;

		// === Implementation difference:
// On 8086/8088, correction "6" is subtracted from AL register only.
// On 80286 and later, correction "6" is subtracted from AX register (so subtract borrow -1 from AH).
#if I8086_CPU_80286_UP || I8086_CPU_AMD	// use Intel 80286 or later, or use AMD
		if (al >= 0xfa)
		{
			ah--;
#if I8086_CPU_AMD
			if (ah == 0x7f) f |= I8086_OF;
#endif // AMD
		}
#endif // 80286 || AMD

		ah--;

#if I8086_CPU_AMD
		if (ah == 0x7f) f |= I8086_OF;
#endif

		f |= I8086_AF | I8086_CF;
	}
	else
		f &= ~(I8086_AF | I8086_CF);

#if I8086_CPU_AMD
	if ((al == 0) && (ah == 0)) f |= I8086_ZF;
	if (ah >= 0x80) f |= I8086_SF;
	f |= I8086_FlagParTab[al] & I8086_PF;
#endif

	al &= 0x0f;

#if I8086_CPU_INTEL
	if (al == 0) f |= I8086_ZF;
	f |= I8086_FlagParTab[al] & I8086_PF;
#endif

	return (f << 16) | (ah << 8) | al;
}

// AAD instruction by Emulator
u32 AAD_E(u32 reg_flags, u32 reg_ax, u8 base)
{
	u16 f = (u16)(reg_flags & ~I8086_FLAGALL);
	u8 al = (u8)reg_ax;
	u8 ah = (u8)(reg_ax >> 8);

	ah *= base;					// AH * 10
	u16 res = al + ah;			// AL + AH*10
	u16 xor = al ^ ah ^ res;	// XOR
	f |= res >> 8;				// I8086_CF carry flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) << 4) & I8086_OF; // overflow flag
	al = (u8)res;
	f |= I8086_FlagParTab[al];	// add parity, sign and zero flag
	ah = 0;

	return (f << 16) | (ah << 8) | al;
}

// AAM instruction by Emulator
u32 AAM_E(u32 reg_flags, u32 reg_ax, u8 base)
{
	u16 f = (u16)(reg_flags & ~I8086_FLAGALL);
	u8 al = (u8)reg_ax;
	u8 ah = (u8)(reg_ax >> 8);

	ah = al / base;				// AL / 10
	al -= ah * base;			// remainder
	f |= I8086_FlagParTab[al];	// add parity, sign and zero flag

	return (f << 16) | (ah << 8) | al;
}

// 8-bit ROL,1 by Emulator
u32 ROL8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u8 n = (u8)reg_ax;

	f &= ~(I8086_CF | I8086_OF); // clear carry and overflow
	u8 n2 = n >> 7;			// carry
	f |= n2;			// add carry flag
	n2 |= n << 1;			// ROL shift
	n = (n ^ n2) >> 7;		// bit 0 = overflow flag (difference of the bits)
	f |= ((u16)n << I8086_OF_BIT);	// set overflow flag

	return (f << 16) | (reg_ax & 0xff00) | n2;
}

// 8-bit ROR,1 by Emulator
u32 ROR8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u8 n = (u8)reg_ax;

	f &= ~(I8086_CF | I8086_OF); // clear carry and overflow
	f |= n & I8086_CF;		// add carry flag
	u8 n2 = (n >> 1) | (n << 7);	// ROR shift
	n = (n ^ n2) >> 7;		// bit 0 = overflow flag (difference of the bits)
	f |= ((u16)n << I8086_OF_BIT);	// set overflow flag

	return (f << 16) | (reg_ax & 0xff00) | n2;
}

// 8-bit RCL,1 by Emulator
u32 RCL8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u8 n = (u8)reg_ax;

	u8 n2 = f & I8086_CF;		// old carry flag
	f &= ~(I8086_CF | I8086_OF);	// clear carry and overflow
	f |= n >> 7;			// add new carry flag
	n2 |= n << 1;			// RCL shift
	n = (n ^ n2) >> 7;		// bit 0 = overflow flag (difference of the bits)
	f |= ((u16)n << I8086_OF_BIT);	// set overflow flag

	return (f << 16) | (reg_ax & 0xff00) | n2;
}

// 8-bit RCR,1 by Emulator
u32 RCR8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u8 n = (u8)reg_ax;

	u8 n2 = (f & I8086_CF) << 7;	// old carry flag
	f &= ~(I8086_CF | I8086_OF);	// clear carry and overflow
	u8 n3 = (n ^ n2) >> 7;		// bit 0 = overflow flag (difference of the bits)
	f |= ((u16)n3 << I8086_OF_BIT);	// set overflow flag
	f |= n & I8086_CF;		// add new carry flag
	n2 |= n >> 1;			// RCR shift

	return (f << 16) | (reg_ax & 0xff00) | n2;
}

// 8-bit SHL,1 by Emulator
u32 SHL8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u8 n = (u8)reg_ax;

	f &= ~I8086_FLAGALL;	// clear flags
	f |= n >> 7;			// I8086_CF carry flag
	f |= ((u16)(n ^ (n >> 1)) << 5) & I8086_OF; // overflow flag
	n <<= 1;			// result

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag
#endif

	f |= I8086_FlagParTab[n];	// add parity, sign and zero flag

	return (f << 16) | (reg_ax & 0xff00) | n;
}

// 8-bit SHR,1 by Emulator
u32 SHR8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u8 n = (u8)reg_ax;

	f &= ~I8086_FLAGALL;	// clear flags
	f |= n & I8086_CF;		// I8086_CF carry flag
	f |= ((u16)n << 4) & I8086_OF;	// overflow flag
	n >>= 1;			// result

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
#endif

	f |= I8086_FlagParTab[n];	// add parity, sign and zero flag

	return (f << 16) | (reg_ax & 0xff00) | n;
}

// 8-bit SAR,1 by Emulator
u32 SAR8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u8 n = (u8)reg_ax;

	f &= ~I8086_FLAGALL;	// clear flags
	f |= n & I8086_CF;		// I8086_CF carry flag
	n = (u8)(((s8)n)>>1);		// result

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
#endif

	f |= I8086_FlagParTab[n];	// add parity, sign and zero flag

	return (f << 16) | (reg_ax & 0xff00) | n;
}

// 8-bit multiplication (AL*AH) by Emulator
u32 MUL8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	u8 al = (u8)reg_ax;
	u8 n = (u8)(reg_ax >> 8);

	u16 res = (u16)n * (u16)al; // AX = AL * operand, unsigned

#if I8086_CPU_INTEL
	f &= ~I8086_FLAGALL; // clear flags
	f |= I8086_FlagParTab[(u8)res] & ~I8086_ZF;	// add parity and sign, zero stays cleared
#else
	f &= ~(I8086_CF | I8086_OF); // clear flags
#endif

	u8 ah = (u8)(res >> 8); // HIGH part of the result
	if (ah != 0) f |= I8086_CF | I8086_OF; // add carry and overflow

	return (f << 16) | res;
}

// 8-bit signed multiplication (AL*AH) by Emulator
u32 IMUL8_E(u32 reg_flags, u32 reg_ax)
{
	u16 f = (u16)reg_flags;
	s16 al = (s16)(s8)(u8)reg_ax;
	s16 n = (s16)(s8)(u8)(reg_ax >> 8);

	s16 res = n * al; // AX = AL * operand, signed

#if I8086_CPU_INTEL
	f &= ~I8086_FLAGALL; // clear flags
	f |= I8086_FlagParTab[(u8)res] & (I8086_PF | I8086_SF); // add parity flag and sign
#else
	f &= ~(I8086_CF | I8086_OF); // clear flags
#endif

	if ((res < -128) || (res > 127)) f |= I8086_CF | I8086_OF; // add carry and overflow if sign extension of AL has been changed

	return (f << 16) | (u16)res;
}

// 8-bit division (AX/BL) by Emulator
u32 DIV8_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = (u16)reg_flags;
	u16 ax = (u16)reg_ax;
	u8 n = (u8)reg_bx;

	// divide
	u8 al = (u8)(ax / n); // get quotient (1st operand = numerator, 2nd operand = denominator)
	u8 ah = (u8)(ax - al*n); // get remainder

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
	f &= ~(I8086_PF | I8086_ZF | I8086_SF);
#endif

#if I8086_CPU_INTEL && !I8086_CPU_80286_UP
	f &= ~(I8086_CF | I8086_OF); // clear flags
#endif

	return (f << 16) | (ah << 8) | al;
}

// 8-bit signed division (AX/BL) by Emulator
u32 IDIV8_E(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	u16 f = (u16)reg_flags;
	s16 ax = (s16)(u16)reg_ax;
	s8 n = (s8)(u8)reg_bx;

	// divide
	s16 res = ax / n; // get quotient (1st operand = numerator, 2nd operand = denominator)
	s16 rem = ax - res*n; // get remainder

	u8 al = (u8)res; // quotient
	u8 ah = (u8)rem; // remainder

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
	f &= ~(I8086_PF | I8086_ZF | I8086_SF);
#endif

#if I8086_CPU_INTEL && !I8086_CPU_80286_UP
	f &= ~(I8086_CF | I8086_OF); // clear flags
#endif

	return (f << 16) | (ah << 8) | al;
}
