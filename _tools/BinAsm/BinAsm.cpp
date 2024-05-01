// Export BIN file to S source file

#include <stdio.h>
#include <malloc.h>

#pragma warning(disable : 4996)

int main(int argc, char* argv[])
{
	// check syntax
	if (argc != 3)
	{
		printf("Use syntax: BinAsm infile.bin outfile.asm\n");
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
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	// allocate buffer
	unsigned char* buf = (unsigned char*)malloc(size);
	if (buf == NULL)
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

	// open output file
	f = fopen(argv[2], "wb");
	if (f == NULL)
	{
		printf("Cannot create output file %s\n", argv[2]);
		return 5;
	}

	// export file
	int pos = 0;
	int i;
	for (i = 0; i < (int)size; i++)
	{
		if (pos == 0) fprintf(f, "\n\tdb\t");
		pos++;
		if ((pos == 16) || (i == (int)size-1))
		{
			fprintf(f, "0%02xh", buf[i]);
			pos = 0;
		}
		else
			fprintf(f, "0%02xh, ", buf[i]);
	}

	// print end
	fprintf(f, "\n");

	// close output file
	fclose(f);

	return 0;
}

