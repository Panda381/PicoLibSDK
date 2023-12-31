
// ****************************************************************************
//
//                               SysTick alarm
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

// Alarm can be triggered with SYSTICK_MS resolution (max. range 49 days)
// Every CPU core has its own alarm list.

#if USE_ALARM			// use SysTick alarm (lib_alarm.c, lib_alarm.h)

#ifndef _LIB_ALARM_H
#define _LIB_ALARM_H

#include "../../_sdk/inc/sdk_cpu.h"
#include "lib_list.h"

#ifdef __cplusplus
extern "C" {
#endif

// list of system alarms (each processor core has its own list)
extern sList AlarmList[CORE_NUM];

struct sAlarm_;

// alarm callback (parametrs: pointer to alarm structure)
// - callback function can unregister alarm, if it is no longer needed)
typedef void (*pAlarmCB)(struct sAlarm_*);

// system alarm
typedef struct sAlarm_ {
	
	// list of alarms
	sListEntry	list;

	// system time in ms to alarm
	u32		time;

	// delta time to repeat in ms (if repetition is not needed,
	//    the alarm must be unregistered from the callback)
	u32		delta;

	// callback function (called from SysTick_Handler with IRQ locked - it must be fast executed)
	pAlarmCB	callback;

	// user data
	u32		cookie;

} sAlarm;

// register alarm (caller must unregister alarm after waking up if it is no longer needed)
// - Callback will be called under the same core from which the alarm was registered.
// - Registered alarm structure must remain valid until the alarm is unregistered.
void AlarmReg(sAlarm* alarm);

// unregister alarm (need to be done after waking up if alarm is no longer needed)
// - Must be called from the same CPU core as it was registered.
void AlarmUnreg(sAlarm* alarm);

// update alarms for this CPU core (caled from SysTick_Handler every SYSTICK_MS ms with IRQ locked)
void AlarmUpdate();

// Initialize alarm lists for both cores
void AlarmInit();

#ifdef __cplusplus
}
#endif

#endif // _LIB_ALARM_H

#endif // USE_ALARM			// use SysTick alarm (lib_alarm.c, lib_alarm.h)
