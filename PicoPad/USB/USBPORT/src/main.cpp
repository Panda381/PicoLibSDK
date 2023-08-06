
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define CHANNUM		7	// number of channels in one direction (max. 7)
#define RANDNUM		(56/4)	// number of random 32-bit DWORDs

sUsbPortPktTest SendPkt;
sUsbPortPktTest RecvPkt;

int main()
{
	int i, j, inx, send_ok, recv_ok, send_err, recv_err;
	u8 key;
	u8 port = 1;		// port (host: 1, device: 2)
	u8 port_send = 1;	// first channel in send direction (host: 1, device: 8)
	u8 port_recv = 8;	// first channel in receive direction (host: 8, device: 1)
	u32 cnt[16];		// sequence counters
	u8 res;
	Bool err;
	u32 t;			// report time

	SendPkt.len = 64;
	SendPkt.cmd = USBPORT_CMD_TEST;

Restart:
	// select menu
	Print("\fSelect port A=1 or B=2, Y=Quit\n");
	do {
		key = KeyGet();
		switch (key)
		{
		// host
		case KEY_A:	
			port = 1;
			port_send = 1;
			port_recv = 8;
			Print("Connecting as port 1 (Master)...");
			break;

		case KEY_B:
			port = 2;
			port_send = 8;
			port_recv = 1;
			Print("Connecting as port 2 (Slave)...");
			break;

		case KEY_Y:
			ResetToBootLoader();
			break;
		}
	} while ((key != KEY_A) && (key != KEY_B));

	// initialize device
	UsbPortInit(port);

	// reset sequence counters
	memset(cnt, 0, sizeof(cnt));
	send_ok = 0; // counter of packets sent OK
	recv_ok = 0; // counter of packets received OK
	send_err = 0; // counter of errors to send packet 
	recv_err = 0; // counter of errors to receive packet

	// wait for connection
	while (!UsbPortConnected())
	{
		if (KeyGet() == KEY_Y)
		{
			UsbPortTerm();
			goto Restart;
		}
	}

	// connected
	Print("OK\n");

	// wait some time to stabilise connection
	WaitMs(100);

	// report time
	t = Time();

	// main loop
	while (True)
	{
		// key get
		key = KeyGet();

#if USE_SCREENSHOT		// use screen shots
		if (key == KEY_X) ScreenShot();
#endif

		// disconnect or quit
		if (!UsbPortConnected() || (key == KEY_Y))
		{
			UsbPortTerm();
			goto Restart;
		}

		// send packets
		for (i = 0; i < CHANNUM; i++)
		{
			// channel index
			inx = port_send+i;

			// set sequence counter
			SendPkt.cnt = cnt[inx];

			// initialize random generator
			RandSet(cnt[inx]);

			// some shifts to fill up higher bytes
			RandShift();
			RandShift();
			RandShift();

			// fill-up packet data
			for (j = 0; j < USBPORT_TEST_DATAMAX; j++)
				SendPkt.data[j] = RandU32();
			
			// send packet
			res = UsbPortSendPkt(inx, &SendPkt);

			// check if channel is busy
			if (res != USBPORT_ERR_BUSY)
			{
				// check result
				if (res == USBPORT_ERR_OK)
				{
					send_ok++; // counter of packets sent OK
					cnt[inx]++; // sequence counter
				}
				else
					send_err++; // count error
			}
		}

		// receive packets
		for (i = 0; i < CHANNUM; i++)
		{
			// channel index
			inx = port_recv+i;

			// receive packet
			res = UsbPortRecvPkt(inx, &RecvPkt);

			// check if channel is busy
			if (res != USBPORT_ERR_BUSY)
			{
				// this is error
				err = (res != USBPORT_ERR_OK);

				// check length and command
				if ((RecvPkt.len != 64) || (RecvPkt.cmd != USBPORT_CMD_TEST)) err = True;

				// check sequence counter
				if (RecvPkt.cnt != cnt[inx])
				{
					cnt[inx] = RecvPkt.cnt;
					err = True;
				}

				// initialize random generator
				RandSet(cnt[inx]);

				// some shifts to fill up higher bytes
				RandShift();
				RandShift();
				RandShift();

				// check packet data
				for (j = 0; j < USBPORT_TEST_DATAMAX; j++)
					if (RecvPkt.data[j] != RandU32()) err = True;

				// count packet or error
				if (err)
					recv_err++; // count error
				else
					recv_ok++; // counter of packets received OK

				// shift sequence counter
				cnt[inx]++;
			}
		}

		// print report
		if ((u32)(Time() - t) >= 5000000)
		{
			t = Time();
			Print("send=%dKB/s, recv=%dKB/s, err=%d/%d\n",
				(send_ok*64+3000)/(5*1024), (recv_ok*64+3000)/(5*1024), send_err, recv_err);
			send_ok = 0;
			recv_ok = 0;
		}
	}
}
