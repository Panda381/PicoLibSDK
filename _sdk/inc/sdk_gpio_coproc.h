
// ****************************************************************************
//
//                          GPIO coprocessor (only RP2350 ARM)
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

#if USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)

#if RISCV
#error "RISC-V does not support GPIO coprocessor"
#endif

#if RP2040
#error "RP2040 does not support GPIO coprocessor"
#endif

#ifndef _SDK_GPIO_COPROC_H
#define _SDK_GPIO_COPROC_H

#ifdef __cplusplus
extern "C" {
#endif

// === OUT mask write instructions

// Output to GPIO low; operation: sio_hw->gpio_out = x;
INLINE void GPIOC_OutAllL(u32 mask) { __asm volatile (" mcr p0, #0, %0, c0, c0\n" : : "r" (mask)); }

// Toggle GPIO low output; operation: sio_hw->gpio_togl = mask;
INLINE void GPIOC_OutXorAllL(u32 mask) { __asm volatile (" mcr p0, #1, %0, c0, c0\n" : : "r" (mask)); }

// Set GPIO low output; operation: sio_hw->gpio_set = mask;
INLINE void GPIOC_OutSetAllL(u32 mask) { __asm volatile (" mcr p0, #2, %0, c0, c0\n" : : "r" (mask)); }

// Clear GPIO low output; operation: sio_hw->gpio_clr = mask;
INLINE void GPIOC_OutClrAllL(u32 mask) { __asm volatile (" mcr p0, #3, %0, c0, c0\n" : : "r" (mask)); }

// Output to GPIO high; operation: sio_hw->gpio_hi_out = mask;
INLINE void GPIOC_OutAllH(u32 mask) { __asm volatile (" mcr p0, #0, %0, c0, c1\n" : : "r" (mask)); }

// Toggle GPIO high output; operation: sio_hw->gpio_hi_togl = mask;
INLINE void GPIOC_OutXorAllH(u32 mask) { __asm volatile (" mcr p0, #1, %0, c0, c1\n" : : "r" (mask)); }

// Set GPIO high output; operation: sio_hw->gpio_hi_set = mask;
INLINE void GPIOC_OutSetAllH(u32 mask) { __asm volatile (" mcr p0, #2, %0, c0, c1\n" : : "r" (mask)); }

// Clear GPIO high output; operation: sio_hw->gpio_hi_clr = mask;
INLINE void GPIOC_OutClrAllH(u32 mask) { __asm volatile (" mcr p0, #3, %0, c0, c1\n" : : "r" (mask)); }

// Output to GPIO 64-bit; operation: sio_hw->gpio_out = mask.L; sio_hw->gpio_hi_out = mask.H;
INLINE void GPIOC_OutAll(u64 mask) { __asm volatile (" mcrr p0, #0, %0, %1, c0\n" : : "r" (mask & 0xffffffff), "r" (mask >> 32)); }

// Toggle GPIO 64-bit output; operation: sio_hw->gpio_togl = mask.L; sio_hw->gpio_hi_togl = mask.H;
INLINE void GPIOC_OutXorAll(u64 mask) { __asm volatile (" mcrr p0, #1, %0, %1, c0\n" : : "r" (mask & 0xffffffff), "r" (mask >> 32)); }

// Set GPIO 64-bit output; operation: sio_hw->gpio_set = mask.L; sio_hw->gpio_hi_set = mask.H;
INLINE void GPIOC_OutSetAll(u64 mask) { __asm volatile (" mcrr p0, #2, %0, %1, c0\n" : : "r" (mask & 0xffffffff), "r" (mask >> 32)); }

// Clear GPIO 64-bit output; operation: sio_hw->gpio_clr = mask.L; sio_hw->gpio_hi_clr = mask.H;
INLINE void GPIOC_OutClrAll(u64 mask) { __asm volatile (" mcrr p0, #3, %0, %1, c0\n" : : "r" (mask & 0xffffffff), "r" (mask >> 32)); }

// === OE mask write instructions

// Output enable to GPIO low; operation: sio_hw->gpio_oe = mask;
INLINE void GPIOC_OeAllL(u32 mask) { __asm volatile (" mcr p0, #0, %0, c0, c4\n" : : "r" (mask)); }

// Toggle GPIO low output enable; operation: sio_hw->gpio_oe_togl = mask;
INLINE void GPIOC_OeXorAllL(u32 mask) { __asm volatile (" mcr p0, #1, %0, c0, c4\n" : : "r" (mask)); }

// Set GPIO low output enable; operation: sio_hw->gpio_oe_set = mask;
INLINE void GPIOC_OeSetAllL(u32 mask) { __asm volatile (" mcr p0, #2, %0, c0, c4\n" : : "r" (mask)); }

// Clear GPIO low output enable; operation: sio_hw->gpio_oe_clr = mask;
INLINE void GPIOC_OeClrAllL(u32 mask) { __asm volatile (" mcr p0, #3, %0, c0, c4\n" : : "r" (mask)); }

// Output enable to GPIO high; operation: sio_hw->gpio_hi_oe = mask;
INLINE void GPIOC_OeAllH(u32 mask) { __asm volatile (" mcr p0, #0, %0, c0, c5\n" : : "r" (mask)); }

// Toggle GPIO high output enable; operation: sio_hw->gpio_hi_oe_togl = mask;
INLINE void GPIOC_OeXorAllH(u32 mask) { __asm volatile (" mcr p0, #1, %0, c0, c5\n" : : "r" (mask)); }

// Set GPIO high output enable; operation: sio_hw->gpio_hi_oe_set = mask;
INLINE void GPIOC_OeSetAllH(u32 mask) { __asm volatile (" mcr p0, #2, %0, c0, c5\n" : : "r" (mask)); }

// Clear GPIO high output enable; operation: sio_hw->gpio_hi_oe_clr = mask;
INLINE void GPIOC_OeClrAllH(u32 mask) { __asm volatile (" mcr p0, #3, %0, c0, c5\n" : : "r" (mask)); }

// Output enable to GPIO 64-bit; operation: sio_hw->gpio_oe = mask.L; sio_hw->gpio_hi_oe = mask.H;
INLINE void GPIOC_OeAll(u64 mask) { __asm volatile (" mcrr p0, #0, %0, %1, c4\n" : : "r" (mask & 0xffffffff), "r" (mask >> 32)); }

// Toggle GPIO 64-bit output enable; operation: sio_hw->gpio_oe_togl = mask.L; sio_hw->gpio_hi_oe_togl = mask.H;
INLINE void GPIOC_OeXorAll(u64 mask) { __asm volatile (" mcrr p0, #1, %0, %1, c4\n" : : "r" (mask & 0xffffffff), "r" (mask >> 32)); }

// Set GPIO 64-bit output enable; operation: sio_hw->gpio_oe_set = mask.L; sio_hw->gpio_hi_oe_set = mask.H;
INLINE void GPIOC_OeSetAll(u64 mask) { __asm volatile (" mcrr p0, #2, %0, %1, c4\n" : : "r" (mask & 0xffffffff), "r" (mask >> 32)); }

// Clear GPIO 64-bit output enable; operation: sio_hw->gpio_oe_clr = mask.L; sio_hw->gpio_hi_oe_clr = mask.H;
INLINE void GPIOC_OeClrAll(u64 mask) { __asm volatile (" mcrr p0, #3, %0, %1, c4\n" : : "r" (mask & 0xffffffff), "r" (mask >> 32)); }

// === Single-bit OUT write instructions

// Output to GPIO pin
INLINE void GPIOC_Out(uint pin, int val) { __asm volatile (" mcrr p0, #4, %0, %1, c0\n" : : "r" (pin), "r" ((val == 0) ? 0 : 1)); }

// Toggle GPIO pin output
INLINE void GPIOC_OutXor(uint pin) { __asm volatile (" mcr p0, #5, %0, c0, c0\n" : : "r" (pin)); }

// Set GPIO pin output
INLINE void GPIOC_OutSet(uint pin) { __asm volatile (" mcr p0, #6, %0, c0, c0\n" : : "r" (pin)); }

// Clear GPIO pin output
INLINE void GPIOC_OutClr(uint pin) { __asm volatile (" mcr p0, #7, %0, c0, c0\n" : : "r" (pin)); }

// Conditionally toggle GPIO pin output
INLINE void GPIOC_OutXor2(uint pin, int val) { __asm volatile (" mcrr p0, #5, %0, %1, c0\n" : : "r" (pin), "r" ((val == 0) ? 0 : 1)); }

// Conditionally set GPIO pin output
INLINE void GPIOC_OutSet2(uint pin, int val) { __asm volatile (" mcrr p0, #6, %0, %1, c0\n" : : "r" (pin), "r" ((val == 0) ? 0 : 1)); }

// Conditionally clear GPIO pin output
INLINE void GPIOC_OutClr2(uint pin, int val) { __asm volatile (" mcrr p0, #7, %0, %1, c0\n" : : "r" (pin), "r" ((val == 0) ? 0 : 1)); }

// === Single-bit OE write instructions

// Output enable to GPIO pin
INLINE void GPIOC_Oe(uint pin, int val) { __asm volatile (" mcrr p0, #4, %0, %1, c4\n" : : "r" (pin), "r" ((val == 0) ? 0 : 1)); }

// Toggle GPIO pin output enable
INLINE void GPIOC_OeXor(uint pin) { __asm volatile (" mcr p0, #5, %0, c0, c4\n" : : "r" (pin)); }

// Set GPIO pin output enable
INLINE void GPIOC_OeSet(uint pin) { __asm volatile (" mcr p0, #6, %0, c0, c4\n" : : "r" (pin)); }

// Clear GPIO pin output enable
INLINE void GPIOC_OeClr(uint pin) { __asm volatile (" mcr p0, #7, %0, c0, c4\n" : : "r" (pin)); }

// Conditionally toggle GPIO pin output enable
INLINE void GPIOC_OeXor2(uint pin, int val) { __asm volatile (" mcrr p0, #5, %0, %1, c4\n" : : "r" (pin), "r" ((val == 0) ? 0 : 1)); }

// Conditionally set GPIO pin output enable
INLINE void GPIOC_OeSet2(uint pin, int val) { __asm volatile (" mcrr p0, #6, %0, %1, c4\n" : : "r" (pin), "r" ((val == 0) ? 0 : 1)); }

// Conditionally clear GPIO pin output enable
INLINE void GPIOC_OeClr2(uint pin, int val) { __asm volatile (" mcrr p0, #7, %0, %1, c4\n" : : "r" (pin), "r" ((val == 0) ? 0 : 1)); }

// === Indexed mask write instructions (index = 0 to LOW, 1 to HIGH)

// Output to GPIO indexed; operation: sio_hw->gpio_out[inx] = mask;
INLINE void GPIOC_OutAllInx(uint inx, u32 mask) { __asm volatile (" mcrr p0, #8, %1, %0, c0\n" : : "r" (inx), "r" (mask)); }

// Toggle GPIO output indexed; operation: sio_hw->gpio_togl[inx] = mask;
INLINE void GPIOC_OutXorAllInx(uint inx, u32 mask) { __asm volatile (" mcrr p0, #9, %1, %0, c0\n" : : "r" (inx), "r" (mask)); }

// Set GPIO output indexed; operation: sio_hw->gpio_set[inx] = mask;
INLINE void GPIOC_OutSetAllInx(uint inx, u32 mask) { __asm volatile (" mcrr p0, #10, %1, %0, c0\n" : : "r" (inx), "r" (mask)); }

// Clear GPIO output indexed; operation: sio_hw->gpio_clr[inx] = mask;
INLINE void GPIOC_OutClrAllInx(uint inx, u32 mask) { __asm volatile (" mcrr p0, #11, %1, %0, c0\n" : : "r" (inx), "r" (mask)); }

// Output enable to GPIO indexed; operation: sio_hw->gpio_oe[inx] = mask;
INLINE void GPIOC_OeAllInx(uint inx, u32 mask) { __asm volatile (" mcrr p0, #8, %1, %0, c4\n" : : "r" (inx), "r" (mask)); }

// Toggle GPIO output enable indexed; operation: sio_hw->gpio_oe_togl[inx] = mask;
INLINE void GPIOC_OeXorAllInx(uint inx, u32 mask) { __asm volatile (" mcrr p0, #9, %1, %0, c4\n" : : "r" (inx), "r" (mask)); }

// Set GPIO output enable indexed; operation: sio_hw->gpio_oe_set[inx] = mask;
INLINE void GPIOC_OeSetAllInx(uint inx, u32 mask) { __asm volatile (" mcrr p0, #10, %1, %0, c4\n" : : "r" (inx), "r" (mask)); }

// Clear GPIO output enable indexed; operation: sio_hw->gpio_oe_clr[inx] = mask;
INLINE void GPIOC_OeClrAllInx(uint inx, u32 mask) { __asm volatile (" mcrr p0, #11, %1, %0, c4\n" : : "r" (inx), "r" (mask)); }

// === Read back output register

// Get GPIO output low; operation: return sio_hw->gpio_out;
INLINE u32 GPIOC_GetOutAllL(void)
{
	u32 res;
	__asm volatile (" mrc p0, #0, %0, c0, c0\n" : "=r" (res));
	return res;
}

// Get GPIO output high; operation: return sio_hw->gpio_hi_out;
INLINE u32 GPIOC_GetOutAllH(void)
{
	u32 res;
	__asm volatile (" mrc p0, #0, %0, c0, c1\n" : "=r" (res));
	return res;
}

// Get GPIO output 64-bit; operation: return sio_hw->gpio_out | (sio_hw->gpio_hi_out << 32);
INLINE u64 GPIOC_GetOutAll(void)
{
	u32 resH, resL;
	__asm volatile (" mrrc p0, #0, %0, %1, c0\n" : "=r" (resL), "=r" (resH));
	return ((u64)resH << 32) | resL;
}

// === Read back output enable register

// Get GPIO output enable low; operation: return sio_hw->gpio_oe;
INLINE u32 GPIOC_GetOeAllL(void)
{
	u32 res;
	__asm volatile (" mrc p0, #0, %0, c0, c4\n" : "=r" (res));
	return res;
}

// Get GPIO output enable high; operation: return sio_hw->gpio_hi_oe;
INLINE u32 GPIOC_GetOeAllH(void)
{
	u32 res;
	__asm volatile (" mrc p0, #0, %0, c0, c5\n" : "=r" (res));
	return res;
}

// Get GPIO output enable 64-bit; operation: return sio_hw->gpio_oe | (sio_hw->gpio_hi_oe << 32);
INLINE u64 GPIOC_GetOeAll(void)
{
	u32 resH, resL;
	__asm volatile (" mrrc p0, #0, %0, %1, c4\n" : "=r" (resL), "=r" (resH));
	return ((u64)resH << 32) | resL;
}

// === Read GPIO input

// Get GPIO input low; operation: return sio_hw->gpio_in;
INLINE u32 GPIOC_InAllL(void)
{
	u32 res;
	__asm volatile (" mrc p0, #0, %0, c0, c8\n" : "=r" (res));
	return res;
}

// Get GPIO input high; operation: return sio_hw->gpio_hi_in;
INLINE u32 GPIOC_InAllH(void)
{
	u32 res;
	__asm volatile (" mrc p0, #0, %0, c0, c9\n" : "=r" (res));
	return res;
}

// Get GPIO input 64-bit; operation: return sio_hw->gpio_in | (sio_hw->gpio_hi_in << 32);
INLINE u64 GPIOC_InAll(void)
{
	u32 resH, resL;
	__asm volatile (" mrrc p0, #0, %0, %1, c8\n" : "=r" (resL), "=r" (resH));
	return ((u64)resH << 32) | resL;
}

#ifdef __cplusplus
}
#endif

#endif // _SDK_GPIO_COPROC_H

#endif // USE_GPIOCOPROC
