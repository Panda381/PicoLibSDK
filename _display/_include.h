// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#ifndef _DISPLAY_INCLUDE_H
#define _DISPLAY_INCLUDE_H

#if USE_DISPHSTX
#include "disphstx/disphstx.h"
#endif

#if USE_DISPHSTXMINI
#include "disphstxmini/disphstxmini.h"
#endif

#if USE_DVI
#include "dvi/dvi.h"
#endif

#if USE_DVIVGA
#include "dvivga/dvivga.h"
#endif

#if USE_MINIVGA
#include "minivga/minivga.h"
#endif

#if USE_ST7789
#include "st7789/st7789.h"
#endif

#endif // _DISPLAY_INCLUDE_H
