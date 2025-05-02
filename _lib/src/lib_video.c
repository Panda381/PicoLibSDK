
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

#include "../../global.h"	// globals

#if USE_VIDEO	// use video player (lib_video.c, lib_video.h)

#include "../../_sdk/inc/sdk_timer.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_multicore.h"
#include "../inc/lib_video.h"
#include "../inc/lib_pwmsnd.h"
#include "../inc/lib_draw.h"
#include "../../_display/st7789/st7789.h"
#include "../../_display/minivga/minivga.h"
#include "../../_display/disphstx/disphstx.h"
#include "../inc/lib_print.h"

// frame buffer - used both to display and to load 2 video frames
ALIGNED u16 FrameBuf[VIDEO_FRAMEBUF_SIZE];

// pointer to frame to display, NULL=none
volatile u8* VideoDisp1Frame = NULL;

// request to break core1 function
volatile Bool VideoDispBreak = False;

// height to display
volatile int VideoDispHeight = HEIGHT;

// core1 function - display video frame
void VideoDispFrame()
{
	int i, j,h;
	u16* pal;
	u8 *frm, *s;
	u16 c, c2;
	u16 redmask, greenmask, bluemask;

	for (;;)
	{
		// break core1 function
		if (VideoDispBreak) break;

		// check request to display video frame
		dmb();
		frm = (u8*)VideoDisp1Frame;
		if (frm != NULL)
		{
			// height to display
			h = VideoDispHeight;

#if USE_MINIVGA || USE_DISPHSTX 	// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)

			// prepare pointers
			pal = (u16*)frm; // pointer to palettes
			s = frm + 2*256; // pointer to start of image data
			u16* d = FrameBuf; // destination

			redmask = 0xf800; // red mask (5 bits)
			greenmask = 0x07e0; // green mask (6 bits)
			bluemask = 0x001f; // blue mask (5 bits)

			// loop through pixels
			for (i = 0; i < h; i++)
			{
				c2 = pal[*s++];

				for (j = 0; j < WIDTH/2-1; j++)
				{
					c = c2;
					*d++ = c; // save 1st pixel (not interpolated)
					c2 = pal[*s++]; // convert pixel to palettes

					*d++ =	((((c & redmask) >> 1) + ((c2 & redmask) >> 1)) & redmask) | // red
						((((c & greenmask) + (c2 & greenmask)) >> 1) & greenmask) | // green
						((((c & bluemask) + (c2 & bluemask)) >> 1) & bluemask);  // blue
				}
				*d++ = c2;
				*d++ = c2;
			}

#else // USE_MINIVGA

			// start sending image to the display
			DispStartImg(0, WIDTH, 0, h);

			// prepare pointers
			pal = (u16*)frm; // pointer to palettes
			s = frm + 2*256; // pointer to start of image data

			redmask = 0xf800; // red mask (5 bits)
			greenmask = 0x07e0; // green mask (6 bits)
			bluemask = 0x001f; // blue mask (5 bits)

			// loop through pixels
			for (i = 0; i < h; i++)
			{
				c2 = pal[*s++];

				for (j = 0; j < WIDTH/2-1; j++)
				{
					c = c2;
					DispSendImg(c & 0xff); // send 1st pixel
					DispSendImg(c >> 8);

					c2 = pal[*s++]; // convert pixel to palettes

					c =	((((c & redmask) >> 1) + ((c2 & redmask) >> 1)) & redmask) | // red
						((((c & greenmask) + (c2 & greenmask)) >> 1) & greenmask) | // green
						((((c & bluemask) + (c2 & bluemask)) >> 1) & bluemask);  // blue

					DispSendImg(c & 0xff); // send 1st pixel
					DispSendImg(c >> 8);
				}
				DispSendImg(c & 0xff); // send 1st pixel
				DispSendImg(c >> 8);
				DispSendImg(c2 & 0xff); // send 2nd copy of the pixel
				DispSendImg(c2 >> 8);
			}

			// stop sending image to the display
			DispStopImg();

#endif // USE_MINIVGA

			// info core0 that work is done
			dmb();
			VideoDisp1Frame = NULL;
		}
	}

	// signalize quit
	dmb();
	VideoDispBreak = False;
} 

// open video (returns False on error - file not found)
//  - must be paired with VideoClose(), if successful
//  - disk must be mounted with DiskMount()
Bool VideoOpen(sVideo* video, const char* filename)
{
	// clear video descriptor
	memset(video, 0, sizeof(sVideo));
	video->volume = VIDEO_VOLUMEDEF;

	// ooen file
	if (!FileOpen(&video->file, filename)) return False;

	// get number of frames
	video->frames = video->file.size / VIDEO_FRAMESIZE;

	// start display service
	VideoDisp1Frame = NULL;
	VideoDispBreak = False;
	VideoDispHeight = HEIGHT;
#if USE_DISPHSTX
	DispHstxCore1Exec(VideoDispFrame);
#elif USE_MINIVGA		// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)
	VgaCore1Exec(VideoDispFrame);
#else
	Core1Exec(VideoDispFrame);
#endif
	return True;
}

// close video (must be paired with successful VideoOpen())
void VideoClose(sVideo* video)
{
 	// stop display service
	VideoDispBreak = True;
	while (VideoDispBreak) dmb();

	// stop sound
	StopSound();

	// close file
	FileClose(&video->file);
}

// repaint video control
void VideoDispCtrl(sVideo* video)
{
	char charbuf[8];
	int pos, w;

	// get control
	Bool ctrl = video->ctrl;

	// display control
	if (ctrl)
	{
		// wait to display previous frame
		while (VideoDisp1Frame != NULL) dmb();

		// clear background
		DrawRect(0, HEIGHT-VIDEO_CTRL_HEIGHT, WIDTH, VIDEO_CTRL_HEIGHT, COL_BLACK);

		// draw frame
		DrawFrame(VIDEO_CTRL_LEFT+1, HEIGHT-VIDEO_CTRL_HEIGHT+1,
			WIDTH-VIDEO_CTRL_LEFT-VIDEO_CTRL_RIGHT-2, VIDEO_CTRL_HEIGHT-2, COL_WHITE);

		// draw pointer
		w = (video->frame * (WIDTH-VIDEO_CTRL_LEFT-VIDEO_CTRL_RIGHT-2-4) / video->frames);
		DrawRect(VIDEO_CTRL_LEFT + 3, HEIGHT-VIDEO_CTRL_HEIGHT+3, w, VIDEO_CTRL_HEIGHT-6, COL_AZURE);

		// get current video position, and limit to 99:59 = 5999
		pos = VideoPos(video);
		if (pos > 5999) pos = 5999;

		// set font 8x16
		SelFont8x16();

		// display time
		MemPrint(charbuf, 7, "%02d:%02d", pos/60, pos % 60);
		DrawText(charbuf, 0, HEIGHT-VIDEO_CTRL_HEIGHT, COL_WHITE);

		// display volume
		if (video->mute)
			DrawText("X", WIDTH-VIDEO_CTRL_RIGHT+4, HEIGHT-VIDEO_CTRL_HEIGHT, COL_WHITE);
		else
		{
			MemPrint(charbuf, 7, "%02d", video->volume);
			DrawText(charbuf, WIDTH-VIDEO_CTRL_RIGHT, HEIGHT-VIDEO_CTRL_HEIGHT, COL_WHITE);
		}

		// update display
		DispUpdate();
	}
}

// play next video frame (returns False on error or end of file)
Bool VideoPlayFrame(sVideo* video)
{
	// check frame index
	if (video->frame >= video->frames) return False;

	// pointer to current frame buffer
#if USE_MINIVGA || USE_DISPHSTX		// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)
	u8* frm = (u8*)FrameBuf + FRAMESIZE*2 + video->bufinx*VIDEO_FRAMESIZE_ALIGNED;
#else // USE_VGA
	u8* frm = (u8*)FrameBuf + video->bufinx*VIDEO_FRAMESIZE_ALIGNED;
#endif // USE_VGA
	video->bufinx ^= 1;

	// read next frame
	if (FileRead(&video->file, frm, VIDEO_FRAMESIZE) != VIDEO_FRAMESIZE) return False;

	// play sound
	if (!video->mute && !video->pause)
	{
		if ((video->frame == 0) || !PlayingSound())
			PlaySoundChan(0, &frm[2*256 + WIDTH/2*HEIGHT], VIDEO_SAMPLES, True, 1.0f,
					(float)video->volume/VIDEO_VOLUMEDEF, SNDFORM_PCM, 0);
		else
			SetNextSound(&frm[2*256 + WIDTH/2*HEIGHT], VIDEO_SAMPLES);
	}

	// wait to display previous frame
	while (VideoDisp1Frame != NULL) dmb();

	// wait time to next frame
	if (video->frame > 0)
	{
		while ((Time() - video->frametime) < VIDEO_TIMEDELTA) {}
	}
	video->frametime = Time();

	// increase frame number (or restore position on pause)
	if (video->pause)
		FileSeek(&video->file, video->frame*VIDEO_FRAMESIZE);
	else
		video->frame++;

	// repaint video control
	VideoDispCtrl(video);

	// request to display next frame
	VideoDisp1Frame = frm;

	return True;
}

// shift relative video position in seconds
void VideoShiftPos(sVideo* video, int shift)
{
	shift *= VIDEO_FPS; // shift in frames
	int pos = shift + video->frame;
	if (pos < 0) pos = 0;
	if (pos > (int)video->frames) pos = video->frames;
	video->frame = pos;

	// seek file
	FileSeek(&video->file, pos*VIDEO_FRAMESIZE);

	// display control
	if (video->pause)
	{
		VideoPlayFrame(video);
		VideoDispCtrl(video);
	}
}

// video set volume 0..20
void VideoSetVol(sVideo* video, s8 vol)
{
	if (vol < 0) vol = 0;
	if (vol > VIDEO_VOLUMEMAX) vol = VIDEO_VOLUMEMAX;
	video->volume = vol;
	video->mute = False;

	// set volume
	VolumeSound((float)vol/VIDEO_VOLUMEDEF);

	// display control
	if (video->pause) VideoDispCtrl(video);
}

// video set pause
void VideoSetPause(sVideo* video, Bool pause)
{
	video->pause = pause;
	StopSound();
	if (video->pause) VideoPlayFrame(video);
}

// video set mute
void VideoSetMute(sVideo* video, Bool mute)
{
	video->mute = mute;
	StopSound();
	if (video->pause) VideoDispCtrl(video);
}

// set video control
void VideoSetCtrl(sVideo* video, Bool ctrl)
{
	// update control
	video->ctrl = ctrl;

	// wait to display previous frame
	while (VideoDisp1Frame != NULL) dmb();

	// set display height
	VideoDispHeight = ctrl ? (HEIGHT - VIDEO_CTRL_HEIGHT) : HEIGHT;

	// display control
	VideoDispCtrl(video);

	// restore screen if control is off
	if (!ctrl && video->pause) VideoPlayFrame(video);
}

#endif // USE_VIDEO	// use video player (lib_video.c, lib_video.h)
