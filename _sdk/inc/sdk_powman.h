
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

// Power supply pins
// -----------------
// IOVDD ... IO supply for GPIO, should be 1.8V to 3.3V. Must not exceed 1.8V in the case of a 1.8V threshold.
// QSPI_IOVDD ... IO supply for QSPI, should be 1.8V to 3.3V. Must not exceed 1.8V in the case of a 1.8V threshold.
// DVDD ... core digital logic supply, should be 1.1V. Can be supplied from external or internal voltage regulator.
//		Add two 100nF to DVDD pins closest to regulator, add 4.7uF to DVDD pin further from the regulator.
// USB_OTP_VDD ... supply for USB PHY and OTP memory, should be 3.3V. Add 100nF to the pin.
// ADC_AVDD ... supply for ADC, should be 1.8V to 3.3V, with lower performance at voltages below 2.97V.
//		Input voltage must not exceed IOVDD. Add 100nF to the pin.
// VREG_VIN ... input supply for on-chip voltage regulator, should be 2.7V to 5.5V. Add 4.7uF to the pin.
// VREG_AVDD ... supply for on.chip voltage regulator's analogue control circuits, should be powered at 3.3V.

// Power domains
// -------------
// AON ... always on (small amount of logic)
// SWCORE ... switched core (processors, buf fabric, peripherals)
// XIP ... XIP cache SRAM 2x 16KB and Boot RAM 1 KB (must always be on if SWCORE is on)
// SRAM0 ... SRAM power domain 0, lower half of large SRAM banks, 4x 64 KB banks 0..3
// SRAM1 ... SRAM power domain 1, upper half of large SRAM banks, 4x 64 KB banks 4..7, and scratch SRAMs 2x 4 KB banks 8..9

#if RP2350

#ifndef _SDK_POWMAN_H
#define _SDK_POWMAN_H

#include "../sdk_addressmap.h"		// Register address offsets

#if USE_ORIGSDK		// include interface of original SDK
#include "orig_rp2350/orig_powman.h"	// constants of original SDK
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// #define POWMAN_BASE			0x40100000
typedef struct {
// start of password protected area (need to set upper bits to POWMAN_PASSWORD to enable write)
	io32	badpasswd;		// 0x00: 1=bad password has been used
	io32	vreg_ctrl;		// 0x04: voltage regulator control
	io32	vreg_sts;		// 0x08: R/O voltage regulator status
	io32	vreg;			// 0x0C: voltage regulator setup
	io32	vreg_lp_entry;		// 0x10: voltage regulator low power entry settings
	io32	vreg_lp_exit;		// 0x14: voltage regulator low power exit settings
	io32	bod_ctrl;		// 0x18: brown-out detection control
	io32	bod;			// 0x1C: brown-out detection settings
	io32	bod_lp_entry;		// 0x20: brown-out detection low power entry settings
	io32	bod_lp_exit;		// 0x24: brown-out detection low power exit settings
	io32	lposc;			// 0x28: low power oscillator control register
	io32	chip_reset;		// 0x2C: chip reset control and status
	io32	wdsel;			// 0x30: allow watchdog reset power manager
	io32	seq_cfg;		// 0x34: configuration of power sequencer
	io32	state;			// 0x38: power state of 4 power domains
	io32	pow_fastdiv;		// 0x3C: divider of power manager clock
	io32	pow_delay;		// 0x40: power state machine delays
	io32	ext_ctrl[2];		// 0x44: GPIO power mode
	io32	ext_time_ref;		// 0x4C: GPIO time reference
	io32	lposc_freq_khz_int;	// 0x50: off clock frequency LPOSC in kHz, 16-bit integer part
	io32	lposc_freq_khz_frac;	// 0x54: off clock frequency LPOSC in kHz, 16-bit fractional part
	io32	xosc_freq_khz_int;	// 0x58: off clock frequency XOSC in kHz, 16-bit integer part
	io32	xosc_freq_khz_frac;	// 0x5C: off clock frequency XOSC in kHz, 16-bit fractional part
	io32	set_time_63to48;	// 0x60: setting time bits 48..63 (write only)
	io32	set_time_47to32;	// 0x64: setting time bits 32..47 (write only)
	io32	set_time_31to16;	// 0x68: setting time bits 16..31 (write only)
	io32	set_time_15to0;		// 0x6C: setting time bits 0..15 (write only)
	io32	read_time_upper;	// 0x70: reading time bits 32..63 (read only)
	io32	read_time_lower;	// 0x74: reading time bits 0..31 (read only)
	io32	alarm_time_63to48;	// 0x78: setting alarm bits 48..63
	io32	alarm_time_47to32;	// 0x7C: setting alarm bits 32..47
	io32	alarm_time_31to16;	// 0x80: setting alarm bits 16..31
	io32	alarm_time_15to0;	// 0x84: setting alarm bits 0..15
	io32	timer;			// 0x88: timer
	io32	pwrup[4];		// 0x8C: GPIO powerup events
	io32	current_pwrup_req;	// 0x9C: powerup request state
	io32	last_swcore_pwrup;	// 0xA0: powerup source
	io32	dbg_pwrcfg;		// 0xA4: power debugger
	io32	bootdis;		// 0xA8: bootrom ignore BOOT0
	io32	dbgconfig;		// 0xAC: debugger config
// end of password protected area
	io32	scratch[8];		// 0xB0: scratch register, information persists in low power mode
	io32	boot[4];		// 0xD0:
	io32	intr;			// 0xE0: raw interrupts
	io32	inte;			// 0xE4: enable interrupts
	io32	intf;			// 0xE8: force interrupts
	io32	ints;			// 0xEC: interupt state
} powman_hw_t;

#define powman_hw ((powman_hw_t*)POWMAN_BASE)

STATIC_ASSERT(sizeof(powman_hw_t) == 0xF0, "Incorrect powman_hw_t!");

// Power manager password (need to write to upper 16 bits of registers 0x00 to 0xAC)
#define POWMAN_PASSWORD		0x5afe0000	// = "SAFE" word

// write 16-bit value to power manager register with password
INLINE void PowMan_Write(volatile u32* reg, u32 val) { *reg = POWMAN_PASSWORD | val; }

// set bits of power manager register with password
INLINE void PowMan_SetBits(volatile u32* reg, u32 val) { RegSet(reg, POWMAN_PASSWORD | val); }

// clear bits of power manager register with password
INLINE void PowMan_ClrBits(volatile u32* reg, u32 val) { RegClr(reg, POWMAN_PASSWORD | val); }

// start timer of power manager (start AON always-on timer)
INLINE void PowMan_Start(void) { PowMan_SetBits(&powman_hw->timer, B1); }

// stop timer of power manager (stop AON always-on timer)
INLINE void PowMan_Stop(void) { PowMan_ClrBits(&powman_hw->timer, B1); }

// check if timer of power manager is running
INLINE Bool PowMan_IsRunning(void) { return (powman_hw->timer & B1) != 0; }

// set time in [ms] of power manager (set time ot AON always-on timer)
void PowMan_SetTime(u64 time);

// fast get time LOW of power manager in [ms] (get time ot AON always-on timer)
INLINE u32 PowMan_GetTimeLow(void) { return powman_hw->read_time_lower; }

// get time of power manager in [ms] (get time ot AON always-on timer)
u64 PowMan_GetTime(void);

// Setup power manager timer source
#define POWMAN_SRC_LPOSC	0	// low power oscillator (usually 32 kHz)
#define POWMAN_SRC_XOSC		1	// crystal oscillator
#define POWMAN_SRC_GPIO12	12	// GPIO12 input (only 1 kHz or 1 Hz)
#define POWMAN_SRC_GPIO14	14	// GPIO14 input (only 1 kHz or 1 Hz)
#define POWMAN_SRC_GPIO20	20	// GPIO20 input (only 1 kHz or 1 Hz)
#define POWMAN_SRC_GPIO22	22	// GPIO22 input (only 1 kHz or 1 Hz)
//  src ... clock source POWMAN_SRC_*
//  freq ... clock frequency at Hz (must be 1000 or 1 for GPIO input)
//	GPIO with frequency 1 Hz will synchronize low power or crystal oscillator
void PowMan_SetSrc(int src, u32 freq);

// get power state of domains (combination of POWMAN_DOMAIN_*, 1=powered up, 0=powered down)
#define POWMAN_DOMAIN_SRAM1	B0	// SRAM power domain 1, upper half of large SRAM banks, 4x 64 KB banks 4..7, and scratch SRAMs 2x 4 KB banks 8..9
#define POWMAN_DOMAIN_SRAM0	B1	// SRAM power domain 0, lower half of large SRAM banks, 4x 64 KB banks 0..3
#define POWMAN_DOMAIN_XIP	B2	// XIP cache SRAM 2x 16KB and Boot RAM 1 KB (must always be on if SWCORE is on)
#define POWMAN_DOMAIN_SWCORE	B3	// switched core (processors, buf fabric, peripherals)
INLINE u8 PowMan_PowerState(void) { return powman_hw->state & 0x0f; }

// set power state of domains (returns False if new state is declined)
//  state ... combination of POWMAN_DOMAIN_*, 1=powered up, 0=powered down
Bool PowMain_SetPowerState(int state);

// clear alarm (alarm must be disabled, or will be fired again)
INLINE void PowMan_ClearAlarm(void) { PowMan_ClrBits(&powman_hw->timer, B6); } // ALARM

// set alarm on at time in [ms]
void PowMan_AlarmOn(u64 time);

// set alarm off
void PowMan_AlarmOff(void);

// setup alarm to wake up at time in [ms]
void PowMan_WakeUpOn(u64 time);

// disable wake up
void PowMan_WakeUpOff(void);

// setup wake up by GPIO
//  event ... wakeup event 0..3
//  gpio ... gpio to wake up from (0..47)
//  edge ... True=edge sensitive, False=level sensitive
//  high ... True=active high or rising, False=active low or falling
void PowMan_GpioOn(int event, int gpio, Bool edge, Bool high);

// disable wake up by GPIO
//  event ... wakeup event 0..3
INLINE void PowMan_GpioOff(int event) { PowMan_ClrBits(&powman_hw->pwrup[event], B6); } // ENABLE

// disable all wake ups
void PowMan_WakeAllOff(void);

// Possible voltage values
#define VREG_VOLTAGE_0_55	0	// 0.55V
#define VREG_VOLTAGE_0_60	1	// 0.60V
#define VREG_VOLTAGE_0_65	2	// 0.65V
#define VREG_VOLTAGE_0_70	3	// 0.70V
#define VREG_VOLTAGE_0_75	4	// 0.75V
#define VREG_VOLTAGE_0_80	5	// 0.80V
#define VREG_VOLTAGE_0_85	6	// 0.85V
#define VREG_VOLTAGE_0_90	7	// 0.90V
#define VREG_VOLTAGE_0_95	8	// 0.95V
#define VREG_VOLTAGE_1_00	9	// 1.00V
#define VREG_VOLTAGE_1_05	10	// 1.05V
#define VREG_VOLTAGE_1_10	11	// 1.10V *default state
#define VREG_VOLTAGE_1_15	12	// 1.15V
#define VREG_VOLTAGE_1_20	13	// 1.20V
#define VREG_VOLTAGE_1_25	14	// 1.25V
#define VREG_VOLTAGE_1_30	15	// 1.30V
// - higher values are protected, need to set vreg_ctrl
#define VREG_VOLTAGE_1_35	16	// 1.35V
#define VREG_VOLTAGE_1_40	17	// 1.40V
#define VREG_VOLTAGE_1_50	18	// 1.50V
#define VREG_VOLTAGE_1_60	19	// 1.60V
#define VREG_VOLTAGE_1_65	20	// 1.65V
#define VREG_VOLTAGE_1_70	21	// 1.70V
#define VREG_VOLTAGE_1_80	22	// 1.80V
#define VREG_VOLTAGE_1_90	23	// 1.90V
#define VREG_VOLTAGE_2_00	24	// 2.00V
#define VREG_VOLTAGE_2_35	25	// 2.35V
#define VREG_VOLTAGE_2_50	26	// 2.50V
#define VREG_VOLTAGE_2_65	27	// 2.65V
#define VREG_VOLTAGE_2_80	28	// 2.80V
#define VREG_VOLTAGE_3_00	29	// 3.00V
#define VREG_VOLTAGE_3_15	30	// 3.15V
#define VREG_VOLTAGE_3_30	31	// 3.30V

#define VREG_VOLTAGE_MIN	VREG_VOLTAGE_0_85	// minimum voltage
#define VREG_VOLTAGE_DEF	VREG_VOLTAGE_1_10	// default voltage after power up
#define VREG_VOLTAGE_MAX	VREG_VOLTAGE_1_30	// maximum voltage

#define VREG_VOLTAGE_UNLOCK	B8	// OR required value with this flag, to enable unlock higher voltages than 1.30V

// set voltage VREG_VOLTAGE_*
// - OR required value with VREG_VOLTAGE_UNLOCK flag, to enable unlock higher voltages than 1.30V
void VregSetVoltage(int vreg);

// get voltage VREG_VOLTAGE_*
INLINE u8 VregVoltage(void) { return (u8)((powman_hw->vreg >> 4) & 0x1f); }

// get voltage in volts
float VregVoltageFloat(void);

// check if voltage is correctly regulated
INLINE Bool VregIsOk(void) { return (powman_hw->vreg & B15) == 0; } // UPDATE_IN_PROGRESS

// wait for regulated state
void VregWait(void);

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

#define VREG_VOLTAGE_DEFAULT VREG_VOLTAGE_DEF

// Set voltage
INLINE void vreg_set_voltage(int voltage) { VregSetVoltage(voltage); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_POWMAN_H

#endif // RP2350
