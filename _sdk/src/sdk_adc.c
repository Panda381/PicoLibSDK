
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

#include "../../global.h"	// globals

#if USE_ADC		// use ADC controller (sdk_adc.c, sdk_adc.h)

#include "../inc/sdk_reset.h"
#include "../inc/sdk_gpio.h"
#include "../inc/sdk_clocks.h"
#include "../inc/sdk_adc.h"
#include "../inc/sdk_timer.h"
#include "../../_lib/inc/lib_config.h"

// temperature noise reduction
#if TEMP_NOISE > 0
u16 ADCTempHist[TEMP_NOISE];		// deep of temperature history
u32 ADCTempHistSum = 0;			// sum of history
int ADCTempHistNum = 0;			// number of entries
int ADCTempHistInx = 0;			// index in temperature history
#endif // TEMP_NOISE

// ADC init
void ADC_Init(void)
{
	// reset ADC
	ADC_Term();

#if TEMP_NOISE > 0
	// clear noise rediction history
	ADCTempHistSum = 0;
	ADCTempHistNum = 0;
	ADCTempHistInx = 0;
#endif

	// enable ADC, reset setup
	adc_hw->cs = B0;

	// wait for power-up sequence
	ADC_Wait();
}

// ADC terminate
void ADC_Term(void)
{
	// reset ADC
	ResetPeriphery(RESET_ADC);
}

// initialize GPIO to use as an ADC pin
//  pin ... pin 26 to 29 (or 40 to 47)
void ADC_PinInit(int pin)
{
	// disable all functions to make output driver high impedance
	GPIO_Fnc(pin, GPIO_FNC_NULL);

	// disable pulls
	GPIO_NoPull(pin);

	// disable input
	GPIO_InDisable(pin);
}

// terminate pin as ADC input
//  pin ... pin 26 to 29 (or 40 to 47)
void ADC_PinTerm(int pin)
{
	GPIO_Reset(pin);
}

// enable temperature sensor
void ADC_TempEnable(void)
{
	// if not enabled, wait some time to stabilize (requires min. 500 us)
	if ((adc_hw->cs & B1) == 0)
	{
		RegSet(&adc_hw->cs, B1);
		WaitMs(1);
	}
}

// do single conversion (takes 96 clock cycles = min. 2 us on 48 MHz clock, real 2.6 us)
u16 ADC_Single(void)
{
	ADC_StartOnce(); // start single conversion
	ADC_Wait(); // wait for end of conversion
	return ADC_Result(); // get conversion result
}

// do single conversion with denoise (returns value 0..0xffff; takes min. 32 us on 48 MHz clock, real 43 us)
u16 ADC_SingleDenoise(void)
{
	int i;
	u16 t = 0;
	for (i = 16; i > 0; i--) t += ADC_Single();
	return t;
}

// do single conversion and recalculate to voltage on 3.3V
float ADC_SingleU(void)
{
#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
	return ADC_SingleDenoise() * ConfigGetAdcRef() / (4096*16);
#else
	return ADC_SingleDenoise() * 3.3f / (4096*16);
#endif
}

// do single conversion and recalculate to voltage on 3.3V, integer in mV (range 0..3300)
int ADC_SingleUint(void)
{
#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
	return ADC_SingleDenoise() * ConfigGetAdcRefInt() / (4096*16);
#else
	return ADC_SingleDenoise() * 3300 / (4096*16);
#endif
}

// set ADC clock divisor as float (with resolution 1/256)
//  clkdiv ... clock divisor (if non-zero, ADC_MultiEnable() will start conversions with that interval)
// Interval of samples will be (1 + clkdiv) cycles. One conversion takes 96 cycles, so minimal value
// of clkdiv is 96 (period 2 us, 500 kHz). Maximal value is 65535 (period 1365 us, 732 Hz).
void ADC_ClkDivFloat(float clkdiv)
{
	adc_hw->div = (u32)(clkdiv*256 + 0.5f);
}

// set ADC sampling frequency of repeated conversions
//  freq ... sampling frequency in Hz, minimal 732 Hz, maximal 500000 Hz, 0 = switch off
void ADC_Freq(u32 freq)
{
	// switch off
	if (freq == 0)
	{
		ADC_ClkDiv(0);
		return;
	}

	// get current frequency of ADC
	u32 freq0 = CurrentFreq[CLK_ADC];

	// clock divisor * 256
	u32 clkdiv = (u32)(((u64)freq0*256 + freq/2) / freq);

	// limit range
	if (clkdiv > 0x10000FF) clkdiv = 0x10000FF;
	if (clkdiv < 0x6000) clkdiv = 0x6000;

	// set clock divisor
	ADC_ClkDiv(clkdiv - 256);
}

// get current sampling frequency in Hz (732 to 500000 Hz, 0 = off)
u32 ADC_GetFreq(void)
{
	// switch off
	u32 clkdiv = adc_hw->div & 0xffffff;
	if (clkdiv == 0) return 0;
	clkdiv += 256;

	// get current frequency of ADC
	u32 freq0 = CurrentFreq[CLK_ADC];

	// frequency
	return (u32)(((u64)freq0*256 + clkdiv/2) / clkdiv);
}

// setup ADC FIFO (FIFO is 4 samples long)
//  en ... True to enable write each conversion result to the FIFO
//  shift ... True to right-shift result to be one byte 8-bit in size (enables DMA to byte buffers)
//  err ... True means bit 15 of FIFO contains error flag for each sample
//  dreq_en ... enable DMA requests when FIFO contains data
//  dreq_thresh ... threshold for DMA requests/FIFO IRQ if enabled (DREQ/IRQ asserted then level >= threshold)
void ADC_FifoSetup(Bool en, Bool shift, Bool err, Bool dreq_en, int dreq_thresh)
{
	RegMask(ADC_FCS, (en ? B0:0) | (shift ? B1:0) | (err ? B2:0) | (dreq_en ? B3:0)
		| ((u32)dreq_thresh << 24), B0+B1+B2+B3 + B24+B25+B26+B27);
}

// get value from ADC FIFO and wait to have data (bit 15 can contain error flag)
u16 ADC_FifoWait(void)
{
	while (ADC_IsEmpty()) {}
	return ADC_Fifo();
}

// discard FIFO results
void ADC_FifoFlush(void)
{
	// wait for end of conversion
	ADC_Wait();

	// flush FIFO
	while (!ADC_IsEmpty()) (void)ADC_Fifo();
}

// get current temperature in °C
float ADC_Temp(void)
{
	// enable temperature sensor
	ADC_TempEnable();
	
	// select ADC input
	ADC_Mux(ADC_MUX_TEMP);

	// do single conversion with fast denoise
	u16 t = ADC_SingleDenoise();

	// temperature noise reduction
#if TEMP_NOISE > 0
	ADCTempHistSum += t;
	if (ADCTempHistNum < TEMP_NOISE) // initialize
	{
		ADCTempHistNum++;
		ADCTempHist[ADCTempHistInx] = t;
		t = (ADCTempHistSum + ADCTempHistNum/2) / ADCTempHistNum;
	}
	else
	{
		ADCTempHistSum -= ADCTempHist[ADCTempHistInx];
		ADCTempHist[ADCTempHistInx] = t;
		t = (ADCTempHistSum + TEMP_NOISE/2) / TEMP_NOISE;
	}
	ADCTempHistInx++;
	if (ADCTempHistInx >= TEMP_NOISE) ADCTempHistInx = 0;
#endif // TEMP_NOISE

	// do single conversion (T = 27 - (ADC_voltage - 0.706) / 0.001721)
#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
	return 27 - (t*ConfigGetAdcRef()/(4096*16) - ConfigGetTempBase()) / ConfigGetTempSlope();
#else
	return 27 - (t*ADC_UREF/(4096*16) - TEMP_BASE) / TEMP_SLOPE;
#endif
}

#endif // USE_ADC		// use ADC controller (sdk_adc.c, sdk_adc.h)
