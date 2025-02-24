
// ****************************************************************************
//
//                              Pico Init
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

#if USE_PICO

#ifndef _PICO_INIT_H
#define _PICO_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

// Device init
void DeviceInit();

// Device terminate
void DeviceTerm();

#define LED1		0	// LED1 index
#define LED_NUM		1	// number of LEDs

// set LED ON (inx = LED index LED?)
void NOFLASH(LedOn)(u8 inx);

// set LED OFF (inx = LED index LED?)
void NOFLASH(LedOff)(u8 inx);

// flip LED (inx = LED index LED?)
void NOFLASH(LedFlip)(u8 inx);

// set LED (inx = LED index LED?)
void NOFLASH(LedSet)(u8 inx, u8 val);

#ifdef __cplusplus
}
#endif

#endif // _PICO_INIT_H

#endif // USE_PICO
