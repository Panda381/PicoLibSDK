
// ****************************************************************************
//
//                               Interpolator
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

// Each CPU core has its own 2 interpolators.

/*
Lane 0 and 1: Value from accumulator is right shifted, masked and added to base.
Both results from Lane 0 and 1 can added to base 2 (without base 0 and base 1) to full result.
Results can be written back to the accumulators.
*/

#if USE_INTERP	// use interpolator (sdk_interp.c, sdk_interp.h)

#ifndef _SDK_INTERP_H
#define _SDK_INTERP_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "sdk_sio.h"			// SIO registers

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_sio.h"		// constants of original SDK
#else
#include "orig_rp2350/orig_sio.h"		// constants of original SDK
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define INTERP_NUM	2		// number of interpolator per CPU cure
#define INTERP_LANE_NUM	2		// number of lanes per interpolator

// Interpolator hardware registers
// interp = 0 or 1 (index of interpolator), lane = 0 or 1 (index of lane) or 2 for common value
//    (SIO does not support aliases for atomic access!)
#define INTERP_ACCUM(interp, lane)	((volatile u32*)(SIO_BASE+(interp)*0x40+(lane)*4+0x80)) // R/W accumulator (lane = 0 or 1)
#define INTERP_BASE(interp, lane)	((volatile u32*)(SIO_BASE+(interp)*0x40+(lane)*4+0x88)) // R/W base (lane = 0 or 1, or 2 to common base)
#define INTERP_POP(interp, lane)	((volatile u32*)(SIO_BASE+(interp)*0x40+(lane)*4+0x94)) // RO read lane result and write to accumulators (lane = 0 or 1, or 2 for full result)
#define INTERP_PEEK(interp, lane)	((volatile u32*)(SIO_BASE+(interp)*0x40+(lane)*4+0xA0)) // RO read lane result without altering state (lane = 0 or 1, or 2 for full result)
#define INTERP_CTRL(interp, lane)	((volatile u32*)(SIO_BASE+(interp)*0x40+(lane)*4+0xAC)) // control register (lane = 0 or 1)
#define INTERP_ADD(interp, lane)	((volatile u32*)(SIO_BASE+(interp)*0x40+(lane)*4+0xB4)) // R/W add to accumulator (lane = 0 or 1)
#define INTERP_BASE01(interp)		((volatile u32*)(SIO_BASE+(interp)*0x40+0xBC)) // WO write LOW 16 bits to BASE0, HIGH 16 bits to BASE1

/* ... this structure is defined in sdk_sio.h
// Interpolator hardware interface
typedef struct {
	io32	accum[2];	// 0x00: Read/write access to accumulator 0
	io32	base[3];	// 0x08: Read/write access to BASE0 register
	io32	pop[3];		// 0x14: Read LANE0 result, and simultaneously write lane results to both accumulators (POP)
	io32	peek[3];	// 0x20: Read LANE0 result, without altering any internal state (PEEK)
	io32	ctrl[2];	// 0x2C: Control register for lane 0
	io32	add_raw[2];	// 0x34: Values written here are atomically added to ACCUM0 or ACCUM1
	io32	base01;		// 0x3C: On write, the lower 16 bits go to BASE0, upper bits to BASE1 simultaneously
} interp_hw_t;

STATIC_ASSERT(sizeof(interp_hw_t) == 0x40, "Incorrect interp_hw_t!");

#define interp_hw_array ((interp_hw_t*)(SIO_BASE + 0x80))
#define interp0_hw (&interp_hw_array[0])
#define interp1_hw (&interp_hw_array[1])
*/

#define interp0 interp0_hw
#define interp1 interp1_hw

// save interpolator state (size 7*4=28 bytes)
typedef struct {
	u32	accum[2];	// accumulators
	u32	base[3];	// bases
	u32	ctrl[2];	// controls
} interp_hw_save_t;

#define INTERP_DEFCTRL	(31<<10)	// default control word (MASK_MSB = 31)

// claimed interpolator lanes
extern u8 InterpClaimed;

// get hardware interface from interpolator index
INLINE interp_hw_t* InterpGetHw(int interp) { return (interp == 0) ? interp0_hw : interp1_hw; }

// get interpolator index from hardware interface
INLINE u8 InterpGetInx(const interp_hw_t* hw) { return (hw == interp0_hw) ? 0 : 1; }

// === claim interpolator lane
// Functions are not atomic safe! (not recommended to be used in both cores or in IRQ at the same time)

// claim interpolator lane (mark it as used)
INLINE void InterpClaim(int interp, int lane) { InterpClaimed |= BIT(interp*2+lane); }

// claim interpolator lanes with mask (mark them as used)
INLINE void InterpClaimMask(int interp, int mask) { InterpClaimed |= mask << (interp*2); }

// unclaim interpolator lane (mark it as not used)
INLINE void InterpUnclaim(int interp, int lane) { InterpClaimed &= ~BIT(interp*2+lane); }

// unclaim interpolator lanes with mask (mark them as not used)
INLINE void InterpUnclaimMask(int interp, int mask) { InterpClaimed &= ~(mask << (interp*2)); }

// check if interpolator lane is claimed
INLINE Bool InterpIsClaimed(int interp, int lane) { return (InterpClaimed & BIT(interp*2+lane)) != 0; }

// === interpolator configuration structure (u32 control word)
// After setup, write configuration word using InterpCfg() function.

// get default configuration word (to setup, write configuration word to control register)
INLINE u32 InterpCfgDef() { return INTERP_DEFCTRL; }

// config set to configuration word (0 is default state)
INLINE void InterpCfg(int interp, int lane, u32 cfg) { *INTERP_CTRL(interp, lane) = cfg; }
INLINE void InterpCfg_hw(interp_hw_t* hw, int lane, u32 cfg) { hw->ctrl[lane] = cfg; }

// config set interpolator shift (right-shift applied to accumulator before masking) (to setup, write configuration word to control register)
//  shift ... shift value 0 to 31
INLINE void InterpCfgShift(u32* cfg, int shift) { *cfg = (*cfg & ~0x1f) | shift; }

// config set interpolator mask range (range of bits that are allowed to pass) (to setup, write configuration word to control register)
//  lsb ... least significant bit allowed to pass, inclusive, 0 to 31
//  msb ... most significant bit allowed to pass, inclusive, 0 to 31 (lsb <= msb)
INLINE void InterpCfgMask(u32* cfg, int lsb, int msb)
	{ *cfg = (*cfg & ~((0x1f<<5)|(0x1f<<10))) | ((u32)lsb<<5) | ((u32)msb<<10); }

// config enable cross input (feed opposite lane's accumulator into this lane's shift+mask input) (to setup, write configuration word to control register)
//  en ... True = feed opposite lane's accumulator to this input, False = use input from own accumulator
INLINE void InterpCfgCrossInput(u32* cfg, Bool en) { if (en) *cfg |= B16; else *cfg &= ~B16; }

// config enable cross results (feed opposite lane's result into this lane's accumulator on POP) (to setup, write configuration word to control register)
//  en ... True = feed opposite lane's result to this accumulator, False = no auto-feed from result
INLINE void InterpCfgCrossResult(u32* cfg, Bool en) { if (en) *cfg |= B17; else *cfg &= ~B17; }

// config set interpolator signed mode (values are sign-extended to 32 bits) (to setup, write configuration word to control register)
INLINE void InterpCfgSigned(u32* cfg) { *cfg |= B15; }

// config set interpolator unsigned mode (default state) (to setup, write configuration word to control register)
INLINE void InterpCfgUnsigned(u32* cfg) { *cfg &= ~B15; }

// config set raw add option (mask+shift is bypassed for this result, add raw input value (does not affect FULL result)) (to setup, write configuration word to control register)
//  en ... True = shift+mask is bypassed, False = shif+mask is applied
INLINE void InterpCfgAddRaw(u32* cfg, Bool en) { if (en) *cfg |= B18; else *cfg &= ~B18; }

// config set blending mode (only on interpolator 0 of each CPU core) (to setup, write configuration word to control register)
//  en ... True = LANE1 result is linear interpolation between BASE0 and BASE1,
//         controlled by 8 LSB bits of LANE1 shift+mask value (fractional number 0 to 255 / 256),
//         LANE0 result does not have BASE0 added (yields only 8 LSB of LANE1 shift+mask),
//         FULL result does not have lane 1 shift+mask value addes (BASE2 + lane0 shift+mask),
// LANE1 SIGNED flag controls whether interpolation is signed or unsigned
INLINE void InterpCfgBlend(u32* cfg, Bool en) { if (en) *cfg |= B21; else *cfg &= ~B21; }

// config set clamping mode (only on interpolator 1 of each CPU core) (to setup, write configuration word to control register)
//  en ... True = LANE0 result is shifted and masked ACCUM0, clamped by
//         lower bound of BASE0 and upper bound of BASE1.
// LANE0 SIGNED flag controls whether comparisons are is signed or unsigned
INLINE void InterpCfgClamp(u32* cfg, Bool en) { if (en) *cfg |= B22; else *cfg &= ~B22; }

// config set forced bits (Two data bits ORed into bits 29:28 of lane result presented to the processor) (no effect on internal paths) (to setup, write configuration word to control register)
// - handy for using lane to generate sequence of pointers into flash or SRAM.
//  bits ... two bits to be ORed into bits 29:28, value 0 to 3
//            0 ... base address 0x00000000
//            1 ... base address 0x10000000, flash memory
//            2 ... base address 0x20000000, SRAM memory
//            3 ... base address 0x30000000
INLINE void InterpCfgMSB(u32* cfg, u8 bits) { *cfg = (*cfg & ~(3<<19)) | ((u32)bits << 19); }

// === interpolator setup

// save interpolator state (for current CPU core)
void NOFLASH(InterpSave)(int interp, interp_hw_save_t* save);

// load interpolator state (for current CPU core)
void NOFLASH(InterpLoad)(int interp, const interp_hw_save_t* save);

// reset interpolator to default state
//  interp ... interpolator 0 or 1
void NOFLASH(InterpReset)(int interp);

// set interpolator shift (right-shift applied to accumulator before masking)
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1
//  shift ... shift value 0 to 31
INLINE void InterpShift(int interp, int lane, int shift)
	{ *INTERP_CTRL(interp, lane) = (*INTERP_CTRL(interp, lane) & ~0x1f) | shift; }
INLINE void InterpShift_hw(interp_hw_t* hw, int lane, int shift)
	{ hw->ctrl[lane] = (hw->ctrl[lane] & ~0x1f) | shift; }

// set interpolator mask range (range of bits that are allowed to pass)
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1
//  lsb ... least significant bit allowed to pass, inclusive, 0 to 31
//  msb ... most significant bit allowed to pass, inclusive, 0 to 31 (lsb <= msb)
INLINE void InterpMask(int interp, int lane, int lsb, int msb)
	{ *INTERP_CTRL(interp, lane) = (*INTERP_CTRL(interp, lane) &
		~((0x1f<<5)|(0x1f<<10))) | ((u32)lsb<<5) | ((u32)msb<<10); }
INLINE void InterpMask_hw(interp_hw_t* hw, int lane, int lsb, int msb)
	{ hw->ctrl[lane] = (hw->ctrl[lane] & ~((0x1f<<5)|(0x1f<<10))) | ((u32)lsb<<5) | ((u32)msb<<10); }

// set interpolator signed mode (values are sign-extended to 32 bits)
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1
INLINE void InterpSigned(int interp, int lane) { *INTERP_CTRL(interp, lane) |= B15; }
INLINE void InterpSigned_hw(interp_hw_t* hw, int lane) { hw->ctrl[lane] |= B15; }

// set interpolator unsigned mode (default state)
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1
INLINE void InterpUnsigned(int interp, int lane) { *INTERP_CTRL(interp, lane) &= ~B15; }
INLINE void InterpUnsigned_hw(interp_hw_t* hw, int lane) { hw->ctrl[lane] &= ~B15; }

// set feed opposite lane's accumulator into this lane's shift+mask input
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1
//  en ... True = feed opposite lane's accumulator to this input, False = use input from own accumulator
INLINE void InterpCrossInput(int interp, int lane, Bool en)
	{ *INTERP_CTRL(interp, lane) = (*INTERP_CTRL(interp, lane) & ~B16) | (en ? B16 : 0); }
INLINE void InterpCrossInput_hw(interp_hw_t* hw, int lane, Bool en)
	{ hw->ctrl[lane] = (hw->ctrl[lane] & ~B16) | (en ? B16 : 0); }

// set feed opposite lane's result into this lane's accumulator on POP
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1
//  en ... True = feed opposite lane's result to this accumulator, False = no auto-feed from result
INLINE void InterpCrossResult(int interp, int lane, Bool en)
	{ *INTERP_CTRL(interp, lane) = (*INTERP_CTRL(interp, lane) & ~B17) | (en ? B17 : 0); }
INLINE void InterpCrossResult_hw(interp_hw_t* hw, int lane, Bool en)
	{ hw->ctrl[lane] = (hw->ctrl[lane] & ~B17) | (en ? B17 : 0); }

// mask+shift is bypassed for this result, add raw input value (does not affect FULL result)
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1
//  en ... True = shift+mask is bypassed, False = shif+mask is applied
INLINE void InterpAddRaw(int interp, int lane, Bool en)
	{ *INTERP_CTRL(interp, lane) = (*INTERP_CTRL(interp, lane) & ~B18) | (en ? B18 : 0); }
INLINE void InterpAddRaw_hw(interp_hw_t* hw, int lane, Bool en)
	{ hw->ctrl[lane] = (hw->ctrl[lane] & ~B18) | (en ? B18 : 0); }

// Two data bits ORed into bits 29:28 of lane result presented to the processor (no effect on internal paths)
// - handy for using lane to generate sequence of pointers into flash or SRAM.
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1
//  bits ... two bits to be ORed into bits 29:28, value 0 to 3
//            0 ... base address 0x00000000
//            1 ... base address 0x10000000, flash memory
//            2 ... base address 0x20000000, SRAM memory
//            3 ... base address 0x30000000
INLINE void InterpMSB(int interp, int lane, int bits)
	{ *INTERP_CTRL(interp, lane) = (*INTERP_CTRL(interp, lane) & ~(3<<19)) | ((u32)bits << 19); }
INLINE void InterpMSB_hw(interp_hw_t* hw, int lane, int bits)
	{ hw->ctrl[lane] = (hw->ctrl[lane] & ~(3<<19)) | ((u32)bits << 19); }

// blending mode (only on interpolator 0 of each CPU core)
//  en ... True = LANE1 result is linear interpolation between BASE0 and BASE1,
//         controlled by 8 LSB bits of LANE1 shift+mask value (fractional number 0 to 255 / 256),
//         LANE0 result does not have BASE0 added (yields only 8 LSB of LANE1 shift+mask),
//         FULL result does not have lane 1 shift+mask value addes (BASE2 + lane0 shift+mask),
// LANE1 SIGNED flag controls whether interpolation is signed or unsigned
INLINE void InterpBlend(Bool en)
	{ *INTERP_CTRL(0, 0) = (*INTERP_CTRL(0, 0) & ~B21) | (en ? B21 : 0); }

// clamping mode (only on interpolator 1 of each CPU core)
//  en ... True = LANE0 result is shifted and masked ACCUM0, clamped by
//         lower bound of BASE0 and upper bound of BASE1.
// LANE0 SIGNED flag controls whether comparisons are is signed or unsigned
INLINE void InterpClamp(Bool en)
	{ *INTERP_CTRL(1, 0) = (*INTERP_CTRL(1, 0) & ~B22) | (en ? B22 : 0); }

// check if any masked-off MSBs in ACCUM are set
//  interp ... interpolator 0 or 1
//  accum ... accumulator 0 or 1, 2 = either 0 or 1 is set
INLINE Bool InterpOver(int interp, int accum) { return ((*INTERP_CTRL(interp, 0) >> (23 + accum)) & 1) != 0; }
INLINE Bool InterpOver_hw(interp_hw_t* hw, int accum) { return ((hw->ctrl[0] >> (23 + accum)) & 1) != 0; }

// === interpolator data processing

// set interpolator accumulator
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1
//  val ... value to set
INLINE void InterpAccum(int interp, int lane, u32 val) { *INTERP_ACCUM(interp, lane) = val; }
INLINE void InterpAccum_hw(interp_hw_t* hw, int lane, u32 val) { hw->accum[lane] = val; }

// get interpolator accumulator
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1
INLINE u32 InterpGetAccum(int interp, int lane) { return *INTERP_ACCUM(interp, lane); }
INLINE u32 InterpGetAccum_hw(const interp_hw_t* hw, int lane) { return hw->accum[lane]; }

// add atomically to interpolator accumulator
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1
//  val ... value to add
INLINE void InterpAdd(int interp, int lane, u32 val) { *INTERP_ADD(interp, lane) = val; }
INLINE void InterpAdd_hw(interp_hw_t* hw, int lane, u32 val) { hw->add_raw[lane] = val; }

// get interpolator raw lane value (result of shift+mask operation, without base added)
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1
INLINE u32 InterpRaw(int interp, int lane) { return *INTERP_ADD(interp, lane); }
INLINE u32 InterpRaw_hw(const interp_hw_t* hw, int lane) { return hw->add_raw[lane]; }

// set interpolator base
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1; or 2 to set common base of both lanes
//  val ... value to set
INLINE void InterpBase(int interp, int lane, u32 val) { *INTERP_BASE(interp, lane) = val; }
INLINE void InterpBase_hw(interp_hw_t* hw, int lane, u32 val) { hw->base[lane] = val; }

// get interpolator base
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1; or 2 to get common base of both lanes
INLINE u32 InterpGetBase(int interp, int lane) { return *INTERP_BASE(interp, lane); }
INLINE u32 InterpGetBase_hw(const interp_hw_t* hw, int lane) { return hw->base[lane]; }

// set interpolator both bases
//  interp ... interpolator 0 or 1
//  val ... value to set, lower 16 bits go to BASE0, upper 16 bits go to BASE1 (can be signed)
INLINE void InterpBaseBoth(int interp, u32 val) { *INTERP_BASE01(interp) = val; }
INLINE void InterpBaseBoth_hw(interp_hw_t* hw, u32 val) { hw->base01 = val; }

// set interpolator both bases
//  interp ... interpolator 0 or 1
//  val0 ... value to set to BASE0
//  val1 ... value to set to BASE1
INLINE void InterpBaseBoth16(int interp, u16 val0, u16 val1) { *INTERP_BASE01(interp) = ((u32)val1 << 16) | val0; }
INLINE void InterpBaseBoth16_hw(interp_hw_t* hw, u16 val0, u16 val1) { hw->base01 = ((u32)val1 << 16) | val0; }

// get lane result and write to both accumulators
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1 (or 2 to get full result)
INLINE u32 InterpPop(int interp, int lane) { return *INTERP_POP(interp, lane); }
INLINE u32 InterpPop_hw(interp_hw_t* hw, int lane) { return hw->pop[lane]; }

// get lane full result and write to both accumulators
//  interp ... interpolator 0 or 1
INLINE u32 InterpPopFull(int interp) { return *INTERP_POP(interp, 2); }
INLINE u32 InterpPopFull_hw(interp_hw_t* hw) { return hw->pop[2]; }

// get lane result without altering any internal state
//  interp ... interpolator 0 or 1
//  lane ... lane 0 or 1 (or 2 to get full result)
INLINE u32 InterpPeek(int interp, int lane) { return *INTERP_PEEK(interp, lane); }
INLINE u32 InterpPeek_hw(interp_hw_t* hw, int lane) { return hw->peek[lane]; }

// get lane full result without altering any internal state
//  interp ... interpolator 0 or 1
INLINE u32 InterpPeekFull(int interp) { return *INTERP_PEEK(interp, 2); }
INLINE u32 InterpPeekFull_hw(interp_hw_t* hw) { return hw->peek[2]; }

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original-SDK

// Interpolator configuration
typedef struct {
	u32 ctrl;
} interp_config;

// get interpolator index
INLINE uint interp_index(interp_hw_t *interp) { return InterpGetInx(interp); }

// Claim the interpolator lane specified
INLINE void interp_claim_lane(interp_hw_t *interp, uint lane) { InterpClaim(interp_index(interp), lane); }
#define interp_lane_claim interp_claim_lane

// Claim the interpolator lanes specified in the mask
INLINE void interp_claim_lane_mask(interp_hw_t *interp, uint lane_mask) { InterpClaimMask(interp_index(interp), lane_mask); }

// Release a previously claimed interpolator lane
INLINE void interp_unclaim_lane(interp_hw_t *interp, uint lane) { InterpUnclaim(interp_index(interp), lane); }
#define interp_lane_unclaim interp_unclaim_lane

// Determine if an interpolator lane is claimed
INLINE bool interp_lane_is_claimed(interp_hw_t *interp, uint lane) { return InterpIsClaimed(interp_index(interp), lane); }

// Release previously claimed interpolator lanes \see interp_claim_lane_mask
INLINE void interp_unclaim_lane_mask(interp_hw_t *interp, uint lane_mask) { InterpUnclaimMask(interp_index(interp), lane_mask); }

// Set the interpolator shift value
INLINE void interp_config_set_shift(interp_config *c, uint shift) { InterpCfgShift(&c->ctrl, shift); }

// Set the interpolator mask range
INLINE void interp_config_set_mask(interp_config *c, uint mask_lsb, uint mask_msb)
	{ InterpCfgMask(&c->ctrl, mask_lsb, mask_msb); }

// Enable cross input
INLINE void interp_config_set_cross_input(interp_config *c, bool cross_input)
	{ InterpCfgCrossInput(&c->ctrl, cross_input); }

// Enable cross results
INLINE void interp_config_set_cross_result(interp_config *c, bool cross_result)
	{ InterpCfgCrossResult(&c->ctrl, cross_result); }

// Set sign extension
INLINE void interp_config_set_signed(interp_config *c, bool _signed)
	{ if (_signed) InterpCfgSigned(&c->ctrl); else InterpCfgUnsigned(&c->ctrl); }

// Set raw add option
INLINE void interp_config_set_add_raw(interp_config *c, bool add_raw) { InterpCfgAddRaw(&c->ctrl, add_raw); }

// Set blend mode
INLINE void interp_config_set_blend(interp_config *c, bool blend) { InterpCfgBlend(&c->ctrl, blend); }

// Set interpolator clamp mode (Interpolator 1 only)
INLINE void interp_config_set_clamp(interp_config *c, bool clamp) { InterpCfgClamp(&c->ctrl, clamp); }

// Set interpolator Force bits
INLINE void interp_config_set_force_bits(interp_config *c, uint bits) { InterpCfgMSB(&c->ctrl, bits); }

// Get a default configuration
INLINE interp_config interp_default_config(void) { interp_config c = { INTERP_DEFCTRL }; return c; }

// Send configuration to a lane
INLINE void interp_set_config(interp_hw_t *interp, uint lane, interp_config *config) { InterpCfg_hw(interp, lane, config->ctrl); }

// Directly set the force bits on a specified lane
INLINE void interp_set_force_bits(interp_hw_t *interp, uint lane, uint bits) { InterpMSB_hw(interp, lane, bits); }

// Save the specified interpolator state
INLINE void interp_save(interp_hw_t *interp, interp_hw_save_t *saver) { InterpSave(interp_index(interp), saver); }

// Restore an interpolator state
INLINE void interp_restore(interp_hw_t *interp, const interp_hw_save_t *saver) { InterpLoad(interp_index(interp), saver); }

// Sets the interpolator base register by lane
INLINE void interp_set_base(interp_hw_t *interp, uint lane, u32 val) { InterpBase_hw(interp, lane, val); }

// Gets the content of interpolator base register by lane
INLINE u32 interp_get_base(interp_hw_t *interp, uint lane) { return InterpGetBase_hw(interp, lane); }

// Sets the interpolator base registers simultaneously
INLINE void interp_set_base_both(interp_hw_t *interp, u32 val) { InterpBaseBoth_hw(interp, val); }

// Sets the interpolator accumulator register by lane
INLINE void interp_set_accumulator(interp_hw_t *interp, uint lane, u32 val) { InterpAccum_hw(interp, lane, val); }

// Gets the content of the interpolator accumulator register by lane
INLINE u32 interp_get_accumulator(interp_hw_t *interp, uint lane) { return InterpGetAccum_hw(interp, lane); }

// Read lane result, and write lane results to both accumulators to update the interpolator
INLINE u32 interp_pop_lane_result(interp_hw_t *interp, uint lane) { return InterpPop_hw(interp, lane); }

// Read lane result
INLINE u32 interp_peek_lane_result(interp_hw_t *interp, uint lane) { return InterpPeek_hw(interp, lane); }

// Read lane result, and write lane results to both accumulators to update the interpolator
INLINE u32 interp_pop_full_result(interp_hw_t *interp) { return InterpPopFull_hw(interp); }

// Read lane result
INLINE u32 interp_peek_full_result(interp_hw_t *interp) { return InterpPeekFull_hw(interp); }

// Add to accumulator
INLINE void interp_add_accumulater(interp_hw_t *interp, uint lane, u32 val) { InterpAdd_hw(interp, lane, val); }

// Get raw lane value
INLINE u32 interp_get_raw(interp_hw_t *interp, uint lane) { return InterpRaw_hw(interp, lane); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_INTERP_H

#endif // USE_INTERP	// use interpolator (sdk_interp.c, sdk_interp.h)
