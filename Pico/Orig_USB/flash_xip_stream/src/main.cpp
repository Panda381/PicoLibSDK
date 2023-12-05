
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// The XIP has some internal hardware that can stream a linear access sequence
// to a DMAable FIFO, while the system is still doing random accesses on flash
// code + data.

/*
Debug output
------------
Starting stream from 10005A74
Starting DMA
DMA complete
76654e22 47207265 616e6e6f 76694720 6f592065 70552075 570a0a22 65722765
206f6e20 61727473 7265676e 6f742073 766f6c20 6f590a65 6e6b2075 7420776f
.....
756f7920 79726320 76654e0a 67207265 616e6e6f 79617320 6f6f6720 65796264
76654e0a 67207265 616e6e6f 6c657420 2061206c 2065696c 20646e61 Data check OK
*/

#include "../include.h"

uint32_t buf[count_of(random_test_data)];

int main()
{
	stdio_init_all();

	// wait to connect terminal
	int ch;
	do {
		printf("Press spacebar to start...\n");
		ch = getchar();
	} while (ch != ' ');

	for (int i = 0; i < count_of(random_test_data); ++i)
		buf[i] = 0;

	// This example won't work with PICO_NO_FLASH builds. Note that XIP stream
	// can be made to work in these cases, if you enable some XIP mode first
	// (e.g. via calling flash_enter_cmd_xip() in ROM). However, you will get
	// much better performance by DMAing directly from the SSI's FIFOs, as in
	// this way you can clock data continuously on the QSPI bus, rather than a
	// series of short transfers.
	if ((uint32_t) &random_test_data[0] >= SRAM_BASE)
	{
		printf("You need to run this example from flash!\n");
		exit(-1);
	}

	// Transfer started by writing nonzero value to stream_ctr. stream_ctr
	// will count down as the transfer progresses. Can terminate early by
	// writing 0 to stream_ctr.
	// It's a good idea to drain the FIFO first!
	printf("Starting stream from %p\n", random_test_data);
	while (!(xip_ctrl_hw->stat & XIP_STAT_FIFO_EMPTY))
		(void) xip_ctrl_hw->stream_fifo;
	xip_ctrl_hw->stream_addr = (uint32_t) &random_test_data[0];
	xip_ctrl_hw->stream_ctr = count_of(random_test_data);

	// Start DMA transfer from XIP stream FIFO to our buffer in memory. Use
	// the auxiliary bus slave for the DMA<-FIFO accesses, to avoid stalling
	// the DMA against general XIP traffic. Doesn't really matter for this
	// example, but it can have a huge effect on DMA throughput.

	printf("Starting DMA\n");
	const uint dma_chan = 0;
	dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
	channel_config_set_read_increment(&cfg, false);
	channel_config_set_write_increment(&cfg, true);
	channel_config_set_dreq(&cfg, DREQ_XIP_STREAM);
	dma_channel_configure(
		dma_chan,
		&cfg,
		(void *) buf,                 // Write addr
		(const void *) XIP_AUX_BASE,  // Read addr
		count_of(random_test_data), // Transfer count
		true                        // Start immediately!
	);

	dma_channel_wait_for_finish_blocking(dma_chan);

	printf("DMA complete\n");

	bool mismatch = false;
	for (int i = 0; i < count_of(random_test_data); ++i)
	{
		if (random_test_data[i] != buf[i])
		{
			printf("Data mismatch: %08x (actual) != %08x (expected)\n", buf[i], random_test_data[i]);
			mismatch = true;
			break;
		}
		printf("%08x%c", buf[i], i % 8 == 7 ? '\n' : ' ');
	}

	if (!mismatch)
		printf("Data check OK\n");

	// Wait for uart output to finish
	sleep_ms(100);

	return 0;
}
