
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

#include "../../global.h"	// globals

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#if USE_USB_DEV_MSC	// use USB MSC Mass Storage Class (device)

#include "../inc/sdk_cpu.h"
#include "../usb_inc/sdk_usb_phy.h" // physical layer
#include "../usb_inc/sdk_usb_dev.h"
#include "../usb_inc/sdk_usb_dev_msc.h"
#include "../usb_inc/sdk_usb_setupd.h"

// MSC device interface
sMscdItf MscdItf;

// MSC buffer size (must accommodate 1 disk sector)
u8 MscDataBuf[MSC_DATA_BUFSIZE];

// logical unit number - number of devices (= max. unit number + 1)
// - MscdLun should be set before initialize USB class driver
u8 MscdLun = 1;

// MSC read data from device callback (NULL=not used)
// - MscdRead10Cb should be set before initialize USB class driver
pMscdRead10Cb MscdRead10Cb = NULL;

// MSC write data to device callback (NULL=not used)
// - MscdWrite10Cb should be set before initialize USB class driver
pMscdWrite10Cb MscdWrite10Cb = NULL;

// MSC inquiry callback (NULL=not used)
// - MscdInquiryCb should be set before initialize USB class driver
pMscdInquiryCb MscdInquiryCb = NULL;

// MSC device ready callback (NULL=not used)
// - MscdReadyCb should be set before initialize USB class driver
pMscdReadyCb MscdReadyCb = NULL;

// MSC get device capacity callback (NULL=not used)
// - MscdGetCapacityCb should be set before initialize USB class driver
pMscdGetCapacityCb MscdGetCapacityCb = NULL;

// MSC process SCSI command callback (NULL=not used)
// - MscdScsiCmdCb should be set before initialize USB class driver
pMscdScsiCmdCb MscdScsiCmdCb = NULL;

// MSC Read10 command complete callback
// - MscdRead10Comp should be set before initialize USB class driver
pMscdRead10Comp MscdRead10Comp = NULL;

// MSC Write10 command complete callback
// - MscdWrite10Comp should be set before initialize USB class driver
pMscdWrite10Comp MscdWrite10Comp = NULL;

// MSC SCSI command complete callback
// - MscdScsiComp should be set before initialize USB class driver
pMscdScsiComp MscdScsiComp = NULL;

// MSC check if medium is writable callback (NULL=not used)
// - MscdIsWritableCb should be set before initialize USB class driver
pMscdIsWritableCb MscdIsWritableCb = NULL;

// set sense
void MscdSetSense(u8 lun, u8 sense_key, u8 sense_code, u8 sense_qualifier)
{
	MscdItf.sense_key = sense_key;
	MscdItf.sense_code = sense_code;
	MscdItf.sense_qualifier = sense_qualifier;
}

// set sense if sense_key = 0: device not ready, cause not reportable
void MscdSetSenseNotReady(u8 lun)
{
	if (MscdItf.sense_key == 0) MscdSetSense(lun, SCSI_SENSE_NOT_READY, 4, 0);
}

// set sense: illegal request (Invalid Command Operation)
void MscdSetSenseIllegal(u8 lun)
{
	MscdSetSense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0);
}

// initialize class driver
// - MscdLun and callbacks should be set before initialize USB class driver
void MscdInit()
{
	memset(&MscdItf, 0, sizeof(MscdItf));
}

// reset class driver
void MscdReset()
{
	MscdInit();
}

// open device class interface (returns size of used interface, 0=not supported)
u16 MscdOpen(const sUsbDescItf* itf, u16 max_len)
{
	// only support SCSI BOT protocol (Bulk-Only SCSI transport)
	if ((itf->itf_class != USB_CLASS_MSC) || // MSC class
		(itf->itf_subclass != 6) || // SCSI subclass
		(itf->itf_protocol != 0x50)) // Bulk-Only transport
		return 0;

	// MSC driver interface length is fixed
	u16 drv_len = sizeof(sUsbDescItf) + 2*sizeof(sUsbDescEp);
	if (drv_len > max_len) return 0;

	// get interface number
	MscdItf.itf_num = itf->itf_num;

	// open endpoint pair with bulk transfer
	if (!DcdOpenEpPair(USB_DESC_NEXT(itf), 2, USB_XFER_BULK, &MscdItf.ep_out, &MscdItf.ep_in)) return 0;

	// prepare for command block wrapper
	UsbXferStart(MscdItf.ep_out, &MscdItf.cbw, sizeof(sCbw), False);

	return drv_len;
}

// control request callback (returns False to stall)
Bool MscdCtrlReq(u8 stage, const sUsbSetupPkt* setup)
{
	// check class
	if (((setup->type >> 5) & 3) != USB_REQ_TYPE_CLASS) return False;

	switch (setup->request)
	{
	// reset MSC interface
	case 255:
		// acknowledging at Status Stage
		UsbDevSetupAck();
		break;

	// get max LUN (max. logical unit number)
	case 254:
		{
			// max. lun = lun count - 1
			u8 maxlun = MscdLun - 1;
			UsbDevSetupStart(&maxlun, 1);
		}
		break;

	// unsupported, stall
	default:
		return False;
	}

	return True;
}

// process built-in SCSI command (returns length of data copied to buffer or -1 = error)
//  lun ... logical unit number (0 .. MscdLun-1)
//  cmd ... SCSI command block (16 bytes)
//  buffer ... additional data buffer
//  bufsize ... buffer size
int MscdProcScsi(u8 lun, const u8* cmd, u8* buffer, u32 bufsize)
{
	// result
	int res = -1;

	// command code
	switch (cmd[0])
	{
	// check if device is ready to transfer data
	case SCSI_CMD_TEST_UNIT_READY:
		res = 0; // if device is ready, result is OK

		// device not ready, return -1
		if ((MscdReadyCb != NULL) && !MscdReadyCb(lun))
		{
			// failed status response
			res = -1;

			// if sense key is not set by callback, default to Logical Unit Not Ready, Cause Not Reportable
			MscdSetSenseNotReady(lun);
		}
		break;

	// start/stop device (not supported, but not error)
	case SCSI_CMD_START_STOP_UNIT:
		res = 0;
		break;

	// read capacity
	case SCSI_CMD_READ_CAPACITY_10:
		if (sizeof(sScsiCap10) <= bufsize)
		{
			// load device capacity
			u32 block_count = 0;
			u16 block_size = 0;
			if (MscdGetCapacityCb != NULL) MscdGetCapacityCb(lun, &block_count, &block_size);

			// invalid, unit is not ready
			if ((block_count == 0) || (block_size == 0))
			{
				// if sense key is not set by callback, default to Logical Unit Not Ready, Cause Not Reportable
				MscdSetSenseNotReady(lun);
			}
			else
			{
				// prepare "read capacity 10 response data"
				sScsiCap10 cap;
				cap.last_lba = Endian(block_count - 1);  // last logicak block address (in Big Endian format = Motorola)
				cap.block_size = Endian((u32)block_size); // block size in bytes (in Big Endian format = Motorola)

				// write data to buffer
				res = sizeof(sScsiCap10);
				UsbMemcpy(buffer, &cap, sizeof(sScsiCap10));
			}
		}
		break;

	// get list of possible format capacities - returns 1 current format
	case SCSI_CMD_READ_FORMAT_CAPACITY:
		if (sizeof(sScsiFormCap) <= bufsize)
		{
			// load device capacity
			u32 block_count = 0;
			u16 block_size = 0;
			if (MscdGetCapacityCb != NULL) MscdGetCapacityCb(lun, &block_count, &block_size);

			// invalid, unit is not ready
			if ((block_count == 0) || (block_size == 0))
			{
				// if sense key is not set by callback, default to Logical Unit Not Ready, Cause Not Reportable
				MscdSetSenseNotReady(lun);
			}
			else
			{
				// prepare descriptor
				sScsiFormCap cap;
				cap.list_len = 8; // only 1 descriptor follows
				cap.block_num = Endian(block_count);  // number of logical blocks (in Big Endian format = Motorola)
				cap.desc_type = 2; // descriptor type: formatted media
				cap.block_size = Swap(block_size);  // block size (in Big Endian format = Motorola)

				// write data to buffer
				res = sizeof(sScsiFormCap);
				UsbMemcpy(buffer, &cap, sizeof(sScsiFormCap));
			}
		}
		break;

	// get basic information from device - get vendor and product
	case SCSI_CMD_INQUIRY:
		if (sizeof(sScsiInquiry) <= bufsize)
		{
			// prepare descriptor
			sScsiInquiry inq;
			memset(&inq, 0, sizeof(sScsiInquiry));
			inq.removable = B7; // device is removable
			inq.version = 2; // version = 2
			inq.flags1 = 2; // response data format = 2
			memset(inq.vendor_id, ' ', sizeof(inq.vendor_id));
			memset(inq.product_id, ' ', sizeof(inq.product_id));
			memset(inq.product_rev, ' ', sizeof(inq.product_rev));

			// inquiry
			if (MscdInquiryCb != NULL) MscdInquiryCb(lun, inq.vendor_id, inq.product_id, inq.product_rev);

			// write data to buffer
			res = sizeof(sScsiInquiry);
			UsbMemcpy(buffer, &inq, sizeof(sScsiInquiry));
		}
		break;

	// report parameters from host - check if medium is writable
	case SCSI_CMD_MODE_SENSE_6:
		if (sizeof(sScsiMode6) <= bufsize)
		{
			// prepare descriptor
			sScsiMode6 desc;
			desc.data_len = 3;	// data length
			desc.medium_type = 0;	// medium type
			desc.protect = 0;	// bit 7: write protected
			desc.desc_len = 0;	// block descriptor length

			// check if writable
			if (MscdIsWritableCb != NULL)
			{
				if (!MscdIsWritableCb(lun)) desc.protect = B7;
			}

			// write data to buffer
			res = sizeof(sScsiMode6);
			UsbMemcpy(buffer, &desc, sizeof(sScsiMode6));
		}
		break;

	// get sense data
	case SCSI_CMD_REQUEST_SENSE:
		if (sizeof(sScsiSense) <= bufsize)
		{
			// prepare descriptor
			sScsiSense sens;
			memset(&sens, 0, sizeof(sScsiSense));
			sens.resp_code = 0x70 | B7; // response code: current errors, valid
			sens.sense_key = MscdItf.sense_key; // sense response key
			sens.sense_code = MscdItf.sense_code; // sense response code
			sens.sense_qualifier = MscdItf.sense_qualifier; // sense response qualifier

			// write data to buffer
			res = sizeof(sScsiSense);
			UsbMemcpy(buffer, &sens, sizeof(sScsiSense));

			// clear sense data after copy
			MscdSetSense(lun, 0, 0, 0);
		}
		break;
	}
	return res;
}

// transfer complete callback (returns False to stall)
Bool MscdXferComp(u8 epinx, u8 xres, u16 len)
{
	// switch by bulk stage
	switch (MscdItf.stage)
	{
	// new CBW (command block wrapper) received - waiting command stage
	case MSC_STAGE_CMD:

		// complete IN while waiting for CMD is usually status of previous SCSI operation, ignore it
		if (epinx != MscdItf.ep_out) return True;

		// check success result and CBW signature
		if ((xres != USB_XRES_OK) || (len != sizeof(sCbw)) ||
			(MscdItf.cbw.signature != MSC_CBW_SIGNATURE)) return False;

		// prepare status
		MscdItf.csw.signature = MSC_CSW_SIGNATURE; // CSW signature
		MscdItf.csw.tag = MscdItf.cbw.tag; // tag sent by host
		MscdItf.csw.data_residue = 0;

		// parse command block and prepare data stage
		MscdItf.stage = MSC_STAGE_DATA; // next will be DATA stage
		MscdItf.total_len = MscdItf.cbw.total_bytes; // total bytes
		MscdItf.xfer_len = 0; // bulk transferred length

		// Read 10 command
		if (MscdItf.cbw.command[0] == SCSI_CMD_READ_10)
			MscdRead10();

		// Write 10 command
		else if (MscdItf.cbw.command[0] == SCSI_CMD_WRITE_10)
			MscdWrite10();

		// other SCSI commands
		else
		{
			// OUT direction (receive from host to device): queue transfer (invoke app callback after done)
			if ((MscdItf.cbw.total_bytes > 0) && ((MscdItf.cbw.dir & B7) == 0))
				UsbXferStart(MscdItf.ep_out, MscDataBuf, MscdItf.total_len, False);
			else
			{
				// process SCSI built-in commands
				int len = MscdProcScsi(MscdItf.cbw.lun, MscdItf.cbw.command, MscDataBuf, MSC_DATA_BUFSIZE);

				// not built-in, invoke user callback
				if ((len < 0) && (MscdItf.sense_key == 0) && (MscdScsiCmdCb != NULL))
					len = MscdScsiCmdCb(MscdItf.cbw.lun, MscdItf.cbw.command, MscDataBuf, MSC_DATA_BUFSIZE);

				// error
				if (len < 0)
				{
					// set failed status
					MscdItf.total_len = 0;
					MscdItf.csw.status = MSC_CSW_STATUS_FAILED;
					MscdItf.stage = MSC_STAGE_STATUS;

					// sense key is not set, set sense: illegal request (Invalid Command Operation)
					if (MscdItf.sense_key == 0) MscdSetSenseIllegal(MscdItf.cbw.lun);

					// stall bulk IN transfer if more data
					if (MscdItf.cbw.total_bytes > 0) UsbDevSetStall(MscdItf.ep_in);
				}

				// command processed OK
				else
				{
					// set result status
					MscdItf.total_len = len;
					MscdItf.csw.status = MSC_CSW_STATUS_PASSED;

					// start transfer if needed send data
					if (len > 0)
					{
						// cannot return more data than host expect
						if (len > MscdItf.cbw.total_bytes) return False;

						// start transfer data from device to host
						UsbXferStart(MscdItf.ep_in, MscDataBuf, len, False);
					}

					// no data to send, go to STATUS stage
					else
						MscdItf.stage = MSC_STAGE_STATUS;
				}
			}
		}
		break;

	// waiting data stage
	case MSC_STAGE_DATA:

		// OUT transfer (receive from host to device): invoke callback
		if ((MscdItf.cbw.dir & B7) == 0)
		{
			// other command than WRITE10
			if (MscdItf.cbw.command[0] != SCSI_CMD_WRITE_10)
			{
				// invoke user callback to process command
				int len = -1;
				if (MscdScsiCmdCb != NULL) len = MscdScsiCmdCb(MscdItf.cbw.lun, MscdItf.cbw.command, MscDataBuf, MscdItf.total_len);

				// error
				if (len < 0)
				{
					// set failed status
					MscdItf.csw.status = MSC_CSW_STATUS_FAILED;

					// set sense: illegal request (Invalid Command Operation)
					MscdSetSenseIllegal(MscdItf.cbw.lun);
				}

				// ok, set passed status
				else
					MscdItf.csw.status = MSC_CSW_STATUS_PASSED;
			}

			// command WRITE10 - write data to media
			else
			{
				// get block count
				u16 block_count = MscdRW10GetCount(MscdItf.cbw.command);
				if (block_count == 0) return False; // invalid block count

				// get block size
				u16 block_size = (u16)(MscdItf.cbw.total_bytes / block_count);
				if (block_size == 0) return False; // invalid block size

				// adjust LBA with already transferred bytes
				u32 lba = MscdRW10GetLba(MscdItf.cbw.command) + (MscdItf.xfer_len / block_size);

				// write data to media
				int n = -1;
				//  lun ... logical unit number (0 .. MscdLun-1)
				//  lba ... logical block address to be written
				//  off ... byte offset from LBA
				//  buf ... buffer
				//  bufsize ... requested bytes
				// Return number of bytes written (can be less than required) or 0 (if device is not ready) or -1 (if error, will stall).
				if (MscdWrite10Cb != NULL) n = MscdWrite10Cb(MscdItf.cbw.lun, lba, MscdItf.xfer_len % block_size, MscDataBuf, len);

				// error
				if (n < 0)
				{
					// set status failed
					MscdItf.csw.data_residue = MscdItf.cbw.total_bytes - MscdItf.xfer_len; // missing data size
					MscdItf.csw.status = MSC_CSW_STATUS_FAILED; // status failed
					MscdItf.stage = MSC_STAGE_STATUS; // go to status stage

					// set sense: illegal request (Invalid Command Operation)
					MscdSetSenseIllegal(MscdItf.cbw.lun);
					break;
				}

				// ok
				else
				{
					// application consume less than what we got (including zero)
					if (n < len)
					{
						// shift data in buffer
						if (n > 0)
						{
							MscdItf.xfer_len += n;
							memmove(MscDataBuf, MscDataBuf + n, len - n);
						}

						// simulate transfer complete so that this driver callback will be fired again
						UsbXferComplete(MscdItf.ep_out, len - n, USB_XRES_OK);
						return True;
					}
				}
			}
		}

		// accumulate data
		MscdItf.xfer_len += len;

		if (MscdItf.xfer_len >= MscdItf.total_len) // all data transferred
			MscdItf.stage = MSC_STAGE_STATUS; // data stage is complete, go to status stage
		else
		{
			// READ10 and WRITE10 can be executed with large bulk of data (write 8 KB in several flash write).
			// We break it into multiple smaller commands.
			if (MscdItf.cbw.command[0] == SCSI_CMD_READ_10)
				MscdRead10();
			else if (MscdItf.cbw.command[0] == SCSI_CMD_WRITE_10)
				MscdWrite10();
		}
		break;

	// waiting status stage (processed below)
	case MSC_STAGE_STATUS:
		break;

	// status has been sent
	case MSC_STAGE_SENT:

		// wait for status phase to complete
		if ((epinx == MscdItf.ep_in) && (len == sizeof(sCsw)))
		{
			// status stage is complete, go to command stage
			MscdItf.stage = MSC_STAGE_CMD;

			// queue for next CBW command block
			UsbXferStart(MscdItf.ep_out, &MscdItf.cbw, sizeof(sCbw), False);
		}
		break;
	}

	// process status stage
	if (MscdItf.stage == MSC_STAGE_STATUS)
	{
		// If either endpoint is stopped, need to wait until host clears it.
		if (UsbEndpoints[MscdItf.ep_in].stalled || UsbEndpoints[MscdItf.ep_out].stalled)
			// simulate transfer complete to be this callback fired again
			//  (use ep_out to prevent confusing with STATUS complete)
			UsbXferComplete(MscdItf.ep_out, 0, USB_XRES_OK);
		else
		{
			// invoke complete callback
			switch (MscdItf.cbw.command[0])
			{
			// read from device media
			case SCSI_CMD_READ_10:
				if (MscdRead10Comp != NULL) MscdRead10Comp(MscdItf.cbw.lun);
				break;

			// write to device media
			case SCSI_CMD_WRITE_10:
				if (MscdWrite10Comp != NULL) MscdWrite10Comp(MscdItf.cbw.lun);
				break;

			default:
				if (MscdScsiComp != NULL) MscdScsiComp(MscdItf.cbw.lun, MscdItf.cbw.command);
				break;
			}

			// go to Status Send stage
			MscdItf.stage = MSC_STAGE_SENT;

			// send SCSI status
			UsbXferStart(MscdItf.ep_in, &MscdItf.csw, sizeof(sCsw), False);
		}
	}

	return True;
}

// Read 10 command (read data from media)
void MscdRead10()
{
	// get block count
	u16 block_count = MscdRW10GetCount(MscdItf.cbw.command);
	if (block_count == 0) return; // invalid block count

	// get block size
	u16 block_size = (u16)(MscdItf.cbw.total_bytes / block_count);
	if (block_size == 0) return; // invalid block size

	// adjust LBA with already transferred bytes
	u32 lba = MscdRW10GetLba(MscdItf.cbw.command) + (MscdItf.xfer_len / block_size);

	// remaining bytes
	int n = MscdItf.cbw.total_bytes - MscdItf.xfer_len;
	if (n > MSC_DATA_BUFSIZE) n = MSC_DATA_BUFSIZE;

	// read data from media
	if (MscdRead10Cb != NULL)
		//  lun ... logical unit number (0 .. MscdLun-1)
		//  lba ... logical block address to be read
		//  off ... byte offset from LBA
		//  buf ... buffer
		//  bufsize ... requested bytes
		// Return number of bytes read (can be less than required) or 0 (if device is not ready) or -1 (if error, will stall).
		n = MscdRead10Cb(MscdItf.cbw.lun, lba, MscdItf.xfer_len % block_size, MscDataBuf, n);
	else
		n = -1;

	// error
	if (n < 0)
	{
		// set status failed
		MscdItf.csw.data_residue = MscdItf.cbw.total_bytes - MscdItf.xfer_len; // missing data size
		MscdItf.csw.status = MSC_CSW_STATUS_FAILED; // status failed

		// set sense "invalid command operation"
		MscdSetSense(MscdItf.cbw.lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0);

		// stall
		UsbDevSetStall(MscdItf.ep_in);
	}

	// device not ready
	else if (n == 0)
	{
		// simulate transfer complete so that this driver callback will be fired again
		UsbXferComplete(MscdItf.ep_in, 0, USB_XRES_OK);
	}

	// transfer is OK
	else
	{
		// send data to host
		UsbXferStart(MscdItf.ep_in, MscDataBuf, n, False);
	}
}

// Write 10 command (write data to media, request to get data from host)
void MscdWrite10()
{
	// check if medium is writable
	Bool writable = True;
	if (MscdIsWritableCb != NULL) writable = MscdIsWritableCb(MscdItf.cbw.lun);

	// not writable, report error
	if (!writable)
	{
		// set status failed
		MscdItf.csw.data_residue = MscdItf.cbw.total_bytes; // missing data size
		MscdItf.csw.status = MSC_CSW_STATUS_FAILED; // status failed

		// set sense "write protected"
		MscdSetSense(MscdItf.cbw.lun, SCSI_SENSE_DATA_PROTECT, 0x27, 0);

		// stall
		UsbDevSetStall(MscdItf.ep_out);
	}

	// writable
	else
	{
		// remaining bytes
		int n = MscdItf.cbw.total_bytes - MscdItf.xfer_len;
		if (n > MSC_DATA_BUFSIZE) n = MSC_DATA_BUFSIZE;

		// read data from host (Write10 callback will be called later from transfer complete)
		UsbXferStart(MscdItf.ep_out, MscDataBuf, n, False);
	}
}

#endif // USE_USB_DEV_MSC	// use USB MSC Mass Storage Class (device)
#endif // USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
