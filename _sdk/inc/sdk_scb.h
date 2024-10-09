
// ****************************************************************************
//
//                            ARM System control block
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

#ifndef _SDK_SCB_H
#define _SDK_SCB_H

#include "../sdk_addressmap.h"		// Register address offsets

#ifdef __cplusplus
extern "C" {
#endif

#if !RISCV

// System Control Block
#if RP2040

typedef struct {
	io32	cpuid;		// 0x00: CPUID Base Register
	io32	icsr;		// 0x04: Interrupt Control and State Register
	io32	vtor;		// 0x08: Vector Table Offset Register
	io32	aircr;		// 0x0C: Application Interrupt and Reset Control Register
	io32	scr;		// 0x10: System Control Register
} armv6m_scb_hw_t;

STATIC_ASSERT(sizeof(armv6m_scb_hw_t) == 0x14, "Incorrect armv6m_scb_hw_t!");

#define scb_hw ((armv6m_scb_hw_t*)(PPB_BASE + 0xED00))

#else // RP2350

typedef struct {
	io32	cpuid;		// 0x00: CPUID Base Register
	io32	icsr;		// 0x04: Interrupt Control and State Register
	io32	vtor;		// 0x08: Vector Table Offset Register
	io32	aircr;		// 0x0C: Application Interrupt and Reset Control Register
	io32	scr;		// 0x10: System Control Register
	io32	ccr;		// 0x14: Configuration and control data Register
	io32	shpr[3];	// 0x18: System handlers priority register
	io32	shcsr;		// 0x24: System handlers control and status register
	io32	cfsr;		// 0x28: Configurable Fault Status Register
	io32	hfsr;		// 0x2C: Hard Fault Status Register
	io32	dfsr;		// 0x30: Debug Flag Status Register
	io32	mmfar;		// 0x34: MPU Memory Fault Address Register
	io32	bfar;		// 0x38: Buf Fault Address Register
	io32	_pad0;		// 0x3C:
	io32	id_pfr[2];	// 0x40: Instruction Set Supported Register
	io32	id_dfr0;	// 0x48: Debug System Register
	io32	id_afr0;	// 0x4C: Implementation Defined Featurs
	io32	id_mmfr[4];	// 0x50: Implemented memory model
	io32	id_isar[6];	// 0x60: Instruction set implemeneted
	io32	_pad1;		// 0x78:
	io32	ctr;		// 0x7C: Architecture of the cache
	io32	_pad2[2];	// 0x80:
	io32	cpacr;		// 0x88: Access privileges for coprocessors
	io32	nsacr;		// 0x8C: Non-secure access permissions
} armv8m_scb_hw_t;

STATIC_ASSERT(sizeof(armv8m_scb_hw_t) == 0x90, "Incorrect armv8m_scb_hw_t!");

#define scb_hw ((armv8m_scb_hw_t*)(PPB_BASE + 0xED00))
#define scb_ns_hw ((armv8m_scb_hw_t*)(PPB_NONSEC_BASE + 0xED00))

#define SCB_CPACR	((volatile u32*)(PPB_BASE + 0xED00 + 0x88)) // Coprocessor Access Control Register

// enable GPIO coprocessor CP0 on current CPU core
INLINE void GPIOC_Enable() { *SCB_CPACR |= B0 | B1; }

// disable GPIO coprocessor CP0 on current CPU core
INLINE void GPIOC_Disable() { *SCB_CPACR &= ~(B0 | B1); }

// enable double-precision coprocessor CP4 on current CPU core
INLINE void DCP_Enable()
{
	*SCB_CPACR |= B8 | B9;

	// clear engaged flag via RCMP
	asm volatile (" mrc p4,#0,r0,c0,c0,#1\n" : : : "r0");
}

// disable double-precision coprocessor CP4 on current CPU core
INLINE void DCP_Disable() { *SCB_CPACR &= ~(B8 | B9); }

// enable floating-point coprocessor CP10 on current CPU core
INLINE void FPU_Enable() { *SCB_CPACR |= B20 | B21; }

// disable floating-point coprocessor CP10 on current CPU core
INLINE void FPU_Disable() { *SCB_CPACR &= ~(B20 | B21); }

#endif

#endif // !RISCV

#ifdef __cplusplus
}
#endif

#endif // _SDK_SCB_H
