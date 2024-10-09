/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sdk_addressmap_rp2350.h"

#define LED_PIN			25		// GPIO with system LED (0..31)
#define LED_PIN2		22		// GPIO with alternate LED (Picopad, 0..31) ... set equal LED_PIN to disable 2nd LED

#define PICO_CRT0_VERSION_MAJOR	1		// CRT0 version major
#define PICO_CRT0_VERSION_MINOR	0		// CRT0 version minor
#define PICO_CRT0_VERSION_BUILD	0		// CRT0 version build

#define VTABLE_FIRST_IRQ	26		// index of first IRQ
#define IRQ_MTIMER_OFF		0x60		// offset of IRQ_MTIMER handler address
#define IRQ_SOFTIRQ_OFF		0x64		// offset of IRQ_SOFTIRQ handler address
#define IRQ_FIRST_OFF		0x68		// offset of first IRQ handler address (= VTABLE_FIRST_IRQ*4)

#define PICO_STACK_SIZE		0x1000u		// CPU 0 stack size
#define PICO_HEAP_SIZE		0x100u		// heap size
#define BOOTLOADER_SIZE		0x10000		// size of boot loader

#define BINARY_INFO_MARKER_START	0x7188ebf2	// start marker of binary info
#define BINARY_INFO_MARKER_END		0xe71aa390	// end marker of binary info

#define PICOBIN_BLOCK_MARKER_START		0xffffded3	// start marke of picobin block
#define PICOBIN_BLOCK_MARKER_END		0xab123579	// end marke of picobin block
#define PICOBIN_BLOCK_ITEM_1BS_IMAGE_TYPE	0x42		// image type marker
#define PICOBIN_BLOCK_ITEM_1BS_VERSION		0x48		// crt0 version marker
#define PICOBIN_BLOCK_ITEM_1BS_VECTOR_TABLE	0x03		// vector table marker
#define PICOBIN_BLOCK_ITEM_1BS_ENTRY_POINT	0x44		// entry point
#define PICOBIN_BLOCK_ITEM_2BS_LAST		(0x80 | 0x7f)	// end marker
#define PICOBIN_INDEX_TO_BITS(y, x)		(y ## _ ## x << y ## _LSB)
#define PICOBIN_IMAGE_TYPE_IMAGE_TYPE_LSB	0
#define PICOBIN_IMAGE_TYPE_IMAGE_TYPE_EXE	0x1
#define PICOBIN_IMAGE_TYPE_IMAGE_TYPE_AS_BITS(x) PICOBIN_INDEX_TO_BITS(PICOBIN_IMAGE_TYPE_IMAGE_TYPE, x)
#define PICOBIN_IMAGE_TYPE_EXE_CPU_LSB		8
#define PICOBIN_IMAGE_TYPE_EXE_CPU_RISCV_	1		// do not use x="RISCV", it is predefined symbol
#define PICOBIN_IMAGE_TYPE_EXE_CPU_AS_BITS(x) PICOBIN_INDEX_TO_BITS(PICOBIN_IMAGE_TYPE_EXE_CPU, x)
#define PICOBIN_IMAGE_TYPE_EXE_CHIP_LSB		12
#define PICOBIN_IMAGE_TYPE_EXE_CHIP_RP2350_	1		// do not use x="RP2350", it is predefined symbol
#define PICOBIN_IMAGE_TYPE_EXE_CHIP_AS_BITS(x) PICOBIN_INDEX_TO_BITS(PICOBIN_IMAGE_TYPE_EXE_CHIP, x)
#define CRT0_TBYB_FLAG				0

#if LED_PIN < LED_PIN2
#define LED_PINBASE LED_PIN
#define LED_PINOFF (LED_PIN2 - LED_PIN)
#else // LED_PIN >= LED_PIN2
#define LED_PINBASE LED_PIN2
#define LED_PINOFF (LED_PIN - LED_PIN2)
#endif

#define RVCSR_MEICONTEXT_OFFSET		0x00000be5
#define RVCSR_MEICONTEXT_CLEARTS_BITS	0x00000002
#define RVCSR_MEINEXT_OFFSET		0x00000be4
#define RVCSR_MEINEXT_UPDATE_BITS	0x00000001

#define BOOTROM_ENTRY_OFFSET		0x7dfc
#define RVCSR_MEIFA_OFFSET		0x00000be2
#define RVCSR_MIE_MEIE_BITS		0x00000800
#define RVCSR_MSTATUS_MIE_BITS		0x00000008


	.section .vectors, "ax"

//	.p2align 6,,		// 64-byte aligned is required if vectoring is enabled
// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - adds incorrect alignment
// after the end of the section. Better to use ". = ALIGN(64);" alignment in *.LD script.

// ==== Start of vector table (MTVEC register must be in vector mode)
// RISC-V vector table is usually located in RAM because it contains exception handlers
// that need to be executed fast. The code is independent of location, it can be moved
// to other locations (ROM or RAM), but the memory used must have rights to execute the code.

.global __vectors
__vectors:
	.option push
	.option norvc
	.option norelax

	// RISC-V traps
	j isr_machine_exception		// 0x00 0: Machine exception
	.word 0				// 0x04 1:
	.word 0				// 0x08 2:
	j isr_machine_soft_irq		// 0x0C 3: Machine software IRQ
	.word 0				// 0x10 4:
	.word 0				// 0x14 5:
	.word 0				// 0x18 6:
	j isr_machine_timer		// 0x1C 7: Machine timer IRQ
	.word 0				// 0x20 8:
	.word 0				// 0x24 9:
	.word 0				// 0x28 10:
	j isr_machine_external_irq	// 0x2C 11: Machine external IRQ

	.option pop

	// RISC-V exceptions
	.word	isr_mcall		// 0x30 -14 IRQ_MCALL (exception 11) call from machine mode (M-mode)
	.word	unhandled_irq		// 0x34 -13 (exception 10) unused exception
	.word	isr_scall		// 0x38 -12 IRQ_SCALL (exception 9) call from supervisor mode (S-mode)
	.word	isr_ucall		// 0x3C -11 IRQ_UCALL (exception 8) call from user mode (U-mode)
	.word	isr_storefault		// 0x40 -10 IRQ_STOREFAULT (exception 7) store accesss fault (bus fault)
	.word	isr_storealign		// 0x44 -9 IRQ_STOREALIGN (exception 6) store address misaligned (alignment)
	.word	isr_loadfault		// 0x48 -8 IRQ_LOADFAULT (exception 5) load access fault (bus fault)
	.word	isr_loadalign		// 0x4C -7 IRQ_LOADALIGN (exception 4) load address misaligned (alignment)
	.word	isr_breakpoint		// 0x50 -6 IRQ_BREAKPOINT (exception 3) breakpoint
	.word	isr_instrilegal		// 0x54 -5 IRQ_INSTRILEGAL (exception 2) illegal instruction
	.word	isr_instrfault		// 0x58 -4 IRQ_INSTRFAULT (exception 1) instruction access fault (bus fault)
	.word	isr_instralign		// 0x5C -3 IRQ_INSTRALIGN (exception 0) instruction fetch was misaligned (alignment)
	.word	isr_mtimer		// 0x60 -2 IRQ_MTIMER (trap 7) machine-mode timer
	.word	isr_softirq		// 0x64 -1 IRQ_SOFTIRQ (trap 3) software IRQ

// here is offset 0x68 = 26 words ... VTABLE_FIRST_IRQ is 26

	// RISC-V external IRQs
	.word	isr_irq0		// 0  IRQ_TIMER0_0	us timer IRQ 0
	.word	isr_irq1		// 1  IRQ_TIMER0_1	us timer IRQ 1
	.word	isr_irq2		// 2  IRQ_TIMER0_2	us timer IRQ 2
	.word	isr_irq3		// 3  IRQ_TIMER0_3	us timer IRQ 3
	.word	isr_irq4		// 4  IRQ_TIMER1_0	sys_clk timer IRQ 0
	.word	isr_irq5		// 5  IRQ_TIMER1_1	sys_clk timer IRQ 1
	.word	isr_irq6		// 6  IRQ_TIMER1_2	sys_clk timer IRQ 2
	.word	isr_irq7		// 7  IRQ_TIMER1_3	sys_clk timer IRQ 3
	.word	isr_irq8		// 8  IRQ_PWM_WRAP_0	PWM wrap 0
	.word	isr_irq9		// 9  IRQ_PWM_WRAP_1	PWM wrap 1
	.word	isr_irq10		// 10 IRQ_DMA_0		DMA IRQ 0
	.word	isr_irq11		// 11 IRQ_DMA_1		DMA IRQ 1
	.word	isr_irq12		// 12 IRQ_DMA_2		DMA IRQ 2
	.word	isr_irq13		// 13 IRQ_DMA_3		DMA IRQ 3
	.word	isr_irq14		// 14 IRQ_USBCTRL	USB ctrl
	.word	isr_irq15		// 15 IRQ_PIO0_0	PIO 0 IRQ 0
	.word	isr_irq16		// 16 IRQ_PIO0_1	PIO 0 IRQ 1
	.word	isr_irq17		// 17 IRQ_PIO1_0	PIO 1 IRQ 0
	.word	isr_irq18		// 18 IRQ_PIO1_1	PIO 1 IRQ 1
	.word	isr_irq19		// 19 IRQ_PIO2_0	PIO 2 IRQ 0
	.word	isr_irq20		// 20 IRQ_PIO2_1	PIO 2 IRQ 1
	.word	isr_irq21		// 21 IRQ_IO_BANK0	pad bank IO
	.word	isr_irq22		// 22 IRQ_IO_BANK0_NS	pad bank IO NS
	.word	isr_irq23		// 23 IRQ_IO_QSPI	qspi bank IO
	.word	isr_irq24		// 24 IRQ_IO_QSPI_NS	qspi bank IO NS
	.word	isr_irq25		// 25 IRQ_SIO_FIFO	SIO FIFO
	.word	isr_irq26		// 26 IRQ_SIO_BELL	SIO BELL
	.word	isr_irq27		// 27 IRQ_SIO_FIFO_NS	SIO FIFO NS
	.word	isr_irq28		// 28 IRQ_SIO_BELL_NS	SIO BELL NS
	.word	isr_irq29		// 29 IRQ_SIO_MTIMECMP	SIO MTIMECMP
	.word	isr_irq30		// 30 IRQ_CLOCKS	clocks
	.word	isr_irq31		// 31 IRQ_SPI0		SPI 0
	.word	isr_irq32		// 32 IRQ_SPI1		SPI 1
	.word	isr_irq33		// 33 IRQ_UART0		UART 0
	.word	isr_irq34		// 34 IRQ_UART1		UART 1
	.word	isr_irq35		// 35 IRQ_ADC_FIFO	ADC FIFO
	.word	isr_irq36		// 36 IRQ_I2C0		I2C 0
	.word	isr_irq37		// 37 IRQ_I2C1		I2C 1
	.word	isr_irq38		// 38 IRQ_OTP		OTP
	.word	isr_irq39		// 39 IRQ_TRNG		TRNG
	.word	isr_irq40		// 40 IRQ_PROC0_CTI
	.word	isr_irq41		// 41 IRQ_PROC1_CTI
	.word	isr_irq42		// 42 IRQ_PLL_SYS	PLL SYS
	.word	isr_irq43		// 43 IRQ_PLL_USB	PLL USB
	.word	isr_irq44		// 44 IRQ_POWMAN_POW	POWMAN POW
	.word	isr_irq45		// 45 IRQ_POWMAN_TIMER	POWMAN Timer
	.word	isr_irq46		// 46 IRQ_SPAREIRQ_0	user IRQ 0
	.word	isr_irq47		// 47 IRQ_SPAREIRQ_1	user IRQ 1
	.word	isr_irq48		// 48 IRQ_SPAREIRQ_2	user IRQ 2
	.word	isr_irq49		// 49 IRQ_SPAREIRQ_3	user IRQ 3
	.word	isr_irq50		// 50 IRQ_SPAREIRQ_4	user IRQ 4
	.word	isr_irq51		// 51 IRQ_SPAREIRQ_5	user IRQ 5


// push all registers, except RA (ABI context stack)
isr_push_all:
	sw	t0,4(sp)		// SP+4 <- t0 (x5)
	sw	t1,8(sp)		// SP+8 <- t1 (x6)
	sw	t2,12(sp)		// SP+12 <- t2 (x7)
	sw	a0,16(sp)		// SP+16 <- a0 (x10)
	sw	a1,20(sp)		// SP+20 <- a1 (x11)
	sw	a2,24(sp)		// SP+24 <- a2 (x12)
	sw	a3,28(sp)		// SP+28 <- a3 (x13)
	sw	a4,32(sp)		// SP+32 <- a4 (x14)
	sw	a5,36(sp)		// SP+36 <- a5 (x15)
	sw	a6,40(sp)		// SP+40 <- a6 (x16)
	sw	a7,44(sp)		// SP+44 <- a7 (x17)
	sw	t3,48(sp)		// SP+48 <- t3 (x28)
	sw	t4,52(sp)		// SP+52 <- t4 (x29)
	sw	t5,56(sp)		// SP+56 <- t5 (x30)
	sw	t6,60(sp)		// SP+60 <- t6 (x31)
	ret


// pop all registers, except RA (ABI context stack)
isr_pop_all:
	lw	t0,4(sp)		// t0 (x5) <- SP+4
	lw	t1,8(sp)		// t1 (x6) <- SP+8
	lw	t2,12(sp)		// t2 (x7) <- SP+12
	lw	a0,16(sp)		// a0 (x10) <- SP+16
	lw	a1,20(sp)		// a1 (x11) <- SP+20
	lw	a2,24(sp)		// a2 (x12) <- SP+24
	lw	a3,28(sp)		// a3 (x13) <- SP+28
	lw	a4,32(sp)		// a4 (x14) <- SP+32
	lw	a5,36(sp)		// a5 (x15) <- SP+36
	lw	a6,40(sp)		// a6 (x16) <- SP+40
	lw	a7,44(sp)		// a7 (x17) <- SP+44
	lw	t3,48(sp)		// t3 (x28) <- SP+48
	lw	t4,52(sp)		// t4 (x29) <- SP+52
	lw	t5,56(sp)		// t5 (x30) <- SP+56
	lw	t6,60(sp)		// t6 (x31) <- SP+60
	ret


// unhandled exception (jump must be positionally independent)
isr_unhandled_exception:
	lui	ra,%hi(unhandled_irq)	// load function address HIGH
	addi	ra,ra,%lo(unhandled_irq) // add function address LOW
	jr	ra			// jump to handler


// handler of machine exception
isr_machine_exception:
	// save RA register and get old MSCRATCH register
	// MSCRATCH = 0 outside of exception handler. Another value means nexted exception.
	csrrw	ra,mscratch,ra		// RA <- MSCRATCH <- RA
	bnez	ra,isr_unhandled_exception // if MSCRATCH is not 0, it means unrecoverable nested exception

	// push all registers (except RA)
	addi	sp,sp,-64		// reserve space in the stack (must be 8-byte aligned)
	jal	isr_push_all		// push all registers (function uses RA as link register)

	// get trap cause -> RA
	//  Bit 31 must be 0, exception code must be 0..11
	csrr	ra,mcause		// get exception code: RA <- MCAUSE
	li	t6,11			// prepare max. valid value for IRQ_MCALL: T6 <- 11
	bltu	t6,ra,isr_unhandled_exception // unhandled exception > 11

	// convert trap index to IRQ index -> T6
	li	ra,VTABLE_FIRST_IRQ - 3	// RA <- first_IRQ - 3
	sub	t6,ra,t6		// T6 <- IRQ index = first_IRQ - 3 - exception_index

	// get VTOR address -> RA
	csrr	ra,mtvec		// RA <- read vector address
	andi	ra,ra,~3		// RA <- clear bits 0 and 1 (vectoring mode)

	// call exception handler
	sh2add	ra,t6,ra		// IRQ_index*4 + VTOR address: RA <- (T6<<2) + RA
	lw	ra,(ra)			// RA <- exception handler from table
	jalr	ra,ra			// call exception handler (RA = handler address, use RA as link register)

	// pop all registers (except RA)
	jal	isr_pop_all		// pop all registers (function uses RA as link register)
	addi	sp,sp,64		// destroy space in the stack

	// restore RA register and clear content of MSCRATCH register
	csrrw	ra,mscratch,zero	// RA <- MSCRATCH <- 0
	mret				// return from machine exception


// handler of machine software IRQ
isr_machine_soft_irq:
	// push all registers
	addi	sp,sp,-64		// reserve space in the stack (must be 8-byte aligned)
	sw	ra,0(sp)		// SP+0 <- ra (x1) save link register
	jal	isr_push_all		// push all registers (function uses RA as link register)

	// get VTOR address -> RA
	csrr	ra,mtvec		// RA <- read vector address
	andi	ra,ra,~3		// RA <- clear bits 0 and 1 (vectoring mode)

	// call IRQ handler
	lw	ra,IRQ_SOFTIRQ_OFF(ra)	// RA <- exception handler from table
	j	isr_machine_timer_jump	// jump to handler


// handler of machine timer IRQ
isr_machine_timer:
	// push all registers
	addi	sp,sp,-64		// reserve space in the stack (must be 8-byte aligned)
	sw	ra,0(sp)		// SP+0 <- ra (x1) save link register
	jal	isr_push_all		// push all registers (function uses RA as link register)

	// get VTOR address -> RA
	csrr	ra,mtvec		// RA <- read vector address
	andi	ra,ra,~3		// RA <- clear bits 0 and 1 (vectoring mode)

	// call IRQ handler
	lw	ra,IRQ_MTIMER_OFF(ra)	// RA <- exception handler from table
isr_machine_timer_jump:
	jalr	ra,ra			// call exception handler (RA = handler address, use RA as link register)

	// pop all registers
	jal	isr_pop_all		// pop all registers (function uses RA as link register)
	lw	ra,0(sp)		// ra (x1) <- SP+0 restore link register
	addi	sp,sp,64		// destroy space in the stack
	mret				// return from machine exception


// handler of machine external IRQ
isr_machine_external_irq:
	// push all registers
	addi	sp,sp,-80		// reserve space in the stack (must be 8-byte aligned)
	sw	ra,0(sp)		// SP+0 <- ra (x1) save link register
	jal	isr_push_all		// push all registers (function uses RA as link register)

	// save machine state (MEPC and MSTATUS)
	csrr	a0,mepc			// a0 <- MEPC machine exception PC (return address from interrupt)
	csrr	a1,mstatus		// a1 <- MSTATUS machine status register
	sw	a0,64(sp)		// SP+64 <- MEPC
	sw	a1,68(sp)		// SP+68 <- MSTATUS

save_meicontext:
	// save MEICONTEXT register, disable Machine_Timer_IRQ and Machine_Software_IRQ
	csrrsi	a2,RVCSR_MEICONTEXT_OFFSET,RVCSR_MEICONTEXT_CLEARTS_BITS // set to clear mie.mtie and mie.msie
	sw	a2,72(sp)		// SP+72 <- MEICONTEXT

check_next_irq:
	// update MEINEXT register and check next IRQ (bit 31 is set, if no external interrupt pending)
	csrrsi	a0,RVCSR_MEINEXT_OFFSET,RVCSR_MEINEXT_UPDATE_BITS // update meicontext to next IRQ
	bltz	a0,no_more_irqs		// jump if NOIRQ is set (= bit 31 is set, number is < 0)

	// enable interrupt (a higher priority interruption may come)
	csrsi	mstatus,RVCSR_MSTATUS_MIE_BITS	// set interrupt enable (set mstatus.mie flag)

	// get VTOR address -> RA
	csrr	ra,mtvec		// RA <- read vector address
	andi	ra,ra,~3		// RA <- clear bits 0 and 1 (vectoring mode)

	// call IRQ handler (A0 contains IRQ_index*4, with base 0)
	add	ra,a0,ra		// RA <- IRQ_index*4 + VTOR address
	lw	ra,IRQ_FIRST_OFF(ra)	// RA <- IRQ handler from table
	jalr	ra,ra			// call exception handler (RA = handler address, use RA as link register)

	// disable interrupts
	csrci	mstatus,RVCSR_MSTATUS_MIE_BITS	// clear interrupt enable (clear mstatus.mie flag)
	j	check_next_irq		// check next IRQ

no_more_irqs:
	// restore machine state
	lw	a0,64(sp)		// a0 (x10) <- SP+64 MEPC
	lw	a1,68(sp)		// a1 (x11) <- SP+68 MSTATUS
	lw	a2,72(sp)		// a2 (x12) <- SP+72 MEICONTEXT
	csrw	mepc,a0			// MEPC <- a0 restore MEPC register
	csrw	mstatus,a1		// MSTATUS <- a1 restore MSTATUS register
	csrw	RVCSR_MEICONTEXT_OFFSET,a2 // MEICONTEXT <- a2 restore MEICONTEXT register

	// pop all registers (except RA)
	jal	isr_pop_all		// pop all registers (function uses RA as link register)

	// last check for new IRQ, to avoid high latency of re-entering service
	csrr	ra,RVCSR_MEINEXT_OFFSET	// read MEINEXT register
	bgez	ra,save_meicontext	// jump if NOIRQ is not set (= bit 31 is clear, number is >= 0)

	// pop rest of registers
	lw	ra,0(sp)		// ra (x1) <- SP+0 restore link register
	addi	sp,sp,80		// destroy space in the stack
	mret				// return from machine exception

// ==== End of vector table
// End offset here is 0x2D0 if compiled without compressed instructions.
// Linker aligns it to offset 0x300 using ". = 0x300 - __logical_binary_start;"
// here is offset 0x300 ... this size must correspond to the VECTOR_TABLE_SIZE


// ==== Application info header (start at offset 192*4 = 0x300)

	.section .picopad_info_header, "a"
	.p2align 2,,			// align to 4 bytes
	// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - it adds incorrect alignment
	// after the end of the section. If really needed it, use two commas ,, - it will not add 2 false bytes after segment.

.global picopad_info_header
picopad_info_header:
	.ascii	"PPAD"			// 0x00 app[192]: identification magic mark
	.word	0x01234567		// 0x04 app[193]: application length after this header (=total length without boot loader and 71*4)
	.word	0x89ABCDEF		// 0x08 app[194]: application CRC of following program data
	.word	__StackTop		// 0x0C app[195]: start stack ...start of program data
	.word	_reset_handler		// 0x10 app[196]: start address
	.ascii	"ppad"			// 0x14 app[197]: identification magic mark 2

// ==== Binary info header
// Header must be in first 1024 bytes of main image

	.section .binary_info_header, "a"
	.p2align 2,,		// align to 4 bytes
	// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - it adds incorrect alignment
	// after the end of the section. If really needed it, use two commas ,, - it will not add 2 false bytes after segment.

binary_info_header:
	.word	BINARY_INFO_MARKER_START	// binary info start marker
	.word	__binary_info_start		// start address of binary info data
	.word	__binary_info_end               // end address of binary info data
	.word	data_cpy_table			// data copy table - we may need to decode pointers that are in RAM at runtime.
	.word	BINARY_INFO_MARKER_END		// binary info end marker


// ==== Embedded Picobin block

	.section .embedded_block, "a"
	.p2align 2,,		// align to 4 bytes
	// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - it adds incorrect alignment
	// after the end of the section. If really needed it, use two commas ,, - it will not add 2 false bytes after segment.

embedded_block:
	// start marker
	.word	PICOBIN_BLOCK_MARKER_START		// picobin start marker

	// image type (block item size: 1 word = 4 bytes)
	.byte	PICOBIN_BLOCK_ITEM_1BS_IMAGE_TYPE	// image type marker
	.byte	1					// 1 word size of this item
							// image type:
							//	1 << 0 ... EXE
							//	1 << 8 ... RISCV
							//	1 << 12 ... RP2350
	.hword	PICOBIN_IMAGE_TYPE_IMAGE_TYPE_AS_BITS(EXE)	| \
		PICOBIN_IMAGE_TYPE_EXE_CPU_AS_BITS(RISCV_)	| \
		PICOBIN_IMAGE_TYPE_EXE_CHIP_AS_BITS(RP2350_)	| \
		CRT0_TBYB_FLAG

	// crt0 version (block item size: 2 words = 8 bytes)
	.byte	PICOBIN_BLOCK_ITEM_1BS_VERSION		// crt0 version marker
	.byte	2					// 2 words wize of this item
	.hword	PICO_CRT0_VERSION_BUILD			// crt0 version build
	.hword	PICO_CRT0_VERSION_MINOR			// crt0 version minor
	.hword	PICO_CRT0_VERSION_MAJOR			// crt0 version major

	// on RISC-V entry point (block item size: 3 words = 12 bytes)
	.byte	PICOBIN_BLOCK_ITEM_1BS_ENTRY_POINT	// entry point marker
	.byte	3					// 3 words size of this item
	.byte	0					// ... align block item size to words
	.byte	0					// ... align block item size to words
	.word	_reset_handler				// address of entry point
	.word	__StackTop				// stack pointer

	// vector table (block item size: 2 words = 8 bytes)
	.byte	PICOBIN_BLOCK_ITEM_1BS_VECTOR_TABLE	// vector table marker
	.byte	2					// 2 words size of this item
	.hword	0					// ... align block item size to words
	.word	__vectors				// address of vector table

	// end block item (block item size: 2 words = 8 bytes)
	.byte	PICOBIN_BLOCK_ITEM_2BS_LAST		// end marker
	.byte	(embedded_block_end-embedded_block-16)/4 // (= 8) total size of all in words LOW, without end block item and without markers
	.byte	0					// total size of all in words HIGH
	.byte	0					// ... align block item size to words
	.word	0					// offset to next block (0 = this is last block)

	// end marker
	.word	PICOBIN_BLOCK_MARKER_END		// picobin end marker
embedded_block_end:


// ==== Default handler

	.section .vector_handlers, "ax"
	.p2align 2,,		// aligned to 4 bytes
	// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - it adds incorrect alignment
	// after the end of the section. If really needed it, use two commas ,, - it will not add 2 false bytes after segment.

	// Declare weak symbols for each ISR (default fall to breakpoint), can be overriden by C function
.macro decl_isr name
.weak \name
.type \name,%function
\name:
.endm

	// RISC-V exceptions
	decl_isr	isr_mcall		// 0x30 -14 IRQ_MCALL (exception 11) call from machine mode (M-mode)
	decl_isr	unhandled_irq		// 0x34 -13 IRQ_INVALID (exception 10)
	decl_isr	isr_scall		// 0x38 -12 IRQ_SCALL (exception 9) call from supervisor mode (S-mode)
	decl_isr	isr_ucall		// 0x3C -11 IRQ_UCALL (exception 8) call from user mode (U-mode)
	decl_isr	isr_storefault		// 0x40 -10 IRQ_STOREFAULT (exception 7) store accesss fault (bus fault)
	decl_isr	isr_storealign		// 0x44 -9 IRQ_STOREALIGN (exception 6) store address misaligned (alignment)
	decl_isr	isr_loadfault		// 0x48 -8 IRQ_LOADFAULT (exception 5) load access fault (bus fault)
	decl_isr	isr_loadalign		// 0x4C -7 IRQ_LOADALIGN (exception 4) load address misaligned (alignment)
	decl_isr	isr_breakpoint		// 0x50 -6 IRQ_BREAKPOINT (exception 3) breakpoint
	decl_isr	isr_instrilegal		// 0x54 -5 IRQ_INSTRILEGAL (exception 2) illegal instruction
	decl_isr	isr_instrfault		// 0x58 -4 IRQ_INSTRFAULT (exception 1) instruction access fault (bus fault)
	decl_isr	isr_instralign		// 0x5C -3 IRQ_INSTRALIGN (exception 0) instruction fetch was misaligned (alignment)
	decl_isr	isr_mtimer		// 0x60 -2 IRQ_MTIMER (trap 7) machine-mode timer
	decl_isr	isr_softirq		// 0x64 -1 IRQ_SOFTIRQ (trap 3) software IRQ

	// RISC-V external IRQs
	decl_isr	isr_irq0		// 0  IRQ_TIMER0_0	us timer IRQ 0
	decl_isr	isr_irq1		// 1  IRQ_TIMER0_1	us timer IRQ 1
	decl_isr	isr_irq2		// 2  IRQ_TIMER0_2	us timer IRQ 2
	decl_isr	isr_irq3		// 3  IRQ_TIMER0_3	us timer IRQ 3
	decl_isr	isr_irq4		// 4  IRQ_TIMER1_0	sys_clk timer IRQ 0
	decl_isr	isr_irq5		// 5  IRQ_TIMER1_1	sys_clk timer IRQ 1
	decl_isr	isr_irq6		// 6  IRQ_TIMER1_2	sys_clk timer IRQ 2
	decl_isr	isr_irq7		// 7  IRQ_TIMER1_3	sys_clk timer IRQ 3
	decl_isr	isr_irq8		// 8  IRQ_PWM_WRAP_0	PWM wrap 0
	decl_isr	isr_irq9		// 9  IRQ_PWM_WRAP_1	PWM wrap 1
	decl_isr	isr_irq10		// 10 IRQ_DMA_0		DMA IRQ 0
	decl_isr	isr_irq11		// 11 IRQ_DMA_1		DMA IRQ 1
	decl_isr	isr_irq12		// 12 IRQ_DMA_2		DMA IRQ 2
	decl_isr	isr_irq13		// 13 IRQ_DMA_3		DMA IRQ 3
	decl_isr	isr_irq14		// 14 IRQ_USBCTRL	USB ctrl
	decl_isr	isr_irq15		// 15 IRQ_PIO0_0	PIO 0 IRQ 0
	decl_isr	isr_irq16		// 16 IRQ_PIO0_1	PIO 0 IRQ 1
	decl_isr	isr_irq17		// 17 IRQ_PIO1_0	PIO 1 IRQ 0
	decl_isr	isr_irq18		// 18 IRQ_PIO1_1	PIO 1 IRQ 1
	decl_isr	isr_irq19		// 19 IRQ_PIO2_0	PIO 2 IRQ 0
	decl_isr	isr_irq20		// 20 IRQ_PIO2_1	PIO 2 IRQ 1
	decl_isr	isr_irq21		// 21 IRQ_IO_BANK0	pad bank IO
	decl_isr	isr_irq22		// 22 IRQ_IO_BANK0_NS	pad bank IO NS
	decl_isr	isr_irq23		// 23 IRQ_IO_QSPI	qspi bank IO
	decl_isr	isr_irq24		// 24 IRQ_IO_QSPI_NS	qspi bank IO NS
	decl_isr	isr_irq25		// 25 IRQ_SIO_FIFO	SIO FIFO
	decl_isr	isr_irq26		// 26 IRQ_SIO_BELL	SIO BELL
	decl_isr	isr_irq27		// 27 IRQ_SIO_FIFO_NS	SIO FIFO NS
	decl_isr	isr_irq28		// 28 IRQ_SIO_BELL_NS	SIO BELL NS
	decl_isr	isr_irq29		// 29 IRQ_SIO_MTIMECMP	SIO MTIMECMP
	decl_isr	isr_irq30		// 30 IRQ_CLOCKS	clocks
	decl_isr	isr_irq31		// 31 IRQ_SPI0		SPI 0
	decl_isr	isr_irq32		// 32 IRQ_SPI1		SPI 1
	decl_isr	isr_irq33		// 33 IRQ_UART0		UART 0
	decl_isr	isr_irq34		// 34 IRQ_UART1		UART 1
	decl_isr	isr_irq35		// 35 IRQ_ADC_FIFO	ADC FIFO
	decl_isr	isr_irq36		// 36 IRQ_I2C0		I2C 0
	decl_isr	isr_irq37		// 37 IRQ_I2C1		I2C 1
	decl_isr	isr_irq38		// 38 IRQ_OTP		OTP
	decl_isr	isr_irq39		// 39 IRQ_TRNG		TRNG
	decl_isr	isr_irq40		// 40 IRQ_PROC0_CTI
	decl_isr	isr_irq41		// 41 IRQ_PROC1_CTI
	decl_isr	isr_irq42		// 42 IRQ_PLL_SYS	PLL SYS
	decl_isr	isr_irq43		// 43 IRQ_PLL_USB	PLL USB
	decl_isr	isr_irq44		// 44 IRQ_POWMAN_POW	POWMAN POW
	decl_isr	isr_irq45		// 45 IRQ_POWMAN_TIMER	POWMAN Timer
	decl_isr	isr_irq46		// 46 IRQ_SPAREIRQ_0	user IRQ 0
	decl_isr	isr_irq47		// 47 IRQ_SPAREIRQ_1	user IRQ 1
	decl_isr	isr_irq48		// 48 IRQ_SPAREIRQ_2	user IRQ 2
	decl_isr	isr_irq49		// 49 IRQ_SPAREIRQ_3	user IRQ 3
	decl_isr	isr_irq50		// 50 IRQ_SPAREIRQ_4	user IRQ 4
	decl_isr	isr_irq51		// 51 IRQ_SPAREIRQ_5	user IRQ 5

.global isr_hardfault
isr_hardfault:

	// unhandled IRQ
.global unhandled_irq
unhandled_irq:

	// disable interrupts
	csrci	mstatus,0x8




// fatal error stop
.global FatalErrorStop
FatalErrorStop:

	// initialize LED output
	lui	a5,%hi(PADS_BANK0_BASE)		// GPIO_PAD base
	li	a4,0x56				// B1|B2|B4|B6 ... value to reset pad setup
	sw	a4,4+LED_PIN*4(a5)		// reset pad setup GPIO_PAD(LED_PIN)
#if LED_PIN != LED_PIN2
	sw	a4,4+LED_PIN2*4(a5)		// reset pad setup GPIO_PAD(LED_PIN2)
#endif

	// setup pin function
	lui	a5,%hi(IO_BANK0_BASE)		// GPIO_CTRL base
	li	a4,5				// GPIO_FNC_SIO
	sw	a4,4+LED_PIN*8(a5)		// set function GPIO_CTRL(LED_PIN)
#if LED_PIN != LED_PIN2
	sw	a4,4+LED_PIN2*8(a5)		// set function GPIO_CTRL(LED_PIN2)
#endif

	// set pin direction
	lui	a5,%hi(SIO_BASE)		// SIO_BASE pin base
	li	a4,1
#if LED_PIN != LED_PIN2
	slli	a4,a4,LED_PINOFF		// mask value of LED_PIN bit
	addi	a4,a4,1				// mask value of 2nd LED
#endif
	slli	a4,a4,LED_PINBASE		// mask value of LEDs
	sw	a4,0x38(a5)			// set direction of both LEDs GPIO_OE_SET

unhandled_irq_2:
	// flip LED output
	sw	a4,0x28(a5)			// flip output GPIO_OUT_XOR

	// wait loop
	lui	a3,3000000 >> 12		// approximately 3000000
unhandled_irq_3:
	addi	a3,a3,-1			// decrement counter
	bnez	a3,unhandled_irq_3		// wait loop
	j	unhandled_irq_2			// next step


// ==== Reset handler

	.section .reset, "ax"
	.p2align 2,,		// aligned to 4 bytes
	// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - it adds incorrect alignment
	// after the end of the section. If really needed it, use two commas ,, - it will not add 2 false bytes after segment.

.type _reset_handler,%function
.global _reset_handler
_reset_handler:

// initialize global pointer (it points to middle of data, using offsets +-2KB)

.option push
.option norelax
	la	gp,__global_pointer$		// setup global pointer
.option pop

// set stack pointer

	la	sp,__StackTop			// top of the stack

// set vector table address, enable vector mode

	la	a0,__vectors + 1		// vectors, we will enable vector mode
	csrw	mtvec,a0			// setup vector address register MTVEC, using vector mode

// Only core 0 should run the C runtime startup code; core 1 is normally
// sleeping in the bootrom at this point but check to be sure

	csrr	a0,mhartid			// read MHARTID register = hart ID 0 or 1 (= CPU ID 0 or 1)
	bnez	a0,Core1ExitBootrom		// run CPU 1 back to bootrom

#if !NO_FLASH

#if BOOT3_LOADER	// Application has not boot2 loader

// Copy boot2 to the stack

	mv	a3,sp				// A2 <- stack pointer SP, end of buffer (end of destination addres)
	addi	sp,sp,-256			// create space for boot2 in the stack
	mv	a2,sp				// A0 <- buffer in the stack, start of buffer (start of destination address)
	la	a1,__boot2_start__		// A1 <- entry point to boot2 function (start of source address) ... macro of auipc and addi instructions
	jal	data_cpy			// copy boot2

// Execute boot2 in the stack

	jalr	sp				// execute boot2 function
	addi	sp,sp,256			// restore stack pointer

#endif // BOOT3_LOADER

// Copy boot2 and R/O data from Flash to RAM

	la	a4,data_cpy_table		// A4 <- table with data to copy
1:	lw	a1,0(a4)			// A1 <- start of source address in flash
	beqz	a1,2f				// end of loop with mark 0
	lw	a2,4(a4)			// A2 <- start of destination address in RAM
	lw	a3,8(a4)			// A3 <- end of destination address in RAM
	addi	a4,a4,12			// shift table pointer to next entry
	jal	data_cpy			// copy data to RAM
	j	1b				// copy next data
2:

#endif // !NO_FLASH

// Zero out the BSS segment

	la	a1,__bss_start__		// start of BSS segment -> A1 (get from linker script *.ld)
	la	a2,__bss_end__			// end of BSS segment -> A2 (get from linker script *.ld)
	j	bss_fill_test

bss_fill_loop:
	sw	zero,(a1)			// store 0 to (a1)
	addi	a1,a1,4				// increment destination address
bss_fill_test:
	bne	a1,a2,bss_fill_loop		// continue with next data

// Runtime initialize, function RuntimeInit()
// - use 'call' pseudo-instruction instead of 'jal' so that linker can use longer sequence if needed

	call	RuntimeInit

// Start main function Main()

	call	main

// Exit and loop
//  Do not use exit, just simple loop - some programs do not use
//  end loop, but remain running after main() is terminated.

1:	j	1b

// Run CPU 1 exit to BOOTROM
.type Core1ExitBootrom,%function
.global Core1ExitBootrom
Core1ExitBootrom:
	csrci	mstatus,RVCSR_MSTATUS_MIE_BITS	// disable interrupts
	li	a0,BOOTROM_ENTRY_OFFSET		// Boot ROM entry offset for RISC-V
	jr	a0				// jump to Boot ROM

// copy data
#if !NO_FLASH
data_cpy_loop:
	lw	a0,(a1)				// load word from (a1) to a0
	sw	a0,(a2)				// save word to (a2) from a0
	addi	a1,a1,4				// increment source address
	addi	a2,a2,4				// increment destination address

// On data_cpy entry:
//  a0 ... temporary register
//  a1 ... start of source address in Flash
//  a2 ... start of destination address in RAM
//  a3 ... end of destination address in RAM
//  ra ... link register
data_cpy:
	bltu	a2,a3,data_cpy_loop		// loop next step
	ret
#endif

// ==== data copy table (copy R/O data from Flash to RAM)

	.p2align 2,,
	// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - it adds incorrect alignment
	// after the end of the section. If really needed it, use two commas ,, - it will not add 2 false bytes after segment.

data_cpy_table:
#if !NO_FLASH
	// copy boot2 from flash to BOOT RAM
	.word __boot2_start__	// start of source address in Flash
	.word BOOTRAM_BASE	// start of destination address in RAM
	.word BOOTRAM_BASE+256	// end of destination address in RAM

	// copy R/O data from flash to RAM
	.word	__etext		// start of source address in Flash
	.word	__data_start__	// start of destination address in RAM
	.word	__data_end__	// end of destination address in RAM
#endif
	.word	0		// null terminator


// ==== Run application
// Called from boot3 loader

// run application from Flash
.type GoToApp,%function
.global GoToApp
GoToApp:
	lui	a0,%hi(XIP_BASE + BOOTLOADER_SIZE) // vector table of application -> A0
1:	addi	a0,a0,1				// set vector mode
	csrw	mtvec,a0			// setup vector address register MTVEC, using vector mode

	lw	a1,0x300+12-1(a0)		// A1 <- load start stack
	lw	a0,0x300+16-1(a0)		// A0 <- load start address
	mv	sp,a1				// set stack pointer
	jr	a0				// run application code

// run application from RAM
.type GoToAppRam,%function
.global GoToAppRam
GoToAppRam:
	lui	a0,%hi(SRAM_BASE)		// vector table of application -> A0
	j	1b	

// ==== Clear IRQ registers
// Enable interrupts globally, but disable all interrupt sources (should be called on start of every core)

	.section .text
.global RuntimeInitCoreIRQ
RuntimeInitCoreIRQ:

	// clear all IRQ force array bits 0 to 3 (clears IRQ 0..63 in 4 windows 0..3)
	li	a0,3
1:	csrw	RVCSR_MEIFA_OFFSET,a0	// select IRQ window A0 and clear all 16 bits in this window
	addi	a0,a0,-1		// decrement window index register
	bgez	a0,1b			// next IRQ window

	// enable external interrupt, disable timer interrupt, disable software interrupt
	li	a0,RVCSR_MIE_MEIE_BITS	// MIE.MEIE (bit 11)
	csrw	mie,a0			// setup interrupt enable register

	// enable global interrupt
	csrsi	mstatus,RVCSR_MSTATUS_MIE_BITS	// set interrupt enable (set mstatus.mie flag)

	// clear MSCRATCH regiser, which is used to detect nested exceptions
	csrw	mscratch,zero
	ret


// ==== CPU 0 stack

	.section .stack0
	.p2align 5,,
	// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - it adds incorrect alignment
	// after the end of the section. If really needed it, use two commas ,, - it will not add 2 false bytes after segment.
	.equ	StackSize,PICO_STACK_SIZE
	.space	StackSize

// ==== Heap

	.section .heap
	.p2align 2,,
	// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - it adds incorrect alignment
	// after the end of the section. If really needed it, use two commas ,, - it will not add 2 false bytes after segment.
	.equ	HeapSize,PICO_HEAP_SIZE
	.space	HeapSize
