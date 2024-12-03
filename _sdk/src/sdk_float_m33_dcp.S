
// ****************************************************************************
//
//                      DCP Double-precision Coprocessor, float
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040

/*
 * Copyright (c) 2024 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../asm_include.inc"
#include "../asm_dcp.inc"

	.syntax unified
	.cpu cortex-m33
	.fpu fpv5-sp-d16
	.thumb

#if USE_FLOAT		// use float support 1=in RAM, 2=in Flash

#if USE_FLOAT == 1
#define float_section section_noflash
#else
#define float_section section_text
#endif

.macro float_wrapper_section func
float_section WRAPPER_FUNC_NAME(\func)
.endm

#if !USE_FLOATLIBC

// ============== STATE SAVE AND RESTORE ===============

.macro saving_func func
  // Note we are usually 32-bit aligned already at this point, as most of the
  // function bodies contain exactly two 16-bit instructions: bmi and bx lr.
  // We want the PCMP word-aligned.
.p2align 2,,
	// On riscv32-unknown-elf-gcc-13.2.0 this alignment works incorrectly - it adds incorrect alignment
	// after the end of the section. If really needed it, use two commas ,, - it will not add 2 false bytes after segment.
  // When the engaged flag is set, branch back here to invoke save routine and
  // hook lr with the restore routine, then fall back through to the entry
  // point. The engaged flag will be clear when checked a second time.
1:
  push {lr}              // 16-bit instruction
  bl generic_save_state  // 32-bit instruction
  b 1f                   // 16-bit instruction
  // This is the actual entry point:
wrapper_func \func
  PCMP apsr_nzcv
  bmi 1b
1:
.endm

.macro saving_func2 func func2
.p2align 2,,
1:
  push {lr}
  bl generic_save_state
  b 1f
wrapper_func \func
regular_func \func2
  PCMP apsr_nzcv
  bmi 1b
1:
.endm

.macro saving_func3 func func2 func3
.p2align 2,,
1:
  push {lr}
  bl generic_save_state
  b 1f
wrapper_func \func
regular_func \func2
regular_func \func3
  PCMP apsr_nzcv
  bmi 1b
1:
.endm

.macro saving_func_return
  bx lr
.endm

float_section __rp2350_dcp_engaged_state_save_restore
.thumb_func
generic_save_state:
  sub sp, #24
  push {r0, r1}
  // do save here
  PXMD r0, r1
  strd r0, r1, [sp, #8 + 0]
  PYMD r0, r1
  strd r0, r1, [sp, #8 + 8]
  REFD r0, r1
  strd r0, r1, [sp, #8 + 16]
  pop {r0, r1}
  blx lr
  // <- wrapped function returns here
  // fall through into restore:
.thumb_func
generic_restore_state:
  // do restore here
  pop {r12, r14}
  WXMD r12, r14
  pop {r12, r14}
  WYMD r12, r14
  pop {r12, r14}
  WEFD r12, r14
  pop {pc}

// ============== ARITHMETIC FUNCTIONS ===============

#if !USE_M33_VFP	// 1=enable VFP floating point instructions on RP2350-ARM, 0=emulate with DCP coprocessor

float_wrapper_section __aeabi_fadd
saving_func2 __aeabi_fadd fadd
  dcp_fadd_m r0,r0,r1
  saving_func_return

float_wrapper_section __aeabi_fsub
saving_func2 __aeabi_fsub fsub
  dcp_fsub_m r0,r0,r1
  saving_func_return

float_wrapper_section __aeabi_frsub
saving_func __aeabi_frsub
  dcp_fsub_m r0,r1,r0
  saving_func_return

float_wrapper_section __aeabi_fmul
saving_func2 __aeabi_fmul fmul
  dcp_fmul_m r0,r0,r1,r0,r1
  saving_func_return

/*
float_section fdiv_fast
saving_func2 fdiv_fast fdiv_fast
  dcp_fdiv_fast_m r0,r0,r1,r0,r1,r2
  saving_func_return

float_wrapper_section __aeabi_fdiv
saving_func2 __aeabi_fdiv fdiv
@ with correct rounding
  dcp_fdiv_m r0,r0,r1,r0,r1,r2,r3
  saving_func_return
*/

#if USE_M33_FDIV == 1		// RP2350-ARM fdiv() function: 1=fast (recommended), 2=precise

float_wrapper_section __aeabi_fdiv
saving_func2 __aeabi_fdiv fdiv
  dcp_fdiv_fast_m r0,r0,r1,r0,r1,r2
  saving_func_return

#else

float_wrapper_section __aeabi_fdiv
saving_func2 __aeabi_fdiv fdiv
  dcp_fdiv_m r0,r0,r1,r0,r1,r2,r3
  saving_func_return

#endif

#endif // !USE_M33_VFP

/*
float_section sqrtf_fast
saving_func2 sqrtf_fast sqrtf_fast
  dcp_fsqrt_fast_m r0,r0,r0,r1,r2,r3
  saving_func_return

float_wrapper_section sqrtf
saving_func sqrtf
@ with correct rounding
  dcp_fsqrt_m r0,r0,r0,r1,r2,r3
  saving_func_return
*/

#if USE_M33_SQRTF == 1		// RP2350-ARM sqrtf() function: 1=fast (recommended), 2=precise

float_wrapper_section sqrtf
saving_func sqrtf
  dcp_fsqrt_fast_m r0,r0,r0,r1,r2,r3
  saving_func_return

#else

float_wrapper_section sqrtf
saving_func sqrtf
  dcp_fsqrt_m r0,r0,r0,r1,r2,r3
  saving_func_return

#endif

// todo not a real thing
float_wrapper_section __aeabi_fclassify
saving_func __aeabi_fclassify
  dcp_fclassify_m apsr_nzcv,r0
  saving_func_return

// ============== CONVERSION FUNCTIONS ===============

float_wrapper_section __aeabi_f2d
saving_func3 __aeabi_f2d f2d float2double
  dcp_float2double_m r0,r1,r0
  saving_func_return

float_wrapper_section __aeabi_i2f
saving_func3 __aeabi_i2f i2f int2float
@ with rounding
  dcp_int2float_m r0,r0
  saving_func_return

float_wrapper_section __aeabi_ui2f
saving_func3 __aeabi_ui2f ui2f uint2float
@ with rounding
  dcp_uint2float_m r0,r0
  saving_func_return

float_wrapper_section __aeabi_f2iz
saving_func3 __aeabi_f2iz f2iz float2int_z
@ with truncation towards 0
  dcp_float2int_m r0,r0
  saving_func_return

float_wrapper_section __aeabi_f2uiz
saving_func3 __aeabi_f2uiz f2ui float2uint
@ with truncation towards 0
  dcp_float2uint_m r0,r0
  saving_func_return

// todo not a real thing
float_wrapper_section __aeabi_f2i_r
saving_func __aeabi_f2i_r
@ with rounding
  dcp_float2int_r_m r0,r0
  saving_func_return

// todo not a real thing
float_wrapper_section __aeabi_f2ui_r
saving_func __aeabi_f2ui_r
@ with rounding
  dcp_float2uint_r_m r0,r0
  saving_func_return

// ============== COMPARISON FUNCTIONS ===============

float_wrapper_section __aeabi_fcmpun
saving_func2 __aeabi_fcmpun fcmpun
  dcp_fcmp_m r0,r0,r1
  // extract unordered bit
  ubfx r0, r0, #28, #1
  saving_func_return

float_wrapper_section __aeabi_fcmp
saving_func __aeabi_cfrcmple
  dcp_fcmp_m apsr_nzcv,r1,r0 // with arguments reversed
  bvs cmp_nan
  saving_func_return

// these next two can be the same function in the absence of exceptions
saving_func __aeabi_cfcmple
  dcp_fcmp_m apsr_nzcv,r0,r1
  bvs cmp_nan
  saving_func_return

// It is not clear from the ABI documentation whether cfcmpeq must set the C flag
// in the same way as cfcmple. If not, we could save the "bvs" below; but we
// err on the side of caution.
saving_func __aeabi_cfcmpeq
  dcp_fcmp_m apsr_nzcv,r0,r1
  bvs cmp_nan
  saving_func_return

// If the result of a flag-setting comparison is "unordered" then we need to set C and clear Z.
// We could conceivably just do lsrs r12,r14,#1, or even cmp r14,r14,lsr#1 as (a) r14 here is a
// return address and r14b0=1 for Thumb mode; (b) we are unlikely to be returning to address 0.
cmp_nan:
  movs r12, #3 // r12 does not need to be preserved by the flag-setting comparisons
  lsrs r12, #1 // set C, clear Z
  saving_func_return

float_wrapper_section __aeabi_fcmpeq
saving_func2 __aeabi_fcmpeq fcmpeq
  dcp_fcmp_m r0,r0,r1
  // extract Z
  ubfx r0, r0, #30, #1
  saving_func_return

float_wrapper_section __aeabi_fcmplt
saving_func2 __aeabi_fcmplt fcmplt
  dcp_fcmp_m apsr_nzcv,r1,r0
  ite hi
  movhi r0,#1
  movls r0,#0
  saving_func_return

float_wrapper_section __aeabi_fcmple
saving_func2 __aeabi_fcmple fcmple
  dcp_fcmp_m apsr_nzcv,r1,r0
  ite hs
  movhs r0,#1
  movlo r0,#0
  saving_func_return

float_wrapper_section __aeabi_fcmpge
saving_func2 __aeabi_fcmpge fcmpge
  dcp_fcmp_m apsr_nzcv,r0,r1
  ite hs
  movhs r0,#1
  movlo r0,#0
  saving_func_return

float_wrapper_section __aeabi_fcmpgt
saving_func2 __aeabi_fcmpgt fcmpgt
  dcp_fcmp_m apsr_nzcv,r0,r1
  ite hi
  movhi r0,#1
  movls r0,#0
  saving_func_return

#endif // !USE_FLOATLIBC

#endif // USE_FLOAT		// use float support
