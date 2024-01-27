
// 8-bit addition (AL+AH) by C
u32 ADD8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { add al,ah }
	SET_FLAGS;
}

// 8-bit C-addition (AL+AH+C) by C
u32 ADC8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { adc al,ah }
	SET_FLAGS;
}

// 8-bit subtraction (AL-AH) by C
u32 SUB8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { sub	al,ah }
	SET_FLAGS;
}

// 8-bit C-subtraction (AL-AH-C) by C
u32 SBB8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { sbb	al,ah }
	SET_FLAGS;
}

// 8-bit AND (AL & AH) by C
u32 AND8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { and	al,ah }
	SET_FLAGS;
}

// 8-bit OR (AL | AH) by C
u32 OR8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { or	al,ah }
	SET_FLAGS;
}

// 8-bit XOR (AL ^ AH) by C
u32 XOR8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { xor	al,ah }
	SET_FLAGS;
}

// 8-bit copmpare (AL ? AH) by C
u32 CMP8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { cmp	al,ah }
	SET_FLAGS;
}

// 8-bit TEST (AL &? AH) by C
u32 TEST8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { test al,ah }
	SET_FLAGS;
}

// 8-bit INC (AL++) by C
u32 INC8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { inc al }
	SET_FLAGS;
}

// 8-bit DEC (AL--) by C
u32 DEC8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { dec al }
	SET_FLAGS;
}

// JO by C
u32 JO_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jo	jo_true
		mov	ax,0
		jmp	jo_stop
	jo_true:
		mov	ax,1
	jo_stop:
	}
	SET_FLAGS;
}

// JNO by C
u32 JNO_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jno	jno_true
		mov	ax,0
		jmp	jno_stop
	jno_true:
		mov	ax,1
	jno_stop:
	}
	SET_FLAGS;
}

// JB by C
u32 JB_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jb	jb_true
		mov	ax,0
		jmp	jb_stop
	jb_true:
		mov	ax,1
	jb_stop:
	}
	SET_FLAGS;
}

// JNB by C
u32 JNB_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jnb	jnb_true
		mov	ax,0
		jmp	jnb_stop
	jnb_true:
		mov	ax,1
	jnb_stop:
	}
	SET_FLAGS;
}

// JE by C
u32 JE_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		je	je_true
		mov	ax,0
		jmp	je_stop
	je_true:
		mov	ax,1
	je_stop:
	}
	SET_FLAGS;
}

// JNE by C
u32 JNE_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jne	jne_true
		mov	ax,0
		jmp	jne_stop
	jne_true:
		mov	ax,1
	jne_stop:
	}
	SET_FLAGS;
}

// JBE by C
u32 JBE_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jbe	jbe_true
		mov	ax,0
		jmp	jbe_stop
	jbe_true:
		mov	ax,1
	jbe_stop:
	}
	SET_FLAGS;
}

// JNBE by C
u32 JNBE_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jnbe jnbe_true
		mov	ax,0
		jmp	jnbe_stop
	jnbe_true:
		mov	ax,1
	jnbe_stop:
	}
	SET_FLAGS;
}

// JS by C
u32 JS_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		js	js_true
		mov	ax,0
		jmp	js_stop
	js_true:
		mov	ax,1
	js_stop:
	}
	SET_FLAGS;
}

// JNS by C
u32 JNS_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jns	jns_true
		mov	ax,0
		jmp	jns_stop
	jns_true:
		mov	ax,1
	jns_stop:
	}
	SET_FLAGS;
}

// JP by C
u32 JP_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jp	jp_true
		mov	ax,0
		jmp	jp_stop
	jp_true:
		mov	ax,1
	jp_stop:
	}
	SET_FLAGS;
}

// JNP by C
u32 JNP_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jnp	jnp_true
		mov	ax,0
		jmp	jnp_stop
	jnp_true:
		mov	ax,1
	jnp_stop:
	}
	SET_FLAGS;
}

// JL by C
u32 JL_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jl	jl_true
		mov	ax,0
		jmp	jl_stop
	jl_true:
		mov	ax,1
	jl_stop:
	}
	SET_FLAGS;
}

// JNL by C
u32 JNL_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jnl	jnl_true
		mov	ax,0
		jmp	jnl_stop
	jnl_true:
		mov	ax,1
	jnl_stop:
	}
	SET_FLAGS;
}

// JLE by C
u32 JLE_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jle	jle_true
		mov	ax,0
		jmp	jle_stop
	jle_true:
		mov	ax,1
	jle_stop:
	}
	SET_FLAGS;
}

// JNLE by C
u32 JNLE_C(u32 reg_flags)
{
	GET_FLAGS0;
	__asm {
		jnle jnle_true
		mov	ax,0
		jmp	jnle_stop
	jnle_true:
		mov	ax,1
	jnle_stop:
	}
	SET_FLAGS;
}

// DAA instruction by C
u32 DAA_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { daa }
	SET_FLAGS;
}

// DAS instruction by C
u32 DAS_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { das }
	SET_FLAGS;
}

// AAA instruction by C
u32 AAA_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { aaa }
	SET_FLAGS;
}

// AAS instruction by C
u32 AAS_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { aas }
	SET_FLAGS;
}

// AAD instruction by C
u32 AAD1_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm {
		; aad 1
		__emit 0xd5
		__emit 1
	}
	SET_FLAGS;
}

u32 AAD2_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm {
		; aad 2
		__emit 0xd5
		__emit 2
	}
	SET_FLAGS;
}

u32 AAD10_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { aad }
	SET_FLAGS;
}

u32 AAD25_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm {
		; aad 25
		__emit 0xd5
		__emit 25
	}
	SET_FLAGS;
}

u32 AAD255_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm {
		; aad 255
		__emit 0xd5
		__emit 255
	}
	SET_FLAGS;
}

// AAM instruction by C
u32 AAM1_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm {
		; aam 1
		__emit 0xd4
		__emit 1
	}
	SET_FLAGS;
}

u32 AAM2_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm {
		; aam 2
		__emit 0xd4
		__emit 2
	}
	SET_FLAGS;
}

u32 AAM10_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { aam }
	SET_FLAGS;
}

u32 AAM25_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm {
		; aam 25
		__emit 0xd4
		__emit 25
	}
	SET_FLAGS;
}

u32 AAM255_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm {
		; aam 255
		__emit 0xd4
		__emit 255
	}
	SET_FLAGS;
}

// 8-bit ROL,1 instruction by C
u32 ROL8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { rol al,1 }
	SET_FLAGS;
}

// 8-bit ROR,1 instruction by C
u32 ROR8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { ror al,1 }
	SET_FLAGS;
}

// 8-bit RCL,1 instruction by C
u32 RCL8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { rcl al,1 }
	SET_FLAGS;
}

// 8-bit RCR,1 instruction by C
u32 RCR8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { rcr al,1 }
	SET_FLAGS;
}

// 8-bit SHL,1 instruction by C
u32 SHL8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { shl al,1 }
	SET_FLAGS;
}

// 8-bit SHR,1 instruction by C
u32 SHR8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { shr al,1 }
	SET_FLAGS;
}

// 8-bit SAR,1 instruction by C
u32 SAR8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { sar al,1 }
	SET_FLAGS;
}

// 8-bit ROL,CL instruction by C
u32 ROL8N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { rol al,cl }
	SET_FLAGS;
}

// 8-bit ROR,CL instruction by C
u32 ROR8N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { ror al,cl }
	SET_FLAGS;
}

// 8-bit RCL,CL instruction by C
u32 RCL8N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { rcl al,cl }
	SET_FLAGS;
}

// 8-bit RCR,CL instruction by C
u32 RCR8N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { rcr al,cl }
	SET_FLAGS;
}

// 8-bit SHL,CL instruction by C
u32 SHL8N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { shl al,cl }
	SET_FLAGS;
}

// 8-bit SHR,CL instruction by C
u32 SHR8N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { shr al,cl }
	SET_FLAGS;
}

// 8-bit SAR,CL instruction by C
u32 SAR8N_C(u32 reg_flags, u32 reg_ax, u32 reg_cx)
{
	GET_FLAGS;
	__asm { mov ecx,reg_cx }
	__asm { sar al,cl }
	SET_FLAGS;
}

// 8-bit multiplication (AL*AH) by C
u32 MUL8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { mul ah }
	SET_FLAGS;
}

// 8-bit signed multiplication (AL*AH) by C
u32 IMUL8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { imul ah }
	SET_FLAGS;
}

// 8-bit division (AX/BL) by C
u32 DIV8_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm { mov ebx,reg_bx }
	__asm { div bl }
	SET_FLAGS;
}

// 8-bit signed division (AX/BL) by C
u32 IDIV8_C(u32 reg_flags, u32 reg_ax, u32 reg_bx)
{
	GET_FLAGS;
	__asm { mov ebx,reg_bx }
	__asm { idiv bl }
	SET_FLAGS;
}
