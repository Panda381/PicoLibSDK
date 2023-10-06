
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

// initialize QSPI pins to connect Flash memory
void NOFLASH(QSPI_InitFlash)()
{
	// SCLK 8mA drive, no pulls, no input, no slew limiting
	*QSPI_PAD(QSPI_PAD_SCLK) = B0 | B5;
	*QSPI_PAD(QSPI_PAD_SS) = B0 | B5;

	// SDx disable Schmitt to reduce delay
	*QSPI_PAD(QSPI_PAD_SD0) = B0 | B5 | B6;
	*QSPI_PAD(QSPI_PAD_SD1) = B0 | B5 | B6;
	*QSPI_PAD(QSPI_PAD_SD2) = B0 | B5 | B6;
	*QSPI_PAD(QSPI_PAD_SD3) = B0 | B5 | B6;

	// reset to XIP function
	QSPI_Fnc(QSPI_PIN_SCLK, QSPI_FNC_XIP);
	QSPI_Fnc(QSPI_PIN_SS, QSPI_FNC_XIP);
	QSPI_Fnc(QSPI_PIN_SD0, QSPI_FNC_XIP);
	QSPI_Fnc(QSPI_PIN_SD1, QSPI_FNC_XIP);
	QSPI_Fnc(QSPI_PIN_SD2, QSPI_FNC_XIP);
	QSPI_Fnc(QSPI_PIN_SD3, QSPI_FNC_XIP);
}
