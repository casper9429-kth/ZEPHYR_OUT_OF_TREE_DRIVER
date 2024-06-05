/*
PYD1598 driver for Zephyr RTOS - out of tree driver

This driver should work with the Excelitas PYD1588 and PYD1598 sensors, but only the PYD1598 has been tested.

In this driver the Zephyr unified sensor API is used sparse,
the reason is that the api does not fit the sensor well. 
And the driver is implemented out of tree, modifying the sensor.h
is not an option.

Only wakeup mode is supported.

Author: Casper Augustsson Savinov
mail: casper9429@gmail.com
*/

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
#include <errno.h> // std error codes : https://github.com/zephyrproject-rtos/zephyr/blob/main/lib/libc/minimal/include/errno.h
#include <stdint.h>

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



// Interface för config:
// Have two driver config buffer, actual config buffer, desired config buffer. Use enum to specify which buffer to use
// The default buffer is the desired buffer for push, get, set. But for fetch it is the actual buffer.
// 
// To make happy flow possible while keeping the sensor robust to error,
// The push, get and set always write to the desired buffer to not allow noise to corrupt the config over time.
// The fetch always reads config to the actual buffer to make sure the sensor does not get corrupted by noise. 
//
// Make examples of how to use the sensor in wake-up mode, and in forced readout mode. Make it impossible to use interrupt readout mode. 
//
// Instead of returning raw adc count: split it into BPF, LPF and Temperature sensor. And if possible make interpertation of the data.
//
// * push: write from driver buffer to sensor - for all configurations
// * pull: read from sensor to driver buffer - for all configurations
// * get: read from driver buffer to user - one for each configuration
// * set: write from user to driver buffer - one for each configuration
// * reset: reset sensor in wake-up mode 

// Interface sensor to user:
// * Reset sensor 
// * Forced readout
// * Wake-up readout : reset senor and force readout






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


/**
 * @brief Pushes config from internal buffer to sensor. 
 * Write configuration to the internal buffer using set_config.
 * 
 * @param dev Pointer to the sensor device
 *
 * @return 0 if successful, negative errno code if failure.
 */
static int pyd1598_push_config(const struct device *dev){
    LOG_DBG("pyd1598_push_config");
    return 0;
}

/**
 * @brief Fetch config from sensor to internal buffer. 
 * 
 * @param dev Pointer to the sensor device
 *
 * @return 0 if successful, negative errno code if failure.
 */
static int pyd1598_fetch_config(const struct device *dev){
    LOG_DBG("pyd1598_fetch_config");
    return 0;
}


// Default config
/**
 * @brief Set default configuration of the sensor to the internal buffer.
 *
 * Default configuration for the sensor:
 * - threshold: 31 (range 0-255)
 * - blind_time: 6 (0.5 s + 0.5 s * blind_time, range 0-15)
 * - pulse_counter: 0 (1 + pulse_counter, range 0-3)
 * - window_time: 0 (2s + 2s * window_time, range 0-3)
 * - operation_mode: 2 (0: Forced Readout, 1: Interrupt Readout, 2: Wake-up Mode): Only Wake-up Mode is supported
 * - signal_source: 1 (0: PIR(BRF), 1: PIR(LPF), 2: Not Allowed, 3: Temperature Sensor)
 * - HPF_Cut_Off: 0 (0: 0.4 Hz, 1: 0.2 Hz)
 * - Count_Mode: 1 (0: count with (0), or without (1) BPF sign change)
 *
 * More information about the configuration values can be found in the datasheet:
 * https://www.excelitas.com/file-download/download/public/66906?filename=PYD_1588_Low-Power_DigiPyro_datasheet_.pdf
 *
 * @param dev Pointer to the sensor device
 * @return 0 if successful, negative errno code if failure.
 */
static int pyd1598_set_default_config(const struct device *dev) {
    LOG_DBG("pyd1598_set_default_config");
    return 0;
}

/**
* @brief Set pyd1598 threshold configuration to the internal buffer. 
*
* @param threshold Threshold value to set (range 0-255)
* @param dev Pointer to the sensor device
*
* @return 0 if successful, negative errno code if failure.
**/
static int pyd1598_set_threshold(const struct device *dev, uint8_t threshold){
    LOG_DBG("pyd1598_set_threshold");
    return 0;
}

/**
 * @brief Set pyd1598 blind time configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param blind_time Blind time value to set (0.5 s + 0.5 s * blind_time, range 0-15)
 * 
 * @return 0 if successful, negative errno code if failure.
*/
static int pyd1598_set_blind_time(const struct device *dev, uint8_t blind_time){
    LOG_DBG("pyd1598_set_blind_time");
    return 0;
}

/**
 * @brief Set pyd1598 pulse counter configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param pulse_counter Pulse counter value to set (1 + pulse_counter, range 0-3)
 * 
 * @return 0 if successful, negative errno code if failure.
*/
static int pyd1598_set_pulse_counter(const struct device *dev, uint8_t pulse_counter){
    LOG_DBG("pyd1598_set_pulse_counter");
    return 0;
}

/**
 * @brief Set pyd1598 window time configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param window_time Window time value to set (2s + 2s * window_time, range 0-3)
 * 
 * @return 0 if successful, negative errno code if failure.
*/
static int pyd1598_set_window_time(const struct device *dev, uint8_t window_time){
    LOG_DBG("pyd1598_set_window_time");
    return 0;
}

/**
 * @brief Set pyd1598 operation mode configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param operation_mode (Only 2: Wake-up Mode implemented) Operation mode value to set (0: Forced Readout, 1: Interrupt Readout, 2: Wake-up Mode)
 * 
 * @return 0 if successful, negative errno code if failure.
*/
static int pyd1598_set_operation_mode(const struct device *dev, uint8_t operation_mode){
    LOG_DBG("pyd1598_set_operation_mode");
    return 0;
}


/**
 * @brief Set pyd1598 signal source configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param signal_source Signal source value to set (0: PIR(BRF), 1: PIR(LPF), 2: Not Allowed, 3: Temperature Sensor)
 * 
 * @return 0 if successful, negative errno code if failure.
*/
static int pyd1598_set_signal_source(const struct device *dev, uint8_t signal_source){
    LOG_DBG("pyd1598_set_signal_source");
    return 0;
}

/**
 * @brief Set pyd1598 HPF Cut Off configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param hpf_cut_off HPF Cut Off value to set (0: 0.4 Hz, 1: 0.2 Hz)
 * 
 * @return 0 if successful, negative errno code if failure.
*/
static int pyd1598_set_hpf_cut_off(const struct device *dev, uint8_t hpf_cut_off){
    LOG_DBG("pyd1598_set_hpf_cut_off");
    return 0;
}

/**
 * @brief Set pyd1598 Count Mode configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param count_mode Count Mode value to set (0: count with (0), or without (1) BPF sign change)
 * 
 * @return 0 if successful, negative errno code if failure.
*/
static int pyd1598_set_count_mode(const struct device *dev, uint8_t count_mode){
    LOG_DBG("pyd1598_set_count_mode");
    return 0;
}


// One get func for each attr

/**
 * @brief Get pyd1598 threshold configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * 
 * @return threshold value if successful, negative errno code if failure.
*/
static uint8_t pyd1598_get_threshold(const struct device *dev){
    LOG_DBG("pyd1598_get_threshold");
    return 0;
}

/**
 * @brief Get pyd1598 blind time configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * 
 * @return blind time value if successful, negative errno code if failure.
*/
static uint8_t pyd1598_get_blind_time(const struct device *dev){
    LOG_DBG("pyd1598_get_blind_time");
    return 0;
}

/**
 * @brief Get pyd1598 pulse counter configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * 
 * @return pulse counter value if successful, negative errno code if failure.
*/
static uint8_t pyd1598_get_pulse_counter(const struct device *dev){
    LOG_DBG("pyd1598_get_pulse_counter");
    return 0;
}

/**
 * @brief Get pyd1598 window time configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * 
 * @return window time value if successful, negative errno code if failure.
*/
static uint8_t pyd1598_get_window_time(const struct device *dev){
    LOG_DBG("pyd1598_get_window_time");
    return 0;
}

/**
 * @brief Get pyd1598 operation mode configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * 
 * @return operation mode value if successful, negative errno code if failure.
*/
static uint8_t pyd1598_get_operation_mode(const struct device *dev){
    LOG_DBG("pyd1598_get_operation_mode");
    return 0;
}

/**
 * @brief Get pyd1598 signal source configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * 
 * @return signal source value if successful, negative errno code if failure.
*/
static uint8_t pyd1598_get_signal_source(const struct device *dev){
    LOG_DBG("pyd1598_get_signal_source");
    return 0;
}

/**
 * @brief Get pyd1598 HPF Cut Off configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * 
 * @return HPF Cut Off value if successful, negative errno code if failure.
*/
static uint8_t pyd1598_get_hpf_cut_off(const struct device *dev){
    LOG_DBG("pyd1598_get_hpf_cut_off");
    return 0;
}

/**
 * @brief Get pyd1598 Count Mode configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * 
 * @return Count Mode value if successful, negative errno code if failure.
*/
static uint8_t pyd1598_get_count_mode(const struct device *dev){
    LOG_DBG("pyd1598_get_count_mode");
    return 0;
}



// Function to fetch the sensor value
static int pyd1598_fetch_measurement(const struct device *dev){
    LOG_DBG("pyd1598_fetch_measurement");
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
			      NULL);


DT_INST_FOREACH_STATUS_OKAY(pyd1598_INIT)
