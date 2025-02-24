
// ****************************************************************************
//
//                         Power Manager (only RP2350)
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

#if RP2350

#include "../inc/sdk_cpu.h"
#include "../inc/sdk_gpio.h"
#include "../inc/sdk_powman.h"

// set time of power manager in [ms] (set time ot AON always-on timer)
void PowMan_SetTime(u64 time)
{
	// check if timer was running
	Bool running = PowMan_IsRunning();

	// stop timer
	if (running) PowMan_Stop();

	// write time
	PowMan_Write(&powman_hw->set_time_15to0, time & 0xffff);
	PowMan_Write(&powman_hw->set_time_31to16, (time >> 16) & 0xffff);
	PowMan_Write(&powman_hw->set_time_47to32, (time >> 32) & 0xffff);
	PowMan_Write(&powman_hw->set_time_63to48, (time >> 48) & 0xffff);

	// run again
	if (running) PowMan_Start();
}

// get time of power manager in [ms] (get time ot AON always-on timer)
u64 PowMan_GetTime(void)
{
	u32 low, high;
	u32 high2 = powman_hw->read_time_upper;  // get time HIGH

	do {
		// accept new time HIGH
		high = high2;

		// get time LOW
		cb(); // compiler barrier (or else the compiler could swap the order of operations)
		low = powman_hw->read_time_lower;

		// get time HIGH again
		cb(); // compiler barrier (or else the compiler could swap the order of operations)
		high2 = powman_hw->read_time_upper;

	// check that HIGH has not changed, otherwise a re-read will be necessary
	} while (high != high2);

	// return result value
	return ((u64)high << 32) | low;
}

// Setup power manager timer source
//  src ... clock source POWMAN_SRC_*
//  freq ... clock frequency at Hz (must be 1000 or 1 for GPIO input)
//	GPIO with frequency 1 Hz will synchronize low power or crystal oscillator
void PowMan_SetSrc(int src, u32 freq)
{
	// check if timer was running
	Bool running = PowMan_IsRunning();

	// stop timer
	if (running) PowMan_Stop();

	// setup source
	u32 freq_int, freq_frac;
	u8 n;
	switch (src)
	{
	case POWMAN_SRC_LPOSC:		// low power oscillator (usually 32 kHz)
		freq_int = freq / 1000;
		freq_frac = (freq - freq_int*1000) * 65536 / 1000;
		PowMan_Write(&powman_hw->lposc_freq_khz_int, freq_int);		// set frequency integer
		PowMan_Write(&powman_hw->lposc_freq_khz_frac, freq_frac);	// set frequency fraction
		PowMan_SetBits(&powman_hw->timer, B8);				// switch to low power oscillator
		if (running)
		{
			PowMan_Start();						// run again
			while ((powman_hw->timer & B8) == 0) {}			// wait to start
		}
		break;

	case POWMAN_SRC_XOSC:		// crystal oscillator
		freq_int = freq / 1000;
		freq_frac = (freq - freq_int*1000) * 65536 / 1000;
		PowMan_Write(&powman_hw->xosc_freq_khz_int, freq_int);		// set frequency integer
		PowMan_Write(&powman_hw->xosc_freq_khz_frac, freq_frac);	// set frequency fraction
		PowMan_SetBits(&powman_hw->timer, B9);				// switch to crystal oscillator
		if (running)
		{
			PowMan_Start();						// run again
			while ((powman_hw->timer & B9) == 0) {}			// wait to start
		}
		break;

	case POWMAN_SRC_GPIO12:		// GPIO12 input (only 1 kHz or 1 Hz)
		n = 0;
		goto PowMan_SetSrc_Gpio;

	case POWMAN_SRC_GPIO14:		// GPIO14 input (only 1 kHz or 1 Hz)
		n = 2;
		goto PowMan_SetSrc_Gpio;

	case POWMAN_SRC_GPIO20:		// GPIO20 input (only 1 kHz or 1 Hz)
		n = 1;
		goto PowMan_SetSrc_Gpio;

	case POWMAN_SRC_GPIO22:		// GPIO22 input (only 1 kHz or 1 Hz)
		n = 3;
PowMan_SetSrc_Gpio:
		GPIO_InEnable(src);			// enable GPIO input
		PowMan_Write(&powman_hw->ext_time_ref, n); // select GPIO pin

		// use 1 Hz mode
		if (freq < 50)
		{
			PowMan_SetBits(&powman_hw->timer, B13);			// switch to GPIO 1 Hz
			if (running)
			{
				PowMan_Start();					// run again
				while ((powman_hw->timer & B19) == 0) {}	// wait to start
			}
		}

		// use 1 kHz mode
		else
		{
			PowMan_SetBits(&powman_hw->timer, B10);			// switch to GPIO 1 kHz
			if (running)
			{
				PowMan_Start();					// run again
				while ((powman_hw->timer & B18) == 0) {}	// wait to start
			}
		}
		break;
	}
}

// set power state of domains (returns False if new state is declined)
//  state ... combination of POWMAN_DOMAIN_*, 1=powered up, 0=powered down
Bool PowMain_SetPowerState(int state)
{
	// clear IGNORED REQUEST bit
	PowMan_ClrBits(&powman_hw->state, B8); // REQ_IGNORED

	// set request to new power state
	PowMan_Write(&powman_hw->state, (~state << 4) & 0xf0);

	// has it been ignored due to ongoing request?
	if ((powman_hw->state & B8) != 0) return False; // REQ_IGNORED

	// is required new state invalid?
	if ((powman_hw->state & B10) != 0) return False; // BAD_SW_REQ

	// we are turning off switched core, so wait for proc to go to sleep
	if ((state & POWMAN_DOMAIN_SWCORE) == 0)
	{
		// short delay
		int i;
		for (i = 100; i > 0; i--)
		{
			// request accepted
			if ((powman_hw->state & B12) != 0) return True; // WAITING
		}

		// timeout, pending power up request
		return False;
	}

	// wait while state is changing
	while ((powman_hw->state & B13) != 0) {} // CHANGING

	return True;
}

// set alarm on at time in [ms]
void PowMan_AlarmOn(u64 time)
{
	// disable alarm
	PowMan_ClrBits(&powman_hw->timer, B4); // ALARM_ENAB

	// write alarm time
	PowMan_Write(&powman_hw->alarm_time_15to0, time & 0xffff);
	PowMan_Write(&powman_hw->alarm_time_31to16, (time >> 16) & 0xffff);
	PowMan_Write(&powman_hw->alarm_time_47to32, (time >> 32) & 0xffff);
	PowMan_Write(&powman_hw->alarm_time_63to48, (time >> 48) & 0xffff);

	// clear alarm
	PowMan_ClearAlarm();

	// enable TIMER interrupt
	PowMan_SetBits(&powman_hw->inte, B1); // TIMER

	// enable alarm
	PowMan_SetBits(&powman_hw->timer, B4); // ALARM_ENAB
}

// set alarm off
void PowMan_AlarmOff(void)
{
	// disable TIMER interrupt
	PowMan_ClrBits(&powman_hw->inte, B1); // TIMER

	// disable alarm
	PowMan_ClrBits(&powman_hw->timer, B4); // ALARM_ENAB
}

// setup alarm to wake up at time in [ms]
void PowMan_WakeUpOn(u64 time)
{
	// set alarm on at time in [ms]
	PowMan_AlarmOn(time);

	// setup wake up
	PowMan_SetBits(&powman_hw->timer, B5); // PWRUP_ON_ALARM
}

// disable wake up
void PowMan_WakeUpOff(void)
{
	// set alarm off
	PowMan_AlarmOff();

	// disable wake up
	PowMan_ClrBits(&powman_hw->timer, B5); // PWRUP_ON_ALARM
}

// setup wake up by GPIO
//  event ... wakeup event 0..3
//  gpio ... gpio to wake up from (0..47)
//  edge ... True=edge sensitive, False=level sensitive
//  high ... True=active high or rising, False=active low or falling
void PowMan_GpioOn(int event, int gpio, Bool edge, Bool high)
{
	// disable
	PowMan_GpioOff(event);

	// enable GPIO input
	GPIO_InEnable(gpio);

	// set power up register
	u32 pwrup = gpio; // SOURCE
	if (edge) pwrup |= B8; // MODE
	if (high) pwrup |= B7; // DIRECTION
	PowMan_Write(&powman_hw->pwrup[event], pwrup);

	// clear state
	PowMan_ClrBits(&powman_hw->pwrup[event], B9); // STATUS

	// enable
	PowMan_SetBits(&powman_hw->pwrup[event], B6); // ENABLE
}

// disable all wake ups
void PowMan_WakeAllOff(void)
{
	PowMan_GpioOff(0);
	PowMan_GpioOff(1);
	PowMan_GpioOff(2);
	PowMan_GpioOff(3);
	PowMan_WakeUpOff();
}

// set voltage VREG_VOLTAGE_*
// - OR required value with VREG_VOLTAGE_UNLOCK flag, to enable unlock higher voltages than 1.30V
void VregSetVoltage(int vreg)
{
	// enable VREG setup
	PowMan_SetBits(&powman_hw->vreg_ctrl, B13); // UNLOCK

	// unlock high voltages
	if ((vreg & VREG_VOLTAGE_UNLOCK) != 0)
		PowMan_SetBits(&powman_hw->vreg_ctrl, B8); // UNLOCK
	else
		PowMan_ClrBits(&powman_hw->vreg_ctrl, B8); // LOCK
	vreg &= 0x1f;

	// wait prior change
	VregWait();

	while ((powman_hw->vreg & B15) != 0) {} // UPDATE_IN_PROGRESS

	// set new value
	PowMan_Write(&powman_hw->vreg, (vreg << 4) | (powman_hw->vreg & B1));

	// wait update in progress
	VregWait();
}

// wait for regulated state
void VregWait(void)
{
	while (!VregIsOk()) {}
}

const float VregVoltageTab[32] = {
	0.55f,		// #define VREG_VOLTAGE_0_55	0	// 0.55V
	0.60f,		// #define VREG_VOLTAGE_0_60	1	// 0.60V
	0.65f,		// #define VREG_VOLTAGE_0_65	2	// 0.65V
	0.70f,		// #define VREG_VOLTAGE_0_70	3	// 0.70V
	0.75f,		// #define VREG_VOLTAGE_0_75	4	// 0.75V
	0.80f,		// #define VREG_VOLTAGE_0_80	5	// 0.80V
	0.85f,		// #define VREG_VOLTAGE_0_85	6	// 0.85V
	0.90f,		// #define VREG_VOLTAGE_0_90	7	// 0.90V
	0.95f,		// #define VREG_VOLTAGE_0_95	8	// 0.95V
	1.00f,		// #define VREG_VOLTAGE_1_00	9	// 1.00V
	1.05f,		// #define VREG_VOLTAGE_1_05	10	// 1.05V
	1.10f,		// #define VREG_VOLTAGE_1_10	11	// 1.10V *default state
	1.15f,		// #define VREG_VOLTAGE_1_15	12	// 1.15V
	1.20f,		// #define VREG_VOLTAGE_1_20	13	// 1.20V
	1.25f,		// #define VREG_VOLTAGE_1_25	14	// 1.25V
	1.30f,		// #define VREG_VOLTAGE_1_30	15	// 1.30V
			// - higher values are protected, need to set vreg_ctrl
	1.35f,		// #define VREG_VOLTAGE_1_35	16	// 1.35V
	1.40f,		// #define VREG_VOLTAGE_1_40	17	// 1.40V
	1.50f,		// #define VREG_VOLTAGE_1_50	18	// 1.50V
	1.60f,		// #define VREG_VOLTAGE_1_60	19	// 1.60V
	1.65f,		// #define VREG_VOLTAGE_1_65	20	// 1.65V
	1.70f,		// #define VREG_VOLTAGE_1_70	21	// 1.70V
	1.80f,		// #define VREG_VOLTAGE_1_80	22	// 1.80V
	1.90f,		// #define VREG_VOLTAGE_1_90	23	// 1.90V
	2.00f,		// #define VREG_VOLTAGE_2_00	24	// 2.00V
	2.35f,		// #define VREG_VOLTAGE_2_35	25	// 2.35V
	2.50f,		// #define VREG_VOLTAGE_2_50	26	// 2.50V
	2.65f,		// #define VREG_VOLTAGE_2_65	27	// 2.65V
	2.80f,		// #define VREG_VOLTAGE_2_80	28	// 2.80V
	3.00f,		// #define VREG_VOLTAGE_3_00	29	// 3.00V
	3.15f,		// #define VREG_VOLTAGE_3_15	30	// 3.15V
	3.30f,		// #define VREG_VOLTAGE_3_30	31	// 3.30V
};

// get voltage in volts
float VregVoltageFloat(void) { return VregVoltageTab[VregVoltage()]; }

#endif // RP2350
