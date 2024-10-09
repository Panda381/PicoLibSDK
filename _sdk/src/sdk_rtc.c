
// ****************************************************************************
//
//                         RTC Real-time clock (only RP2040)
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

#if RP2040

#if USE_RTC	// use RTC Real-time clock (sdk_rtc.c, sdk_rtc.h)

#include "../inc/sdk_rtc.h"
#include "../inc/sdk_irq.h"
#include "../inc/sdk_cpu.h"
#include "../inc/sdk_clocks.h"
#include "../inc/sdk_reset.h"
#include "../../_lib/inc/lib_print.h"

// current RTC alarm callback (NULL=none)
pRtcAlarm RtcAlarm = NULL;

// conversion from old datetime_t to new sDateTime
void Old2NewDateTime(const datetime_t* old_dt, sDateTime* new_dt)
{
	new_dt->timelow = 0;
	new_dt->dayofweek = old_dt->dotw;
	new_dt->sec = old_dt->sec;
	new_dt->min = old_dt->min;
	new_dt->hour = old_dt->hour;
	new_dt->day = old_dt->day;
	new_dt->mon = old_dt->month;
	new_dt->year = old_dt->year;
}

// conversion from new sDateTime to old datetime_t
void New2OldDateTime(const sDateTime* new_dt, datetime_t* old_dt)
{
	old_dt->dotw = new_dt->dayofweek;
	old_dt->sec = new_dt->sec;
	old_dt->min = new_dt->min;
	old_dt->hour = new_dt->hour;
	old_dt->day = new_dt->day;
	old_dt->month = new_dt->mon;
	old_dt->year = new_dt->year;
}

#if USE_ORIGSDK		// include interface of original SDK

// check date and time (returns True = ok)
Bool valid_datetime(const datetime_t* dt)
{
	return 	((u32)dt->sec <= 59) &&
		((u32)dt->min <= 59) &&
		((u32)dt->hour <= 23) &&
		((u32)dt->dotw <= 6) &&
		(dt->day >= 1) &&
		(dt->day <= 31) &&
		(dt->month >= 1) &&
		(dt->month <= 12) &&
		((u32)dt->year <= 4095);
}

#endif // USE_ORIGSDK

// initialize RTC (not running yet - it will start after setting date and time RtcSetDateTime)
void RtcInit()
{
	// get clk_rtc frequency
	u32 freq = ClockGetHz(CLK_RTC);

	// reset RTC
	ResetPeriphery(RESET_RTC);

	// setup clock divider to get 1 Hz clock frequency
	rtc_hw->clkdiv_m1 = freq - 1;
}

// set RTC clock from datetime structure
void RtcSetDateTime(const sDateTime* dt)
{
	// stop RTC
	rtc_hw->ctrl = 0; // clear enable bit

	// wait while RTC is still running
	while (RtcRunning()) { }

	// setup date and time
	rtc_hw->setup_0 = ((u32)dt->year << 12) | ((u32)dt->mon << 8) | dt->day;
	rtc_hw->setup_1 = ((u32)dt->dayofweek << 24) | ((u32)dt->hour << 16) | ((u32)dt->min << 8) | dt->sec;

	// load setup values into RTC
	rtc_hw->ctrl = B4;

	// start RTC
	rtc_hw->ctrl = B0;

	// wait for RTC to be running
	while (!RtcRunning()) {}
}

void RtcSetOldDateTime(const datetime_t* dt)
{
	// stop RTC
	rtc_hw->ctrl = 0; // clear enable bit

	// wait while RTC is still running
	while (RtcRunning()) { }

	// setup date and time
	rtc_hw->setup_0 = ((u32)dt->year << 12) | ((u32)dt->month << 8) | dt->day;
	rtc_hw->setup_1 = ((u32)dt->dotw << 24) | ((u32)dt->hour << 16) | ((u32)dt->min << 8) | dt->sec;

	// load setup values into RTC
	rtc_hw->ctrl = B4;

	// start RTC
	rtc_hw->ctrl = B0;

	// wait for RTC to be running
	while (!RtcRunning()) {}
}

// get RTC to datetime structure
void RtcGetDateTime(sDateTime* dt)
{
	// read registers (RTC_0 must be read before RTC_1)
	u32 rtc0 = rtc_hw->rtc_0;
	cb(); // compiler barrier
	u32 rtc1 = rtc_hw->rtc_1;

	// get date and time
	dt->timelow = 0; // clear us and ms
	dt->dayofweek = (s8)((rtc0 >> 24) & 0x0f);
	dt->sec = (s8)(rtc0 & 0x3f);
	dt->min = (s8)((rtc0 >> 8) & 0x3f);
	dt->hour = (s8)((rtc0 >> 16) & 0x1f);
	dt->day = (s8)(rtc1 & 0x1f);
	dt->mon = (s8)((rtc1 >> 8) & 0xf);
	dt->year = (s16)((rtc1 >> 12) & 0xfff);
}

void RtcGetOldDateTime(datetime_t* dt)
{
	// read registers (RTC_0 must be read before RTC_1)
	u32 rtc0 = rtc_hw->rtc_0;
	cb(); // compiler barrier
	u32 rtc1 = rtc_hw->rtc_1;

	// get date and time
	dt->dotw = (s8)((rtc0 >> 24) & 0x0f);
	dt->sec = (s8)(rtc0 & 0x3f);
	dt->min = (s8)((rtc0 >> 8) & 0x3f);
	dt->hour = (s8)((rtc0 >> 16) & 0x1f);
	dt->day = (s8)(rtc1 & 0x1f);
	dt->month = (s8)((rtc1 >> 8) & 0xf);
	dt->year = (s16)((rtc1 >> 12) & 0xfff);
}

// enable RTC alarm
void RtcAlarmEnable()
{
	// set global match enable bit
	RegSet(&rtc_hw->irq_setup_0, B28);

	// wait for it to start being active
	while ((rtc_hw->irq_setup_0 & B29) == 0) {}
}

// disable RTC alarm
void RtcAlarmDisable()
{
	// clear force request
	rtc_hw->intf = 0;

	// clear global match enable bit
	RegClr(&rtc_hw->irq_setup_0, B28);

	// wait for it to stop being active
	while ((rtc_hw->irq_setup_0 & B29) != 0) {}
}

// check datetime structure, if alarm will be repeated (some entry has value = -1)
Bool RtcDateTimeRepeated(const sDateTime* dt)
{
	return	(dt->sec < 0) ||
		(dt->min < 0) ||
		(dt->hour < 0) ||
		(dt->day < 0) ||
		(dt->mon < 0) ||
		(dt->year < 0);
}

// check if RTC alarm is repeated
Bool RtcRepeated()
{
	return ((rtc_hw->irq_setup_0 & (B26|B25|B24)) != (B26|B25|B24)) ||
		((rtc_hw->irq_setup_1 & (B30|B29|B28)) != (B30|B29|B28));
}

// alarm interrupt handler (default name isr_rtc)
static void RtcHandler()
{
	// disable alarm to clear current IRQ
	RtcAlarmDisable();

	// re-enable alarm, if repeated
	if (RtcRepeated()) RtcAlarmEnable();

	// callback user function
	if (RtcAlarm != NULL) RtcAlarm();
}

// start alarm from datetime structure (set entries to -1 to disable matching and enable repeating)
// - to deactivate, use RtcAlarmStop() function
void RtcAlarmStart(const sDateTime* dt, pRtcAlarm callback)
{
	// disable alarm
	RtcAlarmDisable();

	// setup alarm date and time
	u32 k = 0;
	if (dt->year >= 0) k = ((u32)dt->year << 12) | B26;
	if (dt->mon >= 0) k |= ((u32)dt->mon << 8) | B25;
	if (dt->day >= 0) k |= dt->day | B24;
	rtc_hw->irq_setup_0 = k;

	k = 0;
	if (dt->dayofweek >= 0) k = ((u32)dt->dayofweek << 24) | B31;
	if (dt->hour >= 0) k |= ((u32)dt->hour << 16) | B30;
	if (dt->min >= 0) k |= ((u32)dt->min << 8) | B29;
	if (dt->sec >= 0) k |= dt->sec | B28;
	rtc_hw->irq_setup_1 = k;

	// save callback	
	RtcAlarm = callback;

	// set IRQ handler
	SetHandler(IRQ_RTC, RtcHandler);

	// enable IRQ of RTC
	rtc_hw->inte = B0;

	// enable IRQ of NVIC
	NVIC_IRQEnable(IRQ_RTC);

	// enable alarm
	RtcAlarmEnable();
}

#if USE_ORIGSDK		// include interface of original SDK

// Set a time in the future for the RTC to call a user provided callback
void rtc_set_alarm(const datetime_t *t, rtc_callback_t user_callback)
{
	// disable alarm
	RtcAlarmDisable();

	// setup alarm date and time
	u32 k = 0;
	if (t->year >= 0) k = ((u32)t->year << 12) | B26;
	if (t->month >= 0) k |= ((u32)t->month << 8) | B25;
	if (t->day >= 0) k |= t->day | B24;
	rtc_hw->irq_setup_0 = k;

	k = 0;
	if (t->dotw >= 0) k = ((u32)t->dotw << 24) | B31;
	if (t->hour >= 0) k |= ((u32)t->hour << 16) | B30;
	if (t->min >= 0) k |= ((u32)t->min << 8) | B29;
	if (t->sec >= 0) k |= t->sec | B28;
	rtc_hw->irq_setup_1 = k;

	// save callback	
	RtcAlarm = user_callback;

	// set IRQ handler
	SetHandler(IRQ_RTC, RtcHandler);

	// enable IRQ of RTC
	rtc_hw->inte = B0;

	// enable IRQ of NVIC
	NVIC_IRQEnable(IRQ_RTC);

	// enable alarm
	RtcAlarmEnable();
}

static const char *DATETIME_MONTHS[12] =
{
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};

static const char *DATETIME_DOWS[7] =
{
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
};

// Convert a datetime_t structure to a string
void datetime_to_str(char *buf, uint buf_size, const datetime_t *t)
{
	snprintf(buf,
		buf_size,
		"%s %d %s %d:%02d:%02d %d",
		DATETIME_DOWS[t->dotw],
		t->day,
		DATETIME_MONTHS[t->month - 1],
		t->hour,
		t->min,
		t->sec,
		t->year);
}

#endif // USE_ORIGSDK

// stop alarm (deactivate callback)
void RtcAlarmStop()
{
	// disable IRQ of NVIC
	NVIC_IRQDisable(IRQ_RTC);

	// disable IRQ of RTC
	rtc_hw->inte = 0;

	// disable alarm
	RtcAlarmDisable();

	// clear callback
	RtcAlarm = NULL;
}

#endif // USE_RTC	// use RTC Real-time clock (sdk_rtc.c, sdk_rtc.h)

#endif // RP2040
