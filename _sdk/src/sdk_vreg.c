
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

#include "../../global.h"	// globals

#if RP2040

#include "../inc/sdk_vreg.h"

// set voltage VREG_VOLTAGE_*
void VregSetVoltage(int vreg)
{
	vreg &= 0x0f;
	RegMask(VREG_CTRL, (u32)vreg << 4, B4+B5+B6+B7);
}

// wait for regulated state
void VregWait(void)
{
	while (!VregIsOk()) {}
}


#endif // RP2040
