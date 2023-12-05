
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

#if USE_ORIGSDK		// include interface of original SDK
#include "orig/orig_m0plus.h"		// constants of original SDK
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// ARM Cortex-M0+ core interrupt request indices - Exceptions
#define IRQ_INVALID	-16	// invalid exception - returned 
#define IRQ_RESET	-15
#define IRQ_NMI		-14
#define IRQ_HARDFAULT	-13
#define IRQ_SVCALL	-5
#define IRQ_PENDSV	-2
#define IRQ_SYSTICK	-1
// RP2040 device interrupt request indices - Interrupts
#define IRQ_TIMER_0	0
#define IRQ_TIMER_1	1
#define IRQ_TIMER_2	2
#define IRQ_TIMER_3	3
#define IRQ_PWM_WRAP	4
#define IRQ_USBCTRL	5
#define IRQ_XIP		6
#define IRQ_PIO0_0	7
#define IRQ_PIO0_1	8
#define IRQ_PIO1_0	9
#define IRQ_PIO1_1	10
#define IRQ_DMA_0	11
#define IRQ_DMA_1	12
#define IRQ_IO_BANK0	13
#define IRQ_IO_QSPI	14
#define IRQ_SIO_PROC0	15
#define IRQ_SIO_PROC1	16
#define IRQ_CLOCKS	17
#define IRQ_SPI0	18
#define IRQ_SPI1	19
#define IRQ_UART0	20
#define IRQ_UART1	21
#define IRQ_ADC_FIFO	22
#define IRQ_I2C0	23
#define IRQ_I2C1	24
#define IRQ_RTC		25

#define IRQ_NUM		32		// number of IRQs

// original-SDK name convention
#if USE_ORIGSDK		// include interface of original SDK
#define TIMER_IRQ_0	0
#define TIMER_IRQ_1	1
#define TIMER_IRQ_2	2
#define TIMER_IRQ_3	3
#define PWM_IRQ_WRAP	4
#define USBCTRL_IRQ	5
#define XIP_IRQ		6
#define PIO0_IRQ_0	7
#define PIO0_IRQ_1	8
#define PIO1_IRQ_0	9
#define PIO1_IRQ_1	10
#define DMA_IRQ_0	11
#define DMA_IRQ_1	12
#define IO_IRQ_BANK0	13
#define IO_IRQ_QSPI	14
#define SIO_IRQ_PROC0	15
#define SIO_IRQ_PROC1	16
#define CLOCKS_IRQ	17
#define SPI0_IRQ	18
#define SPI1_IRQ	19
#define UART0_IRQ	20
#define UART1_IRQ	21
#define ADC_IRQ_FIFO	22
#define I2C0_IRQ	23
#define I2C1_IRQ	24
#define RTC_IRQ		25
#endif // USE_ORIGSDK

// symbolic names of interrupt services
//  isr_nmi
//  isr_hardfault
//  isr_svcall
//  isr_pendsv
//  isr_systick
#define isr_timer_0	isr_irq0
#define isr_timer_1	isr_irq1
#define isr_timer_2	isr_irq2
#define isr_timer_3	isr_irq3
#define isr_pwm_wrap	isr_irq4
#define isr_usbctrl	isr_irq5
#define isr_xip		isr_irq6
#define isr_pio0_0	isr_irq7
#define isr_pio0_1	isr_irq8
#define isr_pio1_0	isr_irq9
#define isr_pio1_1	isr_irq10
#define isr_dma_0	isr_irq11
#define isr_dma_1	isr_irq12
#define isr_io_bank0	isr_irq13
#define isr_io_qspi	isr_irq14
#define isr_sio_proc0	isr_irq15
#define isr_sio_proc1	isr_irq16
#define isr_clocks	isr_irq17
#define isr_spi0	isr_irq18
#define isr_spi1	isr_irq19
#define isr_uart0	isr_irq20
#define isr_uart1	isr_irq21
#define isr_adc_fifo	isr_irq22
#define isr_i2c0	isr_irq23
#define isr_i2c1	isr_irq24
#define isr_rtc		isr_irq25
// isr_irq26..isr_irq31

// interrupt hardware priority
#define IRQ_PRIO_REALTIME	0	// highest priority - real-time time-critical interrupts
#define IRQ_PRIO_SYSTICK	1	// high priority - SysTick (must have higher priority than normal interrupts)
#define IRQ_PRIO_NORMAL		2	// default normal priority - normal code, normal interrupts
#define IRQ_PRIO_PENDSV		3	// lowest priority - PendSV handler (must have lowest priority!)

// common interrupt handler definition
typedef void (*irq_handler_t)(void);

// === System control block (RP2040 datasheet page 74)
//#define PPB_BASE		0xe0000000	// Cortex-M0+ internal registers (system control block)

// NVIC hardware registers
#define NVIC_ISER ((volatile u32*)(PPB_BASE + 0xE100))	// address of interrupt set-enable register
#define NVIC_ICER ((volatile u32*)(PPB_BASE + 0xE180))	// address of interrupt clear-enable register
#define NVIC_ISPR ((volatile u32*)(PPB_BASE + 0xE200))	// address of interrupt set-pending register
#define NVIC_ICPR ((volatile u32*)(PPB_BASE + 0xE280))	// address of interrupt clear-pending register
#define NVIC_IPR0 ((volatile u32*)(PPB_BASE + 0xE400))	// address of interrupt priority registers, array of 8*u32 (each entry is 8 bits big, but only top 2 bits are used)

// NVIC hardware interface
typedef struct {
	io32	iser;		// 0x000: Use the Interrupt Set-Enable Register to enable interrupts and determine which interrupts are currently enabled
	io32	_pad0[31];
	io32	icer;		// 0x080: Use the Interrupt Clear-Enable Registers to disable interrupts and determine which interrupts are currently enabled
	io32	_pad1[31];
	io32	ispr;		// 0x100: The NVIC_ISPR forces interrupts into the pending state, and shows which interrupts are pending
	io32	_pad2[31];
	io32	icpr;		// 0x180: Use the Interrupt Clear-Pending Register to clear pending interrupts and determine which interrupts are currently pending
	io32	_pad3[95];
	io32	ipr[8];		// 0x300: Use the Interrupt Priority Registers to assign a priority from 0 to 3 to each of the available interrupts
} nvic_hw_t;

#define nvic_hw ((nvic_hw_t *)(PPB_BASE + 0xE100))

STATIC_ASSERT(sizeof(nvic_hw_t) == 0x320, "Incorrect nvic_hw_t!");

// === System control block of the Cortex M0+ CPU core

// IRQ hardware registers
#define SCB_ICSR ((volatile u32*)(PPB_BASE + 0xED04))	// address of Cortex M0+ interrupt control state register (bit 31: 1=NMI pending, bit 28: 1=PendSV pending,
							//   bit 27: 1=clear pending PendSV, bit 26: 1=SysTick pending, bit 25: 1=clear pending SysTick)
#define SCB_VTOR ((volatile u32*)(PPB_BASE + 0xED08))	// address of register of Cortex M0+ interrupt vector table VTOR (vector table must be aligned to 256 bytes)
#define SCB_AIRCR ((volatile u32*)(PPB_BASE + 0xED0C))	// address of reset control register of Cortex M0+
#define SCB_SHPR2 ((volatile u32*)(PPB_BASE + 0xED1C))	// address of system handler priority 2 of Cortex M0+ (bit 30..31 = priority of SVCall)
#define SCB_SHPR3 ((volatile u32*)(PPB_BASE + 0xED20))	// address of system handler priority 3 of Cortex M0+ (bit 30..31: priority of SysTick, bit 22..23: priority of PendSV)

// IRQ hardware interface
typedef struct {
	io32	cpuid;		// 0x00: Read the CPU ID Base Register to determine: the ID number of the processor core, the version number of the processor...
	io32	icsr;		// 0x04: Use the Interrupt Control State Register to set a pending Non-Maskable Interrupt (NMI), set or clear a pending...
	io32	vtor;		// 0x08: The VTOR holds the vector table offset address
	io32	aircr;		// 0x0C: Use the Application Interrupt and Reset Control Register to: determine data endianness, clear all active state...
	io32	scr;		// 0x10: System Control Register
} armv6m_scb_t;

#define scb_hw ((armv6m_scb_t*)(PPB_BASE + 0xED00))

STATIC_ASSERT(sizeof(armv6m_scb_t) == 0x14, "Incorrect armv6m_scb_t!");

// copy of vector table in RAM
#if !NO_FLASH	// In the RAM version, the vector table is already in RAM
#if USE_IRQ	// use IRQ interrupts (sdk_irq.c, sdk_irq.h)
// copy of vector table in RAM ... must be aligned to 256 bytes
extern u32 __attribute__((section(".ram_vector_table"))) __attribute__((aligned(256))) RamVectorTable[48];
#endif
#endif

// get address of interrupt vector table of this CPU core
INLINE irq_handler_t* GetVTOR(void) { return (irq_handler_t*)*SCB_VTOR; }

// set address of interrupt vector table of this CPU core
INLINE void SetVTOR(irq_handler_t* addr) { cb(); *SCB_VTOR = (u32)addr; cb(); }

// set interrupt service handler (irq = interrupt request indice IRQ_* -15..+25)
// - vector table must be located in RAM
// - vector table must be aligned to 256 bytes
// - if vector table is not in RAM, use services with names isr_*
// - vector table in this SDK is shared between both CPU cores
// - can be used to set Cortex-M0+ exception handlers (irq < 0)
INLINE void SetHandler(int irq, irq_handler_t handler) { cb(); GetVTOR()[16 + irq] = handler; dmb(); }

// get current interrupt servie handler (irq = interrupt request indice IRQ_* -15..+25)
INLINE irq_handler_t GetHandler(int irq) { return GetVTOR()[16 + irq]; }

// enable interrupts of NVIC masked of this CPU core
INLINE void NVIC_IRQEnableMask(u32 mask) { cb(); *NVIC_ISER = mask; cb(); }

// enable interrupt of NVIC of this CPU core (irq = 0..25)
INLINE void NVIC_IRQEnable(int irq) { cb(); NVIC_IRQEnableMask(BIT(irq)); cb(); }

// disable interrupts of NVIC masked of this CPU core
INLINE void NVIC_IRQDisableMask(u32 mask) { cb(); *NVIC_ICER = mask; dsb(); isb(); }

// disable interrupt of NVIC of this CPU core (irq = 0..25)
INLINE void NVIC_IRQDisable(int irq) { NVIC_IRQDisableMask(BIT(irq)); }

// check if interrupt of NVIC of this CPU core is enabled (irq = 0..25)
INLINE Bool NVIC_IRQIsEnabled(int irq) { return (*NVIC_ISER & BIT(irq)) != 0; }

// clear pending interrupt of NVIC of this CPU core (irq = 0..25)
INLINE void NVIC_IRQClear(int irq) { *NVIC_ICPR = BIT(irq); }

// set pending interrupt of NVIC of this CPU core (force interrupt) (irq = 0..25)
INLINE void NVIC_IRQForce(int irq) { *NVIC_ISPR = BIT(irq); }

// check if interrupt of NVIC of this CPU core is pending (irq = 0..25)
INLINE Bool NVIC_IRQIsPending(int irq) { return (*NVIC_ISPR & BIT(irq)) != 0; }

// set interrupt priority of NVIC of this CPU core (prio = priority IRQ_PRIO_*) (irq = 0..25)
void NVIC_IRQPrio(int irq, u8 prio);

// set all interrupt priorities of NVIC of this CPU core to default value
void NVIC_IRQPrioDef(void);

// set interrupt priority of SVCall exception -5 of this CPU core (prio = priority IRQ_PRIO_*)
INLINE void NVIC_SVCallPrio(int prio)
	{ *SCB_SHPR2 = (*SCB_SHPR2 & ~(B30 | B31)) | ((u32)prio << 30); }

// set interrupt priority of PendSV exception -2 of this CPU core (prio = priority IRQ_PRIO_*)
INLINE void NVIC_PendSVPrio(int prio)
	{ *SCB_SHPR3 = (*SCB_SHPR3 & ~(B22 | B23)) | ((u32)prio << 22); }

// raise PendSV exception of this CPU core (reschedule multitask system)
INLINE void NVIC_PendSVForce(void) { *SCB_ICSR |= B28; }

// check if PendSV exception of this CPU core is pending
INLINE Bool NVIC_PendSVIsPending(void) { return (*SCB_ICSR & B28) != 0; }

// clear PendSV pending state of this CPU core
INLINE void NVIC_PendSVClear(void) { *SCB_ICSR |= B27; }

// set interrupt priority of SysTick exception -1 of this CPU core (prio = priority IRQ_PRIO_*)
INLINE void NVIC_SysTickPrio(int prio)
	{ *SCB_SHPR3 = (*SCB_SHPR3 & ~(B30 | B31)) | ((u32)prio << 30); }

// raise SysTick exception of this CPU core
INLINE void NVIC_SysTickForce(void) { *SCB_ICSR |= B26; }

// check if SysTick exception of this CPU core is pending
INLINE Bool NVIC_SysTickIsPending(void) { return (*SCB_ICSR & B26) != 0; }

// clear SysTick pending state of this CPU core
INLINE void NVIC_SystickClear(void) { *SCB_ICSR |= B25; }

// raise NMI exception of this CPU core
INLINE void NVIC_NMIForce(void) { *SCB_ICSR |= B31; }

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
