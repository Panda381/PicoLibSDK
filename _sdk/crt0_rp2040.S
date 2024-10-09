/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sdk_addressmap_rp2040.h"

#define LED_PIN			25		// GPIO with system LED (0..31)
#define LED_PIN2		22		// GPIO with alternate LED (Picopad, 0..31) ... set equal LED_PIN to disable 2nd LED

#define PICO_CRT0_VERSION_MAJOR	1		// CRT0 version major
#define PICO_CRT0_VERSION_MINOR	0		// CRT0 version minor
#define PICO_CRT0_VERSION_BUILD	0		// CRT0 version build

#define SIO_CPUID_OFFSET	0x00000000	// offset of CPUID register
#define PICO_STACK_SIZE		0x1000u		// CPU 0 stack size
#define PICO_HEAP_SIZE		0x100u		// heap size
#define M0PLUS_VTOR_OFFSET	0x0000ed08	// offset of VTOR control register
#define BOOTLOADER_SIZE		0x8000		// size of boot loader
#define SCB_VTOR		0xe000ed08	// address of register of Cortex M0+ interrupt vector table VTOR

#define BINARY_INFO_MARKER_START	0x7188ebf2	// start marker of binary info
#define BINARY_INFO_MARKER_END		0xe71aa390	// end marker of binary info

#define PICOBIN_BLOCK_MARKER_START		0xffffded3	// start marke of picobin block
#define PICOBIN_BLOCK_MARKER_END		0xab123579	// end marke of picobin block
#define PICOBIN_BLOCK_ITEM_1BS_IMAGE_TYPE	0x42		// image type marker
#define PICOBIN_BLOCK_ITEM_1BS_VERSION		0x48		// crt0 version marker
#define PICOBIN_BLOCK_ITEM_1BS_VECTOR_TABLE	0x03		// vector table marker
#define PICOBIN_BLOCK_ITEM_2BS_LAST		(0x80 | 0x7f)	// end marker
#define PICOBIN_INDEX_TO_BITS(y, x)		(y ## _ ## x << y ## _LSB)
#define PICOBIN_IMAGE_TYPE_IMAGE_TYPE_LSB	0
#define PICOBIN_IMAGE_TYPE_IMAGE_TYPE_EXE	0x1
#define PICOBIN_IMAGE_TYPE_IMAGE_TYPE_AS_BITS(x) PICOBIN_INDEX_TO_BITS(PICOBIN_IMAGE_TYPE_IMAGE_TYPE, x)
#define PICOBIN_IMAGE_TYPE_EXE_SECURITY_LSB	4
#define PICOBIN_IMAGE_TYPE_EXE_SECURITY_NS	0x1
#define PICOBIN_IMAGE_TYPE_EXE_SECURITY_AS_BITS(x) PICOBIN_INDEX_TO_BITS(PICOBIN_IMAGE_TYPE_EXE_SECURITY, x)
#define PICOBIN_IMAGE_TYPE_EXE_CPU_LSB		8
#define PICOBIN_IMAGE_TYPE_EXE_CPU_ARM		0
#define PICOBIN_IMAGE_TYPE_EXE_CPU_AS_BITS(x) PICOBIN_INDEX_TO_BITS(PICOBIN_IMAGE_TYPE_EXE_CPU, x)
#define PICOBIN_IMAGE_TYPE_EXE_CHIP_LSB		12
#define PICOBIN_IMAGE_TYPE_EXE_CHIP_RP2040_	0		// do not use x="RP2040", it is predefined symbol
#define PICOBIN_IMAGE_TYPE_EXE_CHIP_AS_BITS(x) PICOBIN_INDEX_TO_BITS(PICOBIN_IMAGE_TYPE_EXE_CHIP, x)
#define CRT0_TBYB_FLAG				0

#if LED_PIN < LED_PIN2
#define LED_PINBASE LED_PIN
#define LED_PINOFF (LED_PIN2 - LED_PIN)
#else // LED_PIN >= LED_PIN2
#define LED_PINBASE LED_PIN2
#define LED_PINOFF (LED_PIN - LED_PIN2)
#endif

	.syntax unified
	.cpu cortex-m0plus
	.thumb

// ==== Vector table

	.section .vectors, "ax"
	.align 8		// aligned to 256 bytes

.global __vectors	// 48 vectors
__vectors:
	// ARM Cortex-M0+ core interrupt request indices - Exceptions
	.word	__StackTop	// -16 end of stack for CPU0
	.word	_reset_handler	// -15 IRQ_RESET reset handler
	.word	isr_nmi		// -14 IRQ_NMI interrupt of NMI
	.word	isr_hardfault	// -13 IRQ_HARDFAULT hard fault
	.word	isr_invalid	// -12 ... Reserved, should never fire
	.word	isr_invalid	// -11 ... Reserved, should never fire
	.word	isr_invalid	// -10 ... Reserved, should never fire
	.word	isr_invalid	// -9 ... Reserved, should never fire
	.word	isr_invalid	// -8 ... Reserved, should never fire
	.word	isr_invalid	// -7 ... Reserved, should never fire
	.word	isr_invalid	// -6 ... Reserved, should never fire
	.word	isr_svcall	// -5 IRQ_SVCALL SuperVisor Call
	.word	isr_invalid	// -4 ... Reserved, should never fire
	.word	isr_invalid	// -3 ... Reserved, should never fire
	.word	isr_pendsv	// -2 IRQ_PENDSV Pendable Service Call
	.word	isr_systick	// -1 IRQ_SYSTICK System Tick Timer

	// RP2040 device interrupt request indices - Interrupts
	.word	isr_irq0	// 0 IRQ_TIMER_0 isr_timer_0
	.word	isr_irq1	// 1 IRQ_TIMER_1 isr_timer_1
	.word	isr_irq2	// 2 IRQ_TIMER_2 isr_timer_2
	.word	isr_irq3	// 3 IRQ_TIMER_3 isr_timer_3
	.word	isr_irq4	// 4 IRQ_PWM_WRAP isr_pwm_wrap
	.word	isr_irq5	// 5 IRQ_USBCTRL isr_usbctrl
	.word	isr_irq6	// 6 IRQ_XIP isr_xip
	.word	isr_irq7	// 7 IRQ_PIO0_0 isr_pio0_0	
	.word	isr_irq8	// 8 IRQ_PIO0_1 isr_pio0_1	
	.word	isr_irq9	// 9 IRQ_PIO1_0 isr_pio1_0	
	.word	isr_irq10	// 10 IRQ_PIO1_1 isr_pio1_1	
	.word	isr_irq11	// 11 IRQ_DMA_0 isr_dma_0	
	.word	isr_irq12	// 12 IRQ_DMA_1 isr_dma_1	
	.word	isr_irq13	// 13 IRQ_IO_BANK0 isr_io_bank0	
	.word	isr_irq14	// 14 IRQ_IO_QSPI isr_io_qspi	
	.word	isr_irq15	// 15 IRQ_SIO_PROC0 isr_sio_proc0	
	.word	isr_irq16	// 16 IRQ_SIO_PROC1 isr_sio_proc1	
	.word	isr_irq17	// 17 IRQ_CLOCKS isr_clocks	
	.word	isr_irq18	// 18 IRQ_SPI0 isr_spi0	
	.word	isr_irq19	// 19 IRQ_SPI1 isr_spi1	
	.word	isr_irq20	// 20 IRQ_UART0 isr_uart0	
	.word	isr_irq21	// 21 IRQ_UART1 isr_uart1	
	.word	isr_irq22	// 22 IRQ_ADC_FIFO isr_adc_fifo	
	.word	isr_irq23	// 23 IRQ_I2C0 isr_i2c0	
	.word	isr_irq24	// 24 IRQ_I2C1 isr_i2c1	
	.word	isr_irq25	// 25 IRQ_RTC isr_rtc		
	.word	isr_irq26	// 26 IRQ_SPAREIRQ_0
	.word	isr_irq27	// 27 IRQ_SPAREIRQ_1
	.word	isr_irq28	// 28 IRQ_SPAREIRQ_2
	.word	isr_irq29	// 29 IRQ_SPAREIRQ_3
	.word	isr_irq30	// 30 IRQ_SPAREIRQ_4
	.word	isr_irq31	// 31 IRQ_SPAREIRQ_5

// ==== Application info header (Flash version: start at offset 48*4 = 0x0C0; RAM version: start at offset 0x100 + 48*4 = 0x1C0)

	.section .picopad_info_header, "a"
	.align 2		// align to 4 bytes

.global picopad_info_header
picopad_info_header:
	.ascii	"PPAD"		// app[48]: identification magic mark
	.word	0x01234567	// app[49]: application length after this header (=total length without boot loader and 51*4)
	.word	0x89ABCDEF	// app[50]: application CRC of following program data
	.word	__StackTop	// app[51]: start stack ...start of program data
	.word	_reset_handler	// app[52]: start address
	.ascii	"ppad"		// app[53]: identification magic mark 2

// ==== Binary info header
// Header must be in first 256 bytes of main image (i.e. excluding flash boot2).
// For flash builds we put it immediately after vector table; for NO_FLASH the
// vectors are at a +0x100 offset because the bootrom enters RAM images directly
// at their lowest address, so we put the header in the VTOR alignment hole.

	.section .binary_info_header, "a"
	.align 2		// align to 4 bytes

binary_info_header:
	.word	BINARY_INFO_MARKER_START	// 0xbinary info start marker
	.word	__binary_info_start		// start address of binary info data
	.word	__binary_info_end		// end address of binary info data
	.word	data_cpy_table			// data copy table - we may need to decode pointers that are in RAM at runtime.
	.word	BINARY_INFO_MARKER_END		// binary info end marker

// ==== Embedded Picobin block

	.section .embedded_block, "a"
	.p2align 2		// align to 4 bytes

embedded_block:
	// start marker
	.word	PICOBIN_BLOCK_MARKER_START		// picobin start marker

	// image type (block item size: 1 word = 4 bytes)
	.byte	PICOBIN_BLOCK_ITEM_1BS_IMAGE_TYPE	// image type marker
	.byte	1					// 1 word size of this item
							// image type:
							//	1 << 0 ... EXE
							//	1 << 4 ... non-secure
							//	0 << 8 ... ARM
							//	0 << 12 ... RP2040
	.hword PICOBIN_IMAGE_TYPE_IMAGE_TYPE_AS_BITS(EXE)	| \
		PICOBIN_IMAGE_TYPE_EXE_SECURITY_AS_BITS(NS)	| \
		PICOBIN_IMAGE_TYPE_EXE_CPU_AS_BITS(ARM)		| \
		PICOBIN_IMAGE_TYPE_EXE_CHIP_AS_BITS(RP2040_)	| \
		CRT0_TBYB_FLAG

	// crt0 version (block item size: 2 words = 8 bytes)
	.byte	PICOBIN_BLOCK_ITEM_1BS_VERSION		// crt0 version marker
	.byte	2					// 2 words wize of this item
	.hword	PICO_CRT0_VERSION_BUILD			// crt0 version build
	.hword	PICO_CRT0_VERSION_MINOR			// crt0 version minor
	.hword	PICO_CRT0_VERSION_MAJOR			// crt0 version major

	// vector table (block item size: 2 words = 8 bytes)
	.byte	PICOBIN_BLOCK_ITEM_1BS_VECTOR_TABLE	// vector table marker
	.byte	2					// 2 words size of this item
	.hword	0					// ... align block item size to words
	.word	__vectors				// address of vector table

	// end block item (block item size: 2 words = 8 bytes)
	.byte	PICOBIN_BLOCK_ITEM_2BS_LAST		// end marker
	.byte	(embedded_block_end-embedded_block-16)/4 // (= 5) total size of all in words LOW, without end block item and without markers
	.byte	0					// total size of all in words HIGH
	.byte	0					// ... align block item size to words
	.word	0					// offset to next block (0 = this is last block)

	// end marker
	.word	PICOBIN_BLOCK_MARKER_END		// picobin end marker
embedded_block_end:

// ==== Default handler

	.section .vector_handlers, "ax"
	.align 2		// aligned to 4 bytes

	// Declare weak symbols for each ISR (default fall to breakpoint), can be overriden by C function
.macro decl_isr name
.weak \name
.type \name,%function
.thumb_func
\name:
.endm
	decl_isr	isr_invalid
	decl_isr	isr_nmi
	decl_isr	isr_hardfault
	decl_isr	isr_svcall
	decl_isr	isr_pendsv
	decl_isr	isr_systick

	decl_isr	isr_irq0
	decl_isr	isr_irq1
	decl_isr	isr_irq2
	decl_isr	isr_irq3
	decl_isr	isr_irq4
	decl_isr	isr_irq5
	decl_isr	isr_irq6
	decl_isr	isr_irq7
	decl_isr	isr_irq8
	decl_isr	isr_irq9
	decl_isr	isr_irq10
	decl_isr	isr_irq11
	decl_isr	isr_irq12
	decl_isr	isr_irq13
	decl_isr	isr_irq14
	decl_isr	isr_irq15
	decl_isr	isr_irq16
	decl_isr	isr_irq17
	decl_isr	isr_irq18
	decl_isr	isr_irq19
	decl_isr	isr_irq20
	decl_isr	isr_irq21
	decl_isr	isr_irq22
	decl_isr	isr_irq23
	decl_isr	isr_irq24
	decl_isr	isr_irq25
	decl_isr	isr_irq26
	decl_isr	isr_irq27
	decl_isr	isr_irq28
	decl_isr	isr_irq29
	decl_isr	isr_irq30
	decl_isr	isr_irq31

	// unhandled IRQ
.global unhandled_irq
unhandled_irq:
	// disable interrupts and exceptions
	cpsid	i				// disable interrupts
	cpsid	f				// disable exceptions
	isb					// instruction synchronization barrier
	
// prepare fatal error descriptor sFatal (on change update sFatalFrame structure in sdk_fatal.h, too)

	// reserve space for fatal error descriptor
	sub	sp,#0x30			// create space in the stack

	// get IRQ number
	mrs	r0,ipsr				// get Interrupt Program Status Register
	uxtb	r0,r0				// only low 8 bits are valid
	subs	r0,#16				// convert to signed value
	str	r0,[sp,#0x00]			// save IRQ number

	// save registers
	str	r4,[sp,#0x04]			// save R4
	str	r5,[sp,#0x08]			// save R5
	str	r6,[sp,#0x0C]			// save R6
	str	r7,[sp,#0x10]			// save R7
	mov	r0,r8
	str	r0,[sp,#0x14]			// save R8
	mov	r0,r9
	str	r0,[sp,#0x18]			// save R9
	mov	r0,r10
	str	r0,[sp,#0x1C]			// save R10
	mov	r0,r11
	str	r0,[sp,#0x20]			// save R11

	// save stack pointer
	mov	r0,sp				// get current stack pointer
	adds	r0,#0x60			// skip fatal frame
	str	r0,[sp,#0x24]			// save current stack pointer

	// get CONTROL register
	mrs	r0,control			// get CONTROL register
	str	r0,[sp,#0x28]			// save CONTROL register

	// fatal error message
	mov	r0,sp
	ldr	r3,=FatalErrorMsg		// fatal error message
	blx	r3				// display fatal error message

// fatal error stop
.global FatalErrorStop
FatalErrorStop:

	// initialize LED output
	ldr	r3,=(PADS_BANK0_BASE + 4 + LED_PINBASE*4) // = GPIO_PAD(LED_PINBASE)
	movs	r2,#0x56			// B1|B2|B4|B6 ... value to reset pad setup
	str	r2,[r3,#0]			// reset pad setup
#if LED_PIN != LED_PIN2
	str	r2,[r3,#LED_PINOFF*4]		// reset pad setup
#endif

	ldr	r3,=(IO_BANK0_BASE + 4 + LED_PINBASE*8) // = GPIO_CTRL(LED_PINBASE) 
	movs	r2,#5				// GPIO_FNC_SIO
	str	r2,[r3,#0]			// set GPIO function GPIO_FNC_SIO, reset all overrides
#if LED_PIN != LED_PIN2
	str	r2,[r3,#LED_PINOFF*8]		// set GPIO function GPIO_FNC_SIO, reset all overrides
#endif

	ldr	r3,=(SIO_BASE + 0x1C)		// GPIO_OUT_XOR
	movs	r2,#1
#if LED_PIN != LED_PIN2
	lsls	r2,#LED_PINOFF			// mask value of LED_PIN bit
	adds	r2,#1
#endif
	lsls	r2,#LED_PINBASE			// mask value of LED_PIN2 bit
	str	r2,[r3,#8]			// enable output GPIO_OE_SET

unhandled_irq_2:
	// flip LED output
	str	r2,[r3,#0]			// flip output GPIO_OUT_XOR

	// wait loop
	ldr	r1,=3000000			// wait period
unhandled_irq_3:
	subs	r1,#1
	bne	unhandled_irq_3
	b	unhandled_irq_2

// ==== Reset handler

	.section .reset, "ax"
	.align 2		// aligned to 4 bytes

.type _reset_handler,%function
.thumb_func
.global _reset_handler
_reset_handler:

// set stack pointer (may be uninitialized from BOOTROM when starting RAM version)

	ldr	r0,=__StackTop			// top of the stack
	msr	MSP,r0				// set stack pointer

// set vector table address (may be uninitialized from BOOTROM when starting RAM version)

	ldr	r0,=__vectors			// vectors
	ldr	r1,=SCB_VTOR			// SCB_VTOR register (PPB_BASE + 0xED08)
	str	r0,[r1,#0]			// set VTOR address

// Only core 0 should run the C runtime startup code; core 1 is normally
// sleeping in the bootrom at this point but check to be sure

	ldr	r0,=(SIO_BASE + SIO_CPUID_OFFSET) // address of CPUID register
	ldr	r0,[r0]				// get current CPU (0 or 1)
	cmp	r0,#0				// is running on CPU 0 ?
	bne	Core1ExitBootrom 		// run CPU 1 back to bootrom

// Copy read-only data from Flash to RAM (only if not RAM version)

#if !NO_FLASH

	ldr	r1,=__etext			// read address of data in Flash
	ldr	r2,=__data_start__		// write address of data to RAM
	ldr	r3,=__data_end__		// end of write address
	b	data_cpy

data_cpy_loop:
	ldm	r1!,{r0}			// load word from [R1] to R0 and increment R1
	stm	r2!,{r0}			// save word to [R2] from R0 and increment R2
data_cpy:
	cmp	r2,r3				// check limit of write address
	blo	data_cpy_loop			// repeat until R2 >= R3

#endif

// Zero out the BSS segment

2:	ldr	r1,=__bss_start__		// start of BSS segment -> R1 (get from linker script *.ld)
	ldr	r2,=__bss_end__			// end of BSS segment -> R2 (get from linker script *.ld)
	movs	r0,#0				// clear value 0 -> R0
	b	bss_fill_test

bss_fill_loop:
	stm	r1!,{r0}			// store 0 from R0 to [R1] and increment R1
bss_fill_test:
	cmp	r1,r2				// end of segment?
	bne	bss_fill_loop			// continue with next data

// Runtime initialize, function RuntimeInit()

	ldr	r1,=RuntimeInit
	blx	r1

// Start main function Main()

	ldr	r1,=main
	blx	r1

// Exit and loop
//  Do not use exit, just simple loop - some programs do not use
//  end loop, but remain running after main() is terminated.

1:	b	1b

// Run CPU 1 exit to BOOTROM
.type Core1ExitBootrom,%function
.thumb_func
.global Core1ExitBootrom
Core1ExitBootrom:
	cpsid	i				// disable interrupts
	ldr	r1,=(PPB_BASE + M0PLUS_VTOR_OFFSET) // address of vector table
	movs	r0,#0				// BOOTROM vector table address
	str	r0,[r1]				// reset VTOR to BOOTROM vector table
	ldmia	r0!,{r1, r2}			// R1 <- stack pointer, R2 <- reset vector
	msr	MSP,r1				// setup stavk pointer
	bx	r2				// jump to BOOTROM reset

// old method - do not use, it could overwrite core 0 stack
// hold_non_core0_in_bootrom:
//	ldr	r0,='W'|('V' << 8)
//	bl	RomFunc				// rom_func_lookup
//	bx	r0

// ==== data copy table (copy R/O data from Flash to RAM)

	.align 2

data_cpy_table:
#if !NO_FLASH
	.word	__etext		// start of source address in Flash
	.word	__data_start__	// start of destination address in RAM
	.word	__data_end__	// end of destination address in RAM
#endif
	.word	0		// null terminator

// ==== Run application
// Called from boot3 loader

// run application from Flash
.type GoToApp,%function
.thumb_func
.global GoToApp
GoToApp:
	ldr	r0,=(XIP_BASE + BOOTLOADER_SIZE) // vector table of application -> R0
	ldr	r1,=(PPB_BASE + M0PLUS_VTOR_OFFSET) // address of vector table
	str	r0,[r1]				// set application vector table
	ldmia	r0,{r0, r1}			// load stack pointer -> R0 and start address -> R1
	msr	msp,r0				// set stack pointer
	bx	r1				// run application code

// run application from RAM
.type GoToAppRam,%function
.global GoToAppRam
GoToAppRam:
	ldr	r0,=(SRAM_BASE)			// vector table of application -> R0
	adds	r0,1				// set thumb mode
	bx	r0				// jump to reset vector

// ==== CPU 0 stack

	.section .stack0
	.align	5
	.equ	StackSize,PICO_STACK_SIZE
	.space	StackSize

// ==== Heap

	.section .heap
	.align	2
	.equ	HeapSize,PICO_HEAP_SIZE
	.space	HeapSize
