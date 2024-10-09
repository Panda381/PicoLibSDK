
// ****************************************************************************
//
//                                GPIO pins
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
#include "../inc/sdk_gpio.h"
#include "../inc/sdk_cpu.h"

// generic callback for GPIO types (NULL = not used)
gpio_irq_callback_t GPIO_Callbacks[CORE_NUM] = { NULL, NULL };

// set GPIO function GPIO_FNC_* (pin = 0..29 or 0..47)
// Resets pad overrides, enables input and output.
// To setup GPIO:
//  - reset GPIO to default state with GPIO_Reset()
//  - setup output level, output enable, pull-up/pull-down, schmitt, slew, drive strength
//  - set GPIO function
//  - if needed, disable input (needed for ADC) and setup overrides
void GPIO_Fnc(int pin, int fnc)
{
	// enable input and output
	io32* pad = GPIO_PadHw(pin);	// get pad control interface
	GPIO_OutEnable_hw(pad);		// output enable
	GPIO_InEnable_hw(pad);		// input enable
	*GPIO_CTRL(pin) = fnc;		// set function, reset all overrides
#if !RP2040
	GPIO_IsolationDisable_hw(pad);	// pad isolation disable
#endif
}

// set GPIO function GPIO_FNC_* with mask (bit '1' to set function of this pin)
// To use pin mask in range (first..last), use function RangeMask.
void GPIO_FncMask(gpio_mask_t mask, int fnc)
{
	int i;
	for (i = 0; i < GPIO_PIN_NUM; i++)
	{
		if ((mask & 1) != 0) GPIO_Fnc(i, fnc);
		mask >>= 1;
	}
}

// acknowledge IRQ interrupt for both CPU
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_EDGE* of events to acknowledge
//  Interrupts LEVEL are not latched, they become inactive on end condition.
void GPIO_IRQAck(int pin, int events)
{
	volatile u32* reg = &GPIO_IRQ_INTR[pin >> 3];
	u32 mask = (u32)events << (4*(pin & 7));
	*reg = mask;
}

// enable IRQ interrupt for selected/current CPU
//   cpu = CPU core 0 or 1, use CpuID() to get current core
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to enable

// After enabling interrupt from pin, following steps are necessary:
//   - set handler of IRQ_IO_BANK0 (use function SetHandler), or use isr_io_bank0 handler
//   - not necessary, but rather clear pending interrupt of IRQ_IO_BANK0 (use function NVIC_IRQClear)
//   - enable NVIC interrupt of IRQ_IO_BANK0 (use function NVIC_IRQEnable)
//   - enable global interrupt ei()

void GPIO_IRQEnableCpu(int cpu, int pin, int events)
{
	volatile u32* reg = &GPIO_IRQ_INTE(cpu)[pin >> 3];
	u32 mask = (u32)events << (4*(pin & 7));
	RegSet(reg, mask);
}

// enable IRQ interrupt for dormant wake
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to enable
void GPIO_IRQEnableDorm(int pin, int events)
{
	volatile u32* reg = &GPIO_IRQ_INTE(2)[pin >> 3];
	u32 mask = (u32)events << (4*(pin & 7));
	RegSet(reg, mask);
}

// disable IRQ interrupt for selected/current CPU
//   cpu = CPU core 0 or 1, use CpuID() to get current core
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to disable
void GPIO_IRQDisableCpu(int cpu, int pin, int events)
{
	volatile u32* reg = &GPIO_IRQ_INTE(cpu)[pin >> 3];
	u32 mask = (u32)events << (4*(pin & 7));
	RegClr(reg, mask);
}

// disable IRQ interrupt for dormant wake
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to disable
void GPIO_IRQDisableDorm(int pin, int events)
{
	volatile u32* reg = &GPIO_IRQ_INTE(2)[pin >> 3];
	u32 mask = (u32)events << (4*(pin & 7));
	RegClr(reg, mask);
}

// force IRQ interrupt for selected/current CPU
//   cpu = CPU core 0 or 1, use CpuID() to get current core
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to force
void GPIO_IRQForceCpu(int cpu, int pin, int events)
{
	volatile u32* reg = &GPIO_IRQ_INTF(cpu)[pin >> 3];
	u32 mask = (u32)events << (4*(pin & 7));
	RegSet(reg, mask);
}

// force IRQ interrupt for dormant wake
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to force
void GPIO_IRQForceDorm(int pin, int events)
{
	volatile u32* reg = &GPIO_IRQ_INTF(2)[pin >> 3];
	u32 mask = (u32)events << (4*(pin & 7));
	RegSet(reg, mask);
}

// clear force IRQ interrupt for selected/current CPU
//   cpu = CPU core 0 or 1, use CpuID() to get current core
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to unforce
void GPIO_IRQUnforceCpu(int cpu, int pin, int events)
{
	volatile u32* reg = &GPIO_IRQ_INTF(cpu)[pin >> 3];
	u32 mask = (u32)events << (4*(pin & 7));
	RegClr(reg, mask);
}

// clear force IRQ interrupt for dormant wake
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to unforce
void GPIO_IRQUnforceDorm(int pin, int events)
{
	volatile u32* reg = &GPIO_IRQ_INTF(2)[pin >> 3];
	u32 mask = (u32)events << (4*(pin & 7));
	RegClr(reg, mask);
}

// check IRQ interrupt status for selected/current CPU (returns 1 if IRQ is pending)
//   core = CPU core 0 or 1, use CpuID() to get current core
//   pin = 0..29 or 0..47
//   returns events = bit mask with IRQ_EVENT_* of incoming events
u8 GPIO_IRQIsPendingCpu(int cpu, int pin)
{
	volatile u32* reg = &GPIO_IRQ_INTS(cpu)[pin >> 3];
	return (u8)((*reg >> (4*(pin & 7))) & IRQ_EVENT_ALL);
}

// check IRQ interrupt status for dormant wake (returns 1 if IRQ is pending)
//   pin = 0..29 or 0..47
//   returns events = bit mask with IRQ_EVENT_* of incoming events
u8 GPIO_IRQIsPendingDorm(int pin)
{
	volatile u32* reg = &GPIO_IRQ_INTS(2)[pin >> 3];
	return (u8)((*reg >> (4*(pin & 7))) & IRQ_EVENT_ALL);
}

// check if IRQ is forced for selected/current CPU (returns 1 if IRQ is pending)
//   cpu = CPU core 0 or 1, use CpuID() to get current core, or 2 = dormant wake
//   pin = 0..29 or 0..47
//   returns events = bit mask with IRQ_EVENT_* of forced events
u8 GPIO_IRQIsForcedCpu(int cpu, int pin)
{
	volatile u32* reg = &GPIO_IRQ_INTF(cpu)[pin >> 3];
	return (u8)((*reg >> (4*(pin & 7))) & IRQ_EVENT_ALL);
}

// check if IRQ is forced for dormant wake (returns 1 if IRQ is pending)
//   pin = 0..29 or 0..47
//   returns events = bit mask with IRQ_EVENT_* of forced events
u8 GPIO_IRQIsForcedDorm(int pin)
{
	volatile u32* reg = &GPIO_IRQ_INTF(2)[pin >> 3];
	return (u8)((*reg >> (4*(pin & 7))) & IRQ_EVENT_ALL);
}

// GPIO default IRQ handler
void GPIO_DefIRQHandler(void)
{
	// get current core
	int core = CpuID();

	// get callback handler
	gpio_irq_callback_t cb = GPIO_Callbacks[core];

	// check pins
	int pin, events;
	for (pin = 0; pin < GPIO_PIN_NUM; pin++)
	{
		// check IRQ events
		events = GPIO_IRQIsPendingCpu(core, pin);
		if (events != 0)
		{
			// acknowledge events
			GPIO_IRQAck(pin, events);

			// callback
			if (cb != NULL) cb(pin, events);
		}
	}
}

// set generic callback for current core (NULL=disable callback)
// - it will install default IRQ handler (as used with GPIO_SetHandler)
// To use:
// - set callback using the GPIO_IRQSetCallback() function
// - enable interrupts on GPIO pins using the GPIO_IRQEnable() function
// - enable NVIC interrupt using the NVIC_IRQEnable(IRQ_IO_BANK0) command
void GPIO_IRQSetCallback(gpio_irq_callback_t cb)
{
	// get current core
	int core = CpuID();

	// set new callback
	GPIO_Callbacks[core] = cb;

	// install default handler
	if (cb != NULL) GPIO_SetHandler(GPIO_DefIRQHandler);
}

// initialize GPIO pin base function, reset pin, set input mode, LOW output value (pin = 0..29 or 0..47)
void GPIO_Init(int pin)
{
	GPIO_Reset(pin);	// reset pin
	GPIO_DirIn(pin);	// disable output
	GPIO_Out0(pin);		// set output to LOW
	GPIO_Fnc(pin, GPIO_FNC_SIO); // set function, reset overrides, reset pad setup
}

// initialize GPIO pin base function masked (bit '1' to initialize this pin)
// To use pin mask in range (first..last), use function RangeMask.
void GPIO_InitMask(gpio_mask_t mask)
{
	GPIO_ResetMask(mask);		// reset pins
	GPIO_DirInMask(mask);		// disable output
	GPIO_ClrMask(mask);		// set output to LOW
	GPIO_FncMask(mask, GPIO_FNC_SIO); // set function, reset overrides, reset pad setup
}

// reset GPIO pin (return to reset state) (pin = 0..29 or 0..47,56..63)
void GPIO_Reset(int pin)
{
	GPIO_IRQDisableCpu(0, pin, IRQ_EVENT_ALL); // disable IRQ
	GPIO_IRQDisableCpu(1, pin, IRQ_EVENT_ALL);
	GPIO_IRQUnforceCpu(0, pin, IRQ_EVENT_ALL); // clear force IRQ
	GPIO_IRQUnforceCpu(1, pin, IRQ_EVENT_ALL);
	GPIO_IRQAck(pin, IRQ_EVENT_ALL); // acknowledge IRQ
	GPIO_DirIn(pin);		// disable output
	GPIO_Out0(pin);			// set output to 0
	GPIO_PadInit(pin);		// reset pad setup
	*GPIO_CTRL(pin) = GPIO_FNC_NULL; // reset function, reset overrides
}

// reset GPIO pins masked (return to reset state)
// To use pin mask in range (first..last), use function RangeMask.
void GPIO_ResetMask(gpio_mask_t mask)
{
	int i;
	for (i = 0; i < GPIO_PIN_NUM; i++)
	{
		if ((mask & 1) != 0) GPIO_Reset(i);
		mask >>= 1;
	}
}
