
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

// To configure I2C pins SCL and SDA, set pull-up enabled, slew rate limited, schmitt trigger enabled.
// Default slave address is 0x055.
// Speed: standard mode 0 to 100 kb/s, fast mode 400 kb/s, fast mode plus 1000 kb/s.

/* Minimum system clock CLK_SYS derived from minimal values of registers:
speed SS:
	clkfreq = 2.7 MHz
	min SPKLEN = 1
	SCL low = 13 clk
	SCL low prog = 12
	SCL low time = 4.7 us
	SCL high = 14 clk
	SCL high prog = 6
	SCL high time = 5.2 us
speed FS:
	clkfreq = 12 MHz
	min SPKLEN = 1
	SCL low = 16 clk
	SCL low prog = 15
	SCL low time = 1.33 us
	SCL high = 14 clk
	SCL high prog = 6
	SCL high time = 1.16 us
speed FM+:
	clkfreq = 32 MHz
	min SPKLEN = 2
	SCL low = 16 clk
	SCL low prog = 15
	SCL low time = 0.5 us
	SCL high = 16 clk
	SCL high prog = 7
	SCL high time = 0.5 us
*/

#if USE_I2C	// use I2C interface (sdk_i2c.c, sdk_i2c.h)

#ifndef _SDK_I2C_H
#define _SDK_I2C_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "../sdk_dreq.h"
#include "sdk_timer.h"

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_i2c.h"		// constants of original SDK
#else
#include "orig_rp2350/orig_i2c.h"		// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_NUM			2		// number of I2C controllers
#define I2C_TXFIFO_NUM		16		// number of I2C tansmit FIFO entries
#define I2C_RXFIFO_NUM		16		// number of I2C receive FIFO entries
#define I2C_SLAVE_ADDR		0x055		// default slave address

// default baudrate
#define I2C_BAUDRATE_SS		100000		// standard mode 100 kb/s
#define I2C_BAUDRATE_FS		400000		// fast mode 400 kb/s
#define I2C_BAUDRATE_FP		1000000		// fast mode plus 1000 kb/s

// I2C hardware registers (i2c = 0 or 1)
#define I2C_BASE(i2c)		(I2C0_BASE+(i2c)*(I2C1_BASE-I2C0_BASE))	// I2C base address (i2c = 0 or 1)

#define I2C_CON(i2c)		((volatile u32*)(I2C_BASE(i2c)+0x00))	// control register ... DW_apb_i2c must be enabled
#define I2C_TAR(i2c)		((volatile u32*)(I2C_BASE(i2c)+0x04))	// target address register
#define I2C_SAR(i2c)		((volatile u32*)(I2C_BASE(i2c)+0x08))	// slave address register
#define I2C_DATA_CMD(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x10))	// data buffer and command register
#define I2C_SS_SCL_HCNT(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x14))	// standard speed I2C clock SCL High count register
#define I2C_SS_SCL_LCNT(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x18))	// standard speed I2C clock SCL Low count register
#define I2C_FS_SCL_HCNT(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x1C))	// fast speed I2C clock SCL High count register
#define I2C_FS_SCL_LCNT(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x20))	// fast speed I2C clock SCL Low count register
#define I2C_INTR_STAT(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x2C))	// interrupt status register
#define I2C_INTR_MASK(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x30))	// interrupt mask register
#define I2C_RAW_INTR_STAT(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x34))	// raw interrupt status register
#define I2C_RX_TL(i2c)		((volatile u32*)(I2C_BASE(i2c)+0x38))	// receive FIFO threshold register
#define I2C_TX_TL(i2c)		((volatile u32*)(I2C_BASE(i2c)+0x3C))	// transmit FIFO threshold register
#define I2C_CLR_INTR(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x40))	// clear interrupt
#define I2C_CLR_RX_UNDER(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x44))	// clear RX_UNDER
#define I2C_CLR_RX_OVER(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x48))	// clear RX_OVER
#define I2C_CLR_TX_OVER(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x4C))	// clear TX_OVER
#define I2C_CLR_RD_REQ(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x50))	// clear RD_REQ
#define I2C_CLR_TX_ABRT(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x54))	// clear TX_ABRT
#define I2C_CLR_RX_DONE(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x58))	// clear RX_DONE
#define I2C_CLR_ACTIVITY(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x5C))	// clear ACTIVITY
#define I2C_CLR_STOP_DET(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x60))	// clear STOP_DET
#define I2C_CLR_START_DET(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x64))	// clear START_DET
#define I2C_CLR_GEN_CALL(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x68))	// clear GEN_CALL
#define I2C_ENABLE(i2c)		((volatile u32*)(I2C_BASE(i2c)+0x6C))	// enable
#define I2C_STATUS(i2c)		((volatile u32*)(I2C_BASE(i2c)+0x70))	// status
#define I2C_TXFLR(i2c)		((volatile u32*)(I2C_BASE(i2c)+0x74))	// transmit FIFO level
#define I2C_RXFLR(i2c)		((volatile u32*)(I2C_BASE(i2c)+0x78))	// receive FIFO level
#define I2C_SDA_HOLD(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x7C))	// SDA hold time length
#define I2C_TX_ABRT_SOURCE(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x80))	// transmit abort source
#define I2C_SLV_DATA_NACK_ONLY(i2c) ((volatile u32*)(I2C_BASE(i2c)+0x84)) // generate slave data NACK
#define I2C_DMA_CR(i2c)		((volatile u32*)(I2C_BASE(i2c)+0x88))	// DMA control
#define I2C_DMA_TDLR(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x8C))	// DMA transmit data level
#define I2C_DMA_RDLR(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x90))	// DMA receive data level
#define I2C_SDA_SETUP(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x94))	// SDA setup
#define I2C_ACK_GENERAL_CALL(i2c) ((volatile u32*)(I2C_BASE(i2c)+0x98))	// ACK general call
#define I2C_ENABLE_STATUS(i2c)	((volatile u32*)(I2C_BASE(i2c)+0x9C))	// enable status
#define I2C_FS_SPKLEN(i2c)	((volatile u32*)(I2C_BASE(i2c)+0xA0))	// spike suppression limit
#define I2C_CLR_RESTART_DET(i2c) ((volatile u32*)(I2C_BASE(i2c)+0xA8))	// clear RESTART_DET
#define I2C_COMP_PARAM_1(i2c)	((volatile u32*)(I2C_BASE(i2c)+0xF4))	// component parameter register 1
#define I2C_COMP_VERSION(i2c)	((volatile u32*)(I2C_BASE(i2c)+0xF8))	// component version
#define I2C_COMP_TYPE(i2c)	((volatile u32*)(I2C_BASE(i2c)+0xFC))	// component type

// I2C hardware interface
typedef struct {
	io32	con;		// 0x00: I2C Control Register
	io32	tar;		// 0x04: I2C Target Address Register
	io32	sar;		// 0x08: I2C Slave Address Register
	io32	_pad0;		// 0x0C:
	io32	data_cmd;	// 0x10: I2C Rx/Tx Data Buffer and Command Register; this is the register the CPU writes to when filling the TX FIFO and the...
	io32	ss_scl_hcnt;	// 0x14: Standard Speed I2C Clock SCL High Count Register
	io32	ss_scl_lcnt;	// 0x18: Standard Speed I2C Clock SCL Low Count Register
	io32	fs_scl_hcnt;	// 0x1C: Fast Mode or Fast Mode Plus I2C Clock SCL High Count Register
	io32	fs_scl_lcnt;	// 0x20: Fast Mode or Fast Mode Plus I2C Clock SCL Low Count Register
	io32	_pad1[2];	// 0x28:
	io32	intr_stat;	// 0x2C: I2C Interrupt Status Register
	io32	intr_mask;	// 0x30: I2C Interrupt Mask Register
	io32	raw_intr_stat;	// 0x34: I2C Raw Interrupt Status Register
	io32	rx_tl;		// 0x38: I2C Receive FIFO Threshold Register
	io32	tx_tl;		// 0x3C: I2C Transmit FIFO Threshold Register
	io32	clr_intr;	// 0x40: Clear Combined and Individual Interrupt Register
	io32	clr_rx_under;	// 0x44: Clear RX_UNDER Interrupt Register
	io32	clr_rx_over;	// 0x48: Clear RX_OVER Interrupt Register
	io32	clr_tx_over;	// 0x4C: Clear TX_OVER Interrupt Register
	io32	clr_rd_req;	// 0x50: Clear RD_REQ Interrupt Register
	io32	clr_tx_abrt;	// 0x54: Clear TX_ABRT Interrupt Register
	io32	clr_rx_done;	// 0x58: Clear RX_DONE Interrupt Register
	io32	clr_activity;	// 0x5C: Clear ACTIVITY Interrupt Register
	io32	clr_stop_det;	// 0x60: Clear STOP_DET Interrupt Register
	io32	clr_start_det;	// 0x64: Clear START_DET Interrupt Register
	io32	clr_gen_call;	// 0x68: Clear GEN_CALL Interrupt Register
	io32	enable;		// 0x6C: I2C Enable Register
	io32	status;		// 0x70: I2C Status Register
	io32	txflr;		// 0x74: I2C Transmit FIFO Level Register This register contains the number of valid data entries in the transmit FIFO buffer
	io32	rxflr;		// 0x78: I2C Receive FIFO Level Register This register contains the number of valid data entries in the receive FIFO buffer
	io32	sda_hold;	// 0x7C: I2C SDA Hold Time Length Register
	io32	tx_abrt_source;	// 0x80: I2C Transmit Abort Source Register
	io32	slv_data_nack_only; // 0x84: Generate Slave Data NACK Register
	io32	dma_cr;		// 0x88: DMA Control Register
	io32	dma_tdlr;	// 0x8C: DMA Transmit Data Level Register
	io32	dma_rdlr;	// 0x90: I2C Receive Data Level Register
	io32	sda_setup;	// 0x94: I2C SDA Setup Register
	io32	ack_general_call; // 0x98: I2C ACK General Call Register
	io32	enable_status;	// 0x9C: I2C Enable Status Register
	io32	fs_spklen;	// 0xA0: I2C SS, FS or FM+ spike suppression limit
	io32	_pad2;		// 0xA4:
	io32	clr_restart_det; // 0xA8: Clear RESTART_DET Interrupt Register
	io32	_pad3[18];	// 0xAC:
	io32	comp_param_1;	// 0xF4: Component Parameter Register 1
	io32	comp_version;	// 0xF8: I2C Component Version Register
	io32	comp_type;	// 0xFC: I2C Component Type Register
} i2c_hw_t;

#define i2c0_hw ((i2c_hw_t*)I2C0_BASE)
#define i2c1_hw ((i2c_hw_t*)I2C1_BASE)

#define i2c0	i2c0_hw
#define i2c1	i2c1_hw

STATIC_ASSERT(sizeof(i2c_hw_t) == 0x100, "Incorrect i2c_hw_t!");

// I2C slave event types.
typedef enum i2c_slave_event_t
{
	I2C_SLAVE_RECEIVE,	// Data from master is available for reading. Slave must read from Rx FIFO.
	I2C_SLAVE_REQUEST,	// Master is requesting data. Slave must write into Tx FIFO.
	I2C_SLAVE_FINISH,	// Master has sent a Stop or Restart signal. Slave may prepare for the next transfer.
} i2c_slave_event_t;

// I2C slave event handler
typedef void (*i2c_slave_handler_t)(int i2c, i2c_slave_event_t event);

// get I2C hardware interface from I2C index
INLINE i2c_hw_t* I2C_GetHw(int i2c) { return (i2c == 0) ? i2c0_hw : i2c1_hw; }

// get I2C index from I2C hardware interface
INLINE u8 I2C_GetInx(const i2c_hw_t* hw) { return (hw == i2c0_hw) ? 0 : 1; }

// I2C reset
void I2C_Reset(int i2c);

// I2C initialize to master mode (to terminate, use I2C_Term())
//   Speed of standard mode 0 to 100 kb/s, fast mode 400 kb/s, fast mode plus 1000 kb/s.
//   Minimum system clock CLK_SYS: standard mode min. 2.7 MHz, fast mode 12 MHz, fast mode plus 32 MHz.
void I2C_Init(int i2c, u32 baudrate);

// I2C terminate
void I2C_Term(int i2c);

// I2C initialize to slave mode, with event handler (called after I2C_Init)
// - first run I2C_Init and then I2C_SlaveInit
// - to terminate, run I2C_SlaveTerm and then I2C_Term
void I2C_SlaveInit(int i2c, u8 addr, i2c_slave_handler_t handler);

// terminate slave mode and return to master mode
void I2C_SlaveTerm(int i2c);

// I2C switch to slave mode
//  addr ... slave address, must be in interval from 0x08 to 0x77
void I2C_SlaveMode(int i2c, u8 addr);

// I2C switch to master mode
void I2C_MasterMode(int i2c);

// check if I2C is enabled
INLINE Bool I2C_IsEnabled(int i2c) { return (*I2C_ENABLE_STATUS(i2c) & B0) != 0; }
INLINE Bool I2C_IsEnabled_hw(const i2c_hw_t* hw) { return (hw->enable_status & B0) != 0; }

// I2C enable (wait to complete)
void I2C_Enable(int i2c);
void I2C_Enable_hw(i2c_hw_t* hw);

// I2C disable (wait to complete)
void I2C_Disable(int i2c);
void I2C_Disable_hw(i2c_hw_t* hw);

// check if I2C transfer is active
INLINE Bool I2C_IsActive(int i2c) { return (*I2C_STATUS(i2c) & B0) != 0; }
INLINE Bool I2C_IsActive_hw(const i2c_hw_t* hw) { return (hw->status & B0) != 0; }

// set I2C baudrate and enable I2C (requied only in master mode)
//   Speed of standard mode 0 to 100 kb/s, fast mode 400 kb/s, fast mode plus 1000 kb/s. Can use I2C_BAUDRATE_*.
//   Minimum system clock CLK_SYS: standard mode min. 2.7 MHz, fast mode 12 MHz, fast mode plus 32 MHz.
void I2C_Baudrate(int i2c, u32 baudrate);

// get current I2C baudrate
u32 I2C_GetBaudrate(int i2c);

// get remaining space in transmit FIFO (0 = transmit FIFO is full)
INLINE u8 I2C_TxFifo(int i2c) { return I2C_TXFIFO_NUM - *I2C_TXFLR(i2c); }
INLINE u8 I2C_TxFifo_hw(const i2c_hw_t* hw) { return I2C_TXFIFO_NUM - hw->txflr; }

// get number of entries in receive FIFO (0 = receive FIFO is empty)
INLINE u8 I2C_RxFifo(int i2c) { return *I2C_RXFLR(i2c); }
INLINE u8 I2C_RxFifo_hw(const i2c_hw_t* hw) { return hw->rxflr; }

// send data from buffer
void I2C_SendData(int i2c, const u8* src, int len);
void I2C_SendData_hw(i2c_hw_t* hw, const u8* src, int len);

// receive data to buffer
void I2C_RecvData(int i2c, u8* dst, int len);
void I2C_RecvData_hw(i2c_hw_t* hw, u8* dst, int len);

// get DREQ to use for pacing transfers
//  i2c ... I2C number 0 or 1
//  tx ... True for sending data to I2C, False for receiving data from I2C
INLINE u8 I2C_GetDreq(int i2c, Bool tx) { return DREQ_I2C0_TX + i2c*2 + (tx ? 0 : 1); }

// send message to slave (THIS is master)
//  i2c ... I2C number 0 or 1
//  addr ... slave address of device to write to, must be in interval from 0x08 to 0x77
//  src ... pointer to data to send
//  len .. length of data in bytest to send (must be > 0)
//  nostop ... no stop after end of transmission (next transfer will begin with Restart, not Start)
//  timeout ... timeout in [us] of one character sent (0=no timeout)
// Returns number of bytes sent, can be less than 'len' in case of error.
int I2C_SendMsg(int i2c, u8 addr, const u8* src, int len, Bool nostop, s32 timeout);

// read message from slave (THIS is master)
//  i2c ... I2C number 0 or 1
//  addr ... slave address of device to read from, must be in interval from 0x08 to 0x77
//  dst ... pointer to buffer to receive data
//  len .. length of data in bytest to receive (must be > 0)
//  nostop ... no stop after end of transfer (next transfer will begin with Restart, not Start)
//  timeout ... timeout in [us] of one character received (0=no timeout)
// Returns number of bytes received, can be less than 'len' in case of error.
int I2C_RecvMsg(int i2c, u8 addr, u8* dst, int len, Bool nostop, s32 timeout);

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original-SDK

#undef i2c0
#undef i2c1
typedef i2c_hw_t i2c_inst_t;
#define i2c0 ((i2c_inst_t*)i2c0_hw)
#define i2c1 ((i2c_inst_t*)i2c1_hw)

// Initialise the I2C HW block
INLINE uint i2c_init(i2c_inst_t* i2c, uint baudrate)
{
	int inx = I2C_GetInx(i2c);
	I2C_Init(inx, baudrate);
	return I2C_GetBaudrate(inx);
}

// Disable the I2C HW block
INLINE void i2c_deinit(i2c_inst_t* i2c) { I2C_Term(I2C_GetInx(i2c)); }

// Set I2C baudrate
INLINE uint i2c_set_baudrate(i2c_inst_t* i2c, uint baudrate)
{
	int inx = I2C_GetInx(i2c);
	I2C_Baudrate(inx, baudrate);
	return I2C_GetBaudrate(inx);
}

// Set I2C port to slave/master mode
INLINE void i2c_set_slave_mode(i2c_inst_t* i2c, bool slave, u8 addr)
{
	int inx = I2C_GetInx(i2c);
	if (slave)
		I2C_SlaveMode(inx, addr);
	else
		I2C_MasterMode(inx);
}

// Convert I2C instance to hardware instance number
INLINE uint i2c_hw_index(i2c_inst_t* i2c) { return I2C_GetInx(i2c); }
INLINE i2c_hw_t* i2c_get_hw(i2c_inst_t* i2c) { return i2c; }
INLINE i2c_inst_t* i2c_get_instance(uint instance) { return I2C_GetHw(instance); }

// Attempt to write specified number of bytes to address, blocking until the specified absolute time is reached.
//   i2c ... Either i2c0 or i2c1
//   addr ... 7-bit address of device to write to
//   src ... Pointer to data to send
//   len ... Length of data in bytes to send
//   nostop ... If true, master retains control of the bus at the end of the transfer (no Stop is issued),
//             and the next transfer will begin with a Restart rather than a Start.
//   until ... The absolute time that the block will wait until the entire transaction is complete.
// return Number of bytes written
INLINE int i2c_write_blocking_until(i2c_inst_t* i2c, u8 addr, const u8* src, size_t len, bool nostop, absolute_time_t until)
	{ return I2C_SendMsg(I2C_GetInx(i2c), addr, src, len, nostop, (s32)((u32)until - Time())); }

// Attempt to read specified number of bytes from address, blocking until the specified absolute time is reached.
//   i2c ... Either i2c0 or i2c1
//   addr ... 7-bit address of device to read from
//   dst ... Pointer to buffer to receive data
//   len ... Length of data in bytes to receive
//   nostop ... If true, master retains control of the bus at the end of the transfer (no Stop is issued),
//             and the next transfer will begin with a Restart rather than a Start.
//   until ... The absolute time that the block will wait until the entire transaction is complete.
// return Number of bytes read
INLINE int i2c_read_blocking_until(i2c_inst_t* i2c, u8 addr, u8* dst, size_t len, bool nostop, absolute_time_t until)
	{ return I2C_RecvMsg(I2C_GetInx(i2c), addr, dst, len, nostop, (s32)((u32)until - Time())); }

// Attempt to write specified number of bytes to address, with timeout
//   i2c ... Either i2c0 or i2c1
//   addr ... 7-bit address of device to write to
//   src ... Pointer to data to send
//   len ... Length of data in bytes to send
//   nostop ... If true, master retains control of the bus at the end of the transfer (no Stop is issued),
//             and the next transfer will begin with a Restart rather than a Start.
//   timeout_us ... The time that the function will wait for the entire transaction to complete.
// return Number of bytes written
INLINE int i2c_write_timeout_us(i2c_inst_t* i2c, u8 addr, const u8* src, size_t len, bool nostop, uint timeout_us)
	{ return I2C_SendMsg(I2C_GetInx(i2c), addr, src, len, nostop, timeout_us); }

INLINE int i2c_write_timeout_per_char_us(i2c_inst_t* i2c, u8 addr, const u8* src, size_t len, bool nostop, uint timeout_us)
	{ return I2C_SendMsg(I2C_GetInx(i2c), addr, src, len, nostop, timeout_us); }

// Attempt to read specified number of bytes from address, with timeout
//   i2c ... Either i2c0 or i2c1
//   addr ... 7-bit address of device to read from
//   dst ... Pointer to buffer to receive data
//   len ... Length of data in bytes to receive
//   nostop ... If true, master retains control of the bus at the end of the transfer (no Stop is issued),
//             and the next transfer will begin with a Restart rather than a Start.
//   timeout_us ... The time that the function will wait for the entire transaction to complete
// return Number of bytes read
INLINE int i2c_read_timeout_us(i2c_inst_t* i2c, u8 addr, u8* dst, size_t len, bool nostop, uint timeout_us)
	{ return I2C_RecvMsg(I2C_GetInx(i2c), addr, dst, len, nostop, timeout_us); }

INLINE int i2c_read_timeout_per_char_us(i2c_inst_t* i2c, u8 addr, u8* dst, size_t len, bool nostop, uint timeout_us)
	{ return I2C_RecvMsg(I2C_GetInx(i2c), addr, dst, len, nostop, timeout_us); }

// Attempt to write specified number of bytes to address, blocking
//   i2c ... Either i2c0 or i2c1
//   addr ... 7-bit address of device to write to
//   src ... Pointer to data to send
//   len ... Length of data in bytes to send
//   nostop ... If true, master retains control of the bus at the end of the transfer (no Stop is issued),
//             and the next transfer will begin with a Restart rather than a Start.
// return Number of bytes written
INLINE int i2c_write_blocking(i2c_inst_t* i2c, u8 addr, const u8* src, size_t len, bool nostop)
	{ return I2C_SendMsg(I2C_GetInx(i2c), addr, src, len, nostop, 0); }

// Attempt to read specified number of bytes from address, blocking
//   i2c ... Either i2c0 or i2c1
//   addr ... 7-bit address of device to read from
//   dst ... Pointer to buffer to receive data
//   len ... Length of data in bytes to receive
//   nostop ... If true, master retains control of the bus at the end of the transfer (no Stop is issued),
//             and the next transfer will begin with a Restart rather than a Start.
// return Number of bytes read
INLINE int i2c_read_blocking(i2c_inst_t* i2c, u8 addr, u8* dst, size_t len, bool nostop)
	{ return I2C_RecvMsg(I2C_GetInx(i2c), addr, dst, len, nostop, 0); }

// Determine non-blocking write space available
//   i2c ... Either i2c0 or i2c1
// return 0 if no space is available in the I2C to write more data. If return is nonzero, at
// least that many bytes can be written without blocking.
INLINE size_t i2c_get_write_available(i2c_inst_t* i2c) { return I2C_TxFifo_hw(i2c); }

// Determine number of bytes received
//   i2c ... Either i2c0 or i2c1
// return 0 if no data available, if return is nonzero at
// least that many bytes can be read without blocking.
INLINE size_t i2c_get_read_available(i2c_inst_t* i2c) { return I2C_RxFifo_hw(i2c); }

// Write direct to TX FIFO
//   i2c ... Either i2c0 or i2c1
//   src ... Data to send
//   len ... Number of bytes to send
INLINE void i2c_write_raw_blocking(i2c_inst_t* i2c, const u8* src, size_t len) { I2C_SendData_hw(i2c, src, len); }

// Read direct from RX FIFO
//   i2c ... Either i2c0 or i2c1
//   dst ... Buffer to accept data
//   len ... Number of bytes to read
INLINE void i2c_read_raw_blocking(i2c_inst_t* i2c, u8* dst, size_t len) { I2C_RecvData_hw(i2c, dst, len); }

// Pop a byte from I2C Rx FIFO.
INLINE u8 i2c_read_byte_raw(i2c_inst_t* i2c) { return (u8)i2c->data_cmd; }

// Push a byte into I2C Tx FIFO.
INLINE void i2c_write_byte_raw(i2c_inst_t* i2c, u8 value) { i2c->data_cmd = value; }

// Return the DREQ to use for pacing transfers to/from a particular I2C instance
//   i2c ... Either i2c0 or i2c1
//   is_tx ... true for sending data to the I2C instance, false for receiving data from the I2C instance
INLINE uint i2c_get_dreq(i2c_inst_t* i2c, bool is_tx) { return I2C_GetDreq(I2C_GetInx(i2c), is_tx); }
STATIC_ASSERT(DREQ_I2C0_RX == DREQ_I2C0_TX + 1, "Invalid DREQ_I2C*");
STATIC_ASSERT(DREQ_I2C1_RX == DREQ_I2C1_TX + 1, "Invalid DREQ_I2C*");
STATIC_ASSERT(DREQ_I2C1_TX == DREQ_I2C0_TX + 2, "Invalid DREQ_I2C*");

// Configure an I2C instance for slave mode.
//   i2c ... Either i2c0 or i2c1
//   address ... 7-bit slave address.
//   handler ... Callback for events from I2C master. It will run from the I2C ISR, on the CPU core
//              where the slave was initialised.
INLINE void i2c_slave_init(i2c_inst_t* i2c, u8 address, i2c_slave_handler_t handler)
	{ I2C_SlaveInit(I2C_GetInx(i2c), address, handler); }

// Restore an I2C instance to master mode.
//   i2c ... Either i2c0 or i2c1
INLINE void i2c_slave_deinit(i2c_inst_t* i2c) { I2C_SlaveTerm(I2C_GetInx(i2c)); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_I2C_H

#endif // USE_I2C	// use I2C interface (sdk_i2c.c, sdk_i2c.h)
