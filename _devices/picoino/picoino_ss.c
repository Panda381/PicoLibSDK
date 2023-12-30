
// ****************************************************************************
//
//                            Picoino Screen Shot
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

#if USE_PICOINO

#include "../../_display/minivga/minivga.h"
#include "../../_sdk/inc/sdk_timer.h"
#include "../../_lib/inc/lib_fat.h"
#include "../../_lib/inc/lib_sd.h"
#include "picoino_ss.h"

#if USE_SCREENSHOT		// use screen shots

// BMP file header (size 70 bytes)
#pragma pack(push,1)
typedef struct {
	// BMP file header (size 14 bytes)
	u16	bfType;			// 0x00: magic, 'B' 'M' = 0x4D42
	u32	bfSize;			// 0x02: file size, aligned to DWORD = 70 + 320*240*2 + 2 = 153672 = 0x25848
	u16	bfReserved1;		// 0x06: = 0
	u16	bfReserved2;		// 0x08: = 0
	u32	bfOffBits;		// 0x0A: offset of data bits after file header = 70 (0x46)
	// BMP info header (size 40 bytes)
	u32	biSize;			// 0x0E: size of this info header = 56 (0x38)
	s32	biWidth;		// 0x12: width = 320 (0x140)
	s32	biHeight;		// 0x16: height, negate if flip row order = -240 (0xFFFFFF10)
	u16	biPlanes;		// 0x1A: planes = 1
	u16	biBitCount;		// 0x1C: number of bits per pixel = 16
	u32	biCompression;		// 0x1E: compression = 3 (BI_BITFIELDS)
	u32	biSizeImage;		// 0x22: size of data of image + aligned file = 320*240*2 + 2 = 153602 (0x25802)
	s32	biXPelsPerMeter;	// 0x26: X pels per meter = 2834 (= 0xB12)
	s32	biYPelsPerMeter;	// 0x2A: Y pels per meter = 2834 (= 0xB12)
	u32	biClrUsed;		// 0x2E: number of user colors (0 = all)
	u32	biClrImportant;		// 0x32: number of important colors (0 = all)
	// palettes: 4 x 256 bytes	// 0x36
	u32	pal[256];
} sBmp;
#pragma pack(pop)

// BMP file header, 8 bits per pixel
const sBmp BmpHeader = {
	// BMP file header (size 14 bytes)
	0x4D42,			// u16	bfType;		// 0x00: magic, 'B' 'M' = 0x4D42
	14 + 40 + 4*256 + WIDTH*HEIGHT + 2,	// u32	bfSize;	// 0x02: file size, aligned to DWORD = 14 + 40 + 4*256 + 320*240 + 2 = 77880 = 0x13038
	0,			// u16	bfReserved1;	// 0x06: = 0
	0,			// u16	bfReserved2;	// 0x08: = 0
	14 + 40 + 4*256,	// u32	bfOffBits;	// 0x0A: offset of data bits after file header = 1078 (0x436) = 14 + 40 + 4*256
	// BMP info header (size 40 bytes)
	40,			// u32	biSize;		// 0x0E: size of this info header = 40 (0x28)
	WIDTH,			// s32	biWidth;	// 0x12: width = 320 (0x140)
	-HEIGHT,		// s32	biHeight;	// 0x16: height, negate if flip row order = -240 (0xFFFFFF10)
	1,			// u16	biPlanes;	// 0x1A: planes = 1
	8,			// u16	biBitCount;	// 0x1C: number of bits per pixel = 8
	0,			// u32	biCompression;	// 0x1E: compression = 0 (BI_BITFIELDS)
	WIDTH*HEIGHT+2,		// u32	biSizeImage;	// 0x22: size of data of image + aligned file = 320*240 + 2 = 76802 (0x12C02)
	2834,			// s32	biXPelsPerMeter; // 0x26: X pels per meter = 2834 (= 0xB12)
	2834,			// s32	biYPelsPerMeter; // 0x2A: Y pels per meter = 2834 (= 0xB12)
	0,			// u32	biClrUsed;	// 0x2E: number of user colors (0 = all)
	0,			// u32	biClrImportant;	// 0x32: number of important colors (0 = all)
	// palettes
	{
	0x000000, 0x000054, 0x0000C3, 0x0000FF, 0x002900, 0x002954, 0x0029C3, 0x0029FF,
	0x005400, 0x005454, 0x0054C3, 0x0054FF, 0x007500, 0x007554, 0x0075C3, 0x0075FF,
	0x00A600, 0x00A654, 0x00A6C3, 0x00A6FF, 0x00C300, 0x00C354, 0x00C3C3, 0x00C3FF,
	0x00E600, 0x00E654, 0x00E6C3, 0x00E6FF, 0x00FF00, 0x00FF54, 0x00FFC3, 0x00FFFF,
	0x290000, 0x290054, 0x2900C3, 0x2900FF, 0x292900, 0x292954, 0x2929C3, 0x2929FF,
	0x295400, 0x295454, 0x2954C3, 0x2954FF, 0x297500, 0x297554, 0x2975C3, 0x2975FF,
	0x29A600, 0x29A654, 0x29A6C3, 0x29A6FF, 0x29C300, 0x29C354, 0x29C3C3, 0x29C3FF,
	0x29E600, 0x29E654, 0x29E6C3, 0x29E6FF, 0x29FF00, 0x29FF54, 0x29FFC3, 0x29FFFF,
	0x540000, 0x540054, 0x5400C3, 0x5400FF, 0x542900, 0x542954, 0x5429C3, 0x5429FF,
	0x545400, 0x545454, 0x5454C3, 0x5454FF, 0x547500, 0x547554, 0x5475C3, 0x5475FF,
	0x54A600, 0x54A654, 0x54A6C3, 0x54A6FF, 0x54C300, 0x54C354, 0x54C3C3, 0x54C3FF,
	0x54E600, 0x54E654, 0x54E6C3, 0x54E6FF, 0x54FF00, 0x54FF54, 0x54FFC3, 0x54FFFF,
	0x750000, 0x750054, 0x7500C3, 0x7500FF, 0x752900, 0x752954, 0x7529C3, 0x7529FF,
	0x755400, 0x755454, 0x7554C3, 0x7554FF, 0x757500, 0x757554, 0x7575C3, 0x7575FF,
	0x75A600, 0x75A654, 0x75A6C3, 0x75A6FF, 0x75C300, 0x75C354, 0x75C3C3, 0x75C3FF,
	0x75E600, 0x75E654, 0x75E6C3, 0x75E6FF, 0x75FF00, 0x75FF54, 0x75FFC3, 0x75FFFF,
	0xA60000, 0xA60054, 0xA600C3, 0xA600FF, 0xA62900, 0xA62954, 0xA629C3, 0xA629FF,
	0xA65400, 0xA65454, 0xA654C3, 0xA654FF, 0xA67500, 0xA67554, 0xA675C3, 0xA675FF,
	0xA6A600, 0xA6A654, 0xA6A6C3, 0xA6A6FF, 0xA6C300, 0xA6C354, 0xA6C3C3, 0xA6C3FF,
	0xA6E600, 0xA6E654, 0xA6E6C3, 0xA6E6FF, 0xA6FF00, 0xA6FF54, 0xA6FFC3, 0xA6FFFF,
	0xC30000, 0xC30054, 0xC300C3, 0xC300FF, 0xC32900, 0xC32954, 0xC329C3, 0xC329FF,
	0xC35400, 0xC35454, 0xC354C3, 0xC354FF, 0xC37500, 0xC37554, 0xC375C3, 0xC375FF,
	0xC3A600, 0xC3A654, 0xC3A6C3, 0xC3A6FF, 0xC3C300, 0xC3C354, 0xC3C3C3, 0xC3C3FF,
	0xC3E600, 0xC3E654, 0xC3E6C3, 0xC3E6FF, 0xC3FF00, 0xC3FF54, 0xC3FFC3, 0xC3FFFF,
	0xE60000, 0xE60054, 0xE600C3, 0xE600FF, 0xE62900, 0xE62954, 0xE629C3, 0xE629FF,
	0xE65400, 0xE65454, 0xE654C3, 0xE654FF, 0xE67500, 0xE67554, 0xE675C3, 0xE675FF,
	0xE6A600, 0xE6A654, 0xE6A6C3, 0xE6A6FF, 0xE6C300, 0xE6C354, 0xE6C3C3, 0xE6C3FF,
	0xE6E600, 0xE6E654, 0xE6E6C3, 0xE6E6FF, 0xE6FF00, 0xE6FF54, 0xE6FFC3, 0xE6FFFF,
	0xFF0000, 0xFF0054, 0xFF00C3, 0xFF00FF, 0xFF2900, 0xFF2954, 0xFF29C3, 0xFF29FF,
	0xFF5400, 0xFF5454, 0xFF54C3, 0xFF54FF, 0xFF7500, 0xFF7554, 0xFF75C3, 0xFF75FF,
	0xFFA600, 0xFFA654, 0xFFA6C3, 0xFFA6FF, 0xFFC300, 0xFFC354, 0xFFC3C3, 0xFFC3FF,
	0xFFE600, 0xFFE654, 0xFFE6C3, 0xFFE6FF, 0xFFFF00, 0xFFFF54, 0xFFFFC3, 0xFFFFFF,
	},
};

// Do one screen shot (generates file SCRxxxxx.BMP in root of SD card)
void ScreenShot()
{
	int i;
	char fn[] = "/SCR00001.BMP";
	sFile file;

	// mounted flag
	Bool ismount = DiskMounted();

	// auto-mount disk
	if (DiskAutoMount())
	{
		// prepare file name
		while (FileExist(fn))
		{
			i = 8;
			for (;;)
			{
				fn[i]++;
				if (fn[i] <= '9') break;
				fn[i] = '0';
				i--;
			}
		}

		if (FileCreate(&file, fn))
		{
			// write BMP file header
			FileWrite(&file, &BmpHeader, sizeof(sBmp));

			// write image data
			FileWrite(&file, FrameBuf, WIDTH*HEIGHT+2);

			// close file
			FileClose(&file);

			// flush disk writes
			DiskFlush();
		}
	}

	// unmount disk
	if (!ismount) DiskUnmount();
}


#endif // USE_SCREENSHOT		// use screen shots

#endif // USE_PICOINO
