// Print mapper controller number of NES ROM file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#pragma warning(disable : 4996)

// base types
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;		// on 8-bit system use "signed int"
typedef unsigned short u16;		// on 8-bit system use "unsigned int"
typedef signed long int s32;		// on 64-bit system use "signed int"
typedef unsigned long int u32;		// on 64-bit system use "unsigned int"
typedef signed long long int s64;
typedef unsigned long long int u64;

typedef unsigned char BYTE;

// CRC-32 Reversed (CRC32B) polynomial
#define CRC32B_POLY 0xEDB88320

// CRC-32 Reversed (CRC32B) init word (reversed bits, not inverted)
// This initialization word is commonly given in its inverted form, as 0xFFFFFFFF.
#define CRC32B_INIT 0

// Calculate CRC-32 Reversed (CRC32B), buffer - slow version
u32 Crc32BBufSlow(u32 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	crc = ~crc;
	int i;

	for (; len > 0; len--)
	{
		crc ^= *s++;
		for (i = 8; i > 0; i--)
		{
			if ((crc & 1) == 0)
				crc >>= 1;
			else
				crc = (crc >> 1) ^ CRC32B_POLY;
		}
	}
	return ~crc;
}

// Calculate CRC-32 Reversed (CRC32B) - slow version
//   Calculation speed: 900 us per 1 KB
u32 Crc32BSlow(const void* buf, int len)
{
	return Crc32BBufSlow(CRC32B_INIT, buf, len);
}

/* .nes File Header */
struct
{
  BYTE byID[4];
  BYTE byRomSize;
  BYTE byVRomSize;
  BYTE byInfo1;
  BYTE byInfo2;
  BYTE byReserve[8];
} NesHeader;

// ROM (without Picopad loader and without config)
#define ROMBUFN (2*1024*1024 - 32*1024 - 4096)
u8 ROM[ROMBUFN];

int main(int argc, char* argv[])
{
	// check syntax
	if (argc != 2)
	{
		printf("Use syntax: MapperNo infile.nes\n");
		return 1;
	}

	// open input file
	FILE* f = fopen(argv[1], "rb");
	if (f == NULL)
	{
		printf("Cannot open input file %s\n", argv[1]);
		return 1;
	}

	// read file header
	if ((int)fread(&NesHeader, 1, sizeof(NesHeader), f) != sizeof(NesHeader))
	{
		printf("Read error\n");
		return 1;
	}

	// check NES header
	if ((NesHeader.byID[0] != 'N') ||
		(NesHeader.byID[1] != 'E') ||
		(NesHeader.byID[2] != 'S') ||
		(NesHeader.byID[3] != 26))
	{
		printf("Incorrect NES header!\n");
		return 1;
	}

	// skip trainer data
	if (NesHeader.byInfo1 & 4) fread(ROM, 1, 512, f);

	// prepare ROM size
	int n = NesHeader.byRomSize * 0x4000;
	n += NesHeader.byVRomSize * 0x2000;
	if (n > ROMBUFN)
	{
		printf("Incorrect ROM size!\n");
		return 1;
	}

	// read ROM
	if ((int)fread(ROM, 1, n, f) != n)
	{
		printf("Read error\n");
		return 1;
	}
	fclose(f);

	// get mapper number NES0
	int MapperNo = NesHeader.byInfo1 >> 4;

	// get mapper number NES2
	if ((NesHeader.byInfo2 & 0x0c) == 0x08)
	{
		MapperNo |= (NesHeader.byInfo2 & 0xf0) | ((NesHeader.byReserve[0] & 0x0f) << 8);
	}

	// get mapper number NES1
	else if ((NesHeader.byReserve[4] == 0) && (NesHeader.byReserve[5] == 0) &&
			(NesHeader.byReserve[6] == 0) && (NesHeader.byReserve[7] == 0))
	{
		MapperNo |= (NesHeader.byInfo2 & 0xf0);
	}

	// calculate CRC32
	u32 crc = Crc32BSlow(ROM, n);

	// print mapper number and CRC
	printf("^ZMBC=%03d CRC=%08X^0\n", MapperNo, crc);

	return 0;
}
