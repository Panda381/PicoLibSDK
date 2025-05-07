
// ****************************************************************************
//
//                                PSRAM memory
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

// Used PSRAM:
//   Pimoroni Pico Plus 2 (PIM724): PSRAM 8MB APS6404L-3SQR-ZR
//   PGA2350 (PIM722): PSRAM 8MB APS6404L-3SQR-ZR
//   SparkFun Pro Micro RP2350: PSRAM 8MB APS6404L-3SQR-ZR

extern u32 PSRAM_Size;	// PSRAM size in bytes (0 = no PSRAM memory) ... base address PSRAM_BASE

#if USE_PSRAM	// use PSRAM Memory (sdk_psram.c, sdk_psram.h)

//#include "sdk_qspi.h"
//#include "sdk_ssi.h"
//#include "sdk_qmi.h"

#ifndef _SDK_PSRAM_H
#define _SDK_PSRAM_H

#ifdef __cplusplus
extern "C" {
#endif

// GPIO with PSRAM CS chip select signal
#ifndef RP2350_PSRAM_CS
#define RP2350_PSRAM_CS		47
#endif

// Max frequency of PSRAM in [Hz] (133 MHz on 3.0V, 109 MHz on 3.3V)
#ifndef RP2350_PSRAM_MAX_SCK_HZ
#define RP2350_PSRAM_MAX_SCK_HZ	109000000
#endif

// CS max pulse width in [ns] (= 8 us)
#ifndef RP2350_PSRAM_MAX_SELCS
#define RP2350_PSRAM_MAX_SELCS	8000
#endif

// CS min deselect time in [ns] (= 50 ns)
#ifndef RP2350_PSRAM_MIN_DESEL
#define RP2350_PSRAM_MIN_DESEL	50
#endif

// setup PSRAM timing (should be called after sys_clock change)
void NOFLASH(PSRAM_Timing)(void);

// initialize PSRAM memory interface (sets PSRAM_Size to 0 if no PSRAM)
void NOFLASH(PSRAM_Init)(void);

#ifdef __cplusplus
}
#endif

#endif // _SDK_PSRAM_H

#endif // USE_PSRAM
