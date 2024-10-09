
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

/* How to use PIO:
- initialize PIO and all SMs with PioInit
- load program with PioLoadProg
- set wrap address with PioSetWrap
- set start address with PioSetAddr if not 0
- setup GPIO pins to use PIO with PioSetupGPIO
- setup state machine parameters with PioSet*
- set pin state and direction with PioSetPin and PioSetPinDir
- setup DMA if needed
- setup IRQ if needed (PioIrqEnable)
- start SM with PioSMEnableMask
- transfer data with PioWriteWait and PioReadWait
*/

#if USE_PIO	// use PIO (sdk_pio.c, sdk_pio.h)

#ifndef _SDK_PIO_H
#define _SDK_PIO_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "../sdk_dreq.h"
#include "sdk_cpu.h"
#include "sdk_gpio.h"

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_pio.h"		// constants of original SDK
#else
#include "orig_rp2350/orig_pio.h"		// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

#if RP2040
#define PIO_NUM			2		// number of PIO controllers
#else
#define PIO_NUM			3		// number of PIO controllers
#endif

#define PIO_SM_NUM		4		// number of PIO state machines
#define PIO_MEM_NUM		32		// number of PIO instructions in memory
#define PIO_IRQ_NUM		2		// number of PIO interrupt requests
#define PIO_FIFO_NUM		4		// number of PIO single FIFO entries
#define PIO_FIFO2_NUM		8		// number of PIO double FIFO entries

//#define NUM_PIO_STATE_MACHINES	PIO_SM_NUM
//#define PIO_INSTRUCTION_COUNT	PIO_MEM_NUM

// PIO hardware registers
#define PIO(pio)		(PIO0_BASE + (pio)*(PIO1_BASE - PIO0_BASE)) // PIO base (pio = 0..2)

#define PIO_CTRL(pio)		((volatile u32*)(PIO(pio)+0x00)) // control register
#define PIO_FSTAT(pio)		((volatile u32*)(PIO(pio)+0x04)) // FIFO status register
#define PIO_FDEBUG(pio)		((volatile u32*)(PIO(pio)+0x08)) // FIFO debug register
#define PIO_FLEVEL(pio)		((volatile u32*)(PIO(pio)+0x0C)) // FIFO level register
#define PIO_TXF(pio, sm)	((volatile u32*)(PIO(pio)+0x10+(sm)*4)) // TX FIFO for this state machine (sm = 0 to 3)
#define PIO_RXF(pio, sm)	((volatile u32*)(PIO(pio)+0x20+(sm)*4)) // RX FIFO for this state machine (sm = 0 to 3)
#define PIO_IRQ(pio)		((volatile u32*)(PIO(pio)+0x30)) // IRQ register
#define PIO_IRQFORCE(pio)	((volatile u32*)(PIO(pio)+0x34)) // IRQ force register
#define PIO_INPUTSYNC(pio)	((volatile u32*)(PIO(pio)+0x38)) // GPIO input synchronizer bypass
#define PIO_DBG_PADOUT(pio)	((volatile u32*)(PIO(pio)+0x3C)) // read pad output
#define PIO_DBG_PADOE(pio)	((volatile u32*)(PIO(pio)+0x40)) // read pad output enables
#define PIO_DBG_CFGINFO(pio)	((volatile u32*)(PIO(pio)+0x44)) // PIO config info
#define PIO_MEM(pio, off)	((volatile u32*)(PIO(pio)+0x48+(off)*4)) // instruction memory (off = 0 to 31)
#define PIO_CLKDIV(pio, sm)	((volatile u32*)(PIO(pio)+0xC8+(sm)*0x18)) // clock divider (sm = 0 to 3)
#define PIO_EXECCTRL(pio, sm)	((volatile u32*)(PIO(pio)+0xCC+(sm)*0x18)) // execution settings (sm = 0 to 3)
#define PIO_SHIFTCTRL(pio, sm)	((volatile u32*)(PIO(pio)+0xD0+(sm)*0x18)) // control shift registers (sm = 0 to 3)
#define PIO_ADDR(pio, sm)	((volatile u32*)(PIO(pio)+0xD4+(sm)*0x18)) // current instruction address (sm = 0 to 3)
#define PIO_INSTR(pio, sm)	((volatile u32*)(PIO(pio)+0xD8+(sm)*0x18)) // current executed instruction (sm = 0 to 3)
#define PIO_PINCTRL(pio, sm)	((volatile u32*)(PIO(pio)+0xDC+(sm)*0x18)) // pin control (sm = 0 to 3)
#define PIO_INTR(pio)		((volatile u32*)(PIO(pio)+0x128)) // raw interrupts
#define PIO_INTE(pio,irq)	((volatile u32*)(PIO(pio)+0x12C+(irq)*12)) // interrupt enable (irq = 0 or 1)
#define PIO_INTF(pio,irq)	((volatile u32*)(PIO(pio)+0x130+(irq)*12)) // interrupt force (irq = 0 or 1)
#define PIO_INTS(pio,irq)	((volatile u32*)(PIO(pio)+0x134+(irq)*12)) // interrupt status (irq = 0 or 1)

// State machine hardware interface
typedef struct pio_sm_hw {
	io32	clkdiv;		// 0x00: Clock divisor register
	io32	execctrl;	// 0x04: Execution/behavioural settings
	io32	shiftctrl;	// 0x08: Control behaviour of the input/output shift registers
	io32	addr;		// 0x0C: Current instruction address
	io32	instr;		// 0x10: Read to see the instruction currently addressed by state machine's program counter
	io32	pinctrl;	// 0x14: State machine pin control
} pio_sm_hw_t;

STATIC_ASSERT(sizeof(pio_sm_hw_t) == 0x18, "Incorrect pio_sm_hw_t!");

// PIO interrupt interface
typedef struct {
	io32	inte;		// 0x00: Interrupt Enable
	io32	intf;		// 0x04: Interrupt Force
	io32	ints;		// 0x08: Interrupt status after masking & forcing
} pio_irq_ctrl_hw_t;

STATIC_ASSERT(sizeof(pio_irq_ctrl_hw_t) == 0x0C, "Incorrect pio_irq_ctrl_hw_t!");

// PIO hardware interface
typedef struct {
	io32	ctrl;			// 0x000: PIO control register
	io32	fstat;			// 0x004: FIFO status register
	io32	fdebug;			// 0x008: FIFO debug register
	io32	flevel;			// 0x00C: FIFO levels
	io32	txf[PIO_SM_NUM];	// 0x010: (4) Direct write access to the TX FIFO for this state machine
	io32	rxf[PIO_SM_NUM];	// 0x020: (4) Direct read access to the RX FIFO for this state machine
	io32	irq;			// 0x030: State machine IRQ flags register
	io32	irq_force;		// 0x034: Writing a 1 to each of these bits will forcibly assert the corresponding IRQ
	io32	input_sync_bypass;	// 0x038: There is a 2-flipflop synchronizer on each GPIO input, which protects PIO logic from metastabilities
	io32	dbg_padout;		// 0x03C: Read to sample the pad output values PIO is currently driving to the GPIOs
	io32	dbg_padoe;		// 0x040: Read to sample the pad output enables (direction) PIO is currently driving to the GPIOs
	io32	dbg_cfginfo;		// 0x044: The PIO hardware has some free parameters that may vary between chip products
	io32	instr_mem[PIO_MEM_NUM];	// 0x048: (32, 0x80 bytes) Write-only access to instruction memory location 0
	pio_sm_hw_t sm[PIO_SM_NUM];	// 0x0C8: (0x18*4 = 0x60) state machines
#if !RP2040
	io32	rxf_putget[PIO_SM_NUM][PIO_FIFO_NUM]; // 0x128: (4*4*4 = 0x40) direct access to RX FIFO entry
	io32	gpiobase;		// 0x168: relocate GPIO base to access more than 32 GPIOs
#endif
	io32	intr;			// 0x128 (0x16C): Raw Interrupts
	union {
		struct {
			io32	inte0;	// 0x12C (0x170): Interrupt Enable for irq0
			io32	intf0;	// 0x130 (0x174): Interrupt Force for irq0
			io32	ints0;	// 0x134 (0x178): Interrupt status after masking & forcing for irq0

			io32	inte1;	// 0x138: (0x17C) Interrupt Enable for irq1
			io32	intf1;	// 0x13C: (0x180) Interrupt Force for irq1
			io32	ints1;	// 0x140: (0x184) Interrupt status after masking & forcing for irq1
		};
		pio_irq_ctrl_hw_t irq_ctrl[PIO_IRQ_NUM]; // 0x12C (0x170)
	};
} pio_hw_t;

#if RP2040
STATIC_ASSERT(sizeof(pio_hw_t) == 0x144, "Incorrect pio_hw_t!");
#else
STATIC_ASSERT(sizeof(pio_hw_t) == 0x188, "Incorrect pio_hw_t!");
#endif

#define pio0_hw ((pio_hw_t*)PIO0_BASE)
#define pio0 pio0_hw

#define pio1_hw ((pio_hw_t*)PIO1_BASE)
#define pio1 pio1_hw

#if !RP2040
#define pio2_hw ((pio_hw_t*)PIO2_BASE)
#define pio2 pio1_hw
#endif

// State machine configuration structure
typedef struct {
	u32	clkdiv;
	u32	execctrl;
	u32	shiftctrl;
	u32	pinctrl;
} pio_sm_config;

// instructions
#define PIO_NOP		0xA042	// NOP pseudoinstruction = MOV Y,Y  side 0

// PIO instruction base bits
#define PIO_INSTR_JMP	0x0000
#define PIO_INSTR_WAIT	0x2000
#define PIO_INSTR_IN	0x4000
#define PIO_INSTR_OUT	0x6000
#define PIO_INSTR_PUSH	0x8000		// bit 7 is also crucial
#define PIO_INSTR_PULL	0x8080		// bit 7 is also crucial
#define PIO_INSTR_MOV	0xA000
#define PIO_INSTR_IRQ	0xC000
#define PIO_INSTR_SET	0xE000

#define PIO_INSTR_MASK	0xE000		// major instruction mask

// PIO JMP instruction condition
#define PIO_COND_ALWAYS		0	// always (no condition)
#define PIO_COND_NOTX		1	// !X (if scratch X is zero)
#define PIO_COND_XDEC		2	// X-- (if scratch X is non-zero, post-decrement)
#define PIO_COND_NOTY		3	// !Y (if scratch Y is zero)
#define PIO_COND_YDEC		4	// Y-- (if scratch Y is non-zero, post-decrement)
#define PIO_COND_NEQU		5	// X!=Y (if scratch X not equal scratch Y)
#define PIO_COND_PIN		6	// PIN (branch on input pin)
#define PIO_COND_NOSRE		7	// !OSRE (if output shift register is not empty)

// PIO WAIT source
#define PIO_WAIT_GPIO		0	// wait for GPIO pin selected by index (not affected by mapping)
#define PIO_WAIT_PIN		1	// wait for input pin selected by index (affected by mapping)
#define PIO_WAIT_IRQ		2	// wait for IRQ flag selected by index

// PIO instruction source/destination
#define PIO_SRCDST_PINS		0	// PINS
#define PIO_SRCDST_X		1	// X (scratch register X)
#define PIO_SRCDST_Y		2	// Y (scratch register Y)
#define PIO_SRCDST_NULL		3	// NULL (read all zeroes, write discard data)
#define PIO_SRCDST_DIRMOV	4	// write PINDIRS, MOV write EXEC as instruction
#define PIO_SRCDST_STATUSPC	5	// read STATUS, write PC
#define PIO_SRCDST_ISR		6	// ISR
#define PIO_SRCDST_OSROUT	7	// OSR, OUT write EXEC as instruction

// PIO MOV operation
#define PIO_OP_NONE		0	// none
#define PIO_OP_INV		1	// invert (bitwise complement)
#define PIO_OP_REV		2	// bit-reverse

// encode instructions (without delay/sideset)
//  JMP instruction, cond = condition PIO_COND_*, addr = destination address 0 to 31
#define PIO_ENCODE_JMP(cond, addr) (PIO_INSTR_JMP | ((cond)<<5) | ((addr)&0x1f))
//  WAIT instruction, pol = polarity wait for 1 or 0, src = source PIO_WAIT_*, inx = pin or IRQ index
#define PIO_ENCODE_WAIT(pol, src, inx) (PIO_INSTR_WAIT | ((pol)<<7) | ((src)<<5) | (inx))
//  IN instruction, src = source PIO_SRCDST_*, cnt = bit count 1..32
#define PIO_ENCODE_IN(src, cnt) (PIO_INSTR_IN | ((src)<<5) | ((cnt)&0x1f))
//  OUT instruction, dst = destination PIO_SRCDST_*, cnt = bit count 1..32
#define PIO_ENCODE_OUT(dst, cnt) (PIO_INSTR_OUT | ((dst)<<5) | ((cnt)&0x1f))
//  PUSH instruction, iffull = 1 do nothing until threshold, block = 1 stall if RX full
#define PIO_ENCODE_PUSH(iffull, block) (PIO_INSTR_PUSH | ((iffull)<<6) | ((block)<<5))
//  PULL instruction, ifempty = 1 do nothing until threshold, block = 1 stall if TX empty
#define PIO_ENCODE_PULL(ifempty, block) (PIO_INSTR_PULL | ((ifempty)<<6) | ((block)<<5))
//  MOV instruction, dst = destination PIO_SRCDST_*, op = operation PIO_OP_*, src = source PIO_SRCDST_*
#define PIO_ENCODE_MOV(dst, op, src) (PIO_INSTR_MOV | ((dst)<<5) | ((op)<<3) | (src))
//  IRQ instruction, clr = if 1 clear flag selected by inx, wait = if 1 halt, inx = IRQ index
#define PIO_ENCODE_IRQ(clr, wait, inx) (PIO_INSTR_IRQ | ((clr)<<6) | ((wait)<<5) | (inx))
//  SET instruction, dst = destination PIO_SRCDST_*, data = data 5-bit value 0..31
#define PIO_ENCODE_SET(dst, data) (PIO_INSTR_SET | ((dst)<<5) | (data))

// encode side-set of instructions (can be OR-ed with encoded instruction code)
//   bits = bits of sideset 0 to 5 including opt bit (0 = use only wait, 5 = use only sideset)
//   opt = 1 use sideset, or 0 use only wait, or 0 if not using opt bit
//   sideset = output sideset bits, or 0 use only wait
//   wait = wait cycles 0 to 31 or less
#define PIO_ENCODE_SIDESET(bits, opt, sideset, wait) (((opt)<<12) | ((sideset)<<(13-(bits))) | ((wait)<<8))

// PIO interrupt type
#define PIO_INT_RXNEMPTY	0	// RX FIFO is not empty
#define PIO_INT_TXNFULL		1	// TX FIFO is not full
#define PIO_INT_SM		2	// interrupt from state machine

// PIO FIFO joining type
#define PIO_FIFO_JOIN_RXTX	0		// use separate RX FIFO and TX FIFO, every 4-entries deep
#define PIO_FIFO_JOIN_TX	1		// use only TX FIFO, 8-entries deep
#define PIO_FIFO_JOIN_RX	2		// use only RX FIFO, 8-entries deep

#if !RP2040
#define PIO_FIFO_JOIN_RXGET	4		// RX FIFO will be disabled and used for read GET instruction, CPU can access RX FIFO
#define PIO_FIFO_JOIN_RXPUT	8		// RX FIFO will be disabled and used for write PUT instruction, CPU can access RX FIFO
#define PIO_FIFO_JOIN_RXGETPUT	12		// RX FIFO will be disabled and used for read/write GET/PUT instruction, CPU cannot access RX FIFO
#endif

#define PIO_FIFO_JOIN_NONE	PIO_FIFO_JOIN_RXTX

// claimed PIO state machines
#if RP2040
extern u8 PioClaimed;
#else
extern u16 PioClaimed;
#endif

// map of used instruction memory
extern u32 PioUsedMap[PIO_NUM];

#if RP2040
// get hardware interface from PIO index
INLINE pio_hw_t* PioGetHw(int pio) { return (pio == 0) ? pio0_hw : pio1_hw; }

// get PIO index from hardware interface
INLINE u8 PioGetInx(const pio_hw_t* hw) { return (hw == pio0_hw) ? 0 : 1; }
#else // RP2040
// get hardware interface from PIO index
INLINE pio_hw_t* PioGetHw(int pio) { return (pio == 0) ? pio0_hw : ((pio == 1) ? pio1_hw : pio2_hw); }

// get PIO index from hardware interface
INLINE u8 PioGetInx(const pio_hw_t* hw) { return (hw == pio0_hw) ? 0 : ((hw == pio1_hw) ? 1 : 2); }
#endif // RP2040

// === claim state machine

// claim PIO state machine (mark it as used)
//  This function is not atomic safe! (not recommended to be used in both cores or in IRQ at the same time)
INLINE void PioClaim(int pio, int sm) { PioClaimed |= BIT(pio*PIO_SM_NUM+sm); }

// claim PIO state machines with mask (mark them as used)
//  This function is not atomic safe! (not recommended to be used in both cores or in IRQ at the same time)
INLINE void PioClaimMask(int pio, int mask) { PioClaimed |= mask << (pio*PIO_SM_NUM); }

// unclaim PIO state machine (mark it as not used)
//  This function is not atomic safe! (not recommended to be used in both cores or in IRQ at the same time)
INLINE void PioUnclaim(int pio, int sm) { PioClaimed &= ~BIT(pio*PIO_SM_NUM+sm); }

// unclaim PIO state machines with mask (mark them as not used)
//  This function is not atomic safe! (not recommended to be used in both cores or in IRQ at the same time)
INLINE void PioUnclaimMask(int pio, int mask) { PioClaimed &= ~(mask << (pio*PIO_SM_NUM)); }

// check if PIO state machine is claimed
INLINE Bool PioIsClaimed(int pio, int sm) { return (PioClaimed & BIT(pio*PIO_SM_NUM+sm)) != 0; }

// claim free unused PIO state machine (returns -1 on error)
//  This function is not atomic safe! (not recommended to be used in both cores or in IRQ at the same time)
s8 PioClaimFree(int pio);

// === program used map

// clear map of used instruction memory
INLINE void PioClearUsedMap(int pio) { PioUsedMap[pio] = 0; }

// mark one instruction in memory as used
INLINE void PioSetUsedIns(int pio, int off) { PioUsedMap[pio] |= BIT(off); }

// mark one instruction in memory as not used
INLINE void PioUnsetUsedIns(int pio, int off) { PioUsedMap[pio] &= ~BIT(off); }

// check if entry in instruction memory is used
Bool PioIsUsedIns(int pio, int off);

// check free space for the program
Bool PioIsFreeProg(int pio, int off, int num);

// find free space for the program (returns offset or -1 on error)
int PioFindFreeProg(int pio, int num);

// mark program as used
void PioSetUsedProg(int pio, int off, int num);

// mark program as not used
void PioUnsetUsedProg(int pio, int off, int num);

// === state machine configuration structure (pio_sm_config)

// get default configuration structure of state machine (use PioSetCfg() to apply configuration)
INLINE pio_sm_config PioCfgDef(void)
{
	pio_sm_config c = {
		0x10000,	// clkdiv: clock divisor (= 1.00)
		31 << 12,	// execctrl: execution settings (wrap top = 31)
		B18 | B19,	// shiftctrl: control shift registers (OSR and ISR shift right)
		5 << 26,	// pinctrl: pin control
	};
	return c;
}

// apply state machine configuration
void PioCfg_hw(pio_hw_t* hw, int sm, const pio_sm_config* cfg);
INLINE void PioCfg(int pio, int sm, const pio_sm_config* cfg) { PioCfg_hw(PioGetHw(pio), sm, cfg); }

// config set state machine clock divider (frequency = sys_clk / clock_divider)
//  cfg ... state machine configuration
//  clkdiv ... clock_divider * 256 (default 0x100, means clock_divider=1.00, 1 instruction per 1 system clock)
INLINE void PioCfgClkdiv(pio_sm_config* cfg, u32 clkdiv) { cfg->clkdiv = clkdiv << 8; }

// config set state machine clock divider as float (frequency = sys_clk / clock_divider)
//  cfg ... state machine configuration
//  clkdiv ... clock_divider (default 1.00, 1 instruction per 1 system clock)
INLINE void PioCfgClkdivFloat(pio_sm_config* cfg, float clkdiv) { cfg->clkdiv = (u32)(clkdiv*256 + 0.5f) << 8; }

// config setup sideset
//  cfg ... state machine configuration
//  base ... base GPIO pin asserted by sideset output, 0 to 31 (default 0)
//  count ... number of bits used for side set (inclusive enable bit if present), 0 to 5 (default 0)
//  optional ... topmost side set bit is used as enable flag for whether apply side set on that instruction (default False)
//  pindirs ... side set affects pin directions rather than values (default False)
INLINE void PioCfgSideset(pio_sm_config* cfg, int base, int count, Bool optional, Bool pindirs)
{
	cfg->pinctrl = (cfg->pinctrl & ~(B29+B30+B31 + B10+B11+B12+B13+B14)) |
		((u32)count << 29) | ((u32)base << 10);
	cfg->execctrl = (cfg->execctrl & ~(B29+B30)) | (optional ? B30 : 0) | (pindirs ? B29 : 0);
}

// PIO set GPIO pin for JMP PIN instruction
//  cfg ... state machine configuration
//  pin ... GPIO pin to use as condition for JMP PIN (0 to 29, default 0)
INLINE void PioCfgJmpPin(pio_sm_config* cfg, int pin)
	{ cfg->execctrl = (cfg->execctrl & ~(B24+B25+B26+B27+B28)) | ((u32)pin << 24); }

// config set special OUT
//  cfg ... state machine configuration
//  sticky ... enable 'sticky' output (continuously re-asserting most recent OUT/SET values to the pins; default False)
//  has_enable_pin ... use auxiliary OUT enable pin (default False; If True, use a bit of OUT data as an auxiliary
//                 write enable. When used in conjunction with OUT_STICKY, writes with an enable of False
//                 will deassert the latest pin write. This can create useful masking/override behaviour
//                 due to the priority ordering of state nachine pin writes SM0 < SM1 < … )
//  enable_pin_index ... Which data bit to use for auxiliary OUT enable (0..31; default 0)
INLINE void PioCfgOutSpecial(pio_sm_config* cfg, Bool sticky, Bool has_enable_pin, int enable_pin_index)
{
	cfg->execctrl = (cfg->execctrl & ~(B17+B18+B19+B20+B21+B22+B23)) |
		(sticky ? B17 : 0) | (has_enable_pin ? B18 : 0) | ((u32)enable_pin_index << 19);
}

// config set wrap address
//  cfg ... state machine configuration
//  wrap_target ... wrap destination, after reaching wrap_top execution is wrapped to this address (0 to 31, default 0)
//  wrap_top ... wrap source, after reaching this address execution is then wrapped to wrap_target (0 to 31, default 31)
// If wrap_top instruction is jump with true condition, the jump takes priority.
INLINE void PioCfgWrap(pio_sm_config* cfg, int wrap_target, int wrap_top)
{
	cfg->execctrl = (cfg->execctrl & ~((0x1f << 7) | (0x1f << 12))) |
		((u32)(wrap_target & 0x1f) << 7) | ((u32)(wrap_top & 0x1f) << 12);
}

// PIO set "MOV x,STATUS"
//  cfg ... state machine configuration
//  status_rx ... True compare RX FIFO level, False compare TX FIFO level (default False)
//  level ... comparison level N (default 0)
// Instruction "MOV x,STATUS" fills all-ones if FIFO level < N, otherwise all-zeroes
INLINE void PioCfgMovStatus(pio_sm_config* cfg, Bool status_rx, int level)
	{ cfg->execctrl = (cfg->execctrl & ~0x1f) | (status_rx ? B4 : 0) | (level & 0x0f); }

// config set FIFO joining
//  cfg ... state machine configuration
//  join ... set FIFO joining PIO_FIFO_JOIN_* (default PIO_FIFO_JOIN_RXTX)
INLINE void PioCfgFifoJoin(pio_sm_config* cfg, int join)
	{ cfg->shiftctrl = (cfg->shiftctrl & ~(B30+B31)) | ((u32)join << 30); }

// config set OUT shifting
//  cfg ... state machine configuration
//  shift_right ... shift OSR right (default True)
//  autopull ... autopull enable (pull when OSR is empty, default False)
//  pull_threshold ... threshold in bits to shift out before auto/conditional re-pulling OSR (1 to 32, default 32)
INLINE void PioCfgOutShift(pio_sm_config* cfg, Bool shift_right, Bool autopull, int pull_threshold)
{
	cfg->shiftctrl = (cfg->shiftctrl & ~(B17+B19+B25+B26+B27+B28+B29)) |
		(shift_right ? B19 : 0) | (autopull ? B17 : 0) | ((u32)(pull_threshold & 0x1f) << 25);
}

// config set IN shifting
//  cfg ... state machine configuration
//  shift_right ... shift ISR right (data enters from left, default True)
//  autopush ... autopush enable (push when ISR is filled, default False)
//  push_threshold ... threshold in bits to shift in before auto/conditional re-pushing ISR (1 to 32, default 32)
INLINE void PioCfgInShift(pio_sm_config* cfg, Bool shift_right, Bool autopush, int push_threshold)
{
	cfg->shiftctrl = (cfg->shiftctrl & ~(B16+B18+B20+B21+B22+B23+B24)) |
		(shift_right ? B18 : 0) | (autopush ? B16 : 0) | ((u32)(push_threshold & 0x1f) << 20);
}

// config setup SET pins
//  cfg ... state machine configuration
//  base ... base GPIO pin asserted by SET instruction, 0 to 31 (default 0)
//  count ... number of GPIO pins asserted by SET instruction, 0 to 5 (default 5)
INLINE void PioCfgSet(pio_sm_config* cfg, int base, int count)
{
	cfg->pinctrl = (cfg->pinctrl & ~(B5+B6+B7+B8+B9 + B26+B27+B28)) | ((u32)base << 5) | ((u32)count << 26);
}

// config setup OUT pins
//  cfg ... state machine configuration
//  base ... base GPIO pin asserted by OUT instruction, 0 to 31 (default 0)
//  count ... number of GPIO pins asserted by OUT instruction, 1 to 32 (default 32)
INLINE void PioCfgOut(pio_sm_config* cfg, int base, int count)
{
	cfg->pinctrl = (cfg->pinctrl & ~(B0+B1+B2+B3+B4 + B20+B21+B22+B23+B24+B25)) | base | ((u32)(count & 0x1f) << 20);
}

// config setup IN pins
//  cfg ... state machine configuration
//  base ... base GPIO pin asserted by IN instruction, 0 to 31 (default 0)
INLINE void PioCfgIn(pio_sm_config* cfg, int base)
{
	cfg->pinctrl = (cfg->pinctrl & ~(B15+B16+B17+B18+B19)) | ((u32)base << 15);
}

// === setup state machine using hardware registers

// PIO set default setup of state machine
//  pio ... PIO number
//  sm ... state machine 0 to 3
void PioSMDefault(int pio, int sm);
void PioSMDefault_hw(pio_hw_t* hw, int sm);

// PIO set state machine clock divider (frequency = sys_clk / clock_divider)
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  clkdiv ... clock_divider * 256 (default 0x100, means clock_divider=1.00, 1 instruction per 1 system clock)
INLINE void PioSetClkdiv(int pio, int sm, u32 clkdiv) { *PIO_CLKDIV(pio, sm) = clkdiv << 8; }
INLINE void PioSetClkdiv_hw(pio_hw_t* hw, int sm, u32 clkdiv) { hw->sm[sm].clkdiv = clkdiv << 8; }

// PIO set state machine clock divider as float (frequency = sys_clk / clock_divider)
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  clkdiv ... clock_divider (default 1.00, 1 instruction per 1 system clock)
void PioSetClkdivFloat(int pio, int sm, float clkdiv);
void PioSetClkdivFloat_hw(pio_hw_t* hw, int sm, float clkdiv);

// PIO setup sideset
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  base ... base GPIO pin asserted by sideset output, 0 to 31 (default 0)
//  count ... number of bits used for side set (inclusive enable bit if present), 0 to 5 (default 0)
//  optional ... topmost side set bit is used as enable flag for whether apply side set on that instruction (default False)
//  pindirs ... side set affects pin directions rather than values (default False)
void PioSetupSideset(int pio, int sm, int base, int count, Bool optional, Bool pindirs);
void PioSetupSideset_hw(pio_hw_t* hw, int sm, int base, int count, Bool optional, Bool pindirs);

// PIO set GPIO pin for JMP PIN instruction
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  pin ... GPIO pin to use as condition for JMP PIN (0 to 29, default 0)
INLINE void PioSetJmpPin(int pio, int sm, int pin) { RegMask(PIO_EXECCTRL(pio, sm), (u32)pin << 24, B24+B25+B26+B27+B28); }
INLINE void PioSetJmpPin_hw(pio_hw_t* hw, int sm, int pin) { RegMask(&hw->sm[sm].execctrl, (u32)pin << 24, B24+B25+B26+B27+B28); }

// PIO set special OUT
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  sticky ... enable 'sticky' output (continuously re-asserting most recent OUT/SET values to the pins; default False)
//  has_enable_pin ... use auxiliary OUT enable pin (default False; If True, use a bit of OUT data as an auxiliary
//                 write enable. When used in conjunction with OUT_STICKY, writes with an enable of False
//                 will deassert the latest pin write. This can create useful masking/override behaviour
//                 due to the priority ordering of state nachine pin writes SM0 < SM1 < … )
//  enable_pin_index ... Which data bit to use for auxiliary OUT enable (0..31; default 0)
INLINE void PioSetOutSpecial(int pio, int sm, Bool sticky, Bool has_enable_pin, int enable_pin_index)
{
 	RegMask(PIO_EXECCTRL(pio, sm), (sticky ? B17 : 0) | (has_enable_pin ? B18 : 0) |
		((u32)enable_pin_index << 19), B17 + B18 + B19+B20+B21+B22+B23);
}

INLINE void PioSetOutSpecial_hw(pio_hw_t* hw, int sm, Bool sticky, Bool has_enable_pin, int enable_pin_index)
{
 	RegMask(&hw->sm[sm].execctrl, (sticky ? B17 : 0) | (has_enable_pin ? B18 : 0) |
		((u32)enable_pin_index << 19), B17 + B18 + B19+B20+B21+B22+B23);
}

// PIO set wrap address
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  wrap_target ... wrap destination, after reaching wrap_top execution is wrapped to this address (0 to 31, default 0)
//  wrap_top ... wrap source, after reaching this address execution is then wrapped to wrap_target (0 to 31, default 31)
// If wrap_top instruction is jump with true condition, the jump takes priority.
INLINE void PioSetWrap(int pio, int sm, int wrap_target, int wrap_top)
{
	RegMask(PIO_EXECCTRL(pio, sm), ((u32)(wrap_target & 0x1f) << 7) | 
		((u32)(wrap_top & 0x1f) << 12), (0x1f << 7) | (0x1f << 12));
}

INLINE void PioSetWrap_hw(pio_hw_t* hw, int sm, int wrap_target, int wrap_top)
{
	RegMask(&hw->sm[sm].execctrl, ((u32)(wrap_target & 0x1f) << 7) | 
		((u32)(wrap_top & 0x1f) << 12), (0x1f << 7) | (0x1f << 12));
}

#if RP2040
// PIO set "MOV x,STATUS"
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  status_rx ... True compare RX FIFO level, False compare TX FIFO level (default False)
//  level ... comparison level N (default 0)
// Instruction "MOV x,STATUS" fills all-ones if FIFO level < N, otherwise all-zeroes
INLINE void PioSetMovStatus(int pio, int sm, Bool status_rx, int level) { RegMask(PIO_EXECCTRL(pio, sm), (status_rx ? B4 : 0) | (level & 0x0f), 0x1f); }
INLINE void PioSetMovStatus_hw(pio_hw_t* hw, int sm, Bool status_rx, int level) { RegMask(&hw->sm[sm].execctrl, (status_rx ? B4 : 0) | (level & 0x0f), 0x1f); }
#else // RP2040
// PIO set "MOV x,STATUS"
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  status_rx ... 0 compare TX FIFO level (default), 1 compare RX FIFO level, 2 indexed IRQ flag raised
//  level ... comparison level N (default 0); IRQ: 0..7 this PIO, 8..0xF lower PIO, 0x10..0x17 higher PIO
// Instruction "MOV x,STATUS" fills all-ones if FIFO level < N, otherwise all-zeroes
INLINE void PioSetMovStatus(int pio, int sm, int status_rx, int level) { RegMask(PIO_EXECCTRL(pio, sm), (status_rx << 5) | (level & 0x1f), 0x7f); }
INLINE void PioSetMovStatus_hw(pio_hw_t* hw, int sm, int status_rx, int level) { RegMask(&hw->sm[sm].execctrl, (status_rx << 5) | (level & 0x1f), 0x7f); }
#endif // RP2040

// PIO set FIFO joining
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  join ... set FIFO joining PIO_FIFO_JOIN_* (default PIO_FIFO_JOIN_RXTX)
#if RP2040
INLINE void PioSetFifoJoin(int pio, int sm, int join) { RegMask(PIO_SHIFTCTRL(pio, sm), (u32)join << 30, B30+B31); }
INLINE void PioSetFifoJoin_hw(pio_hw_t* hw, int sm, int join) { RegMask(&hw->sm[sm].shiftctrl, (u32)join << 30, B30+B31); }
#else
INLINE void PioSetFifoJoin(int pio, int sm, int join) { RegMask(PIO_SHIFTCTRL(pio, sm),
	(((u32)(join & 3) << 30) | ((u32)(join & 0x0c) << (14-2))), B14+B15+B30+B31); }
INLINE void PioSetFifoJoin_hw(pio_hw_t* hw, int sm, int join) { RegMask(&hw->sm[sm].shiftctrl,
	(((u32)(join & 3) << 30) | ((u32)(join & 0x0c) << (14-2))), B14+B15+B30+B31); }
#endif

#if !RP2040
// PIO set number of pins visible by IN PINS, WAIT PIN and MOV x,PINS instructions
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  count ... number of visible pins
INLINE void PioSetInCount(int pio, int sm, int count) { RegMask(PIO_SHIFTCTRL(pio, sm), count, B0+B1+B2+B3+B4); }
INLINE void PioSetInCount_hw(pio_hw_t* hw, int sm, int count) { RegMask(&hw->sm[sm].shiftctrl, count, B0+B1+B2+B3+B4); }
#endif

// PIO set OUT shifting
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  shift_right ... shift OSR right (default True)
//  autopull ... autopull enable (pull when OSR is empty, default False)
//  pull_threshold ... threshold in bits to shift out before auto/conditional re-pulling OSR (1 to 32, default 32)
INLINE void PioSetOutShift(int pio, int sm, Bool shift_right, Bool autopull, int pull_threshold)
{
	RegMask(PIO_SHIFTCTRL(pio, sm), (shift_right ? B19 : 0) | (autopull ? B17 : 0) |
		((u32)(pull_threshold & 0x1f) << 25), B17+B19+B25+B26+B27+B28+B29);
}

INLINE void PioSetOutShift_hw(pio_hw_t* hw, int sm, Bool shift_right, Bool autopull, int pull_threshold)
{
	RegMask(&hw->sm[sm].shiftctrl, (shift_right ? B19 : 0) | (autopull ? B17 : 0) |
		((u32)(pull_threshold & 0x1f) << 25), B17+B19+B25+B26+B27+B28+B29);
}

// PIO set IN shifting
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  shift_right ... shift ISR right (data enters from left, default True)
//  autopush ... autopush enable (push when ISR is filled, default False)
//  push_threshold ... threshold in bits to shift in before auto/conditional re-pushing ISR (1 to 32, default 32)
INLINE void PioSetInShift(int pio, int sm, Bool shift_right, Bool autopush, int push_threshold)
{
	RegMask(PIO_SHIFTCTRL(pio, sm), (shift_right ? B18 : 0) | (autopush ? B16 : 0) |
		((u32)(push_threshold & 0x1f) << 20), B16+B18+B20+B21+B22+B23+B24);
}

INLINE void PioSetInShift_hw(pio_hw_t* hw, int sm, Bool shift_right, Bool autopush, int push_threshold)
{
	RegMask(&hw->sm[sm].shiftctrl, (shift_right ? B18 : 0) | (autopush ? B16 : 0) |
		((u32)(push_threshold & 0x1f) << 20), B16+B18+B20+B21+B22+B23+B24);
}

// PIO setup SET pins
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  base ... base GPIO pin asserted by SET instruction, 0 to 31 (default 0)
//  count ... number of GPIO pins asserted by SET instruction, 0 to 5 (default 5)
INLINE void PioSetupSet(int pio, int sm, int base, int count)
{
	RegMask(PIO_PINCTRL(pio, sm), ((u32)base << 5) | ((u32)count << 26), B5+B6+B7+B8+B9 + B26+B27+B28);
}

INLINE void PioSetupSet_hw(pio_hw_t* hw, int sm, int base, int count)
{
	RegMask(&hw->sm[sm].pinctrl, ((u32)base << 5) | ((u32)count << 26), B5+B6+B7+B8+B9 + B26+B27+B28);
}

// PIO setup OUT pins
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  base ... base GPIO pin asserted by OUT instruction, 0 to 31 (default 0)
//  count ... number of GPIO pins asserted by OUT instruction, 1 to 32 (default 32)
INLINE void PioSetupOut(int pio, int sm, int base, int count)
{
	RegMask(PIO_PINCTRL(pio, sm), base | ((u32)(count & 0x1f) << 20), B0+B1+B2+B3+B4 + B20+B21+B22+B23+B24+B25);
}

INLINE void PioSetupOut_hw(pio_hw_t* hw, int sm, int base, int count)
{
	RegMask(&hw->sm[sm].pinctrl, base | ((u32)(count & 0x1f) << 20), B0+B1+B2+B3+B4 + B20+B21+B22+B23+B24+B25);
}

// PIO setup IN pins
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  base ... base GPIO pin asserted by IN instruction, 0 to 31 (default 0)
INLINE void PioSetupIn(int pio, int sm, int base)
{
	RegMask(PIO_PINCTRL(pio, sm), (u32)base << 15, B15+B16+B17+B18+B19);
}

INLINE void PioSetupIn_hw(pio_hw_t* hw, int sm, int base)
{
	RegMask(&hw->sm[sm].pinctrl, (u32)base << 15, B15+B16+B17+B18+B19);
}

// === setup PIO

// get DREQ to use for pacing transfers to state machine
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  tx ... True for sending data from CPU to state machine, False for receiving data from state machine to CPU
INLINE u8 PioGetDreq(int pio, int sm, Bool tx)
{
	return DREQ_PIO0_TX0 + pio*(DREQ_PIO1_TX0 - DREQ_PIO0_TX0) + sm + (tx ? 0 : PIO_SM_NUM);
}

// setup GPIO pins to use output from PIO
//  pio ... PIO number
//  pin ... pin base 0 to 29
//  count ... number of pins 0 to 29
INLINE void PioSetupGPIO(int pio, int pin, int count) { GPIO_FncMask(RangeMask(pin, pin+count-1), GPIO_FNC_PIO0 + pio); }

// enable PIO state machine
//  pio ... PIO number
//  sm ... state machine 0 to 3
INLINE void PioSMEnable(int pio, int sm) { RegSet(PIO_CTRL(pio), 1 << sm); }
INLINE void PioSMEnable_hw(pio_hw_t* hw, int sm) { RegSet(&hw->ctrl, 1 << sm); }

// disable PIO state machine
//  pio ... PIO number
//  sm ... state machine 0 to 3
INLINE void PioSMDisable(int pio, int sm) { RegClr(PIO_CTRL(pio), 1 << sm); }
INLINE void PioSMDisable_hw(pio_hw_t* hw, int sm) { RegClr(&hw->ctrl, 1 << sm); }

// enable multiple PIO state machines
//  pio ... PIO number
//  sm_mask ... state machines mask of bit B0 to B3
// To use mask in range (first..last), use function RangeMask.
INLINE void PioSMEnableMask(int pio, int sm_mask) { RegSet(PIO_CTRL(pio), sm_mask); }
INLINE void PioSMEnableMask_hw(pio_hw_t* hw, int sm_mask) { RegSet(&hw->ctrl, sm_mask); }

// enable multiple PIO state machines synchronized (resets their clock dividers)
//  pio ... PIO number
//  sm_mask ... state machines mask of bit B0 to B3
// To use mask in range (first..last), use function RangeMask.
INLINE void PioSMEnableMaskSync(int pio, int sm_mask) { RegSet(PIO_CTRL(pio), sm_mask | ((u32)sm_mask << 8)); }
INLINE void PioSMEnableMaskSync_hw(pio_hw_t* hw, int sm_mask) { RegSet(&hw->ctrl, sm_mask | ((u32)sm_mask << 8)); }

// disable multiple PIO state machines
//  pio ... PIO number
//  sm_mask ... state machines mask of bits B0 to B3
// To use mask in range (first..last), use function RangeMask.
INLINE void PioSMDisableMask(int pio, int sm_mask) { RegClr(PIO_CTRL(pio), sm_mask); }
INLINE void PioSMDisableMask_hw(pio_hw_t* hw, int sm_mask) { RegClr(&hw->ctrl, sm_mask); }

// restart PIO state machine
//  pio ... PIO number
//  sm ... state machine 0 to 3
// This method clears ISR, shift counters, clock divider counter,
// pin write flags, delay counter, latched EXEC instruction, IRQ wait condition
INLINE void PioSMRestart(int pio, int sm) { RegSet(PIO_CTRL(pio), 1 << (sm+4)); }
INLINE void PioSMRestart_hw(pio_hw_t* hw, int sm) { RegSet(&hw->ctrl, 1 << (sm+4)); }

// restart multiple PIO state machines
//  pio ... PIO number
//  sm_mask ... state machines mask of bits B0 to B3
// This method clears ISR, shift counters, clock divider counter,
// pin write flags, delay counter, latched EXEC instruction, IRQ wait condition
INLINE void PioSMRestartMask(int pio, int sm_mask) { RegSet(PIO_CTRL(pio), (u32)sm_mask << 4); }
INLINE void PioSMRestartMask_hw(pio_hw_t* hw, int sm_mask) { RegSet(&hw->ctrl, (u32)sm_mask << 4); }

// restart clock divider of PIO state machine (resets fractional counter)
//  pio ... PIO number
//  sm ... state machine 0 to 3
INLINE void PioClkdivRestart(int pio, int sm) { RegSet(PIO_CTRL(pio), 1 << (sm+8)); }
INLINE void PioClkdivRestart_hw(pio_hw_t* hw, int sm) { RegSet(&hw->ctrl, 1 << (sm+8)); }

// restart clock divider of multiple PIO state machines (resets fractional counter)
//  pio ... PIO number
//  sm_mask ... state machines mask of bits B0 to B3
INLINE void PioClkdivRestartMask(int pio, int sm_mask) { RegSet(PIO_CTRL(pio), (u32)sm_mask << 8); }
INLINE void PioClkdivRestartMask_hw(pio_hw_t* hw, int sm_mask) { RegSet(&hw->ctrl, (u32)sm_mask << 8); }

// === PIO program control

// get current program counter of PIO state machine (returns 0..31)
//  pio ... PIO number
//  sm ... state machine 0 to 3
INLINE u8 PioGetPC(int pio, int sm) { return (u8)(*PIO_ADDR(pio, sm) & 0x1f); }
INLINE u8 PioGetPC_hw(const pio_hw_t* hw, int sm) { return (u8)(hw->sm[sm].addr & 0x1f); }

// PIO execute one instruction and then resume execution of main program
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  instr ... instruction
INLINE void PioExec(int pio, int sm, u16 instr) { *PIO_INSTR(pio, sm) = instr; }
INLINE void PioExec_hw(pio_hw_t* hw, int sm, u16 instr) { hw->sm[sm].instr = instr; }

// check if instruction set by PioExec is still running
INLINE Bool PioIsExec(int pio, int sm) { return (*PIO_EXECCTRL(pio, sm) & B31) != 0; }
INLINE Bool PioIsExec_hw(const pio_hw_t* hw, int sm) { return (hw->sm[sm].execctrl & B31) != 0; }

// PIO execute one instruction, wait to complete and then resume execution of main program
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  instr ... instruction
// State machine must be enabled with valid clock divider.
void PioExecWait(int pio, int sm, u16 instr);
void PioExecWait_hw(pio_hw_t* hw, int sm, u16 instr);

// write data to TX FIFO of PIO state machine
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  data ... data value
// If TX FIFO is full, the most recent value will be overwritten.
INLINE void PioWrite(int pio, int sm, u32 data) { *PIO_TXF(pio, sm) = data; }
INLINE void PioWrite_hw(pio_hw_t* hw, int sm, u32 data) { hw->txf[sm] = data; }

// read data from RX FIFO of PIO state machine
//  pio ... PIO number
//  sm ... state machine 0 to 3
// If RX FIFO is empty, the return value is zero.
INLINE u32 PioRead(int pio, int sm) { return *PIO_RXF(pio, sm); }
INLINE u32 PioRead_hw(const pio_hw_t* hw, int sm) { return hw->rxf[sm]; }

// check if RX FIFO of PIO state machine is full
//  pio ... PIO number
//  sm ... state machine 0 to 3
INLINE Bool PioRxIsFull(int pio, int sm) { return (*PIO_FSTAT(pio) & BIT(sm)) != 0; }
INLINE Bool PioRxIsFull_hw(const pio_hw_t* hw, int sm) { return (hw->fstat & BIT(sm)) != 0; }

// check if RX FIFO of PIO state machine is empty
//  pio ... PIO number
//  sm ... state machine 0 to 3
INLINE Bool PioRxIsEmpty(int pio, int sm) { return (*PIO_FSTAT(pio) & BIT(sm + 8)) != 0; }
INLINE Bool PioRxIsEmpty_hw(const pio_hw_t* hw, int sm) { return (hw->fstat & BIT(sm + 8)) != 0; }

// get number of elements in RX FIFO of PIO state machine
INLINE u8 PioRxLevel(int pio, int sm) { return (*PIO_FLEVEL(pio) >> (sm*8 + 4)) & 0x0f; }
INLINE u8 PioRxLevel_hw(const pio_hw_t* hw, int sm) { return (hw->flevel >> (sm*8 + 4)) & 0x0f; }

// check if TX FIFO of PIO state machine is full
//  pio ... PIO number
//  sm ... state machine 0 to 3
INLINE Bool PioTxIsFull(int pio, int sm) { return (*PIO_FSTAT(pio) & BIT(sm + 16)) != 0; }
INLINE Bool PioTxIsFull_hw(const pio_hw_t* hw, int sm) { return (hw->fstat & BIT(sm + 16)) != 0; }

// check if TX FIFO of PIO state machine is empty
//  pio ... PIO number
//  sm ... state machine 0 to 3
INLINE Bool PioTxIsEmpty(int pio, int sm) { return (*PIO_FSTAT(pio) & BIT(sm + 24)) != 0; }
INLINE Bool PioTxIsEmpty_hw(const pio_hw_t* hw, int sm) { return (hw->fstat & BIT(sm + 24)) != 0; }

// get number of elements in TX FIFO of PIO state machine
INLINE u8 PioTxLevel(int pio, int sm) { return (*PIO_FLEVEL(pio) >> (sm*8)) & 0x0f; }
INLINE u8 PioTxLevel_hw(const pio_hw_t* hw, int sm) { return (hw->flevel >> (sm*8)) & 0x0f; }

// write data to TX FIFO of PIO state machine, wait if TX FIFO is full
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  data ... data value
void PioWriteWait(int pio, int sm, u32 data);
void PioWriteWait_hw(pio_hw_t* hw, int sm, u32 data);

// read data from RX FIFO of PIO state machine, wait if RX FIFO is empty
//  pio ... PIO number
//  sm ... state machine 0 to 3
u32 NOFLASH(PioReadWait)(int pio, int sm);
u32 NOFLASH(PioReadWait_hw)(pio_hw_t* hw, int sm);

// clear TX and RX FIFOs of PIO state machine
void PioFifoClear(int pio, int sm);
void PioFifoClear_hw(pio_hw_t* hw, int sm);

// clear RX FIFO (read values)
void PioRxFifoClear(int pio, int sm);
void PioRxFifoClear_hw(pio_hw_t* hw, int sm);

// clear TX FIFO (executes OUT/PULL instruction; state machine should not be enabled)
void PioTxFifoClear(int pio, int sm);
void PioTxFifoClear_hw(pio_hw_t* hw, int sm);

// Load program into PIO memory
//  pio ... PIO number
//  program ... array of program instructions
//  len ... length of program in number of instrucions
//  off ... offset in PIO memory (program is wrapped to 32 instructions)
// Jump instructions are auto-reallocated to new offset address (program is always compiled from offset 0).
void PioLoadProg(int pio, const u16* program, int len, int off);

// fill PIO program by NOP instructions (= instruction MOV Y,Y)
//  pio ... PIO number
//  len ... length of program, number of instructions
//  off ... offset in PIO memory (program is wrapped to 32 instructions)
void PioNopProg(int pio, int len, int off);

// clear PIO program by "JMP 0" instructions (= default reset value 0x0000)
//  pio ... PIO number
//  len ... length of program, number of instrucions
//  off ... offset in PIO memory (program is wrapped to 32 instructions)
void PioClearProg(int pio, int len, int off);

// set output value of pin controlled by the PIO (after initialization with PioSetupGPIO, but before running state machine)
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  pin ... first pin index 0..29
//  count ... number of pins 1..30
//  val ... pin output value 0 or 1
// This is done by executing SET instruction on "victim" state machine (state machine should not be enabled).
void PioSetPin(int pio, int sm, int pin, int count, int val);
void PioSetPin_hw(pio_hw_t* hw, int sm, int pin, int count, int val);

// set direction of pin controlled by the PIO (after initialization with PioSetupGPIO, but before running state machine)
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  pin ... first pin index 0..29
//  count ... number of pins 1..30
//  dir ... pin direction, 1=output, 0=input
// This is done by executing SET instruction on "victim" state machine (state machine should not be enabled).
void PioSetPinDir(int pio, int sm, int pin, int count, int dir);
void PioSetPinDir_hw(pio_hw_t* hw, int sm, int pin, int count, int dir);

// set current program address of PIO state machine
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  addr ... program address 0 to 31
// This is done by executing JMP instruction on "victim" state machine (state machine should not be enabled).
void PioSetAddr(int pio, int sm, int addr);
void PioSetAddr_hw(pio_hw_t* hw, int sm, int addr);

// reset debug flags of PIO state machine
//  pio ... PIO number
//  sm ... state machine 0 to 3
INLINE void PioDebugClear(int pio, int sm) { *PIO_FDEBUG(pio) = (B24+B16+B8+B0) << sm; }
INLINE void PioDebugClear_hw(pio_hw_t* hw, int sm) { hw->fdebug = (B24+B16+B8+B0) << sm; }

// check if IRQ is set
//  pio ... PIO number
//  irq ... IRQ index 0 to 7
INLINE Bool PioIrqIsSet(int pio, int irq) { return (*PIO_IRQ(pio) & BIT(irq)) != 0; }
INLINE Bool PioIrqIsSet_hw(const pio_hw_t* hw, int irq) { return (hw->irq & BIT(irq)) != 0; }

// clear IRQ request
//  pio ... PIO number
//  irq ... IRQ index 0 to 7
INLINE void PioIrqClear(int pio, int irq) { *PIO_IRQ(pio) = BIT(irq); }
INLINE void PioIrqClear_hw(pio_hw_t* hw, int irq) { hw->irq = BIT(irq); }

// clear IRQ requests by mask
//  pio ... PIO number
//  irq_mask ... IRQ bits B0 to B7
INLINE void PioIrqClearMask(int pio, int irq_mask) { *PIO_IRQ(pio) = irq_mask; }
INLINE void PioIrqClearMask_hw(pio_hw_t* hw, int irq_mask) { hw->irq = irq_mask; }

// force IRQ request
//  pio ... PIO number
//  irq ... IRQ index 0 to 7
INLINE void PioIrqForce(int pio, int irq) { *PIO_IRQFORCE(pio) = BIT(irq); }
INLINE void PioIrqForce_hw(pio_hw_t* hw, int irq) { hw->irq_force = BIT(irq); }

// force IRQ requests by mask
//  pio ... PIO number
//  irq_mask ... IRQ bits B0 to B7
INLINE void PioIrqForceMask(int pio, int irq_mask) { *PIO_IRQFORCE(pio) = irq_mask; }
INLINE void PioIrqForceMask_hw(pio_hw_t* hw, int irq_mask) { hw->irq_force = irq_mask; }

// switch GPIO synchronizer OFF, synchronizer will be bypassed (used for high speed inputs)
//  pio ... PIO number
//  pin ... GPIO pin 0 to 31
INLINE void PioInBypass(int pio, int pin) { RegSet(PIO_INPUTSYNC(pio), BIT(pin)); }
INLINE void PioInBypass_hw(pio_hw_t* hw, int pin) { RegSet(&hw->input_sync_bypass, BIT(pin)); }

// switch GPIO synchronizer OFF by mask, synchronizer will be bypassed (used for high speed inputs)
//  pio ... PIO number
//  pin_mask ... GPIO pin mask of bits B0 to B31
INLINE void PioInBypassMask(int pio, u32 pin_mask) { RegSet(PIO_INPUTSYNC(pio), pin_mask); }
INLINE void PioInBypassMask_hw(pio_hw_t* hw, u32 pin_mask) { RegSet(&hw->input_sync_bypass, pin_mask); }

// switch GPIO synchronizer ON, input will be synchronized (default state)
//  pio ... PIO number
//  pin ... GPIO pin 0 to 31
INLINE void PioInSync(int pio, int pin) { RegClr(PIO_INPUTSYNC(pio), BIT(pin)); }
INLINE void PioInSync_hw(pio_hw_t* hw, int pin) { RegClr(&hw->input_sync_bypass, BIT(pin)); }

// switch GPIO synchronizer ON by mask, input will be synchronized (default state)
//  pio ... PIO number
//  pin_mask ... GPIO pin mask of bits B0 to B31
INLINE void PioInSyncMask(int pio, u32 pin_mask) { RegClr(PIO_INPUTSYNC(pio), pin_mask); }
INLINE void PioInSyncMask_hw(pio_hw_t* hw, u32 pin_mask) { RegClr(&hw->input_sync_bypass, pin_mask); }

// read current values currently driving from PIO to GPIOs outputs
INLINE u32 PioGetPadOut(int pio) { return *PIO_DBG_PADOUT(pio); }
INLINE u32 PioGetPadOut_hw(const pio_hw_t* hw) { return hw->dbg_padout; }

// read current values currently driving from PIO to GPIOs output enables (direction)
INLINE u32 PioGetPadOE(int pio) { return *PIO_DBG_PADOE(pio); }
INLINE u32 PioGetPadOE_hw(const pio_hw_t* hw) { return hw->dbg_padoe; }

// get size of instruction memory in number of instructions (typically 32)
INLINE u8 PioMemSize(int pio) { return (u8)((*PIO_DBG_CFGINFO(pio) >> 16) & 0x3f); }
INLINE u8 PioMemSize_hw(const pio_hw_t* hw) { return (u8)((hw->dbg_cfginfo >> 16) & 0x3f); }

// get number of state machines of one PIO (typically 4)
INLINE u8 PioSMCount(int pio) { return (u8)((*PIO_DBG_CFGINFO(pio) >> 8) & 0x0f); }
INLINE u8 PioSMCount_hw(const pio_hw_t* hw) { return (u8)((hw->dbg_cfginfo >> 8) & 0x0f); }

// get depth of one FIFO in number of words (typically 4)
INLINE u8 PioFifoDepth(int pio) { return (u8)(*PIO_DBG_CFGINFO(pio) & 0x3f); }
INLINE u8 PioFifoDepth_hw(const pio_hw_t* hw) { return (u8)(hw->dbg_cfginfo & 0x3f); }

// get raw unmasked interrupt state (returns 1 if raw interrupt is set)
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  type ... interrupt type PIO_INT_*
INLINE u8 PioIntRaw(int pio, int sm, int type) { return (u8)((*PIO_INTR(pio) >> (type*4 + sm)) & 1); }
INLINE u8 PioIntRaw_hw(const pio_hw_t* hw, int sm, int type) { return (u8)((hw->intr >> (type*4 + sm)) & 1); }

// enable PIO interrupt
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  irq ... IRQ 0 or 1
//  type ... interrupt type PIO_INT_*
INLINE void PioIntEnable(int pio, int sm, int irq, int type) { RegSet(PIO_INTE(pio, irq), BIT(type*4 + sm)); }
INLINE void PioIntEnable0_hw(pio_hw_t* hw, int sm, int type) { RegSet(&hw->inte0, BIT(type*4 + sm)); }
INLINE void PioIntEnable1_hw(pio_hw_t* hw, int sm, int type) { RegSet(&hw->inte1, BIT(type*4 + sm)); }

// disable PIO interrupt (default state)
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  irq ... IRQ 0 or 1
//  type ... interrupt type PIO_INT_*
INLINE void PioIntDisable(int pio, int sm, int irq, int type) { RegClr(PIO_INTE(pio, irq), BIT(type*4 + sm)); }
INLINE void PioIntDisable0_hw(pio_hw_t* hw, int sm, int type) { RegClr(&hw->inte0, BIT(type*4 + sm)); }
INLINE void PioIntDisable1_hw(pio_hw_t* hw, int sm, int type) { RegClr(&hw->inte1, BIT(type*4 + sm)); }

// force PIO interrupt
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  irq ... IRQ 0 or 1
//  type ... interrupt type PIO_INT_*
INLINE void PioIntForce(int pio, int sm, int irq, int type) { RegSet(PIO_INTF(pio, irq), BIT(type*4 + sm)); }
INLINE void PioIntForce0_hw(pio_hw_t* hw, int sm, int type) { RegSet(&hw->intf0, BIT(type*4 + sm)); }
INLINE void PioIntForce1_hw(pio_hw_t* hw, int sm, int type) { RegSet(&hw->intf1, BIT(type*4 + sm)); }

// unforce PIO interrupt (default state)
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  irq ... IRQ 0 or 1
//  type ... interrupt type PIO_INT_*
INLINE void PioIntUnforce(int pio, int sm, int irq, int type) { RegClr(PIO_INTF(pio, irq), BIT(type*4 + sm)); }
INLINE void PioIntUnforce0_hw(pio_hw_t* hw, int sm, int type) { RegClr(&hw->intf0, BIT(type*4 + sm)); }
INLINE void PioIntUnforce1_hw(pio_hw_t* hw, int sm, int type) { RegClr(&hw->intf1, BIT(type*4 + sm)); }

// get interrupt state (returns 1 if interrupt is set)
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  irq ... IRQ 0 or 1
//  type ... interrupt type PIO_INT_*
INLINE u8 PioIntStatus(int pio, int sm, int irq, int type) { return (u8)((*PIO_INTS(pio, irq) >> (type*4 + sm)) & 1); }
INLINE u8 PioIntStatus0_hw(const pio_hw_t* hw, int sm, int type) { return (u8)((hw->ints0 >> (type*4 + sm)) & 1); }
INLINE u8 PioIntStatus1_hw(const pio_hw_t* hw, int sm, int type) { return (u8)((hw->ints1 >> (type*4 + sm)) & 1); }

// initialize PIO state machine, prepare default state
//  pio ... PIO number
//  sm ... state machine 0 to 3
void PioSMInit(int pio, int sm);

// initialite PIO state machine using configuration
//  pio ... PIO number
//  sm ... state machine 0 to 3
//  addr ... initial PC address
//  cfg ... configuration (NULL = use default configuration)
void PioSMInitCfg(int pio, int sm, int addr, const pio_sm_config* cfg);

// initialize PIO with all state machines
//  pio ... PIO number
void PioInit(int pio);

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

typedef pio_hw_t* PIO;

// program definition (result of compilation)
typedef struct pio_program {
	const u16*	instructions;
	u8		length;
	s8		origin; // required instruction memory origin or -1
} __packed pio_program_t;

// Set the 'out' pins in a state machine configuration
INLINE void sm_config_set_out_pins(pio_sm_config *c, uint out_base, uint out_count)
	{ PioCfgOut(c, out_base, out_count); }

// Set the 'set' pins in a state machine configuration
INLINE void sm_config_set_set_pins(pio_sm_config *c, uint set_base, uint set_count)
	{ PioCfgSet(c, set_base, set_count); }

// Set the 'in' pins in a state machine configuration
INLINE void sm_config_set_in_pins(pio_sm_config *c, uint in_base)
	{ PioCfgIn(c, in_base); }

// Set the 'sideset' pins in a state machine configuration
INLINE void sm_config_set_sideset_pins(pio_sm_config *c, uint sideset_base)
	{ c->pinctrl = (c->pinctrl & ~(B10+B11+B12+B13+B14)) | ((u32)sideset_base << 10); }

// Set the 'sideset' options in a state machine configuration
INLINE void sm_config_set_sideset(pio_sm_config *c, uint bit_count, bool optional, bool pindirs)
{
	c->pinctrl = (c->pinctrl & ~(B29+B30+B31)) | ((u32)bit_count << 29);
	c->execctrl = (c->execctrl & ~(B29+B30)) | (optional ? B30 : 0) | (pindirs ? B29 : 0);
}

// Set the state machine clock divider (from integer and fractional parts - 16:8) in a state machine configuration
INLINE void sm_config_set_clkdiv_int_frac(pio_sm_config *c, u16 div_int, u8 div_frac)
	{ c->clkdiv = (((uint)div_frac) << 8) | (((uint)div_int) << 16); }

INLINE void pio_calculate_clkdiv_from_float(float div, u16* div_int, u8* div_frac)
{
	u32 k = (u32)(div*256 + 0.5f);
	if (k < 0x100) k = 0;
	*div_int = (u16)(k >> 8);
        *div_frac = (u8)k;
}

// Set the state machine clock divider (from a floating point value) in a state machine configuration
INLINE void sm_config_set_clkdiv(pio_sm_config *c, float div) { PioCfgClkdivFloat(c, div); }

// Set the wrap addresses in a state machine configuration
INLINE void sm_config_set_wrap(pio_sm_config *c, uint wrap_target, uint wrap)
	{ PioCfgWrap(c, wrap_target, wrap); }

// Set the 'jmp' pin in a state machine configuration
INLINE void sm_config_set_jmp_pin(pio_sm_config *c, uint pin) { PioCfgJmpPin(c, pin); }

// Setup 'in' shifting parameters in a state machine configuration
INLINE void sm_config_set_in_shift(pio_sm_config *c, bool shift_right, bool autopush, uint push_threshold)
	{ PioCfgInShift(c, shift_right, autopush, push_threshold); }

// Setup 'out' shifting parameters in a state machine configuration
INLINE void sm_config_set_out_shift(pio_sm_config *c, bool shift_right, bool autopull, uint pull_threshold)
	{ PioCfgOutShift(c, shift_right, autopull, pull_threshold); }

// PIO FIFO joining type
#undef PIO_FIFO_JOIN_NONE
#undef PIO_FIFO_JOIN_TX
#undef PIO_FIFO_JOIN_RX

enum pio_fifo_join {
	PIO_FIFO_JOIN_NONE = 0,
	PIO_FIFO_JOIN_TX = 1,
	PIO_FIFO_JOIN_RX = 2,
};

// Setup the FIFO joining in a state machine configuration
INLINE void sm_config_set_fifo_join(pio_sm_config *c, enum pio_fifo_join join) { PioCfgFifoJoin(c, join); }

// Set special 'out' operations in a state machine configuration
INLINE void sm_config_set_out_special(pio_sm_config *c, bool sticky, bool has_enable_pin, uint enable_pin_index)
	{ PioCfgOutSpecial(c, sticky, has_enable_pin, enable_pin_index); }

enum pio_mov_status_type
{
	STATUS_TX_LESSTHAN = 0,
	STATUS_RX_LESSTHAN = 1
};

// Set source for 'mov status' in a state machine configuration
INLINE void sm_config_set_mov_status(pio_sm_config *c, enum pio_mov_status_type status_sel, uint status_n)
	{ PioCfgMovStatus(c, status_sel != STATUS_TX_LESSTHAN, status_n); }

// Get the default state machine configuration
INLINE pio_sm_config pio_get_default_sm_config(void)
{
	pio_sm_config c = PioCfgDef();
	PioCfgSet(&c, 0, 0); // default sideset count was 5, but original-SDK uses 0
	return c;
}

// Apply a state machine configuration to a state machine
INLINE void pio_sm_set_config(PIO pio, uint sm, const pio_sm_config *config) { PioCfg_hw(pio, sm, config); }

// Return the instance number of a PIO instance
INLINE uint pio_get_index(PIO pio) { return PioGetInx(pio); }

// Setup the function select for a GPIO to use output from the given PIO instance
INLINE void pio_gpio_init(PIO pio, uint pin) { PioSetupGPIO(PioGetInx(pio), pin, 1); }

// Return the DREQ to use for pacing transfers to/from a particular state machine FIFO
INLINE uint pio_get_dreq(PIO pio, uint sm, bool is_tx) { return PioGetDreq(PioGetInx(pio), sm, is_tx); }

// find offset for program (-1 = not found)
INLINE int _pio_find_offset_for_program(PIO pio, const pio_program_t *program)
{
	u8 p = PioGetInx(pio);
	u8 length = program->length;
	s8 origin = program->origin;
	if (origin >= 0)
		return PioIsFreeProg(p, origin, length) ? origin : -1;
	else
		return PioFindFreeProg(p, length);
}

// Determine whether the given program can (at the time of the call) be loaded onto the PIO instance
INLINE bool pio_can_add_program(PIO pio, const pio_program_t *program)
	{ return _pio_find_offset_for_program(pio, program) >= 0; }

// Determine whether the given program can (at the time of the call) be loaded onto the PIO instance starting at a particular location
INLINE bool pio_can_add_program_at_offset(PIO pio, const pio_program_t *program, uint offset)
	{ return PioIsFreeProg(PioGetInx(pio), offset, program->length); }

// add program at offset
INLINE void _pio_add_program_at_offset(PIO pio, const pio_program_t *program, uint offset)
	{ PioLoadProg(PioGetInx(pio), program->instructions, program->length, offset); }

// Attempt to load the program, panicking if not possible (returns -1 if cannot load)
INLINE uint pio_add_program(PIO pio, const pio_program_t *program)
{
	int offset = _pio_find_offset_for_program(pio, program);
	if (offset < 0) return -1;
	_pio_add_program_at_offset(pio, program, (uint)offset);
	return offset;
}

// Attempt to load the program at the specified instruction memory offset, panicking if not possible
INLINE void pio_add_program_at_offset(PIO pio, const pio_program_t *program, uint offset)
	{ _pio_add_program_at_offset(pio, program, offset); }

// Remove a program from a PIO instance's instruction memory
INLINE void pio_remove_program(PIO pio, const pio_program_t *program, uint loaded_offset)
	{ PioClearProg(PioGetInx(pio), program->length, loaded_offset); }

// Clears all of a PIO instance's instruction memory
INLINE void pio_clear_instruction_memory(PIO pio)
	{ PioClearProg(PioGetInx(pio), PIO_MEM_NUM, 0); }

// Resets the state machine to a consistent state, and configures it
INLINE void pio_sm_init(PIO pio, uint sm, uint initial_pc, const pio_sm_config *config)
	{ PioSMInitCfg(PioGetInx(pio), sm, initial_pc, config); }

// Enable or disable a PIO state machine
INLINE void pio_sm_set_enabled(PIO pio, uint sm, bool enabled)
	{ if (enabled) PioSMEnable_hw(pio, sm); else PioSMDisable_hw(pio, sm); }

// Enable or disable multiple PIO state machines
INLINE void pio_set_sm_mask_enabled(PIO pio, u32 mask, bool enabled)
	{ if (enabled) PioSMEnableMask_hw(pio, mask); else PioSMDisableMask_hw(pio, mask); }

// Restart a state machine with a known state
INLINE void pio_sm_restart(PIO pio, uint sm) { PioSMRestart_hw(pio, sm); }

// Restart multiple state machine with a known state
INLINE  void pio_restart_sm_mask(PIO pio, u32 mask) { PioSMRestartMask_hw(pio, mask); }

// Restart a state machine's clock divider from a phase of 0
INLINE void pio_sm_clkdiv_restart(PIO pio, uint sm) { PioClkdivRestart_hw(pio, sm); }

// Restart multiple state machines' clock dividers from a phase of 0.
INLINE void pio_clkdiv_restart_sm_mask(PIO pio, u32 mask) { PioClkdivRestartMask_hw(pio, mask); }

// Enable multiple PIO state machines synchronizing their clock dividers
INLINE void pio_enable_sm_mask_in_sync(PIO pio, u32 mask) { PioSMEnableMaskSync_hw(pio, mask); }

// PIO interrupt source numbers for pio related IRQs
enum pio_interrupt_source {
	pis_interrupt0 = PIO_INTR_SM0_LSB,
	pis_interrupt1 = PIO_INTR_SM1_LSB,
	pis_interrupt2 = PIO_INTR_SM2_LSB,
	pis_interrupt3 = PIO_INTR_SM3_LSB,
	pis_sm0_tx_fifo_not_full = PIO_INTR_SM0_TXNFULL_LSB,
	pis_sm1_tx_fifo_not_full = PIO_INTR_SM1_TXNFULL_LSB,
	pis_sm2_tx_fifo_not_full = PIO_INTR_SM2_TXNFULL_LSB,
	pis_sm3_tx_fifo_not_full = PIO_INTR_SM3_TXNFULL_LSB,
	pis_sm0_rx_fifo_not_empty = PIO_INTR_SM0_RXNEMPTY_LSB,
	pis_sm1_rx_fifo_not_empty = PIO_INTR_SM1_RXNEMPTY_LSB,
	pis_sm2_rx_fifo_not_empty = PIO_INTR_SM2_RXNEMPTY_LSB,
	pis_sm3_rx_fifo_not_empty = PIO_INTR_SM3_RXNEMPTY_LSB,
};

// Enable/Disable a single source on a PIO's IRQ 0
INLINE void pio_set_irq0_source_enabled(PIO pio, enum pio_interrupt_source source, bool enabled)
{
	if (enabled)
		hw_set_bits(&pio->inte0, BIT(source));
	else
		hw_clear_bits(&pio->inte0, BIT(source));
}

// Enable/Disable a single source on a PIO's IRQ 1
INLINE void pio_set_irq1_source_enabled(PIO pio, enum pio_interrupt_source source, bool enabled)
{
	if (enabled)
		hw_set_bits(&pio->inte1, BIT(source));
	else
		hw_clear_bits(&pio->inte1, BIT(source));
}

// Enable/Disable multiple sources on a PIO's IRQ 0
INLINE void pio_set_irq0_source_mask_enabled(PIO pio, u32 source_mask, bool enabled)
{
	if (enabled)
		hw_set_bits(&pio->inte0, source_mask);
	else
		hw_clear_bits(&pio->inte0, source_mask);
}

// Enable/Disable multiple sources on a PIO's IRQ 1
INLINE void pio_set_irq1_source_mask_enabled(PIO pio, u32 source_mask, bool enabled)
{
	if (enabled)
		hw_set_bits(&pio->inte1, source_mask);
	else
		hw_clear_bits(&pio->inte1, source_mask);
}

// Enable/Disable a single source on a PIO's specified (0/1) IRQ index
INLINE void pio_set_irqn_source_enabled(PIO pio, uint irq_index, enum pio_interrupt_source source, bool enabled)
{
	if (irq_index != 0)
		pio_set_irq1_source_enabled(pio, source, enabled);
	else
		pio_set_irq0_source_enabled(pio, source, enabled);
}

// Enable/Disable multiple sources on a PIO's specified (0/1) IRQ index
INLINE void pio_set_irqn_source_mask_enabled(PIO pio, uint irq_index, u32 source_mask, bool enabled)
{
	if (irq_index)
		pio_set_irq1_source_mask_enabled(pio, source_mask, enabled);
	else
		pio_set_irq0_source_mask_enabled(pio, source_mask, enabled);
}

// Determine if a particular PIO interrupt is set
INLINE bool pio_interrupt_get(PIO pio, uint pio_interrupt_num) { return PioIrqIsSet_hw(pio, pio_interrupt_num); }

// Clear a particular PIO interrupt
INLINE void pio_interrupt_clear(PIO pio, uint pio_interrupt_num) { PioIrqClear_hw(pio, pio_interrupt_num); }

// Return the current program counter for a state machine
INLINE u8 pio_sm_get_pc(PIO pio, uint sm) { return PioGetPC_hw(pio, sm); }

// Immediately execute an instruction on a state machine
INLINE void pio_sm_exec(PIO pio, uint sm, uint instr) { PioExec_hw(pio, sm, (u16)instr); }

// Determine if an instruction set by pio_sm_exec() is stalled executing
INLINE bool pio_sm_is_exec_stalled(PIO pio, uint sm) { return PioIsExec_hw(pio, sm); }

// Immediately execute an instruction on a state machine and wait for it to complete
INLINE void pio_sm_exec_wait_blocking(PIO pio, uint sm, uint instr) { PioExecWait_hw(pio, sm, (u16)instr); }

// Set the current wrap configuration for a state machine
INLINE void pio_sm_set_wrap(PIO pio, uint sm, uint wrap_target, uint wrap)
	{ PioSetWrap_hw(pio, sm, wrap_target, wrap); }

// Set the current 'out' pins for a state machine
INLINE void pio_sm_set_out_pins(PIO pio, uint sm, uint out_base, uint out_count)
	{ PioSetupOut_hw(pio, sm, out_base, out_count); }

// Set the current 'set' pins for a state machine
INLINE void pio_sm_set_set_pins(PIO pio, uint sm, uint set_base, uint set_count)
	{ PioSetupSet_hw(pio, sm, set_base, set_count); }

// Set the current 'in' pins for a state machine
INLINE void pio_sm_set_in_pins(PIO pio, uint sm, uint in_base)
	{ PioSetupIn_hw(pio, sm, in_base); }

// Set the current 'sideset' pins for a state machine
INLINE void pio_sm_set_sideset_pins(PIO pio, uint sm, uint sideset_base)
	{ RegMask(&pio->sm[sm].pinctrl, (u32)sideset_base << 10, B10+B11+B12+B13+B14); }

// Write a word of data to a state machine's TX FIFO
INLINE void pio_sm_put(PIO pio, uint sm, u32 data) { PioWrite_hw(pio, sm, data); }

// Read a word of data from a state machine's RX FIFO
INLINE u32 pio_sm_get(PIO pio, uint sm) { return PioRead_hw(pio, sm); }

// Determine if a state machine's RX FIFO is full
INLINE bool pio_sm_is_rx_fifo_full(PIO pio, uint sm) { return PioRxIsFull_hw(pio, sm); }

// Determine if a state machine's RX FIFO is empty
INLINE bool pio_sm_is_rx_fifo_empty(PIO pio, uint sm) { return PioRxIsEmpty_hw(pio, sm); }

// Return the number of elements currently in a state machine's RX FIFO
INLINE uint pio_sm_get_rx_fifo_level(PIO pio, uint sm) { return PioRxLevel_hw(pio, sm); }

// Determine if a state machine's TX FIFO is full
INLINE bool pio_sm_is_tx_fifo_full(PIO pio, uint sm) { return PioTxIsFull_hw(pio, sm); }

// Determine if a state machine's TX FIFO is empty
INLINE bool pio_sm_is_tx_fifo_empty(PIO pio, uint sm) { return PioTxIsEmpty_hw(pio, sm); }

// Return the number of elements currently in a state machine's TX FIFO
INLINE uint pio_sm_get_tx_fifo_level(PIO pio, uint sm) { return PioTxLevel_hw(pio, sm); }

// Write a word of data to a state machine's TX FIFO, blocking if the FIFO is full
INLINE void pio_sm_put_blocking(PIO pio, uint sm, u32 data) { PioWriteWait_hw(pio, sm, data); }

// Read a word of data from a state machine's RX FIFO, blocking if the FIFO is empty
INLINE u32 pio_sm_get_blocking(PIO pio, uint sm) { return PioReadWait_hw(pio, sm); }

// Empty out a state machine's TX FIFO
INLINE void pio_sm_drain_tx_fifo(PIO pio, uint sm) { PioTxFifoClear_hw(pio, sm); }

// set the current clock divider for a state machine using a 16:8 fraction
INLINE void pio_sm_set_clkdiv_int_frac(PIO pio, uint sm, u16 div_int, u8 div_frac)
	{ pio->sm[sm].clkdiv = (((uint)div_frac) << 8) | (((uint)div_int) << 16); }

// set the current clock divider for a state machine
INLINE void pio_sm_set_clkdiv(PIO pio, uint sm, float div) { PioSetClkdivFloat_hw(pio, sm, div); }

// Clear a state machine's TX and RX FIFOs
INLINE void pio_sm_clear_fifos(PIO pio, uint sm) { PioFifoClear_hw(pio, sm); }

// Use a state machine to set a value on all pins for the PIO instance
INLINE void pio_sm_set_pins(PIO pio, uint sm, u32 pin_values)
{
	u8 pin;
	for (pin = 0; pin < 30; pin++) PioSetPin_hw(pio, sm, pin, 1, (pin_values >> pin) & 1);
}

// Use a state machine to set a value on multiple pins for the PIO instance
INLINE void pio_sm_set_pins_with_mask(PIO pio, uint sm, u32 pin_values, u32 pin_mask)
{
	u8 pin;
	for (pin = 0; pin < 30; pin++)
		if (((pin_mask >> pin) & 1) != 0)
			PioSetPin_hw(pio, sm, pin, 1, (pin_values >> pin) & 1);
}

// Use a state machine to set the pin directions for multiple pins for the PIO instance
INLINE void pio_sm_set_pindirs_with_mask(PIO pio, uint sm, u32 pin_dirs, u32 pin_mask)
{
	u8 pin;
	for (pin = 0; pin < 30; pin++)
		if (((pin_mask >> pin) & 1) != 0)
			PioSetPinDir_hw(pio, sm, pin, 1, (pin_dirs >> pin) & 1);
}

// Use a state machine to set the same pin direction for multiple consecutive pins for the PIO instance
INLINE void pio_sm_set_consecutive_pindirs(PIO pio, uint sm, uint pin_base, uint pin_count, bool is_out)
	{ PioSetPinDir_hw(pio, sm, pin_base, pin_count, is_out ? 1 : 0); }

// Mark a state machine as used
INLINE void pio_sm_claim(PIO pio, uint sm) { PioClaim(PioGetInx(pio), sm); }

// Mark multiple state machines as used
INLINE void pio_claim_sm_mask(PIO pio, uint sm_mask) { PioClaimMask(PioGetInx(pio), sm_mask); }

// Mark a state machine as no longer used
INLINE void pio_sm_unclaim(PIO pio, uint sm) { PioUnclaim(PioGetInx(pio), sm); }

// Claim a free state machine on a PIO instance
INLINE int pio_claim_unused_sm(PIO pio, bool required) { return PioClaimFree(PioGetInx(pio)); }

// Determine if a PIO state machine is claimed
INLINE bool pio_sm_is_claimed(PIO pio, uint sm) { return PioIsClaimed(PioGetInx(pio), sm); }

// === encode instructions

// enumerate instruction code
enum pio_instr_bits
{
	pio_instr_bits_jmp	= 0x0000,
	pio_instr_bits_wait	= 0x2000,
	pio_instr_bits_in	= 0x4000,
	pio_instr_bits_out	= 0x6000,
	pio_instr_bits_push	= 0x8000,
	pio_instr_bits_pull	= 0x8080,
	pio_instr_bits_mov	= 0xa000,
	pio_instr_bits_irq	= 0xc000,
	pio_instr_bits_set	= 0xe000,
};

// Enumeration of values to pass for source/destination args for instruction encoding functions
enum pio_src_dest
{
	pio_pins	= 0,
	pio_x		= 1,
	pio_y		= 2,
	pio_null	= 3,
	pio_pindirs	= 4,
	pio_exec_mov	= 4,
	pio_status	= 5,
	pio_pc		= 5,
	pio_isr		= 6,
	pio_osr		= 7,
	pio_exec_out	= 7,
};

// get instruction major code
INLINE uint _pio_major_instr_bits(uint instr) { return instr & 0xe000; }

// encode instruction with arguments
INLINE uint _pio_encode_instr_and_args(enum pio_instr_bits instr_bits, uint arg1, uint arg2)
	{ return instr_bits | (arg1 << 5) | (arg2 & 0x1f); }

// encode instruction with source and destiaton
INLINE uint _pio_encode_instr_and_src_dest(enum pio_instr_bits instr_bits, enum pio_src_dest dest, uint value)
	{ return _pio_encode_instr_and_args(instr_bits, dest & 7, value); }

// Encode just the delay slot bits of an instruction
INLINE uint pio_encode_delay(uint cycles) { return cycles << 8; }

// Encode just the side set bits of an instruction (in non optional side set mode)
INLINE uint pio_encode_sideset(uint sideset_bit_count, uint value) { return value << (13 - sideset_bit_count); }

// Encode just the side set bits of an instruction (in optional -`opt` side set mode)
INLINE uint pio_encode_sideset_opt(uint sideset_bit_count, uint value)
	{ return 0x1000 | value << (12 - sideset_bit_count); }

// Encode an unconditional JMP instruction
INLINE uint pio_encode_jmp(uint addr) { return _pio_encode_instr_and_args(pio_instr_bits_jmp, 0, addr); }

// Encode a conditional JMP if scratch X zero instruction
INLINE uint pio_encode_jmp_not_x(uint addr) { return _pio_encode_instr_and_args(pio_instr_bits_jmp, 1, addr); }

// Encode a conditional JMP if scratch X non-zero (and post-decrement X) instruction
INLINE uint pio_encode_jmp_x_dec(uint addr) { return _pio_encode_instr_and_args(pio_instr_bits_jmp, 2, addr); }

// Encode a conditional JMP if scratch Y zero instruction
INLINE uint pio_encode_jmp_not_y(uint addr) { return _pio_encode_instr_and_args(pio_instr_bits_jmp, 3, addr); }

// Encode a conditional JMP if scratch Y non-zero (and post-decrement Y) instruction
INLINE uint pio_encode_jmp_y_dec(uint addr) { return _pio_encode_instr_and_args(pio_instr_bits_jmp, 4, addr); }

// Encode a conditional JMP if scratch X not equal scratch Y instruction
INLINE uint pio_encode_jmp_x_ne_y(uint addr) { return _pio_encode_instr_and_args(pio_instr_bits_jmp, 5, addr); }

// Encode a conditional JMP if input pin high instruction
INLINE uint pio_encode_jmp_pin(uint addr) { return _pio_encode_instr_and_args(pio_instr_bits_jmp, 6, addr); }

// Encode a conditional JMP if output shift register not empty instruction
INLINE uint pio_encode_jmp_not_osre(uint addr) { return _pio_encode_instr_and_args(pio_instr_bits_jmp, 7, addr); }

// encode IRQ
INLINE uint _pio_encode_irq(bool relative, uint irq) { return (relative ? 0x10 : 0) | irq; }

// Encode a WAIT for GPIO pin instruction
INLINE uint pio_encode_wait_gpio(bool polarity, uint gpio)
	{ return _pio_encode_instr_and_args(pio_instr_bits_wait, 0 | (polarity ? 4 : 0), gpio); }

// Encode a WAIT for pin instruction
INLINE uint pio_encode_wait_pin(bool polarity, uint pin)
	{ return _pio_encode_instr_and_args(pio_instr_bits_wait, 1 | (polarity ? 4 : 0), pin); }

// Encode a WAIT for IRQ instruction
INLINE uint pio_encode_wait_irq(bool polarity, bool relative, uint irq)
	{ return _pio_encode_instr_and_args(pio_instr_bits_wait, 2 | (polarity ? 4 : 0), _pio_encode_irq(relative, irq)); }

// Encode an IN instruction
INLINE uint pio_encode_in(enum pio_src_dest src, uint count)
	{ return _pio_encode_instr_and_src_dest(pio_instr_bits_in, src, count); }

// Encode an OUT instruction
INLINE uint pio_encode_out(enum pio_src_dest dest, uint count)
	{ return _pio_encode_instr_and_src_dest(pio_instr_bits_out, dest, count); }

// Encode a PUSH instruction
INLINE uint pio_encode_push(bool if_full, bool block)
	{ return _pio_encode_instr_and_args(pio_instr_bits_push, (if_full ? 2 : 0) | (block ? 1 : 0), 0); }

// Encode a PULL instruction
INLINE uint pio_encode_pull(bool if_empty, bool block)
	{ return _pio_encode_instr_and_args(pio_instr_bits_pull, (if_empty ? 2 : 0) | (block ? 1 : 0), 0); }

// Encode a MOV instruction
INLINE uint pio_encode_mov(enum pio_src_dest dest, enum pio_src_dest src)
	{ return _pio_encode_instr_and_src_dest(pio_instr_bits_mov, dest, src & 7); }

// Encode a MOV instruction with bit invert
INLINE uint pio_encode_mov_not(enum pio_src_dest dest, enum pio_src_dest src)
	{ return _pio_encode_instr_and_src_dest(pio_instr_bits_mov, dest, (1 << 3) | (src & 7)); }

// Encode a MOV instruction with bit reverse
INLINE uint pio_encode_mov_reverse(enum pio_src_dest dest, enum pio_src_dest src)
	{ return _pio_encode_instr_and_src_dest(pio_instr_bits_mov, dest, (2 << 3) | (src & 7)); }

// Encode a IRQ SET instruction
INLINE uint pio_encode_irq_set(bool relative, uint irq)
	{ return _pio_encode_instr_and_args(pio_instr_bits_irq, 0, _pio_encode_irq(relative, irq)); }

// Encode a IRQ WAIT instruction
INLINE uint pio_encode_irq_wait(bool relative, uint irq)
	{ return _pio_encode_instr_and_args(pio_instr_bits_irq, 1, _pio_encode_irq(relative, irq)); }

// Encode a IRQ CLEAR instruction
INLINE uint pio_encode_irq_clear(bool relative, uint irq)
	{ return _pio_encode_instr_and_args(pio_instr_bits_irq, 2, _pio_encode_irq(relative, irq)); }

// Encode a SET instruction
INLINE uint pio_encode_set(enum pio_src_dest dest, uint value)
	{ return _pio_encode_instr_and_src_dest(pio_instr_bits_set, dest, value); }

// Encode a NOP instruction
INLINE uint pio_encode_nop(void) { return pio_encode_mov(pio_y, pio_y); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_PIO_H

#endif // USE_PIO	// use PIO (sdk_pio.c, sdk_pio.h)
