
// ****************************************************************************
//
//                                Multicore
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

#include "../inc/sdk_multicore.h"

// Core 1 stack (located in reserved section stack1)
u32 __attribute__((section(".stack1"))) Core1Stack[CORE1_STACK_SIZE/4];

#if USE_MULTICORE	// use Multicore (sdk_multicore.c, sdk_multicore.h)

#include "../inc/sdk_cpu.h"
#include "../inc/sdk_fifo.h"
#include "../inc/sdk_irq.h"
#include "../inc/sdk_bootrom.h"
#include "../inc/sdk_reset.h"
#include "../inc/sdk_timer.h"

// flag that core 1 is running
volatile Bool Core1IsRunning = False;

// Core1 trampoline to jump to Core1Wrapper
static void __attribute__ ((naked)) Core1Trampoline()
{
#if RISCV
	//  a0 ... input parameter 1, pointer to user function
	//  a1 ... jump to Core1Wrapper
	//  gp ... initialize global pointer
	__asm volatile (
		" lw a0, 0(sp)\n"
		" lw a1, 4(sp)\n"
		" lw gp, 8(sp)\n"
		" addi sp, sp, 12\n"
		" jr a1\n"
	);
#else
	//  r0 ... input parameter 1, pointer to user function
	//  pc ... jump to Core1Wrapper
	__asm volatile (" pop {r0, pc}\n");
#endif
}

#if RISCV
// initialize interrupts on RISC-V
void RuntimeInitCoreIRQ();
#endif

// Core1 wrapper to call user function
void Core1Wrapper(pCore1Fnc entry)
{
	// set all interrupt priorities of NVIC of this CPU core to default value
	NVIC_IRQPrioDef();

	// set flag that core 1 is running
	Core1IsRunning = True;
	dmb(); // data memory barrier

#if RISCV
	// initialize interrupts on RISC-V
	RuntimeInitCoreIRQ();
#endif

#if RP2350 && !RISCV
	// enable GPIO coprocessor
	GPIOC_Enable();

	// enable single-precision coprocessor
	FPU_Enable();

	// enable double-precision coprocessor
	DCP_Enable();
#endif

	// call user function
	entry();

	// disable interrupts
	di();

	// core 1 not running
	Core1IsRunning = False;
	dmb(); // data memory barrier

	// return to bootrom
}

// reset CPU core 1 (core 1 will send '0' to the FIFO when done)
void Core1Reset()
{
	// start resetting core 1
	RegSet(PSM_OFF, BIT(POWER_PROC1));

	// wait for core 1 to be in correct reset state
	while ((*PSM_OFF & BIT(POWER_PROC1)) == 0) {}

	// temporary disable interrupt from SIO FIFO during handshake
	Bool en = NVIC_IRQIsEnabled(SIO_FIFO_IRQ_NUM(0));
	NVIC_IRQDisable(SIO_FIFO_IRQ_NUM(0));

	// stop resetting core 1
	RegClr(PSM_OFF, BIT(POWER_PROC1));

	// wait some time for core 1 initialized (or it may lock in Core1Exec function)
	WaitUs(20);

	// flush mailbox
	FifoReadFlush();

	// core 1 not running
	Core1IsRunning = False;
	dmb(); // data memory barrier

	// enable interrupt IRQ_SIO_PROC0
	if (en) NVIC_IRQEnable(SIO_FIFO_IRQ_NUM(0));
}

// start core 1 function (must be called from core 0)
//  entry ... entry into Core1 function (function can exit to stop core 1)
// Core1IsRunning ... flag that core 1 is running
// Both cores use the same VTOR interrupt table (handlers are shared) but have separate NVIC interrupt controllers.
void Core1Exec(pCore1Fnc entry)
{
	// core 1 reset
	Core1Reset();

	// temporary disable interrupt from SIO FIFO
	Bool en = NVIC_IRQIsEnabled(SIO_FIFO_IRQ_NUM(0));
	NVIC_IRQDisable(SIO_FIFO_IRQ_NUM(0));

	// save and lock interrupts
	IRQ_LOCK;

	// prepare stack pointer to reset core 1
	u32* sp = &Core1Stack[CORE1_STACK_SIZE/4];
#if RISCV
	// prepare to initialize global pointer
	sp -= 3;
	__asm volatile (" mv %0, gp" : "=r" (sp[2]));
#else
	sp -= 2;
#endif
	sp[0] = (u32)entry;	// parameter 1 - address of user function
	sp[1] = (u32)Core1Wrapper; // jump to Core1Wrapper

	// prepare data frame to launch core 1 from bootrom_rt0.S
	u32 frame[] = {
		0,		// synchronization
		0,		// synchronization
		1,		// command to start data sequence
#if RISCV
		(u32)GetVTOR()|1, // address of vector table (shared with core 0) ... set vector mode
#else
		(u32)GetVTOR(),	// address of vector table (shared with core 0)
#endif
		(u32)sp,	// stack pointer
		(u32)Core1Trampoline};	// execute trampoline function

	// send data frame
	int i = 0;
	do {
		// flush read FIFO on synchronization
		if (i < 2)
		{
			// flush all read messages
			FifoReadFlush();

			// send event - core 1 may be waiting for FIFO via wfe()
			sev();
		}

		// get next word to send
		u32 c = frame[i++];

		// send data word
		FifoWrite(c);

		// receive response - it will be 0 if core 1 is just starting handshake
		u32 res = FifoRead();

		// reset on incorrect response (must be echoed value)
		if (res != c) i = 0;

	} while (i < count_of(frame));

	// unlock interrupts
	IRQ_UNLOCK;

	// enable interrupt IRQ_SIO_PROC0
	if (en) NVIC_IRQEnable(SIO_FIFO_IRQ_NUM(0));
}

// Lockout handler (must be in RAM and must not use flash functions - to enable flash writting from other core)
//  Name of alternative handler in Flash: isr_sio_proc0 and isr_sio_proc1
void NOFLASH(LockoutHandler)()
{
	// save and lock interrupts
	IRQ_LOCK;

	// clear FIFO errors
	*FIFO_STATE = 0xff;

	// while inter-core RX FIFO is ready to read
	while ((*FIFO_STATE & FIFO_VLD) != 0)
	{
		// check start command 
		if (*FIFO_READ == LOCKOUT_MAGIC_START)
		{
			// send response
			while ((*FIFO_STATE & FIFO_RDY) == 0) {}
			*FIFO_WRITE = LOCKOUT_MAGIC_START;
			sev();

			// wait for end of lockout
			do {
				while ((*FIFO_STATE & FIFO_VLD) == 0) wfe();
			} while (*FIFO_READ != LOCKOUT_MAGIC_END);

			// send response
			while ((*FIFO_STATE & FIFO_RDY) == 0) {}
			*FIFO_WRITE = LOCKOUT_MAGIC_END;
			sev();
		}
	}

	// unlock interrupts
	IRQ_UNLOCK;
}

// initialize lockout handler for core 0 or 1
void LockoutInit(int core)
{
	// prepare IRQ number
	int irq = SIO_FIFO_IRQ_NUM(core);

	// set handler
	SetHandler(irq, LockoutHandler);

	// enable IRQ
	NVIC_IRQEnable(irq);
}

// deinitialize lockout handler for core 0 or 1 (disables IRQ handler)
void LockoutTerm(int core)
{
	// disable IRQ
	NVIC_IRQDisable(SIO_FIFO_IRQ_NUM(core));
}

// start lockout other core
void LockoutStart()
{
	FifoHandshake(LOCKOUT_MAGIC_START);
}

// stop lockout other core
void LockoutStop()
{
	FifoHandshake(LOCKOUT_MAGIC_END);
}

#endif // USE_MULTICORE	// use Multicore (sdk_multicore.c, sdk_multicore.h)
