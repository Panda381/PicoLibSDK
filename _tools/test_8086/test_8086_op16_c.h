
// 16-bit addition (AX+BX) by C
u32 ADD16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm {
		mov ebx,reg_bx
		add	ax,bx
	}
	SET_FLAGS;
}

// 16-bit addition (AX+BX) by C
u32 ADC16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm {
		mov ebx,reg_bx
		adc	ax,bx
	}
	SET_FLAGS;
}

// 16-bit subtraction (AX-BX) by C
u32 SUB16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm {
		mov ebx,reg_bx
		sub	ax,bx
	}
	SET_FLAGS;
}

// 16-bit C-subtraction (AX-BX-C) by C
u32 SBB16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm {
		mov ebx,reg_bx
		sbb	ax,bx
	}
	SET_FLAGS;
}

// 16-bit AND (AX & BX) by C
u32 AND16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm {
		mov ebx,reg_bx
		and	ax,bx
	}
	SET_FLAGS;
}

// 16-bit OR (AX | BX) by C
u32 OR16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm {
		mov ebx,reg_bx
		or	ax,bx
	}
	SET_FLAGS;
}

// 16-bit XOR (AX ^ BX) by C
u32 XOR16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm {
		mov ebx,reg_bx
		xor	ax,bx
	}
	SET_FLAGS;
}

// 16-bit compare (AX ? BX) by C
u32 CMP16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm {
		mov ebx,reg_bx
		cmp	ax,bx
	}
	SET_FLAGS;
}

// 16-bit TEST (AX &? BX) by C
u32 TEST16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm {
		mov ebx,reg_bx
		test ax,bx
	}
	SET_FLAGS;
}

// 16-bit INC (AX++) by C
u32 INC16_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { inc ax }
	SET_FLAGS;
}

// 16-bit DEC (AX--) by C
u32 DEC16_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { dec ax }
	SET_FLAGS;
}

// 16-bit ROL,1 instruction by C
u32 ROL16_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { rol ax,1 }
	SET_FLAGS;
}

// 16-bit ROR,1 instruction by C
u32 ROR16_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { ror ax,1 }
	SET_FLAGS;
}

// 16-bit RCL,1 instruction by C
u32 RCL16_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { rcl ax,1 }
	SET_FLAGS;
}

// 16-bit RCR,1 instruction by C
u32 RCR16_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { rcr ax,1 }
	SET_FLAGS;
}

// 16-bit SHL,1 instruction by C
u32 SHL16_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { shl ax,1 }
	SET_FLAGS;
}

// 16-bit SHR,1 instruction by C
u32 SHR16_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { shr ax,1 }
	SET_FLAGS;
}

// 16-bit SAR,1 instruction by C
u32 SAR16_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { sar ax,1 }
	SET_FLAGS;
}

// 16-bit ROL,CL instruction by C
u32 ROL16N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { rol ax,cl }
	SET_FLAGS;
}

// 16-bit ROR,CL instruction by C
u32 ROR16N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { ror ax,cl }
	SET_FLAGS;
}

// 16-bit RCL,CL instruction by C
u32 RCL16N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { rcl ax,cl }
	SET_FLAGS;
}

// 16-bit RCR,CL instruction by C
u32 RCR16N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { rcr ax,cl }
	SET_FLAGS;
}

// 16-bit SHL,CL instruction by C
u32 SHL16N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { shl ax,cl }
	SET_FLAGS;
}

// 16-bit SHR,CL instruction by C
u32 SHR16N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { shr ax,cl }
	SET_FLAGS;
}

// 16-bit SAR,CL instruction by C
u32 SAR16N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { sar ax,cl }
	SET_FLAGS;
}

// 16-bit multiplication (AX*BX) by C
u32 MUL16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm {
		mov ebx,reg_bx
		mul bx
	}
	SET_FLAGS;
}

// 16-bit signed multiplication (AX*BX) by C
u32 IMUL16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm {
		mov ebx,reg_bx
		imul bx
	}
	SET_FLAGS;
}

// 16-bit division (DX:AX/BX) by C
u32 DIV16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm { mov edx,eax }
	__asm { shr edx,16 }
	__asm { mov ebx,reg_bx }
	__asm { div bx }
	SET_FLAGS;
}

// 16-bit signed division (DX:AX/BX) by C
u32 IDIV16_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm { mov edx,eax }
	__asm { shr edx,16 }
	__asm { mov ebx,reg_bx }
	__asm { idiv bx }
//	__asm { add ax,dx }
	SET_FLAGS;
}
