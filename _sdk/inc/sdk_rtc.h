
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

// Leap year does not work OK for years 1900 and 2100,
//   so RTC with leap should be used in range 1901..2099.

// Only RP2040. RP2350 uses for this purpose AON always-on timer of Power Manager.

#if RP2040

#if USE_RTC	// use RTC Real-time clock (sdk_rtc.c, sdk_rtc.h)

#ifndef _SDK_RTC_H
#define _SDK_RTC_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "../../_lib/inc/lib_calendar.h"		// datetime structure

#if USE_ORIGSDK		// include interface of original SDK
#include "orig_rp2040/orig_rtc.h"	// constants of original SDK
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// RTC hardware registers
#define RTC_CLKDIV	((volatile u32*)(RTC_BASE+0x00)) // divider minus 1 for 1-second counter
#define RTC_SETUP0	((volatile u32*)(RTC_BASE+0x04)) // setup 0
#define RTC_SETUP1	((volatile u32*)(RTC_BASE+0x08)) // setup 1
#define RTC_CTRL	((volatile u32*)(RTC_BASE+0x0C)) // control and status
#define RTC_IRQSET0	((volatile u32*)(RTC_BASE+0x10)) // IRQ setup 0
#define RTC_IRQSET1	((volatile u32*)(RTC_BASE+0x14)) // IRQ setup 1
#define RTC_1		((volatile u32*)(RTC_BASE+0x18)) // RTC register 1
#define RTC_0		((volatile u32*)(RTC_BASE+0x1C)) // RTC register 0 (read this before RTC_1)
#define RTC_INTR	((volatile u32*)(RTC_BASE+0x20)) // raw interrupts
#define RTC_INTE	((volatile u32*)(RTC_BASE+0x24)) // interrupt enable
#define RTC_INTF	((volatile u32*)(RTC_BASE+0x28)) // interrupt force
#define RTC_INTS	((volatile u32*)(RTC_BASE+0x2C)) // interrupt status

// RTC hardware interface
typedef struct {
	io32	clkdiv_m1;	// 0x00: Divider minus 1 for the 1-second counter
	io32	setup_0;	// 0x04: RTC setup register 0
	io32	setup_1;	// 0x08: RTC setup register 1
	io32	ctrl;		// 0x0C: RTC Control and status
	io32	irq_setup_0;	// 0x10: Interrupt setup register 0
	io32	irq_setup_1;	// 0x14: Interrupt setup register 1
	io32	rtc_1;		// 0x18: RTC register 1
	io32	rtc_0;		// 0x1C: RTC register 0
	io32	intr;		// 0x20: Raw Interrupts
	io32	inte;		// 0x24: Interrupt Enable
	io32	intf;		// 0x28: Interrupt Force
	io32	ints;		// 0x2C: Interrupt status after masking & forcing
} rtc_hw_t;

#define rtc_hw ((rtc_hw_t*)RTC_BASE)

STATIC_ASSERT(sizeof(rtc_hw_t) == 0x30, "Incorrect rtc_hw_t!");

/*
// Structure containing date and time information (old-style from original-SDK)
typedef struct {
	s16	year;	// 0..4095
	s8	month;	// 1..12, 1 is January
	s8	day;	// 1..28,29,30,31 depending on month
	s8	dotw;	// 0..6, 0 is Sunday
	s8	hour;	// 0..23
	s8	min;	// 0..59
	s8	sec;	// 0..59
} datetime_t;
*/

// conversion from old datetime_t to new sDateTime
void Old2NewDateTime(const datetime_t* old_dt, sDateTime* new_dt);

// conversion from new sDateTime to old datetime_t
void New2OldDateTime(const sDateTime* new_dt, datetime_t* old_dt);

// RTC alarm callback
typedef void (*pRtcAlarm)(void);

// current RTC alarm callback (NULL=none)
extern pRtcAlarm RtcAlarm;

// check if RTC is running
INLINE Bool RtcRunning(void) { return (rtc_hw->ctrl & B1) != 0; }

// check if RTC alarm is pending
INLINE Bool RtcPending(void) { return (rtc_hw->ints & B0) != 0; }

// initialize RTC (not running yet - it will start after setting date and time RtcSetDateTime)
void RtcInit(void);

// set RTC clock from datetime structure
void RtcSetDateTime(const sDateTime* dt);
void RtcSetOldDateTime(const datetime_t* dt);

// get RTC to datetime structure
void RtcGetDateTime(sDateTime* dt);
void RtcGetOldDateTime(datetime_t* dt);

// enable RTC alarm
void RtcAlarmEnable(void);

// disable RTC alarm
void RtcAlarmDisable(void);

// check datetime structure, if alarm will be repeated (some entry has value = -1)
Bool RtcDateTimeRepeated(const sDateTime* dt);

// check if RTC alarm is repeated
Bool RtcRepeated(void);

// start alarm from datetime structure (set entries to -1 to disable matching and enable repeating)
// - to deactivate, use RtcAlarmStop() function
void RtcAlarmStart(const sDateTime* dt, pRtcAlarm callback);

// stop alarm (deactivate callback)
void RtcAlarmStop(void);

// force alarm IRQ (must be cleared in IRQ handler)
INLINE void RtcAlarmForce(void) { rtc_hw->intf = B0; }

// unforce alarm IRQ
INLINE void RtcAlarmUnforce(void) { rtc_hw->intf = 0; }

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// Callback function type for RTC alarms
typedef void (*rtc_callback_t)(void);

// Initialise the RTC system
INLINE void rtc_init(void) { RtcInit(); }

// check date and time (returns True = ok)
Bool valid_datetime(const datetime_t* dt);

// Set the RTC to the specified time
INLINE bool rtc_set_datetime(const datetime_t *t)
{
	if (!valid_datetime(t)) return False;
	RtcSetOldDateTime(t);
	return True;
}

// Is the RTC running?
INLINE bool rtc_running(void) { return RtcRunning(); }

// Get the current time from the RTC
INLINE bool rtc_get_datetime(datetime_t *t)
{
	if (!rtc_running()) return false;
	RtcGetOldDateTime(t);
	return True;
}

// Set a time in the future for the RTC to call a user provided callback
void rtc_set_alarm(const datetime_t *t, rtc_callback_t user_callback);

// Enable the RTC alarm (if inactive)
INLINE void rtc_enable_alarm(void) { RtcAlarmEnable(); }

// Disable the RTC alarm (if active)
INLINE void rtc_disable_alarm(void) { RtcAlarmDisable(); }

// Convert a datetime_t structure to a string
void datetime_to_str(char *buf, uint buf_size, const datetime_t *t);

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_RTC_H

#endif // USE_RTC	// use RTC Real-time clock (sdk_rtc.c, sdk_rtc.h)

#endif // RP2040
