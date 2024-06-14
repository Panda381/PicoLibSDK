// Export BIN file to C++ source file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#pragma warning(disable : 4996)

int main(int argc, char* argv[])
{
	// check syntax
	if (argc != 6)
	{
		printf("Use syntax: GBprep infile.gbc outfile.cpp progname progext maxsize\n");
		printf("   infile.gbc .... input ROM file (GB or GBC)\n");
		printf("   outfile.cpp ... output source file (C or CPP)\n");
		printf("   progname ...... program name (without extension), max. 8 characters uppercase\n");
		printf("   progext ....... program extension, max. 3 characters uppercase\n");
		printf("   maxsize ...... limit max. size of ROM in Flash memory\n");
		return 1;
	}

	// get max. size of Flash
	int maxsize = atoi(argv[5]);

	// open input file
	FILE* f = fopen(argv[1], "rb");
	if (f == NULL)
	{
		printf("Cannot open input file %s\n", argv[1]);
		return 2;
	}

	// get size of input file
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	// allocate buffer
	unsigned char* buf = (unsigned char*)malloc(size);
	if ((buf == NULL) || (size < 0x150))
	{
		printf("Input file size or memory error\n");
		return 3;
	}
		 
	// read file
	if (fread(buf, 1, size, f) != size)
	{
		printf("Read error\n");
		return 4;
	}
	fclose(f);

	// prepare number of pages
#define PAGESIZE 0x4000
	int pagenum = ((int)size + PAGESIZE-1)/PAGESIZE;

	// allocate page lists
	int* pagesize = (int*)malloc(pagenum * sizeof(int));
	int* pageoff = (int*)malloc(pagenum * sizeof(int));
	int* pagestuff = (int*)malloc(pagenum * sizeof(int));
	int* pagelast = (int*)malloc(pagenum * sizeof(int));
	if ((pagesize == NULL) || (pageoff == NULL) || (pagestuff == NULL) || (pagelast == NULL))
	{
		printf("Memory error\n");
		return 3;
	}

	// detect pages
	int realsize = 0;
	int page, i;
	int pageflash = pagenum;
	unsigned char *b, stuff, last;
	for (page = 0; page < pagenum; page++)
	{
		b = &buf[page*PAGESIZE]; // current page
		i = PAGESIZE-1; // end of current page
		if (page == pagenum-1)
		{
			i = (int)(size & (PAGESIZE-1)) - 1; // size of last page
			if (i < 0) i = PAGESIZE-1;
		}
		last = b[i]; // get last byte
		if (i == PAGESIZE-1)
		{
			i--;		// index to pre-last
			stuff = b[i]; // get stuff from pre-last byte of the page
		}
		else
			stuff = last; // expand last byte to full last page (behind end of file)

		for (; i > 0; i--) // map end of page
		{
			if (b[i-1] != stuff) break; // not stuff
		}

		if (realsize + i > maxsize) // overload Flash size
		{
			pageflash = page;
			break;
		}

		pagesize[page] = i;
		pageoff[page] = realsize;
		pagestuff[page] = stuff;
		pagelast[page] = last;
		realsize += i;
	}

	// open output file
	f = fopen(argv[2], "wb");
	if (f == NULL)
	{
		printf("Cannot create output file %s\n", argv[2]);
		return 5;
	}

	// print include
	fprintf(f, "#include \"../include.h\"\n\n");

	// print total size and number of pages
	fprintf(f, "char ProgName[] = \"%s\";\n", argv[3]);
	fprintf(f, "int ProgNameLen = %d;\n", strlen(argv[3]));
	fprintf(f, "char ProgExt[] = \"%s\";\n", argv[4]);
	fprintf(f, "int ProgExtLen = %d;\n", strlen(argv[4]));
	fprintf(f, "int gameRomLen = %d;\n", realsize);
	fprintf(f, "int gameRomOrig = %d;\n", size);
	fprintf(f, "int gameRomPages = %d;\n", pagenum);
	fprintf(f, "int gameFlashPages = %d;\n\n", pageflash);

	if (pageflash < pagenum) printf("WARNING: Emulator requires ROM file!\n");

	// print size table
	fprintf(f, "u16 gameRomSizeList[%d] = {\n", pageflash);
	for (page = 0; page < pageflash; page++) fprintf(f, "\t%d,\t// page %d\n", pagesize[page], page);
	fprintf(f, "};\n\n");

	// print offsets table
	fprintf(f, "u32 gameRomOffList[%d] = {\n", pageflash);
	for (page = 0; page < pageflash; page++) fprintf(f, "\t%d,\t// page %d\n", pageoff[page], page);
	fprintf(f, "};\n\n");

	// print stuff table
	fprintf(f, "u8 gameRomStuffList[%d] = {\n", pageflash);
	for (page = 0; page < pageflash; page++) fprintf(f, "\t0x%02x,\t// page %d\n", pagestuff[page], page);
	fprintf(f, "};\n\n");

	// print last table
	fprintf(f, "u8 gameRomLastList[%d] = {\n", pageflash);
	for (page = 0; page < pageflash; page++) fprintf(f, "\t0x%02x,\t// page %d\n", pagelast[page], page);
	fprintf(f, "};\n\n");

	// print Rom head
	fprintf(f, "const u8 gameRom[%u] = {\n", realsize);

	// export file
	int pos = 0;
	for (page = 0; page < pageflash; page++)
	{
		b = &buf[page*PAGESIZE]; // current page

		for (i = 0; i < pagesize[page]; i++)
		{
			if (pos == 0) fprintf(f, "\t");

			pos++;
			if (pos == 16)
			{
				fprintf(f, "0x%02x,\n", b[i]);
				pos = 0;
			}
			else
				fprintf(f, "0x%02x, ", b[i]);
		}
	}

	// print end
	if (pos != 0) fprintf(f, "\n");
	fprintf(f, "};\n");

	// close output file
	fclose(f);

	return 0;
}

