
// ****************************************************************************
//
//                         USB Mass Storage Class (device)
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

// This code is based on TinyUSB library, Copyright (c) 2019 Ha Thach (tinyusb.org)

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#if USE_USB_DEV_MSC	// use USB MSC Mass Storage Class (device)

#ifndef _SDK_USB_MSCD_H
#define _SDK_USB_MSCD_H

#include "sdk_usb_def.h"	// definitions

#ifdef __cplusplus
extern "C" {
#endif

// SCSI Command Operation Code
#define SCSI_CMD_TEST_UNIT_READY		0x00	// test if device is ready
#define SCSI_CMD_INQUIRY			0x12	// get basic information from device
#define SCSI_CMD_MODE_SELECT_6			0x15
#define SCSI_CMD_MODE_SENSE_6			0x1A
#define SCSI_CMD_START_STOP_UNIT		0x1B
#define SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL	0x1E
#define SCSI_CMD_READ_CAPACITY_10		0x25	// read capacity
#define SCSI_CMD_REQUEST_SENSE			0x03	// request sense
#define SCSI_CMD_READ_FORMAT_CAPACITY		0x23	// request list of possible format capacities
#define SCSI_CMD_READ_10			0x28	// read from device media
#define SCSI_CMD_WRITE_10			0x2A	// write to device media

// SCSI Sense Key
#define SCSI_SENSE_NONE				0x00	// no specific Sense Key
#define SCSI_SENSE_RECOVERED_ERROR		0x01	// last operation ok with recovered error
#define SCSI_SENSE_NOT_READY			0x02	// logical unit cannot be accessed
#define SCSI_SENSE_MEDIUM_ERROR			0x03	// non-recovered error
#define SCSI_SENSE_HARDWARE_ERROR		0x04	// nonrecoverable hardware failure
#define SCSI_SENSE_ILLEGAL_REQUEST		0x05	// illegal parameter
#define SCSI_SENSE_UNIT_ATTENTION		0x06	// disc drive reset
#define SCSI_SENSE_DATA_PROTECT			0x07	// write protected
#define SCSI_SENSE_FIRMWARE_ERROR		0x08	// vendor specific sense key
#define SCSI_SENSE_ABORTED_COMMAND		0x0b	// aborted
#define SCSI_SENSE_EQUAL			0x0c	// SEARCH DATA satisfied comparison
#define SCSI_SENSE_VOLUME_OVERFLOW		0x0d	// end of medium
#define SCSI_SENSE_MISCOMPARE			0x0e	// data did not match

// MSC stage
#define MSC_STAGE_CMD		0	// waiting command
#define MSC_STAGE_DATA		1	// waiting data
#define MSC_STAGE_STATUS	2	// waiting status
#define MSC_STAGE_SENT		3	// status has been sent

// MSC CSW status
#define MSC_CSW_STATUS_PASSED		0
#define MSC_CSW_STATUS_FAILED		1
#define MSC_CSW_STATUS_PHASE_ERROR	2

// MSC read data from device callback
//  lun ... logical unit number (0 .. MscdLun-1)
//  lba ... logical block address to be read
//  off ... byte offset from LBA
//  buf ... buffer
//  bufsize ... requested bytes
// Return number of bytes read (can be less than required) or 0 (if device is not ready) or -1 (if error, will stall).
typedef int (*pMscdRead10Cb) (u8 lun, u32 lba, u32 off, void* buf, int bufsize);

// MSC write data to device callback
//  lun ... logical unit number (0 .. MscdLun-1)
//  lba ... logical block address to be written
//  off ... byte offset from LBA
//  buf ... buffer
//  bufsize ... requested bytes
// Return number of bytes written (can be less than required) or 0 (if device is not ready) or -1 (if error, will stall).
typedef int (*pMscdWrite10Cb) (u8 lun, u32 lba, u32 off, void* buf, int bufsize);

// MSC inquiry callback
//  lun ... logical unit number (0 .. MscdLun-1)
//  vendor_id ... pointer to vendor ASCII id, space padded, max. 8 characters
//  product_id ... pointer to product ASCII id, space padded, max. 16 characters
//  product_rev ... pointer to product ASCII revision, space padded, max. 4 characters
typedef void (*pMscdInquiryCb) (u8 lun, u8* vendor_id, u8* product_id, u8* product_rev);

// MSC device ready callback (returns False if not ready - SD card not inserted)
//  lun ... logical unit number (0 .. MscdLun-1)
typedef Bool (*pMscdReadyCb) (u8 lun);

// MSC get device capacity callback
//  lun ... logical unit number (0 .. MscdLun-1)
//  block_count ... pointer to number of blocks
//  block_size ... pointer to size of block
typedef void (*pMscdGetCapacityCb) (u8 lun, u32* block_count, u16* block_size);

// MSC process SCSI command callback (returns length of data copied to buffer or -1 = error)
//  lun ... logical unit number (0 .. MscdLun-1)
//  cmd ... SCSI command block (16 bytes)
//  buffer ... additional data buffer
//  bufsize ... buffer size
// Invoked when received SCSI command not in built-in: TEST_UNIT_READY, START_STOP_UNIT,
//      READ_CAPACITY_10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE_6, REQUEST_SENSE
// READ10 and WRITE10 have their own callbacks.
typedef int (*pMscdScsiCmdCb) (u8 lun, const u8* cmd, u8* buffer, u32 bufsize);

// MSC Read10 command complete callback
//  lun ... logical unit number (0 .. MscdLun-1)
typedef void (*pMscdRead10Comp) (u8 lun);

// MSC Write10 command complete callback
//  lun ... logical unit number (0 .. MscdLun-1)
typedef void (*pMscdWrite10Comp) (u8 lun);

// MSC SCSI command complete callback
//  lun ... logical unit number (0 .. MscdLun-1)
//  cmd ... command buffer (16 bytes)
typedef void (*pMscdScsiComp) (u8 lun, const u8* cmd);

// MSC check if medium is writable callback (returns False if medium is protected)
//  lun ... logical unit number (0 .. MscdLun-1)
typedef Bool (*pMscdIsWritableCb) (u8 lun);

// command block wrapper (31 bytes)
typedef struct PACKED {
	u32	signature;	// signature
	u32	tag;		// tag sent by host
	u32	total_bytes;	// number of bytes
	u8	dir;		// direction 0 = OUT from host to device, B7 = IN from device to host
	u8	lun;		// device logical unit number
	u8	cmd_len;	// command length
	u8	command[16];	// command block
} sCbw;
#define MSC_CBW_SIGNATURE 0x43425355	// CBW signature (little endian = Intel)

// command status wrapper (13 bytes)
typedef struct PACKED {
	u32	signature;	// signature
	u32	tag;		// tag sent by host
	u32	data_residue;	// device report difference of amount of data (data not transferred)
	u8	status;		// status
} sCsw;
#define MSC_CSW_SIGNATURE 0x53425355	// CSW signature (little endian = Intel)

// SCSI Read 10 and Write 10 command (10 bytes)
typedef struct PACKED {
	u8	cmd_code;	// SCSI opcode
	u8	res;		// ... reserved
	u32	lba;		// first LBA address (in Big Endian format = Motorola)
	u8	res2;		// ... reserved
	u16	block_count;	// number of blocks (in Big Endian format = Motorola)
	u8	control;
} sScsiRW10;

// SCSI Read Capacity 10 response data (8 bytes)
typedef struct PACKED {
	u32	last_lba;	// last logical block address (in Big Endian format = Motorola)
	u32	block_size;	// block size in bytes (in Big Endian format = Motorola)
} sScsiCap10;

// SCSI Read Format Capacity (12 bytes)
typedef struct PACKED {
	u8	res[3];		// ... reserved
	u8	list_len;	// length in bytes of formattable capacity descriptor followed it (must be 8*n)
	u32	block_num;	// number of logical blocks (in Big Endian format = Motorola)
	u8	desc_type;	// descriptor type: 1=informatted, 2=formatted, 3=no media
	u8	res2;		// reserved
	u16	block_size;	// block size (in Big Endian format = Motorola)
} sScsiFormCap;

// SCSI Inquiry Response Data (36 bytes)
typedef struct PACKED {
	u8	type;		// bit 0..4: device type, bit 5..7: peripheral qualifier
	u8	removable;	// bit 7: device is removable
	u8	version;	// version
	u8	flags1;		// bit 0..3: response data format, bit 4: hierarchival support, bit 5: normal aca
	u8	add_length;	// additional length
	u8	flags2;		// bit 0: protect, bit 3: third party copy, bit 4..5: target port group support,
				//    bit 6: access control coordinator, bit 7: scc support
	u8	flags3;		// bit 0: addr16, bit 4: multi port, bit 6: enclosure service
	u8	flags4;		// bit 1: cmd que, bit 4: sync, bit 5: wbus16
	u8	vendor_id[8];	// vendor ASCII id (space padded)
	u8	product_id[16];	// product ASCII id (space padded)
	u8	product_rev[4];	// product ASCII revision (space padded)
} sScsiInquiry;

// SCSI mode parameter 6 (4 bytes)
typedef struct PACKED {
	u8	data_len;	// data length
	u8	medium_type;	// medium type
	u8	protect;	// bit 7: write protected
	u8	desc_len;	// block descriptor length
} sScsiMode6;

// SCSI get sense response data (18 bytes)
typedef struct PACKED {
	u8	resp_code;	// bit 0..6: response code (0x70 current errors, 0x71 deferred errors), bit 7: valid
	u8	res;		// ... reserved
	u8	sense_key;	// bit 0..3: sense key, bit 5: incorrect length indicator, bit 6: end of medium, bit 7: filemark
	u32	information;	// information
	u8	sense_len;	// additional sense length
	u32	cmd_info;	// command specific info
	u8	sense_code;	// sense code
	u8	sense_qualifier; // sense qualifier
	u8	repl_code;	// field replaceable unit code
	u8	key_spec[3];	// sense key specific (valid if bit 7 of key_spec[0])
} sScsiSense;

// MSC device interface
typedef struct {
	u8	itf_num;	// interface number
	u8	ep_in;		// input endpoint
	u8	ep_out;		// output endpoint
	u8	stage;		// bulk stage
	u8	sense_key;	// sense response key
	u8	sense_code;	// sense response code
	u8	sense_qualifier; // sense response qualifier
	u32	total_len;	// bulk total length
	u32	xfer_len;	// bulk transferred length
	sCbw	cbw;		// command block wrapper
	sCsw	csw;		// command status wrapper
} sMscdItf;

// MSC device interface
extern sMscdItf MscdItf;

// MSC buffer size (must accommodate 1 disk sector)
#define MSC_DATA_BUFSIZE	512
extern u8 MscDataBuf[MSC_DATA_BUFSIZE];

// logical unit number - number of devices (= max. unit number + 1)
// - MscdLun should be set before initialize USB class driver
extern u8 MscdLun;

// MSC read data from device callback (NULL=not used)
// - MscdRead10Cb should be set before initialize USB class driver
extern pMscdRead10Cb MscdRead10Cb;

// MSC write data to device callback (NULL=not used)
// - MscdWrite10Cb should be set before initialize USB class driver
extern pMscdWrite10Cb MscdWrite10Cb;

// MSC inquiry callback (NULL=not used)
// - MscdInquiryCb should be set before initialize USB class driver
extern pMscdInquiryCb MscdInquiryCb;

// MSC device ready callback (returns False if not ready; NULL=not used)
// - MscdReadyCb should be set before initialize USB class driver
extern pMscdReadyCb MscdReadyCb;

// MSC get device capacity callback (NULL=not used)
// - MscdGetCapacityCb should be set before initialize USB class driver
extern pMscdGetCapacityCb MscdGetCapacityCb;

// MSC process SCSI command callback (NULL=not used)
// - MscdScsiCmdCb should be set before initialize USB class driver
extern pMscdScsiCmdCb MscdScsiCmdCb;

// MSC Read10 command complete callback
// - MscdRead10Comp should be set before initialize USB class driver
extern pMscdRead10Comp MscdRead10Comp;

// MSC Write10 command complete callback
// - MscdWrite10Comp should be set before initialize USB class driver
extern pMscdWrite10Comp MscdWrite10Comp;

// MSC SCSI command complete callback
// - MscdScsiComp should be set before initialize USB class driver
extern pMscdScsiComp MscdScsiComp;

// MSC check if medium is writable callback (NULL=not used)
// - MscdIsWritableCb should be set before initialize USB class driver
extern pMscdIsWritableCb MscdIsWritableCb;

// get LBA address from Read10/Write10 command (LBA is in Big Endian format = Motorola)
INLINE u32 MscdRW10GetLba(const u8* cmd) { return Endian(((const sScsiRW10*)cmd)->lba); }

// get blok count from Read10/Write10 command (block count is in Big Endian format = Motorola)
INLINE u16 MscdRW10GetCount(const u8* cmd) { return Swap(((const sScsiRW10*)cmd)->block_count); }

// set sense
void MscdSetSense(u8 lun, u8 sense_key, u8 sense_code, u8 sense_qualifier);

// set sense if sense_key = 0: device not ready, cause not reportable
void MscdSetSenseNotReady(u8 lun);

// set sense: illegal request (Invalid Command Operation)
void MscdSetSenseIllegal(u8 lun);

// initialize class driver
// - MscdLun and callbacks should be set before initialize USB class driver
void MscdInit();

// reset class driver
void MscdReset();

// open device class interface (returns size of used interface, 0=not supported)
u16 MscdOpen(const sUsbDescItf* itf, u16 max_len);

// control request callback (returns False to stall)
Bool MscdCtrlReq(u8 stage, const sUsbSetupPkt* setup);

// process built-in SCSI command (returns length of data copied to buffer or -1 = error)
//  lun ... logical unit number (0 .. MscdLun-1)
//  cmd ... SCSI command block (16 bytes)
//  buffer ... additional data buffer
//  bufsize ... buffer size
int MscdProcScsi(u8 lun, const u8* cmd, u8* buffer, u32 bufsize);

// transfer complete callback (returns False to stall)
Bool MscdXferComp(u8 epinx, u8 xres, u16 len);

// Read 10 command (read data from media)
void MscdRead10();

// Write 10 command (write data to media, request to get data from host)
void MscdWrite10();

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_MSCD_H

#endif // USE_USB_DEV_MSC	// use USB MSC Mass Storage Class (device)
#endif // USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
