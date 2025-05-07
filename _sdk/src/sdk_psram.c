
// ****************************************************************************
//
//                                PSRAM memory
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

u32 PSRAM_Size = 0;		// PSRAM size in bytes (0 = no PSRAM memory) ... base address PSRAM_BASE

#if USE_PSRAM	// use PSRAM Memory (sdk_psram.c, sdk_psram.h)

#include "../inc/sdk_psram.h"
#include "../inc/sdk_cpu.h"
#include "../inc/sdk_gpio.h"
#include "../inc/sdk_clocks.h"
#include "../inc/sdk_timer.h"
#include "../inc/sdk_qmi.h"
#include "../inc/sdk_xip.h"

// PSRAM SPI command codes
#define PSRAM_CMD_QUAD_END	0xF5	// end QSPI mode
#define PSRAM_CMD_QUAD_ENABLE	0x35	// enable QSPI mode
#define PSRAM_CMD_READ_ID	0x9F	// read PSRAM ID
#define PSRAM_CMD_RSTEN		0x66	// reset + enable
#define PSRAM_CMD_RST		0x99	// reset
#define PSRAM_CMD_QUAD_READ	0xEB	// read data in QSPI format
#define PSRAM_CMD_QUAD_WRITE	0x38	// write data in QSPI format
#define PSRAM_CMD_NOOP		0xFF

#define RP2350_PSRAM_ID		0x5D	// ID of PSRAM

// transmit data to PSRAM in direct mode (returns received data)
//	B0..B15: (16 bits) DATA data to transmit, LSB transmitted first
//	B16..B17: (2 bits) IWIDTH interface width 0=SPI, 1=DSPI, 2=QSPI
//	B18: DWIDTH data width 0=8 bit, 1=16 bit
//	B19: OE output enable (active-high) (SPI: SD0 always output, SD1 always input)
//	B20: NOPUSH inhibit RX FIFO push
NOINLINE u32 NOFLASH(PSRAM_Trans)(u32 d)
{
	// transmit data (and setup)
	qmi_hw->direct_tx = d;

	// wait data to be send
	while (!QMI_TxIsEmpty()) {}

	// wait transfer to complete
	QMI_Wait();

	// receive data
	return qmi_hw->direct_rx;
}

// transmit data with CS
NOINLINE u32 NOFLASH(PSRAM_TransCS)(u32 d)
{
	// assert CS1n chip select line
	qmi_hw->direct_csr |= B3; // B3: ASSERT_CS1N assert CS1n chip select line

	// transmit data
	d = PSRAM_Trans(d);

	// de-assert CS1n chip select line
	qmi_hw->direct_csr &= ~B3; // B3: ASSERT_CS1N assert CS1n chip select line

	// short delay
	int i;
	for (i = 100; i > 0; i--) nop();

	return d;
}

// get PSRAM size in bytes (returns 0 on error or if no PSRAM)
int NOFLASH(PSRAM_GetSize)(void)
{
	int i;
	u32 d, oldcsr;
	u8 kgd, eid;
	int ramsize = 0;

	// setup PSRAM CS pin
	GPIO_Fnc(RP2350_PSRAM_CS, GPIO_FUNC_XIP_CS1);

	// disable interrupts
	IRQ_LOCK;

// ---- switch to direct mode

	// enable direct serial mode
	qmi_hw_t* qmi = qmi_hw;
	oldcsr = qmi->direct_csr; // save old setup
	qmi->direct_csr = (30 << 22) | B0; // B22..B29: CLKDIV = 30 (slow access), B0: EN enable direct mode

	// wait if last transfer is busy
	QMI_Wait();

// ---- exit QSPI mode

	// transmit the command to exit QSPI quad mode, to read ID as standard SPI ... use 8-bit transfer
	// B19: OE output enable
	// B16-B17: IWIDTH interface width 0=SPI, 1=DSPI, 2=QSPI
	// 0xF5 ... command to exit QSPI quad mode
	d = PSRAM_TransCS(B19 | (2 << 16) | PSRAM_CMD_QUAD_END);

// ---- read ID

	// assert CS1n chip select line
	qmi->direct_csr |= B3; // B3: ASSERT_CS1N assert CS1n chip select line

	// read ID (use 8-bit transfers in SPI mode)
	d = PSRAM_Trans(PSRAM_CMD_READ_ID);
	d = PSRAM_Trans(PSRAM_CMD_NOOP);
	d = PSRAM_Trans(PSRAM_CMD_NOOP);
	d = PSRAM_Trans(PSRAM_CMD_NOOP);
	d = PSRAM_Trans(PSRAM_CMD_NOOP);
	kgd = (u8)PSRAM_Trans(PSRAM_CMD_NOOP);
	eid = (u8)PSRAM_Trans(PSRAM_CMD_NOOP);

	// de-assert CS1n chip select line, disable direct mode
	oldcsr &= ~(B3 | B0); // B3: ASSERT_CS1N assert CS1n chip select line, B0: EN enable direct mode
	qmi->direct_csr = oldcsr; // return old setup

	// enable interrupts
	IRQ_UNLOCK;

// ---- parse PSRAM ID

	// check valid PSRAM
	if (kgd == RP2350_PSRAM_ID)
	{
		// detect RAM size
		ramsize = 1024 * 1024;	// 1 MB
		i = eid >> 5;
		if ((eid == 0x26) || (i == 2))
			ramsize <<= 3; // 8 MB or more
		else if (i == 0)
			ramsize << 1; // 2 MB
		else if (i == 1)
			ramsize <<= 2; // 4 MB
	}

	PSRAM_Size = ramsize;
	return ramsize;
}

// setup PSRAM timing (should be called after sys_clock change)
void NOFLASH(PSRAM_Timing)(void)
{
	// no PSRAM
	if (PSRAM_Size == 0) return;

	// get system clock in Hz (usually 150000000, minimum 12000000, maximum 600000000)
	u32 sysHz = ClockGetHz(CLK_SYS);

	// calculate clock divider ... result frequency should be always less than max. frequency, so round divider up
	//  minimum: (12000000 + 109000000 - 1) / 109000000 = 1
	//  usually: (150000000 + 109000000 - 1) / 109000000 = 2
	//  maximum: (600000000 + 109000000 - 1) / 109000000 = 6
	u8 clockDivider = (sysHz + RP2350_PSRAM_MAX_SCK_HZ - 1) / RP2350_PSRAM_MAX_SCK_HZ;

	// system clock/128 (to get easier 32-bit integer calculations)
	//  minimum: 12000000/128 = 93750
	//  usually: 150000000/128 = 1171875
	//  maximum: 600000000/128 = 4687500
	sysHz >>= 7;

	// get clock cycle in nanoseconds*128 (round to nearest)
	//  minimum: (1000000000 + 93750/2) / 93750 = 10667
	//  usually: (1000000000 + 1171875/2) / 1171875 = 853
	//  maximum: (1000000000 + 4687500/2) / 4687500 = 213
	int nsPerCycle = (1000000000ul + sysHz/2) / sysHz;

// CS max pulse width in [ns] (= 8 us) - recalculate to nanoseconds*128, and to multiply of 64 clock cycles
//  8000 * 128 / 64 = 16000
#define RP2350_PSRAM_MAX_SELCS_TEMP	(RP2350_PSRAM_MAX_SELCS * 128 / 64)

	// maxSelect time - defined in units of 64 clock cycles (round down)
	//  minimum: 16000 / 10667 = 1
	//  usually: 16000 / 853 = 18
	//  maximum: 16000 / 213 = 75
	u8 maxSelect = RP2350_PSRAM_MAX_SELCS_TEMP / nsPerCycle;

// CS min deselect time in [ns] (= 50 ns) - recalculate to nanoseconds*128
//  50 * 128 = 6400
#define RP2350_PSRAM_MIN_DESEL_TEMP	(RP2350_PSRAM_MIN_DESEL * 128)

	// minDeselect time - defined in system clock cycles (round up)
	//  minimum: (6400 + 10667 - 1) / 10667 = 1
	//  usually: (6400 + 853 - 1) / 853 = 8
	//  maximum: (6400 + 213 - 1) / 213 = 31
	u8 minDeselect = (RP2350_PSRAM_MIN_DESEL_TEMP + nsPerCycle - 1) / nsPerCycle;

	// disable interrupts
	IRQ_LOCK;

	// wait if last transfer is busy
	QMI_Wait();

	// setup PSRAM timing
	qmi_hw->m[1].timing =
		(1 << 30) |		// COOLDOWN (bit 30-31): chip select cooldown period (64*COOLDOWN system clock cycles)
		(2 << 28) |		// PAGEBREAK (bit 28-29): deassert when crossing 1024-byte quad-page boundary
		(3 << 23) |		// SELECT_HOLD (bit 23-24): delay releasing CS for 3 extra system cycles
		(maxSelect << 17) |	// MAX_SELECT (bit 17-22): max. assertion duration of chip select (64*MAX_SELECT clock cycles)
		(minDeselect << 12) |	// MIN_DESELECT (bit 12-16): remains deasserted in number of clock cycles
		(1 << 8) |		// RXDELAY (bit 8-10): delay the read data sample timing in half-cycles
		clockDivider;		// CLKDIV (bit 0.7): clock divisor

	// enable interrupts
	IRQ_UNLOCK;
}

// initialize PSRAM memory interface (sets PSRAM_Size to 0 if no PSRAM)
void NOFLASH(PSRAM_Init)(void)
{
	u32 d, oldcsr;

// ---- check PSRAM and get size in bytes

	// get PSRAM size in bytes (returns 0 on error or if no PSRAM)
	int ramsize = PSRAM_GetSize();
	if (ramsize == 0) return; // no PSRAM

	// disable interrupts
	IRQ_LOCK;

// ---- switch to direct mode

	// enable direct serial mode
	qmi_hw_t* qmi = qmi_hw;
	qmi->direct_csr = (30 << 22) | B0; // B22..B29: CLKDIV = 30 (slow access), B0: EN enable direct mode

	// wait if last transfer is busy
	QMI_Wait();

// ---- setup PSRAM - reset and enable QSPI mode

	d = PSRAM_TransCS(PSRAM_CMD_RSTEN);	// reset + enable
	d = PSRAM_TransCS(PSRAM_CMD_RST);	// reset
	d = PSRAM_TransCS(PSRAM_CMD_QUAD_ENABLE); // enable QSPI

	// de-assert CS1n chip select line, disable direct mode
	qmi->direct_csr &= ~(B3 | B0); // B3: ASSERT_CS1N assert CS1n chip select line, B0: EN enable direct mode

	// enable interrupts
	IRQ_UNLOCK;

// ---- setup PSRAM timing

	// setup PSRAM timing
	PSRAM_Timing();

// ---- setup memory mapping

	// disable interrupts
	IRQ_RELOCK;

	// wait if last transfer is busy
	QMI_Wait();

	// read format
	qmi->m[1].rfmt = 
		(2 << 0) |	// PREFIX_WIDTH (bit 0-1): command prefix quad width
		(2 << 2) |	// ADDR_WIDTH (bit 2-3): address (24 bits) quad width
		(2 << 4) |	// SUFFIX_WIDTH (bit 4-5): post-addres command suffix quad width
		(2 << 6) |	// DUMMY_WIDTH (bit 6-7): dummy phase quad width
		(2 << 8) |	// DATA_WIDTH (bit 8-9): data transfer quad width
		(1 << 12) |	// PREFIX_LEN (bit 12): command prefix 8 bit
		(0 << 14) |	// SUFFIX_LEN (bit 14-15): post-address command sufix none
		(6 << 16);	// DUMMY_LEN (bit 16-18): dummy phase between command sufix and data 24 bits

	// read command
	qmi->m[1].rcmd =
		(PSRAM_CMD_QUAD_READ << 0) |	// command prefix to read data in QSPI format
		(0 << 8);			// no command suffix

	// write format
	qmi->m[1].wfmt =
		(2 << 0) |	// PREFIX_WIDTH (bit 0-1): command prefix quad width
		(2 << 2) |	// ADDR_WIDTH (bit 2-3): address (24 bits) quad width
		(2 << 4) |	// SUFFIX_WIDTH (bit 4-5): post-addres command suffix quad width
		(2 << 6) |	// DUMMY_WIDTH (bit 6-7): dummy phase quad width
		(2 << 8) |	// DATA_WIDTH (bit 8-9): data transfer quad width
		(1 << 12) |	// PREFIX_LEN (bit 12): command prefix 8 bit
		(0 << 14) |	// SUFFIX_LEN (bit 14-15): post-address command sufix none
		(0 << 16);	// DUMMY_LEN (bit 16-18): dummy phase between command sufix and data none

	// write command
	qmi->m[1].wcmd =
		(PSRAM_CMD_QUAD_WRITE << 0) |	// command prefix to write data in QSPI format
		(0 << 8);			// no command suffix

	// setup XIP control register - mark that we can write to PSRAM
	xip_ctrl_hw->ctrl |= B11;	// enable writes to XIP memory window 1 (address 0x11000000 through 0x11ffffff)

	// enable interrupts
	IRQ_UNLOCK;
}

#endif // USE_PSRAM
