// Game Boy ROM setup checksum.

#include <stdio.h>
#include <string.h>

#pragma warning(disable : 4996) // function not safe

typedef unsigned char       u8;

#define BUFN (8*1024*1024)
u8 buf[BUFN];

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Syntax: GBcrc filename.gbc\n");
		return 1;
	}

	FILE* f = fopen(argv[1], "rb");
	if (f == NULL)
	{
		printf("File %s not found\n", argv[1]);
		return 1;
	}

	size_t n = fread(buf, 1, BUFN, f);
	if (n < 0x150)
	{
		printf("Incorrect %s file size\n", argv[1]);
		return 1;
	}
	fclose(f);

	int i;
	u8 sum = 0;
	for (i = 0x134; i <= 0x14c; i++) sum = sum - buf[i] - 1;
	if (sum == buf[0x14d])
	{
		printf("Check sum of %s is already set\n", argv[1]);
		return 0;
	}
	buf[0x14d] = sum;

	f = fopen(argv[1], "wb");
	if (f == NULL)
	{
		printf("Cannot open %s for write\n", argv[1]);
		return 1;
	}

	size_t n2 = fwrite(buf, 1, n, f);
	if (n2 != n)
	{
		printf("Cannot write to %s\n", argv[1]);
		return 1;
	}
	fclose(f);

	printf("Checksum of %s set OK\n", argv[1]);

	return 0;
}
