
// ****************************************************************************
//
//                        Voltage regulator (only RP2040)
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

#if RP2040

#ifndef _SDK_VREG_H
#define _SDK_VREG_H

#include "../sdk_addressmap.h"		// Register address offsets

#if USE_ORIGSDK		// include interface of original SDK
#include "orig_rp2040/orig_vreg.h"	// constants of original SDK
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

//#define VREG_AND_CHIP_RESET_BASE 0x40064000
#define VREG_CTRL	((volatile u32*)VREG_AND_CHIP_RESET_BASE)

typedef struct {
	io32	vreg;		// 0x00: Voltage regulator control and status
	io32	bod;		// 0x04: brown-out detection control
	io32	chip_reset;	// 0x08: Chip reset control and status
} vreg_and_chip_reset_hw_t;

#define vreg_and_chip_reset_hw ((vreg_and_chip_reset_hw_t*)VREG_AND_CHIP_RESET_BASE)

STATIC_ASSERT(sizeof(vreg_and_chip_reset_hw_t) == 0x0C, "Incorrect vreg_and_chip_reset_hw_t!");

// Possible voltage values
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

#define VREG_VOLTAGE_MIN	VREG_VOLTAGE_0_85	// minimum voltage
#define VREG_VOLTAGE_DEF	VREG_VOLTAGE_1_10	// default voltage after power up
#define VREG_VOLTAGE_MAX	VREG_VOLTAGE_1_30	// maximum voltage

// set voltage VREG_VOLTAGE_*
void VregSetVoltage(int vreg);

// get voltage VREG_VOLTAGE_*
INLINE u8 VregVoltage(void) { return (u8)((*VREG_CTRL >> 4) & 0x0f); }

// get voltage in volts
INLINE float VregVoltageFloat(void) { return (VregVoltage() + 11) * 0.05f; }

// check if voltage is correctly regulated
INLINE Bool VregIsOk(void) { return (*VREG_CTRL & B12) != 0; }

// wait for regulated state
void VregWait(void);

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

#define VREG_VOLTAGE_DEFAULT VREG_VOLTAGE_DEF

// Set voltage
INLINE void vreg_set_voltage(int voltage) { VregSetVoltage(voltage); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_VREG_H

#endif // RP2040
