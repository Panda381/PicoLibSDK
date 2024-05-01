
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// (GPIO19) GPIO26, pin 0: LCD_MOSI 8-bit data, MSB first, LSB last
// (GPIO20) GPIO27, pin 1: LCD_DC data/command, HIGH data, LOW command
// (GPIO21) GPIO28, pin 2: LCD_CS chip select, HIGH inactive, LOW active
// (GPIO22) GPIO29, pin 3: LCD_SCK clock, idle HIGH, rising (LOW->HIGH) sample data ... 1 pulse 20 ns up to 8 ns

#ifndef _MAIN_H
#define _MAIN_H

//#define SPI_GPIO_FIRST	(19) 26		// first SPI GPIO

#define SPI_PIO		0	// SPI grabber PIO
#define SPI_SM		0	// SPI grabber state machine
#define SPI_PIO_OFF	0	// SPI grabber program offset

#define SPI_GPIO_NUM	4	// number of SPI GPIOs
#define SPI_GPIO_DC	(SPI_GPIO_FIRST+1)	// LCD_DC gpio pin
#define SPI_GPIO_CS	(SPI_GPIO_FIRST+2)	// LCD_CS gpio pin, to use in JMP PIN instruction

#if !USE_EXTDISP		// use ExtDisp module: 0=use version 0 (with Pico), 1=use version 1 (with RP2040)
#define DISPSEL_GPIO	28	// display selection GPIO (only ExtDisp version 0)
#endif

// input grabber
extern "C" void NOFLASH(Grabber)();

#endif // _MAIN_H
