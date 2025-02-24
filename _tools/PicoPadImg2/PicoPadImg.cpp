// PicoLibSDK - Alternative SDK library for Raspberry Pico/Pico2 and RP2040/RP2350
// Copyright (c) 2023-2025 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

// Images for PicoPad and PicoLibSDK
// PicoPadImg version 2.0, January 2025

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#pragma warning(disable : 4996) // unsafe fopen

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long int s32;		// on 64-bit system use "signed int"
typedef unsigned long int u32;		// on 64-bit system use "unsigned int"

typedef unsigned int BOOL;
#define TRUE  1
#define FALSE 0

// output formats
enum {
	FORMAT_1 = 0,	// "1": 1-bit Y1 (8 pixels per byte, MSB first pixel, LSB last pixel), requires 1-bit input image
					//			[7654 3210]
	FORMAT_2,		// "2": 2-bit Y2 (4 pixels per byte, MSB first pixel, LSB last pixel), requires 4-bit input image with max. 4 palettes
					//			[3322 1100]
	FORMAT_3,		// "3": 3-bit RGB111 (10 pixels packed in 32-bit word, LSB fist pixel, MSB last pixel), requires 4-bit input image with max. 8 palettes
					//			[xx99 9888 7776 6655 5444 3332 2211 1000]
	FORMAT_4,		// "4": 4-bit YRGB1111 (2 pixels per byte, MSB first pixel, LSB last pixel), requires 4-bit input image with max. 16 palettes
					//			[1111 0000]
	FORMAT_6,		// "6": 6-bit RGB222 (5 pixels packed in 32-bit word, LSB fist pixel, MSB last pixel), requires 8-bit input image with max. 64 palettes
					//			[xx44 4444 3333 3322 2222 1111 1100 0000]
	FORMAT_8,		// "8": 8-bit RGB332 (1 pixel per byte), requires 8-bit input image with max. 256 palettes
	FORMAT_12,		// "12": 12-bit RGB444 (8 pixels packed in three 32-bit words, LSB fist pixel, MSB last pixel), requires 15-bit input image
					//			[2222 2222 1111 1111 1111 0000 0000 0000]
					//			[5555 4444 4444 4444 3333 3333 3333 2222]
					//			[7777 7777 7777 6666 6666 6666 5555 5555]
	FORMAT_12D,		// "12d": 12-bit RGB444 with dithering
	FORMAT_15,		// "15": 15-bit RGB555 (1 pixel per 2 bytes), requires 15-bit input image
	FORMAT_16,		// "16": 16-bit RGB565 (1 pixel per 2 bytes), requires 24-bit input image
	FORMAT_RLE4,	// "r4": 4-bit compression RLE4, requires 4-bit input image with max. 16 palettes
	FORMAT_RLE8,	// "r8": 8-bit compression RLE8, requires 8-bit input image with max. 256 palettes
	FORMAT_HSTX15,	// "x15": 15-bit HSTX compression, requires 15-bit input image
	FORMAT_HSTX16,	// "x16": 16-bit HSTX compression, requires 24-bit input image
	FORMAT_ATTR1,	// "a1": attribute compression 1, 4-bit pixels, cell 8x8, 2 colors
	FORMAT_ATTR2,	// "a2": attribute compression 2, 4-bit pixels, cell 4x4, 2 colors
	FORMAT_ATTR3,	// "a3": attribute compression 3, 4-bit pixels, cell 8x8, 4 colors
	FORMAT_ATTR4,	// "a4": attribute compression 4, 4-bit pixels, cell 4x4, 4 colors
	FORMAT_ATTR5,	// "a5": attribute compression 5, 8-bit pixels, cell 8x8, 2 colors
	FORMAT_ATTR6,	// "a6": attribute compression 6, 8-bit pixels, cell 4x4, 2 colors
	FORMAT_ATTR7,	// "a7": attribute compression 7, 8-bit pixels, cell 8x8, 4 colors
	FORMAT_ATTR8,	// "a8": attribute compression 8, 8-bit pixels, cell 4x4, 4 colors

	FORMAT_NUM
};

#define FORMAT_ATTR_FIRST FORMAT_ATTR1
#define FORMAT_ATTR_LAST FORMAT_ATTR8

const char* SwitchTxt[] = {
	"1",	// FORMAT_1: 1-bit Y1 (8 pixels per byte, MSB first pixel, LSB last pixel), requires 1-bit input image
	"2",	// FORMAT_2: 2-bit Y2 (4 pixels per byte, MSB first pixel, LSB last pixel), requires 4-bit input image with max. 4 palettes
	"3",	// FORMAT_3: 3-bit RGB111 (10 pixels packed in 32-bit word, LSB fist pixel, MSB last pixel), requires 4-bit input image with max. 8 palettes
	"4",	// FORMAT_4: 4-bit YRGB1111 (2 pixels per byte, MSB first pixel, LSB last pixel), requires 4-bit input image with max. 16 palettes
	"6",	// FORMAT_6: 6-bit RGB222 (5 pixels packed in 32-bit word, LSB fist pixel, MSB last pixel), requires 8-bit input image with max. 64 palettes
	"8",	// FORMAT_8: 8-bit RGB332 (1 pixel per byte), requires 8-bit input image with max. 256 palettes
	"12",	// FORMAT_12: 12-bit RGB444 (8 pixels per three 32-bit, LSB fist pixel, MSB last pixel), requires 15-bit input image
	"12d",	// FORMAT_12D: 12-bit RGB444 with dithering
	"15",	// FORMAT_15: 15-bit RGB555 (1 pixel per 2 bytes), requires 15-bit input image
	"16",	// FORMAT_16: 16-bit RGB565 (1 pixel per 2 bytes), requires 24-bit input image
	"r4",	// FORMAT_RLE4: 4-bit compression RLE4, requires 4-bit input image with max. 16 palettes
	"r8",	// FORMAT_RLE8: 8-bit compression RLE8, requires 8-bit input image with max. 256 palettes
	"x15",	// FORMAT_HSTX15: 15-bit HSTX compression, requires 15-bit input image
	"x16",	// FORMAT_HSTX16: 16-bit HSTX compression, requires 24-bit input image
	"a1",	// FORMAT_ATTR1: attribute compression 1, 4-bit pixels, cell 8x8, 2 colors
	"a2",	// FORMAT_ATTR2: attribute compression 2, 4-bit pixels, cell 4x4, 2 colors
	"a3",	// FORMAT_ATTR3: attribute compression 3, 4-bit pixels, cell 8x8, 4 colors
	"a4",	// FORMAT_ATTR4: attribute compression 4, 4-bit pixels, cell 4x4, 4 colors
	"a5",	// FORMAT_ATTR5: attribute compression 5, 8-bit pixels, cell 8x8, 2 colors
	"a6",	// FORMAT_ATTR6: attribute compression 6, 8-bit pixels, cell 4x4, 2 colors
	"a7",	// FORMAT_ATTR7: attribute compression 7, 8-bit pixels, cell 8x8, 4 colors
	"a8",	// FORMAT_ATTR8: attribute compression 8, 8-bit pixels, cell 4x4, 4 colors
};

#pragma pack(push,1)
typedef struct _bmpBITMAPFILEHEADER { // 14 bytes
		u16		bfType;
		u32		bfSize;
		u16		bfReserved1;
		u16		bfReserved2;
		u32		bfOffBits;
} bmpBITMAPFILEHEADER;

typedef struct _bmpBITMAPINFOHEADER{ // 40 bytes at offset 0x0E
		u32		biSize;
		s32		biWidth;
		s32		biHeight;
		u16		biPlanes;
		u16		biBitCount;
		u32		biCompression;
		u32		biSizeImage;
		s32		biXPelsPerMeter;
		s32		biYPelsPerMeter;
		u32		biClrUsed;
		u32		biClrImportant;
		// palettes at offset 54 = 0x36
} bmpBITMAPINFOHEADER;
#pragma pack(pop)

#define bmpBI_RGB	0
#define bmpBI_RLE8	1
#define bmpBI_RLE4	2

int OutFormat;	// output format FORMAT_*
u8* Img = NULL; // input file buffer
int W,H,WB,PalN; // input file width and height
int B; // bit count
u8* D; // start of image line
u16* DD;
u8* Pal;

// export palettes
void PrintPal(FILE* f, char* argv[], int num)
{
	int i, n;

	// palette header
	u8* pal = Pal;
	if (PalN == 0) PalN = num;
	if (PalN > num) PalN = num;
	fprintf(f, "const u16 %s_Pal[%d] __attribute__ ((aligned(4))) = {", argv[3], PalN);

	// load palettes
	n = 0;
	for (i = 0; i < PalN; i++)
	{
		if ((n & 0x0f) == 0) fprintf(f, "\n\t");
		u8 blue = pal[0];
		u8 green = pal[1];
		u8 red = pal[2];
		u16 b = (blue >> 3) | ((green >> 2) << 5) | ((red >> 3) << 11);
		fprintf(f, "0x%04X, ", b);
		n++;
		pal += 4;
	}
}

// ============================================================================
//                           Attribute compression
// ============================================================================

// attribute compression
BOOL AttrMode = FALSE;	// attribute compression active
int AttrInBits;	// attribute compression - number of input bits per pixel (4 or 8)
int AttrCW;		// attribute compression - cell width (4 or 8)
int AttrCH;		// attribute compression - cell height (4 or 8)
int AttrColNum; // attribute compression - number of output colors per cell (2 or 4)
int AttrOutBits; // attribute compression - number of output bits per pixel (1 or 2)

#define ATTR_MAXCOL	4	// max. number of output colors

u8 AttrLum[256];	// palette luminance 0..255 (0.299*red + 0.587*green + 0.114*blue = (77*red + 151*green + 29*blue)>>8)

u8* AttrInx; // result pixel indices
u8* AttrCol; // result pixel colors
u8* AttrTest; // test image

int isqr(int n) { return n*n; }

// get luminance (0..255)
u8 GetLum(u8 red, u8 green, u8 blue) { return (77*red + 151*green + 29*blue)>>8; }

// check distance to palette
int PalDist(const u8* pal, int inx, u8 blue, u8 green, u8 red)
{
	const u8* p = pal + inx*4;
	return isqr(p[0] - blue) + isqr(p[1] - green) + isqr(p[2] - red);
}

// convert palettes to luminance
void AttrPalLum(u8* pal, int paln)
{
	int i;
	u8 blue, green, red;
	for (i = 0; i < paln; i++)
	{
		// get color components
		blue = pal[4*i+0];
		green = pal[4*i+1];
		red = pal[4*i+2];

		// luminance (0..255)
		AttrLum[i] = GetLum(red, green, blue);
	}
}

// do attribute compression (Color Cell Compression)
void DoAttr(u8* pal, int paln, FILE* testimg, int size, int off)
{
	int i, j, k, m, t, x, y, cw, ch, wn, hn, coln;
	u8 pix, blue, green, red;
	int pix_n[ATTR_MAXCOL], pix_b[ATTR_MAXCOL], pix_g[ATTR_MAXCOL], pix_r[ATTR_MAXCOL];
	int avg_lev[2], lev;
	const u8* src;
	const u8* col;
	u8* dst;

	// temporary line width
	int wb = WB;
	if (AttrInBits == 4) wb *= 2;

	// number of colors per cell
	coln = AttrColNum;
	memset(AttrCol, 0, W*H*ATTR_MAXCOL);

	// convert palettes to luminance
	AttrPalLum(pal, paln);

	// cell width and height
	cw = AttrCW;
	ch = AttrCH;

	// number of cells
	wn = W/cw;
	hn = H/ch;

	// search cells
	for (y = 0; y < hn; y++) // rows
	{
		for (x = 0; x < wn; x++) // columns
		{
			// cell source address
			src = &D[y*ch*wb + x*cw];

			// calculate average level of the cell
			lev = 0;
			for (i = 0; i < ch; i++) // lines
			{
				for (j = 0; j < cw; j++) // pixels
				{
					// get pixel
					pix = src[i*wb + j];

					// get level
					blue = pal[4*pix+0];
					green = pal[4*pix+1];
					red = pal[4*pix+2];
					lev += GetLum(red, green, blue);
				}
			}
			lev /= ch*cw;

			// sort pixels into 2 groups
			for (i = 0; i < ATTR_MAXCOL; i++) pix_n[i] = 0;
			for (i = 0; i < 2; i++) avg_lev[i] = 0;
			dst = &AttrInx[y*ch*wb + x*cw];
			for (i = 0; i < ch; i++) // lines
			{
				for (j = 0; j < cw; j++) // pixels
				{
					// get pixel
					pix = src[i*wb + j];

					// sort pixel by luminance, group 0 or 1
					k = (AttrLum[pix] < lev) ? 0 : 1;
					dst[i*wb + j] = k;

					// get color
					blue = pal[4*pix+0];
					green = pal[4*pix+1];
					red = pal[4*pix+2];

					// sum color level of this group
					pix_n[k]++;
					avg_lev[k] += AttrLum[pix];
				}
			}

			// average level
			for (i = 0; i < 2; i++)
			{
				k = pix_n[i];
				if (k > 0)
				{
					avg_lev[i] /= k;
				}
			}

			// sort pixels into 4 groups
			if (AttrColNum == 4)
			{
				pix_n[0] = pix_n[1] = 0;
				for (i = 0; i < ch; i++) // lines
				{
					for (j = 0; j < cw; j++) // pixels
					{
						pix = src[i*wb + j];
						k = dst[i*wb + j]; // current group 0 or 1
						k += (AttrLum[pix] < avg_lev[k]) ? 0 : 2; // sort to subgroups 0+2 or 1+3, by luminance
						dst[i*wb + j] = k; // group 0, 1, 2 or 3
						pix_n[k]++;
					}
				}
			}

			// average color of groups
			for (i = 0; i < ATTR_MAXCOL; i++) pix_b[i] = pix_g[i] = pix_r[i] = 0;
			for (i = 0; i < ch; i++) // lines
			{
				for (j = 0; j < cw; j++) // pixels
				{
					// get pixel
					pix = src[i*wb + j];

					// get pixel index (group 0..3)
					k = dst[i*wb + j];

					// get color
					blue = pal[4*pix+0];
					green = pal[4*pix+1];
					red = pal[4*pix+2];

					// sum colors of this group
					pix_b[k] += blue;
					pix_g[k] += green;
					pix_r[k] += red;
				}
			}

			// search palette
			for (k = 0; k < coln; k++)
			{
				if (pix_n[k] > 0)
				{
					// average color
					pix_b[k] /= pix_n[k];
					pix_g[k] /= pix_n[k];
					pix_r[k] /= pix_n[k];

					blue = pix_b[k];
					green = pix_g[k];
					red = pix_r[k];

					// search nearest palette
					t = 100000000;
					for (i = 0; i < paln; i++)
					{
						m = PalDist(pal, i, blue, green, red);
						if (m < t)
						{
							t = m;

							// save color index
							AttrCol[(y*wb + x)*coln + k] = i;
						}
					}
				}
			}
		}
	}

	// output test image
	if (testimg != NULL)
	{
		// copy original image
		memcpy(AttrTest, Img, size);

		// export image data
		for (y = 0; y < H; y++) // rows
		{
			for (x = 0; x < W; x++) // columns
			{
				src = &AttrInx[y*wb + x]; // index of color 0..3
				col = &AttrCol[((y/ch)*wb + x/cw) * coln]; // array of 2 or 4 pixel colors
				pix = col[*src];

				if (AttrInBits == 4)
				{
					dst = &AttrTest[y*WB + x/2 + off]; // output file
					if ((x & 1) == 0)
						*dst = pix << 4;
					else
						*dst |= pix;
				}
				else
				{
					dst = &AttrTest[y*wb + x + off]; // output file
					*dst = pix;
				}
			}
		}

		// write file
		fwrite(AttrTest, 1, size, testimg);
	}
}

// ============================================================================
//                             RLE8 compression
// ============================================================================

// RLE compression
BOOL Rle = FALSE; // flag - do RLE compression
int* RleOff; // buffer of line offsets
u8* RleBuf; // RLE buffer with compressed image
u8* RleDst; // RLE destination pointer
int RawNum; // counter of raw pixels
u8* RawStart; // start of raw data
int RleNum;	// size of data in RLE buffer, counter of RLE equal pixels
u8 RlePix; // last RLE pixel

// write byte into RLE buffer
void WriteByte(u8 b)
{
	*RleDst++ = b;
}

// flush RAW string
void FlushRaw()
{
	while (RawNum > 0)
	{
		// prepare length of one sub-string
		int rawnum = RawNum;
		if (rawnum > 128) rawnum = 128;
		RawNum -= rawnum;

		// write raw pixels (flag byte: 0..127 means 1..128 raw pixels)
		WriteByte(rawnum-1); // number of pixels
		for (; rawnum > 0; rawnum--) WriteByte(*RawStart++);
	}
}

// flush RLE string
void FlushRle()
{
	while (RleNum > 0)
	{
		// prepare lenth of one sub-string
		int rlenum = RleNum;
		if (rlenum > 130) rlenum = 130;
		RleNum -= rlenum;

		// convert short RLE string to RAW string
		if (rlenum < 3)
		{
			RawNum += rlenum;
		}

		// write long RLE string
		else
		{
			// at first, flush RAW string
			FlushRaw();

			// write RLE string (flag byte: 128..255 means 3..130 raw pixels)
			WriteByte((rlenum-3)+128); // number of pixels
			WriteByte(RlePix); // write RLE pixel

			// shift RAW start
			RawStart += rlenum;
		}
	}
}

// do RLE compression
void DoRle()
{
	u8 c;
	int i, j;
	u8* s = D;
	RleDst = RleBuf;	// destination pointer	
	RleNum = 0; // reset counter of equal pixels
	RawNum = 0; // reset counter of raw pixels
	RleOff[0] = 0; // offset of line 0

	for (i = 0; i < H; i++)
	{
		RawStart = s; // start of RAW string

		for (j = 0; j < W; j++)
		{
			// load next pixel
			c = s[j];

			// flush RLE if cannot repeat
			if ((RleNum > 0) && (c != RlePix))
			{
				FlushRle();
			}

			// increase RLE
			RlePix = c;
			RleNum++;
		}

		// flush strings on end of row
		FlushRle();
		FlushRaw();
		s += WB;

		// offset of next line
		RleOff[i+1] = (int)(RleDst - RleBuf);
	}

	// number of bytes in buffer
	RleNum = (int)(RleDst - RleBuf);
}

// ============================================================================
//                             RLE4 compression
// ============================================================================

// flush RAW4 string
void FlushRaw4()
{
	while (RawNum > 0)
	{
		// prepare length of one sub-string
		int rawnum = RawNum;
		if (rawnum > 8) rawnum = 8;
		RawNum -= rawnum;

		// write raw pixels (flag byte: 0..7 means 1..8 raw pixels)
		WriteByte(rawnum-1); // number of pixels
		for (; rawnum > 0; rawnum--) WriteByte(*RawStart++);
	}
}

// flush RLE4 string
void FlushRle4()
{
	while (RleNum > 0)
	{
		// prepare lenth of one sub-string
		int rlenum = RleNum;
		if (rlenum > 10) rlenum = 10;
		RleNum -= rlenum;

		// convert short RLE string to RAW string
		if (rlenum < 3)
		{
			RawNum += rlenum;
		}

		// write long RLE string
		else
		{
			// at first, flush RAW string
			FlushRaw4();

			// write RLE string (flag byte: 8..15 means 3..10 raw pixels)
			WriteByte((rlenum-3)+8); // number of pixels
			WriteByte(RlePix); // write RLE pixel

			// shift RAW start
			RawStart += rlenum;
		}
	}
}

// do RLE4 compression
void DoRle4()
{
	u8 c;
	int i, j;
	u8* s = D;
	RleDst = RleBuf;	// destination pointer	
	RleNum = 0; // reset counter of equal pixels
	RawNum = 0; // reset counter of raw pixels
	RleOff[0] = 0; // offset of line 0

	for (i = 0; i < H; i++)
	{
		RawStart = s; // start of RAW string

		for (j = 0; j < W; j++)
		{
			// load next pixel
			c = s[j];

			// flush RLE if cannot repeat
			if ((RleNum > 0) && (c != RlePix))
			{
				FlushRle4();
			}

			// increase RLE
			RlePix = c;
			RleNum++;
		}

		// flush strings on end of row
		FlushRle4();
		FlushRaw4();
		s += WB*2;

		// offset of next line
		RleOff[i+1] = (int)(RleDst - RleBuf);
	}

	// number of bytes in buffer
	RleNum = (int)(RleDst - RleBuf);
}

// unpack 4-bit image
void Unpack4()
{
	u8* s = &D[WB*H];
	u8* d = &D[WB*2*H];
	int i;
	for (i = WB*H; i > 0; i--)
	{
		d--;
		s--;
		*d = *s & 0x0f;
		d--;
		*d = *s >> 4;
	}
}

// ============================================================================
//                            HSTX compression
// ============================================================================

// HSTX RLE compression
BOOL Hstx = FALSE; // flag - do HSTX compression
u16* HstxIn; // input buffer in 15-bit or 16-bit format
int* HstxOff; // buffer of line offsets
u16* HstxBuf; // HSTX output buffer
u16* HstxDst; // HSTX RLE destination pointer
int HstxRawNum; // counter of HSTX raw data
u16* HstxRawStart; // start of HSTX raw data
int HstxRleNum; // result size of data in HSTX output buffer, counter of HSTX RLE equal pixels
u16 HstxRlePix; // last HSTX RLE pixel

#define HSTX_CMD_TMDS			(0x2u << 12)	// write TMDS mark - raw data
#define HSTX_CMD_TMDS_REPEAT	(0x3u << 12)	// write TMDS mark - repeated data
#define HSTX_MAXLEN				4095			// max. length of HSTX string

// write word into HSTX RLE buffer
void WriteHstx(u16 w)
{
	*HstxDst++ = w;
}

// flush HSTX RAW string
void FlushHstxRaw()
{
	while (HstxRawNum > 0)
	{
		// prepare length of one sub-string
		int rawnum = HstxRawNum;
		if (rawnum > HSTX_MAXLEN) rawnum = HSTX_MAXLEN;
		HstxRawNum -= rawnum;

		// write raw pixels
		WriteHstx(HSTX_CMD_TMDS | rawnum); // number of pixels
		for (; rawnum > 0; rawnum--) WriteHstx(*HstxRawStart++);
	}
}

// flush HSTX RLE string
void FlushHstxRle()
{
	while (HstxRleNum > 0)
	{
		// prepare lenth of one sub-string
		int rlenum = HstxRleNum;
		if (rlenum > HSTX_MAXLEN) rlenum = HSTX_MAXLEN;
		HstxRleNum -= rlenum;

		// convert short RLE string to RAW string
		if (rlenum < 3)
		{
			HstxRawNum += rlenum;
		}

		// write long RLE string
		else
		{
			// at first, flush RAW string
			FlushHstxRaw();

			// write RLE string
			WriteHstx(HSTX_CMD_TMDS_REPEAT | rlenum); // number of pixels
			WriteHstx(HstxRlePix); // write RLE pixel

			// shift RAW start
			HstxRawStart += rlenum;
		}
	}
}

// do HSTX RLE compression
void DoHstxRle(FILE* f, char* argv[])
{
	u16 c;
	int i, j;
	u16* s = HstxIn; // input buffer
	HstxDst = HstxBuf;	// destination pointer	
	HstxRawNum = 0; // reset counter of raw pixels
	HstxRleNum = 0; // reset counter of equal pixels
	HstxOff[0] = 0; // offset of line 0

	for (i = 0; i < H; i++)
	{
		HstxRawStart = s; // start of RAW string

		for (j = 0; j < W; j++)
		{
			// load next pixel
			c = s[j];

			// flush RLE if cannot repeat
			if ((HstxRleNum > 0) && (c != HstxRlePix))
			{
				FlushHstxRle();
			}

			// increase RLE
			HstxRlePix = c;
			HstxRleNum++;
		}

		// flush strings on end of row
		FlushHstxRle();
		FlushHstxRaw();
		s = (u16*)((u8*)s + WB);

		// offset of next line
		HstxOff[i+1] = (int)(HstxDst - HstxBuf);
	}

	// number of bytes in buffer
	HstxRleNum = (int)(HstxDst - HstxBuf);

	// info header
	fprintf(f, "#include \"../include.h\"\n\n");
	fprintf(f, "// format: %d-bit pixel graphics with HSTX RLE compression\n", (B == 16) ? 15 : 16);
	fprintf(f, "// compression ratio: %d%%\n", (HstxRleNum*100+W*H/2)/(W*H));
	fprintf(f, "// required memory: %d B offsets, %d B data\n", (H+1)*2, HstxRleNum*2);
	fprintf(f, "// image width: %d pixels\n", W);
	fprintf(f, "// image height: %d lines\n", H);
	fprintf(f, "const u32 %s_Off[%d] __attribute__ ((aligned(4))) = {\n", argv[3], H+1);

	// line offsets
	for (i = 0; i < H; i++)
	{
		fprintf(f, "\t%d,\t// line %d, length %d\n", HstxOff[i], i, HstxOff[i+1] - HstxOff[i]);
	}
	fprintf(f, "\t%d,\t// end of last line\n};\n\n", HstxOff[i]);

	// image data
	fprintf(f, "const u16 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], HstxRleNum);

	// load image
	j = 0;
	for (i = 0; i < HstxRleNum; i++)
	{
		if ((j & 0x0f) == 0) fprintf(f, "\n\t");
		fprintf(f, "0x%04X, ", HstxBuf[i]);
		j++;
	}
}

// ============================================================================
//                           1-bit input image
// ============================================================================

void Do1Bit(FILE* f, char* argv[])
{
	int i, j, n;

	// info header
	int wb = (W+7)/8;
	wb = (wb + 3) & ~3;
	fprintf(f, "#include \"../include.h\"\n\n");
	fprintf(f, "// format: 1-bit pixel graphics\n");
	fprintf(f, "// image width: %d pixels\n", W);
	fprintf(f, "// image height: %d lines\n", H);
	fprintf(f, "// image pitch: %d bytes\n", wb);

	// image header
	fprintf(f, "const u8 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], wb*H);

	// load image
	n = 0;
	for (i = 0; i < H; i++)
	{
		for (j = 0; j < wb; j++)
		{
			if ((n & 0x0f) == 0) fprintf(f, "\n\t");
			fprintf(f, "0x%02X, ", D[j]);
			n++;
		}
		D += WB;
	}
}

// ============================================================================
//                           4-bit input image
// ============================================================================

// 4-bit Attribute
void Do4BitAttr(FILE* f, char* argv[], FILE* f2, int size, int off)
{
	int i, n;

	// unpack image to 8-bit format
	Unpack4();

	// prepare buffers
	AttrInx = (u8*)malloc(W*H);
	AttrCol = (u8*)malloc(W*H*ATTR_MAXCOL);
	AttrTest = (u8*)malloc(size);
	if ((AttrInx == NULL) || (AttrCol == NULL) || (AttrTest == NULL))
	{
		printf("Memory error\n");
		return;
	}

	// number of palettes
	u8* pal = Pal;
	if (PalN == 0) PalN = 16;
	if (PalN > 16) PalN = 16;

	// do attribute compression
	DoAttr(Pal, PalN, f2, size, off);

	// info header
	int wb = (W+1)/2;
	wb = (wb + 3) & ~3;
	int pixsize = W*H/8*AttrOutBits;
	int attrsize = W/AttrCW*H/AttrCH*AttrColNum/2;
	int fullsize = wb*H;

	fprintf(f, "#include \"../include.h\"\n\n");
	fprintf(f, "// format: Attribute compressed %dx%d/%d colors %d-bit paletted pixel graphics\n",
				AttrCW, AttrCH, AttrColNum, AttrInBits);
	fprintf(f, "// compression ratio: %d%%\n", (pixsize + attrsize)*100/fullsize);
	fprintf(f, "// required memory: %d B pixels, %d B attributes (%d B total, unpacked %d B)\n",
				pixsize, attrsize, pixsize + attrsize, fullsize);
	fprintf(f, "// image width: %d pixels\n", W);
	fprintf(f, "// image height: %d lines\n", H);
	fprintf(f, "// image pixels pitch: %d bytes\n", W/8*AttrOutBits*((OutFormat == FORMAT_ATTR2) ? 2 : 1));
	fprintf(f, "// image attribute pitch: %d bytes\n", W/AttrCW*AttrColNum/2);

	// palette header
	fprintf(f, "const u16 %s_Pal[%d] __attribute__ ((aligned(4))) = {", argv[3], PalN);

	// palettes
	n = 0;
	for (i = 0; i < PalN; i++)
	{
		if ((n & 0x0f) == 0) fprintf(f, "\n\t");
		u8 blue = pal[0];
		u8 green = pal[1];
		u8 red = pal[2];
		u16 b = (blue >> 3) | ((green >> 2) << 5) | ((red >> 3) << 11);
		fprintf(f, "0x%04X, ", b);
		n++;
		pal += 4;
	}

	// attribute data
	fprintf(f, "\n};\n\nconst u8 %s_Attr[%d] __attribute__ ((aligned(4))) = {", argv[3], attrsize);

	int wn = W/AttrCW;
	int hn = H/AttrCH;
	wb = WB * 2;
	int coln = AttrColNum;
	int x, y;
	n = 0;
	u8* src;
	for (y = 0; y < hn; y++)
	{
		for (x = 0; x < wn; x++)
		{
			src = &AttrCol[(y*wb + x)*coln];
			for (i = 0; i < coln; i += 2)
			{
				if ((n & 0x0f) == 0) fprintf(f, "\n\t");
				fprintf(f, "0x%02X, ", (src[i] << 4) | src[i+1]);
				n++;
			}
		}
	}

	// pixel data
	fprintf(f, "\n};\n\nconst u8 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], pixsize);

	n = 0;
	if (OutFormat == FORMAT_ATTR2)
	{
		for (y = 0; y < H; y += 2)
		{
			for (x = 0; x < W; x += 4)
			{
				src = &AttrInx[y*wb + x];
				if ((n & 0x0f) == 0) fprintf(f, "\n\t");
				fprintf(f, "0x%02X, ", (src[0] << 7) | (src[1] << 6) | (src[2] << 5) | (src[3] << 4) | 
						(src[0+wb] << 3) | (src[1+wb] << 2) | (src[2+wb] << 1) | src[3+wb]);
				n++;
			}
		}
	}
	else
	{
		int k = 8/AttrOutBits; // number of pixels per byte (8 or 4)
		for (y = 0; y < H; y++)
		{
			for (x = 0; x < W; x += k)
			{
				src = &AttrInx[y*wb + x];
				if ((n & 0x0f) == 0) fprintf(f, "\n\t");
				if (k == 8)
				{
					fprintf(f, "0x%02X, ", (src[0] << 7) | (src[1] << 6) | (src[2] << 5) | (src[3] << 4) | 
							(src[4] << 3) | (src[5] << 2) | (src[6] << 1) | src[7]);
				}
				else
				{
					fprintf(f, "0x%02X, ", (src[0] << 6) | (src[1] << 4) | (src[2] << 2) | src[3]);
				}
				n++;
			}
		}
	}

	free(AttrInx);
	free(AttrCol);
	free(AttrTest);
}

// 4-bit
void Do4Bit(FILE* f, char* argv[], FILE* f2, int size, int off)
{
	int i, j, n;

	// Attribute compression
	if (AttrMode)
	{
		Do4BitAttr(f, argv, f2, size, off);
	}

	// 2-bit output format
	else if (OutFormat == FORMAT_2)
	{
		int wb = (W+3)/4;
		wb = (wb + 3) & ~3;

		// info header
		fprintf(f, "#include \"../include.h\"\n\n");
		fprintf(f, "// format: 2-bit paletted pixel graphics\n");
		fprintf(f, "// image width: %d pixels\n", W);
		fprintf(f, "// image height: %d lines\n", H);
		fprintf(f, "// image pitch: %d bytes\n", wb);

		// palettes
		PrintPal(f, argv, 4);

		// image header
		fprintf(f, "\n};\n\nconst u8 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], wb*H);

		// load image
		n = 0;
		for (i = 0; i < H; i++)
		{
			for (j = 0; j < wb; j++)
			{
				if ((n & 0x0f) == 0) fprintf(f, "\n\t");

				fprintf(f, "0x%02X, ", ((D[j*2] & 0x30)<<2) | ((D[j*2] & 0x03)<<4) | ((D[j*2+1] & 0x30)>>2) | ((D[j*2+1] & 0x03)));
				n++;
			}
			D += WB;
		}
	}
	// 3-bit output format
	else if (OutFormat == FORMAT_3)
	{
		int wb32 = (W+9)/10;

		// info header
		fprintf(f, "#include \"../include.h\"\n\n");
		fprintf(f, "// format: 3-bit paletted pixel graphics (10 pixels per u32)\n");
		fprintf(f, "// image width: %d pixels\n", W);
		fprintf(f, "// image height: %d lines\n", H);
		fprintf(f, "// image pitch: %d bytes\n", wb32*4);

		// palettes
		PrintPal(f, argv, 8);

		// image header
		fprintf(f, "\n};\n\nconst u32 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], wb32*H);

		// load image
		n = 0;
		u8* s;
		for (i = 0; i < H; i++)
		{
			for (j = 0; j < wb32; j++)
			{
				if ((n & 0x07) == 0) fprintf(f, "\n\t");

				s = &D[j*5];
				fprintf(f, "0x%08X, ",
					((s[0] >> 4) & 0x07) |
					((s[0] & 0x07) << 3) |
					(((s[1] >> 4) & 0x07) << 6) |
					((s[1] & 0x07) << 9) |
					(((s[2] >> 4) & 0x07) << 12) |
					((s[2] & 0x07) << 15) |
					(((s[3] >> 4) & 0x07) << 18) |
					((s[3] & 0x07) << 21) |
					(((s[4] >> 4) & 0x07) << 24) |
					((s[4] & 0x07) << 27));
				n++;
			}
			D += WB;
		}
	}
	else
	{
		int wb = (W+1)/2;
		wb = (wb + 3) & ~3;

		// do RLE compression
		RleBuf = NULL;
		RleOff = NULL;
		RleNum = 0;
		if (Rle)
		{
			// prepare RLE buffer
			RleBuf = (u8*)malloc(W*2*H);
			RleOff = (int*)malloc((H+1)*sizeof(int));
			if ((RleBuf == NULL) || (RleOff == NULL))
			{
				printf("Memory error\n");
				return;
			}

			// do compression
			Unpack4();
			DoRle4();
		}

		// info header
		fprintf(f, "#include \"../include.h\"\n\n");
		if (Rle)
		{
			fprintf(f, "// format: RLE compressed 4-bit paletted pixel graphics\n");
			fprintf(f, "// compression ratio: %d%%\n", (RleNum*100+wb*H/2)/(wb*H));
		}
		else
			fprintf(f, "// format: 4-bit paletted pixel graphics\n");
		fprintf(f, "// image width: %d pixels\n", W);
		fprintf(f, "// image height: %d lines\n", H);
		if (!Rle) fprintf(f, "// image pitch: %d bytes\n", wb);

		// RLE offset table
		if (Rle)
		{
			fprintf(f, "const u32 %s_Off[%d] __attribute__ ((aligned(4))) = {\n", argv[3], H+1);
			for (i = 0; i < H; i++)
			{
				fprintf(f, "\t%d,\t// line %d, length %d\n", RleOff[i], i, RleOff[i+1] - RleOff[i]);
			}
			fprintf(f, "\t%d,\t// end of last line\n};\n\n", RleOff[i]);
		}

		// palettes
		PrintPal(f, argv, 16);

		// compressed image
		if (Rle)
		{
			// image header
			fprintf(f, "\n};\n\nconst u8 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], (RleNum+1)/2);

			// load image
			n = 0;
			u8* s = RleBuf;
			for (i = 0; i < RleNum; i += 2)
			{
				if ((n & 0x0f) == 0) fprintf(f, "\n\t");
				u8 bb = *s++ << 4;
				bb |= *s++;
				fprintf(f, "0x%02X, ", bb);
				n++;
			}
		}

		// uncompressed image
		else
		{
			// image header
			fprintf(f, "\n};\n\nconst u8 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], wb*H);

			// load image
			n = 0;
			for (i = 0; i < H; i++)
			{
				for (j = 0; j < wb; j++)
				{
					if ((n & 0x0f) == 0) fprintf(f, "\n\t");
					fprintf(f, "0x%02X, ", D[j]);
					n++;
				}
				D += WB;
			}
		}

		free(RleBuf);
		free(RleOff);
	}
}

// ============================================================================
//                           8-bit input image
// ============================================================================

// 8-bit Attribute
void Do8BitAttr(FILE* f, char* argv[], FILE* f2, int size, int off)
{
	int i, n;

	// prepare buffers
	AttrInx = (u8*)malloc(W*H);
	AttrCol = (u8*)malloc(W*H*ATTR_MAXCOL);
	AttrTest = (u8*)malloc(size);
	if ((AttrInx == NULL) || (AttrCol == NULL) || (AttrTest == NULL))
	{
		printf("Memory error\n");
		return;
	}

	// number of palettes
	u8* pal = Pal;
	if (PalN == 0) PalN = 256;
	if (PalN > 256) PalN = 256;

	// do attribute compression
	DoAttr(pal, PalN, f2, size, off);

	// info header
	int wb = W;
	wb = (wb + 3) & ~3;
	int pixsize = W*H/8*AttrOutBits;
	int attrsize = W/AttrCW*H/AttrCH*AttrColNum;
	int fullsize = wb*H;

	fprintf(f, "#include \"../include.h\"\n\n");
	fprintf(f, "// format: Attribute compressed %dx%d/%d colors %d-bit paletted pixel graphics\n",
				AttrCW, AttrCH, AttrColNum, AttrInBits);
	fprintf(f, "// compression ratio: %d%%\n", (pixsize + attrsize)*100/fullsize);
	fprintf(f, "// required memory: %d B pixels, %d B attributes (%d B total, unpacked %d B)\n",
				pixsize, attrsize, pixsize + attrsize, fullsize);
	fprintf(f, "// image width: %d pixels\n", W);
	fprintf(f, "// image height: %d lines\n", H);
	fprintf(f, "// image pixels pitch: %d bytes\n", W/8*AttrOutBits*((OutFormat == FORMAT_ATTR6) ? 2 : 1));
	fprintf(f, "// image attribute pitch: %d bytes\n", W/AttrCW*AttrColNum);

	// palette header
	fprintf(f, "const u16 %s_Pal[%d] __attribute__ ((aligned(4))) = {", argv[3], PalN);

	// palettes
	n = 0;
	for (i = 0; i < PalN; i++)
	{
		if ((n & 0x0f) == 0) fprintf(f, "\n\t");
		u8 blue = pal[0];
		u8 green = pal[1];
		u8 red = pal[2];
		u16 b = (blue >> 3) | ((green >> 2) << 5) | ((red >> 3) << 11);
		fprintf(f, "0x%04X, ", b);
		n++;
		pal += 4;
	}

	// attribute data
	fprintf(f, "\n};\n\nconst u8 %s_Attr[%d] __attribute__ ((aligned(4))) = {", argv[3], attrsize);

	int wn = W/AttrCW;
	int hn = H/AttrCH;
	wb = WB;
	int coln = AttrColNum;
	int x, y;
	n = 0;
	u8* src;
	for (y = 0; y < hn; y++)
	{
		for (x = 0; x < wn; x++)
		{
			src = &AttrCol[(y*wb + x)*coln];
			for (i = 0; i < coln; i++)
			{
				if ((n & 0x0f) == 0) fprintf(f, "\n\t");
				fprintf(f, "0x%02X, ", src[i]);
				n++;
			}
		}
	}

	// pixel data
	fprintf(f, "\n};\n\nconst u8 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], pixsize);

	n = 0;
	if (OutFormat == FORMAT_ATTR6)
	{
		for (y = 0; y < H; y += 2)
		{
			for (x = 0; x < W; x += 4)
			{
				src = &AttrInx[y*wb + x];
				if ((n & 0x0f) == 0) fprintf(f, "\n\t");
				fprintf(f, "0x%02X, ", (src[0] << 7) | (src[1] << 6) | (src[2] << 5) | (src[3] << 4) | 
						(src[0+wb] << 3) | (src[1+wb] << 2) | (src[2+wb] << 1) | src[3+wb]);
				n++;
			}
		}
	}
	else
	{
		int k = 8/AttrOutBits; // number of pixels per byte (4 or 8)
		for (y = 0; y < H; y++)
		{
			for (x = 0; x < W; x += k)
			{
				src = &AttrInx[y*wb + x];
				if ((n & 0x0f) == 0) fprintf(f, "\n\t");
				if (k == 8)
				{
					fprintf(f, "0x%02X, ", (src[0] << 7) | (src[1] << 6) | (src[2] << 5) | (src[3] << 4) | 
							(src[4] << 3) | (src[5] << 2) | (src[6] << 1) | src[7]);
				}
				else // if (k == 4)
				{
					fprintf(f, "0x%02X, ", (src[0] << 6) | (src[1] << 4) | (src[2] << 2) | src[3]);
				}
				n++;
			}
		}
	}

	free(AttrInx);
	free(AttrCol);
	free(AttrTest);
}

// 8-bit
void Do8Bit(FILE* f, char* argv[], FILE* f2, int size, int off)
{
	int i, j, n;

	// Attribute compression
	if (AttrMode)
	{
		Do8BitAttr(f, argv, f2, size, off);
	}
	// 6-bit output format
	else if (OutFormat == FORMAT_6)
	{
		int wb32 = (W+4)/5;

		// info header
		fprintf(f, "#include \"../include.h\"\n\n");
		fprintf(f, "// format: 6-bit paletted pixel graphics (5 pixels per u32)\n");
		fprintf(f, "// image width: %d pixels\n", W);
		fprintf(f, "// image height: %d lines\n", H);
		fprintf(f, "// image pitch: %d bytes\n", wb32*4);

		// palettes
		PrintPal(f, argv, 64);

		// image header
		fprintf(f, "\n};\n\nconst u32 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], wb32*H);

		// load image
		n = 0;
		u8* s;
		for (i = 0; i < H; i++)
		{
			for (j = 0; j < wb32; j++)
			{
				if ((n & 0x07) == 0) fprintf(f, "\n\t");

				s = &D[j*5];
				fprintf(f, "0x%08X, ",
					(s[0] & 0x3f) |
					((s[1] & 0x3f) << 6) |
					((s[2] & 0x3f) << 12) |
					((s[3] & 0x3f) << 18) |
					((s[4] & 0x3f) << 24));
				n++;
			}
			D += WB;
		}
	}
	else
	{
		int wb = W;
		wb = (wb + 3) & ~3;

		// do RLE compression
		RleBuf = NULL;
		RleOff = NULL;
		RleNum = 0;
		if (Rle)
		{
			// prepare RLE buffer
			RleOff = (int*)malloc((H+1)*sizeof(int));
			RleBuf = (u8*)malloc(W*2*H);
			if ((RleBuf == NULL) || (RleOff == NULL))
			{
				printf("Memory error\n");
				return;
			}

			// do compression
			DoRle();
		}

		// info header
		fprintf(f, "#include \"../include.h\"\n\n");
		if (Rle)
		{
			fprintf(f, "// format: RLE compressed 8-bit paletted pixel graphics\n");
			fprintf(f, "// compression ratio: %d%%\n", (RleNum*100+wb*H/2)/(wb*H));
		}
		else
			fprintf(f, "// format: 8-bit paletted pixel graphics\n");
		fprintf(f, "// image width: %d pixels\n", W);
		fprintf(f, "// image height: %d lines\n", H);
		if (!Rle) fprintf(f, "// image pitch: %d bytes\n", wb);

		// RLE offset table
		if (Rle)
		{
			fprintf(f, "const u32 %s_Off[%d] __attribute__ ((aligned(4))) = {\n", argv[3], H+1);
			for (i = 0; i < H; i++)
			{
				fprintf(f, "\t%d,\t// line %d, length %d\n", RleOff[i], i, RleOff[i+1] - RleOff[i]);
			}
			fprintf(f, "\t%d,\t// end of last line\n};\n\n", RleOff[i]);
		}

		// palettes
		PrintPal(f, argv, 256);

		// compressed image
		if (Rle)
		{
			// image header
			fprintf(f, "\n};\n\nconst u8 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], RleNum);

			// load image
			n = 0;
			u8* s = RleBuf;
			for (i = 0; i < RleNum; i++)
			{
				if ((n & 0x0f) == 0) fprintf(f, "\n\t");
				fprintf(f, "0x%02X, ", *s++);
				n++;
			}
		}

		// uncompressed image
		else
		{
			// image header
			fprintf(f, "\n};\n\nconst u8 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], wb*H);

			// load image
			n = 0;
			for (i = 0; i < H; i++)
			{
				for (j = 0; j < wb; j++)
				{
					if ((n & 0x0f) == 0) fprintf(f, "\n\t");
					fprintf(f, "0x%02X, ", D[j]);
					n++;
				}
				D += WB;
			}
		}

		free(RleBuf);
		free(RleOff);
	}
}

// ============================================================================
//                           15-bit input image
// ============================================================================

// 15-bit
void Do15Bit(FILE* f, char* argv[])
{
	int i, j, k, n, t;

	// HSTX compression
	if (Hstx)
	{
		// prepare buffers
		HstxOff = (int*)malloc((H+1)*sizeof(int));
		HstxBuf = (u16*)malloc(W*2*H*2);
		if ((HstxOff == NULL) || (HstxBuf == NULL))
		{
			printf("Memory error\n");
			return;
		}

		// do compression
		HstxIn = (u16*)D;
		DoHstxRle(f, argv);

		free(HstxOff);
		free(HstxBuf);
	}
	// 12-bit output format
	else if ((OutFormat == FORMAT_12) || (OutFormat == FORMAT_12D))
	{
		int wb32 = (W+7)/8*3;

		// info header
		fprintf(f, "#include \"../include.h\"\n\n");
		if (OutFormat == FORMAT_12D)
			fprintf(f, "// format: 12-bit pixel graphics with dithering (8 pixels per three 32-bit)\n");
		else
			fprintf(f, "// format: 12-bit pixel graphics (8 pixels per three 32-bit)\n");
		fprintf(f, "// image width: %d pixels\n", W);
		fprintf(f, "// image height: %d lines\n", H);
		fprintf(f, "// image pitch: %d bytes\n", wb32*4);

		// image header
		fprintf(f, "const u32 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], wb32*H);

		// load image
		n = 0;
		u16* s;
		int r, g, b;
		int m[8];
		for (i = 0; i < H; i++)
		{
			for (j = 0; j < (W+7)/8; j++)
			{
				s = (u16*)&D[j*16];

				// read words
				for (k = 0; k < 8; k++)
				{
					t = s[k];
					if ((OutFormat == FORMAT_12D) && (((i ^ k) & 1) != 0))
					{
						r = (((t >> 10) & 0x1f) + 1) >> 1;
						g = (((t >> 5) & 0x1f) + 1) >> 1;
						b = ((t & 0x1f) + 1) >> 1;

						if (r > 0x0f) r = 0x0f;
						if (g > 0x0f) g = 0x0f;
						if (b > 0x0f) b = 0x0f;
					}
					else
					{
						r = (t >> 11) & 0x0f;
						g = (t >> 6) & 0x0f;
						b = (t >> 1) & 0x0f;
					}
					m[k] = b | (g << 4) | (r << 8);
				}

				if ((n & 0x07) == 0) fprintf(f, "\n\t");
				fprintf(f, "0x%08X, ", m[0] | (m[1] << (3*4)) | (m[2] << (6*4)));
				n++;

				if ((n & 0x07) == 0) fprintf(f, "\n\t");
				fprintf(f, "0x%08X, ", (m[2] >> (2*4)) | (m[3] << (1*4)) | (m[4] << (4*4)) | (m[5] << (7*4)));
				n++;

				if ((n & 0x07) == 0) fprintf(f, "\n\t");
				fprintf(f, "0x%08X, ", (m[5] >> (1*4)) | (m[6] << (2*4)) | (m[7] << (5*4)));
				n++;
			}
			D += WB;
		}
	}
	else
	{
		int wb = W*2;
		wb = (wb + 3) & ~3;

		// info header
		fprintf(f, "#include \"../include.h\"\n\n");
		fprintf(f, "// format: 15-bit pixel graphics\n");
		fprintf(f, "// image width: %d pixels\n", W);
		fprintf(f, "// image height: %d lines\n", H);
		fprintf(f, "// image pitch: %d bytes\n", wb);
		fprintf(f, "const u16 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], wb/2*H);

		// load image
		DD = (u16*)D;
		WB /= 2;
		n = 0;
		for (i = 0; i < H; i++)
		{
			for (j = 0; j < wb/2; j++)
			{
				if ((n & 0x0f) == 0) fprintf(f, "\n\t");
				fprintf(f, "0x%04X, ", DD[j] & 0x7fff);
				n++;
			}
			DD += WB;
		}
	}
}

// ============================================================================
//                           24-bit input image
// ============================================================================

// 24-bit
void Do24Bit(FILE* f, char* argv[])
{
	int i, j, n;

	// HSTX compression
	if (Hstx)
	{
		// prepare buffers
		HstxIn = (u16*)malloc(W*2*H);
		HstxOff = (int*)malloc((H+1)*sizeof(int));
		HstxBuf = (u16*)malloc(W*2*H*2);
		if ((HstxIn == NULL) || (HstxOff == NULL) || (HstxBuf == NULL))
		{
			printf("Memory error\n");
			return;
		}

		// convert image from 24 bits to 16 bits
		u16* dst = HstxIn;
		for (i = 0; i < H; i++)
		{
			for (j = 0; j < W; j++)
			{
				u8 blue = D[j*3];
				u8 green = D[j*3+1];
				u8 red = D[j*3+2];
				u16 b = (blue >> 3) | ((green >> 2) << 5) | ((red >> 3) << 11);
				*dst++ = b;
			}
			D += WB;
		}

		// do compression
		WB = W*2;
		DoHstxRle(f, argv);

		free(HstxIn);
		free(HstxOff);
		free(HstxBuf);
	}
	else
	{
		int wb = W*2;
		wb = (wb + 3) & ~3;

		// info header
		fprintf(f, "#include \"../include.h\"\n\n");
		fprintf(f, "// format: 16-bit pixel graphics\n");
		fprintf(f, "// image width: %d pixels\n", W);
		fprintf(f, "// image height: %d lines\n", H);
		fprintf(f, "// image pitch: %d bytes\n", wb);
		fprintf(f, "const u16 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], wb/2*H);

		// load image
		n = 0;
		for (i = 0; i < H; i++)
		{
			for (j = 0; j < wb/2; j++)
			{
				if ((n & 0x0f) == 0) fprintf(f, "\n\t");
				u8 blue = D[j*3];
				u8 green = D[j*3+1];
				u8 red = D[j*3+2];
				u16 b = (blue >> 3) | ((green >> 2) << 5) | ((red >> 3) << 11);
				fprintf(f, "0x%04X, ", b);
				n++;
			}
			D += WB;
		}
	}
}

// ============================================================================
//                              Main
// ============================================================================

void Help()
{
		printf("PicoPadImg version 2.0, (c) 2025 Miroslav Nemecek\n");
		printf("Syntax: input.bmp output.cpp name format [test.bmp]\n");
		printf("  'input.bmp' input image in BMP format\n");
		printf("  'output.cpp' output file as C++ source\n");
		printf("  'name' name of data array\n");
		printf("  'format' specifies required output format:\n");
		printf("       1 ... 1-bit Y1 (8 pixels per byte), requires 1-bit input image\n");
		printf("       2 ... 2-bit Y2 (4 pixels per byte), requires 4-bit input image with 4 palettes\n");
		printf("       3 ... 3-bit RGB111 (10 pixels per 32-bit), requires 4-bit input image with 8 palettes\n");
		printf("       4 ... 4-bit YRGB1111 (2 pixels per byte), requires 4-bit input image\n");
		printf("       6 ... 6-bit RGB222 (5 pixels per 32-bit), requires 8-bit input image\n");
		printf("       8 ... 8-bit RGB332 (1 pixel per byte), requires 8-bit input image\n");
		printf("       12 ... 12-bit RGB444 (8 pixels per three 32-bit), requires 15-bit input image\n");
		printf("       12d ... 12-bit RGB444 with dithering, requires 15-bit input image\n");
		printf("       15 ... 15-bit RGB555 (1 pixel per 2 bytes), requires 15-bit input image\n");
		printf("       16 ... 16-bit RGB565 (1 pixel per 2 bytes), requires 24-bit input image\n");
		printf("       r4 ... 4-bit compression RLE4, requires 4-bit input image\n");
		printf("       r8 ... 8-bit compression RLE8, requires 8-bit input image\n");
		printf("       x15 ... 15-bit HSTX compression, requires 15-bit input image\n");
		printf("       x16 ... 16-bit HSTX compression, requires 24-bit input image\n");
		printf("       a1 ... attribute compression 1, cell 8x8, 2 colors, requires 4-bit input image\n");
		printf("       a2 ... attribute compression 2, cell 4x4, 2 colors, requires 4-bit input image\n");
		printf("       a3 ... attribute compression 3, cell 8x8, 4 colors, requires 4-bit input image\n");
		printf("       a4 ... attribute compression 4, cell 4x4, 4 colors, requires 4-bit input image\n");
		printf("       a5 ... attribute compression 5, cell 8x8, 2 colors, requires 8-bit input image\n");
		printf("       a6 ... attribute compression 6, cell 4x4, 2 colors, requires 8-bit input image\n");
		printf("       a7 ... attribute compression 7, cell 8x8, 4 colors, requires 8-bit input image\n");
		printf("       a8 ... attribute compression 8, cell 4x4, 4 colors, requires 8-bit input image\n");
		printf("  'test.bmp' output test image of attribute compression\n");
}

// main function
int main(int argc, char* argv[])
{
	int i;

	if ((sizeof(_bmpBITMAPFILEHEADER) != 14) ||
		(sizeof(_bmpBITMAPINFOHEADER) != 40))
	{
		printf("Incorrect size of data types. Check typedef of s32/u32 or do 32-bit compilation.\n");
		return 1;
	}

	// print input file
	if (argc == 4) printf("%s -> %s\n", argv[1], argv[2]);
	if (argc > 4) printf("%s -> %s, format: %s\n", argv[1], argv[2], argv[4]);

	// base check syntax
	if ((argc != 5) && (argc != 6))
	{
		Help();
		return 1;
	}

	// search flags
	for (i = 0; i < FORMAT_NUM; i++)
	{
		if (strcmp(argv[4], SwitchTxt[i]) == 0)
		{
			OutFormat = i;
			if ((i == FORMAT_RLE4) || (i == FORMAT_RLE8)) Rle = TRUE;
			if ((i == FORMAT_HSTX15) || (i == FORMAT_HSTX16)) Hstx = TRUE;
			if ((i >= FORMAT_ATTR_FIRST) && (i <= FORMAT_ATTR_LAST)) AttrMode = TRUE;
			break;
		}
	}

	if (i == FORMAT_NUM)
	{
		Help();
		return 1;
	}

	// attribute compression setup
	switch (OutFormat)
	{
	case FORMAT_ATTR1:	// "a1": attribute compression 1, 4-bit pixels, cell 8x8, 2 colors
			AttrInBits = 4;
			AttrCW = 8;
			AttrCH = 8;
			AttrColNum = 2;
			AttrOutBits = 1;
			break;

	case FORMAT_ATTR2:	// "a2": attribute compression 2, 4-bit pixels, cell 4x4, 2 colors
			AttrInBits = 4;
			AttrCW = 4;
			AttrCH = 4;
			AttrColNum = 2;
			AttrOutBits = 1;
			break;

	case FORMAT_ATTR3:	// "a3": attribute compression 3, 4-bit pixels, cell 8x8, 4 colors
			AttrInBits = 4;
			AttrCW = 8;
			AttrCH = 8;
			AttrColNum = 4;
			AttrOutBits = 2;
			break;

	case FORMAT_ATTR4:	// "a4": attribute compression 4, 4-bit pixels, cell 4x4, 4 colors
			AttrInBits = 4;
			AttrCW = 4;
			AttrCH = 4;
			AttrColNum = 4;
			AttrOutBits = 2;
			break;

	case FORMAT_ATTR5:	// "a5": attribute compression 5, 8-bit pixels, cell 8x8, 2 colors
			AttrInBits = 8;
			AttrCW = 8;
			AttrCH = 8;
			AttrColNum = 2;
			AttrOutBits = 1;
			break;

	case FORMAT_ATTR6:	// "a6": attribute compression 6, 8-bit pixels, cell 4x4, 2 colors
			AttrInBits = 8;
			AttrCW = 4;
			AttrCH = 4;
			AttrColNum = 2;
			AttrOutBits = 1;
			break;

	case FORMAT_ATTR7:	// "a7": attribute compression 7, 8-bit pixels, cell 8x8, 4 colors
			AttrInBits = 8;
			AttrCW = 8;
			AttrCH = 8;
			AttrColNum = 4;
			AttrOutBits = 2;
			break;

	case FORMAT_ATTR8:	// "a8": attribute compression 8, 8-bit pixels, cell 4x4, 4 colors
			AttrInBits = 8;
			AttrCW = 4;
			AttrCH = 4;
			AttrColNum = 4;
			AttrOutBits = 2;
			break;
	}

// === read input file

	// open main input file
	FILE* f = fopen(argv[1], "rb");
	if (f == NULL)
	{
		printf("Error opening input file %s\n", argv[1]);
		return 1;
	}

	// size of input file
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (size < 40)
	{
		printf("Incorrect size of input file %s\n", argv[1]);
		return 1;
	}

	// create buffer (double size to unpack 4-bit format, + some more to align)
	Img = (u8*)malloc(size*2 + 16);
	if (Img == NULL)
	{
		printf("Memory error\n");
		return 1;
	}

	// read file
	int size2 = (int)fread(Img, 1, size, f);
	fclose(f);
	if (size2 != size)
	{
		printf("Error reading input file %s\n", argv[1]);
		return 1;
	}

	// check BMP header
	bmpBITMAPFILEHEADER* bmf = (bmpBITMAPFILEHEADER*)Img;
	bmpBITMAPINFOHEADER* bmi = (bmpBITMAPINFOHEADER*)&bmf[1];
	W = bmi->biWidth;
	H = bmi->biHeight;
	B = bmi->biBitCount;
	PalN = bmi->biClrUsed;
	Pal = (u8*)&bmi[1];
	if (H < 0) H = -H;
	if ((bmf->bfType != 0x4d42) ||
		(bmf->bfOffBits < 0x30) || (bmf->bfOffBits > 0x440) ||
		(bmi->biCompression != bmpBI_RGB) ||
		(W < 4) || (W > 10000) || (H < 4) || (H > 10000) ||
		((B != 24) && (B != 16) && (B != 8) && (B != 4) && (B != 1)))
	{
		printf("Incorrect format of input file %s, must be Windows-BMP\n", argv[1]);
		printf("  24-bit, 15-bit, 8-bit, 4-bit or 1-bit uncompressed.\n");
		return 1;
	}
	D = &Img[bmf->bfOffBits];

	// check input format
	switch (OutFormat)
	{
	case FORMAT_1:	// "1": 1-bit Y1 (8 pixels per byte, MSB first pixel, LSB last pixel), requires 1-bit input image
		if (B != 1)
		{
			printf("Output format '1' requires 1-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_2:	// "2": 2-bit Y2 (4 pixels per byte, MSB first pixel, LSB last pixel), requires 4-bit input image with max. 4 palettes
		if ((B != 4) || (PalN > 4))
		{
			printf("Output format '2' requires 4-bit input image %s with max. 4 palettes\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_3:	// "3": 3-bit RGB111 (10 pixels packed in 32-bit word, LSB fist pixel, MSB last pixel), requires 4-bit input image with max. 8 palettes
		if ((B != 4) || (PalN > 8))
		{
			printf("Output format '3' requires 4-bit input image %s with max. 8 palettes\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_4:	// "4": 4-bit YRGB1111 (2 pixels per byte, MSB first pixel, LSB last pixel), requires 4-bit input image with max. 16 palettes
		if (B != 4)
		{
			printf("Output format '4' requires 4-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_6:	// "6": 6-bit RGB222 (5 pixels packed in 32-bit word, LSB fist pixel, MSB last pixel), requires 8-bit input image with max. 64 palettes
		if ((B != 8) || (PalN > 64))
		{
			printf("Output format '6' requires 8-bit input image %s with max. 64 palettes\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_8:	// "8": 8-bit RGB332 (1 pixel per byte), requires 8-bit input image with max. 256 palettes
		if (B != 8)
		{
			printf("Output format '8' requires 8-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_12:	// "12": 12-bit RGB444 (2 pixels packed in 3 bytes, LSB fist pixel, MSB last pixel), requires 15-bit input image
	case FORMAT_12D: // "12d": 12-bit RGB444 with dithering
		if (B != 16)
		{
			printf("Output formats '12' and '12d' require 15-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_15:	// "15": 15-bit RGB555 (1 pixel per 2 bytes), requires 15-bit input image
		if (B != 16)
		{
			printf("Output format '15' requires 15-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_16:	// "16": 16-bit RGB565 (1 pixel per 2 bytes), requires 24-bit input image
		if (B != 24)
		{
			printf("Output format '16' requires 24-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_RLE4:	// "r4": 4-bit compression RLE4, requires 4-bit input image with max. 16 palettes
		if (B != 4)
		{
			printf("Output format 'r4' requires 4-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_RLE8:	// "r8": 8-bit compression RLE8, requires 8-bit input image with max. 256 palettes
		if (B != 8)
		{
			printf("Output format 'r8' requires 8-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_HSTX15:	// "x15": 15-bit HSTX compression, requires 15-bit input image
		if (B != 16)
		{
			printf("Output format 'x15' requires 15-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_HSTX16:	// "x16": 16-bit HSTX compression, requires 24-bit input image
		if (B != 24)
		{
			printf("Output format 'x16' requires 24-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_ATTR1:	// "a1": attribute compression 1, 4-bit pixels, cell 8x8, 2 colors
		if (B != 4)
		{
			printf("Output format 'a1' requires 4-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_ATTR2:	// "a2": attribute compression 2, 4-bit pixels, cell 4x4, 2 colors
		if (B != 4)
		{
			printf("Output format 'a2' requires 4-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_ATTR3:	// "a3": attribute compression 3, 4-bit pixels, cell 8x8, 4 colors
		if (B != 4)
		{
			printf("Output format 'a3' requires 4-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_ATTR4:	// "a4": attribute compression 4, 4-bit pixels, cell 4x4, 4 colors
		if (B != 4)
		{
			printf("Output format 'a4' requires 4-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_ATTR5:	// "a5": attribute compression 5, 8-bit pixels, cell 8x8, 2 colors
		if (B != 8)
		{
			printf("Output format 'a5' requires 8-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_ATTR6:	// "a6": attribute compression 6, 8-bit pixels, cell 4x4, 2 colors
		if (B != 8)
		{
			printf("Output format 'a6' requires 8-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_ATTR7:	// "a7": attribute compression 7, 8-bit pixels, cell 8x8, 4 colors
		if (B != 8)
		{
			printf("Output format 'a7' requires 8-bit input image %s\n", argv[1]);
			return 1;
		}
		break;

	case FORMAT_ATTR8:	// "a8": attribute compression 8, 8-bit pixels, cell 4x4, 4 colors
		if (B != 8)
		{
			printf("Output format 'a8' requires 8-bit input image %s\n", argv[1]);
			return 1;
		}
		break;
	};

	if (AttrMode)
	{
		if (((W & (AttrCW-1)) != 0) || ((H & (AttrCH-1)) != 0))
		{
			printf("Selected attribute compression requires dimensions of multiple %dx%d: %s.\n", AttrCW, AttrCH, argv[1]);
			return 1;
		}
	}

	// open output file
	f = fopen(argv[2], "wb");
	if (f == NULL)
	{
		printf("Error creating %s\n", argv[2]);
		return 1;
	}

	// open test file
	FILE* f2 = NULL;
	if ((argc > 5) && AttrMode)
	{
		f2 = fopen(argv[5], "wb");
		if (f2 == NULL)
		{
			printf("Error creating %s\n", argv[5]);
			return 1;
		}
	}

	// save header
	WB = (((W*B+7)/8) + 3)& ~3;

	// 1-bit image
	if (B == 1)
	{
		Do1Bit(f, argv);
	}

	// paletted 4-bit image
	else if (B == 4)
	{
		Do4Bit(f, argv, f2, size, bmf->bfOffBits);
	}

	// paletted 8-bit image
	else if (B == 8)
	{
		Do8Bit(f, argv, f2, size, bmf->bfOffBits);
	}

	// 15-bit image
	else if (B == 16)
	{
		Do15Bit(f, argv);
	}

	// 24-bit image
	else
	{
		Do24Bit(f, argv);
	}

	fprintf(f, "\n};\n");

	fclose(f);
	free(Img);

	if (f2 != NULL) fclose(f2);

	return 0;
}

