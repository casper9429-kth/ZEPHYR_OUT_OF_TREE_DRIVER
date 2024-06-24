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
#include <errno.h> // std error codes : https://github.com/zephyrproject-rtos/zephyr/blob/main/lib/libc/minimal/include/errno.h
#include <stdint.h>
#include <stdbool.h>
#include <float.h>


LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

// Macro to count the number of OKAY children of #master_node
#define COUNT_CHILDREN_OKAY(child) +1
#define NUM_PYD1598_OKAY (0 DT_FOREACH_CHILD_STATUS_OKAY(DT_ALIAS(pir_master), COUNT_CHILDREN_OKAY))



int main(void)
{

    const struct device *devices[NUM_PYD1598_OKAY] = {DT_FOREACH_CHILD_STATUS_OKAY_SEP(DT_ALIAS(pir_master), DEVICE_DT_GET,(,))};
    // * - threshold: 31 (range 0-255)
    // * - blind_time: 6 (0.5 s + 0.5 s * blind_time, range 0-15)
    // * - pulse_counter: 0 (1 + pulse_counter, range 0-3)
    // * - window_time: 0 (2s + 2s * window_time, range 0-3)
    // * - operation_mode: 2 (0: Forced Readout, 1: Interrupt Readout, 2: Wake-up Mode): Only Wake-up Mode is supported
    // * - signal_source: 1 (0: PIR(BRF), 1: PIR(LPF), 2: Not Allowed, 3: Temperature Sensor)
    // * - HPF_Cut_Off: 0 (0: 0.4 Hz, 1: 0.2 Hz)
    // * - Count_Mode: 1 (0: count with (0), or without (1) BPF sign change)


    int ret = 1;
    while (ret != 0)
    {
        ret = pyd1598_set_default_config(devices[0]);
        if (ret != 0)
        {
            LOG_INF("pyd1598_set_default_configuration: %d", ret);
        }
        // Set mode 
        ret = pyd1598_set_operation_mode(devices[0], PYD1598_FORCED_READOUT);
        if (ret != 0)
        {
            LOG_INF("pyd1598_get_operation_mode: %d", ret);
        }
    }


    // Push the configuration to the sensor
    ret = pyd1598_push(devices[0]);
    if (ret != 0)
    {
        LOG_INF("pyd1598_push: %d", ret);
    }


    // Create a float
    float success = 0.5;

    while (true)
    {
    



        // Fetch the data from the sensor
        ret = pyd1598_fetch(devices[0]);
        if (ret != 0)
        {
            success = success*0.99;
        }
        else
        {
        success = success*0.99 + 0.01;
        }


        // Sleep for 10 ms
        k_msleep(10);

        // make float to string
        char success_str[10];
        snprintf(success_str, 10, "%f", success);
        LOG_INF("Success: %s", success_str);







    }





    return 0;
}
