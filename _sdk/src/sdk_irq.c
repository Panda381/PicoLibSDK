
// ****************************************************************************
//
//                                    IRQ
//                             Interrupt Request
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

#if USE_IRQ	// use IRQ interrupts (sdk_irq.c, sdk_irq.h)

#include "../inc/sdk_irq.h"
#include "../inc/sdk_cpu.h"

#if !RISCV
// set interrupt priority of NVIC of this CPU core (prio = priority IRQ_PRIO_*) (irq = 0..25)
void NVIC_IRQPrio(int irq, u8 prio)
{
	volatile u32* reg = &nvic_hw->ipr[irq >> 2]; // register (every register contains 4 entries)
	irq = 8 * (irq & 3) + 6; // bit in the register
	u32 val = (u32)prio << irq; // prepare priority
	u32 mask = 0xffUL << irq; // prepare mask
	*reg = (*reg & ~mask) | val;
}

// set all interrupt priorities of NVIC of this CPU core to default value
void NVIC_IRQPrioDef(void)
{
	// set RP2040 device interrupts
	int i;
	for (i = 0; i < IRQ_NUM; i++) NVIC_IRQPrio((u8)i, IRQ_PRIO_NORMAL);

	// set exception priorities
	NVIC_SVCallPrio(IRQ_PRIO_NORMAL); // SVCall
	NVIC_PendSVPrio(IRQ_PRIO_PENDSV); // PendSV
	NVIC_SysTickPrio(IRQ_PRIO_SYSTICK); // SysTick
}
#endif

/*
// system reset (send reset signal)
void SystemReset()
{
	// data synchronization barrier
	dsb();

	// send reset signal (0x05FAUL is register key)
	*SCB_AIRCR = (0x05FAUL << 16) | B2;

	// data synchronization barrier
	dsb();

	// waiting for reset in infinite loop
	for (;;) nop();
}
*/

#endif // USE_IRQ	// use IRQ interrupts (sdk_irq.c, sdk_irq.h)
