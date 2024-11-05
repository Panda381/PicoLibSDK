// test.cpp : Defines the entry point for the console application.
//


#include <stdio.h>
#include <windows.h>
#include <math.h>

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long int s32;
typedef unsigned long int u32;
typedef signed long long int s64;
typedef unsigned long long int u64;

typedef unsigned int uint;

typedef unsigned char Bool;
#define True 1
#define False 0

#define BIT(pos) (1UL<<(pos))

// Source: NesPalette[] from PicoSystem_InfoNes: main.cpp
// Colors are stored as ggggbbbbaaaarrrr  (GBAR4444)
// converted from http://wiki.picosystem.com/en/tools/image-converter
u16 NesPalette1[64] = {
    0x77f7,  // 00
    0x37f0,  // 01
    0x28f2,  // 02
    0x28f4,  // 03
    0x17f6,  // 04
    0x14f8,  // 05
    0x11f8,  // 06
    0x20f6,  // 07
    0x30f4,  // 08
    0x40f1,  // 09
    0x40f0,  // 0a
    0x41f0,  // 0b
    0x44f0,  // 0c
    0x00f0,  // 0d
    0x00f0,  // 0e
    0x00f0,  // 0f
    0xbbfb,  // 10
    0x7cf1,  // 11
    0x6df4,  // 12
    0x5df8,  // 13
    0x4bfb,  // 14
    0x47fc,  // 15
    0x43fc,  // 16
    0x50fa,  // 17
    0x60f7,  // 18
    0x70f4,  // 19
    0x81f1,  // 1a
    0x84f0,  // 1b
    0x88f0,  // 1c
    0x00f0,  // 1d
    0x00f0,  // 1e
    0x00f0,  // 1f
    0xFFFF,  // 20
    0xcff6,  // 21
    0xbff9,  // 22
    0x9ffd,  // 23
    0x9fff,  // 24
    0x9cff,  // 25
    0x98ff,  // 26
    0xa5ff,  // 27
    0xb3fc,  // 28
    0xc3f9,  // 29
    0xd6f6,  // 2a
    0xd9f4,  // 2b
    0xddf4,  // 2c
    0x55f5,  // 2d
    0x00f0,  // 2e
    0x00f0,  // 2f
    0xFFFF,  // 30
    0xeffc,  // 31
    0xeffe,  // 32
    0xefff,  // 33
    0xefff,  // 34
    0xdfff,  // 35
    0xddff,  // 36
    0xecff,  // 37
    0xebff,  // 38
    0xfbfd,  // 39
    0xfcfc,  // 3a
    0xfdfb,  // 3b
    0xfffb,  // 3c
    0xccfc,  // 3d
    0x00f0,  // 3e
    0x00f0}; // 3f

// Source: NesPalette[] from PicoSystem_InfoNes: InfoNES_System_Win.cpp
// RGB555-Win
u16 NesPalette2[64] = {
  0x39ce, 0x1071, 0x0015, 0x2013, 0x440e, 0x5402, 0x5000, 0x3c20,
  0x20a0, 0x0100, 0x0140, 0x00e2, 0x0ceb, 0x0000, 0x0000, 0x0000,
  0x5ef7, 0x01dd, 0x10fd, 0x401e, 0x5c17, 0x700b, 0x6ca0, 0x6521,
  0x45c0, 0x0240, 0x02a0, 0x0247, 0x0211, 0x0000, 0x0000, 0x0000,
  0x7fff, 0x1eff, 0x2e5f, 0x223f, 0x79ff, 0x7dd6, 0x7dcc, 0x7e67,
  0x7ae7, 0x4342, 0x2769, 0x2ff3, 0x03bb, 0x0000, 0x0000, 0x0000,
  0x7fff, 0x579f, 0x635f, 0x6b3f, 0x7f1f, 0x7f1b, 0x7ef6, 0x7f75,
  0x7f94, 0x73f4, 0x57d7, 0x5bf9, 0x4ffe, 0x0000, 0x0000, 0x0000
};

// Source: NesPalette[] from PicoSystem_InfoNes: InfoNES_System_ppc2003.cpp (Matthew Conte's Palette)
// RGB888-Matthew
u32 NesPalette3[64] = {
	0x808080, 0x0000bb, 0x3700bf, 0x8400a6,
	0xbb006a, 0xb7001e, 0xb30000, 0x912600,
	0x7b2b00, 0x003e00, 0x00480d, 0x003c22,
	0x002f66, 0x000000, 0x050505, 0x050505,

	0xc8c8c8, 0x0059ff, 0x443cff, 0xb733cc,
	0xff33aa, 0xff375e, 0xff371a, 0xd54b00,
	0xc46200, 0x3c7b00, 0x1e8415, 0x009566,
	0x0084c4, 0x111111, 0x090909, 0x090909,

	0xffffff, 0x0095ff, 0x6f84ff, 0xd56fff,
	0xff77cc, 0xff6f99, 0xff7b59, 0xff915f,
	0xffa233, 0xa6bf00, 0x51d96a, 0x4dd5ae,
	0x00d9ff, 0x666666, 0x0d0d0d, 0x0d0d0d,

	0xffffff, 0x84bfff, 0xbbbbff, 0xd0bbff,
	0xffbfea, 0xffbfcc, 0xffc4b7, 0xffccae,
	0xffd9a2, 0xcce199, 0xaeeeb7, 0xaaf7ee,
	0xb3eeff, 0xdddddd, 0x111111, 0x111111
};

// Source: Piotr Grochowski, https://lospec.com/palette-list/nintendo-entertainment-system
// RGB888-Piotr
u32 NesPalette4[64] = {
	0x585858,  // 00
	0x00237C,  // 01
	0x0D1099,  // 02
	0x300092,  // 03
	0x4F006C,  // 04
	0x600035,  // 05
	0x5C0500,  // 06
	0x461800,  // 07
	0x272D00,  // 08
	0x093E00,  // 09
	0x004500,  // 0a
	0x004106,  // 0b
	0x003545,  // 0c
	0x000000,  // 0d
	0x000000,  // 0e
	0x000000,  // 0f
	0xA1A1A1,  // 10
	0x0B53D7,  // 11
	0x3337FE,  // 12
	0x6621F7,  // 13
	0x9515BE,  // 14
	0xAC166E,  // 15
	0xA62721,  // 16
	0x864300,  // 17
	0x596200,  // 18
	0x2D7A00,  // 19
	0x0C8500,  // 1a
	0x007F2A,  // 1b
	0x006D85,  // 1c
	0x000000,  // 1d
	0x000000,  // 1e
	0x000000,  // 1f
	0xFFFFFF,  // 20
	0x51A5FE,  // 21
	0x8084FE,  // 22
	0xBC6AFE,  // 23
	0xF15BFE,  // 24
	0xFE5EC4,  // 25
	0xFE7269,  // 26
	0xE19321,  // 27
	0xADB600,  // 28
	0x79D300,  // 29
	0x51DF21,  // 2a
	0x3AD974,  // 2b
	0x39C3DF,  // 2c
	0x424242,  // 2d
	0x000000,  // 2e
	0x000000,  // 2f
	0xFFFFFF,  // 30
	0xB5D9FE,  // 31
	0xCACAFE,  // 32
	0xE3BEFE,  // 33
	0xF9B8FE,  // 34
	0xFEBAE7,  // 35
	0xFEC3BC,  // 36
	0xF4D199,  // 37
	0xDEE086,  // 38
	0xC6EC87,  // 39
	0xB2F29D,  // 3a
	0xA7F0C3,  // 3b
	0xA8E7F0,  // 3c
	0xACACAC,  // 3d
	0x000000,  // 3e
	0x000000}; // 3f

// Source: NesDev 2C02 https://www.nesdev.org/wiki/PPU_palettes
// RGB888-NesDev
u32 NesPalette5[64] = {
	0x626262,  // 00
	0x001FB2,  // 01
	0x2404C8,  // 02
	0x5200B2,  // 03
	0x730076,  // 04
	0x800024,  // 05
	0x730B00,  // 06
	0x522800,  // 07
	0x244400,  // 08
	0x005700,  // 09
	0x005C00,  // 0a
	0x005324,  // 0b
	0x003C76,  // 0c
	0x000000,  // 0d
	0x000000,  // 0e
	0x000000,  // 0f
	0xABABAB,  // 10
	0x0D57FF,  // 11
	0x4B30FF,  // 12
	0x8A13FF,  // 13
	0xBC08D6,  // 14
	0xD21269,  // 15
	0xC72E00,  // 16
	0x9D5400,  // 17
	0x607B00,  // 18
	0x209800,  // 19
	0x00A300,  // 1a
	0x009942,  // 1b
	0x007DB4,  // 1c
	0x000000,  // 1d
	0x000000,  // 1e
	0x000000,  // 1f
	0xFFFFFF,  // 20
	0x53AEFF,  // 21
	0x9085FF,  // 22
	0xD365FF,  // 23
	0xFF57FF,  // 24
	0xFF5DCF,  // 25
	0xFF7757,  // 26
	0xFA9E00,  // 27
	0xBDC700,  // 28
	0x7AE700,  // 29
	0x43F611,  // 2a
	0x26EF7E,  // 2b
	0x2CD5F6,  // 2c
	0x4E4E4E,  // 2d
	0x000000,  // 2e
	0x000000,  // 2f
	0xFFFFFF,  // 30
	0xB6E1FF,  // 31
	0xCED1FF,  // 32
	0xE9C3FF,  // 33
	0xFFBCFF,  // 34
	0xFFBDF4,  // 35
	0xFFC6C3,  // 36
	0xFFD59A,  // 37
	0xE9E681,  // 38
	0xCEF481,  // 39
	0xB6FB9A,  // 3a
	0xA9FAC3,  // 3b
	0xA9F0F4,  // 3c
	0xB8B8B8,  // 3d
	0x000000,  // 3e
	0x000000}; // 3f

// Source: Wikipedia https://en.wikipedia.org/wiki/List_of_video_game_console_palettes
// RGB888-Wiki
u32 NesPalette6[64] = {
	0x59595f,  // 00
	0x00008f,  // 01
	0x18008f,  // 02
	0x3f0077,  // 03
	0x550055,  // 04
	0x550011,  // 05
	0x550000,  // 06
	0x442200,  // 07
	0x333300,  // 08
	0x113300,  // 09
	0x003311,  // 0a
	0x004444,  // 0b
	0x004466,  // 0c
	0x000000,  // 0d
	0x080808,  // 0e
	0x080808,  // 0f
	0xaaaaaa,  // 10
	0x0044dd,  // 11
	0x5511ee,  // 12
	0x7700ee,  // 13
	0x9900bb,  // 14
	0xaa0055,  // 15
	0x993300,  // 16
	0x884400,  // 17
	0x666600,  // 18
	0x336600,  // 19
	0x006600,  // 1a
	0x006655,  // 1b
	0x005588,  // 1c
	0x080808,  // 1d
	0x080808,  // 1e
	0x080808,  // 1f
	0xeeeeee,  // 20
	0x4488ff,  // 21
	0x7777ff,  // 22
	0x9944ff,  // 23
	0xbb44ee,  // 24
	0xcc5599,  // 25
	0xdd6644,  // 26
	0xcc8800,  // 27
	0xbbaa00,  // 28
	0x77bb00,  // 29
	0x22bb22,  // 2a
	0x22bb77,  // 2b
	0x22bbcc,  // 2c
	0x444444,  // 2d
	0x080808,  // 2e
	0x080808,  // 2f
	0xeeeeee,  // 30
	0x99ccff,  // 31
	0xaaaaff,  // 32
	0xbb99ff,  // 33
	0xdd99ff,  // 34
	0xee99dd,  // 35
	0xeeaaaa,  // 36
	0xeebb99,  // 37
	0xeedd88,  // 38
	0xbbdd88,  // 39
	0x99dd99,  // 3a
	0x99ddbb,  // 3b
	0x99ddee,  // 3c
	0xaaaaaa,  // 3d
	0x080808,  // 3e
	0x080808}; // 3f

// Source: Smooth https://www.firebrandx.com/nespalette.html
// RGB888-Smooth
u32 NesPalette7[64] = {
	0x6a6d6a,  // 00
	0x001380,  // 01
	0x1e008a,  // 02
	0x39007a,  // 03
	0x550056,  // 04
	0x5a0018,  // 05
	0x4f1000,  // 06
	0x3d1c00,  // 07
	0x253200,  // 08
	0x003d00,  // 09
	0x004000,  // 0a
	0x003924,  // 0b
	0x002e55,  // 0c
	0x000000,  // 0d
	0x000000,  // 0e
	0x000000,  // 0f
	0xb9bcb9,  // 10
	0x1850c7,  // 11
	0x4b30e3,  // 12
	0x7322d6,  // 13
	0x951fa9,  // 14
	0x9d285c,  // 15
	0x983700,  // 16
	0x7f4c00,  // 17
	0x5e6400,  // 18
	0x227700,  // 19
	0x027e02,  // 1a
	0x007645,  // 1b
	0x006e8a,  // 1c
	0x000000,  // 1d
	0x000000,  // 1e
	0x000000,  // 1f
	0xffffff,  // 20
	0x68a6ff,  // 21
	0x8c9cff,  // 22
	0xb586ff,  // 23
	0xd975fd,  // 24
	0xe377b9,  // 25
	0xe58d68,  // 26
	0xd49d29,  // 27
	0xb3af0c,  // 28
	0x7bc211,  // 29
	0x55ca47,  // 2a
	0x46cb81,  // 2b
	0x47c1c5,  // 2c
	0x4a4d4a,  // 2d
	0x000000,  // 2e
	0x000000,  // 2f
	0xffffff,  // 30
	0xcceaff,  // 31
	0xdddeff,  // 32
	0xecdaff,  // 33
	0xf8d7fe,  // 34
	0xfcd6f5,  // 35
	0xfddbcf,  // 36
	0xf9e7b5,  // 37
	0xf1f0aa,  // 38
	0xdafaa9,  // 39
	0xc9ffbc,  // 3a
	0xc3fbd7,  // 3b
	0xc4f6f6,  // 3c
	0xbec1be,  // 3d
	0x000000,  // 3e
	0x000000}; // 3f

#define SOURCE	3		// 1=GBAR4444-main, 2=RGB555-Win, 3=RGB888-Matthew,
						// 4=RGB888-Piotr, 5=RGB888-NesDev, 6=RGB888-Wiki
						// 7=RGB888-Smooth

int main(int argc, char **argv)
{
	int i;
	for (i = 0; i < 64; i++)
	{
#if SOURCE == 1

		// GBAR4444-main
		u16 w = NesPalette1[i];
		u8 r = (u8)(w & 0x0f); r = (r << 4) | r;
		u8 g = (u8)(w >> 12); g = (g << 4) | g;
		u8 b = (u8)((w >> 8) & 0x0f); b = (b << 4) | b;

#elif SOURCE == 2

		// RGB555-Win
		u16 w = NesPalette2[i];
		u8 r = (u8)((w >> 10) & 0x1f); r = (r << 3) | (r >> 2);
		u8 g = (u8)((w >> 5) & 0x1f); g = (g << 3) | (g >> 2);
		u8 b = (u8)(w & 0x1f); b = (b << 3) | (b >> 2);

#elif SOURCE == 3

		// RGB888-Matthew
		u32 d = NesPalette3[i];
		u8 r = (u8)(d >> 16);
		u8 g = (u8)(d >> 8);
		u8 b = (u8)d;

#elif SOURCE == 4

		// RGB888-Piotr
		u32 d = NesPalette4[i];
		u8 r = (u8)(d >> 16);
		u8 g = (u8)(d >> 8);
		u8 b = (u8)d;

#elif SOURCE == 5

		// RGB888-NesDev
		u32 d = NesPalette5[i];
		u8 r = (u8)(d >> 16);
		u8 g = (u8)(d >> 8);
		u8 b = (u8)d;

#elif SOURCE == 6

		// RGB888-Wiki
		u32 d = NesPalette6[i];
		u8 r = (u8)(d >> 16);
		u8 g = (u8)(d >> 8);
		u8 b = (u8)d;

#elif SOURCE == 7

		// RGB888-Smooth
		u32 d = NesPalette7[i];
		u8 r = (u8)(d >> 16);
		u8 g = (u8)(d >> 8);
		u8 b = (u8)d;

#endif

#if 1	// 1=use compact listing, 0=use full listing

		// compact listing
		if ((i & 7) == 0) printf("\n\t");
		printf("0x%04x, ", ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
#else

		// full listing
		printf("\t0x%04x,\t// 0x%02x: GBAR4444=0x%04x, RGB555=0x%04x, RGB888=0x%06x, R=%u G=%u B=%u\n", 
			((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3),			// RGB565
			i,
			((b >> 4) << 8) | ((g >> 4) << 12) | (r >> 4) | 0xf0,	// GBAR4444
			((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3),			// RGB555
			(r << 16) | (g << 8) | b,								// RGB888
			r, g, b);
#endif
	}
	return 0;
}
