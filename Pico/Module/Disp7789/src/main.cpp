
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define ST7789_CASET		0x2A	// set start and end column of draw window
#define ST7789_RASET		0x2B	// set start and end row of draw window
#define ST7789_RAMWR		0x2C	// start write data to draw window
#define ST7789_RAMCTRL		0xB0	// RAM control, data 2 bytes
#define ST7789_COLMOD		0x3A	// set color mode (data COLOR_MODE_*)
#define ST7789_MADCTL		0x36	// set rotation mode (data RotationTab)

/*
// display select
#define DISPSEL_NO	0
#define DISPSEL_VGA	1
#define DISPSEL_DVI	2
u8 DispSelected = DISPSEL_NO;	// current selected display

// select new display
void DispSel(u8 sel)
{
	// no change
	if (DispSelected == sel) return;

	// deactivate current display
	switch (DispSelected)
	{
	// VGA display
	case DISPSEL_VGA:
		VgaStop();
		WaitMs(100);
		break;

	// DVI display
	case DISPSEL_DVI:
		DviStop();
		WaitMs(100);
		break;
	}

	// activate new display
	DispSelected = sel;
	switch (sel)
	{
	// VGA display
	case DISPSEL_VGA:
		VgaStart();
		WaitMs(100);
		break;

	// DVI display
	case DISPSEL_DVI:
		DviStart();
		WaitMs(100);
		break;
	}
}
*/

// initialize SPI grabber PIO
void SpiPioInit()
{
	// initialize PIO
	PioInit(SPI_PIO);

	// load PIO program
	PioLoadProg(SPI_PIO, grabber_program_instructions, count_of(grabber_program_instructions), SPI_PIO_OFF);

	// PIO set wrap address
	PioSetWrap(SPI_PIO, SPI_SM, SPI_PIO_OFF + grabber_wrap_target, SPI_PIO_OFF + grabber_wrap);

	// set start address
	PioSetAddr(SPI_PIO, SPI_SM, SPI_PIO_OFF + grabber_offset_start);

	// configure GPIOs for use by PIO
	PioSetupGPIO(SPI_PIO, SPI_GPIO_FIRST, SPI_GPIO_NUM);

	// set pin direction to input
	PioSetPinDir(SPI_PIO, SPI_SM, SPI_GPIO_FIRST, SPI_GPIO_NUM, 0);

	// switch OFF pin synchronization to get faster input
	PioInBypassMask(SPI_PIO, RangeMask(SPI_GPIO_FIRST, SPI_GPIO_FIRST+SPI_GPIO_NUM-1));

	// map state machine's IN pins	
	PioSetupIn(SPI_PIO, SPI_SM, SPI_GPIO_FIRST);

	// set sideset pins (no sideset)
	PioSetupSideset(SPI_PIO, SPI_SM, 0, 0, False, False);

	// setup JMP PIN instructin to check CS pin
	PioSetJmpPin(SPI_PIO, SPI_SM, SPI_GPIO_CS);

	// join FIFO to receive only
	PioSetFifoJoin(SPI_PIO, SPI_SM, PIO_FIFO_JOIN_RX);

	// PIO clock divider
	PioSetClkdiv(SPI_PIO, SPI_SM, 1*256);

	// shift left, autopush on 9 bits
	PioSetInShift(SPI_PIO, SPI_SM, False, True, 9);

	// enable state machine
	PioSMEnable(SPI_PIO, SPI_SM);
}

// process
void NOFLASH(Proc)()
{
	u32 k, k2;
	int y = 0;
	int x = 0;
	int x1 = 0;		// start X of window
	int x2 = WIDTH;		// end X of window
	int y1 = 0;		// start Y of window
	int y2 = HEIGHT;	// end Y of window
	u16* d = FrameBuf;	// destination pointer
	Bool first = True;	// first data byte

	// select mode
	GPIO_Init(DISPSEL_GPIO);
	GPIO_DirIn(DISPSEL_GPIO);
	GPIO_PullUp(DISPSEL_GPIO);
	WaitUs(100);


	if (GPIO_In(DISPSEL_GPIO))
		DviStart();
	else
		VgaStart();



//	VgaStart();
//	DviStart();


/*
	int fps = DviFrame;
	while(True)
	{
		WaitMs(1000);
		int fps2 = DviFrame;
		Print("FPS = %d\n", fps2 - fps);
		fps = fps2;

//		LedFlip(0);
	}

*/

//	DispSel(GPIO_In(DISPSEL_GPIO) + 1);

	// main loop
	while (True)
	{
		// read byte
		k = PioReadWait(SPI_PIO, SPI_SM);

		// command
		if (k < 0x100)
		{

//		printf("cmd=%08X   \r", k);


			switch (k)
			{
			// set start and end column of draw window
			case ST7789_CASET:
				x1 = (PioReadWait(SPI_PIO, SPI_SM) & 0xff) << 8; // start X HIGH
				x1 |= PioReadWait(SPI_PIO, SPI_SM) & 0xff; // start X LOW
//				if (x1 >= WIDTH) x1 = WIDTH-1;
				x2 = (PioReadWait(SPI_PIO, SPI_SM) & 0xff) << 8; // stop X HIGH
				x2 |= PioReadWait(SPI_PIO, SPI_SM) & 0xff; // stop X LOW
				x2++;
//				if (x2 <= x1) x2 = x1 + 1;
//				x = x1;
//				y = y1;
//				d = &FrameBuf[x + y*WIDTH];
//				first = True;
				break;

			// set start and end row of draw window
			case ST7789_RASET:
				y1 = (PioReadWait(SPI_PIO, SPI_SM) & 0xff) << 8; // start Y HIGH
				y1 |= PioReadWait(SPI_PIO, SPI_SM) & 0xff; // start Y LOW
//				if (y1 >= HEIGHT) y1 = HEIGHT-1;
				y2 = (PioReadWait(SPI_PIO, SPI_SM) & 0xff) << 8; // stop Y HIGH
				y2 |= PioReadWait(SPI_PIO, SPI_SM) & 0xff; // stop Y LOW
				y2++;
//				if (y2 <= y1) y2 = y1 + 1;
//				x = x1;
//				y = y1;
//				d = &FrameBuf[x + y*WIDTH];
//				first = True;
				break;

			// start write data to draw window
			case ST7789_RAMWR:
				x = x1;
				y = y1;
				d = &FrameBuf[x + y*WIDTH];

//				first = True;

//				printf("x1=%d x2=%d y1=%d y2=%d   \r", x1, x2, y1, y2);

				break;

			// RAM control, data 2 bytes
			case ST7789_RAMCTRL:
				PioReadWait(SPI_PIO, SPI_SM);
				PioReadWait(SPI_PIO, SPI_SM);
				break;				

			// set color mode (data COLOR_MODE_*)
			case ST7789_COLMOD:
			// set rotation mode (data RotationTab)
			case ST7789_MADCTL:
				PioReadWait(SPI_PIO, SPI_SM);
				break;				
			}
		}

		// image data
		else
		{

			k &= 0xff;

			k |= (PioReadWait(SPI_PIO, SPI_SM) & 0xff) << 8;
			*d++ = k;


/*
			k &= 0xff;

			// first byte
			if (first)
			{
				k2 = k;
				first = False;
			}

			// second byte
			else
			{
				// write one pixel
				k2 |= k << 8;
				//*d++ = (u16)k2;
*/
				x++;
//				first = True;

				// end of row
				if (x >= x2)
				{
					x = x1;
					d += WIDTH - (x2 - x1);
					y++;

					// end of image
					if (y >= y2)
					{
						y = y1;
						d = &FrameBuf[x + y*WIDTH];
					}
				}
//			}
		}
	}
}

int main()
{
//	int y = 0;

	// initialize SPI grabber PIO
	SpiPioInit();

	// draw rainbow gradient
//	for (y; y < HEIGHT; y++) GenGrad(&FrameBuf[y*WIDTH], WIDTH);
//	DispUpdateAll();

	// print setup
//	printf("CLK_SYS=%dMHz\n", ClockGetHz(CLK_SYS)/1000000);
//	printf("CLKDIV=%d\n", SSI_FlashClkDiv());
//	printf("VREG=%.2fV\n", VregVoltageFloat());

	Proc();
}
