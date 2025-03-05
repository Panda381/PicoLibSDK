// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023-2025 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#pragma warning(disable : 4996) // unsafe fopen

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef unsigned short WORD;
//typedef signed long int s32;		// on 64-bit system use "signed int"
//typedef unsigned long int u32;		// on 64-bit system use "unsigned int"
//typedef unsigned long int DWORD;	// on 64-bit system use "unsigned int"
typedef signed int s32;
typedef unsigned int u32;
typedef unsigned int DWORD;

typedef unsigned int BOOL;
#define TRUE  1
#define FALSE 0

// WAV format
#pragma pack(push,1)

// WAV data descriptor (8 Bytes)
typedef struct WAVDATA_ {
	char			tDataIdent[4];		// (4) data identifier "data"
	DWORD			nDataSize;			// (4) size of following sound data
} WAVDATA;

// WAV format descriptor (28 Bytes, or 30 bytes for ADPCM)
typedef struct WAVFORMAT_ {
	char			tFormatIdent[8];	// (8) format identifier (= "WAVEfmt ")
	DWORD			nFormatSize;		// (4) size of following format data

	WORD			wFormatTag;			// (2) data format (1 = PCM, 17 = Intel DVI ADPCM)
	WORD			nChannels;			// (2) number of channels (1 = mono, 2 = stereo)
	DWORD			nSamplesPerSec;		// (4) sample frequency (number of samples per second)
	DWORD			nAvgBytesPerSec;	// (4) transfer rate (number of bytes per second)
	WORD			nBlockAlign;		// (2) data align (bits*channels/8)

	WORD			wBitsPerSample;		// (2) number of bits per one sample

//	WORD			wExtraByte;			// (2) ADPCM: number of following extra bytes (= 2)
//	WORD			wSampBlock;			// (2) ADPCM: number of samples per block including 1st sample in header

// ADPCM: Each block starts with 4 bytes of preamble (stereo starts with 2 preambles):
//  s16	... audio sample (= 1st sample, initial predictor)
//  u8 ... table index
//  u8 ... dummy byte (= 0)
// 1st data sample is in LOW 4 bits, nest sample is in HIGH 4 bits

} WAVFORMAT;

// WAV file header (44 B)
typedef struct WAVHEAD_ {
	char		tWavIdent[4];		// (4) file identifier (= "RIFF")
	DWORD		nFileSize;			// (4) size of following file data

	WAVFORMAT	WavFormat;			// format descriptor
	WAVDATA		WavData;			// data block
} WAVHEAD;
#pragma pack( pop )

u8* Snd = NULL; // input file buffer

int main(int argc, char* argv[])
{
	if (sizeof(DWORD) != 4)
	{
		printf("Incorrect size of data types. Check typedef of s32/u32 or do 32-bit compilation.\n");
		return 1;
	}

	// check syntax
	if (argc != 4)
	{
		printf("Syntax: input.wav output.cpp name\n");
		return 1;
	}

// === read input file

	// open main input file
	FILE* f = fopen(argv[1], "rb");
	if (f == NULL)
	{
		printf("Error opening %s\n", argv[1]);
		return 1;
	}

	// size of input file
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (size < 44)
	{
		printf("Incorrect size of %s\n", argv[1]);
		return 1;
	}

	// create buffer
	Snd = (unsigned char*)malloc(size);
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
		printf("Error reading %s\n", argv[1]);
		return 1;
	}

	// check WAV header
	WAVHEAD* wav = (WAVHEAD*)Snd;
	WAVFORMAT* fmt = &wav->WavFormat;
	WAVDATA* data = (WAVDATA*)((u8*)fmt + 12 + fmt->nFormatSize);
	int i;
	for (i = 10; i > 0; i--) // skip "JUNK"
	{
		if (memcmp(data->tDataIdent, "data", 4) == 0) break;
		data = (WAVDATA*)((u8*)data + 8 + data->nDataSize);
	}
	if ((memcmp(wav->tWavIdent, "RIFF", 4) != 0) || // check "RIFF" header
		(memcmp(fmt->tFormatIdent, "WAVEfmt ", 8) != 0) || // check "WAVEfmt " header
		(memcmp(data->tDataIdent, "data", 4) != 0) || // check "data" header
		((fmt->wFormatTag != 1) && (fmt->wFormatTag != 17)) || // check PCM of ADPCM format
//		(fmt->nChannels != 1) || // check mono
//		(fmt->nSamplesPerSec != 22050) || // check rate
		((fmt->wBitsPerSample != 16) && (fmt->wBitsPerSample != 8) && (fmt->wBitsPerSample != 4))) // check bits per sample
	{
		printf("Incorrect format of input file %s,\n", argv[1]);
		printf("  must be PCM 8-bit or 16-bit\n");
		printf("  or IMA ADPCM 4-bit.\n");
		return 1;
	}
	u8* s = (u8*)&data[1]; // start of sound data
	int n = data->nDataSize; // data size

// === save output file

	// open output file
	f = fopen(argv[2], "wb");
	if (f == NULL)
	{
		printf("Error creating %s\n", argv[2]);
		return 1;
	}

	// header
	fprintf(f, "#include \"../include.h\"\n\n");
	if (fmt->wFormatTag == 17)
	{
		fprintf(f, "// sound format: Intel IMA ADPCM %s 4-bit %dHz\n",
			(fmt->nChannels == 1) ? "mono" : "stereo", fmt->nSamplesPerSec);
	}
	else
	{
		fprintf(f, "// sound format: PCM %s %d-bit %dHz\n",
			(fmt->nChannels == 1) ? "mono" : "stereo", fmt->wBitsPerSample, fmt->nSamplesPerSec);
	}

	// sound format
	fprintf(f, "// sound format = %s\n", 
		(fmt->wBitsPerSample == 4) ?
			((fmt->nChannels == 1) ? "SNDFORM_ADPCM" : "SNDFORM_ADPCM_S") :
			((fmt->wBitsPerSample == 8) ?
				((fmt->nChannels == 1) ? "SNDFORM_PCM" : "SNDFORM_PCM_S") :
				((fmt->nChannels == 1) ? "SNDFORM_PCM16" : "SNDFORM_PCM16_S")));

	// relative sound speed
	fprintf(f, "// sound speed relative to 22050Hz = %.5ff\n", (double)fmt->nSamplesPerSec/22050);

	// ADPCM samples per block
	if (fmt->wFormatTag == 17)
	{
		if (fmt->nFormatSize >= 20)
			fprintf(f, "const u16 %s_SampBlock = %d; // number of samples per block\n",
				argv[3], (&fmt->wBitsPerSample)[2]);
	}

	// data header
	fprintf(f, "const u8 %s[%d] __attribute__ ((aligned(4))) = {", argv[3], n);

	// load sound
	for (i = 0; i < n; i++)
	{
		if ((i & 0x0f) == 0) fprintf(f, "\n\t");
		fprintf(f, "0x%02X, ", s[i]);
	}
	fprintf(f, "\n};\n");

	fclose(f);
	free(Snd);

	return 0;
}
