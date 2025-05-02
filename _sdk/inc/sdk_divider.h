
// ****************************************************************************
//
//                                  Divider
//                    Integer Division And Multiplication
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

// Notes:
// Every CPU core has its own integer divider unit.
//
// This SDK does not use the DIRTY flag (so it does not depend on the order
// of register reads), but locks the IRQ interrupt during access to the divider.
// Thus, there is no need to store the state of the divider during the interrupt.

// This SDK implementation does not trap the division by zero error (__aeabi_idiv0).
// The result of division by zero is undefined.

#ifndef _SDK_DIVIDER_H
#define _SDK_DIVIDER_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "sdk_sio.h"			// SIO registers
#include "sdk_cpu.h"			// CPU

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_sio.h"		// constants of original SDK
#else
#include "orig_rp2350/orig_sio.h"		// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// structure to save divider state
typedef struct { u32 values[4]; } hw_divider_state_t;

#if RP2040

// Divider hardware registers
//#define SIO_BASE		0xd0000000	// SIO registers
//  (SIO does not support aliases for atomic access!)
#define DIV_UDIVIDEND	((volatile u32*)(SIO_BASE+0x60)) // unsigned dividend 'p'
#define DIV_UDIVISOR	((volatile u32*)(SIO_BASE+0x64)) // unsigned divisor 'q'
#define DIV_SDIVIDEND	((volatile u32*)(SIO_BASE+0x68)) // signed dividend 'p'
#define DIV_SDIVISOR	((volatile u32*)(SIO_BASE+0x6C)) // signed divisor 'q'
#define DIV_QUOTIENT	((volatile u32*)(SIO_BASE+0x70)) // result quotient = p / q (must be read last, after remainder)
#define DIV_REMAINDER	((volatile u32*)(SIO_BASE+0x74)) // result remainder = p % q (quotient must be read too)
#define DIV_CSR		((volatile u32*)(SIO_BASE+0x78)) // control and status register

// Divider hardware interface
typedef struct {
	io32	udividend;	// 0x00: unsigned dividend 'p'
	io32	udivisor;	// 0x04: unsigned divisor 'q'
	io32	sdividend;	// 0x08: signed dividend 'p'
	io32	sdivisor;	// 0x0C: signed divisor 'q'
	io32	quotient;	// 0x10: result quotient = p / q (must be read last, after remainder)
	io32	remainder;	// 0x14: result remainder = p % q (quotient must be read too)
	io32	csr;		// 0x18: control and status register
} divider_hw_t;

#define	divider_hw	((divider_hw_t*)(SIO_BASE+0x60))

STATIC_ASSERT(sizeof(divider_hw_t) == 0x1C, "Incorrect divider_hw_t!");

// start signed division p/q
// Interrupt should be disabled.
INLINE void DivStartS32(s32 p, s32 q) { divider_hw->sdividend=(u32)p; divider_hw->sdivisor=(u32)q; }

// start unsigned division p/q
// Interrupt should be disabled.
INLINE void DivStartU32(u32 p, u32 q) { divider_hw->udividend=p; divider_hw->udivisor=q; }

// wait for divide to complete calculation (takes 8 clock cycles)
// Interrupt should be disabled.
INLINE void DivWait(void) { while((divider_hw->csr & B0) == 0) {} }

// get result remainder after complete calculation
// Interrupt should be disabled.
INLINE u32 DivRemainder(void) { return divider_hw->remainder; } // retype to (s32) after signed division

// get result quotient after complete calculation
// Interrupt should be disabled.
INLINE u32 DivQuotient(void) { return divider_hw->quotient; } // retype to (s32) after signed division

// get result after complete calculation: quotient p/q in low 32 bits, remainder p%q in high 32 bits
// Interrupt should be disabled.
INLINE u64 DivResult(void) { return ((u64)divider_hw->remainder << 32) | divider_hw->quotient; }

// save divider state (for calling core)
// Interrupt should be disabled.
void DivSaveState(hw_divider_state_t* dst);

// load divider state (for calling core)
// Interrupt should be disabled.
void DivLoadState(const hw_divider_state_t* src);

// Divide signed S32
//   a ... signed dividend s32
//   b ... signed divisor s32
//   result ... signed quotient s32 c=a/b
// Temporary disables interrupt to protect divider state.
// takes 280 ns
s32 DivS32(s32 a, s32 b);

// Divide unsigned U32
//   a ... unsigned dividend u32
//   b ... unsigned divisor u32
//   result ... unsigned quotient u32 c=a/b
// Temporary disables interrupt to protect divider state.
// takes 280 ns
u32 DivU32(u32 a, u32 b);

// Modulo signed S32
//   a ... signed dividend s32
//   b ... signed divisor s32
//   result ... signed remainder s32 d=a%b
// Temporary disables interrupt to protect divider state.
// takes 280 ns
s32 ModS32(s32 a, s32 b);

// Modulo unsigned U32
//   a ... unsigned dividend u32
//   b ... unsigned divisor u32
//   result ... unsigned remainder u32 d=a%b
// Temporary disables interrupt to protect divider state.
// takes 280 ns
u32 ModU32(u32 a, u32 b);

// Divide modulo signed S32
//   a ... signed dividend s32
//   b ... signed divisor s32
//   rem ... pointer to store remainder s32 d=a%b
//   result ... signed quotient s32 c=a/d
// Temporary disables interrupt to protect divider state.
s32 DivModS32(s32 a, s32 b, s32* rem);

// Divide modulo unsigned U32
//   a ... unsigned dividend u32
//   b ... unsigned divisor u32
//   rem ... pointer to store remainder u32 d=a%b
//   result ... unsigned quotient u32 c=a/d
// Temporary disables interrupt to protect divider state.
u32 DivModU32(u32 a, u32 b, u32* rem);

// Divide signed S64
//   a ... signed dividend s64
//   b ... signed divisor s64
//   result ... signed quotient s64 c=a/b
// Temporary disables interrupt to protect divider state.
// Takes 500 ns to 8 us
s64 DivS64(s64 a, s64 b);

// Divide unsigned U64
//   a ... unsigned dividend u64
//   b ... unsigned divisor u64
//   result ... unsigned quotient u64 c=a/b
// Temporary disables interrupt to protect divider state.
// Takes 500 ns to 8 us
u64 DivU64(u64 a, u64 b);

// Modulo signed S64
//   a ... signed dividend s64
//   b ... signed divisor s64
//   result ... signed remainder s64 d=a%b
// Temporary disables interrupt to protect divider state.
// Takes 500 ns to 8 us
s64 ModS64(s64 a, s64 b);

// Modulo unsigned U64
//   a ... unsigned dividend u64
//   b ... unsigned divisor u64
//   result ... unsigned remainder u64 d=a%b
// Temporary disables interrupt to protect divider state.
// Takes 500 ns to 8 us
u64 ModU64(u64 a, u64 b);

// Divide modulo signed S64
//   a ... signed dividend s64
//   b ... signed divisor s64
//   rem ... pointer to store remainder s64 d=a%b
//   result ... signed quotient s64 c=a/d
// Temporary disables interrupt to protect divider state.
// Takes 500 ns to 8 us
s64 DivModS64(s64 a, s64 b, s64* rem);

// Divide modulo unsigned U64
//   a ... unsigned dividend u64
//   b ... unsigned divisor u64
//   rem ... pointer to store remainder u64 d=a%b
//   result ... unsigned quotient u64 c=a/d
// Temporary disables interrupt to protect divider state.
// Takes 500 ns to 8 us
u64 DivModU64(u64 a, u64 b, u64* rem);

// Unsigned multiply u32*u32 with result u64
// Takes 330 ns
u64 UMul(u32 a, u32 b);

// Signed multiply s32*s32 with result s64
// Takes 370 ns
s64 SMul(s32 a, s32 b);

// Unsigned multiply u32*u32 with result u32 high, simplified, without carry from LOW
//u32 UMulHighSimple(u32 a, u32 b);

// Square u32*u32 with result u64
// Takes 240 ns
u64 Sqr(u32 a);

// Signed/unsigned multiply u64*u64 with result u64 (= __aeabi_lmul)
u64 UMul64(u64 a, u64 b);
s64 SMul64(s64 a, s64 b);

// check if integer number is power of 2
//INLINE Bool IsPow2(u32 a) { return ((a & (a-1)) == 0); }

#else // RP2040

// ------------------------------------------------------------------------------
//                              Divider emulation
// ------------------------------------------------------------------------------

// temporary saved result of divider emulation
extern u32 DivModResult_Div[CORE_NUM];
extern u32 DivModResult_Mod[CORE_NUM];

// save divider state (for calling core)
// Interrupt should be disabled.
INLINE void DivSaveState(hw_divider_state_t* dst) {}

// load divider state (for calling core)
// Interrupt should be disabled.
INLINE void DivLoadState(const hw_divider_state_t* src) {}

// Divide signed S32
//   a ... signed dividend s32
//   b ... signed divisor s32
//   result ... signed quotient s32 c=a/b
INLINE s32 DivS32(s32 a, s32 b) { return (b == 0) ? (s32)0x80000000ULL : (a/b); }

// Divide unsigned U32
//   a ... unsigned dividend u32
//   b ... unsigned divisor u32
//   result ... unsigned quotient u32 c=a/b
INLINE u32 DivU32(u32 a, u32 b) { return (b == 0) ? (u32)-1 : (a/b); }

// Modulo signed S32
//   a ... signed dividend s32
//   b ... signed divisor s32
//   result ... signed remainder s32 d=a%b
INLINE s32 ModS32(s32 a, s32 b) { return (b == 0) ? a : (a%b); }

// Modulo unsigned U32
//   a ... unsigned dividend u32
//   b ... unsigned divisor u32
//   result ... unsigned remainder u32 d=a%b
INLINE u32 ModU32(u32 a, u32 b) { return (b == 0) ? a : (a%b); }

// Divide modulo signed S32
//   a ... signed dividend s32
//   b ... signed divisor s32
//   rem ... pointer to store remainder s32 d=a%b
//   result ... signed quotient s32 c=a/d
INLINE s32 DivModS32(s32 a, s32 b, s32* rem)
{
	if (b == 0)
	{
		*rem = a;
		return (s32)0x80000000;
	}

	s32 res = a/b;
	*rem = a - res*b;
	return res;
}

// Divide modulo unsigned U32
//   a ... unsigned dividend u32
//   b ... unsigned divisor u32
//   rem ... pointer to store remainder u32 d=a%b
//   result ... unsigned quotient u32 c=a/d
INLINE u32 DivModU32(u32 a, u32 b, u32* rem)
{
	if (b == 0)
	{
		*rem = a;
		return (u32)-1;
	}

	u32 res = a/b;
	*rem = a - res*b;
	return res;
}

// Divide signed S64
//   a ... signed dividend s64
//   b ... signed divisor s64
//   result ... signed quotient s64 c=a/b
INLINE s64 DivS64(s64 a, s64 b) { return (b == 0) ? (s64)0x8000000000000000ULL : (a/b); }

// Divide unsigned U64
//   a ... unsigned dividend u64
//   b ... unsigned divisor u64
//   result ... unsigned quotient u64 c=a/b
INLINE u64 DivU64(u64 a, u64 b) { return (b == 0) ? (u64)-1LL : (a/b); }

// Modulo signed S64
//   a ... signed dividend s64
//   b ... signed divisor s64
//   result ... signed remainder s64 d=a%b
INLINE s64 ModS64(s64 a, s64 b) { return (b == 0) ? a : (a%b); }

// Modulo unsigned U64
//   a ... unsigned dividend u64
//   b ... unsigned divisor u64
//   result ... unsigned remainder u64 d=a%b
INLINE u64 ModU64(u64 a, u64 b) { return (b == 0) ? a : (a%b); }

// Divide modulo signed S64
//   a ... signed dividend s64
//   b ... signed divisor s64
//   rem ... pointer to store remainder s64 d=a%b
//   result ... signed quotient s64 c=a/d
INLINE s64 DivModS64(s64 a, s64 b, s64* rem)
{
	if (b == 0)
	{
		*rem = a;
		return (s64)0x8000000000000000ULL;
	}

	s64 res = a/b;
	*rem = a - res*b;
	return res;
}

// Divide modulo unsigned U64
//   a ... unsigned dividend u64
//   b ... unsigned divisor u64
//   rem ... pointer to store remainder u64 d=a%b
//   result ... unsigned quotient u64 c=a/d
INLINE u64 DivModU64(u64 a, u64 b, u64* rem)
{
	if (b == 0)
	{
		*rem = a;
		return (u64)-1LL;
	}

	u32 res = a/b;
	*rem = a - res*b;
	return res;
}

// Unsigned multiply u32*u32 with result u64
INLINE u64 UMul(u32 a, u32 b) { return (u64)a*b; }

// Signed multiply s32*s32 with result s64
INLINE s64 SMul(s32 a, s32 b) { return (s64)a*b; }

/*
// Unsigned multiply u32*u32 with result u32 high, simplified, without carry from LOW
INLINE u32 UMulHighSimple(u32 a, u32 b)
{
	u32 aL = (a & 0xffff);
	u32 aH = (a >> 16);
	u32 bL = (b & 0xffff);
	u32 bH = (b >> 16);

	u32 res = aH * bH;
	res += (aH * bL) >> 16;
	res += (aL * bH) >> 16;
	return res;
}
*/

// Square u32*u32 with result u64
INLINE u64 Sqr(u32 a) { return (u64)a*a; }

// Signed/unsigned multiply u64*u64 with result u64 (= __aeabi_lmul)
INLINE u64 UMul64(u64 a, u64 b) { return a*b; }
INLINE s64 SMul64(s64 a, s64 b) { return a*b; }

// check if integer number is power of 2
//INLINE Bool IsPow2(u32 a) { return ((a & (a-1)) == 0); }

// start signed division p/q
INLINE void DivStartS32(s32 p, s32 q)
{
	int core = CpuID();
	if (q == 0)
	{
		DivModResult_Div[core] = (s32)0x80000000ULL;
		DivModResult_Mod[core] = p;
	}
	else
	{
		DivModResult_Div[core] = p/q;
		DivModResult_Mod[core] = p%q;
	}
}

// start unsigned division p/q
INLINE void DivStartU32(u32 p, u32 q)
{
	int core = CpuID();
	if (q == 0)
	{
		DivModResult_Div[core] = (u32)-1;
		DivModResult_Mod[core] = p;
	}
	else
	{
		DivModResult_Div[core] = p/q;
		DivModResult_Mod[core] = p%q;
	}
}

// wait for divide to complete calculation (takes 8 clock cycles)
INLINE void DivWait(void) { }

// get result remainder after complete calculation
INLINE u32 DivRemainder(void) { return DivModResult_Mod[CpuID()]; }

// get result quotient after complete calculation
INLINE u32 DivQuotient(void) { return DivModResult_Div[CpuID()]; }

// get result after complete calculation: quotient p/q in low 32 bits, remainder p%q in high 32 bits
INLINE u64 DivResult(void)
{
	int core = CpuID();
	return ((u64)DivModResult_Mod[core] << 32) | DivModResult_Div[core];
}

#endif // RP2040

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// start signed division p/q (interrupt should be disabled)
INLINE void hw_divider_divmod_s32_start(s32 p, s32 q) { DivStartS32(p, q); }

// start unsigned division p/q (interrupt should be disabled)
INLINE void hw_divider_divmod_u32_start(u32 p, u32 q) { DivStartU32(p, q); }

// wait for divide to complete calculation (takes 8 clock cycles)
INLINE void hw_divider_wait_ready(void) { DivWait(); }

typedef u64 divmod_result_t;

// get result after complete calculation: quotient p/q in low 32 bits, remainder p%q in high 32 bits
INLINE divmod_result_t hw_divider_result_nowait(void) { return (divmod_result_t)DivResult(); }

// Return result of last asynchronous HW divide
//   result: Most significant 32 bits are the remainder, lower 32 bits are the quotient.
INLINE divmod_result_t hw_divider_result_wait(void)
{
	hw_divider_wait_ready();
	return hw_divider_result_nowait();
}

// Return result of last asynchronous HW divide, unsigned quotient only
INLINE u32 hw_divider_u32_quotient_wait(void)
{
	hw_divider_wait_ready();
	return DivQuotient();
}

// Return result of last asynchronous HW divide, signed quotient only
INLINE s32 hw_divider_s32_quotient_wait(void) { return (s32)hw_divider_u32_quotient_wait(); }

// Return result of last asynchronous HW divide, unsigned remainder only
INLINE u32 hw_divider_u32_remainder_wait(void)
{
	hw_divider_wait_ready();
	return DivRemainder();
}

// Return result of last asynchronous HW divide, signed remainder only
INLINE s32 hw_divider_s32_remainder_wait(void) { return (s32)hw_divider_u32_remainder_wait(); }

// Do a signed HW divide and wait for result
// Divide a by b, wait for calculation to complete, return result as a fixed point 32p32 value.
INLINE divmod_result_t hw_divider_divmod_s32(s32 a, s32 b) { return (((u64)(a%b))<<32u) | (u32)(a/b); }

// Do an unsigned HW divide and wait for result
// Divide \p a by \p b, wait for calculation to complete, return result as a fixed point 32p32 value.
INLINE divmod_result_t hw_divider_divmod_u32(u32 a, u32 b) { return (((u64)(a%b))<<32u) | (a/b); }

// Efficient extraction of unsigned quotient from 32p32 fixed point
INLINE u32 to_quotient_u32(divmod_result_t r) { return (u32)r; }

// Efficient extraction of signed quotient from 32p32 fixed point
INLINE s32 to_quotient_s32(divmod_result_t r) { return (s32)(u32)r; }

// Efficient extraction of unsigned remainder from 32p32 fixed point
INLINE u32 to_remainder_u32(divmod_result_t r) { return (u32)(r >> 32u); }

// Efficient extraction of signed remainder from 32p32 fixed point
INLINE s32 to_remainder_s32(divmod_result_t r) { return (s32)(r >> 32u); }

// Do an unsigned HW divide, wait for result, return quotient
INLINE u32 hw_divider_u32_quotient(u32 a, u32 b)
	{  return to_quotient_u32(hw_divider_divmod_u32(a, b)); }

// Do an unsigned HW divide, wait for result, return remainder
INLINE u32 hw_divider_u32_remainder(u32 a, u32 b)
	{ return to_remainder_u32(hw_divider_divmod_u32(a, b)); }

// Do a signed HW divide, wait for result, return quotient
INLINE s32 hw_divider_quotient_s32(s32 a, s32 b)
	{ return to_quotient_s32(hw_divider_divmod_s32(a, b)); }

// Do a signed HW divide, wait for result, return remainder
INLINE s32 hw_divider_remainder_s32(s32 a, s32 b)
	{ return to_remainder_s32(hw_divider_divmod_s32(a, b)); }

// Pause for exact amount of time needed for a asynchronous divide to complete
INLINE void hw_divider_pause(void) { DivWait(); }

// Do a hardware unsigned HW divide, wait for result, return quotient
//INLINE u32 hw_divider_u32_quotient(u32 a, u32 b) { return a / b; }
INLINE u32 hw_divider_u32_quotient_inlined(u32 a, u32 b)
	{ return hw_divider_u32_quotient(a,b); }

// Do a hardware unsigned HW divide, wait for result, return remainder
//INLINE u32 hw_divider_u32_remainder(u32 a, u32 b) { return a % b; }
INLINE u32 hw_divider_u32_remainder_inlined(u32 a, u32 b)
	{ return hw_divider_u32_remainder(a,b); }

// Do a hardware signed HW divide, wait for result, return quotient
INLINE s32 hw_divider_s32_quotient(s32 a, s32 b) { return a / b; }
INLINE s32 hw_divider_s32_quotient_inlined(s32 a, s32 b)
	{ return hw_divider_s32_quotient(a,b); }

// Do a hardware signed HW divide, wait for result, return remainder
INLINE s32 hw_divider_s32_remainder(s32 a, s32 b) { return a % b; }
INLINE s32 hw_divider_s32_remainder_inlined(s32 a, s32 b)
	{ return hw_divider_s32_remainder(a,b); }

// Save the calling cores hardware divider state
// Interrupt should be disabled.
INLINE void hw_divider_save_state(hw_divider_state_t* dst) { DivSaveState(dst); }

// Load a saved hardware divider state into the current core's hardware divider
// Interrupt should be disabled.
INLINE void hw_divider_restore_state(const hw_divider_state_t* src) { DivLoadState(src); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_DIVIDER_H
