#include <zephyr/logging/log.h>



#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/pm/device.h>
#include <zephyr/sys/poweroff.h>
#include <soc.h>
#include <hal/nrf_gpio.h>
#include <hal/nrf_regulators.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <pyd1598.h>


LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

const struct device *dev_0;
const struct device *dev_1;
const struct device *dev_2;


int main(void)
{
    dev_0 = DEVICE_DT_GET(DT_ALIAS(pir0));
    dev_1 = DEVICE_DT_GET(DT_ALIAS(pir1));
    dev_2 = DEVICE_DT_GET(DT_ALIAS(pir2));

    if (!dev_0) {
        LOG_ERR("Failed to get binding for PYD1598_0");
        return 0;
    }
    if (!dev_1) {
        LOG_ERR("Failed to get binding for PYD1598_1");
        return 0;
    }
    if (!dev_2) {
        LOG_ERR("Failed to get binding for PYD1598_2");
        return 0;
    }

    while (1) {
        struct sensor_value val;
        int ret;

        // Fetch sample from the sensor
        ret = sensor_sample_fetch(dev_0);
        if (ret) {
            LOG_ERR("Failed to fetch sample from PYD1598_0");
            continue;
        }

        // Get sensor value
        ret = sensor_channel_get(dev_0, SENSOR_CHAN_ALL, &val);
        if (ret) {
            LOG_ERR("Failed to get sensor value from PYD1598_0");
            continue;
        }

        LOG_INF("PYD1598_0: val = %d.%06d", val.val1, val.val2);

        // Similarly, you can fetch and get values from dev_1 and dev_2
        ret = sensor_sample_fetch(dev_1);
        if (ret) {
            LOG_ERR("Failed to fetch sample from PYD1598_1");
            continue;
        }

        ret = sensor_channel_get(dev_1, SENSOR_CHAN_ALL, &val);
        if (ret) {
            LOG_ERR("Failed to get sensor value from PYD1598_1");
            continue;
        }

        LOG_INF("PYD1598_1: val = %d.%06d", val.val1, val.val2);

        ret = sensor_sample_fetch(dev_2);
        if (ret) {
            LOG_ERR("Failed to fetch sample from PYD1598_2");
            continue;
        }

        ret = sensor_channel_get(dev_2, SENSOR_CHAN_ALL, &val);
        if (ret) {
            LOG_ERR("Failed to get sensor value from PYD1598_2");
            continue;
        }

        LOG_INF("PYD1598_2: val = %d.%06d", val.val1, val.val2);

        // Sleep for a while before fetching again
        k_sleep(K_MSEC(1000));
    }
    return 0;
}
