// PicoLibSDK - Alternative SDK library for Raspberry Pico, RP2040 and RP2350
// Copyright (c) 2024 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#include <stdio.h>
#include <string.h>

#pragma warning(disable : 4996) // function not safe

typedef unsigned char       u8;
typedef unsigned int        u32;

#define LOADER_SIZE		0x8000
#define LOADER2_SIZE	0x10000
u8 Buf[LOADER2_SIZE+4];

int main(int argc, char* argv[])
{
	if ((argc != 4) || ((*argv[3] != '0') && (*argv[3] != '1') && (*argv[3] != '2')))
	{
		printf("Syntax: LoaderBin loader.bin loader_bin.S format\n");
		printf("    format ... 0=RP2040, 1=RP2350-ARM, 2=RP2350-RISCV\n");
		return 1;
	}

	FILE* f = fopen(argv[1], "rb");
	if (f == NULL)
	{
		printf("File %s not found\n", argv[1]);
		return 1;
	}

	char format = *argv[3];
	int size = (format == '0') ? LOADER_SIZE : LOADER2_SIZE;

	memset(Buf, 0xff, size);
	int n = (int)fread(Buf, 1, size+4, f);
	if ((n > size) || (n < 4))
	{
		printf("Incorrect %s file size\n", argv[1]);
		return 1;
	}
	fclose(f);

	f = fopen(argv[2], "wb");
	if (f == NULL)
	{
		printf("Cannot open %s for write\n", argv[2]);
		return 1;
	}

	fprintf(f, "// ****************************************************************************\n");
	fprintf(f, "//\n");
	fprintf(f, "//                                PicoPad Boot3 Loader\n");
	fprintf(f, "//\n");
	fprintf(f, "// ****************************************************************************\n");
	fprintf(f, "// Do not modify - auto generated\n");
	fprintf(f, "\n");

	if (format == '0') // RP2040
	{
		fprintf(f, "\t.syntax unified\n");
		fprintf(f, "\t.cpu cortex-m0plus\n");
		fprintf(f, "\t.thumb\n");
	}
	else if (format == '1') // RP2350-ARM
	{
		fprintf(f, "\t.syntax unified\n");
		fprintf(f, "\t.cpu cortex-m33\n");
		fprintf(f, "\t.thumb\n");
	}
//	else // RP2350-RISCV

	fprintf(f, "\t.section .boot2,\"ax\"\n");
//	fprintf(f, "\t.align 2\n");	// do not use .align, RISC-V compiler adds invalid bytes on end of the section
	fprintf(f, "\n");

	int i;
	u32* s = (u32*)Buf;
	for (i = size/4; i > 0; i--) fprintf(f, "\t.word\t0x%08x\n", *s++);

	fclose(f);

	return 0;
}
