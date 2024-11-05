// Export BIN file to C++ source file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#pragma warning(disable : 4996)

int main(int argc, char* argv[])
{
	// check syntax
	if (argc != 4)
	{
		printf("Use syntax: NESprep infile.nes outfile.cpp progname\n");
		printf("   infile.nes .... input ROM file (NES)\n");
		printf("   outfile.cpp ... output source file (C or CPP)\n");
		printf("   progname ...... program name (without extension), max. 8 characters uppercase\n");
		return 1;
	}

	// open input file
	FILE* f = fopen(argv[1], "rb");
	if (f == NULL)
	{
		printf("Cannot open input file %s\n", argv[1]);
		return 2;
	}

	// get size of input file
	fseek(f, 0, SEEK_END);
	int size = (int)ftell(f);
	fseek(f, 0, SEEK_SET);

	// allocate buffer
	unsigned char* buf = (unsigned char*)malloc(size);
	if ((buf == NULL) || (size < 0x150))
	{
		printf("Input file size or memory error\n");
		return 3;
	}
		 
	// read file
	if ((int)fread(buf, 1, size, f) != size)
	{
		printf("Read error\n");
		return 4;
	}
	fclose(f);

	// open output file
	f = fopen(argv[2], "wb");
	if (f == NULL)
	{
		printf("Cannot create output file %s\n", argv[2]);
		return 5;
	}

	// print include
	fprintf(f, "#include \"../include.h\"\n\n");

	// print program name
	fprintf(f, "char ProgName[] = \"%s\";\n", argv[3]);
	fprintf(f, "int ProgNameLen = %d;\n\n", strlen(argv[3]));

	// print ROM size
	fprintf(f, "int builtinrom_len = %d;\n", size);

	// print Rom head
	fprintf(f, "const u8 builtinrom[%u] = {\n", size);

	// export file
	int i;
	int pos = 0;
	for (i = 0; i < size; i++)
	{
		if (pos == 0) fprintf(f, "\t");

		pos++;
		if (pos == 16)
		{
			fprintf(f, "0x%02x,\n", buf[i]);
			pos = 0;
		}
		else
			fprintf(f, "0x%02x, ", buf[i]);
	}

	// print end
	if (pos != 0) fprintf(f, "\n");
	fprintf(f, "};\n");

	// close output file
	fclose(f);

	return 0;
}

