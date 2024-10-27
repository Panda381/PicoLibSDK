
// ****************************************************************************
//
//                           Assembler includes
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

// Copyright (c) 2020 Mark Owen https://www.quinapalus.com - GPLv2 licence
// Copyright (c) 2020 Raspberry Pi (Trading) Ltd. - SPDX-License-Identifier: BSD-3-Clause

#include "config.h"			// project configuration
#include "sdk_addressmap.h"
#include "sdk_sftable.h"

// original-SDK define unsigned number
#ifndef _u
#define _u(x) x
#endif

#define WRAPPER_FUNC_NAME(x) __wrap_##x		// wrapper function (to replace standard library function)

// macro - .text.* section
.macro section_text x
.section .text.\x	// located in separate section .text.NAME
.align 2,,		// align address to 4-byte boundary
.endm

// macro - .time_critical.* section
.macro section_noflash x
.section .time_critical.\x // located in separate section .time_critical.NAME
.align 2,,		// align address to 4-byte boundary
.endm

// macro - .text.* data section
.macro regular_data_text x
section_text \x		// located in separate section .text.NAME
.align 2,,		// align address to 4-byte boundary (required by 32-bit numbers)
\x:			// label
.endm

// macro - .time_critical.* data section
.macro regular_data_noflash x
section_noflash \x	// located in separate section .time_critical.NAME
.align 2,,		// align address to 4-byte boundary (required by 32-bit numbers)
\x:			// label
.endm

// macro - header of regular function 'x'
.macro regular_func x
.global \x		// global publishing function
.type \x,%function	// declare label as a function object
#if !RISCV
.thumb_func		// following label is a thumb-mode label
#endif
\x:			// label of the function
.endm

// macro - header of regular function 'x' in .text.* section
.macro regular_func_text x
section_text \x		// located in separate section .text.NAME
regular_func \x
.endm

// macro - header of regular function 'x' in .time_critical.* section
.macro regular_func_noflash x
section_noflash \x		// located in separate section .time_critical.NAME
regular_func \x
.endm

// macro - header of wrapper function 'x' (to replace standard library function)
.macro wrapper_func x
regular_func WRAPPER_FUNC_NAME(\x) // wrapper function __wrap_NAME
.endm

// macro - header of wrapper function 'x' in .text.* section
.macro wrapper_func_text x
regular_func_text WRAPPER_FUNC_NAME(\x) // wrapper function __wrap_NAME
.endm

// macro - header of wrapper function 'x' in .time_critical.* section
.macro wrapper_func_noflash x
regular_func_noflash WRAPPER_FUNC_NAME(\x) // wrapper function __wrap_NAME
.endm

// macro - delay 2 clock cycles (= 2*NOP)
.macro nop2
	b	nop2_\@	// [2]
nop2_\@:
.endm

// macro - delay 8 clock cycles
.macro nop8
	nop2	// [2]
	nop2	// [2]
	nop2	// [2]
	nop2	// [2]
.endm

// macro - delay 6 clock cycles
.macro nop6
	nop2	// [2]
	nop2	// [2]
	nop2	// [2]
.endm

// macro - delay 4 clock cycles
.macro nop4
	nop2	// [2]
	nop2	// [2]
.endm

// macro - double negate s64 number in 2 registers
.macro dneg lo,hi
	mvns	\hi,\hi		// NOT high register
	negs	\lo,\lo		// negate low register
	bne	dneg_\@		// result is not 0, no carry to high register
	adds	\hi,#1		// carry to high register
dneg_\@:
.endm

// macro - save interrupt into register, disable interrupt
.macro irq_lock r
	mrs	\r,PRIMASK		// save control register
	cpsid	i			// disable interrupt
.endm

// macro - restore interrupt from register
.macro irq_unlock r
	msr	PRIMASK,\r		// restore control register
.endm

// macro - pre-initialize function (called on program start)
//.macro __pre_init func, priority	// function name and priority string
//.section .preinit_array.\priority	// priority (array will be sorted alphabetically)
//.align 2,,				// align function address to u32
//.word \func				// pointer to pre-initialize function
//.endm

// hardware divider
.equ DIV_PORT     ,0xd0000000
.equ DIV_UDIVIDEND,0x00000060
.equ DIV_UDIVISOR ,0x00000064
.equ DIV_QUOTIENT ,0x00000070
.equ DIV_REMAINDER,0x00000074
.equ DIV_CSR      ,0x00000078

// macro - unsigned multiply u32 * u32 -> u64
//  input: RX = u32 x
//         RY = u32 y
// output: RZH:RZL = u64 z (result)
// trashes: RT0, RT1, RT2
// - can have RZL=RX, in which case RX trashed.
// - can have RZH=RY, in which case RY trashed.
.macro mul32_32_64 rx,ry,rzl,rzh,rt0,rt1,rt2
				//   t0   t1   t2   rzl  rzh
				//                  (x)  (y)
	// multipy XL*YL -> L (RT0)
	uxth	\rt0,\rx	//   xl                      ... load 16 low bits of X to RT0
	uxth	\rt1,\ry	//        yl                 ... load 16 low bits of Y to RT1
	muls	\rt0,\rt1	// xlyl=L                    ... multiply low bits X and low bits Y -> low member L

	// multiply XH*YL -> M0 (RT1)
	lsrs	\rt2,\rx,#16	//             xh            ... load 16 high bits of X to RT2
	muls	\rt1,\rt2	//      xhyl=M0              ... multiply high bits X and low bits Y -> middle member M0

	// multiply XH*YH -> H (RT2)
	lsrs	\rzh,\ry,#16	//                        yh ... load 16 high bits of Y to RZH
	muls	\rt2,\rzh	//           xhyh=H          ... multiply high bits X and high bits Y -> high member H

	// multiply XL*YH -> M1 (RZL)
	uxth	\rzl,\rx	//                   xl      ... load 16 low bits of X to RZL
	muls	\rzl,\rzh	//                 xlyh=M1   ... multiply low bits X and high bits Y -> middle member M1

	// sum middle members M0 + M1 -> M (RT1)
	adds	\rt1,\rzl	//      M0+M1=M              ... sum middle members
	bcc	l\@_1		// no carry

	// add carry from middle members to high member
	movs	\rzl,#1		// RZL <- 1
	lsls	\rzl,#16	// RZL <- 0x10000
	adds	\rt2,\rzl	// add carry to high member
l\@_1:
	// add middle member to low and high members
	lsls	\rzl,\rt1,#16	// middle member low -> RZL
	lsrs	\rzh,\rt1,#16	// middle member high -> RZH
	adds	\rzl,\rt0	// sum middle member low and low member
	adcs	\rzh,\rt2	// sum middle member high and high member
.endm

// macro - multiply s32 * u32 -> s64
//  input: RX = s32 x
//         RY = u32 y
// output: RZH:RZL = s64 z (result)
// trashes: RT0, RT1, RT2
// - can have RZL=RX, in which case RX trashed.
// - can have RZH=RY, in which case RY trashed.
.macro muls32_32_64 rx,ry,rzl,rzh,rt0,rt1,rt2
				//   t0   t1   t2   rzl  rzh
				//                  (x)  (y)
	// multipy XL*YL -> L (RT0)
	uxth	\rt0,\rx	//   xl                      ... load 16 low bits of X to RT0
	uxth	\rt1,\ry	//        yl                 ... load 16 low bits of Y to RT1
	muls	\rt0,\rt1	// xlyl=L                    ... multiply low bits X and low bits Y -> low member L

	// multiply XH*YL -> M0 (RT1)
	asrs	\rt2,\rx,#16	//             xh            ... load 16 high bits of X to RT2
	muls	\rt1,\rt2	//      xhyl=M0              ... multiply high bits X and low bits Y -> middle member M0

	// multiply XH*YH -> H (RT2)
	lsrs	\rzh,\ry,#16	//                        yh ... load 16 high bits of Y to RZH
	muls	\rt2,\rzh	//           xhyh=H          ... multiply high bits X and high bits Y -> high member H

	// multiply XL*YH -> M1 (RZL)
	uxth	\rzl,\rx	//                   xl      ... load 16 low bits of X to RZL
	muls	\rzl,\rzh	//                 xlyh=M1   ... multiply low bits X and high bits Y -> middle member M1

	// sum middle members M0 + M1 -> M (RT1)
	asrs	\rzh,\rt1,#31	// sign extension high of M0 -> RZH
	adds	\rt1,\rzl	//      M0+M1=M ... sum middle members

	// add carry from middle members to high member
	movs	\rzl,#0		// RZL <- 0
	adcs	\rzh,\rzl	// add carry to sign extension high of M0
	lsls	\rzh,#16	// shift carry from middle member to position 16
	adds	\rt2,\rzh	// add carry from middle member to high member

	// add middle member to low and high members
	lsls	\rzl,\rt1,#16	// middle member low -> RZL
	lsrs	\rzh,\rt1,#16	// middle member high -> RZH
	adds	\rzl,\rt0	// sum middle member low and low member
	adcs	\rzh,\rt2	// sum middle member high and high member
.endm

// macro - signed multiply s32 * s32 -> s64
//  input: RX = s32 x
//         RY = s32 y
// output: RZH:RZL = s64 z (result)
// trashes: RT0, RT1, RT2
// - can have RZL=RX, in which case RX trashed.
// - can have RZH=RY, in which case RY trashed.
.macro muls32_s32_64 rx,ry,rzl,rzh,rt0,rt1,rt2
				//   t0   t1   t2   rzl  rzh
				//                  (x)  (y)
	// multipy XL*YL -> L (RT0)
	uxth	\rt0,\rx	//   xl                      ... load 16 low bits of X to RT0
	uxth	\rt1,\ry	//        yl                 ... load 16 low bits of Y to RT1
	muls	\rt0,\rt1	// xlyl=L                    ... multiply low bits X and low bits Y -> low member L

	// multiply XH*YL -> M0 (RT1)
	asrs	\rt2,\rx,#16	//             xh            ... load 16 high bits of X to RT2
	muls	\rt1,\rt2	//      xhyl=M0              ... multiply high bits X and low bits Y -> middle member M0

	// multiply XH*YH -> H (RT2)
	asrs	\rzh,\ry,#16	//                        yh ... load 16 high bits of Y to RZH
	muls	\rt2,\rzh	//           xhyh=H          ... multiply high bits X and high bits Y -> high member H

	// multiply XL*YH -> M1 (RZL)
	uxth	\rzl,\rx	//                   xl      ... load 16 low bits of X to RZL
	muls	\rzl,\rzh	//                 xlyh=M1   ... multiply low bits X and high bits Y -> middle member M1

	// sum middle members M0 + M1 -> M (RT1)
	adds	\rt1,\rzl	//     M0+M1=M ... sum middle members
	asrs	\rzl,\rt1,#31	// sign extension high of M
	bvc	l\@_1		// no overflow
	mvns	\rzl,\rzl	// flip sign extension bits if overflow

l\@_1:
	// add carry from middle members to high member
	lsls	\rzl,#16	// shift carry from middle member to position 16
	adds	\rt2,\rzl	// add carry from middle member to high member

	// add middle member to low and high members
	lsls	\rzl,\rt1,#16	// middle member low -> RZL
	lsrs	\rzh,\rt1,#16	// middle member high -> RZH
	adds	\rzl,\rt0	// sum middle member low and low member
	adcs	\rzh,\rt2	// sum middle member high and high member
.endm

// macro - square u32 -> u64
//  input: RX = u32 x
// output: RZH:RZL = u64 z (result)
// trashes: RT0, RT1
// - can have RZH==RX or RZL==RX
.macro square32_64 rx,rzl,rzh,rt0,rt1
				//   t0   t1   zh   zl
	// multiply XL*XL -> L (RT0)
	uxth	\rt0,\rx	//   xl                ... load 16 low bits of X to RT0
	muls	\rt0,\rt0	// xlxl=L              ... multiply low bits X and low bits X -> low member L

	// multiply XL*XH -> M (RZH)
	uxth	\rzh,\rx	//        xl           ... load 16 low bits of X to RZH
	lsrs	\rt1,\rx,#16	//             xh      ... load 16 high bits of X to RT1
	muls	\rzh,\rt1	//      xlxh=M         ... multiply low bits X and high bits X -> middle member M

	// multiply XH*XH -> H (RT1)
	muls	\rt1,\rt1	//           xhxh=H    ... multiply high bits X and high bits X -> high member H

	// add 2*M to low and high members
	lsls	\rzl,\rzh,#17	// middle member low * 2 -> RZL
	lsrs	\rzh,\rzh,#15	// middle member high * 2 -> RZH
	adds	\rzl,\rt0	// sum middle member low and low member
	adcs	\rzh,\rt1	// sum middle member high and high member
.endm

// macro - unsigned multiply u32 * u32 -> (u64>>32), simplified, without carry from LOW
//  input: RX = u32 x
//         RY = u32 y
// output: RZ = u32 zH (result high)
// trashes: RT1, RT2
// - can have RZ=RX, in which case RX trashed.
.macro mul32_32_32_highsimple rx,ry,rz,rt1,rt2

	// multiply XL*YH
	uxth	\rt2,\rx	// RT2 <- XL
	lsrs	\rt1,\ry,#16	// RT1 <- YH
	muls	\rt2,\rt1	// RT2 <- XL*YH
	lsrs	\rt2,\rt2,#16	// RT2 >> 16

	// multiply XH*YH
	lsrs	\rz,\rx,#16	// RZ <- XH, can be the same register
	muls	\rt1,\rz	// RT1 <- YH*XH
	adds	\rt2,\rt1	// add XH*YH to result

	// multiply XH*YL
	uxth	\ry,\ry		// RY <- YL
	muls	\rz,\ry		// RZ <- XH*YL
	lsrs	\rz,\rz,#16	// RZ >> 16
	adds	\rz,\rt2	// add XH*YL to result
.endm


#if RISCV

// Feature detection macros for Hazard3 custom extensions
// Version 1.0 of these four extensions
// (encoded as major * 100 + minor)
#define __hazard3_extension_xh3power 100
#define __hazard3_extension_xh3bextm 100
#define __hazard3_extension_xh3irq 100
#define __hazard3_extension_xh3pmpm 100

// Assembly language instruction macros for Hazard3 custom instructions

// h3.bextm: Extract up to 8 consecutive bits from register rs1, with the
// first bit indexed by rs2, and bit count configured by an immediate value.
// R-format instruction. Pseudocode:
//
//     rd = (rs1 >> rs2[4:0]) & ~(-1 << nbits)
//
// rd ... destination register
// rs1 ... source register
// rs2 ... register with start bit position
// nbits ... number of bits to extract

.macro h3.bextm rd rs1 rs2 nbits
.if (\nbits < 1) || (\nbits > 8)
.err
.endif
#ifdef __hazard3_extension_xh3bextm
	.insn	r 0x0b, 0x4, (((\nbits - 1) & 0x7 ) << 1), \rd, \rs1, \rs2
#else
	srl	\rd,\rs1,\rs2			// RD <- RS1 >> RS2 ... get bits from RS1, starting with bit RS2
	andi	\rd,\rd,((1 << \nbits) - 1)	// mask required number of bits
#endif
.endm

// h3.bextmi: Extract up to 8 consecutive bits from register rs1, with the
// first bit index and the number of bits both configured by immediate
// values. I-format instruction. Pseudocode:
//
//     rd = (rs1 >> shamt) & ~(-1 << nbits)
//
// rd ... destination register
// rs1 ... source register
// shamt ... start bit position
// nbits ... number of bits to extract

.macro h3.bextmi rd rs1 shamt nbits
.if (\nbits < 1) || (\nbits > 8)
.err
.endif
.if (\shamt < 0) || (\shamt > 31)
.err
.endif
#ifdef __hazard3_extension_xh3bextm
	.insn	i 0x0b, 0x4, \rd, \rs1, (\shamt & 0x1f) | (((\nbits - 1) & 0x7 ) << 6)
#else
	srli	\rd,\rs1,\shamt			// RD <- RS1 >> shamt ... get bits from RS1, starting with bit shamt
	andi	\rd,\rd,((1 << \nbits) - 1)	// mask required number of bits
#endif
.endm

// h3.block: enter an idle state until another processor in the same
// multiprocessor complex executes an h3.unblock instruction, or the
// processor is interrupted. Fall through immediately if an h3.unblock has
// been received since the last execution of an h3.block on this processor.
// On RP2350, processors also have their own h3.unblock signals reflected
// back to them.

.macro h3.block
#ifdef __hazard3_extension_xh3power
	slt	x0,x0,x0
#else
	nop
#endif
.endm

// h3.unblock: signal other processors in the same multiprocessor complex to
// exit the idle state entered by an h3.block instruction. On RP2350, this
// signal is also reflected back to the processor that executed the
// h3.unblock, which will cause that processor's next h3.block to fall
// through immediately.

.macro h3.unblock
#ifdef __hazard3_extension_xh3power
	slt	x0,x0,x1
#else
	nop
#endif
.endm

#endif
