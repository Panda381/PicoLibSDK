
// ****************************************************************************
//                                 
//                              All Includes
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

// globals
#include "global.h"			// globals

// SDK
#include "_sdk/_include.h"		// SDK includes

// library
#include "_lib/_include.h"		// library includes

// big interers
#if USE_BIGINT
#include "_lib/bigint/bigint.h"		// big integers
#endif

// real numbers
#if USE_REAL	// use real numbers
#define REAL_PATH	"real_h.h"
#include "_lib/real/real.h"		// all real numbers
#endif // USE_REAL

// fonts
#include "_font/_include.h"

// devices
#if USE_PICO			// use Pico device configuration
#include "_devices/pico/_include.h"
#endif

#if USE_DEMOVGA			// use DemoVGA device configuration
#include "_devices/demovga/_include.h"
#endif

#if USE_PICOTRON		// use Picotron device configuration
#include "_devices/picotron/_include.h"
#endif

#if USE_PICOINO			// use Picoino device configuration
#include "_devices/picoino/_include.h"
#endif

#if USE_PICOPAD		// use PicoPad device configuration
#include "_devices/picopad/_include.h"
#endif
