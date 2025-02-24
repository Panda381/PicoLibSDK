// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

// Video for PicoPad

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define FPS 10	// video FPS
#define SOUNDRATE 22050	// sound rate
#define SOUNDSAMP (SOUNDRATE/FPS) // sound samples per frame (= 2205)
//#define SOUNDSAMP_DEL (FPS / (SOUNDRATE - SOUNDRATE/FPS*FPS)) // number of frames to delete 1 sound sample (= 2)
#define SOUNDFILE "SOUND.wav"
#define OUTFILE "VIDEO.VID"
#define BMPFILE "BMP/%06d.bmp"
#define WIDTH 160 // image width
#define HEIGHT 240 // image height
#define IMGSIZE (WIDTH*HEIGHT + 4*256 + 54 + 1000) // input image size + reserve (= 40474)
#define IMGMINSIZE (WIDTH*HEIGHT + 4*2 + 54) // minimal input image size

#pragma warning(disable : 4996) // unsafe fopen

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;

typedef unsigned int BOOL;
#define TRUE  1
#define FALSE 0


// WAV format
#pragma pack(push,1)

// WAV data descriptor (8 Bytes)
typedef struct WAVDATA_ {
	char			tDataIdent[4];		// (4) data identifier "data"
	u32				nDataSize;			// (4) size of following sound data
} WAVDATA;

// WAV format descriptor (28 Bytes)
typedef struct WAVFORMAT_ {
	char			tFormatIdent[8];	// (8) format identifier (= "WAVEfmt ")
	u32				nFormatSize;		// (4) size of following format data

	u16				wFormatTag;			// (2) data format (1 = PCM)
	u16				nChannels;			// (2) number of channels (1 = mono, 2 = stereo)
	u32				nSamplesPerSec;		// (4) sample frequency (number of samples per second)
	u32				nAvgBytesPerSec;	// (4) transfer rate (number of bytes per second)
	u16				nBlockAlign;		// (2) data align (bits*channels/8)

	u16				wBitsPerSample;		// (2) number of bits per one sample
} WAVFORMAT;

// WAV file header (44 B)
typedef struct WAVHEAD_ {
	char		tWavIdent[4];		// (4) file identifier (= "RIFF")
	u32			nFileSize;			// (4) size of following file data

	WAVFORMAT	WavFormat;			// format descriptor
	WAVDATA		WavData;			// data block
} WAVHEAD;
#pragma pack( pop )

// BMP file
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

char Filename[30];
u8 Img[IMGSIZE]; // input file buffer
u8* ImgData; // start of image line

u8* Snd = NULL; // buffer with sound
u8* SndData; // start of sound data
int SndNum; // number of sound samples
int SndOff; // sound sample offset

// main function
int main(int argc, char* argv[])
{
	int processed_frames = 0;
	int i, j;

	if ((sizeof(_bmpBITMAPFILEHEADER) != 14) ||
		(sizeof(_bmpBITMAPINFOHEADER) != 40))
	{
		printf("Incorrect size of data types. Check typedef of s32/u32 or do 32-bit compilation.\n");
		return 1;
	}

	// open and load sound
	FILE* f = fopen(SOUNDFILE, "rb");
	if (f == NULL)
	{
		printf("Error - cannot find sound file " SOUNDFILE "\n");
		return 1;
	}

	// size of sound file
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (size < 44)
	{
		printf("Incorrect size of " SOUNDFILE "\n");
		return 1;
	}

	// create buffer
	Snd = (u8*)malloc(size + SOUNDSAMP + 100);
	if (Snd == NULL)
	{
		printf("Memory error\n");
		return 1;
	}

	// read file
	int size2 = (int)fread(Snd, 1, size, f);
	fclose(f);
	if (size2 != size)
	{
		printf("Error reading " SOUNDFILE "\n");
		return 1;
	}

	// check WAV header
	WAVHEAD* wav = (WAVHEAD*)Snd;
	WAVFORMAT* fmt = &wav->WavFormat;
	WAVDATA* data = &wav->WavData;
	for (i = 10; i > 0; i--) // skip "JUNK"
	{
		if (memcmp(data->tDataIdent, "data", 4) == 0) break;
		data = (WAVDATA*)((u8*)data + 8 + data->nDataSize);
	}
	if ((memcmp(wav->tWavIdent, "RIFF", 4) != 0) || // check "RIFF" header
		(memcmp(fmt->tFormatIdent, "WAVEfmt ", 8) != 0) || // check "WAVEfmt " header
		(memcmp(data->tDataIdent, "data", 4) != 0) || // check "data" header
		(fmt->wFormatTag != 1) || // check PCM format
		(fmt->nChannels != 1) || // check mono
		(fmt->nSamplesPerSec != 22050) || // check rate
		(fmt->wBitsPerSample != 8)) // check bits per sample
	{
		printf("Incorrect format of input file %s,\n", SOUNDFILE);
		printf("  must be PCM mono 8-bit 22050Hz.\n");
		return 1;
	}
	SndData = (u8*)&data[1]; // start of sound data
	SndNum = data->nDataSize; // number of samples
	memset(&SndData[SndNum], 0, SOUNDSAMP);

	// create output file
	FILE* fout = fopen(OUTFILE, "wb");
	if (fout == NULL)
	{
		printf("Error - cannot create output file " OUTFILE "\n");
		return 1;
	}

	// loop all files
	printf("Processing");
	SndOff = 0;
	//int snddel = 0;
	for (i = 0; i < 1000000; i++)
	{
		// prepare file name
		sprintf(Filename, BMPFILE, i);

		// open image file
		f = fopen(Filename, "rb");
		if (f == NULL)
		{
			if (i == 0)
				continue;
			else
				break;
		}

		// read image file
		int size2 = (int)fread(Img, 1, IMGSIZE, f);
		fclose(f);
		if (size2 < IMGMINSIZE)
		{
			printf("\nError reading %s\n", Filename);
			return 1;
		}

		// check BMP header
		bmpBITMAPFILEHEADER* bmf = (bmpBITMAPFILEHEADER*)Img;
		bmpBITMAPINFOHEADER* bmi = (bmpBITMAPINFOHEADER*)&bmf[1];
		int W = bmi->biWidth;
		int H = bmi->biHeight;
		int B = bmi->biBitCount;
		if (H < 0) H = -H;
		if ((bmf->bfType != 0x4d42) ||
			(bmf->bfOffBits < 0x30) || (bmf->bfOffBits > 0x440) ||
			(bmi->biCompression != bmpBI_RGB) ||
			(W != WIDTH) || (H != HEIGHT) || (B != 8))
		{
			printf("\nIncorrect format of input file %s,\n", Filename);
			printf("  must be 8-bit paletted uncompressed %d x %d.\n", WIDTH, HEIGHT);
			return 1;
		}
		ImgData = &Img[bmf->bfOffBits];

		// export palettes
		u8* pal = (u8*)&bmi[1]; // start of palettes
		int palN = bmi->biClrUsed; // number of palettes
		if (palN == 0) palN = 256;
		for (j = 0; j < 256; j++)
		{
			u8 blue = 0;
			u8 green = 0;
			u8 red = 0;

			if (j < palN)
			{
				blue = pal[0];
				green = pal[1];
				red = pal[2];
				pal += 4;
			}
			u16 b = (blue >> 3) | ((green >> 2) << 5) | ((red >> 3) << 11);
			fwrite(&b, 1, 2, fout);
		}

		// write image data (even image: even pixels, odd image: odd pixels)
		if (bmi->biHeight < 0)
		{
			if (fwrite(ImgData, 1, WIDTH*HEIGHT, fout) != WIDTH*HEIGHT)
			{
				printf("\nError write to output file " OUTFILE "\n");
				return 1;
			}
		}
		else
		{
			for (j = WIDTH*HEIGHT-WIDTH; j >= 0; j -= WIDTH)
			{
				if (fwrite(&ImgData[j], 1, WIDTH, fout) != WIDTH)
				{
					printf("\nError write to output file " OUTFILE "\n");
					return 1;
				}
			}
		}

		// write sound sample
		fwrite(&SndData[SndOff], 1, SOUNDSAMP, fout);
		SndOff += SOUNDSAMP; // shift sound offset
		/*
		snddel++;
		if (snddel >= SOUNDSAMP_DEL)
		{
			snddel = 0;
			SndOff++; // skip 1 sound sample to synchronize with video
		}
		*/
		if (SndOff > SndNum) SndOff = SndNum;

		processed_frames++;
		printf(".");
	}

	// close output file
	fclose(fout);

	printf("\nProcessed %d frames\n", processed_frames);

	return 0;
}

