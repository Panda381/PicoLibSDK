
// ****************************************************************************
//
//                           QSPI flash pins
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
#include "../inc/sdk_qspi.h"

// initialize QSPI pins to connect Flash memory (must be run from RAM)
void NOFLASH(QSPI_InitFlash)()
{
	// SCLK: B0=1 fast slew rate, B1=0 schmitt disable, B23=0 no pulls, B45=2 8 mA, B6=0 input disable, B7=0 outpu enable
	*QSPI_PAD(QSPI_PAD_SCLK) = B0 | B5;

	// SS: B0=0 slow slew rate, B1=1 schmitt enable, B23=2 pull up, B45=1 4 mA, B6=1 input enable, B7=0 output enable
	*QSPI_PAD(QSPI_PAD_SS) = B1 | B3 | B4 | B6;

	// SDx: B0=0 slow slew rate, B1=0 schmitt disable, B23=0 no pulls, B45=1 4 mA, B6=1 input enable, B7=0 output enable
	*QSPI_PAD(QSPI_PAD_SD0) = B4 | B6;
	*QSPI_PAD(QSPI_PAD_SD1) = B4 | B6;
	*QSPI_PAD(QSPI_PAD_SD2) = B4 | B6;
	*QSPI_PAD(QSPI_PAD_SD3) = B4 | B6;

	// reset to XIP function
	QSPI_Fnc(QSPI_PIN_SCLK, QSPI_FNC_XIP);
	QSPI_Fnc(QSPI_PIN_SS, QSPI_FNC_XIP);
	QSPI_Fnc(QSPI_PIN_SD0, QSPI_FNC_XIP);
	QSPI_Fnc(QSPI_PIN_SD1, QSPI_FNC_XIP);
	QSPI_Fnc(QSPI_PIN_SD2, QSPI_FNC_XIP);
	QSPI_Fnc(QSPI_PIN_SD3, QSPI_FNC_XIP);
}
