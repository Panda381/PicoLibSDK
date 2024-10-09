
// ****************************************************************************
//
//                                    IRQ
//                             Interrupt Request
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

// NVIC = nested vector interrupt control
// VTOR = vector table offset register


// Each core has its own pointer to vector table VTOR and its own interrupt controller NVIC.
// Almost all functions can only be used for interrupts from the RP2040 devices (IRQ >= 0).
// A few functions can be used to setup Cortex-M0+ core exceptions SVCall, PendSV and SysTick.

// Shared interrupts are not supported. If you need to share IRQ for multiple handlers,
// call all other handlers from your interrupt yourself.

#if USE_IRQ	// use IRQ interrupts (sdk_irq.c, sdk_irq.h)

#ifndef _SDK_IRQ_H
#define _SDK_IRQ_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "sdk_cpu.h"
#include "sdk_scb.h"

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_m0plus.h"		// constants of original SDK
#else
#include "orig_rp2350/orig_m33.h"		// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// interrupt request indices
#if RP2040		// 1=use MCU RP2040
#include "sdk_irq_rp2040.h"
#endif

#if RP2350		// 1=use MCU RP2350
#include "sdk_irq_rp2350.h"
#endif

// index of first IRQ
#if RISCV
#define VTABLE_FIRST_IRQ	26	// RISCV
#else
#define VTABLE_FIRST_IRQ	16	// ARM
#endif

// interrupt hardware priority
#define IRQ_PRIO_REALTIME	0	// highest priority - real-time time-critical interrupts
#define IRQ_PRIO_SYSTICK	1	// high priority - SysTick (must have higher priority than normal interrupts)
#define IRQ_PRIO_NORMAL		2	// default normal priority - normal code, normal interrupts
#define IRQ_PRIO_PENDSV		3	// lowest priority - PendSV handler (must have lowest priority!)

// common interrupt handler definition
typedef void (*irq_handler_t)(void);

// === System control block

// NVIC hardware registers
#define NVIC_ISER ((volatile u32*)(PPB_BASE + 0xE100))	// address of interrupt set-enable register
#define NVIC_ICER ((volatile u32*)(PPB_BASE + 0xE180))	// address of interrupt clear-enable register
#define NVIC_ISPR ((volatile u32*)(PPB_BASE + 0xE200))	// address of interrupt set-pending register
#define NVIC_ICPR ((volatile u32*)(PPB_BASE + 0xE280))	// address of interrupt clear-pending register

// IRQ mask
#if RP2040
typedef u32 iqr_mask_t;		// 32-bit IRQ mask
INLINE u32 IrqRangeMask(int first, int last) { return RangeMask(first, last); } // mask of range of IRQs
#else
typedef u64 iqr_mask_t;		// 64-bit IRQ mask
INLINE u64 IrqRangeMask(int first, int last) { return RangeMask64(first, last); } // mask of range of IRQs
#endif

#define NVIC_IPR0 ((volatile u32*)(PPB_BASE + 0xE400))	// address of interrupt priority registers

// NVIC hardware interface
#if RP2040
typedef struct {
	io32	iser;		// 0x000: Interrupt Set-Enable Register
	io32	_pad0[31];
	io32	icer;		// 0x080: Interrupt Clear-Enable Register
	io32	_pad1[31];
	io32	ispr;		// 0x100: Interrupt Set-Pending Register
	io32	_pad2[31];
	io32	icpr;		// 0x180: Interrupt Clear-Pending Register
	io32	_pad3[95];
	io32	ipr[8];		// 0x300: Interrupt Priority Register
} nvic_hw_t;
STATIC_ASSERT(sizeof(nvic_hw_t) == 0x320, "Incorrect nvic_hw_t!");
#else
typedef struct {
	io32	iser[2];	// 0x000: Interrupt Set-Enable Registers
	io32	_pad0[30];
	io32	icer[2];	// 0x080: Interrupt Clear-Enable Registers
	io32	_pad1[30];
	io32	ispr[2];	// 0x100: Interrupt Set-Pending Registers
	io32	_pad2[30];
	io32	icpr[2];	// 0x180: Interrupt Clear-Pending Registers
	io32	_pad3[30];
	io32	iabr[2];	// 0x200: Interrupt Active State Registers
	io32	_pad4[30];
	io32	itns[2];	// 0x280: Interrupt Target Non-secure Registers
	io32	_pad5[30];
	io32	ipr[16];	// 0x300: Interrupt Priority Registers
} nvic_hw_t;
STATIC_ASSERT(sizeof(nvic_hw_t) == 0x340, "Incorrect nvic_hw_t!");
#endif

#define nvic_hw ((nvic_hw_t *)(PPB_BASE + 0xE100))

// === System control block of the Cortex CPU core

// IRQ hardware registers
#define SCB_CPUID	((volatile u32*)(PPB_BASE + 0xED00))	// CPUID Base Register
#define SCB_ICSR	((volatile u32*)(PPB_BASE + 0xED04))	// Interrupt Control and State Register
#define SCB_VTOR	((volatile u32*)(PPB_BASE + 0xED08))	// Vector Table Offset Register
#define SCB_AIRCR	((volatile u32*)(PPB_BASE + 0xED0C))	// Application Interrupt and Reset Control Register
#define SCB_SCR		((volatile u32*)(PPB_BASE + 0xED10))	// System Control Register

#define SCB_SHPR2	((volatile u32*)(PPB_BASE + 0xED1C))	// address of system handler priority 2
								//	(bit 30..31 = priority of SVCall)
#define SCB_SHPR3	((volatile u32*)(PPB_BASE + 0xED20))	// address of system handler priority 3
								//	(bit 30..31: priority of SysTick, bit 22..23: priority of PendSV)

#if RISCV // && !defined(RVCSR_MTVEC_OFFSET)
#undef RVCSR_MTVEC_OFFSET
#undef RVCSR_MEIEA_OFFSET
#undef RVCSR_MEIFA_OFFSET
#define RVCSR_MTVEC_OFFSET	0x305	// machine trap-handler base address 'mtvec'
#define RVCSR_MEIEA_OFFSET	0xbe0	// external interrupt enable array
#define RVCSR_MEIFA_OFFSET	0xbe2	// external interrupt force array
#endif

// size of vector table in bytes
// This constant must correspond to real vector table size in crt0_*.S
#if RISCV
#define VECTOR_TABLE_SIZE	0x300	// RISC-V vectors including exception handlers = 768
#else // RP2040, RP2350
#define VECTOR_TABLE_SIZE	((IRQ_NUM+VTABLE_FIRST_IRQ)*4)	// RP2040: 48*4=192, RP2350: 68*4=272
#endif

// copy of vector table in RAM
#if !NO_FLASH	// In the RAM version, the vector table is already in RAM
#if USE_IRQ	// use IRQ interrupts (sdk_irq.c, sdk_irq.h)
// copy of vector table in RAM ... must be aligned to 256 bytes
extern u32 __attribute__((section(".ram_vector_table"))) __attribute__((aligned(256))) RamVectorTable[VECTOR_TABLE_SIZE];
#endif
#endif

// get address of interrupt vector table of this CPU core
#if RISCV
INLINE irq_handler_t* GetVTOR(void) { return (irq_handler_t*)(RISCV_READ_CSR(0x305) & ~0x3u); } // RVCSR_MTVEC_OFFSET (clear MODE bits 0 and 1)
#else
INLINE irq_handler_t* GetVTOR(void) { return (irq_handler_t*)*SCB_VTOR; }
#endif

// set address of interrupt vector table of this CPU core
#if RISCV // 0x305 = mtvec, machine trap-handler base address
INLINE void SetVTOR(irq_handler_t* addr) { cb(); RISCV_WRITE_CSR(0x305, ((u32)addr & ~0x3u) | 1); cb(); } // RVCSR_MTVEC_OFFSET; set VECTOR mode
#else
INLINE void SetVTOR(irq_handler_t* addr) { cb(); *SCB_VTOR = (u32)addr; cb(); }
#endif

// Set thumb bit of the address
#if RISCV
INLINE void* SetThumbBit(void* addr) { return addr; }
#else
INLINE void* SetThumbBit(void* addr) { return (void*)(((u32)addr) | 1); }
#endif

// Clear thumb bit of the address
#if RISCV
INLINE void* ClrThumbBit(void* addr) { return addr; }
#else
INLINE void* ClrThumbBit(void* addr) { return (void*)(((u32)addr) & (u32)~1); }
#endif

// set interrupt service handler (irq = interrupt request indice IRQ_*, negative exception numbers can also be used)
// - vector table must be located in RAM
// - vector table must be aligned to 256 bytes (RISC-V: 64 bytes)
// - if vector table is not in RAM, use services with names isr_*, it overrides the weak handler
// - vector table in this SDK is shared between both CPU cores
// - can be used to set exception handlers (irq < 0)
INLINE void SetHandler(int irq, irq_handler_t handler) { cb(); GetVTOR()[VTABLE_FIRST_IRQ + irq] = handler; dmb(); }

// get current interrupt servie handler (irq = interrupt request indice IRQ_*, negative exception numbers can also be used)
INLINE irq_handler_t GetHandler(int irq) { return GetVTOR()[VTABLE_FIRST_IRQ + irq]; }

// clear pending interrupts of NVIC masked of this CPU core (bit 0.. = IRQ 0..)
//  To use IRQ mask in range (first..last), use function IrqRangeMask.
INLINE void NVIC_IRQClearMask(iqr_mask_t mask)
{
	cb();
#if RP2040
	nvic_hw->icpr = mask;
#elif RISCV
	RISCV_IRQARRAY_CLR(RVCSR_MEIFA_OFFSET, 0, (u16)mask);
	RISCV_IRQARRAY_CLR(RVCSR_MEIFA_OFFSET, 1, (u16)(mask >> 16));
	RISCV_IRQARRAY_CLR(RVCSR_MEIFA_OFFSET, 2, (u16)(mask >> 32));
	RISCV_IRQARRAY_CLR(RVCSR_MEIFA_OFFSET, 3, (u16)(mask >> 48));
#else
	nvic_hw->icpr[0] = (u32)mask;
	nvic_hw->icpr[1] = (u32)(mask >> 32);
#endif
	cb();
}

// clear pending interrupt of NVIC of this CPU core (irq = 0..25 or 0..51)
INLINE void NVIC_IRQClear(int irq)
{
#if RP2040
	nvic_hw->icpr = BIT(irq);
#elif RISCV
	RISCV_IRQARRAY_CLR(RVCSR_MEIFA_OFFSET, irq >> 4, BIT(irq & 0x0f));
#else
	nvic_hw->icpr[irq >> 5] = BIT(irq & 0x1f);
#endif
}

// set pending interrupts of NVIC masked of this CPU core (force interrupt; bit 0.. = IRQ 0..)
//  To use IRQ mask in range (first..last), use function IrqRangeMask.
INLINE void NVIC_IRQForceMask(iqr_mask_t mask)
{
	cb();
#if RP2040
	nvic_hw->ispr = mask;
#elif RISCV
	RISCV_IRQARRAY_SET(RVCSR_MEIFA_OFFSET, 0, (u16)mask);
	RISCV_IRQARRAY_SET(RVCSR_MEIFA_OFFSET, 1, (u16)(mask >> 16));
	RISCV_IRQARRAY_SET(RVCSR_MEIFA_OFFSET, 2, (u16)(mask >> 32));
	RISCV_IRQARRAY_SET(RVCSR_MEIFA_OFFSET, 3, (u16)(mask >> 48));
#else
	nvic_hw->ispr[0] = (u32)mask;
	nvic_hw->ispr[1] = (u32)(mask >> 32);
#endif
	cb();
}

// set pending interrupt of NVIC of this CPU core (force interrupt) (irq = 0..25 or 0..51)
INLINE void NVIC_IRQForce(int irq)
{
#if RP2040
	nvic_hw->ispr = BIT(irq);
#elif RISCV
	RISCV_IRQARRAY_SET(RVCSR_MEIFA_OFFSET, irq >> 4, BIT(irq & 0x0f));
#else
	nvic_hw->ispr[irq >> 5] = BIT(irq & 0x1f);
#endif
}

// check if interrupt of NVIC of this CPU core is pending (irq = 0..25 or 0..51)
INLINE Bool NVIC_IRQIsPending(int irq)
{
#if RP2040
	return (nvic_hw->ispr & BIT(irq)) != 0;
#elif RISCV
	return (RISCV_IRQARRAY_READ(RVCSR_MEIFA_OFFSET, irq >> 4) & BIT(irq & 0x0f)) != 0;
#else
	return (nvic_hw->ispr[irq >> 5] & BIT(irq & 0x1f)) != 0;
#endif
}

// enable interrupts of NVIC masked of this CPU core
//  To use IRQ mask in range (first..last), use function IrqRangeMask.
INLINE void NVIC_IRQEnableMask(iqr_mask_t mask)
{
	// clear pending interrupts
	NVIC_IRQClearMask(mask);

	cb();
#if RP2040
	nvic_hw->iser = mask;
#elif RISCV
	RISCV_IRQARRAY_SET(RVCSR_MEIEA_OFFSET, 0, (u16)mask);
	RISCV_IRQARRAY_SET(RVCSR_MEIEA_OFFSET, 1, (u16)(mask >> 16));
	RISCV_IRQARRAY_SET(RVCSR_MEIEA_OFFSET, 2, (u16)(mask >> 32));
	RISCV_IRQARRAY_SET(RVCSR_MEIEA_OFFSET, 3, (u16)(mask >> 48));
#else
	nvic_hw->iser[0] = (u32)mask;
	nvic_hw->iser[1] = (u32)(mask >> 32);
#endif
	cb();
}

// enable interrupt of NVIC of this CPU core (irq = 0..25 or 0..51)
INLINE void NVIC_IRQEnable(int irq)
{
	// clear pending interrupt
	NVIC_IRQClear(irq);

	cb();
#if RP2040
	nvic_hw->iser = BIT(irq);
#elif RISCV
	RISCV_IRQARRAY_SET(RVCSR_MEIEA_OFFSET, irq >> 4, BIT(irq & 0x0f));
#else
	nvic_hw->iser[irq >> 5] = BIT(irq & 0x1f);
#endif
	cb();
}

// disable interrupts of NVIC masked of this CPU core
//  To use IRQ mask in range (first..last), use function IrqRangeMask.
INLINE void NVIC_IRQDisableMask(iqr_mask_t mask)
{
	cb();
#if RP2040
	nvic_hw->icer = mask;
#elif RISCV
	RISCV_IRQARRAY_CLR(RVCSR_MEIEA_OFFSET, 0, (u16)mask);
	RISCV_IRQARRAY_CLR(RVCSR_MEIEA_OFFSET, 1, (u16)(mask >> 16));
	RISCV_IRQARRAY_CLR(RVCSR_MEIEA_OFFSET, 2, (u16)(mask >> 32));
	RISCV_IRQARRAY_CLR(RVCSR_MEIEA_OFFSET, 3, (u16)(mask >> 48));
#else
	nvic_hw->icer[0] = (u32)mask;
	nvic_hw->icer[1] = (u32)(mask >> 32);
#endif
	// recommended in cmsis cortex source code:
	dsb();
	isb();
}

// disable interrupt of NVIC of this CPU core (irq = 0..25 or 0..51)
INLINE void NVIC_IRQDisable(int irq)
{
	cb();
#if RP2040
	nvic_hw->icer = BIT(irq);
#elif RISCV
	RISCV_IRQARRAY_CLR(RVCSR_MEIEA_OFFSET, irq >> 4, BIT(irq & 0x0f));
#else
	nvic_hw->icer[irq >> 5] = BIT(irq & 0x1f);
#endif
	// recommended in cmsis cortex source code:
	dsb();
	isb();
}

// check if interrupt of NVIC of this CPU core is enabled (irq = 0..25 or 0..51)
INLINE Bool NVIC_IRQIsEnabled(int irq)
{
#if RP2040
	return (nvic_hw->iser & BIT(irq)) != 0;
#elif RISCV
	return (RISCV_IRQARRAY_READ(RVCSR_MEIEA_OFFSET, irq >> 4) & BIT(irq & 0x0f)) != 0;
#else
	return (nvic_hw->iser[irq >> 5] & BIT(irq & 0x1f)) != 0;
#endif
}

// set interrupt priority of NVIC of this CPU core (prio = priority IRQ_PRIO_*) (irq = 0..25 or 0..51)
#if RISCV
INLINE void NVIC_IRQPrio(int irq, u8 prio) {}
#else
void NVIC_IRQPrio(int irq, u8 prio);
#endif

// set all interrupt priorities of NVIC of this CPU core to default value
#if RISCV
INLINE void NVIC_IRQPrioDef(void) {}
#else
void NVIC_IRQPrioDef(void);
#endif

// set interrupt priority of SVCall exception -5 of this CPU core (prio = priority IRQ_PRIO_*)
#if !RISCV
INLINE void NVIC_SVCallPrio(int prio) {	*SCB_SHPR2 = (*SCB_SHPR2 & ~(B30 | B31)) | ((u32)prio << 30); }
#endif

// set interrupt priority of PendSV exception -2 of this CPU core (prio = priority IRQ_PRIO_*)
#if !RISCV
INLINE void NVIC_PendSVPrio(int prio) { *SCB_SHPR3 = (*SCB_SHPR3 & ~(B22 | B23)) | ((u32)prio << 22); }
#endif

// raise PendSV exception of this CPU core (reschedule multitask system)
#if !RISCV
INLINE void NVIC_PendSVForce(void) { *SCB_ICSR |= B28; }
#endif

// check if PendSV exception of this CPU core is pending
#if !RISCV
INLINE Bool NVIC_PendSVIsPending(void) { return (*SCB_ICSR & B28) != 0; }
#endif

// clear PendSV pending state of this CPU core
#if !RISCV
INLINE void NVIC_PendSVClear(void) { *SCB_ICSR |= B27; }
#endif

// set interrupt priority of SysTick exception -1 of this CPU core (prio = priority IRQ_PRIO_*)
#if RISCV
INLINE void NVIC_SysTickPrio(int prio) { }
#else
INLINE void NVIC_SysTickPrio(int prio) { *SCB_SHPR3 = (*SCB_SHPR3 & ~(B30 | B31)) | ((u32)prio << 30); }
#endif

// raise SysTick exception of this CPU core
#if !RISCV
INLINE void NVIC_SysTickForce(void) { *SCB_ICSR |= B26; }
#endif

// check if SysTick exception of this CPU core is pending
#if !RISCV
INLINE Bool NVIC_SysTickIsPending(void) { return (*SCB_ICSR & B26) != 0; }
#endif

// clear SysTick pending state of this CPU core
#if !RISCV
INLINE void NVIC_SystickClear(void) { *SCB_ICSR |= B25; }
#endif

// raise NMI exception of this CPU core
#if !RISCV
INLINE void NVIC_NMIForce(void) { *SCB_ICSR |= B31; }
#endif

// system reset (send reset signal)
//void SystemReset(void);

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// Set specified interrupt's priority
INLINE void irq_set_priority(uint num, u8 hardware_priority) { NVIC_IRQPrio(num, hardware_priority); }

// Get specified interrupt's priority
INLINE uint irq_get_priority(uint num) { return (u8)(NVIC_IPR0[num >> 2] >> (8 * (num & 3))); }

// Enable or disable a specific interrupt on the executing core
INLINE void irq_set_enabled(uint num, bool enabled)
	{ if (enabled) NVIC_IRQEnable(num); else NVIC_IRQDisable(num); }

// Determine if a specific interrupt is enabled on the executing core
INLINE bool irq_is_enabled(uint num) { return NVIC_IRQIsEnabled(num); }

// Enable/disable multiple interrupts on the executing core
INLINE void irq_set_mask_enabled(uint32_t mask, bool enabled)
	{ if (enabled) NVIC_IRQEnableMask(mask); else NVIC_IRQDisableMask(mask); }

// Set an exclusive interrupt handler for an interrupt on the executing core.
INLINE void irq_set_exclusive_handler(int num, irq_handler_t handler) { SetHandler(num, handler); }

// Get the exclusive interrupt handler for an interrupt on the executing core.
INLINE irq_handler_t irq_get_exclusive_handler(int num) { return GetHandler(num); }

// Add a shared interrupt handler for an interrupt on the executing core
// - sharing is not supported, set as exclusive handler
INLINE void irq_add_shared_handler(uint num, irq_handler_t handler, u8 order_priority) { SetHandler(num, handler); }

// Remove a specific interrupt handler for the given irq number on the executing core
// - sharing is not supported
INLINE void irq_remove_handler(uint num, irq_handler_t handler) {}

// Determine if the current handler for the given number is shared
// - sharing is not supported
INLINE bool irq_has_shared_handler(uint num) { return false; }

// Get the current IRQ handler for the specified IRQ from the currently installed hardware vector table (VTOR)
INLINE irq_handler_t irq_get_vtable_handler(int num) { return GetHandler(num); }

// Clear a specific interrupt on the executing core
INLINE void irq_clear(uint int_num) { NVIC_IRQClear(int_num); }

// Force an interrupt to be pending on the executing core
INLINE void irq_set_pending(uint num) { NVIC_IRQForce(num); }

// Perform IRQ priority initialization for the current core
INLINE void irq_init_priorities(void) { NVIC_IRQPrioDef(); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_IRQ_H

#endif // USE_IRQ	// use IRQ interrupts (sdk_irq.c, sdk_irq.h)
