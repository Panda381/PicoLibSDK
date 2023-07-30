
// ****************************************************************************
//
//                              PicoPad Init
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

#ifndef _PICOPAD_INIT_H
#define _PICOPAD_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

// Device init
void DeviceInit();

// Device terminate
void DeviceTerm();

#ifdef __cplusplus
}
#endif

#endif // _PICOPAD_INIT_H
