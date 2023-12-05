
// ****************************************************************************
//
//                                 CPU control
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

#ifndef _SDK_CPU_H
#define _SDK_CPU_H

//#include "resource.h"	// resources
#include "../sdk_addressmap.h"		// Register address offsets
#include "sdk_sio.h"			// SIO registers
#include "sdk_bootrom.h"

#ifdef __cplusplus
extern "C" {
#endif

// number of CPU cores
#define CORE_NUM	2

// get index of current processor core (0 or 1)
INLINE int CpuID(void) { return *((volatile u32*)(SIO_BASE+0)); }

// bit tables
extern const u8 RevBitsTab[16]; // reverse bit table
extern const u8 OrdBitsTab[16]; // order bit table

// compiler barrier
INLINE void cb(void)
{
	__asm volatile ("" ::: "memory");
}

// no operation (nop instruction)
INLINE void nop(void)
{
	__asm volatile (" nop\n" ::: "memory");
}

// no operation, 2 clock cycles
INLINE void nop2(void)
{
	__asm volatile (" b 1f\n1:\n" ::: "memory");
}

// disable global interrupts
INLINE void di(void)
{
	__asm volatile (" cpsid i\n" : : : "memory");
}

// enable global interrupts
INLINE void ei(void)
{
	__asm volatile (" cpsie i\n" : : : "memory");
}

// get global interrupts (returns True if interrupts are enabled)
INLINE Bool geti(void)
{
	u32 val;
	__asm volatile (" mrs %0,PRIMASK\n" : "=r" (val) :: "memory");
	return (Bool)((~val) & 1);
}

// set interrupts (True to enable interrupts)
INLINE void seti(Bool enable)
{
	if (enable) ei(); else di();	
}

// save and disable interrupts
INLINE u32 LockIRQ(void)
{
	u32 state;
	__asm volatile (" mrs %0,PRIMASK\n" : "=r" (state) :: "memory");
	__asm volatile (" cpsid i\n" : : : "memory");
	// cb() compiler barrier not needed after __asm
	return state;
}

// restore interrupts
INLINE void UnlockIRQ(u32 state)
{
	// cb() compiler barrier not needed before __asm
	__asm volatile (" msr PRIMASK,%0\n" :: "r" (state) : "memory");
}

// IRQ lock (temporary disables interrupt)
#define IRQ_LOCK u32 irq_state = LockIRQ()	// lock on begin of critical section
#define IRQ_UNLOCK UnlockIRQ(irq_state)		// unlock on end of critical section

// get current IRQ_* if the CPU is handling an exception (IRQ_INVALID = no exception, thread mode)
INLINE int GetCurrentIRQ(void)
{
	u32 exception;
	__asm volatile (" mrs %0,ipsr\n" : "=l" (exception));
	return (exception & 0x3f) - 16;
}

// get control register
INLINE u32 GetControl(void)
{
	u32 res;
	__asm volatile (" mrs %0,CONTROL\n" : "=r" (res));
	return res;
}

// set control register
INLINE void SetControl(u32 val)
{
	__asm volatile (" msr CONTROL,%0\n" :: "r" (val) : "memory");
}

// get process stack pointer
INLINE void* GetPsp(void)
{
	void* psp;
	__asm volatile (" mrs %0,psp\n" : "=r" (psp));
	return psp;
}

// set process stack pointer
INLINE void SetPsp(void* psp)
{
	__asm volatile (" msr psp,%0\n" : : "r" (psp));
}

// get main stack pointer
INLINE void* GetMsp(void)
{
	void* msp;
	__asm volatile (" mrs %0,msp\n" : "=r" (msp));
	return msp;
}

// set main stack pointer
INLINE void SetMsp(void* msp)
{
	__asm volatile (" msr msp,%0\n" : : "r" (msp));
}

// get current stack pointer
INLINE void* GetSp(void)
{
	void* res;
	__asm volatile (" mov %0,r13\n" : "=r" (res));
	return res;
}

// send event (to both cores)
INLINE void sev(void)
{
	__asm volatile (" sev\n");
}

// wait for event
INLINE void wfe(void)
{
	__asm volatile (" wfe\n");
}

// wait for interrupt (to wake up the core)
INLINE void wfi(void)
{
	__asm volatile (" wfi\n");
}

// instruction synchronization barrier
INLINE void isb(void)
{
	__asm volatile (" isb\n");
}

// data memory barrier
INLINE void dmb(void)
{
	__asm volatile (" dmb\n" ::: "memory");
}

// data synchronization barrier
INLINE void dsb(void)
{
	__asm volatile (" dsb\n" ::: "memory");
}

// reverse byte order of DWORD (exchange Big-endian and Little-endian byte order)
// (little endian LSB MSB = Intel, big endian MSB LSB = Motorola)
INLINE u32 Endian(u32 val)
{
	u32 res;
	__asm volatile (" rev %0,%1\n" : "=r" (res) : "r" (val));
	return res;
}

// swap bytes of WORD (exchange Big-endian and Little-endian byte order)
// (little endian LSB MSB = Intel, big endian MSB LSB = Motorola)
INLINE u16 Swap(u16 val)
{
	u32 val2 = val;
	u32 res;
	__asm volatile (" rev16 %0,%1\n" : "=r" (res) : "r" (val2));
	return (u16)res;
}

// swap bytes in two WORDs (exchange Big-endian and Little-endian byte order)
// (little endian LSB MSB = Intel, big endian MSB LSB = Motorola)
INLINE u32 Swap2(u32 val)
{
	u32 res;
	__asm volatile (" rev16 %0,%1\n" : "=r" (res) : "r" (val));
	return res;
}

// rotate bits in DWORD right by 'num' bits (carry lower bits to higher bits)
INLINE u32 Ror(u32 val, u8 num)
{
	u32 num2 = num;
	__asm volatile (" ror %0,%1\n" : "+r" (val) : "r" (num2));
	return val;
}

// rotate bits in DWORD left by 'num' bits (carry higher bits to lower bits)
INLINE u32 Rol(u32 val, u8 num)
{
	u32 num2 = 32 - num;
	__asm volatile (" ror %0,%1\n" : "+r" (val) : "r" (num2));
	return val;
}

// reverse 8 bits
u8 Reverse8(u8 val); // takes 170 ns
//INLINE u8 Reverse8(u8 val) { return (u8)(reverse(val) >> 24); } // takes 257 ns

// reverse 16 bits ... use faster ROM function reverse()
//u16 Reverse16(u16 val); // takes 670 ns
INLINE u16 Reverse16(u16 val) { return (u16)(reverse(val) >> 16); } // takes 257 ns

// reverse 32 bits ... use faster ROM function reverse()
//u32 Reverse32(u32 val); // takes 1120 ns
INLINE u32 Reverse32(u32 val) { return reverse(val); } // takes 250 ns

// get number of leading zeros of u32 number
// takes 296 ns ... better to use bootrom version clz(), it takes 161 ns
//u8 Clz(u32 val);
INLINE u8 Clz(u32 val) { return (u8)clz(val); }

// get number of leading zeros of u64 number
u8 clz64(u64 num);

// get bit order of 8-bit value (takes 152 ns) (logarithm, returns position of highest bit + 1: 1..8, 0=no bit)
u8 Order8(u8 val);

// get bit order of value (logarithm, returns position of highest bit + 1: 1..32, 0=no bit) ... use faster ROM function 32-clz()
//u8 Order(u32 val); // takes 217 ns
INLINE u8 Order(u32 val) { return (u8)(32 - clz(val)); } // takes 176 ns

// get bit order of 64-bit value (logarithm, returns position of highest bit + 1: 1..64, 0=no bit)
u8 Order64(u64 val);

// get mask of value (0x123 returns 0x1FF)
INLINE u32 Mask(u32 val) { return (u32)-1 >> clz(val); }

// range mask - returns bits set to '1' on range 'first' to 'last' (RangeMask(7,14) returns 0x7F80)
INLINE u32 RangeMask(int first, int last) { return (~((u32)-1 << (last+1))) & ((u32)-1 << first); }

#if USE_STACKCHECK	// use Stack check (sdk_cpu.c, sdk_cpu.h)

// stack top and bottom of core 0 and core 1
extern u8 __Stack0Top;
extern u8 __Stack1Top;
extern u8 __Stack0Bottom;
extern u8 __Stack1Bottom;

// get stack top of current CPU core
INLINE void* StackTop(void) { if (CpuID() == 0) return (void*)&__Stack0Top; return (void*)&__Stack1Top; }

// get stack bottom of current CPU core
INLINE void* StackBottom(void) { if (CpuID() == 0) return (void*)&__Stack0Bottom; return (void*)&__Stack1Bottom; }

// get current free space in the stack of current CPU core (distance from stack pointer to stack bottom)
INLINE int StackFree(void) { return (u32)GetSp() - (u32)StackBottom(); }

// feed stack of current CPU core with the food (returns free space, or 0 on stack error)
u32 StackFeed(void);

// check stack limit of current CPU core (check used stack space)
u32 StackCheck(void);

#endif // USE_STACKCHECK	// use Stack check (sdk_cpu.c, sdk_cpu.h)

// fatal error - display panic message and halt execution
void __attribute__((noreturn)) panic(const char *fmt, ...);

// stop execution
void __attribute__((noreturn)) isr_hardfault();
INLINE void __attribute__((noreturn)) __breakpoint() { isr_hardfault(); }

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// Insert a SEV instruction in to the code path.
INLINE void __sev(void) { sev(); }

// Insert a WFE instruction in to the code path.
INLINE void __wfe(void) { wfe(); }

// Insert a WFI instruction in to the code path.
INLINE void __wfi(void) { wfi(); }

// Insert a DMB instruction in to the code path.
INLINE void __dmb(void) { dmb(); }

// Insert a DSB instruction in to the code path.
INLINE void __dsb(void) { dsb(); }

// Insert a ISB instruction in to the code path.
INLINE void __isb(void) { isb(); }

// Acquire a memory fence
INLINE void __mem_fence_acquire(void) { dmb(); }

// Release a memory fence
INLINE void __mem_fence_release(void) { dmb(); }

// Save and disable interrupts
INLINE u32 save_and_disable_interrupts(void) { return LockIRQ(); }

// Restore interrupts to a specified state
INLINE void restore_interrupts(u32 status) { UnlockIRQ(status); }

INLINE void tight_loop_contents(void) {}

// get index of current processor core (0 or 1)
INLINE uint get_core_num(void) { return CpuID(); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_CPU_H
