#define DT_DRV_COMPAT excelitas_pyd1598

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <pyd1598.h>
#include <zephyr/irq.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <errno.h>


// Stored in RAM, save all settings here that might be changed during runtime
struct pyd1598_data {
    int test_1;
    int test_2;
    int test_3;
	const struct pyd1598_config *cfg;
};

// Read only after configuration: https://docs.zephyrproject.org/latest/kernel/drivers/index.html
struct pyd1598_config {
	int instance;

	struct gpio_dt_spec serial_in;
	struct gpio_dt_spec direct_link;
};


// Sensor api definitions
// https://github.com/zephyrproject-rtos/zephyr/blob/main/include/zephyr/drivers/sensor.h

// Spi bitbang driver:
// https://github.com/GeorgeGkinis/zephyr/blob/5f4f9ba793d6cb18762decaf2c2e62b9ba05ae33/drivers/spi/spi_bitbang.c

// Extend sensor_api thrrough the header file, I am not alone doing this: 
// The reason is that the pyd1598 must get configured and all configurations must be applied at the same time
// this is not the way intended by the sensor api, but it is the way the pyd1598 works

// Error codes:
// https://docs.zephyrproject.org/apidoc/latest/group__system__errno.html



LOG_MODULE_REGISTER(PYD1598, CONFIG_SENSOR_LOG_LEVEL);

/**
 * @brief Set pyd1598 attributes.
 *
 * @param dev Pointer to the pyd1598 device structure
 * @param chan Channel to set.
 *             Supported channels :
 *               SENSOR_CHAN_WEIGHT
 *               SENSOR_CHAN_ALL
 * @param attr Attribute to change.
 *             Supported attributes :
 *               SENSOR_ATTR_SAMPLING_FREQUENCY
 *               SENSOR_ATTR_OFFSET
 *               SENSOR_ATTR_CALIBRATION
 *               SENSOR_ATTR_SLOPE
 *               SENSOR_ATTR_GAIN
 * @param val   Value to set.
 * @retval 0 on success
 * @retval -ENOTSUP if an invalid attribute is given
 *
 */


// Initialize the sensor device, do not configure the sensor here
static int pyd1598_init(const struct device *dev)
{
	LOG_DBG("Initialising pyd1598");
	const struct pyd1598_config *cfg = dev->config;

	LOG_DBG("pyd1598 Instance: %d", cfg->instance);
    LOG_DBG("Serial in GPIO port : %s", cfg->serial_in.port->name);
    LOG_DBG("Serial in Pin : %d", cfg->serial_in.pin);
    LOG_DBG("Direct link GPIO port : %s", cfg->direct_link.port->name);
    LOG_DBG("Direct link Pin : %d", cfg->direct_link.pin);

	return 0;
}



// Fill local buffer of desired configuration of the sensor
static int pyd1598_attr_set(const struct device *dev, enum sensor_channel chan,
			    enum sensor_attribute attr, const struct sensor_value *val)
{
    LOG_DBG("pyd1598_attr_set");
    return 0;
}


// Apply the configuration to the sensor if all configurations are valid and set
static int pyd1598_apply_config(const struct device *dev, struct pyd1598_config *config){

}


// Get the current configuration of the sensor for a given attribute
static int pyd1598_attr_get(const struct device *dev, enum sensor_channel chan,
			    enum sensor_attribute attr, struct sensor_value *val)
{
    LOG_DBG("pyd1598_attr_get");
    return 0;
}


// Save new data to local representation, tell if the sensor has been triggerd
static int pyd1598_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    LOG_DBG("pyd1598_sample_fetch");
    return 0;
}

// Get the current sensor value
static int pyd1598_channel_get(const struct device *dev, enum sensor_channel chan,
			       struct sensor_value *val)
{
    LOG_DBG("pyd1598_channel_get");
    return 0;
}

static const struct sensor_driver_api pyd1598_api = {
    .attr_set = pyd1598_attr_set,
    .attr_get = pyd1598_attr_get,
    .sample_fetch = pyd1598_sample_fetch,
    .channel_get = pyd1598_channel_get,
};

#define pyd1598_INIT(index)                                                      \
	static struct pyd1598_data pyd1598_data_##index = {                        \
        .test_1 = 0,                                                   \
        .test_2 = 0,                                                   \
        .test_3 = 0,                                                   \
	};                                                                     \
	static const struct pyd1598_config pyd1598_config_##index = {              \
		.instance = index,                                             \
        .serial_in = GPIO_DT_SPEC_INST_GET(index, serial_in_gpios),        \
        .direct_link = GPIO_DT_SPEC_INST_GET(index, direct_link_gpios)};      \
                                                                               \
                                                                               \
	DEVICE_DT_INST_DEFINE(index, pyd1598_init, PM_DEVICE_DT_INST_GET(index), \
			      &pyd1598_data_##index, &pyd1598_config_##index,      \
			      POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,        \
			      &pyd1598_api);


DT_INST_FOREACH_STATUS_OKAY(pyd1598_INIT)
