
// ****************************************************************************
//
//                            Voltage regulator
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

#ifndef _SDK_VREG_H
#define _SDK_VREG_H

#include "../sdk_addressmap.h"		// Register address offsets

#ifdef __cplusplus
extern "C" {
#endif

//#define VREG_AND_CHIP_RESET_BASE 0x40064000
#define VREG_CTRL	((volatile u32*)VREG_AND_CHIP_RESET_BASE)

// Possible voltage values
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

#define VREG_VOLTAGE_MIN	VREG_VOLTAGE_0_80	// minimum voltage
#define VREG_VOLTAGE_DEF	VREG_VOLTAGE_1_10	// default voltage after power up
#define VREG_VOLTAGE_MAX	VREG_VOLTAGE_1_30	// maximum voltage

// set voltage VREG_VOLTAGE_*
void VregSetVoltage(u8 vreg);

// check if voltage is correctly regulated
INLINE Bool VregIsOk() { return (*VREG_CTRL & B12) != 0; }

// wait for regulated state
void VregWait();

#ifdef __cplusplus
}
#endif

#endif // _SDK_VREG_H
