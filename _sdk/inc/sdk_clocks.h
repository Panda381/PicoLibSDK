
// ****************************************************************************
//
//                               Clocks
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#ifndef _SDK_CLOCKS_H
#define _SDK_CLOCKS_H

#include "../sdk_addressmap.h"		// Register address offsets

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_clocks.h"		// constants of original SDK
#else
#include "orig_rp2350/orig_clocks.h"		// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// clock lines
#define CLK_GPOUT0	0	// GPIO muxing output GPOUT0 (GPIO21, max. 50 MHz)
#define CLK_GPOUT1	1	// GPIO muxing output GPOUT1 (GPIO23, max. 50 MHz ... internal signal on Pico board)
#define CLK_GPOUT2	2	// GPIO muxing output GPOUT2 (GPIO24, max. 50 MHz ... internal signal on Pico board)
#define CLK_GPOUT3	3	// GPIO muxing output GPOUT3 (CPIO25, max. 50 MHz ... internal signal on Pico board)
#define CLK_REF		4	// watchdog and timers reference clock (6..12 Mhz) ... has glitchless mux
#define CLK_SYS		5	// system clock to processors, bus fabric, memory,
				//      memory mapped registers (125 MHz, up to 133 MHz) ... has glitchless mux
#define CLK_PERI	6	// peripheral clock for UART and SPI (12..125 MHz)
#if RP2040
#define CLK_USB		7	// USB clock (must be 48 MHz)
#define CLK_ADC		8	// ADC clock (must be 48 MHz)
#define CLK_RTC		9	// RTC real-time counter clock (46875 Hz)
#else
#define CLK_HSTX	7	// HSTX clock
#define CLK_USB		8	// USB clock (must be 48 MHz)
#define CLK_ADC		9	// ADC clock (must be 48 MHz)
#endif
#define CLK_NUM		10	// number of clock lines

// clock generators
#define CLK_ROSC	10	// ring oscillator ROSC (6 MHz)
#define CLK_XOSC	11	// crystal oscillator XOSC (12 MHz)
#define CLK_PLL_SYS	12	// system PLL (125 MHz)
#define CLK_PLL_USB	13	// USB PLL (48 MHz)
#define CLK_GPIN0	14	// GPIO muxing input GPIN0 (GPIO20)
#define CLK_GPIN1	15	// GPIO muxing input GPIN1 (GPIO22)

#if RP2040
#define CLK_SRC_NUM	16	// number of clock sources
#else
#define CLK_LPOSC	16	// low power oscillator LPOSC (32768 Hz)
#define CLK_OTP_2FC	17	// OTP CLK2FC
#define CLK_PLL_OPCG	18	// USB PLL primary ref OPCG

#define CLK_SRC_NUM	19	// number of clock sources
#endif

// clock index in original-SDK style
enum clock_index {
	// clock lines
	clk_gpout0 = 0,		// 0: GPIO muxing output GPOUT0 (max. 50 MHz)
	clk_gpout1,		// 1: GPIO muxing output GPOUT1 (max. 50 MHz)
	clk_gpout2,		// 2: GPIO muxing output GPOUT2 (max. 50 MHz)
	clk_gpout3,		// 3: GPIO muxing output GPOUT3 (max. 50 MHz)
	clk_ref,		// 4: watchdog and timers reference clock (6..12 Mhz) ... has glitchless mux
	clk_sys,		// 5: system clock to processors, bus fabric, memory, memory mapped registers
				//	(RP2040: 125 MHz, RP2350: 150 Mhz) ... has glitchless mux
	clk_peri,		// 6: peripheral clock for UART and SPI (RP2040: 12..125 MHz, RP2350: 12..150 MHz)
#if RP2040
	clk_usb,		// 7: USB clock (must be 48 MHz)
	clk_adc,		// 8: ADC clock (must be 48 MHz)
	clk_rtc,		// 9: RTC real-time counter clock (46875 Hz)
#else
	clk_hstx,		// 7: HSTX clock (150 MHz)
	clk_usb,		// 8: USB clock (must be 48 MHz)
	clk_adc,		// 9: ADC clock (must be 48 MHz)
#endif
	CLK_COUNT,		// 10: number of clock lines

	// clock generators
	clk_rosc = CLK_COUNT,	// 10: ring oscillator ROSC (RP2040: 6 MHz, RP2350: 11 MHz)
	clk_xosc,		// 11: crystal oscillator XOSC (12 MHz)
	clk_pll_sys,		// 12: system PLL (125 MHz)
	clk_pll_usb,		// 13: USB PLL (48 MHz)
	clk_gpin0,		// 14: GPIO muxing input GPIN0
	clk_gpin1,		// 15: GPIO muxing input GPIN1

#if !RP2040
	clk_lposc,		// 16: low power oscillator LPOSC (32 kHz)
	clk_otp_2fc,		// 17: OTP CLK2FC
	clk_pll_opcg,		// 18: USB PLL primary ref OPCG
#endif

	CLK_SRC_COUNT,		// 16 (19): number of clock sources
};

/* supported clock source of clock lines (x=auxiliary mux, o=glitchless mux, !=default mux)

RP2040:
              GPOUT  REF   SYS  PERI   USB   ADC   RTC
CLK_GPOUT0      .     .     .     .     .     .     .
CLK_GPOUT1      .     .     .     .     .     .     .
CLK_GPOUT2      .     .     .     .     .     .     .
CLK_GPOUT3      .     .     .     .     .     .     .
CLK_REF	        x     .     o!    .     .     .     .
CLK_SYS         x     .     .     x!    .     .     .
CLK_PERI        .     .     .     .     .     .     .
CLK_USB	        x     .     .     .     .     .     .
CLK_ADC	        x     .     .     .     .     .     .
CLK_RTC	        x     .     .     .     .     .     .
CLK_ROSC        x     o!    x     x     x     x     x
CLK_XOSC        x     o     x     x     x     x     x
CLK_PLL_SYS     x!    .     x     x     x     x     x
CLK_PLL_USB     x     x     x     x     x!    x!    x!
CLK_GPIN0       x     x     x     x     x     x     x
CLK_GPIN1       x     x     x     x     x     x     x

RP2350:
              GPOUT  REF   SYS  PERI  HSTX   USB   ADC 
CLK_GPOUT0      .     .     .     .     .     .     .  
CLK_GPOUT1      .     .     .     .     .     .     .  
CLK_GPOUT2      .     .     .     .     .     .     .  
CLK_GPOUT3      .     .     .     .     .     .     .  
CLK_REF	        x     .     o!    .     .     .     . 
CLK_SYS         x     .     .     x!    x!    .     . 
CLK_PERI        x     .     .     .     .     .     . 
CLK_HSTX        x     .     .     .     .     .     .
CLK_USB	        x     .     .     .     .     .     . 
CLK_ADC	        x     .     .     .     .     .     . 
CLK_ROSC        x     o!    x     x     .     x     x 
CLK_XOSC        x     o     x     x     .     x     x 
CLK_PLL_SYS     x!    .     x     x     x     x     x 
CLK_PLL_USB     x     x     x     x     x     x!    x!
CLK_GPIN0       x     x     x     x     x     x     x 
CLK_GPIN1       x     x     x     x     x     x     x 
CLK_LPOSC       x     o     .     .     .     .     .
CLK_OTP_2FC     x     .     .     .     .     .     .
CLK_PLL_OPCG    x     x     .     .     .     .     .
*/

// clock line hardware registers
//#define CLOCKS_BASE		0x40008000	// clocks
#define CLK(clk)		(CLOCKS_BASE + (clk)*12)	// clock base (RP2350: bit 28 = R/O clock generator is enabled)
#define CLK_CTRL(clk)		((volatile u32*)(CLK(clk)+0))	// control
#define CLK_DIV(clk)		((volatile u32*)(CLK(clk)+4))	// divisor
#define CLK_SEL(clk)		((volatile u32*)(CLK(clk)+8))	// bits of selected glitchless source src

// number of bits of fraction part of divider
#if RP2040
#define CLK_DIV_FRAC_BITS	8
#else
#define CLK_DIV_FRAC_BITS	16
#endif

// clock line hardware interface
typedef struct {
	io32	ctrl;		// 0x00: control
	io32	div;		// 0x04: divisor
	io32	selected;	// 0x08: bits of selected glitchless source src
} clock_hw_t;

STATIC_ASSERT(sizeof(clock_hw_t) == 0x0C, "Incorrect clock_hw_t!");

// resus hardware interface
typedef struct {
	io32	ctrl;		// 0x00: resus control
	io32	status;		// 0x04: resus status
} clock_resus_hw_t;

STATIC_ASSERT(sizeof(clock_resus_hw_t) == 0x08, "Incorrect clock_resus_hw_t!");

// resus hardware registers
#if RP2040
#define CLK_RES_CTRL		((volatile u32*)(CLOCKS_BASE+0x78)) // resus CTRL
#define CLK_RES_STATUS		((volatile u32*)(CLOCKS_BASE+0x7C)) // resus status
#define clocks_resus_hw ((clocks_resus_hw_t*)(CLOCKS_BASE + 0x78))
#else
#define CLK_RES_CTRL		((volatile u32*)(CLOCKS_BASE+0x84)) // resus CTRL
#define CLK_RES_STATUS		((volatile u32*)(CLOCKS_BASE+0x88)) // resus status
#define clocks_resus_hw ((clocks_resus_hw_t*)(CLOCKS_BASE + 0x84))
#endif

// frequency counter hardware registers
#if RP2040
#define CLK_FC0_REF		((volatile u32*)(CLOCKS_BASE+0x80)) // reference clock frequency in kHz
#define CLK_FC0_MIN		((volatile u32*)(CLOCKS_BASE+0x84)) // minimum pass frequency in kHz
#define CLK_FC0_MAX		((volatile u32*)(CLOCKS_BASE+0x88)) // maximum pass frequency in kHz
#define CLK_FC0_DELAY		((volatile u32*)(CLOCKS_BASE+0x8C)) // delay start of frequency counting
#define CLK_FC0_INTERVAL	((volatile u32*)(CLOCKS_BASE+0x90)) // test interval
#define CLK_FC0_SRC		((volatile u32*)(CLOCKS_BASE+0x94)) // clock sent to frequency counter
#define CLK_FC0_STATUS		((volatile u32*)(CLOCKS_BASE+0x98)) // status of frequency counter
#define CLK_FC0_RESULT		((volatile u32*)(CLOCKS_BASE+0x9C)) // result of frequency measurement
#else
#define CLK_FC0_REF		((volatile u32*)(CLOCKS_BASE+0x8C)) // reference clock frequency in kHz
#define CLK_FC0_MIN		((volatile u32*)(CLOCKS_BASE+0x90)) // minimum pass frequency in kHz
#define CLK_FC0_MAX		((volatile u32*)(CLOCKS_BASE+0x94)) // maximum pass frequency in kHz
#define CLK_FC0_DELAY		((volatile u32*)(CLOCKS_BASE+0x98)) // delay start of frequency counting
#define CLK_FC0_INTERVAL	((volatile u32*)(CLOCKS_BASE+0x9C)) // test interval
#define CLK_FC0_SRC		((volatile u32*)(CLOCKS_BASE+0xA0)) // clock sent to frequency counter
#define CLK_FC0_STATUS		((volatile u32*)(CLOCKS_BASE+0xA4)) // status of frequency counter
#define CLK_FC0_RESULT		((volatile u32*)(CLOCKS_BASE+0xA8)) // result of frequency measurement
#endif

// frequency counter hardware interface
typedef struct {
	io32	ref_khz;	// 0x00: reference clock frequency in kHz
	io32	min_khz;	// 0x04: minimum pass frequency in kHz
	io32	max_khz;	// 0x08: maximum pass frequency in kHz
	io32	delay;		// 0x0C: delay start of frequency counting
	io32	interval;	// 0x10: test interval
	io32	src;		// 0x14: clock sent to frequency counter
	io32	status;		// 0x18: status of frequency counter
	io32	result;		// 0x1C: result of frequency measurement
} fc_hw_t;

#if RP2040
#define fc_hw ((fc_hw_t*)(CLOCKS_BASE + 0x80))
#else
#define fc_hw ((fc_hw_t*)(CLOCKS_BASE + 0x8C))
#endif

STATIC_ASSERT(sizeof(fc_hw_t) == 0x20, "Incorrect fc_hw_t!");

// Clock hardware registers
// sleep state
#define CLK_WAKE_EN0		((volatile u32*)(CLOCKS_BASE+0xA0)) // enable clock in wake mode
#define CLK_WAKE_EN1		((volatile u32*)(CLOCKS_BASE+0xA4)) // enable clock in wake mode
#define CLK_SLEEP_EN0		((volatile u32*)(CLOCKS_BASE+0xA8)) // enable clock in sleep mode
#define CLK_SLEEP_EN1		((volatile u32*)(CLOCKS_BASE+0xAC)) // enable clock in sleep mode
#define CLK_ENABLED0		((volatile u32*)(CLOCKS_BASE+0xB0)) // state of clock enable
#define CLK_ENABLED1		((volatile u32*)(CLOCKS_BASE+0xB4)) // state of clock enable

// resus interrupts
#define CLK_INTR		((volatile u32*)(CLOCKS_BASE+0xB8)) // raw interrupts of resus
#define CLK_INTE		((volatile u32*)(CLOCKS_BASE+0xBC)) // interrupt enable of resus
#define CLK_INTF		((volatile u32*)(CLOCKS_BASE+0xC0)) // interrupt force of resus
#define CLK_INTS		((volatile u32*)(CLOCKS_BASE+0xC4)) // interrupt status of resus

// Clock hardware interface
typedef struct {
	clock_hw_t		clk[CLK_COUNT];	// 0x00: 10 clock lines
#if RP2350
	io32			dftclk_xosc_ctrl; // 0x78
	io32			dftclk_rosc_ctrl; // 0x7C
	io32			dftclk_lposc_ctrl; // 0x80
#endif
	clock_resus_hw_t	resus;		// 0x78 (0x84): resus registers
	fc_hw_t			fc0;		// 0x80 (0x8C): frequency counter
	// sleep state
	io32			wake_en0;	// 0xA0 (0xAC): enable clock in wake mode
	io32			wake_en1;	// 0xA4 (0xB0): enable clock in wake mode
	io32			sleep_en0;	// 0xA8 (0xB4): enable clock in sleep mode
	io32			sleep_en1;	// 0xAC (0xB8): enable clock in sleep mode
	io32			enabled0;	// 0xB0 (0xBC): state of clock enable
	io32			enabled1;	// 0xB4 (0xC0): state of clock enable
	// resus interrupts
	io32			intr;		// 0xB8 (0xC4): raw interrupts of resus
	io32			inte;		// 0xBC (0xC8): interrupt enable of resus
	io32			intf;		// 0xC0 (0xCC): interrupt force of resus
	io32			ints;		// 0xC4 (0xD0): interrupt status of resus
} clocks_hw_t;

#define clocks_hw ((clocks_hw_t*)CLOCKS_BASE)

#if RP2350
STATIC_ASSERT(sizeof(clocks_hw_t) == 0xD4, "Incorrect clocks_hw_t!");
#else
STATIC_ASSERT(sizeof(clocks_hw_t) == 0xC8, "Incorrect clocks_hw_t!");
#endif

// check if clock has glitchless mux
#define GLITCHLESS_MUX(clk) (((clk) == CLK_SYS) || ((clk) == CLK_REF))

// get clock line hardware interface from clock line index
INLINE clock_hw_t* ClockGetHw(int clk) { return &clocks_hw->clk[clk]; }

// get clock line index from clock line hardware interface
INLINE u8 ClockGetInx(const clock_hw_t* hw) { return (u8)(hw - clocks_hw->clk); }

// current clock frequency in Hz (clock lines and clock generators)
extern u32 CurrentFreq[CLK_SRC_NUM];

// resus callback function type
typedef void (*resus_callback_t)(void);

// resus callback user function (NULL=not used)
extern resus_callback_t ResusCallback;

// Get current clock frequency in [Hz] of clock line or clock generator.
//  clk ... clock line or clock generator CLK_*
INLINE u32 ClockGetHz(int clk) { return CurrentFreq[clk]; }

// current clock source of clock lines (0 = no clock source)
extern u8 CurrentClkSrc[CLK_SRC_NUM];

// Get current clock source of clock line or clock generator (returns CLK_*, or 0 = no clock source)
//  clk ... clock line or clock generator CLK_*
INLINE u8 ClockGetSrc(int clk) { return CurrentClkSrc[clk]; }

// convert clock line to frequency counter internal index
extern const u8 ClockFreqTab[];

// convert frequency counter internal index to clock line (CLK_SRC_NUM = not valid index)
extern const u8 ClockFC0Tab[];

// clock source names
extern const char* ClockName[CLK_SRC_NUM];

// Get short clock name of clock line or clock generator (returns string "GPOUT0" ... "GPIN1")
//  clk ... clock line or clock generator CLK_*
INLINE const char* ClockGetName(int clk) { return ClockName[clk]; }

// stop clock CLK_* (cannot stop SYS and REF clocks)
void ClockStop(int clk);

// setup clock line (returns new frequency in Hz or 0 on error)
//  clk ... clock line index CLK_GPOUT0..CLK_RTC (CLK_ADC)
//  clksrc ... clock source CLK_REF..CLK_GPIN1 (CLK_PLL_OPCG) (see table which sources are supported)
//  freq ... required frequency in Hz, 0=use source frequency
//  freqsrc ... frequency in Hz of source, 0=get from table (must be freqsrc >= freq)
u32 ClockSetup(int clk, int clksrc, u32 freq, u32 freqsrc);

// initialize clocks after start
void ClockInit(void);

#if USE_PLL	// use PLL phase-locked loop (sdk_pll.c, sdk_pll.h)

// set system clock PLL to new setup (dependent clocks are not updated)
//   fbdiv ... feedback divisor, 16..320
//   div1 ... post divider 1, 1..7
//   div2 ... post divider 2, 1..7 (should be div1 >= div2, but auto-corrected)
void ClockPllSysSetup(int fbdiv, int div1, int div2);

// set system clock PLL to new frequency in kHz (dependent clocks are not updated)
//   freq ... required frequency in [kHz]
void ClockPllSysFreq(u32 freq);

// get recommended flash divider by system clock in kHz
int GetClkDivBySysClock(u32 freq);

// get recommended voltage by system clock in kHz (return VREG_VOLTAGE_1_10 .. VREG_VOLTAGE_1_30)
int GetVoltageBySysClock(u32 freq);

// set system clock PLL to new frequency in kHz and auto-set system voltage and flash divider (dependent clocks are not updated)
//   freq ... required frequency in [kHz]
void ClockPllSysFreqVolt(u32 freq);

#endif // USE_PLL	// use PLL phase-locked loop (sdk_pll.c, sdk_pll.h)

// precise measure frequency of clock CLK_REF..CLK_GPIN1 (CLK_PLL_OPCG) with result in Hz
//  clk ... clock line or clock generator CLK_*
//  - measure interval: 128 ms, resolution +-15 Hz
u32 FreqCount(int clk);

// fast measure frequency of clock CLK_REF..CLK_GPIN1 (CLK_PLL_OPCG) with result in kHz
//  clk ... clock line or clock generator CLK_*
//  - measure interval: 2 ms, resolution +-1 kHz
u32 FreqCountkHz(int clk);

// enable resus function with callback handler (NULL=not used)
// - Resus event come when clk_sys is stopped
void ClockResusEnable(resus_callback_t cb);

// disable resus function
void ClockResusDisable();

// enable divided clock to GPIO pin, set divider (returns new frequency in Hz or 0 on error)
//  gpio ... GPIO pin 21, 23, 24 or 25 (only GPIO21 is available on the Pico board)
//  clksrc ... clock source CLK_REF..CLK_GPIN1 (see table which sources are supported)
//  clkdiv ... clock_divider * 256 (default 0x100, means clock_divider=1.00)
u32 ClockGpoutDiv(int gpio, int clksrc, u32 clkdiv);
INLINE u32 ClockGpoutDivFloat(int gpio, int clksrc, float clkdiv)
	{ return ClockGpoutDiv(gpio, clksrc, (u32)(clkdiv*256 + 0.5f)); }

// enable divided clock to GPIO pin, set frequency (returns new frequency in Hz or 0 on error)
//  gpio ... GPIO pin 21, 23, 24 or 25 (only GPIO21 is available on the Pico board)
//  clksrc ... clock source CLK_REF..CLK_GPIN1 (see table which sources are supported)
//  freq ... required frequency in Hz, 0=use source frequency
u32 ClockGpoutFreq(int gpio, int clksrc, u32 freq);

// disable divided clock to GPIO pin
//  gpio ... GPIO pin 21, 23, 24 or 25 (only GPIO21 is available on the Pico board)
void ClockGpoutDisable(int gpio);

// Configure a clock to come from a gpio input (returns new frequency in Hz or 0 on error)
//  clk ... clock line index CLK_GPOUT0..CLK_RTC (CLK_ADC)
//  gpio ... GPIO pin 20 or 22 (RP2350: 12, 14, 20 or 22)
//  freq ... required frequency in Hz
//  freqsrc ... frequency in Hz of source (must be freqsrc >= freq)
u32 ClockGpinSetup(int clk, int gpio, u32 freq, u32 freqsrc);

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

#define configured_freq CurrentFreq

// check if clock has glitchless mux
INLINE bool has_glitchless_mux(enum clock_index clk) { return GLITCHLESS_MUX(clk); }

// stop clock CLK_* (cannot stop SYS and REF clocks)
INLINE void clock_stop(enum clock_index clk) { ClockStop(clk); }

// initialize clocks after start
INLINE void clocks_init(void) { ClockInit(); }

// Configure the specified clock
bool clock_configure(enum clock_index clk_index, u32 src, u32 auxsrc, u32 src_freq, u32 freq);

// Get current clock frequency in [Hz] of clock line or clock generator.
//  clk ... clock line or clock generator CLK_*
INLINE u32 clock_get_hz(enum clock_index clk) { return ClockGetHz(clk); }

// set frequency reported by clock_get_hz() in Hz
INLINE void clock_set_reported_hz(enum clock_index clk, u32 hz) { CurrentFreq[clk] = hz; }

// measure frequency of clock CLK_REF..CLK_GPIN1 with result in kHz
//  clk ... clock line or clock generator CLK_*
//  - measure interval: 2 ms, resolution +-1 kHz
INLINE u32 frequency_count_khz(uint clk)
{
#if RP2040
	if ((clk < 1) || (clk > 13)) return 0;
#else
	if ((clk < 1) || (clk > 16)) return 0;
#endif
	return FreqCountkHz(ClockFC0Tab[clk]);
}

// measure frequency of clock with result in MHz
INLINE float frequency_count_mhz(uint clk) { return (float)frequency_count_khz(clk) / 1000; }

// Enable the resus function. Restarts clk_sys if it is accidentally stopped.
INLINE void clocks_enable_resus(resus_callback_t resus_callback) { ClockResusEnable(resus_callback); }

// Output an optionally divided clock to the specified gpio pin (Valid GPIOs are: 21, 23, 24, 25)
void clock_gpio_init_int_frac(uint gpio, uint src, u32 div_int, u8 div_frac);

INLINE void clock_gpio_init(uint gpio, uint src, float div)
{
	u32 k = (u32)(div*256 + 0.5f);
	clock_gpio_init_int_frac(gpio, src, (k >> 8), (u8)k);
}

// Configure a clock to come from a gpio input (Valid GPIOs are: 20 and 22)
bool clock_configure_gpin(enum clock_index clk_index, uint gpio, u32 src_freq, u32 freq);

// Initialise the system clock to 48MHz
void set_sys_clock_48mhz(void);

// Initialise the system clock
void set_sys_clock_pll(uint32_t vco_freq, uint post_div1, uint post_div2);

// Check if a given system clock frequency is valid/attainable
bool check_sys_clock_khz(uint32_t freq_khz, uint *vco_freq_out, uint *post_div1_out, uint *post_div2_out);

// Attempt to set a system clock frequency in khz
bool set_sys_clock_khz(u32 freq_khz, bool required);

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_CLOCKS_H
