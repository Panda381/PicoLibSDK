// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#ifndef _LIB_INCLUDE_H
#define _LIB_INCLUDE_H

#if USE_ALARM
#include "inc/lib_alarm.h"		// SysTick alarm
#endif

#if USE_CALENDAR
#include "inc/lib_calendar.h"		// calendar 32-bit
#endif

#if USE_CALENDAR64
#include "inc/lib_calendar64.h"		// calendar 64-bit
#endif

#if USE_CANVAS
#include "inc/lib_canvas.h"		// canvas
#endif

#if USE_COLOR
#include "inc/lib_color.h"		// color vector
#endif

#if USE_CONFIG
#include "inc/lib_config.h"		// device configuration
#endif

#if USE_CRC
#include "inc/lib_crc.h"		// CRC Check Sum
#endif

#if USE_DECNUM
#include "inc/lib_decnum.h"		// decode number
#endif

#include "inc/lib_draw.h"		// draw to frame buffer

#if USE_DRAWCAN
#include "inc/lib_drawcan.h"		// drawing canvas
#include "inc/lib_drawcan1.h"
#include "inc/lib_drawcan2.h"
#include "inc/lib_drawcan3.h"
#include "inc/lib_drawcan4.h"
#include "inc/lib_drawcan6.h"
#include "inc/lib_drawcan8.h"
#include "inc/lib_drawcan12.h"
#include "inc/lib_drawcan16.h"
#endif

#if USE_ESCPKT
#include "inc/lib_escpkt.h"		// escape packet protocol
#endif

#if USE_EVENT
#include "inc/lib_event.h"		// event ring buffer
#endif

#if USE_FAT
#include "inc/lib_fat.h"		// FAT file system
#endif

#if USE_FILESEL
#include "inc/lib_filesel.h"		// file selection
#endif

#if USE_LIST
#include "inc/lib_list.h"		// Doubly Linked List
#endif

#if USE_MALLOC
#include "inc/lib_malloc.h"		// memory allocator
#endif

#include "inc/lib_mat2d.h"		// 2D transformation matrix

#if USE_MINIRING
#include "inc/lib_miniring.h"		// Mini-Ring buffer
#endif

#if USE_PMALLOC
#include "inc/lib_pmalloc.h"		// PSRAM memory allocator
#endif

#if USE_PRINT
#include "inc/lib_print.h"		// formatted print
#endif

#include "inc/lib_pwmsnd.h"		// PWM sound

#if USE_RAND
#include "inc/lib_rand.h"		// random generator
#endif

#if USE_RECT
#include "inc/lib_rect.h"		// rectangle
#endif

#if USE_RING
#include "inc/lib_ring.h"		// Ring buffer
#endif

#if USE_RINGRX
#include "inc/lib_ringrx.h"		// Ring buffer with DMA in receiver mode
#endif

#if USE_RINGTX
#include "inc/lib_ringtx.h"		// Ring buffer with DMA in transmitter mode
#endif

#if USE_SD
#include "inc/lib_sd.h"			// SD card
#endif

#if USE_STREAM
#include "inc/lib_stream.h"		// data stream
#endif

#if USE_TPRINT
#include "inc/lib_tprint.h"		// text print
#endif

#include "inc/lib_text.h"		// text strings

#if USE_TEXTLIST
#include "inc/lib_textlist.h"		// list of text strings
#endif

#if USE_TREE
#include "inc/lib_tree.h"		// tree list
#endif

#if USE_VIDEO
#include "inc/lib_video.h"		// video player
#endif


#if USE_MP3
#include "mp3/lib_mp3.h"		// MP3 decoder
#endif


#if USE_EMU
#include "emu/emu.h"			// emulators
#endif

#endif // _LIB_INCLUDE_H
