
// ****************************************************************************
//
//                              Watchdog Timer
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
#include "../inc/sdk_watchdog.h"

#if RP2040
#if USE_WATCHDOG || USE_TIMER // use Watchdog or Timer
// Start watchdog tick generator (generated ticks are shared with main timer counter!)
//  ref_mhz ... frequency of CLK_REF in MHz
void WatchdogStart(u32 ref_mhz)
{
	watchdog_hw->tick = ref_mhz | B9; // setup ticks and run tick generator
}
#endif // USE_WATCHDOG || USE_TIMER // use Watchdog or Timer
#endif // RP2040

#if USE_WATCHDOG	// use Watchdog timer (sdk_watchdog.c, sdk_watchdog.h)

// watchdog update value (to restart time counting) = RP2040: time in [us] * 2, RP2350: time in [us]
u32 WatchdogUpdateValue;

// Setup watchdog generator (does not set reason into scratch registers)
//  us = delay before reset in [us], 0=trigger now, max. RP2040: 0x7fffff us = 8.4 sec, RP2350: 0xffffff us = 16.8 sec
//  pause_debug = pause watchdog on debugging
void WatchdogSetup(u32 us, Bool pause_debug)
{
	// pause watchdog
	WatchdogDisable();

	// select which peripherals to reset (reset everything apart from ROSC and XOSC)
	RegSet(&psm_hw->wdsel, PSM_WDSEL_RESETBITS);

	// pause on debug
	if (pause_debug)
		RegSet(&watchdog_hw->ctrl, B24|B25|B26);
	else
		RegClr(&watchdog_hw->ctrl, B24|B25|B26);

	// trigger now
	if (us == 0)
	{
		WatchdogTrigger();
	}
	else
	{
#if RP2040
		us *= 2;
#endif

		// limit max. value
		if (us > WATCHDOG_CTRL_TIMEMASK) us = WATCHDOG_CTRL_TIMEMASK;

		// update time counter
		WatchdogUpdateValue = us;
		WatchdogUpdate();

		// run watchdog
		RegSet(&watchdog_hw->ctrl, B30);
	}
}

// watchdog enable (set reason WATCHDOG_NON_REBOOT_MAGIC)
//  us = delay before reset in [us], 0=trigger now, max. RP2040: 0x7fffff us = 8.4 sec, RP2350: 0xffffff us = 16.8 sec
//  pause_debug = pause watchdog on debugging
void WatchdogEnable(u32 us, Bool pause_debug)
{
	// set reason
	watchdog_hw->scratch[4] = WATCHDOG_NON_REBOOT_MAGIC;

	// enable watchdog generator
	WatchdogSetup(us, pause_debug);
}

// Check reset reason WatchdogEnable()
Bool WatchdogReasonEnable(void)
{
	return (WatchdogReason() != RESET_REASON_HW) &&
		(watchdog_hw->scratch[4] == WATCHDOG_NON_REBOOT_MAGIC);
}

// Setup watchdog generator to do hard reset (set reason 0)
//  us = delay before reset in [us], 0=trigger now, max. RP2040: 0x7fffff us = 8.4 sec, RP2350: 0xffffff us = 16.8 sec
//  pause_debug = pause watchdog on debugging
void WatchdogSetupReset(u32 us, Bool pause_debug)
{
	// clear scratch register 4 with magic word to jump into code
	watchdog_hw->scratch[4] = 0;

	// enable watchdog generator
	WatchdogSetup(us, pause_debug);
}

// Setup watchdog genererator to do soft reset
//  us = delay before reset in [us], 0=trigger now, max. RP2040: 0x7fffff us = 8.4 sec, RP2350: 0xffffff us = 16.8 sec
//  pc = pointer to code to restart in soft reset (0 = do hard reset)
//  sp = stack pointer
void WatchdogSetupReboot(u32 us, u32 pc, u32 sp)
{
	// pause watchdog before setting up scratch registers
	RegClr(&watchdog_hw->ctrl, B30);

	// setup scratch registers
	if (pc != 0)
	{
		watchdog_hw->scratch[4] = WATCHDOG_REBOOT_MAGIC; // set magic
		watchdog_hw->scratch[5] = pc ^ -WATCHDOG_REBOOT_MAGIC; // set PC with magic
		watchdog_hw->scratch[6] = sp; // set SP
		watchdog_hw->scratch[7] = pc; // set PS
	}
	else
		// do hard reset; clear scratch register 4 with magic word to jump into code
		watchdog_hw->scratch[4] = 0;

	// enable watchdog generator (no pause on debugging)
	WatchdogSetup(us, False);
}

#if USE_PICOPAD || USE_PICOINO || USE_PICOTRON || USE_DEMOVGA || USE_PICOPADHSTX
// wait for no key pressed
void KeyWaitNoPressed();
#endif

// Device terminate
void DeviceTerm();

// reset to boot loader
void ResetToBootLoader()
{
#if USE_PICOPAD || USE_PICOINO || USE_PICOTRON || USE_DEMOVGA || USE_PICOPADHSTX
	// wait for no key pressed
	KeyWaitNoPressed();
#endif

	// device terminate
	DeviceTerm();

	// set loader magic
	watchdog_hw->scratch[4] = WATCHDOG_LOADER_MAGIC;

	// enable watchdog generator, trigger now
	WatchdogSetup(0, False);
}

#endif // USE_WATCHDOG	// use Watchdog timer (sdk_watchdog.c, sdk_watchdog.h)
