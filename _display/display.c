
// ****************************************************************************
//
//                                 Displays
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

#include "../global.h"	// globals

#if USE_DISPHSTX
#include "disphstx/disphstx.c"
#endif

#if USE_DISPHSTXMINI
#include "disphstxmini/disphstxmini.c"
#endif

#if USE_DVI
#include "dvi/dvi.c"
#endif

#if USE_DVIVGA
#include "dvivga/dvivga.c"
#endif

#if USE_MINIVGA
#include "minivga/minivga.c"
#endif

#if USE_ST7789
#include "st7789/st7789.c"
#endif
