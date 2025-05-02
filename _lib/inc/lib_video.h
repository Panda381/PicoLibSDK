
// ****************************************************************************
//
//                                Video player
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

// Video player requires ST7789 driver and 16-bit frame buffer.
// If you use it, default frame buffer must be disabled using USE_FRAMEBUF set to 0.

#if USE_VIDEO	// use video player (lib_video.c, lib_video.h)

#ifndef _LIB_VIDEO_H
#define _LIB_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lib_fat.h"

#define VIDEO_FPS	10	// number of frames per second
#define VIDEO_SAMPLES	(SOUNDRATE/VIDEO_FPS) // number of sound samples per frame (= 2205)
#define VIDEO_FRAMESIZE	(512 + WIDTH/2*HEIGHT + VIDEO_SAMPLES) // size of one frame (= 41117)
#define VIDEO_TIMEDELTA	(1000000/VIDEO_FPS) // time delta between frames in [us]
#define VIDEO_FRAMESIZE_ALIGNED	((VIDEO_FRAMESIZE + 3) & ~3) // frame size aligned to u32 (= 41120)

// size of video frame buffer to hold 2 video frames
//  FRAMESIZE*2 = 320x240x2 = 153600
//  VIDEO_FRAMESIZE_ALIGNED*2 = 82240
//  -> VIDEO_FRAMEBUF_SIZE = 153600, almost half of the screen remained unused

#if USE_MINIVGA || USE_DISPHSTX 	// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)

#define VIDEO_FRAMEBUF_SIZE (FRAMESIZE + VIDEO_FRAMESIZE_ALIGNED*2/2)

#else // USE_MINIVGA

#if (VIDEO_FRAMESIZE_ALIGNED*2 < FRAMESIZE*2)
#define VIDEO_FRAMEBUF_SIZE FRAMESIZE
#else
#define VIDEO_FRAMEBUF_SIZE (VIDEO_FRAMESIZE_ALIGNED*2/2)
#endif

#endif // USE_MINIVGA

// One frame:
// - (512 bytes) 256 u16 palettes in RGB565 format
// - (38400 bytes) 160*240 u8 pixels
// - (VIDEO_SAMPLE = 2205 bytes) sound sample

#define VIDEO_VOLUMEMAX		20	// max. volume
#define VIDEO_VOLUMEDEF		8	// default volume
#define VIDEO_CTRL_HEIGHT	13	// control height on bottom of screen
#define VIDEO_CTRL_LEFT		40	// control left on bottom of screen
#define VIDEO_CTRL_RIGHT	16	// control right on bottom of screen

// video descriptor
typedef struct {
	u32	frametime;	// time of last frame
	u32	frames;		// total number of frames
	u32	frame;		// current frame
	sFile	file;		// open file descriptor
	u8	bufinx;		// current buffer index 0 or 1
	s8	volume;		// sound volume 0..15
	Bool	mute;		// sound mute
	Bool	pause;		// paused
	Bool	ctrl;		// display control
} sVideo;

// open video (returns False on error - file not found)
//  - must be paired with VideoClose(), if successful
//  - disk must be mounted with DiskMount()
Bool VideoOpen(sVideo* video, const char* filename);

// close video (must be paired with successful VideoOpen())
void VideoClose(sVideo* video);

// repaint video control
void VideoDispCtrl(sVideo* video);

// play next video frame (returns False on error or end of file)
Bool VideoPlayFrame(sVideo* video);

// get video length in seconds
INLINE int VideoLen(sVideo* video) { return video->frames / VIDEO_FPS; }

// get current video position in seconds
INLINE int VideoPos(sVideo* video) { return video->frame / VIDEO_FPS; }

// shift relative video position in seconds
void VideoShiftPos(sVideo* video, int shift);

// video set volume 0..20
void VideoSetVol(sVideo* video, s8 vol);

// video set pause
void VideoSetPause(sVideo* video, Bool pause);

// video set mute
void VideoSetMute(sVideo* video, Bool mute);

// set video control
void VideoSetCtrl(sVideo* video, Bool ctrl);

#ifdef __cplusplus
}
#endif

#endif // _LIB_VIDEO_H

#endif // USE_VIDEO	// use video player (lib_video.c, lib_video.h)
