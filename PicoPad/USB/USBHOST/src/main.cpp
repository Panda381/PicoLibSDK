
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
/*
Sample of saved log:
-------------------
Speed: Low Speed 1.5 Mbps
MaxPkt=8 B
Address=1
DevDesc=0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08, 0xf2, 0x04, 0x08, 0x04, 0x65, 0x01, 0x01, 0x02,
	0x00, 0x01
CfgDesc len=59, real len=59
CfgDesc=0x09, 0x02, 0x3b, 0x00, 0x02, 0x01, 0x00, 0xa0, 0x32, 0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x01,
	0x01, 0x00, 0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, 0x41, 0x00, 0x07, 0x05, 0x81, 0x03, 0x08,
	0x00, 0x0a, 0x09, 0x04, 0x01, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x09, 0x21, 0x11, 0x01, 0x00,
	0x01, 0x22, 0x92, 0x00, 0x07, 0x05, 0x82, 0x03, 0x00, 0x00, 0x00
Desc list=C-I-CC-E-I-CC-E
Set Cfg=1
Devices: Keyb Hid2
*/

#include "../include.h"

#define TIMEOUT		1000	// transfer time-out in [ms]
#define DEV_ADDR	1	// device address (0 to 4 = USE_USB_HOST)
#define LOGFILE		"USB_LOG$.TXT" // log file

// current state
enum {
	STATE_OFF = 0,		// not connected
	STATE_GETMAXPKT,	// get MaxPkt
	STATE_ADDR,		// set device new address
	STATE_DEVDESC,		// get device descriptor
	STATE_CFGLEN,		// get length of configuration descriptor
	STATE_CHECKCFGLEN,	// check length of configuration descriptor
	STATE_CFGDESC,		// get configuration descriptor
	STATE_PARSE,		// parse configuration
	STATE_SETCFG,		// set configuration
	STATE_CFGCOMP,		// config complete
	STATE_LIST,		// list of devices
};

// errors
enum {
	ERR_OK = 0,		// all OK
	ERR_TIMEOUT,		// timeout error
	ERR_DATASEQ,		// data sequence error
	ERR_RXTIME,		// receive timeout
	ERR_STALL,		// stall from device
	ERR_CFGLEN,		// configuration descriptor is too big
	ERR_PARSE,		// parse error
	ERR_MOUNT,		// mounting error
};

u8 Error = ERR_OK;

const char* ErrText[] = {
	"OK",			// ERR_OK, all OK
	"time-out",		// ERR_TIMEOUT, timeout error
	"data-seq",		// ERR_DATASEQ, data sequence error
	"rx-timeout",		// ERR_RXTIME, receive timeout
	"stall",		// ERR_STALL, stall from device
	"cfg-len",		// ERR_CFGLEN, configuration descriptor is too large
	"parse",		// ERR_PARSE, parse error
	"mount",		// ERR_MOUNT, mounting error
};

// mark of device descriptor
#define DESCMARK_NUM	38
const char* DescMark[DESCMARK_NUM] = {
	"0",			// 0
	"D",			// 1 USB_DESC_DEVICE
	"C",			// 2 USB_DESC_CONFIGURATION
	"S",			// 3 USB_DESC_STRING
	"I",			// 4 USB_DESC_INTERFACE
	"E",			// 5 USB_DESC_ENDPOINT
	"QU",			// 6 USB_DESC_DEVICE_QUALIFIER
	"OS",			// 7 USB_DESC_OTHER_SPEED_CONFIG
	"PW",			// 8 USB_DESC_INTERFACE_POWER
	"OT",			// 9 USB_DESC_OTG
	"DB",			// 10 USB_DESC_DEBUG
	"A",			// 11 USB_DESC_INTERFACE_ASSOCIATION
	"12",			// 12
	"13",			// 13
	"14",			// 14
	"B",			// 15 USB_DESC_BOS
	"CA",			// 16 USB_DESC_DEVICE_CAPABILITY
	"17",			// 17
	"18",			// 18
	"19",			// 19
	"20",			// 20
	"21",			// 21
	"22",			// 22
	"23",			// 23
	"24",			// 24
	"25",			// 25
	"26",			// 26
	"27",			// 27
	"28",			// 28
	"29",			// 29
	"30",			// 30
	"31",			// 31
	"32",			// 32
	"CD",			// 33 USB_DESC_CS_DEVICE
	"CC",			// 34 USB_DESC_CS_CONFIGURATION
	"CS",			// 35 USB_DESC_CS_STRING
	"CI",			// 36 USB_DESC_CS_INTERFACE
	"CE",			// 37 USB_DESC_CS_ENDPOINT
};

int State;

// current speed
//#define USB_SPEED_OFF	0	// disconnected
//#define USB_SPEED_LS	1	// low speed
//#define USB_SPEED_FS	2	// full speed
u8 Speed;
const char* SpeedText[4] = { "Not connected", "Low Speed 1.5 Mbps", "Full Speed 12 Mbps", "-" };

// device max packet size
u16 DevMaxPkt;

// device descriptor
sUsbDescDev DescDev;

// full configuration descriptor
u16 DescCfgTotalLen;
u16 DescCfgTotalLenReal;
ALIGNED u8 DescCfg[USB_HOSTSETUPBUFF_MAX];

// print log to SD
void SaveLog()
{
	int i, n;
	sUsbHostHidInter* hid;

	PrintText("Saving...           \r");

	// mount disk
	if (!DiskAutoMount())
	{
		PrintText("No SD card!         \r");
		return;
	}

	// delete old file
	FileDelete(LOGFILE);

	// create file
	sFile f;
	if (!FileCreate(&f, LOGFILE))
	{
		PrintText("Cannot create file! \r");
		return;
	}

	if (State == STATE_OFF)
		FilePrintText(&f, "Not connected\n");

	if (State > STATE_OFF)
		FilePrint(&f, "Speed: %s\n", SpeedText[Speed]);

	if (State > STATE_GETMAXPKT)
		FilePrint(&f, "MaxPkt=%d B\n", DevMaxPkt);

	if (State > STATE_ADDR)
		FilePrint(&f, "Address=%d\n", DEV_ADDR);

	if (State > STATE_DEVDESC)
		FilePrint(&f, "DevDesc=%.*m\n", sizeof(sUsbDescDev), &DescDev);

	if (State > STATE_CFGLEN)
		FilePrint(&f, "CfgDesc len=%d, real len=%d\n", DescCfgTotalLen, DescCfgTotalLenReal);

	if (State > STATE_CFGDESC)
	{
		FilePrint(&f, "CfgDesc=%.*m\n", DescCfgTotalLen, DescCfg);
		FilePrintText(&f, "Desc list=");

		// display list of descriptors
		// D device, C config, I interface, E endpoint, A association, C* class specific
		n = DescCfgTotalLen;
		i = 0;
		while (i < n)
		{
			// invalid descriptor
			if (USB_DESC_LEN(DescCfg+i) < 3) break;

			// mark "-"
			if (i > 0) FilePrintChar(&f, '-');

			// print mark of the desscriptor
			if (USB_DESC_TYPE(DescCfg+i) >= DESCMARK_NUM)
				FilePrintChar(&f, '?');
			else
				FilePrintText(&f, DescMark[USB_DESC_TYPE(DescCfg+i)]);

			// next descriptor
			i += USB_DESC_LEN(DescCfg+i);
		}
		FilePrintChar(&f, '\n');
	}

	if (State > STATE_SETCFG)
		FilePrint(&f, "Set Cfg=%d\n", ((sUsbDescCfg*)DescCfg)->config);

	if (State > STATE_CFGCOMP)
	{
		FilePrint(&f, "Devices:");

		// list of COM devices
		for (i = 0; i < USE_USB_HOST_CDC; i++)
		{
			if (UsbHostCdcInxIsMounted(i))
			{
				if (i > 0)
					FilePrint(&f, " COM%d", i+1);
				else
					FilePrintText(&f, " COM");
			}
		}

		// list of HID devices
		for (i = 0; i < USE_USB_HOST_HID; i++)
		{
			if (UsbHostHidInxIsMounted(i))
			{
				hid = &UsbHostHidInter[i];
				if (hid->protocol == USB_HID_ITF_KEYB)
				{
					if (i > 0)
						FilePrint(&f, " Keyb%d", i+1);
					else
						FilePrintText(&f, " Keyb");
				}
				else if (hid->protocol == USB_HID_ITF_MOUSE)
				{
					if (i > 0)
						FilePrint(&f, " Mouse%d", i+1);
					else
						FilePrintText(&f, " Mouse");
				}
				else
				{
					if (i > 0)
						FilePrint(&f, " Hid%d", i+1);
					else
						FilePrintText(&f, " Hid");
				}
			}
		}
		FilePrintChar(&f, '\n');
	}

	// flush file writes and flush disk buffers (returns False on error)
	if (!FileFlush(&f))
	{
		PrintText("Error write file!   \r");
		return;
	}

	// close file
	if (!FileClose(&f))
	{
		PrintText("Error close file!   \r");
		return;
	}

	// all OK
	PrintText("Saved OK            \r");
}

// wait transfer (returns False on error)
Bool TransWait();

// control transfer complete (returns False on error)
Bool TransComp()
{
	// get descriptor
	sEndpoint* sep = &UsbEndpoints[0];

	// device address (0 or 1)
	u8 dev_addr = sep->dev_addr;

	// device endpoint index (0 or 1)
	u8 dev_epinx = sep->dev_epinx;

	// length of transferred data
	u16 len = sep->xfer_len;

	// clear active flag
	sep->active = False;

	// get setup packet with request
	const sUsbSetupPkt* setup = &UsbSetupRequest;

	// check transfer stage
	switch(UsbHostSetupStage)
	{
	// setup stage
	case USB_STAGE_SETUP:

		// check if any data in data stage is required
		if (setup->length > 0)
		{
			// go to data stage
			UsbHostSetupStage = USB_STAGE_DATA;

			// prepare required direction on endpoint 0
			u8 dev_epinx = USB_EPINX(0, USB_EPADDR_DIR(setup->type));

			// transfer data
			UsbHostXferStart(dev_addr, dev_epinx, UsbSetupDataBuff, setup->length, False);

			// wait transfer (returns False on error)
			return TransWait();
		}

		// if setup->length == 0, continue to DATA stage case

	// data stage
	case USB_STAGE_DATA:
		{
			// store data length
			UsbSetupDataXfer = len;

			// go to acknowledge stage
			UsbHostSetupStage = USB_STAGE_ACK;

			// prepare opposite direction on endpoint 0
			u8 dev_epinx = USB_EPINX(0, USB_EPADDR_DIR(setup->type) ^ 1);

			// transfer zero length packet ZLP to mark end of transmission (endpoint number is 0, oposite direction)
			// Note: status packet is always DATA1 - this is ensured by UsbHostEpInit after changing direction.
			UsbHostXferStart(dev_addr, dev_epinx, NULL, 0, False);

			// wait transfer (returns False on error)
			return TransWait();
		}

	// acknowledge stage
	case USB_STAGE_ACK:

		// transfer complete OK
		return True;

	default:
		return False;
	}
}

// wait transfer (returns False on error)
Bool TransWait()
{
	// start time of operation
	u32 t = Time();

	while (True)
	{
		// timeout
		if ((s32)(Time() - t) >= TIMEOUT*1000)
		{
			Error = ERR_TIMEOUT;
			PrintText(ErrText[Error]);
			return False;
		}

		// get SIE status
		u32 status = *USB_SIE_STATUS;

		// data sequence error
		if ((status & B31) != 0)
		{
			RegClr(USB_SIE_STATUS, B31);
			Error = ERR_DATASEQ;
			PrintText(ErrText[Error]);
			return False;
		}

		// receive timeout
		if ((status & B27) != 0)
		{
			RegClr(USB_SIE_STATUS, B27);
			Error = ERR_RXTIME;
			PrintText(ErrText[Error]);
			return False;
		}

		// stall from device
		if ((status & B29) != 0)
		{
			RegClr(USB_SIE_STATUS, B29);
			Error = ERR_STALL;
			PrintText(ErrText[Error]);
			return False;
		}

		// transfer complete of SETUP packet
		if ((status & B18) != 0)
		{
			// clear interrupt flag SIE_STATUS.TRANS_COMPLETE
			RegClr(USB_SIE_STATUS, B18);

			// SETUP was sent (B1 = send setup packet)
			if ((*USB_SIE_CTRL & B1) != 0) // _SEND_SETUP
			{
				// short delay (FS min 5 us, LS min. 200 us) to allow the device to prepare to receive next SETUP packet.
				WaitUs(400);

				// proces "transfer complete" of setup packet
				UsbEndpoints[0].xfer_len = USB_SETUP_PKT_SIZE;
				return TransComp();
			}
		}

		// transfer complete od data buffers (directions IN and OUT)
		if ((*USB_BUFF_STATUS & 3) != 0)
		{
			// reset buffer bit
			RegClr(USB_BUFF_STATUS, 3);

			// continue transfer (returns True if transfer is complete)
			if (UsbXferCont(0))
			{
				// process "transfer complete" event
				return TransComp();
			}
		}
	}
}

// main function
int main()
{
	u8 speed;
	sUsbDescDev* desc_dev;
	sUsbDescCfg* desc_cfg;
	sUsbHostDev* dev;
	sUsbSetupPkt* setup;
	sUsbHostHidInter* hid;
	int i, n;

// restart new connection
Restart:

	// USB init in host mode
	UsbHostInit();

	// interrupt disable
	UsbIntDisable();

// ==== STATE_OFF: Not connected

	// reset current state
	State = STATE_OFF;
	PrintText("\fNot connected...");

	// reset error
	Error = ERR_OK;

// ==== Connected, device speed -> Speed

	// wait for connection
	do Speed = UsbGetDevSpeed(); while (Speed == USB_SPEED_OFF);
	Print("\fSpeed: %s\n", SpeedText[Speed]);

	// prepare interface	
	UsbHostDev[0].hub_addr = 0;
	UsbHostDev[0].hub_port = 0;
	UsbHostDev[0].hub_speed = Speed; // device speed

// ==== Reset device

	// reset signal 10 ms
	PrintText("Reset signal 10 ms...");
	RegSet(USB_SIE_CTRL, B13);
	WaitMs(10);
	PrintText("OK\n");

// ==== Delay to initialize device

	// delay 400 ms
	PrintText("Wait 400 ms...");
	WaitMs(400);
	PrintText("OK\n");

// ==== STATE_GETMAXPKT: Get max. packet size

	// shift state
	State++;

	// get MaxPkt
	PrintText("Get MaxPkt...");

	// initialize host control endpoint 0 to get 8 bytes from device descriptor (to get max packet size)
	UsbHostEp0Open(0, 8);

	// request to get part of device descriptor (8 bytes)
	UsbHostGetDesc(0, USB_DESC_DEVICE, 0, 0, &DescDev, 8, NULL);

	// wait transfer (returns False on error)
	if (!TransWait()) goto Stopall;

	// pointer to received device descriptor
	desc_dev = &DescDev;

	// device max packet size
	DevMaxPkt = desc_dev->pktmax;

	// print packet size
	Print("%d B\n", DevMaxPkt);

// ==== STATE_ADDR: Set device address to "1"

	// shift state
	State++;

	// set address
	PrintText("Set address...");

	// setup device on new address
	UsbHostEnumAddr = DEV_ADDR;
	dev = &UsbHostDev[DEV_ADDR];
	dev->hub_addr = 0;			// hub address (0 = root)
	dev->hub_port = 0;			// hub port
	dev->hub_speed = Speed;			// device speed
	dev->flag = USB_HOST_FLAG_CONN;		// device is connected
	dev->pktmax = DevMaxPkt;		// max. packet size for endpoint 0

	// pointer to setup packet
	setup = &UsbSetupRequest;

	// prepare setup request
	setup->type =	0 |				// direction: OUT
			(USB_REQ_TYPE_STANDARD << 5) |	// request type: standard
			USB_REQ_RCPT_DEVICE;		// recipient: device
	setup->request = USB_REQ_SET_ADDRESS;		// request: set address
	setup->value = DEV_ADDR;			// value: new address
	setup->index = 0;				// index: 0
	setup->length = 0;				// length of data: 0

	// start control transfer with callback (device address is = 0)
	UsbHostCtrlXfer(0, NULL, NULL);

	// wait transfer (returns False on error)
	if (!TransWait()) goto Stopall;
	PrintText("OK\n");

	// delay 10 ms to device change its address
	WaitMs(10);

// ==== STATE_DEVDESC: Get device descriptor

	// shift state
	State++;

	// get device descriptor
	PrintText("Get DevDesc...");

	// close all open endpoints belonging to device 0
	UsbHostDevClose(0);

	// open control pipe for new address
	UsbHostEp0Open(DEV_ADDR, DevMaxPkt);

	// request to get full device descriptor
	UsbHostGetDesc(DEV_ADDR, USB_DESC_DEVICE, 0, 0, &DescDev, sizeof(sUsbDescDev), NULL);

	// wait transfer (returns False on error)
	if (!TransWait()) goto Stopall;

	// display info
	Print("cls=%d sub=%d prot=%d\n",
		desc_dev->dev_class, desc_dev->dev_subclass, desc_dev->dev_protocol);

// ==== STATE_CFGLEN: Get length of configuration descriptor

	// shift state
	State++;

	// get length of configuration descriptor
	PrintText("Get CfgDesc len...");

	// get base configuration descriptor, index 0
	UsbHostGetDesc(DEV_ADDR, USB_DESC_CONFIGURATION, 0, 0, DescCfg, sizeof(sUsbDescCfg), NULL);

	// wait transfer (returns False on error)
	if (!TransWait()) goto Stopall;

	// pointer to received device descriptor
	desc_cfg = (sUsbDescCfg*)&DescCfg;

	// total length of configuration descriptor
	DescCfgTotalLen = desc_cfg->totallen;
	DescCfgTotalLenReal = UsbSetupDataXfer;

	// print total length of configuration descriptor
	Print("%d B\n", DescCfgTotalLen);

// ==== STATE_CHECKCFGLEN: check length of configuration descriptor

	// shift state
	State++;

	// configuration is too big
	if (DescCfgTotalLen > USB_HOSTSETUPBUFF_MAX)
	{
		PrintText("ERROR: CfgDesc it too big!\n");
		Error = ERR_CFGLEN;
		goto Stopall;
	}

// ==== STATE_CFGDESC: get configuration descriptor

	// shift state
	State++;

	// get configuration descriptor
	PrintText("Get CfgDesc...");

	// get full configuration descriptor, index 0
	UsbHostGetDesc(DEV_ADDR, USB_DESC_CONFIGURATION, 0, 0, DescCfg, DescCfgTotalLen, NULL);

	// wait transfer (returns False on error)
	if (!TransWait()) goto Stopall;

	// display list of descriptors
	// D device, C config, I interface, E endpoint, A association, C* class specific
	n = DescCfgTotalLen;
	i = 0;
	while (i < n)
	{
		// invalid descriptor
		if (USB_DESC_LEN(DescCfg+i) < 3) break;

		// mark "-"
		if (i > 0) PrintChar('-');

		// print mark of the desscriptor
		if (USB_DESC_TYPE(DescCfg+i) >= DESCMARK_NUM)
			PrintChar('?');
		else
			PrintText(DescMark[USB_DESC_TYPE(DescCfg+i)]);

		// next descriptor
		i += USB_DESC_LEN(DescCfg+i);
	}
	PrintChar('\n');

// ==== STATE_PARSE: parse configuration

	// shift state
	State++;

	// parse configuration descriptor
	PrintText("Parsing...");

	// parse configuration
	if (!UsbHostParseCfg(DEV_ADDR, DescCfg))
	{
		PrintText("ERROR\n");
		Error = ERR_PARSE;
		goto Stopall;
	}
	PrintText("OK\n");

// ==== STATE_SETCFG: set configuration

	// shift state
	State++;

	// set configuration
	i = ((sUsbDescCfg*)DescCfg)->config;
	Print("Set Cfg %d...", i);

	// prepare setup request
	setup->type =	0 |				// direction: OUT
			(USB_REQ_TYPE_STANDARD << 5) |	// request type: standard
			USB_REQ_RCPT_DEVICE;		// recipient: device
	setup->request = USB_REQ_SET_CONFIGURATION;	// request: select configuration
	setup->value = i;				// value: configuration number (1-based)
	setup->index = 0;				// index: 0
	setup->length = 0;				// length of data: 0

	// start control transfer with callback
	UsbHostCtrlXfer(DEV_ADDR, NULL, NULL);

	// wait transfer (returns False on error)
	if (!TransWait()) goto Stopall;
	PrintText("OK\n");

// ==== STATE_CFGCOMP: config complete

	// shift state
	State++;

	// configuration complete
	PrintText("Cfg Complete...");

	// mark device as configured
	dev->flag |= USB_HOST_FLAG_CONF;

	// set config complete
	UsbHostCfgComp(DEV_ADDR, (u8)-1);

	// check result
	if (UsbIsMounted())
	{
		PrintText("ERROR\n");
		Error = ERR_MOUNT;
		goto Stopall;
	}
	PrintText("OK\n");

//===== STATE_LIST: list of devices

	// shift state
	State++;

	// restart enumeration
	UsbHostInit();

	// list of devies
	PrintText("Devices:");

	// wait for enumeration
	WaitMs(1000);

	// list of COM devices
	for (i = 0; i < USE_USB_HOST_CDC; i++)
	{
		if (UsbHostCdcInxIsMounted(i))
		{
			if (i > 0)
				Print(" COM%d", i+1);
			else
				PrintText(" COM");
		}
	}

	// list of HID devices
	for (i = 0; i < USE_USB_HOST_HID; i++)
	{
		if (UsbHostHidInxIsMounted(i))
		{
			hid = &UsbHostHidInter[i];
			if (hid->protocol == USB_HID_ITF_KEYB)
			{
				if (i > 0)
					Print(" Keyb%d", i+1);
				else
					PrintText(" Keyb");
			}
			else if (hid->protocol == USB_HID_ITF_MOUSE)
			{
				if (i > 0)
					Print(" Mouse%d", i+1);
				else
					PrintText(" Mouse");
			}
			else
			{
				if (i > 0)
					Print(" Hid%d", i+1);
				else
					PrintText(" Hid");
			}
		}
	}
	PrintChar('\n');

// end of operation, wait for a key press
Stopall:

	PrintText("\nA=save to SD, B=restart, Y=quit\n");

	while(True)
	{
		switch (KeyGet())
		{
		// save to SD
		case KEY_A:
			SaveLog();
			break;

		// restart
		case KEY_B:
			goto Restart;

#if USE_SCREENSHOT		// use screen shots
		case KEY_X:
			ScreenShot();
			break;
#endif

		// reset to boot loader
		case KEY_Y:
			ResetToBootLoader();
			break;
		}
	}
}
