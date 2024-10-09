
// ****************************************************************************
//
//                                  ADC
//                      Analogue to Digital Converter
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

/*
Conversion speed: 500 kS/s (using 48 MHz clock CLK_ADC)
Capturing a sample takes 96 clock cycles (91 x 1/48MHz) = 2 us per sample (500 kS/s)
4 inpus on GPIO26 .. GPIO29, 1 input internal temperature sensor
Temperature: T = 27 - (ADC_voltage - 0.706) / 0.001721

Input voltage should not exceed IOVDD supply voltage (not ADC supply voltage ADC_AVDD).

Hot to use:
1) initialize ADC with ADC_Init()
2) prepare GPIO inputs with ADC_PinInit()
 ... or if using temperature sensor 2) enable temperature sensor with ADC_TempEnable()
3) select input with ADC_Mux()
4) read conversion with ADC_Single() ... takes 2 us
*/

#if USE_ADC		// use ADC controller (sdk_adc.c, sdk_adc.h)

#ifndef _SDK_ADC_H
#define _SDK_ADC_H

#include "../sdk_addressmap.h"		// Register address offsets

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_adc.h"		// constants of original SDK
#else
#include "orig_rp2350/orig_adc.h"		// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// ADC hardware registers
#define ADC_CS		((volatile u32*)(ADC_BASE+0x00)) // control and status
#define ADC_RESULT	((volatile u32*)(ADC_BASE+0x04)) // result
#define ADC_FCS		((volatile u32*)(ADC_BASE+0x08)) // FIFO control and status
#define ADC_FIFO	((volatile u32*)(ADC_BASE+0x0C)) // conversion result FIFO
#define ADC_DIV		((volatile u32*)(ADC_BASE+0x10)) // clock divider
#define ADC_INTR	((volatile u32*)(ADC_BASE+0x14)) // raw interrupt
#define ADC_INTE	((volatile u32*)(ADC_BASE+0x18)) // interrupt enable
#define ADC_INTF	((volatile u32*)(ADC_BASE+0x1C)) // interrupt force
#define ADC_INTS	((volatile u32*)(ADC_BASE+0x20)) // interrupt status

// ADC hardware interface
typedef struct {
	io32	cs;		// 0x00: control and status
	io32	result;		// 0x04: result
	io32	fcs;		// 0x08: FIFO control and status
	io32	fifo;		// 0x0C: FIFO result
	io32	div;		// 0x10: clock divider
	io32	intr;		// 0x14: raw interrupt
	io32	inte;		// 0x18: interrupt enable
	io32	intf;		// 0x1C: interrupt force
	io32	ints;		// 0x20: interrupt status
} adc_hw_t;

#define	adc_hw	((adc_hw_t*)ADC_BASE)

STATIC_ASSERT(sizeof(adc_hw_t) == 0x24, "Incorrect adc_hw_t!");

// ADC inputs
#if RP2350B // QFN-80
#define ADC_MUX_GPIO40	0	// pin GPIO 40
#define ADC_MUX_GPIO41	1	// pin GPIO 41
#define ADC_MUX_GPIO42	2	// pin GPIO 42
#define ADC_MUX_GPIO43	3	// pin GPIO 43
#define ADC_MUX_GPIO44	4	// pin GPIO 44
#define ADC_MUX_GPIO45	5	// pin GPIO 45
#define ADC_MUX_GPIO46	6	// pin GPIO 46
#define ADC_MUX_GPIO47	7	// pin GPIO 47
#define ADC_MUX_TEMP	8	// temperature sensor
#else // QFN-60
#define ADC_MUX_GPIO26	0	// pin GPIO 26
#define ADC_MUX_GPIO27	1	// pin GPIO 27
#define ADC_MUX_GPIO28	2	// pin GPIO 28
#define ADC_MUX_GPIO29	3	// pin GPIO 29
#define ADC_MUX_TEMP	4	// temperature sensor
#endif

// ADC init
void ADC_Init(void);

// ADC terminate
void ADC_Term(void);

// ADC disable
INLINE void ADC_Disable() { RegClr(&adc_hw->cs, B0); }

// ADC enable
INLINE void ADC_Enable() { RegSet(&adc_hw->cs, B0); }

// initialize GPIO to use as an ADC pin
//  pin ... pin 26 to 29 (or 40 to 47)
void ADC_PinInit(int pin);

// terminate pin as ADC input
//  pin ... pin 26 to 29 (or 40 to 47)
void ADC_PinTerm(int pin);

// select ADC input
//  input ... input 0 to 4 or 0 to 8 (use ADC_MUX_*)
#if RP2350B // QFN-80
INLINE void ADC_Mux(int input) { RegMask(&adc_hw->cs, input << 12, 0x0f << 12); }
#else
INLINE void ADC_Mux(int input) { RegMask(&adc_hw->cs, input << 12, 0x07 << 12); }
#endif

// get currently selected ADC input (returns 0 to 4 or 0 to 8)
#if RP2350B // QFN-80
INLINE u8 ADC_GetMux(void) { return (u8)((adc_hw->cs >> 12) & 0x0f); }
#else
INLINE u8 ADC_GetMux(void) { return (u8)((adc_hw->cs >> 12) & 0x07); }
#endif

// set round robin sampling selector
//  mask ... mask of bits B0 to B4 (or B0 to B8) of inputs (use BIT(ADC_MUX_*))
//           set 0 to disable round robin sampling
#if RP2350B // QFN-80
INLINE void ADC_RoundRobin(int mask) { RegMask(&adc_hw->cs, mask << 16, 0x1ff << 16); }
#else
INLINE void ADC_RoundRobin(int mask) { RegMask(&adc_hw->cs, mask << 16, 0x1f << 16); }
#endif

// enable temperature sensor
void ADC_TempEnable(void);

// disable temperature sensor (saves 40 uA)
INLINE void ADC_TempDisable(void) { RegClr(&adc_hw->cs, B1); }

// start single conversion
INLINE void ADC_StartOnce(void) { RegSet(&adc_hw->cs, B2); }

// check if ADC is ready (False if conversion is in progress)
INLINE Bool ADC_Ready(void) { return (adc_hw->cs & B8) != 0; }

// wait for end of conversion
INLINE void ADC_Wait(void) { while (!ADC_Ready()) {} }

// get last conversion result
INLINE u16 ADC_Result(void) { return (u16)(adc_hw->result & 0xfff); }

// do single conversion (takes 96 clock cycles = min. 2 us on 48 MHz clock, real 2.6 us)
u16 ADC_Single(void);

// do single conversion with denoise (returns value 0..0xffff; takes min. 32 us on 48 MHz clock, real 43 us)
u16 ADC_SingleDenoise(void);

// do single conversion and recalculate to voltage on 3.3V
float ADC_SingleU(void);

// do single conversion and recalculate to voltage on 3.3V, integer in mV (range 0..3300)
int ADC_SingleUint(void);

// enable continuously repeated conversion (free-running sampling mode)
INLINE void ADC_MultiEnable(void) { RegSet(&adc_hw->cs, B3); }

// disable continuously repeated conversion
INLINE void ADC_MultiDisable(void) { RegClr(&adc_hw->cs, B3); }

// check if most recent ADC conversion encountered an error; result is undefined or noisy
INLINE Bool ADC_Err(void) { return (adc_hw->cs & B9) != 0; }

// some past ADC conversion encountered an error (clear with ADC_ErrClear())
INLINE Bool ADC_ErrSticky(void) { return (adc_hw->cs & B10) != 0; }

// clear sticky error
INLINE void ADC_ErrClear(void) { RegSet(&adc_hw->cs, B10); }

// set ADC clock divisor
//  clkdiv ... clock divisor * 256 (if non-zero, ADC_MultiEnable() will start conversions with that interval)
// Interval of samples will be (1 + clkdiv/256) cycles. One conversion takes 96 cycles, so minimal value
// of clkdiv is 96*256 = 0x5F00 (period 2 us, 500 kHz). Maximal value is 0xFFFFFF (period 1365 us, 732 Hz).
INLINE void ADC_ClkDiv(int clkdiv) { adc_hw->div = clkdiv; }

// set ADC clock divisor as float (with resolution 1/256)
//  clkdiv ... clock divisor (if non-zero, ADC_MultiEnable() will start conversions with that interval)
// Interval of samples will be (1 + clkdiv) cycles. One conversion takes 96 cycles, so minimal value
// of clkdiv is 96 (period 2 us, 500 kHz). Maximal value is 65535 (period 1365 us, 732 Hz).
void ADC_ClkDivFloat(float clkdiv);

// set ADC sampling frequency of repeated conversions
//  freq ... sampling frequency in Hz, minimal 732 Hz, maximal 500000 Hz, 0 = switch off
void ADC_Freq(u32 freq);

// get current sampling frequency in Hz (732 to 500000 Hz, 0 = off)
u32 ADC_GetFreq(void);

// setup ADC FIFO (FIFO is 4 samples long)
//  en ... True to enable write each conversion result to the FIFO
//  shift ... True to right-shift result to be one byte 8-bit in size (enables DMA to byte buffers)
//  err ... True means bit 15 of FIFO contains error flag for each sample
//  dreq_en ... enable DMA requests when FIFO contains data
//  dreq_thresh ... threshold for DMA requests/FIFO IRQ if enabled (DREQ/IRQ asserted then level >= threshold)
void ADC_FifoSetup(Bool en, Bool shift, Bool err, Bool dreq_en, int dreq_thresh);

// check if ADC FIFO is empty
INLINE Bool ADC_IsEmpty(void) { return ((adc_hw->fcs >> 8) & 1) != 0; }

// check if ADC FIFO is full
INLINE Bool ADC_IsFull(void) { return ((adc_hw->fcs >> 9) & 1) != 0; }

// check if ADC FIFO has been underflow
INLINE Bool ADC_IsUnder(void) { return ((adc_hw->fcs >> 10) & 1) != 0; }

// check if ADC FIFO has been overflow
INLINE Bool ADC_IsOver(void) { return ((adc_hw->fcs >> 11) & 1) != 0; }

// clear flag of ADC FIFO has been underflow
INLINE void ADC_ClearUnder(void) { RegSet(&adc_hw->fcs, B10); }

// clear flag of ADC FIFO has been overflow
INLINE void ADC_ClearOver(void) { RegSet(&adc_hw->fcs, B11); }

// get number of samples waiting in FIFO
INLINE u8 ADC_Level(void) { return (u8)((adc_hw->fcs >> 16) & 0x0f); }

// get value from ADC FIFO (bit 15 can contain error flag)
INLINE u16 ADC_Fifo(void) { return (u16)(adc_hw->fifo & 0x8fff); }

// get value from ADC FIFO and wait to have data (bit 15 can contain error flag)
u16 ADC_FifoWait(void);

// discard FIFO results
void ADC_FifoFlush(void);

// get raw interrupt flag (= FIFO reaches threshold level)
INLINE Bool ADC_IntRaw(void) { return (adc_hw->intr & B0) != 0; }

// enable FIFO interrupt
INLINE void ADC_IntEnable(void) { RegSet(&adc_hw->inte, B0); }

// disable FIFO interrupt
INLINE void ADC_IntDisable(void) { RegClr(&adc_hw->inte, B0); }

// force FIFO interrupt request
INLINE void ADC_Force(void) { RegSet(&adc_hw->intf, B0); }

// clear forced FIFO interrupt request
INLINE void ADC_ForceClear(void) { RegClr(&adc_hw->intf, B0); }

// check if force FIFO interrupt request is set (can be reset with ADC_ForceClear())
INLINE Bool ADC_IsForced(void) { return (adc_hw->intf & B0) != 0; }

// get interrupt status after masking and forcing
INLINE Bool ADC_IntStatus(void) { return (adc_hw->ints & B0) != 0; }

// get current temperature in °C
float ADC_Temp();

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original-SDK

// ADC init
INLINE void adc_init(void) { ADC_Init(); }

// initialize GPIO to use as an ADC pin
//  pin ... pin 26 to 29 or 40 to 47
INLINE void adc_gpio_init(uint pin) { ADC_PinInit(pin); }

// select ADC input
//  input ... input 0 to 4 or 0 to 8 (use ADC_MUX_*)
INLINE void adc_select_input(uint input) { ADC_Mux(input); }

// get currently selected ADC input (returns 0 to 4 or 0 to 8)
INLINE uint adc_get_selected_input(void) { return ADC_GetMux(); }

// set round robin sampling selector
//  mask ... mask of bits B0 to B4 or B0 to B8 of inputs (use BIT(ADC_MUX_*))
//           set 0 to disable round robin sampling
INLINE void adc_set_round_robin(uint mask) { ADC_RoundRobin(mask); }

// enable or disable temperature sensor
INLINE void adc_set_temp_sensor_enabled(bool enable) { if (enable) ADC_TempEnable(); else ADC_TempDisable(); }

// do single conversion
INLINE u16 adc_read(void) { return ADC_Single(); }

// enable continuously repeated conversion (free-running sampling mode)
INLINE void adc_run(bool run) { if (run) ADC_MultiEnable(); else ADC_MultiDisable(); }

// set ADC clock divisor as float (with resolution 1/256)
//  clkdiv ... clock divisor (if non-zero, ADC_MultiEnable() will start conversions with that interval)
// Interval of samples will be (1 + clkdiv) cycles. One conversion takes 96 cycles, so minimal value
// of clkdiv is 96 (period 2 us, 500 kHz). Maximal value is 65535 (period 1365 us, 732 Hz).
INLINE void adc_set_clkdiv(float clkdiv) { ADC_ClkDivFloat(clkdiv); }

// setup ADC FIFO (FIFO is 4 samples long)
//  en ... True to enable write each conversion result to the FIFO
//  dreq_en ... enable DMA requests when FIFO contains data
//  dreq_thresh ... threshold for DMA requests/FIFO IRQ if enabled (DREQ/IRQ asserted then level >= threshold)
//  err_in_fifo ... True means bit 15 of FIFO contains error flag for each sample
//  shift ... True to right-shift result to be one byte 8-bit in size (enables DMA to byte buffers)
INLINE void adc_fifo_setup(bool en, bool dreq_en, u16 dreq_thresh, bool err_in_fifo, bool byte_shift)
	{ ADC_FifoSetup(en, byte_shift, err_in_fifo, dreq_en, dreq_thresh); }

// check if ADC FIFO is empty
INLINE bool adc_fifo_is_empty(void) { return ADC_IsEmpty(); }

// get number of samples waiting in FIFO
INLINE u8 adc_fifo_get_level(void) { return ADC_Level(); }

// get result value from ADC FIFO (bit 15 can contain error flag)
INLINE u16 adc_fifo_get(void) { return ADC_Fifo(); }

// get value from ADC FIFO and wait to have data (bit 15 can contain error flag)
INLINE u16 adc_fifo_get_blocking(void) { return ADC_FifoWait(); }

// discard FIFO results
INLINE void adc_fifo_drain(void) { ADC_FifoFlush(); }

// enable/disable ADC interrupt
INLINE void adc_irq_set_enabled(bool enabled) { adc_hw->inte = !!enabled; }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_ADC_H

#endif // USE_ADC		// use ADC controller (sdk_adc.c, sdk_adc.h)
