
// ****************************************************************************
//
//                                  PIO
//                    Programmable Input/Output Block
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

#if USE_PIO	// use PIO (sdk_pio.c, sdk_pio.h)

#include "../inc/sdk_pio.h"

#if RP2040
// claimed PIO state machines
u8 PioClaimed = 0;

// map of used instruction memory
u32 PioUsedMap[PIO_NUM] = { 0, 0 };
#else // RP2040
// claimed PIO state machines
u16 PioClaimed = 0;

// map of used instruction memory
u32 PioUsedMap[PIO_NUM] = { 0, 0, 0 };
#endif // RP2040

// === claim state machine

// claim free unused PIO state machine (returns -1 on error)
//  This function is not atomic safe! (not recommended to be used in both cores or in IRQ at the same time)
s8 PioClaimFree(int pio)
{
	int inx, mask;
	mask = B0 << (pio*PIO_SM_NUM);
	for (inx = 0; inx < PIO_SM_NUM; inx++)
	{
		// check if PIO state machine is already claimed
		if ((PioClaimed & mask) == 0)
		{
			// claim this PIO state machine
			PioClaimed |= mask;
			return inx;
		}

		// shift to next PIO state machine
		mask <<= 1;
	}

	// no free PIO state machine
	return -1;
}

// === program used map

// check if entry in instruction memory is used
Bool PioIsUsedIns(int pio, int off)
{
	return ((u32)off >= PIO_MEM_NUM) || ((PioUsedMap[pio] & BIT(off)) != 0);
}

// check free space for the program
Bool PioIsFreeProg(int pio, int off, int num)
{
	// check valid range
	if (((u32)off > PIO_MEM_NUM) || ((u32)num > PIO_MEM_NUM) || ((u32)(off + num) > PIO_MEM_NUM)) return False;

	// check free space
	u32 map = PioUsedMap[pio] >> off;
	u32 mask = BIT(num) - 1;
	return (map & mask) == 0;
}

// find free space for the program (returns offset or -1 on error)
int PioFindFreeProg(int pio, int num)
{
	// search down from the top
	int off;
	for (off = PIO_MEM_NUM - num; off >= 0; off--) if (PioIsFreeProg(pio, off, num)) break;
	return off;
}

// mark program as used
void PioSetUsedProg(int pio, int off, int num)
{
	PioUsedMap[pio] |= (BIT(num) - 1) << off;
}

// mark program as not used
void PioUnsetUsedProg(int pio, int off, int num)
{
	PioUsedMap[pio] &= ~((BIT(num) - 1) << off);
}

// === state machine configuration structure (pio_sm_config)

void PioCfg_hw(pio_hw_t* hw, int sm, const pio_sm_config* cfg)
{
	pio_sm_hw_t* s = &hw->sm[sm];
	s->clkdiv = cfg->clkdiv;
	s->execctrl = cfg->execctrl;
	s->shiftctrl = cfg->shiftctrl;
	s->pinctrl = cfg->pinctrl;
}

// === setup state machine using hardware registers

// PIO set default setup of state machine
//  pio ... PIO number 0 or 1
//  sm ... state machine 0 to 3
void PioSMDefault(int pio, int sm)
{
	*PIO_CLKDIV(pio, sm) = 0x10000;
	*PIO_EXECCTRL(pio, sm) = 0x1f << 12;
	*PIO_SHIFTCTRL(pio, sm) = B19 + B18;
	*PIO_PINCTRL(pio, sm) = 5 << 26;
}

void PioSMDefault_hw(pio_hw_t* hw, int sm)
{
	pio_sm_hw_t* s = &hw->sm[sm];
	s->clkdiv = 0x10000;
	s->execctrl = 0x1f << 12;
	s->shiftctrl = B19 + B18;
	s->pinctrl = 5 << 26;
}

// PIO set state machine clock divider as float (frequency = sys_clk / clock_divider)
//  pio ... PIO number 0 or 1
//  sm ... state machine 0 to 3
//  clkdiv ... clock_divider (default 1.00, 1 instruction per 1 system clock)
void PioSetClkdivFloat(int pio, int sm, float clkdiv)
{
	u32 clk = (u32)(clkdiv*256 + 0.5f);
	*PIO_CLKDIV(pio, sm) = clk << 8;
}

void PioSetClkdivFloat_hw(pio_hw_t* hw, int sm, float clkdiv)
{
	u32 clk = (u32)(clkdiv*256 + 0.5f);
	hw->sm[sm].clkdiv = clk << 8;
}

// PIO setup sideset
//  pio ... PIO number 0 or 1
//  sm ... state machine 0 to 3
//  base ... base GPIO pin asserted by sideset output, 0 to 31 (default 0)
//  count ... number of bits used for side set (inclusive enable bit if present), 0 to 5 (default 0)
//  optional ... topmost side set bit is used as enable flag for whether apply side set on that instruction (default False)
//  pindirs ... side set affects pin directions rather than values (default False)
void PioSetupSideset(int pio, int sm, int base, int count, Bool optional, Bool pindirs)
{
	RegMask(PIO_PINCTRL(pio, sm), ((u32)count << 29) | ((u32)base << 10), B29+B30+B31 + B10+B11+B12+B13+B14);
	RegMask(PIO_EXECCTRL(pio, sm), (optional ? B30 : 0) | (pindirs ? B29 : 0), B29+B30);
}

void PioSetupSideset_hw(pio_hw_t* hw, int sm, int base, int count, Bool optional, Bool pindirs)
{
	pio_sm_hw_t* s = &hw->sm[sm];
	RegMask(&s->pinctrl, ((u32)count << 29) | ((u32)base << 10), B29+B30+B31 + B10+B11+B12+B13+B14);
	RegMask(&s->execctrl, (optional ? B30 : 0) | (pindirs ? B29 : 0), B29+B30);
}

// === PIO program control

// PIO execute one instruction, wait to complete and then resume execution of main program
//  pio ... PIO number 0 or 1
//  sm ... state machine 0 to 3
//  instr ... instruction
// State machine must be enabled with valid clock divider.
void PioExecWait(int pio, int sm, u16 instr)
{
	*PIO_INSTR(pio, sm) = instr;
	while ((*PIO_EXECCTRL(pio, sm) & B31) != 0) {}
}

void PioExecWait_hw(pio_hw_t* hw, int sm, u16 instr)
{
	pio_sm_hw_t* s = &hw->sm[sm];
	s->instr = instr;
	while ((s->execctrl & B31) != 0) {}
}

// write data to TX FIFO of PIO state machine, wait if TX FIFO is full
//  pio ... PIO number 0 or 1
//  sm ... state machine 0 to 3
//  data ... data value
void PioWriteWait(int pio, int sm, u32 data)
{
	while (PioTxIsFull(pio, sm)) {}
	PioWrite(pio, sm, data);
}

void PioWriteWait_hw(pio_hw_t* hw, int sm, u32 data)
{
	while (PioTxIsFull_hw(hw, sm)) {}
	PioWrite_hw(hw, sm, data);
}

// read data from RX FIFO of PIO state machine, wait if RX FIFO is empty
//  pio ... PIO number 0 or 1
//  sm ... state machine 0 to 3
u32 NOFLASH(PioReadWait)(int pio, int sm)
{
	while (PioRxIsEmpty(pio, sm)) {}
	return PioRead(pio, sm);
}

u32 NOFLASH(PioReadWait_hw)(pio_hw_t* hw, int sm)
{
	while (PioRxIsEmpty_hw(hw, sm)) {}
	return PioRead_hw(hw, sm);
}

// clear TX and RX FIFOs of PIO state machine
void PioFifoClear(int pio, int sm)
{
	// changing the FIFO join state clears the fifo
	RegXor(PIO_SHIFTCTRL(pio, sm), B30+B31);
	RegXor(PIO_SHIFTCTRL(pio, sm), B30+B31);
}

void PioFifoClear_hw(pio_hw_t* hw, int sm)
{
	// changing the FIFO join state clears the fifo
	pio_sm_hw_t* s = &hw->sm[sm];
	RegXor(&s->shiftctrl, B30+B31);
	RegXor(&s->shiftctrl, B30+B31);
}

// clear RX FIFO (read values)
void PioRxFifoClear(int pio, int sm)
{
	while (!PioRxIsEmpty(pio, sm)) { PioRead(pio, sm); }
}

void PioRxFifoClear_hw(pio_hw_t* hw, int sm)
{
	while (!PioRxIsEmpty_hw(hw, sm)) { PioRead_hw(hw, sm); }
}

// clear TX FIFO (executes OUT/PULL instruction; state machine should not be enabled)
void PioTxFifoClear(int pio, int sm)
{
	// prepare instruction - use "OUT null,32" if autopull, or "PULL" if not
	u32 instr = ((*PIO_SHIFTCTRL(pio, sm) & B17) != 0) ? PIO_ENCODE_OUT(PIO_SRCDST_NULL, 32) : PIO_ENCODE_PULL(0, 0);
	u32 save = *PIO_PINCTRL(pio, sm); // save control register
	*PIO_PINCTRL(pio, sm) = 0; // no side set
	while (!PioTxIsEmpty(pio, sm)) PioExec(pio, sm, instr);
	*PIO_PINCTRL(pio, sm) = save; // restore control register
}

void PioTxFifoClear_hw(pio_hw_t* hw, int sm)
{
	// prepare instruction - use "OUT null,32" if autopull, or "PULL" if not
	pio_sm_hw_t* s = &hw->sm[sm];
	u32 instr = ((s->shiftctrl & B17) != 0) ? PIO_ENCODE_OUT(PIO_SRCDST_NULL, 32) : PIO_ENCODE_PULL(0, 0);
	u32 save = s->pinctrl; // save control register
	s->pinctrl = 0; // no side set
	while (!PioTxIsEmpty_hw(hw, sm)) PioExec_hw(hw, sm, instr);
	s->pinctrl = save; // restore control register
}

// Load program into PIO memory
//  pio ... PIO number 0 or 1
//  program ... array of program instructions
//  len ... length of program in number of instrucions
//  off ... offset in PIO memory (program is wrapped to 32 instructions)
// Jump instructions are auto-reallocated to new offset address (program is always compiled from offset 0).
void PioLoadProg(int pio, const u16* program, int len, int off)
{
	// mark memory as used
	PioSetUsedProg(pio, off, len);

	u16 instr;
	int inx = off;
	for (; len > 0; len--)
	{
		// wrap instruction offset
		while (inx < 0) inx += PIO_MEM_NUM;
		while (inx >= PIO_MEM_NUM) inx -= PIO_MEM_NUM;

		// reallocate jump instruction (wrap to 32-addresses)
		instr = *program++;
		if ((instr & PIO_INSTR_MASK) == PIO_INSTR_JMP) // JMP instruction
			instr = (instr & ~0x1F) | ((instr + off) & 0x1F);

		// save instruction into program memory
		*PIO_MEM(pio, inx) = instr;
		inx++;
	}
}

// fill PIO program by NOP instructions (= instruction MOV Y,Y)
//  pio ... PIO number 0 or 1
//  len ... length of program, number of instructions
//  off ... offset in PIO memory (program is wrapped to 32 instructions)
void PioNopProg(int pio, int len, int off)
{
	for (; len > 0; len--)
	{
		// wrap instruction offset
		while (off < 0) off += PIO_MEM_NUM;
		while (off >= PIO_MEM_NUM) off -= PIO_MEM_NUM;

		// save instruction into program memory
		*PIO_MEM(pio, off) = PIO_NOP;
		off++;
	}
}

// clear PIO program by "JMP 0" instructions (= default reset value 0x0000)
//  pio ... PIO number 0 or 1
//  len ... length of program, number of instrucions
//  off ... offset in PIO memory (program is wrapped to 32 instructions)
void PioClearProg(int pio, int len, int off)
{
	// mark memory as not used
	PioUnsetUsedProg(pio, off, len);

	for (; len > 0; len--)
	{
		// wrap instruction offset
		while (off < 0) off += PIO_MEM_NUM;
		while (off >= PIO_MEM_NUM) off -= PIO_MEM_NUM;

		// save instruction into program memory
		*PIO_MEM(pio, off) = 0x0000;
		off++;
	}
}

// set output value of pin controlled by the PIO (after initialization with PioSetupGPIO, but before running state machine)
//  pio ... PIO number 0 or 1
//  sm ... state machine 0 to 3
//  pin ... first pin index 0..29
//  count ... number of pins 1..30
//  val ... pin output value 0 or 1
// This is done by executing SET instruction on "victim" state machine (state machine should not be enabled).
void PioSetPin(int pio, int sm, int pin, int count, int val)
{
	u32 save = *PIO_PINCTRL(pio, sm); // save control register
	for (; count > 0; count--)
	{
		*PIO_PINCTRL(pio, sm) = (1 << 26) | ((u32)pin << 5); // setup SET instruction pin, no sideset
		PioExec(pio, sm, PIO_INSTR_SET + val); // execute SET instruction
		pin++;
	}
	*PIO_PINCTRL(pio, sm) = save; // restore control register
}

void PioSetPin_hw(pio_hw_t* hw, int sm, int pin, int count, int val)
{
	pio_sm_hw_t* s = &hw->sm[sm];
	u32 save = s->pinctrl; // save control register
	for (; count > 0; count--)
	{
		s->pinctrl = (1 << 26) | ((u32)pin << 5); // setup SET instruction pin, no sideset
		PioExec_hw(hw, sm, PIO_INSTR_SET + val); // execute SET instruction
		pin++;
	}
	s->pinctrl = save; // restore control register
}

// set direction of pin controlled by the PIO (after initialization with PioSetupGPIO, but before running state machine)
//  pio ... PIO number 0 or 1
//  sm ... state machine 0 to 3
//  pin ... first pin index 0..29
//  count ... number of pins 1..30
//  dir ... pin direction, 1=output, 0=input
// This is done by executing SET instruction on "victim" state machine (state machine should not be enabled).
void PioSetPinDir(int pio, int sm, int pin, int count, int dir)
{
	PioSetPin(pio, sm, pin, count, dir + (4 << 5)); // set to PINDIR destination
}

void PioSetPinDir_hw(pio_hw_t* hw, int sm, int pin, int count, int dir)
{
	PioSetPin_hw(hw, sm, pin, count, dir + (4 << 5)); // set to PINDIR destination
}

// set current program address of PIO state machine
//  pio ... PIO number 0 or 1
//  sm ... state machine 0 to 3
//  addr ... program address 0 to 31
// This is done by executing JMP instruction on "victim" state machine (state machine should not be enabled).
void PioSetAddr(int pio, int sm, int addr)
{
	u32 save = *PIO_PINCTRL(pio, sm); // save control register
	*PIO_PINCTRL(pio, sm) = 0; // no side set
	PioExec(pio, sm, PIO_INSTR_JMP + addr); // execute JMP instruction
	*PIO_PINCTRL(pio, sm) = save; // restore control register
}

void PioSetAddr_hw(pio_hw_t* hw, int sm, int addr)
{
	pio_sm_hw_t* s = &hw->sm[sm];
	u32 save = s->pinctrl; // save control register
	s->pinctrl = 0; // no side set
	PioExec_hw(hw, sm, PIO_INSTR_JMP + addr); // execute JMP instruction
	s->pinctrl = save; // restore control register
}

// initialize PIO state machine, prepare default state
//  pio ... PIO number 0 or 1
//  sm ... state machine 0 to 3
void PioSMInit(int pio, int sm)
{
	// disable PIO state machine
	PioSMDisable(pio, sm);

	// clear TX and RX FIFOs of PIO state machine
	PioFifoClear(pio, sm);

	// restart PIO state machine
	PioSMRestart(pio, sm);

	// restart clock divider of PIO state machine (resets fractional counter)
	PioClkdivRestart(pio, sm);

	// reset debug flags of PIO state machine
	PioDebugClear(pio, sm);

	// PIO set default setup of state machine
	PioSMDefault(pio, sm);

	// reset current program address of PIO state machine to 0
	PioSetAddr(pio, sm, 0);
}

// initialite PIO state machine using configuration
//  pio ... PIO number 0 or 1
//  sm ... state machine 0 to 3
//  addr ... initial PC address
//  cfg ... configuration (NULL = use default configuration)
void PioSMInitCfg(int pio, int sm, int addr, const pio_sm_config* cfg)
{
	// initialize PIO state machine (and disable it)
	PioSMInit(pio, sm);

	// set configuration
	if (cfg != NULL)
		PioCfg(pio, sm, cfg);
	else
		PioSMDefault(pio, sm);

	// set initial PC address
	PioSetAddr(pio, sm, addr);
}

// initialize PIO with all state machines
//  pio ... PIO number 0 or 1
void PioInit(int pio)
{
	int sm;
	PioClearUsedMap(pio);
	for (sm = 0; sm < PIO_SM_NUM; sm++) PioSMInit(pio, sm); // initialize PIO state machine
	*PIO_IRQFORCE(pio) = 0; // clear IRQ force
	*PIO_IRQ(pio) = 0xff; // clear IRQ requests
	*PIO_INTE(pio, 0) = 0; // disable IRQ 0
	*PIO_INTE(pio, 1) = 0; // disable IRQ 1
	*PIO_INTF(pio, 0) = 0; // reset IRQ 0 force
	*PIO_INTF(pio, 1) = 0; // reset IRQ 1 force
}

#endif // USE_PIO	// use PIO (sdk_pio.c, sdk_pio.h)
