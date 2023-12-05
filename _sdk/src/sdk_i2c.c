
// ****************************************************************************
//
//                                    I2C
//                          Inter-Integrated Circuit
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

#if USE_I2C	// use I2C interface (sdk_i2c.c, sdk_i2c.h)

#include "../inc/sdk_cpu.h"
#include "../inc/sdk_timer.h"
#include "../inc/sdk_i2c.h"
#include "../inc/sdk_clocks.h"
#include "../inc/sdk_reset.h"
#include "../sdk_dreq.h"

// flag - restart I2C on next transmission
Bool I2C_RestartOnNext[I2C_NUM] = { False, False };

// flag - slave transfer is in progress
Bool I2C_TransferInProgress[I2C_NUM] = { False, False };

// I2C slave event handlers
i2c_slave_handler_t I2C_SlaveHandler[I2C_NUM] = { NULL, NULL };

// I2C slave event
void I2C_SlaveIrqHandler()
{
	// get current I2C index
	int i2c = (GetCurrentIRQ() == IRQ_I2C0) ? 0 : 1;

	// get hw interface
	i2c_hw_t* hw = I2C_GetHw(i2c);

	// get interrupt status register
	u32 stat = hw->intr_stat;
	if (stat == 0) return; // no interrupt

	// flag - finish transfer
	Bool finish = False;

	// R_TX_ABRT transmit abort
	if ((stat & B6) != 0)
	{
		// clear transmit abort flag
		(void)hw->clr_tx_abrt;

		// finish transfer
		finish = True;
	}

	// R_START_DET start or restart condition has occured
	if ((stat & B10) != 0)
	{
		// clear start det flag
		(void)hw->clr_start_det;

		// finish transfer
		finish = True;
	}

	// R_STOP_DET stop condition has occured
	if ((stat & B9) != 0)
	{
		// clear stop det flag
		(void)hw->clr_stop_det;

		// finish transfer
		finish = True;
	}

	// finish transfer
	if (finish && I2C_TransferInProgress[i2c])
	{
		// call handler to finish transfer
		i2c_slave_handler_t handler = I2C_SlaveHandler[i2c];
		if (handler != NULL) handler(i2c, I2C_SLAVE_FINISH);

		// transfer not in progress
		I2C_TransferInProgress[i2c] = False;
	}

	// R_RX_FULL receiving data from master (receive buffer reaches or goes above RX_TL threshold)
	if ((stat & B2) != 0)
	{
		// transfer is in progress
		I2C_TransferInProgress[i2c] = True;

		// receive data from master
		i2c_slave_handler_t handler = I2C_SlaveHandler[i2c];
		if (handler != NULL) handler(i2c, I2C_SLAVE_RECEIVE);
	}

	// R_RD_REQ sending data to master (master required data, slave holds I2C bus in a wait state until data are sent)
	if ((stat & B5) != 0)
	{
		// clear read request flag
		hw->clr_rd_req;

		// transfer is in progress
		I2C_TransferInProgress[i2c] = True;

		// send data to master
		i2c_slave_handler_t handler = I2C_SlaveHandler[i2c];
		if (handler != NULL) handler(i2c, I2C_SLAVE_REQUEST);
	}
}

// I2C reset
void I2C_Reset(int i2c)
{
	u8 peri = (i2c == 0) ? RESET_I2C0 : RESET_I2C1;
	ResetPeriphery(peri);
}

// I2C initialize to master mode (to terminate, use I2C_Term())
//   Speed of standard mode 0 to 100 kb/s, fast mode 400 kb/s, fast mode plus 1000 kb/s.
//   Minimum system clock CLK_SYS: standard mode min. 2.7 MHz, fast mode 12 MHz, fast mode plus 32 MHz.
void I2C_Init(int i2c, u32 baudrate)
{
	// I2C disable and reset
	I2C_Term(i2c);

	// no restart on next
	I2C_RestartOnNext[i2c] = False;

	// get hw interface
	i2c_hw_t* hw = I2C_GetHw(i2c);

	// disable I2C (not needed, but wait to complete initialization)
	I2C_Disable_hw(hw);

	// configure fast-mode master with Restart support, 7-bit addresses
	hw->con = 
		B0 +		// Master mode enabled
		(2 << 1) + 	// fast mode
		B5 +		// master restart support
		B6 +		// slave disable
		B8;		// controlled generation of TX_EMPTY interrupt
		
	// set FIFO thresholds to minimum samples
	hw->rx_tl = 0;	// receive FIFO threshold register
	hw->tx_tl = 0;	// transmit FIFO threshold register

	// enable TX and RX DREQ signalling - harmless if DMA is not listening
	hw->dma_cr = B0+B1;

	// set I2C baudrate and enable I2C (requied only in master mode)
	I2C_Baudrate(i2c, baudrate);
}

// I2C terminate
void I2C_Term(int i2c)
{
	// I2C disable (wait to complete)
	I2C_Disable(i2c);

	// I2C reset
	I2C_Reset(i2c);
}

// I2C initialize to slave mode, with event handler (called after I2C_Init)
// - first run I2C_Init and then I2C_SlaveInit
// - to terminate, run I2C_SlaveTerm and then I2C_Term
void I2C_SlaveInit(int i2c, u8 addr, i2c_slave_handler_t handler)
{
	// disable IRQ
	int irq = IRQ_I2C0 + i2c;
	NVIC_IRQDisable(irq);

	// get hw interface
	i2c_hw_t* hw = I2C_GetHw(i2c);

	// disable I2C
	I2C_Disable_hw(hw);

	// set slave handler
	I2C_TransferInProgress[i2c] = False;
	I2C_SlaveHandler[i2c] = handler;

	// set interrupt mask (B2=RX_FULL, B5=RD_REQ, B6=TX_ABRT, B9=STOP_DET, B10=START_DET)
	hw->intr_mask = B2 | B5 | B6 | B9 | B10;

	// set IRQ handler
	SetHandler(irq, I2C_SlaveIrqHandler);

	// enable IRQ
	NVIC_IRQEnable(irq);

	// set slave mode and I2C enable
	I2C_SlaveMode(i2c, addr);
}

// terminate slave mode and return to master mode
void I2C_SlaveTerm(int i2c)
{
	// disable IRQ
	int irq = IRQ_I2C0 + i2c;
	NVIC_IRQDisable(irq);

	// get hw interface
	i2c_hw_t* hw = I2C_GetHw(i2c);

	// disable I2C
	I2C_Disable_hw(hw);

	// deactivate slave interrupt handler
	I2C_SlaveHandler[i2c] = NULL;
	I2C_TransferInProgress[i2c] = False;

	// set interrupt mask to default reset state
	hw->intr_mask = 0x8ff;

	// set master mode and I2C enable
	I2C_MasterMode(i2c);
}

// I2C switch to slave mode
//  addr ... slave address, must be in interval from 0x08 to 0x77
void I2C_SlaveMode(int i2c, u8 addr)
{
	// get hw interface
	i2c_hw_t* hw = I2C_GetHw(i2c);

	// disable I2C
	I2C_Disable_hw(hw);

	// set slave mode (enable slave, disable master, hold bus)
	RegMask(&hw->con, B9, B9+B6+B0);

	// set slave address
	hw->sar = addr;

	// I2C enable
	I2C_Enable_hw(hw);
}

// I2C switch to master mode
void I2C_MasterMode(int i2c)
{
	// get hw interface
	i2c_hw_t* hw = I2C_GetHw(i2c);

	// disable I2C
	I2C_Disable_hw(hw);

	// set master mode (disable slave, enable master, not holding bus)
	RegMask(&hw->con, B6+B0, B9+B6+B0);

	// I2C enable
	I2C_Enable_hw(hw);
}

// I2C enable (wait to complete)
void I2C_Enable(int i2c)
{
	cb();
	RegSet(I2C_ENABLE(i2c), B0);
	cb();
	while (!I2C_IsEnabled(i2c)) {}
	cb();
}

void I2C_Enable_hw(i2c_hw_t* hw)
{
	cb();
	RegSet(&hw->enable, B0);
	cb();
	while (!I2C_IsEnabled_hw(hw)) {}
	cb();
}

// I2C disable (wait to complete)
void I2C_Disable(int i2c)
{
	cb();
	RegClr(I2C_ENABLE(i2c), B0);
	cb();
	while (I2C_IsEnabled(i2c)) {}
	cb();
}

void I2C_Disable_hw(i2c_hw_t* hw)
{
	cb();
	RegClr(&hw->enable, B0);
	cb();
	while (I2C_IsEnabled_hw(hw)) {}
	cb();
}

// set I2C baudrate and enable I2C (requied only in master mode)
//   Speed of standard mode 0 to 100 kb/s, fast mode 400 kb/s, fast mode plus 1000 kb/s. Can use I2C_BAUDRATE_*.
//   Minimum system clock CLK_SYS: standard mode min. 2.7 MHz, fast mode 12 MHz, fast mode plus 32 MHz.
void I2C_Baudrate(int i2c, u32 baudrate)
{
	// get hw interface
	i2c_hw_t* hw = I2C_GetHw(i2c);

	// I2C must be disabled
	I2C_Disable_hw(hw);
	if (baudrate == 0) return;

	// set standard or fast mode (high speed mode is not supported)
	RegMask(&hw->con, ((baudrate >= 110000) ? 2 : 1) << 1, B1+B2);	

	// get current system clock
	u32 freq = CurrentFreq[CLK_SYS];

// LOW pulse: LCNT + 1 (+ SCL rise time)
// HIGH pulse: HCNT + SPKLEN + 7 (+ SCL fall time)
// period: LCNT + HCNT + SPKLEN + 8 (+ SCL rise+fall time)
// required:
//   SPKLEN >= 1
//   LCNT >= SPKLEN + 7
//   HCNT >= SPKLEN + 5 (SS: max. 65525)
//   period SPKLEN+LCNT+HCNT >= 1+7 + 1+5 + 1 + 8 = 23

// SCL minimum high period: 4 us for 100 kbps, 0.6 us for 400 kpbs, 0.26 for 1000 kbps
// SCL minimum low period: 4.7 us for 100 kbps, 1.3 us for 400 kpbs, 0.5 for 1000 kbps

// CNT high period on 48 MHz: 192 clk for 100 kbps, 29 clk for 400 kpbs, 12 clk for 1000 kbps
// CNT low period on 48 MHz: 226 clk for 100 kbps, 62 ckl for 400 kpbs, 24 clk for 1000 kbps

	// total clock cycles per one bit
	u32 period = (freq + baudrate/2) / baudrate;
	if (period < 23) period = 23;

	// get pulses (period: LCNT + HCNT + SPKLEN + 8)
	period -= 8;
	u32 spklen = period / 16;
	if (spklen < 1) spklen = 1;
	if (spklen > 255) spklen = 255;
	period -= spklen;
	u32 lcnt = period * 3 / 5; // lcnt should be a little longer than hcnt
	if (lcnt < spklen + 7) lcnt = spklen + 7;
	if (lcnt > 0xffff) lcnt = 0xffff;
	u32 hcnt = period - lcnt;
	if (hcnt < spklen + 5) hcnt = spklen + 5;
	if (hcnt > 65525) hcnt = 65525;

	// set counters
	hw->ss_scl_hcnt = hcnt;
	hw->ss_scl_lcnt = lcnt;
	hw->fs_scl_hcnt = hcnt;
	hw->fs_scl_lcnt = lcnt;
	hw->fs_spklen = spklen;

	// set tx hold count (300 ns for standard mode and fast mode, 120 fs for fast mode plus)
	u32 hold;
	if (baudrate < 600000)
		hold = freq*3/10000000+1; // 300 ns
	else
		hold = freq*3/25000000+1; // 120 ns
	if (hold > lcnt - 2) hold = lcnt - 2;
	RegMask(&hw->sda_hold, hold, 0xffff);

	// I2C enable
	I2C_Enable_hw(hw);
}

// get current I2C baudrate
u32 I2C_GetBaudrate(int i2c)
{
	// get hw interface
	i2c_hw_t* hw = I2C_GetHw(i2c);

	// get system frequency
	u32 freq = CurrentFreq[CLK_SYS];

	// get counter setup
	u32 hcnt = hw->fs_scl_hcnt & 0xffff;
	u32 lcnt = hw->fs_scl_lcnt & 0xffff;
	u32 spklen = hw->fs_spklen & 0xff;

	// get frequency
	u32 period = lcnt + hcnt + spklen + 8;
	return (freq + period/2) / period;
}

// send data from buffer
void I2C_SendData(int i2c, const u8* src, int len)
{
	for (; len > 0; len--)
	{
		while (I2C_TxFifo(i2c) == 0) {}
		*I2C_DATA_CMD(i2c) = *src++;
	}
}

void I2C_SendData_hw(i2c_hw_t* hw, const u8* src, int len)
{
	for (; len > 0; len--)
	{
		while (I2C_TxFifo_hw(hw) == 0) {}
		hw->data_cmd = *src++;
	}
}

// receive data to buffer
void I2C_RecvData(int i2c, u8* dst, int len)
{
	for (; len > 0; len--)
	{
		while (I2C_RxFifo(i2c) == 0) {}
		*dst++ = (u8)*I2C_DATA_CMD(i2c);
	}
}

void I2C_RecvData_hw(i2c_hw_t* hw, u8* dst, int len)
{
	for (; len > 0; len--)
	{
		while (I2C_RxFifo_hw(hw) == 0) {}
		*dst++ = (u8)hw->data_cmd;
	}
}

// send message to slave (THIS is master)
//  i2c ... I2C number 0 or 1
//  addr ... slave address of device to write to, must be in interval from 0x08 to 0x77
//  src ... pointer to data to send
//  len .. length of data in bytest to send (must be > 0)
//  nostop ... no stop after end of transmission (next transfer will begin with Restart, not Start)
//  timeout ... timeout in [us] of one character sent (0=no timeout)
// Returns number of bytes sent, can be less than 'len' in case of error.
int I2C_SendMsg(int i2c, u8 addr, const u8* src, int len, Bool nostop, s32 timeout)
{
	// cannot send 0 data bytes (or time elapsed)
	if ((len <= 0) || (timeout < 0)) return 0;

	// get hw interface
	i2c_hw_t* hw = I2C_GetHw(i2c);

	// set target address (temporary disable I2C)
	hw->enable = 0; // I2C disable
	hw->tar = addr;	 // set slave address
	hw->enable = B0; // I2C enable

	// send data
	Bool abort = False; // clear abort transfer flag
	u32 d, t;
	int data_sent;
	for (data_sent = 0; data_sent < len; data_sent++)
	{
		// get data byte to send
		d = *src++;	// data byte to send

		// restart on start of new transmission
		if ((data_sent == 0) && I2C_RestartOnNext[i2c]) d |= B10;

		// stop message, sent STOP condition
		if ((data_sent == len-1) && !nostop) d |= B9;

		// write data
		hw->data_cmd = d;

		// wait for end of transmission or timeout
		t = Time(); // start time
		do {
			// check time-out
			if (timeout > 0)
			{
				if ((u32)(Time() - t) >= timeout) abort = True;
			}

		} while (!abort && ((hw->raw_intr_stat & B4) == 0)); // check TX_EMPTY flag

		// character was sent (no timeout)
		if (!abort)
		{
			// check transmission abort
			if (hw->tx_abrt_source != 0)
			{
				// clear abort flags by reading flag register
				(void)hw->clr_tx_abrt;
				abort = True;
			}

			// if abort or if last byte was sent, wait for end of transmission (wait for STOP condition)
			if (abort || ((data_sent == len-1) && !nostop))
			{
				do {
					// check time-out
					if (timeout > 0)
					{
						if ((u32)(Time() - t) >= timeout) abort = True;
					}
				} while (!abort && ((hw->raw_intr_stat & B9) == 0)); // check STOP_DET flag

				// if not timeout, clear stop-detection flag
				if (!abort) (void)hw->clr_stop_det;
			}
		}

		// abort (timeout or transmission abort)
		if (abort) break;
	}

	// restart on next transmission
	I2C_RestartOnNext[i2c] = nostop;

	// return data sent (should be equal to len)
	return data_sent;
}

// read message from slave (THIS is master)
//  i2c ... I2C number 0 or 1
//  addr ... slave address of device to read from, must be in interval from 0x08 to 0x77
//  dst ... pointer to buffer to receive data
//  len .. length of data in bytest to receive (must be > 0)
//  nostop ... no stop after end of transfer (next transfer will begin with Restart, not Start)
//  timeout ... timeout in [us] of one character received (0=no timeout)
// Returns number of bytes received, can be less than 'len' in case of error.
int I2C_RecvMsg(int i2c, u8 addr, u8* dst, int len, Bool nostop, s32 timeout)
{
	// cannot receive 0 data bytes (or time elapsed)
	if ((len <= 0) || (timeout < 0)) return 0;

	// get hw interface
	i2c_hw_t* hw = I2C_GetHw(i2c);

	// set target address (temporary disable I2C)
	hw->enable = 0; // I2C disable
	hw->tar = addr;	 // set slave address
	hw->enable = B0; // I2C enable

	// receive data
	Bool abort = False; // clear abort transfer flag
	u32 d, t;
	int data_recv;
	for (data_recv = 0; data_recv < len; data_recv++)
	{
		// wait for TX FIFO
		while (I2C_TxFifo_hw(hw) == 0) {}

		// write command "read data"	
		d = B8; // command: master read
		if ((data_recv == 0) && I2C_RestartOnNext[i2c]) d |= B10; // restart on start of new transmission
		if ((data_recv == len-1) && !nostop) d |= B9; // stop message, sent STOP condition
		hw->data_cmd = d; // write command

		// wait for receiving character or timeout
		t = Time(); // start time
		do {
			// check transfer abort
			if (hw->tx_abrt_source != 0)
			{
				// clear abort flags by reading flag register
				(void)hw->clr_tx_abrt;
				abort = True;
			}

			// check timeout
			if (timeout > 0)
			{
				if ((u32)(Time() - t) >= timeout) abort = True;
			}

		} while (!abort && (I2C_RxFifo_hw(hw) == 0));

		// abort (timeout or transfer abort)
		if (abort) break;

		// read data into buffer
		*dst++ = (u8)hw->data_cmd;
	}

	// restart on next transmission
	I2C_RestartOnNext[i2c] = nostop;

	// return data received (should be equal to len)
	return data_recv;
}

#endif // USE_I2C	// use I2C interface (sdk_i2c.c, sdk_i2c.h)
