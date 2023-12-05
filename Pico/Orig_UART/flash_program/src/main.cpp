
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// We're going to erase and reprogram a region 256k from the start of flash.
// Once done, we can access this at XIP_BASE + 256k.

/*
Debug output
------------
Generated random data:
51 b1 b5 03 70 fd 8b 29 89 03 cd 77 a3 e4 83 e1
2d 1f 03 f7 a7 a8 a0 4a 53 18 e3 ab 43 81 52 9a
c3 07 9b 1f 78 e5 7d 56 59 3b 44 12 99 82 b5 15
df cb a3 98 6d d5 31 c5 0a af ee 3a f3 48 eb 11
e5 41 70 b4 69 89 9a ba 6d e3 9e b9 27 83 1e 11
57 b1 36 4f 91 b5 20 86 7b 6e 47 25 e9 c9 06 1f
d3 d8 75 2f ae 57 bd 5e d2 c1 5f de 2d a3 7f 55
2b 5e be 84 57 f6 94 c5 44 4a b1 df 6d 50 e0 61
b7 fa fa e1 63 ae 90 5b c2 f7 5c af be 1b 0c 76
21 ec ad 7f 6a d3 c4 06 f0 74 25 5c 22 02 2f 59
ba 16 98 c9 a0 63 d4 7b 14 96 b1 20 0f 0d ac 2f
0a 49 d2 6b 0e 03 4e 46 36 a7 20 f8 04 75 fe d6
52 de ac 46 03 7b e2 7c 0e 52 3a 33 df 20 66 86
e2 dd d4 cc c3 1b 30 17 e9 ef be 56 f5 b1 b9 5e
6e 67 cb 2c 8f 81 7f 07 cb 01 63 72 30 ab 30 5a
20 03 35 d9 d8 9c d3 a5 12 70 e7 31 06 bf f7 ff

Erasing target region...
Done. Read back target region:
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff

Programming target region...
Done. Read back target region:
51 b1 b5 03 70 fd 8b 29 89 03 cd 77 a3 e4 83 e1
2d 1f 03 f7 a7 a8 a0 4a 53 18 e3 ab 43 81 52 9a
c3 07 9b 1f 78 e5 7d 56 59 3b 44 12 99 82 b5 15
df cb a3 98 6d d5 31 c5 0a af ee 3a f3 48 eb 11
e5 41 70 b4 69 89 9a ba 6d e3 9e b9 27 83 1e 11
57 b1 36 4f 91 b5 20 86 7b 6e 47 25 e9 c9 06 1f
d3 d8 75 2f ae 57 bd 5e d2 c1 5f de 2d a3 7f 55
2b 5e be 84 57 f6 94 c5 44 4a b1 df 6d 50 e0 61
b7 fa fa e1 63 ae 90 5b c2 f7 5c af be 1b 0c 76
21 ec ad 7f 6a d3 c4 06 f0 74 25 5c 22 02 2f 59
ba 16 98 c9 a0 63 d4 7b 14 96 b1 20 0f 0d ac 2f
0a 49 d2 6b 0e 03 4e 46 36 a7 20 f8 04 75 fe d6
52 de ac 46 03 7b e2 7c 0e 52 3a 33 df 20 66 86
e2 dd d4 cc c3 1b 30 17 e9 ef be 56 f5 b1 b9 5e
6e 67 cb 2c 8f 81 7f 07 cb 01 63 72 30 ab 30 5a
20 03 35 d9 d8 9c d3 a5 12 70 e7 31 06 bf f7 ff
Programming successful!
*/

#include "./include.h"

#define FLASH_TARGET_OFFSET (256 * 1024)

const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);

void print_buf(const uint8_t *buf, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	{
		printf("%02x", buf[i]);
		if (i % 16 == 15)
			printf("\n");
		else
			printf(" ");
	}
}

int main()
{
	stdio_init_all();

	// wait to connect terminal
	int ch;
	do {
		printf("Press spacebar to start...\n");
		ch = getchar();
	} while (ch != ' ');

	uint8_t random_data[FLASH_PAGE_SIZE];
	for (int i = 0; i < FLASH_PAGE_SIZE; ++i)
		random_data[i] = rand() >> 16;

	printf("Generated random data:\n");
	print_buf(random_data, FLASH_PAGE_SIZE);

	// Note that a whole number of sectors must be erased at a time.
	printf("\nErasing target region...\n");
	flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
	printf("Done. Read back target region:\n");
	print_buf(flash_target_contents, FLASH_PAGE_SIZE);

	printf("\nProgramming target region...\n");
	flash_range_program(FLASH_TARGET_OFFSET, random_data, FLASH_PAGE_SIZE);
	printf("Done. Read back target region:\n");
	print_buf(flash_target_contents, FLASH_PAGE_SIZE);

	bool mismatch = false;
	for (int i = 0; i < FLASH_PAGE_SIZE; ++i)
	{
		if (random_data[i] != flash_target_contents[i])
			mismatch = true;
	}

	if (mismatch)
		printf("Programming failed!\n");
	else
		printf("Programming successful!\n");

	// Wait for uart output to finish
	sleep_ms(100);

	return 0;
}
