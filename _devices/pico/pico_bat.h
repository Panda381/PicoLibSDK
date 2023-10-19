
// ****************************************************************************
//
//                              Pico Battery
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

#ifndef _PICO_BAT_H
#define _PICO_BAT_H

#ifdef __cplusplus
extern "C" {
#endif

#if !USE_EXTDISP

// init battery measurement
void BatInit();

// get battery voltage in V
float GetBat();

// get battery voltage, integer in mV
int GetBatInt();

// terminate battery measurement
void BatTerm();

#endif // !USE_EXTDISP

#ifdef __cplusplus
}
#endif

#endif // _PICO_BAT_H
