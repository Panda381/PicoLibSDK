
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// GPIO19, pin 0: LCD_MOSI 8-bit data, MSB first, LSB last
// GPIO20, pin 1: LCD_DC data/command, HIGH data, LOW command
// GPIO21, pin 2: LCD_CS chip select, HIGH inactive, LOW active
// GPIO22, pin 3: LCD_SCK clock, idle HIGH, rising (LOW->HIGH) sample data ... 1 pulse 20 ns up to 8 ns

#ifndef _MAIN_H
#define _MAIN_H

#define SPI_PIO		0	// SPI grabber PIO
#define SPI_SM		0	// SPI grabber state machine
#define SPI_PIO_OFF	0	// SPI grabber program offset
#define SPI_GPIO_FIRST	19	// first SPI GPIO
#define SPI_GPIO_NUM	4	// number of SPI GPIOs
#define SPI_GPIO_DC	20	// LCD_DC gpio pin
#define SPI_GPIO_CS	21	// LCD_CS gpio pin, to use in JMP PIN instruction
#define DISPSEL_GPIO	28	// display selection GPIO

// input grabber
extern "C" void NOFLASH(Grabber)();

// format: compressed 8-bit paletted pixel graphics
// image width: 320 pixels
// image height: 240 lines
// image pitch: 320 bytes
extern const u16 MonoscopeImg_Pal[137] __attribute__ ((aligned(4)));
extern const u8 MonoscopeImg[19266] __attribute__ ((aligned(4)));

#endif // _MAIN_H
