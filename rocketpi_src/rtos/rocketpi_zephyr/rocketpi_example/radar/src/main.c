#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "driver_mg58f18_radar_test.h"

int main(void)
{
	printk("MG58F18 radar sample start\n");

	mg58f18_radar_test_run();

	while (1) {
		mg58f18_radar_test_poll();
		k_msleep(20);
	}

	return 0;
}
