
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Example of reading/writing an external serial flash using the PL022 SPI interface

#include "../include.h"

//#define FLASH_PAGE_SIZE        256
//#define FLASH_SECTOR_SIZE      4096

#define FLASH_CMD_PAGE_PROGRAM 0x02
#define FLASH_CMD_READ         0x03
#define FLASH_CMD_STATUS       0x05
#define FLASH_CMD_WRITE_EN     0x06
#define FLASH_CMD_SECTOR_ERASE 0x20

#define FLASH_STATUS_BUSY_MASK 0x01

#define PIN_MISO 4
#define PIN_CS   5
#define PIN_SCK  6
#define PIN_MOSI 7

static inline void cs_select(uint cs_pin)
{
	asm volatile("nop \n nop \n nop"); // FIXME
	gpio_put(cs_pin, 0);
	asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin)
{
	asm volatile("nop \n nop \n nop"); // FIXME
	gpio_put(cs_pin, 1);
	asm volatile("nop \n nop \n nop"); // FIXME
}

void __not_in_flash_func(flash_read)(spi_inst_t *spi, uint cs_pin, uint32_t addr, uint8_t *buf, size_t len)
{
	cs_select(cs_pin);
	uint8_t cmdbuf[4] =
	{
		FLASH_CMD_READ,
		(uint8_t)(addr >> 16),
		(uint8_t)(addr >> 8),
		(uint8_t)(addr)
	};
	spi_write_blocking(spi, cmdbuf, 4);
	spi_read_blocking(spi, 0, buf, len);
	cs_deselect(cs_pin);
}

void __not_in_flash_func(flash_write_enable)(spi_inst_t *spi, uint cs_pin)
{
	cs_select(cs_pin);
	uint8_t cmd = FLASH_CMD_WRITE_EN;
	spi_write_blocking(spi, &cmd, 1);
	cs_deselect(cs_pin);
}

void __not_in_flash_func(flash_wait_done)(spi_inst_t *spi, uint cs_pin)
{
	uint8_t status;
	do {
		cs_select(cs_pin);
		uint8_t buf[2] = {FLASH_CMD_STATUS, 0};
		spi_write_read_blocking(spi, buf, buf, 2);
		cs_deselect(cs_pin);
		status = buf[1];
	} while (status & FLASH_STATUS_BUSY_MASK);
}

void __not_in_flash_func(flash_sector_erase)(spi_inst_t *spi, uint cs_pin, uint32_t addr)
{
	uint8_t cmdbuf[4] =
	{
		FLASH_CMD_SECTOR_ERASE,
		(uint8_t)(addr >> 16),
		(uint8_t)(addr >> 8),
		(uint8_t)(addr)
	};
	flash_write_enable(spi, cs_pin);
	cs_select(cs_pin);
	spi_write_blocking(spi, cmdbuf, 4);
	cs_deselect(cs_pin);
	flash_wait_done(spi, cs_pin);
}

void __not_in_flash_func(flash_page_program)(spi_inst_t *spi, uint cs_pin, uint32_t addr, uint8_t data[])
{
	uint8_t cmdbuf[4] =
	{
		FLASH_CMD_PAGE_PROGRAM,
		(uint8_t)(addr >> 16),
		(uint8_t)(addr >> 8),
		(uint8_t)(addr)
	};
	flash_write_enable(spi, cs_pin);
	cs_select(cs_pin);
	spi_write_blocking(spi, cmdbuf, 4);
	spi_write_blocking(spi, data, FLASH_PAGE_SIZE);
	cs_deselect(cs_pin);
	flash_wait_done(spi, cs_pin);
}

void printbuf(uint8_t buf[FLASH_PAGE_SIZE])
{
	for (int i = 0; i < FLASH_PAGE_SIZE; ++i)
	{
		if (i % 16 == 15)
			printf("%02x\n", buf[i]);
		else
			printf("%02x ", buf[i]);
	}
}

int main()
{
	// Enable UART so we can print status output
	stdio_init_all();

	// wait to connect terminal
	int ch;
	do {
		printf("Press spacebar to start...\n");
		ch = getchar();
	} while (ch != ' ');

	printf("SPI flash example\n");

	// Enable SPI 0 at 1 MHz and connect to GPIOs
	spi_init(spi0, 1000 * 1000);
	gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
	gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
	gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

	// Chip select is active-low, so we'll initialise it to a driven-high state
	gpio_init(PIN_CS);
	gpio_put(PIN_CS, 1);
	gpio_set_dir(PIN_CS, GPIO_OUT);

	printf("SPI initialised, let's goooooo\n");

	uint8_t page_buf[FLASH_PAGE_SIZE];

	const uint32_t target_addr = 0;

	flash_sector_erase(spi0, PIN_CS, target_addr);
	flash_read(spi0, PIN_CS, target_addr, page_buf, FLASH_PAGE_SIZE);

	printf("After erase:\n");
	printbuf(page_buf);

	for (int i = 0; i < FLASH_PAGE_SIZE; ++i) page_buf[i] = i;
	flash_page_program(spi0, PIN_CS, target_addr, page_buf);
	flash_read(spi0, PIN_CS, target_addr, page_buf, FLASH_PAGE_SIZE);

	printf("After program:\n");
	printbuf(page_buf);

	flash_sector_erase(spi0, PIN_CS, target_addr);
	flash_read(spi0, PIN_CS, target_addr, page_buf, FLASH_PAGE_SIZE);

	printf("Erase again:\n");
	printbuf(page_buf);

	// Wait for uart output to finish
	sleep_ms(100);

	return 0;
}
