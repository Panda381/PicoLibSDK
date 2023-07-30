// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#include "../../_display/qvga/qvga.h" // ST7789 display
#include "../../_lib/inc/lib_qdraw.h" // drawing to display buffer
#include "../../_lib/inc/lib_pwmsnd.h" // PWM sound
#include "picoino_bat.h"	// battery
#include "picoino_key.h"	// keyboard
#include "picoino_init.h"	// initialize
#include "picoino_ss.h"		// screen shot
