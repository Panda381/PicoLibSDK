
// ****************************************************************************
//
//  Reduced functions of the PicoLibSDK library if the original Raspberry SDK library PicoSDK is used
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico/Pico2 and RP2040/RP2350
// Copyright (c) 2023-2025 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

// Not supported:
//  DISPHSTX_CHECK_LOAD ... not implemented function TimeSysClk()
//  RISC-V ... because of IRQ service very incompatible with PicoLobSDK

//#define RISCV	1	// 1=use RISC-V CPU ... This mode will probably not work with the original SDK



// =======================================================================================
//
//  The content of this file is only a prepared outline, not yet functional - do not use!
//
// =======================================================================================



#if USE_DISPHSTX && DISPHSTX_PICOSDK	// 1=use HSTX Display driver

#ifndef _DISPHSTX_PICOLIBSDK_H
#define _DISPHSTX_PICOLIBSDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/vreg.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"

#include "hardware/structs/hstx_ctrl.h"
#include "hardware/structs/hstx_fifo.h"
#include "hardware/structs/resets.h"
#include "hardware/structs/qmi.h"

#include <stdlib.h>
#include <string.h>

// ========================================================
//                          Misc
// ========================================================

// base types
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;		// on 8-bit system use "signed int"
typedef unsigned short u16;		// on 8-bit system use "unsigned int"
typedef signed long int s32;		// on 64-bit system use "signed int"
typedef unsigned long int u32;		// on 64-bit system use "unsigned int"
typedef signed long long int s64;
typedef unsigned long long int u64;

// bit position
#define	B0 (1<<0)
#define	B1 (1<<1)
#define	B2 (1<<2)
#define	B3 (1<<3)
#define	B4 (1<<4)
#define	B5 (1<<5)
#define	B6 (1<<6)
#define	B7 (1<<7)
#define	B8 (1U<<8)
#define	B9 (1U<<9)
#define	B10 (1U<<10)
#define	B11 (1U<<11)
#define	B12 (1U<<12)
#define	B13 (1U<<13)
#define	B14 (1U<<14)
#define	B15 (1U<<15)
#define B16 (1UL<<16)
#define B17 (1UL<<17)
#define B18 (1UL<<18)
#define	B19 (1UL<<19)
#define B20 (1UL<<20)
#define B21 (1UL<<21)
#define B22 (1UL<<22)
#define B23 (1UL<<23)
#define B24 (1UL<<24)
#define B25 (1UL<<25)
#define B26 (1UL<<26)
#define B27 (1UL<<27)
#define B28 (1UL<<28)
#define B29 (1UL<<29)
#define B30 (1UL<<30)
#define B31 (1UL<<31)

#define BIT(pos) (1UL<<(pos))

// optimised Bool
typedef unsigned char Bool;
#define True 1
#define False 0

// NULL
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

// request to use inline
#define INLINE __attribute__((always_inline)) inline static

// avoid to use inline
#define NOINLINE __attribute__((noinline))

// place time critical function into RAM
#define NOFLASH(fnc) NOINLINE __attribute__((section(".time_critical." #fnc))) fnc

// fast function optimization
#define FASTCODE __attribute__ ((optimize("-Ofast")))

// align array to 4-bytes
#define ALIGNED __attribute__((aligned(4)))

// compile-time check
#ifdef __cplusplus
#define STATIC_ASSERT(c, msg) static_assert((c), msg)
#else
#define STATIC_ASSERT(c, msg) _Static_assert((c), msg)
#endif

// IO u32
typedef volatile u32 io32;

// Set bits of IRQ array
#define RISCV_IRQARRAY_SET(csr, index, data) (riscv_set_csr(csr, (index) | ((u32)(data) << 16)))

// Clear bits of IRQ array
#define RISCV_IRQARRAY_CLR(csr, index, data) (riscv_clear_csr(csr, (index) | ((u32)(data) << 16)))

#ifndef PLL_KHZ
#if RP2040
#define PLL_KHZ		125000		// PLL system frequency in kHz (default 125000 kHz)
#else // RP2350
#define PLL_KHZ		150000		// PLL system frequency in kHz (default 150000 kHz)
#endif
#endif

#if !RISCV
#define USE_GPIOCOPROC  1		// 1=use GPIO coprocessor (lib_gpiocoproc*.c, lib_gpiocoproc*.h)
#endif

#define FONT			FontBold8x8	// default system font
#define FONTW			8		// width of system font
#define FONTH			8		// height of system font

// wait for delay in [us] (max. 71 minutes, 4'294'967'295 us)
INLINE void WaitUs(u32 us) { busy_wait_us(us); }

// wait for delay in [ms] (max. 71 minutes, 4'294'967 ms)
INLINE void WaitMs(int ms) { busy_wait_ms(ms); }

// ========================================================
//                           CPU
// ========================================================

// compiler barrier
INLINE void cb(void) { __asm volatile ("" ::: "memory"); }

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


// instruction synchronization barrier
INLINE void isb(void)
{
#if RISCV
	__asm volatile (" fence.i\n" ::: "memory");
#else // ARM
	__asm volatile (" isb\n" ::: "memory");
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

// core 1 exec function (can exit to stop core 1)
typedef void (*pCore1Fnc)();

// start core 1 function (must be called from core 0)
//  entry ... entry into Core1 function (function can exit to stop core 1)
// Core1IsRunning ... flag that core 1 is running
// Both cores use the same VTOR interrupt table (handlers are shared) but have separate NVIC interrupt controllers.
INLINE void Core1Exec(pCore1Fnc entry) { multicore_launch_core1(entry); }

// reset CPU core 1 (core 1 will send '0' to the FIFO when done)
INLINE void Core1Reset(void) { multicore_reset_core1(); }

// ========================================================
//                         Reset
// ========================================================

#define RESET_PLL_USB	15
#define RESET_PLL_SYS	14

// reset hardware registers
#define RESETS_RESET	((volatile u32*)(RESETS_BASE+0)) // 1=peripheral is in reset
#define RESETS_WDSEL	((volatile u32*)(RESETS_BASE+4)) // 1=watchdog will reset this peripheral
#define RESETS_DONE	((volatile u32*)(RESETS_BASE+8)) // 1=peripheral's registers are ready to access (read only)

// start resetting periphery specified by the mask RESET_*
INLINE void ResetPeripheryOnMask(u32 mask) { hw_set_bits(RESETS_RESET, mask); }
INLINE void ResetPeripheryOn(int peri) { ResetPeripheryOnMask(BIT(peri)); }

// stop resetting periphery specified by the mask RESET_*
INLINE void ResetPeripheryOffMask(u32 mask) { hw_clear_bits(RESETS_RESET, mask); }
INLINE void ResetPeripheryOff(int peri) { ResetPeripheryOffMask(BIT(peri)); }

// check if periphery is ready to be accessed after reset
INLINE Bool ResetPeripheryDoneMask(u32 mask) { return (~*RESETS_DONE & mask) == 0; }
INLINE Bool ResetPeripheryDone(int peri) { return ResetPeripheryDoneMask(BIT(peri)); }

// stop resetting periphery specified by the mask RESET_* and wait
INLINE void ResetPeripheryOffWaitMask(u32 mask)
{
	ResetPeripheryOffMask(mask);  // stop resetting periphery
	while (!ResetPeripheryDoneMask(mask)) {} // wait periphery to be done
}

// hard reset periphery (and wait to be accessed again)
//  Takes 0.5 us.
INLINE void ResetPeripheryMask(u32 mask)
{
	ResetPeripheryOnMask(mask);  // start resetting periphery
	ResetPeripheryOffWaitMask(mask); // stop resetting periphery and wait
}

// hard reset periphery (and wait to be accessed again)
//  Takes 0.5 us.
void ResetPeripheryMask(u32 mask);
INLINE void ResetPeriphery(int peri) { ResetPeripheryMask(BIT(peri)); }

// ========================================================
//                         GPIO
// ========================================================

// flip output to pin (pin = 0..29 or 0..47,56..63)
INLINE void GPIO_Flip(int pin) { gpio_xor_mask(BIT(pin)); }

// number of GPIO pin registers
#define GPIO_PIN_NUM		48

// GPIO pin functions
#define GPIO_FNC_HSTX	0	// HSTX (only pins 12..19)
#define GPIO_FNC_SIO	5	// SIO (GPIO digital function)

typedef u64 gpio_mask_t;		// 64-bit pin mask

// pin control hardware registers
#define GPIO_CTRL(pin) ((volatile u32*)(IO_BANK0_BASE+(pin)*8+4)) // GPIO pin control, pin = 0..29 or 0..47

// get pointer to pad control interface (pin = 0..31 or 0..49)
INLINE io32* GPIO_PadHw(int pin) { return &pads_bank0_hw->io[pin]; }

// GPIO output enable; disable has priority over GPIO_DirOut (default state; pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_OutEnable_hw(io32* hw) { hw_clear_bits(hw, B7); }

// enable pin input (default state on RP2040; pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_InEnable_hw(io32* hw) { hw_set_bits(hw, B6); }

// Disable isolation latches (pin = 0..31 or 0..49, including SWCLK and SWD)
//  If pad isolation is disable, all output settings are transparent to the pad.
INLINE void GPIO_IsolationDisable_hw(io32* hw) { hw_clear_bits(hw, B8); }

// set GPIO function GPIO_FNC_* (pin = 0..29 or 0..47)
// Resets pad overrides, enables input and output.
// To setup GPIO:
//  - reset GPIO to default state with GPIO_Reset()
//  - setup output level, output enable, pull-up/pull-down, schmitt, slew, drive strength
//  - set GPIO function
//  - if needed, disable input (ADC) and set overrides
void GPIO_Fnc(int pin, int fnc);

// set GPIO function GPIO_FNC_* with mask (bit '1' to set function of this pin)
// To use pin mask in range (first..last), use function PinRangeMask.
void GPIO_FncMask(gpio_mask_t mask, int fnc);

// range mask - returns bits set to '1' on range 'first' to 'last' (RangeMask(7,14) returns 0x7F80)
INLINE u32 RangeMask(int first, int last) { return (~(((u32)-1) << (last+1))) & (((u32)-1) << first); }

// pin range mask
INLINE u32 PinRangeMask(int first, int last) { return RangeMask(first, last); } // mask of range of pins

// reset GPIO pin (return to reset state) (pin = 0..29 or 0..47,56..63)
INLINE void GPIO_Reset(int pin) { gpio_deinit(pin); }

// reset GPIO pins masked (return to reset state)
// To use pin mask in range (first..last), use function PinRangeMask.
void GPIO_ResetMask(gpio_mask_t mask);

#define GPIO_PAD(pin) ((volatile u32*)(PADS_BANK0_BASE+(pin)*4+4)) // GPIO pad control register, pin = 0..31 or 0..49 (including SWCLK and SWD)

// set pull down (default state; pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_PullDown(int pin) { hw_write_masked(GPIO_PAD(pin), B2, B2|B3); }

// set pull up (pin = 0..31 or 0..49, including SWCLK and SWD; default state for SWCLK and SWD pins)
INLINE void GPIO_PullUp(int pin) { hw_write_masked(GPIO_PAD(pin), B3, B2|B3); }

// set no pulls (pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_NoPull(int pin) { hw_clear_bits(GPIO_PAD(pin), B2|B3); }

// set output strength to 8 mA (pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_Drive8mA(int pin) { hw_write_masked(GPIO_PAD(pin), B5, B4|B5); }

// Clear GPIO 64-bit output enable; operation: sio_hw->gpio_oe_clr = mask.L; sio_hw->gpio_hi_oe_clr = mask.H;
INLINE void GPIOC_OeClrAll(u64 mask) { __asm volatile (" mcrr p0, #3, %0, %1, c4\n" : : "r" (mask & 0xffffffff), "r" (mask >> 32)); }

// Clear GPIO 64-bit output; operation: sio_hw->gpio_clr = mask.L; sio_hw->gpio_hi_clr = mask.H;
INLINE void GPIOC_OutClrAll(u64 mask) { __asm volatile (" mcrr p0, #3, %0, %1, c0\n" : : "r" (mask & 0xffffffff), "r" (mask >> 32)); }

// Get GPIO input 64-bit; operation: return sio_hw->gpio_in | (sio_hw->gpio_hi_in << 32);
INLINE u64 GPIOC_InAll(void)
{
	u32 resH, resL;
	__asm volatile (" mrrc p0, #0, %0, %1, c8\n" : "=r" (resL), "=r" (resH));
	return ((u64)resH << 32) | resL;
}

// initialize GPIO pin base function masked (bit '1' = initialize this pin)
// To use pin mask in range (first..last), use function PinRangeMask.
void GPIO_InitMask(gpio_mask_t mask);
// set input direction of pins masked
// To use pin mask in range (first..last), use function PinRangeMask.
INLINE void GPIO_DirInMask(gpio_mask_t mask)
{
#if RP2040
	sio_hw->gpio_oe_clr = mask;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OeClrAll(mask);
#else
	sio_hw->gpio_oe_clr = (u32)mask;
	sio_hw->gpio_hi_oe_clr = (u32)(mask >> 32);
#endif
}

// clear output pins masked
// To use pin mask in range (first..last), use function PinRangeMask.
INLINE void GPIO_ClrMask(gpio_mask_t mask)
{
#if RP2040
	sio_hw->gpio_clr = mask;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OutClrAll(mask);
#else
	sio_hw->gpio_clr = (u32)mask;
	sio_hw->gpio_hi_clr = (u32)(mask >> 32);
#endif
}

// get all input pins
INLINE gpio_mask_t GPIO_InAll()
{
#if RP2040
	return sio_hw->gpio_in;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	return GPIOC_InAll();
#else
	return sio_hw->gpio_in | ((u64)sio_hw->gpio_hi_in << 32);
#endif
}

// ========================================================
//                       IRQ
// ========================================================

#define IRQ_DMA_1		11	// DMA IRQ 1

#define SCB_VTOR	((volatile u32*)(PPB_BASE + 0xED08))	// Vector Table Offset Register

#define IRQ_PRIO_REALTIME	0	// highest priority - real-time time-critical interrupts

// get address of interrupt vector table of this CPU core
#if RISCV
INLINE irq_handler_t* GetVTOR(void) { return (irq_handler_t*)(_riscv_read_csr(0x305) & ~0x3u); } // RVCSR_MTVEC_OFFSET (clear MODE bits 0 and 1)
#else
INLINE irq_handler_t* GetVTOR(void) { return (irq_handler_t*)*SCB_VTOR; }
#endif

// set interrupt service handler (irq = interrupt request indice IRQ_*, negative exception numbers can also be used)
// - vector table must be located in RAM
// - vector table must be aligned to 256 bytes (RISC-V: 64 bytes)
// - if vector table is not in RAM, use services with names isr_*, it overrides the weak handler
// - vector table in this SDK is shared between both CPU cores
// - can be used to set exception handlers (irq < 0)
INLINE void SetHandler(int irq, irq_handler_t handler) { cb(); GetVTOR()[VTABLE_FIRST_IRQ + irq] = handler; dmb(); }

// clear pending interrupt of NVIC of this CPU core (irq = 0..25 or 0..51)
INLINE void NVIC_IRQClear(int irq)
{
#if RISCV
	RISCV_IRQARRAY_CLR(RVCSR_MEIFA_OFFSET, irq >> 4, BIT(irq & 0x0f));
#else
	nvic_hw->icpr[irq >> 5] = BIT(irq & 0x1f);
#endif
}

// enable interrupt of NVIC of this CPU core (irq = 0..25 or 0..51)
INLINE void NVIC_IRQEnable(int irq)
{
	// clear pending interrupt
	NVIC_IRQClear(irq);

	cb();
#if RISCV
	RISCV_IRQARRAY_SET(RVCSR_MEIEA_OFFSET, irq >> 4, BIT(irq & 0x0f));
#else
	nvic_hw->iser[irq >> 5] = BIT(irq & 0x1f);
#endif
	cb();
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

INLINE void NVIC_IRQPrio(int irq, u8 prio) {}

// ========================================================
//                          DMA
// ========================================================

// DMA requests
#define DREQ_FORCE	0x3F	// 63, DMA permanent request

// DMA transfer size
#define DMA_SIZE_8	0	// 1 byte (8 bits)
#define DMA_SIZE_16	1	// 2 bytes (16 bits)
#define DMA_SIZE_32	2	// 4 bytes (32 bits)

#define DMA_CTRL_BUSY		B26	// 1 = DMA transfer is busy, transfer was triggered but not finished yet (read only)
#define DMA_CTRL_SNIFF		B25	// 1 = transfer is visible to sniff CRC hardware, 0 = normal DMA transfer
#define DMA_CTRL_BSWAP		B24	// 1 = swap order of bytes
#define DMA_CTRL_QUIET		B23	// 1 = generate IRQ when NULL is written to a trigger, 0 = generate IRQ at end of every transfer block
#define DMA_CTRL_TREQ(dreq)	((u32)(dreq)<<17) // 0..0x3f select transfer request signal 0..0x3a = DREQ_*, 0x3b..0x3e = Timer0..Timer3, 0x3f = permanent request
#define DMA_CTRL_TREQ_FORCE	DMA_CTRL_TREQ(DREQ_FORCE) // permanent request
#define DMA_CTRL_CHAIN(chan)	((chan)<<13) // trigger channel 0..15 when completed, disable by setting to this channel (=default state)
#define DMA_CTRL_RING_WRITE	B12	//  1 = write addresses are wrapped, 0 = read addresses are wrapped
#define DMA_CTRL_RING_SIZE(order) ((order)<<8) // size 1..15 of address wrap region in bytes '1 << order' (2..32768 bytes), 0 = don't wrap
					//     Ring buffer must be aligned to wrap size (only lowest 'order' bits are changed)
#define DMA_CTRL_INC_WRITE_REV	B7	// write address is 1 = decremented, 0 = incremented
#define DMA_CTRL_INC_WRITE	B6	// 1 = increment/decrement write address with each transfer (memory), 0 = do not change (port)
#define DMA_CTRL_INC_READ_REV	B5	// read address is 1 = decremented, 0 = incremented
#define DMA_CTRL_INC_READ	B4	// 1 = increment read address with each transfer (memory), 0 = do not increment (port)
#define DMA_CTRL_SIZE(size)	((size)<<2) // data transfer size DMA_SIZE_* (0=8 bits, 1=16 bits, 2=32 bits)
#define DMA_CTRL_HIGH_PRIORITY	B1	// 1 = channel is preferred during DMA scheduling round, 0 = normal priority
#define DMA_CTRL_EN		B0	// 1 = channel is enabled and responds to trigger, 0 = channel is paused (BUSY will remain unchanged)

// set DMA config, without trigger
//  dma = channel 0..11 (or 0..15)
//  src = source address
//  dst = destination address
//  count = number of transfers (can be combined with flags DMA_COUNT_TRIGGER or DMA_COUNT_ENDLESS)
//  ctrl = control word (can be prepared using DMA_Cfg* functions)
void DMA_Config(int dma, const volatile void* src, volatile void* dst, u32 count, u32 ctrl);

// set DMA read address, without trigger
INLINE void DMA_SetRead(int dma, const volatile void* addr) { dma_hw->ch[dma].read_addr = (u32)addr; }

// set DMA read address, with trigger
INLINE void DMA_SetReadTrig(int dma, const volatile void* addr) { cb(); dma_hw->ch[dma].al3_read_addr_trig = (u32)addr; }

// set DMA write address, without trigger
INLINE void DMA_SetWrite(int dma, volatile void* addr) { dma_hw->ch[dma].write_addr = (u32)addr; }

// set DMA transfer count, without trigger
//  Count can be max. 0x0fffffff (268'435'455).
//  Count can be combined with flags DMA_COUNT_TRIGGER or DMA_COUNT_ENDLESS.
//  Count is number of transfers, not number of bytes.
INLINE void DMA_SetCount(int dma, u32 count) { dma_hw->ch[dma].transfer_count = count; }

// set DMA control register, without trigger
INLINE void DMA_SetCtrl(int dma, u32 ctrl) { dma_hw->ch[dma].al1_ctrl = ctrl; }

// clear DMA interrupt request IRQ_DMA_0..IRQ_DMA_3
INLINE void DMA_IRQ1Clear(int dma) { dma_hw->ints1 = BIT(dma); cb(); }

// enable interrupt from DMA channels for IRQ_DMA_0..IRQ_DMA_3 masked (set bits 0..15 to enable channel 0..15)
// To use DMA mask in range (first..last), use function RangeMask.
INLINE void DMA_IRQ1EnableMask(u32 mask) { hw_set_bits(&dma_hw->inte1, mask); }

// disable interrupt from DMA channel for IRQ_DMA_0..IRQ_DMA_3
INLINE void DMA_IRQ1DisableMask(u32 mask) { hw_clear_bits(&dma_hw->inte1, mask); }
INLINE void DMA_IRQ1Disable(int dma) { DMA_IRQ1DisableMask(BIT(dma)); }

// enable interrupt from DMA channel for IRQ_DMA_0..IRQ_DMA_3

INLINE void DMA_IRQ1Enable(int dma) { DMA_IRQ1EnableMask(BIT(dma)); }

// disable DMA channel (pause - transfer will be paused but not aborted, busy will stay active)
INLINE void DMA_Disable(int dma) { hw_clear_bits(&dma_hw->ch[dma].al1_ctrl, DMA_CTRL_EN); }

// abort DMA transfer
INLINE void DMA_Abort(int dma) { dma_channel_abort(dma); }

// ========================================================
//                         HSTX
// ========================================================

// HSTX commands (4 bits << 12)
#define HSTX_CMD_RAW		(0x0u << 12)	// write raw data
#define HSTX_CMD_RAW_REPEAT	(0x1u << 12)	// write raw data repeated
#define HSTX_CMD_TMDS		(0x2u << 12)	// write TMDS mark
#define HSTX_CMD_TMDS_REPEAT	(0x3u << 12)	// write TMDS mark repeated
#define HSTX_CMD_NOP		(0xfu << 12)	// no operation

// ========================================================
//                     Power manager
// ========================================================

// Power manager password (need to write to upper 16 bits of registers 0x00 to 0xAC)
#define POWMAN_PASSWORD		0x5afe0000	// = "SAFE" word

// set bits of power manager register with password
INLINE void PowMan_SetBits(volatile u32* reg, u32 val) { hw_set_bits(reg, POWMAN_PASSWORD | val); }

// clear bits of power manager register with password
INLINE void PowMan_ClrBits(volatile u32* reg, u32 val) { hw_clear_bits(reg, POWMAN_PASSWORD | val); }

// write 16-bit value to power manager register with password
INLINE void PowMan_Write(volatile u32* reg, u32 val) { *reg = POWMAN_PASSWORD | val; }

// Possible voltage values
#define VREG_VOLTAGE_0_55	0	// 0.55V
#define VREG_VOLTAGE_0_60	1	// 0.60V
#define VREG_VOLTAGE_0_65	2	// 0.65V
#define VREG_VOLTAGE_0_70	3	// 0.70V
#define VREG_VOLTAGE_0_75	4	// 0.75V
#define VREG_VOLTAGE_0_80	5	// 0.80V
#define VREG_VOLTAGE_0_85	6	// 0.85V
#define VREG_VOLTAGE_0_90	7	// 0.90V
#define VREG_VOLTAGE_0_95	8	// 0.95V
#define VREG_VOLTAGE_1_00	9	// 1.00V
#define VREG_VOLTAGE_1_05	10	// 1.05V
#define VREG_VOLTAGE_1_10	11	// 1.10V *default state
#define VREG_VOLTAGE_1_15	12	// 1.15V
#define VREG_VOLTAGE_1_20	13	// 1.20V
#define VREG_VOLTAGE_1_25	14	// 1.25V
#define VREG_VOLTAGE_1_30	15	// 1.30V

#define VREG_VOLTAGE_UNLOCK	B8	// OR required value with this flag, to enable unlock higher voltages than 1.30V

// get recommended voltage by system clock in kHz (return VREG_VOLTAGE_1_10 .. VREG_VOLTAGE_1_30)
int GetVoltageBySysClock(u32 freq);

// check if voltage is correctly regulated
INLINE Bool VregIsOk(void) { return (powman_hw->vreg & B15) == 0; } // UPDATE_IN_PROGRESS

// wait for regulated state
void VregWait(void);

// set voltage VREG_VOLTAGE_*
void VregSetVoltage(int vreg);

// ========================================================
//                         Clock
// ========================================================

// get recommended flash divider by system clock in kHz
int GetClkDivBySysClock(u32 freq);

// PLL ranges
#define PLL_VCO_MIN	400		// VCO min. frequency in MHz
#define PLL_VCO_MAX	1600		// VCO max. frequency in MHz
#define PLL_FBDIV_MIN	16		// min. feedback divisor
#define PLL_FBDIV_MAX	320		// max. feedback divisor
#define PLL_DIV_MIN	1		// min. post divider
#define PLL_DIV_MAX	7		// max. post divider

#define PLL_SYS		0	// system PLL (generate system clock, default 125 MHz, max. 133 MHz)
#define PLL_USB		1	// USB PLL (generate 48 MHz USB reference clock)

#define PLL(pll)	(PLL_SYS_BASE + (pll)*(PLL_USB_BASE - PLL_SYS_BASE))	// PLL base

// get hardware interface from the PLL index
INLINE pll_hw_t* PllGetHw(int pll) { return (pll_hw_t*)PLL(pll); }

// Search PLL setup
//  reqkhz ... required output frequency in kHz
//  input ... PLL input reference frequency in kHz (default 12000)
//  vcomin ... minimal VCO frequency in kHz (default 400000)
//  vcomax ... maximal VCO frequency in kHz (default 1600000)
//  lowvco ... prefer low VCO (lower power but more jiter)
// outputs:
//  outkhz ... output achieved frequency in kHz (0=not found)
//  outvco ... output VCO frequency in kHz
//  outfbdiv ... output fbdiv (16..320)
//  outpd1 ... output postdiv1 (1..7)
//  outpd2 ... output postdiv2 (1..7)
// Returns True if precise frequency has been found, or near frequency used otherwise.
Bool PllCalc(u32 reqkhz, u32 input, u32 vcomin, u32 vcomax, Bool lowvco,
		u32* outkhz, u32* outvco, u16* outfbdiv, u8* outpd1, u8* outpd2);

// PLL setup (returns result frequency in Hz)
//   pll ... PLL select, PLL_SYS or PLL_USB
//   refdiv ... divide input reference clock, 1..63 (minimum reference frequency XOSC/refdiv is 5 MHz)
//   fbdiv ... feedback divisor, 16..320
//   div1 ... post divider 1, 1..7
//   div2 ... post divider 2, 1..7 (should be div1 >= div2, but auto-corrected)
// All clocks should be disconnected from the PLL.
// Result frequency = (XOSC / refdiv) * fbdiv / (div1 * div2).
// Do not call this function directly, if SYS clock is connected, call the ClockPllSysSetup function.
u32 PllSetup(int pll, int refdiv, int fbdiv, int div1, int div2);

// setup PLL frequency in kHz (returns result frequency in kHz, or 0 if cannot setup)
// Do not call this function directly, if SYS clock is connected, call the ClockPllSysFreq function.
u32 PllSetFreq(int pll, u32 freq);

// set system clock PLL to new frequency in kHz and auto-set system voltage and flash divider (dependent clocks are not updated)
//   freq ... required frequency in [kHz]
void ClockPllSysFreqVolt(u32 freq);

// initialize Flash interface (clkdiv = clock divider, must be even number, FLASHQSPI_CLKDIV_DEF=2 is default) (must be run from RAM)
//   Supported devices: Winbond W25Q080, W25Q16JV, AT25SF081, S25FL132K0
//   Raspberry Pico cointains W25Q16JVUXIQ, Raspberry Pico 2 cointains W25Q32RVXHJQ
INLINE void FlashInit(int clkdiv) { qmi_hw->m[0].timing = clkdiv | (2 << 8) | (1 << 30); }

INLINE u32 Clz(u32 val)
{
	u32 res;
	__asm volatile (" clz %0,%1\n" : "=r" (res) : "r" (val));
	return res;
}

// get bit order of value (logarithm, returns position of highest bit + 1: 1..32, 0=no bit)
INLINE u32 Order(u32 val) { return 32 - Clz(val); }

// get mask of value (0x123 returns 0x1FF)
INLINE u32 Mask(u32 val) { return ((u32)-1) >> Clz(val); }

#ifdef __cplusplus
}
#endif

#endif // _DISPHSTX_PICOLIBSDK_H

#endif // USE_DISPHSTX && DISPHSTX_PICOSDK
