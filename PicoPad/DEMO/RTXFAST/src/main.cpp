
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define DISP_FPS	0		// 1=display FPS

#define CS_ON	GPIO_Out0(DISP_CS_PIN); cb()	// activate chip selection
#define CS_OFF	cb(); GPIO_Out1(DISP_CS_PIN)	// deactivate chip selection
#define DC_CMD	GPIO_Out0(DISP_DC_PIN); cb()	// set command mode
#define DC_DATA	GPIO_Out1(DISP_DC_PIN); cb()	// set data mode

// display start DMA update
void FASTCODE NOFLASH(DispUpdateStartDMA)()
{
	int dma = DMA_TEMP_CHAN();
	spi_hw_t* hw = SPI_GetHw(DISP_SPI);

	// get DMA channel hardware interface
	dma_channel_hw_t* hwdma = DMA_GetHw(dma);

	// synchronize external display (to start waiting for active CS)
	u8 d = 0xff;
	CS_OFF; 	// deactivate chip selection
	DC_CMD;		// set command mode
	SPI_Send8(DISP_SPI, &d, 1); // send data to SPI

	// set draw window
	DispWindow((u16)0, (u16)WIDTH, (u16)0, (u16)HEIGHT);

	// send data from frame buffer
	CS_ON;		// activate chip selection
	DC_DATA;	// set data mode

	// configure DMA
	DMA_Abort(dma); // abort current transfer
	DMA_ClearError_hw(hwdma); // clear errors
	DMA_SetRead_hw(hwdma, FrameBuf); // set source address
	DMA_SetWrite_hw(hwdma, &hw->dr); // set destination address
	DMA_SetCount_hw(hwdma, FRAMESIZE*2); // set count of elements
	cb(); // compiler barrier
	DMA_SetCtrlTrig_hw(hwdma,
		DMA_CTRL_TREQ(SPI_GetDreq_hw(hw, True)) |
		DMA_CTRL_CHAIN(dma) |
		//DMA_CTRL_INC_WRITE |
		DMA_CTRL_INC_READ |
		DMA_CTRL_SIZE(DMA_SIZE_8)
		| DMA_CTRL_EN); // set control and trigger transfer
}

// display wait DMA update
void FASTCODE NOFLASH(DispUpdateWaitDMA)()
{
	int dma = DMA_TEMP_CHAN();
	while (DMA_IsBusy(dma)) {}
	spi_hw_t* hw = SPI_GetHw(DISP_SPI);

	// get DMA channel hardware interface
	dma_channel_hw_t* hwdma = DMA_GetHw(dma);

	// disable DMA channel
	DMA_Disable_hw(hwdma);

	// waiting for transmission to complete
	while (SPI_IsBusy_hw(hw)) SPI_RxFlush_hw(hw);

	// flush rest of received data
	SPI_RxFlush_hw(hw);

	// clear interrupt on receive overrun status
	SPI_RxOverClear_hw(hw);

	CS_OFF; 	// deactivate chip selection
}

int FASTCODE NOFLASH(main)()
{
	SSI_InitFlash(10);
	VregSetVoltage(VREG_VOLTAGE_1_30);
	ClockPllSysFreq(330000);

	float a = 0.2f;
	float b = 0.8f;
	float c = 1.2f;
	float d = 1.9f;

#if DISP_FPS			// 1=display FPS
	u32 t = Time();
	u32 t2;
	char buf[20];
#endif

	while(True)
	{
		// animation
		FloorPos -= 0.2f; if (FloorPos <= -2.0f) FloorPos += 2.0f;
		Spheres[0].pos.y = absf(sinf(a))*Spheres[0].rad + Spheres[0].rad + FLOORY;
		Spheres[1].pos.y = absf(sinf(b))*Spheres[1].rad*1.5f + Spheres[1].rad + FLOORY;
		Spheres[2].pos.y = absf(sinf(c))*Spheres[2].rad + Spheres[2].rad + FLOORY;
		Spheres[3].pos.y = absf(sinf(d))*Spheres[3].rad + Spheres[3].rad + FLOORY;
		a += 0.2f; if (a >= (float)PI2) a -= (float)PI2;
		b += 0.24f; if (b >= (float)PI2) b -= (float)PI2;
		c += 0.12f; if (c >= (float)PI2) c -= (float)PI2;
		d += 0.17f; if (d >= (float)PI2) d -= (float)PI2;

		// render core 0: 116 ms
		// render core 1: 119 ms
		// display DMA: 51 ms
		// render + display all: 120 ms

		// start rendering on core 1
		Core1Exec(Render3D);

		// rendering on core 0
		Render3D();
		dsb();
		while (Core1IsRunning) { dsb(); }

#if DISP_FPS			// 1=display FPS
		// FPS
		t2 = Time();
		MemPrint(buf, 20, "%.2f ", 1000000.0/(t2-t));
		DispDrawText(buf, 0, 0, 0, 0, COL_WHITE, COL_BLACK);
		WaitMs(100);
		t = Time();
#endif

		// start transfer to LCD using DMA
		DispUpdateWaitDMA();
		DispUpdateStartDMA();

		// keys
		switch (KeyGet())
		{
		// reset to boot loader
		case KEY_Y: ResetToBootLoader();

#if USE_SCREENSHOT		// use screen shots
		case KEY_X:
			ScreenShot();
			break;
#endif
		}
	}
}
