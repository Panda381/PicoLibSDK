
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

#if RISCV
#include "orig_rp2350/orig_rvcsr.h"	// RISC-V registers
#endif

#ifdef __cplusplus
extern "C" {
#endif

// number of CPU cores
#define CORE_NUM	2

// get index of current processor core (0 or 1)
INLINE int CpuID(void)
{
#if RISCV
	u32 val;
	__asm volatile (" csrr %0,mhartid\n" : "=r" (val) :: "memory");
	return val;
#else // RISCV
	return *((volatile u32*)(SIO_BASE+0));
#endif
}

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
#if RP2040 // RP2040
	__asm volatile (" nop\n" ::: "memory");
#elif RISCV // RP2350 RISC-V
	__asm volatile (" nop\n" ::: "memory");
#else // RP2350 ARM
	__asm volatile (" nop.w\n" ::: "memory"); // force 16-bit nop
#endif
}

// no operation, 2 clock cycles
INLINE void nop2(void)
{
#if RISCV
	__asm volatile (" nop\n nop\n" ::: "memory");
#else // ARM
	__asm volatile (" b 1f\n1:\n" ::: "memory");
#endif
}

// disable global interrupts
INLINE void di(void)
{
#if RISCV
	__asm volatile (" csrci mstatus, 0x8\n" ::: "memory");
#else // ARM
	__asm volatile (" cpsid i\n" : : : "memory");
#endif
}

// enable global interrupts
INLINE void ei(void)
{
#if RISCV
	__asm volatile (" csrsi mstatus, 0x8\n" ::: "memory");
#else // ARM
	__asm volatile (" cpsie i\n" : : : "memory");
#endif
}

// get global interrupts (returns True if interrupts are enabled)
INLINE Bool geti(void)
{
	u32 val;
#if RISCV
	__asm volatile (" csrr %0, mstatus\n" : "=r" (val) :: "memory");
	return (Bool)((val >> 3) & 1);
#else // ARM
	__asm volatile (" mrs %0,PRIMASK\n" : "=r" (val) :: "memory");
	return (Bool)((~val) & 1);
#endif
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
#if RISCV
	__asm volatile (" csrrci %0, mstatus, 0x8\n" : "=r" (state) :: "memory");
#else // ARM
	__asm volatile (" mrs %0,PRIMASK\n" : "=r" (state) :: "memory");
	__asm volatile (" cpsid i\n" : : : "memory");
#endif
	// cb() compiler barrier not needed after __asm
	return state;
}

// restore interrupts
INLINE void UnlockIRQ(u32 state)
{
#if RISCV
	if ((state & 0x08) != 0)
		__asm volatile (" csrsi mstatus, 0x8\n" ::: "memory");
	else
		__asm volatile (" csrci mstatus, 0x8\n" ::: "memory");
#else // ARM
	// cb() compiler barrier not needed before __asm
	__asm volatile (" msr PRIMASK,%0\n" :: "r" (state) : "memory");
#endif
}

// IRQ lock (temporary disables interrupt)
#define IRQ_LOCK u32 irq_state = LockIRQ()	// lock on begin of critical section
#define IRQ_UNLOCK UnlockIRQ(irq_state)		// unlock on end of critical section

// busy wait at least number of sys_clk cycles
INLINE void BusyWaitCycles(u32 cycles)
{
	__asm volatile (
#if RISCV
		".option push\n"
		".option norvc\n" // force 32 bit addi, so branch prediction guaranteed
		".p2align 2\n"
		"1: \n"
		" addi %0, %0, -2 \n"
		" bgez %0, 1b\n"
		".option pop"
#else // ARM
		".syntax unified\n"
		"1: subs %0, #3\n"
		" bcs 1b\n"
#endif
		: "+r" (cycles) : : "cc", "memory");
}

// get current IRQ_* if the CPU is handling an exception (IRQ_INVALID = -16 = no exception, thread mode)
INLINE int GetCurrentIRQ(void)
{
	u32 exception;
#if RP2040 // RP2040
	__asm volatile (" mrs %0,ipsr\n" : "=l" (exception));
	return (exception & 0x3f) - 16;
#elif RISCV // RP2350 RISC-V
	u32 meicontext;
	__asm volatile (
		" csrr %0, %1\n"
		: "=r" (meicontext) : "i" (RVCSR_MEICONTEXT_OFFSET)
	);

	if (meicontext & RVCSR_MEICONTEXT_NOIRQ_BITS) return -16;
	return (meicontext & RVCSR_MEICONTEXT_IRQ_BITS) >> RVCSR_MEICONTEXT_IRQ_LSB;
#else // RP2350 ARM
	__asm volatile (
		" mrs %0, ipsr\n" 
		" uxtb %0, %0\n"
		: "=l" (exception));
	return exception - 16;
#endif
}

#if RISCV
// - constant 0..31 can use immediate mode

// RISC-V read from control register (_riscv_read_csr)
#define RISCV_READ_CSR(csrname) ({ u32 reg; __asm volatile (" csrr %0, " #csrname "\n" : "=r" (reg)); reg; })
#define _riscv_read_csr(csrname) RISCV_READ_CSR(csrname)
#define riscv_read_csr(csrname) RISCV_READ_CSR(csrname)

// RISC-V write to control register (_riscv_write_csr)
#define RISCV_WRITE_CSR(csrname, data) ({					\
	if (__builtin_constant_p(data) && !((data) & -32u)) {			\
		__asm volatile (" csrwi " #csrname ", %0\n" : : "i" (data));	\
	} else {								\
		__asm volatile (" csrw " #csrname ", %0\n" : : "r" (data));	\
	}})
#define _riscv_write_csr(csrname, data) RISCV_WRITE_CSR(csrname, data)
#define riscv_write_csr(csrname, data) RISCV_WRITE_CSR(csrname, data)

// RISC-V set bits in control register
#define RISCV_SET_CSR(csrname, data) ({						\
	if (__builtin_constant_p(data) && !((data) & -32u)) {			\
		__asm volatile (" csrsi " #csrname ", %0\n" : : "i" (data));	\
	} else {								\
		__asm volatile (" csrs " #csrname ", %0\n" : : "r" (data));	\
	}})
#define _riscv_set_csr(csrname, data) RISCV_SET_CSR(csrname, data)
#define riscv_set_csr(csrname, data) RISCV_SET_CSR(csrname, data)

// RISC-V clear bits in control register
#define RISCV_CLR_CSR(csrname, data) ({						\
	if (__builtin_constant_p(data) && !((data) & -32u)) {			\
		__asm volatile (" csrci " #csrname ", %0\n" : : "i" (data));	\
	} else {								\
		__asm volatile (" csrc " #csrname ", %0\n" : : "r" (data));	\
	}})
#define _riscv_clear_csr(csrname, data) RISCV_CLR_CSR(csrname, data)
#define riscv_clear_csr(csrname, data) RISCV_CLR_CSR(csrname, data)

// RISC-V read and write to control register
#define RISCV_READ_WRITE_CSR(csrname, data) ({ u32 reg;						\
	if (__builtin_constant_p(data) && !((data) & -32u)) {					\
		__asm volatile (" csrrwi %0, " #csrname ", %1\n" : "=r" (reg) : "i" (data));	\
	} else {										\
		__asm volatile (" csrrw %0, " #csrname ", %1\n" : "=r" (reg) : "r" (data));	\
	} reg; })
#define _riscv_read_write_csr(csrname, data) RISCV_READ_WRITE_CSR(csrname, data)
#define riscv_read_write_csr(csrname, data) RISCV_READ_WRITE_CSR(csrname, data)

// RISC-V read and set bits in control register
#define RISCV_READ_SET_CSR(csrname, data) ({ u32 reg;						\
	if (__builtin_constant_p(data) && !((data) & -32u)) {					\
		__asm volatile (" csrrsi %0, " #csrname ", %1\n" : "=r" (reg) : "i" (data));	\
	} else {										\
		__asm volatile (" csrrs %0, " #csrname ", %1\n" : "=r" (reg) : "r" (data));	\
	} reg; })
#define _riscv_read_set_csr(csrname, data) RISCV_READ_SET_CSR(csrname, data)
#define riscv_read_set_csr(csrname, data) RISCV_READ_SET_CSR(csrname, data)

// RISC-V read and clear bits in control register
#define RISCV_READ_CLR_CSR(csrname, data) ({ u32 reg;						\
	if (__builtin_constant_p(data) && !((data) & -32u)) {					\
		__asm volatile (" csrrci %0, " #csrname ", %1\n" : "=r" (reg) : "i" (data));	\
	} else {										\
		__asm volatile (" csrrc %0, " #csrname ", %1\n" : "=r" (reg) : "r" (data));	\
	} reg; })
#define _riscv_read_clear_csr(csrname, data) RISCV_READ_CLR_CSR(csrname, data)
#define riscv_read_clear_csr(csrname, data) RISCV_READ_CLR_CSR(csrname, data)

// Read IRQ array
#define RISCV_IRQARRAY_READ(csr, index) (RISCV_READ_SET_CSR(csr, (index)) >> 16)

// Write IRQ array
#define RISCV_IRQARRAY_WRITE(csr, index, data) (RISCV_WRITE_CSR(csr, (index) | ((u32)(data) << 16)))

// Set bits of IRQ array
#define RISCV_IRQARRAY_SET(csr, index, data) (RISCV_SET_CSR(csr, (index) | ((u32)(data) << 16)))

// Clear bits of IRQ array
#define RISCV_IRQARRAY_CLR(csr, index, data) (RISCV_CLR_CSR(csr, (index) | ((u32)(data) << 16)))

// RISC-V encode immediate U-format "lui"
INLINE u32 RiscvEncodeImmU(u32 x) { return (x >> 12) << 12; }
INLINE u32 riscv_encode_imm_u(u32 x) { return riscv_encode_imm_u(x); }

// RISC-V encode immediate I-format "addi"
INLINE u32 RiscvEncodeImmI(u32 x) { return (x & 0xfff) << 20; }
INLINE u32 riscv_encode_imm_i(u32 x) { return RiscvEncodeImmI(x); }

// RISC-V encode U-format of U+I 32-bit immediate
// - add signed 12-bit constant to the "lui" value, correct carry
INLINE u32 RiscvEncodeImmUHI(u32 x) { x += (x & 0x800) << 1; return RiscvEncodeImmU(x); }
INLINE u32 riscv_encode_imm_u_hi(u32 x) { return RiscvEncodeImmUHI(x); }

// RISC-V encode immediate B-format "bgeu"
INLINE u32 RiscvEncodeImmB(u32 x) { return
	(((x >> 12) & 0x01) << 31) |
	(((x >>  5) & 0x3f) << 25) |
	(((x >>  1) & 0x0f) <<  8) |
	(((x >> 11) & 0x01) <<  7); }
INLINE u32 riscv_encode_imm_b(u32 x) { return RiscvEncodeImmB(x); }

// RISC-V encode immediate S-format "sw"
INLINE u32 RiscvEncodeImmS(u32 x) { return
	(((x >> 5) & 0x7f) << 25) |
	(((x >> 0) & 0x1f) <<  7); }
INLINE u32 riscv_encode_imm_s(u32 x) { return RiscvEncodeImmS(x); }

// RISC-V encode immediate J-format "jal"
INLINE u32 RiscvEncodeImmJ(u32 x) { return
	(((x >> 20) & 0x001) << 31) |
	(((x >>  1) & 0x3ff) << 21) |
	(((x >> 11) & 0x001) << 20) |
	(((x >> 12) & 0x0ff) << 12); }
INLINE u32 riscv_encode_imm_j(u32 x) { return RiscvEncodeImmJ(x); }

// RISC-V encode immediate CJ-format "c.jal"
INLINE u16 RiscvEncodeImmCJ(u32 x) { return (u16)(
	(((x >> 11) & 0x1) << 12) |
	(((x >>  4) & 0x1) << 11) |
	(((x >>  8) & 0x3) <<  9) |
	(((x >> 10) & 0x1) <<  8) |
	(((x >>  6) & 0x1) <<  7) |
	(((x >>  7) & 0x1) <<  6) |
	(((x >>  1) & 0x7) <<  3) |
	(((x >>  5) & 0x1) <<  2)); }
INLINE u16 riscv_encode_imm_cj(u32 x) { return RiscvEncodeImmCJ(x); }

// RISC-V encode immediate CB-format "c.beqz"
INLINE u16 RiscvEncodeImmCB(u32 x) { return (u16)(
	(((x >> 8) & 0x1) << 12) |
	(((x >> 3) & 0x3) << 10) |
	(((x >> 6) & 0x3) <<  5) |
	(((x >> 1) & 0x3) <<  3) |
	(((x >> 5) & 0x1) <<  2)); }
INLINE u16 riscv_encode_imm_cb(u32 x) { return RiscvEncodeImmCB(x); }

// RISC-V encode immediate CI-format "c.addi"
INLINE u16 RiscvEncodeImmCI(u32 x) { return (u16)(
	(((x >> 5) & 0x01) << 12) |
	(((x >> 0) & 0x1f) <<  2)); }
INLINE u16 riscv_encode_imm_ci(u32 x) { return RiscvEncodeImmCI(x); }

#endif // RISCV

#if !RISCV
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
#endif // !RISCV

// get current stack pointer
INLINE void* GetSp(void)
{
	void* res;
#if RISCV
	__asm volatile (" mv %0,sp\n" : "=r" (res));
#else // ARM
	__asm volatile (" mov %0,r13\n" : "=r" (res));
#endif
	return res;
}

// send event (to both cores)
INLINE void sev(void)
{
#if RISCV
	__asm volatile (" slt x0, x0, x1\n" : : : "memory");
#else // ARM
	__asm volatile (" sev\n");
#endif
}

// wait for event
INLINE void wfe(void)
{
#if RISCV
	__asm volatile (" slt x0, x0, x0\n" : : : "memory");
#else // ARM
	__asm volatile (" wfe\n");
#endif
}

// wait for interrupt (to wake up the core)
INLINE void wfi(void)
{
	__asm volatile (" wfi\n");
}

// instruction synchronization barrier
INLINE void isb(void)
{
#if RISCV
	__asm volatile (" fence.i\n" ::: "memory");
#else // ARM
	__asm volatile (" isb\n" ::: "memory");
#endif
}

// data memory barrier
INLINE void dmb(void)
{
#if RISCV
	__asm volatile (" fence rw, rw\n" ::: "memory");
#else // ARM
	__asm volatile (" dmb\n" ::: "memory");
#endif
}

// data synchronization barrier
INLINE void dsb(void)
{
#if RISCV
	__asm volatile (" fence rw, rw\n" ::: "memory");
#else // ARM
	__asm volatile (" dsb\n" ::: "memory");
#endif
}

// check if CPU is in non-secure mode
INLINE Bool NonSecure(void)
{
#if RISCV
	// RISC-V has no secure concept
	return False;
#else // ARM
	u32 tt;
	__asm volatile (" movs %0,#0\n tt %0,%0\n" : "=r" (tt) : : "cc" );
	return (tt & B22) == 0;
#endif
}

// reverse byte order of DWORD (exchange Big-endian and Little-endian byte order)
// (little endian LSB MSB = Intel, big endian MSB LSB = Motorola)
INLINE u32 Endian(u32 val)
{
	u32 res;
#if RISCV
	__asm volatile (" rev8 %0,%1\n" : "=r" (res) : "r" (val));
//	res = __builtin_bswap32(val);
#else // ARM
	__asm volatile (" rev %0,%1\n" : "=r" (res) : "r" (val));
#endif
	return res;
}

// swap bytes of WORD (exchange Big-endian and Little-endian byte order)
// (little endian LSB MSB = Intel, big endian MSB LSB = Motorola)
INLINE u16 Swap(u16 val)
{
#if RISCV
	return __builtin_bswap16(val);
#else // ARM
	u32 val2 = val;
	u32 res;
	__asm volatile (" rev16 %0,%1\n" : "=r" (res) : "r" (val2));
	return (u16)res;
#endif
}

// swap bytes in two WORDs (exchange Big-endian and Little-endian byte order)
// (little endian LSB MSB = Intel, big endian MSB LSB = Motorola)
INLINE u32 Swap2(u32 val)
{
#if RISCV
	return __builtin_bswap16((u16)val) | ((u32)__builtin_bswap16((u16)(val >> 16)) << 16);
#else // ARM
	u32 res;
	__asm volatile (" rev16 %0,%1\n" : "=r" (res) : "r" (val));
	return res;
#endif
}

// rotate bits in DWORD right by 'num' bits (carry lower bits to higher bits)
INLINE u32 Ror(u32 val, u8 num)
{
#if RISCV
	num &= 0x1f;
	val = (val >> num) | (val << (32 - num));
#else // ARM
	u32 num2 = num;
	__asm volatile (" ror %0,%1\n" : "+l" (val) : "l" (num2) : "cc");
#endif
	return val;
}

// rotate bits in DWORD left by 'num' bits (carry higher bits to lower bits)
INLINE u32 Rol(u32 val, u8 num)
{
#if RISCV
	num &= 0x1f;
	val = (val << num) | (val >> (32 - num));
#else // ARM
	u32 num2 = 32 - num;
	__asm volatile (" ror %0,%1\n" : "+l" (val) : "l" (num2) : "cc");
#endif
	return val;
}

// reverse 32 bits
INLINE u32 Reverse32(u32 val)
{
#if RP2040
	return reverse(val);
#elif RISCV
	u32 res;
	__asm volatile (" rev8 %0,%1\n brev8 %0,%0\n" : "=r" (res) : "r" (val));
	return res;
#else // RP2350
	u32 res;
	__asm volatile (" rbit %0,%1\n" : "=r" (res) : "r" (val));
	return res;
#endif
}

#if !RP2040
INLINE u32 reverse(u32 val) { return Reverse32(val); }
#endif

INLINE u32 __rev(u32 val) { return Reverse32(val); }

// reverse 16 bits
INLINE u16 Reverse16(u16 val)
{
	return (u16)(Reverse32(val) >> 16);
}

// reverse 8 bits
#if RP2040
u8 Reverse8(u8 val);
#else
INLINE u8 Reverse8(u8 val)
{
	u32 val2 = val;
	u32 res;
#if RISCV
	__asm volatile (" brev8 %0,%1\n" : "=r" (res) : "r" (val2));
	return (u8)res;
#else // RP2350
	__asm volatile (" rbit %0,%1\n" : "=r" (res) : "r" (val2));
	return (u8)(res >> 24);
#endif
}
#endif

// reverse 64 bits
INLINE u64 Reverse64(u64 val)
{
	return ((u64)Reverse32((u32)val) << 32) | Reverse32((u32)(val >> 32));
}

INLINE u64 __revll(u64 val) { return Reverse64(val); }

// get number of leading zeros of u32 number
#if RP2040
INLINE u32 Clz(u32 val) { return clz(val); }
#else
INLINE u32 Clz(u32 val)
{
	u32 res;
	__asm volatile (" clz %0,%1\n" : "=r" (res) : "r" (val));
	return res;
}

INLINE u32 clz(u32 val) { return Clz(val); }
#endif

// get number of leading zeros of u64 number
INLINE u32 Clz64(u64 num)
{
	if (num >= 0x100000000ULL)
		return clz((u32)(num >> 32));
	else
		return clz((u32)num) + 32;
}

INLINE u32 clz64(u64 val) { return Clz64(val); }

// get number of trailing zeros of u32 number
#if RP2040
INLINE u32 Ctz(u32 val) { return ctz(val); }
#else
INLINE u32 Ctz(u32 val)
{
#if RISCV
	u32 res;
	__asm volatile (" ctz %0,%1\n" : "=r" (res) : "r" (val));
	return res;
#else
	u32 res;
	__asm volatile (" rbit %0,%1\n clz %0,%0\n" : "=r" (res) : "r" (val));
	return res;
#endif
}

INLINE u32 ctz(u32 val) { return Ctz(val); }
#endif

// get number of trailing zeros of u64 number
INLINE u32 Ctz64(u64 num)
{
	if ((u32)num == 0)
		return ctz((u32)(num >> 32)) + 32;
	else
		return ctz((u32)num);
}

INLINE u32 ctz64(u64 val) { return Ctz64(val); }

// counts '1' bits (population count)
#if RP2040
INLINE u32 Popcount(u32 val) { return popcount(val); }
#else
INLINE u32 Popcount(u32 val) { return __builtin_popcount(val); }
INLINE u32 popcount(u32 val) { return Popcount(val); }
#endif

INLINE u32 Popcount64(u64 val) { return Popcount((u32)val) + Popcount((u32)(val >> 32)); }

// get bit order of 8-bit value (logarithm, returns position of highest bit + 1: 1..8, 0=no bit)
#if RP2040
u8 Order8(u8 val);
#else
INLINE u32 Order8(u8 val) { return 32 - Clz((u32)(u8)val); }
#endif

// get bit order of value (logarithm, returns position of highest bit + 1: 1..32, 0=no bit)
INLINE u32 Order(u32 val) { return 32 - Clz(val); }

// get bit order of 64-bit value (logarithm, returns position of highest bit + 1: 1..64, 0=no bit)
INLINE u32 Order64(u64 val)
{
	if (val >= 0x100000000ULL)
		return Order((u32)(val >> 32)) + 32;
	else
		return Order((u32)val);
}

// get mask of value (0x123 returns 0x1FF)
INLINE u32 Mask(u32 val) { return ((u32)-1) >> Clz(val); }

// range mask - returns bits set to '1' on range 'first' to 'last' (RangeMask(7,14) returns 0x7F80)
INLINE u32 RangeMask(int first, int last) { return (~(((u32)-1) << (last+1))) & (((u32)-1) << first); }

// range mask - returns bits set to '1' on range 'first' to 'last' (RangeMask(7,14) returns 0x7F80)
INLINE u64 RangeMask64(int first, int last) { return (~(((u64)-1) << (last+1))) & (((u64)-1) << first); }

// check if integer number is power of 2
INLINE Bool IsPow2(u32 a) { return ((a & (a-1)) == 0); }

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
//void __attribute__((noreturn)) isr_hardfault();
//INLINE void __attribute__((noreturn)) __breakpoint() { isr_hardfault(); }

void __attribute__((noreturn)) FatalErrorStop();
INLINE void __attribute__((noreturn)) __breakpoint() { FatalErrorStop(); }

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
