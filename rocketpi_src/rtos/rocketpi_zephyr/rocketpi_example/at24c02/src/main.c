#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "driver_at24cxx_read_test.h"

LOG_MODULE_REGISTER(at24cxx_sample, CONFIG_LOG_DEFAULT_LEVEL);

int main(void)
{
	int ret = at24cxx_read_test(AT24C02, AT24CXX_ADDRESS_A000);

	if (ret != 0) {
		LOG_ERR("AT24C02 read_test failed (%d)", ret);
	} else {
		LOG_INF("AT24C02 read_test passed");
	}

	while (1) {
		k_sleep(K_SECONDS(5));
	}
}
