
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Example of using cache hit/access counters, and showing the effect of
// invalidate on cache miss/hit.

/*
Debug output
------------
At boot: 616263203 hits, 616265477 accesses
Hit rate so far: 100.0%
Calculate 25th fibonacci number: 121393
New hit rate after printf and fibonacci: 100.0%
Hit/miss check passed
*/

#include "../include.h"

const uint32_t test_data[8] = {0, 1, 2, 3, 4, 5, 6, 7};

void __no_inline_not_in_flash_func(check)(bool cond, const char *msg)
{
	if (!cond)
	{
		puts(msg);
		exit(-1);
	}
}

void __no_inline_not_in_flash_func(check_hit_miss_invalidate)()
{
	io_rw_32 *test_data_ptr = (io_rw_32 *) test_data;

	// Flush cache to make sure we miss the first time we access test_data
	xip_ctrl_hw->flush = 1;
	while (!(xip_ctrl_hw->stat & XIP_STAT_FLUSH_READY_BITS))
		tight_loop_contents();

	// Clear counters (write any value to clear)
	xip_ctrl_hw->ctr_acc = 1;
	xip_ctrl_hw->ctr_hit = 1;

	(void) *test_data_ptr;
	check(xip_ctrl_hw->ctr_hit == 0 && xip_ctrl_hw->ctr_acc == 1,
		"First access to data should miss");

	(void) *test_data_ptr;
	check(xip_ctrl_hw->ctr_hit == 1 && xip_ctrl_hw->ctr_acc == 2,
		"Second access to data should hit");

	// Write to invalidate individual cache lines (64 bits)
	// Writes must be directed to the cacheable, allocatable alias (address 0x10.._....)
	*test_data_ptr = 0;
	(void) *test_data_ptr;
	check(xip_ctrl_hw->ctr_hit == 1 && xip_ctrl_hw->ctr_acc == 3,
		"Should miss after invalidation");
	(void) *test_data_ptr;
	check(xip_ctrl_hw->ctr_hit == 2 && xip_ctrl_hw->ctr_acc == 4,
		"Second access after invalidation should hit again");
}

// Some code which achieves a very high cache hit rate:
int recursive_fibonacci(int n)
{
	if (n <= 1)
		return 1;
	else
		return recursive_fibonacci(n - 1) + recursive_fibonacci(n - 2);
}

int main()
{
	stdio_init_all();

	// wait to connect terminal
	int ch;
	do {
		printf("Press spacebar to start...\n");
		ch = getchar();
	} while (ch != ' ');

	uint hit = xip_ctrl_hw->ctr_hit;
	uint access = xip_ctrl_hw->ctr_acc;

	if (access == 0)
		printf("It looks like you're running this example from SRAM. This probably won't go well!\n");

	// Note the hit rate will appear quite low at boot, as the .data,
	// .time_critical init in crt0 read a lot of read-once data from flash
	printf("At boot: %d hits, %d accesses\n", hit, access);
	printf("Hit rate so far: %.1f%%\n", hit * 100.f / access);

	printf("Calculate 25th fibonacci number: %d\n", recursive_fibonacci(25));
	printf("New hit rate after printf and fibonacci: %.1f%%\n", xip_ctrl_hw->ctr_hit * 100.f / xip_ctrl_hw->ctr_acc);

	check_hit_miss_invalidate();

	printf("Hit/miss check passed\n");

	// Wait for uart output to finish
	sleep_ms(100);

	return 0;
}
