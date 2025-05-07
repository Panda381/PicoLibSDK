
// ****************************************************************************
//
//                   Library files - for faster bulk compilation
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

#if USE_ALARM
#include "src/lib_alarm.c"
#endif

#if USE_CALENDAR
#include "src/lib_calendar.c"
#endif

#if USE_CALENDAR64
#include "src/lib_calendar64.c"
#endif

#if USE_CANVAS
#include "src/lib_canvas.c"
#endif

#if USE_COLOR
#include "src/lib_color.c"
#endif

#if USE_CONFIG
#include "src/lib_config.c"
#endif

#if USE_CRC
#include "src/lib_crc.c"
#endif

#if USE_DECNUM
#include "src/lib_decnum.c"
#endif

#include "src/lib_draw.c"


#if USE_DRAWCAN
#include "src/lib_drawcan.c"

#if USE_DRAWCAN1
#include "src/lib_drawcan1.c"
#endif

#if USE_DRAWCAN2
#include "src/lib_drawcan2.c"
#endif

#if USE_DRAWCAN3
#include "src/lib_drawcan3.c"
#endif

#if USE_DRAWCAN4
#include "src/lib_drawcan4.c"
#endif

#if USE_DRAWCAN6
#include "src/lib_drawcan6.c"
#endif

#if USE_DRAWCAN8
#include "src/lib_drawcan8.c"
#endif

#if USE_DRAWCAN12
#include "src/lib_drawcan12.c"
#endif

#if USE_DRAWCAN16
#include "src/lib_drawcan16.c"
#endif
#endif // USE_DRAWCAN


#if USE_ESCPKT
#include "src/lib_escpkt.c"
#endif

#if USE_EVENT
#include "src/lib_event.c"
#endif

#if USE_FAT
#include "src/lib_fat.c"
#endif

#if USE_FILESEL
#include "src/lib_filesel.c"
#endif

#if USE_LIST
#include "src/lib_list.c"
#endif

#if USE_MALLOC
#include "src/lib_malloc.c"
#endif

#if USE_MAT2D
#include "src/lib_mat2d.c"
#endif

#if USE_MINIRING
#include "src/lib_miniring.c"
#endif

#if USE_PMALLOC
#include "src/lib_pmalloc.c"
#endif

#if USE_PRINT
#include "src/lib_print.c"
#endif

#include "src/lib_pwmsnd.c"

#if USE_RAND
#include "src/lib_rand.c"
#endif

#if USE_RECT
#include "src/lib_rect.c"
#endif

#if USE_RING
#include "src/lib_ring.c"
#endif

#if USE_RINGRX
#include "src/lib_ringrx.c"
#endif

#if USE_RINGTX
#include "src/lib_ringtx.c"
#endif

#if USE_SD
#include "src/lib_sd.c"
#endif

#if USE_STREAM
#include "src/lib_stream.c"
#endif

#if USE_TEXT
#include "src/lib_text.c"
#endif

#if USE_TEXTLIST
#include "src/lib_textlist.c"
#endif

#if USE_TEXT
#include "src/lib_textnum.c"
#endif

#if USE_TPRINT
#include "src/lib_tprint.c"
#endif

#if USE_TREE
#include "src/lib_tree.c"
#endif

#if USE_VIDEO
#include "src/lib_video.c"
#endif



#if USE_MP3
#include "mp3/lib_mp3.c"
#endif
#if USE_EMU
#include "emu/emu.c"
#endif



#if USE_BIGINT
#include "bigint/bigint.c"
#endif

#if BIGINT_BERN_NUM > 0
#include "bigint/bernoulli.c"
#endif



#if USE_REAL
#include "real/real.c"
#endif
