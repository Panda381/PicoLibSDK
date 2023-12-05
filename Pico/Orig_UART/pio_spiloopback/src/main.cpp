
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// This program instantiates a PIO SPI with each of the four possible
// CPOL/CPHA combinations, with the serial input and output pin mapped to the
// same GPIO. Any data written into the state machine's TX FIFO should then be
// serialised, deserialised, and reappear in the state machine's RX FIFO.

/*
Debug output
------------
CPHA = 0, CPOL = 0
TX: 51 b1 b5 03 70 fd 8b 29 89 03 cd 77 a3 e4 83 e1 2d 1f 03 f7
RX: 51 b1 b5 03 70 fd 8b 29 89 03 cd 77 a3 e4 83 e1 2d 1f 03 f7
OK
CPHA = 0, CPOL = 1
TX: a7 a8 a0 4a 53 18 e3 ab 43 81 52 9a c3 07 9b 1f 78 e5 7d 56
RX: a7 a8 a0 4a 53 18 e3 ab 43 81 52 9a c3 07 9b 1f 78 e5 7d 56
OK
CPHA = 1, CPOL = 0
TX: 59 3b 44 12 99 82 b5 15 df cb a3 98 6d d5 31 c5 0a af ee 3a
RX: 59 3b 44 12 99 82 b5 15 df cb a3 98 6d d5 31 c5 0a af ee 3a
OK
CPHA = 1, CPOL = 1
TX: f3 48 eb 11 e5 41 70 b4 69 89 9a ba 6d e3 9e b9 27 83 1e 11
RX: f3 48 eb 11 e5 41 70 b4 69 89 9a ba 6d e3 9e b9 27 83 1e 11
OK
*/

#include "../include.h"

#define PIN_SCK 18
#define PIN_MOSI 16
#define PIN_MISO 16 // same as MOSI, so we get loopback

#define BUF_SIZE 20

void test(const pio_spi_inst_t *spi)
{
	static uint8_t txbuf[BUF_SIZE];
	static uint8_t rxbuf[BUF_SIZE];
	printf("TX:");
	for (int i = 0; i < BUF_SIZE; ++i)
	{
		txbuf[i] = rand() >> 16;
		rxbuf[i] = 0;
		printf(" %02x", (int) txbuf[i]);
	}
	printf("\n");

	pio_spi_write8_read8_blocking(spi, txbuf, rxbuf, BUF_SIZE);

	printf("RX:");
	bool mismatch = false;
	for (int i = 0; i < BUF_SIZE; ++i)
	{
		printf(" %02x", (int) rxbuf[i]);
		mismatch = mismatch || rxbuf[i] != txbuf[i];
	}
	if (mismatch)
		printf("\nNope\n");
	else
		printf("\nOK\n");
}

int main()
{
	stdio_init_all();

	// wait to connect terminal
	int c;
	do {
		printf("Press spacebar to start...\n");
		c = getchar();
	} while (c != ' ');

	pio_spi_inst_t spi =
	{
		.pio = pio0,
		.sm = 0
	};
	float clkdiv = 31.25f;  // 1 MHz @ 125 clk_sys
	uint cpha0_prog_offs = pio_add_program(spi.pio, &spi_cpha0_program);
	uint cpha1_prog_offs = pio_add_program(spi.pio, &spi_cpha1_program);

	for (int cpha = 0; cpha <= 1; ++cpha)
	{
		for (int cpol = 0; cpol <= 1; ++cpol)
		{
			printf("CPHA = %d, CPOL = %d\n", cpha, cpol);
			pio_spi_init(spi.pio, spi.sm,
				cpha ? cpha1_prog_offs : cpha0_prog_offs,
				8,       // 8 bits per SPI frame
				clkdiv,
				cpha,
				cpol,
				PIN_SCK,
				PIN_MOSI,
				PIN_MISO
			);
			test(&spi);
			sleep_ms(10);
		}
	}

	// Wait for uart output to finish
	sleep_ms(100);

	return 0;
}
