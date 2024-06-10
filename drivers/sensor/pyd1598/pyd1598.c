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
// After all actions, leave the gpio pins in high impedance mode.
//
// * push: write from driver buffer to sensor - for all configurations
// * fetch: read from sensor to driver buffer - for all configurations and sensor value
// * get: read from driver buffer to user - one for each configuration and sensor value: for messurments validate returned conf to actual conf for mode
// * set: write from user to driver buffer - one for each configuration
// * set_default: set default configuration to driver buffer
// * reset: reset sensor in wake-up mode - not possible if not in wake-up mode
// * sensor_triggerd: check if sensor has been triggerd, only meaningful in wake-up mode

// How to interact with the sensor:
// * Read from sensor forced readout mode
//   - fetch 
//   - get 
// * Read from sensor wake-up mode
//   - if sensor_triggerd
//     - reset
//     - fetch
//     - get
//   - else
//     - continue
// * Configure sensor
//   - set_default
//   - set_XXX
//   - push
// * Get sensor configuration
//   - fetch  - doubble check if actual is desired
//   - get_all(desired)
//   - compare


// Sensor api definitions
// https://github.com/zephyrproject-rtos/zephyr/blob/main/include/zephyr/drivers/sensor.h

// Spi bitbang driver:
// https://github.com/GeorgeGkinis/zephyr/blob/5f4f9ba793d6cb18762decaf2c2e62b9ba05ae33/drivers/spi/spi_bitbang.c

// Extend sensor_api thrrough the header file, I am not alone doing this: 
// The reason is that the pyd1598 must get configured and all configurations must be applied at the same time
// this is not the way intended by the sensor api, but it is the way the pyd1598 works

// Error codes:
// https://docs.zephyrproject.org/apidoc/latest/group__system__errno.html


#define DT_DRV_COMPAT excelitas_pyd1598

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/irq.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <errno.h> // std error codes : https://github.com/zephyrproject-rtos/zephyr/blob/main/lib/libc/minimal/include/errno.h
#include <stdint.h>
#include <stdbool.h>
#include <pyd1598.h>

LOG_MODULE_REGISTER(PYD1598, CONFIG_SENSOR_LOG_LEVEL);


// Stored in RAM, save all settings here that might be changed during runtime
struct pyd1598_configuration {
    uint32_t raw_bits;
    uint8_t threshold; 
    uint8_t blind_time;
    uint8_t pulse_counter;
    uint8_t window_time;
    uint8_t operation_mode;
    uint8_t signal_source;
    uint8_t hpf_cut_off;
    uint8_t count_mode;
};

struct pyd1598_measurement {
    uint32_t raw_bits;
    // out of range bit 39
    uint8_t out_of_range;
    // adc count bit 38-25
    uint16_t adc_count;
};


struct pyd1598_data {
    // Save all user set configurations here, get configuration from here if nothing else is specified
    struct pyd1598_configuration sensor_desired_configuration; // Desired configuration of the sensor
    // Get the actual configuration from the sensor, save it here if nothing else is specified
    struct pyd1598_configuration sensor_actual_configuration;  // Actual configuration of the sensor
    // Is there any data in the actual configuration
    bool data_in_actual_configuration;  // data in actual configuration is valid
    // Save messurement data here
    struct pyd1598_measurement measurement; // Measurement data from the sensor


	const struct pyd1598_config *cfg;
};

// Read only after configuration: https://docs.zephyrproject.org/latest/kernel/drivers/index.html
struct pyd1598_config {
	int instance;

	struct gpio_dt_spec serial_in;
	struct gpio_dt_spec direct_link;
};


// Initialize the sensor device, do not configure the sensor here
static int pyd1598_init(const struct device *dev)
{
	LOG_DBG("Initialising pyd1598");
    if (dev == NULL || dev->data == NULL || dev->config == NULL) {
        return -EINVAL;
    }
	const struct pyd1598_config *cfg = dev->config; 
    struct pyd1598_data *data = dev->data; // Void pointer, needs to be casted


    // Check if the GPIO pins are ready
    if (!gpio_is_ready_dt(&cfg->serial_in)) {
        LOG_ERR("Serial in GPIO pin %d is not ready", cfg->serial_in.pin);
        return -ENODEV;
    }

    if (!gpio_is_ready_dt(&cfg->direct_link)) {
        LOG_ERR("Direct link GPIO pin %d is not ready", cfg->direct_link.pin);
        return -ENODEV;
    }


    // Configure the GPIO pins Serial in and Direct link
    int ret = 0;
    ret = gpio_pin_configure_dt(&cfg->serial_in, GPIO_INPUT | cfg->serial_in.dt_flags);
    if (ret != 0) {
        LOG_ERR("Failed to configure serial in GPIO pin %d", cfg->serial_in.pin);
        return ret;
    }

    ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_INPUT | cfg->direct_link.dt_flags);
    if (ret != 0) {
        LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
        return ret;
    }


    // Make all configurations zero
    memset(&(data->desired_configuration), 0, sizeof(struct pyd1598_configuration));
    memset(&(data->actual_configuration), 0, sizeof(struct pyd1598_configuration));
    // Make all measurement data zero
    memset(&(data->measurement), 0, sizeof(struct pyd1598_measurement));


    // Set reserved bits in desired configuration, to allow for user to not set them even if encouraged 
    struct pyd1598_configuration *configuration;
    configuration = &(data->desired_configuration); 
    // 4-3 = 2 bits set to dec 2
    configuration->raw_bits = (configuration->raw_bits & ~((0x3<<3))) | (((uint32_t)2) << 3);
    // 1 = 1 bit set to dec 0
    configuration->raw_bits = (configuration->raw_bits & ~((0x1<<1))) | (((uint32_t)0) << 1);


    // Log the configuration
	LOG_DBG("pyd1598 Instance: %d", cfg->instance);
    LOG_DBG("Serial in GPIO port : %s, INS %d", cfg->serial_in.port->name, cfg->instance);
    LOG_DBG("Serial in Pin : %d, INS %d", cfg->serial_in.pin, cfg->instance);
    LOG_DBG("Direct link GPIO port : %s, INS %d", cfg->direct_link.port->name, cfg->instance);
    LOG_DBG("Direct link Pin : %d, INS %d", cfg->direct_link.pin, cfg->instance);

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
int pyd1598_push_config(const struct device *dev){
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
int pyd1598_fetch(const struct device *dev){
    LOG_DBG("pyd1598_fetch_config");
    return 0;
}



/**
* @brief Set pyd1598 reserved bits configuration to the internal buffer.
*
* @param dev Pointer to the sensor device
*
* @return 0 if successful, negative errno code if failure.
*/
int pyd1598_set_reserved_bits(const struct device *dev){
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;
    configuration = &(dev->data->desired_configuration);
    // 4-3 = 2 bits set to dec 2
    configuration->raw_bits = (configuration->raw_bits & ~((0x3<<3))) | (((uint32_t)2) << 3);
    // 1 = 1 bit set to dec 0
    configuration->raw_bits = (configuration->raw_bits & ~((0x1<<1))) | (((uint32_t)0) << 1);

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
int pyd1598_set_threshold(const struct device *dev, uint8_t threshold){
    if (threshold > 255 || dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;
    configuration = &(dev->data->desired_configuration);
    configuration->threshold = threshold;

    // Set raw bits in configuration at 24-17 to threshold, leave the rest of the bits as they are
    configuration->raw_bits = (configuration->raw_bits & ~((0xFF<<17))) | (((uint32_t)threshold) << 17);

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
int pyd1598_set_blind_time(const struct device *dev, uint8_t blind_time){
    if (blind_time > 15 || dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;
    configuration = &(dev->data->desired_configuration);
    configuration->blind_time = blind_time;

    // bit 16-13 = 4 bits
    configuration->raw_bits = (configuration->raw_bits & ~((0xF<<13))) | (((uint32_t)blind_time) << 13);

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
int pyd1598_set_pulse_counter(const struct device *dev, uint8_t pulse_counter){
    if (pulse_counter > 3 || dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;
    configuration = &(dev->data->desired_configuration);
    configuration->pulse_counter = pulse_counter;

    // 12-11 = 2 bits 
    configuration->raw_bits = (configuration->raw_bits & ~((0x3<<11))) | (((uint32_t)pulse_counter) << 11);

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
int pyd1598_set_window_time(const struct device *dev, uint8_t window_time){
    if (window_time > 3 || dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;
    configuration = &(dev->data->desired_configuration);
    configuration->window_time = window_time;

    // 10-9 = 2 bits
    configuration->raw_bits = (configuration->raw_bits & ~((0x3<<9))) | (((uint32_t)window_time) << 9);

    return 0;
}

/**
 * @brief Set pyd1598 operation mode configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param operation_mode  Operation mode (PYD1598_FORCED_READOUT, PYD1598_WAKE_UP)
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_set_operation_mode(const struct device *dev, enum pyd1598_operation_mode operation_mode){
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;
    configuration = &(dev->data->desired_configuration);
    configuration->operation_mode = operation_mode;

    // 8-7 = 2 bits
    configuration->raw_bits = (configuration->raw_bits & ~((0x3<<7))) | (((uint32_t)operation_mode) << 7);

    return 0;
}

/**
 * @brief Set pyd1598 signal source configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param signal_source Signal source (PYD1598_PIR_BPF, PYD1598_PIR_LPF, PYD1598_TEMPERATURE_SENSOR)
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_set_signal_source(const struct device *dev, enum pyd1598_signal_source signal_source){
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;
    configuration = &(dev->data->desired_configuration);
    configuration->signal_source = signal_source;

    // 6-5 = 2 bits
    configuration->raw_bits = (configuration->raw_bits & ~((0x3<<5))) | (((uint32_t)signal_source) << 5);

    return 0;
}

/**
 * @brief Set pyd1598 HPF Cut Off configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param hpf_cut_off HPF Cut Off (PYD1598_HPF_CUTOFF_0_4HZ, PYD1598_HPF_CUTOFF_0_2HZ)
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_set_hpf_cut_off(const struct device *dev, enum pyd1598_hpf_cutoff hpf_cut_off){
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;
    configuration = &(dev->data->desired_configuration);
    configuration->hpf_cut_off = hpf_cut_off;

    // 2 = 1 bit 
    configuration->raw_bits = (configuration->raw_bits & ~((0x1<<2))) | (((uint32_t)hpf_cut_off) << 2);

    return 0;
}

/**
 * @brief Set pyd1598 Count Mode configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param count_mode Count Mode (PYD1598_COUNT_SIGN_CHANGE, PYD1598_COUNT_ALL)
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_set_count_mode(const struct device *dev, enum pyd1598_count_mode count_mode){
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;
    configuration = &(dev->data->desired_configuration);
    configuration->count_mode = count_mode;

    // 0 = 1 bit
    configuration->raw_bits = (configuration->raw_bits & ~((0x1<<0))) | (((uint32_t)count_mode) << 0);

    return 0;
}

/**
 * @brief Get pyd1598 threshold configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param threshold Pointer to where the threshold value should be stored
 * 
 * @return 0 successful, negative errno code if failure.
*/
int pyd1598_get_threshold(const struct device *dev, uint8_t *threshold){ 
    if (dev == NULL || dev->data == NULL || threshold == NULL) {
        return -EINVAL;
    }

    *threshold = dev->data->desired_configuration->threshold;

    return 0;
}


/**
 * @brief Get pyd1598 blind time configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param blind_time Pointer to where the blind time value should be stored
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_get_blind_time(const struct device *dev, uint8_t *blind_time){
    if (dev == NULL || dev->data == NULL || blind_time == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;

    configuration = &(dev->data->desired_configuration);
    *blind_time = configuration->blind_time;

    return 0;
}


/**
 * @brief Get pyd1598 pulse counter configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param pulse_counter Pointer to where the pulse counter value should be stored
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_get_pulse_counter(const struct device *dev, uint8_t *pulse_counter){
    if (dev == NULL || dev->data == NULL || pulse_counter == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;

    configuration = &(dev->data->desired_configuration);
    *pulse_counter = configuration->pulse_counter;


    return 0;
}


/**
 * @brief Get pyd1598 window time configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param window_time Pointer to where the window time value should be stored
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_get_window_time(const struct device *dev, uint8_t *window_time){
    if (dev == NULL || dev->data == NULL || window_time == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;

    configuration = &(dev->data->desired_configuration);
    *window_time = configuration->window_time;

    return 0;
}


/**
 * @brief Get pyd1598 operation mode configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param operation_mode Pointer to where the operation mode value should be stored
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_get_operation_mode(const struct device *dev, enum pyd1598_operation_mode *operation_mode){
    if (dev == NULL || dev->data == NULL || operation_mode == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;
    configuration = &(dev->data->desired_configuration);
    *operation_mode = configuration->operation_mode;


    return 0;
}


/**
 * @brief Get pyd1598 signal source configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param signal_source Pointer to where the signal source value should be stored
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_get_signal_source(const struct device *dev, enum pyd1598_signal_source *signal_source){
    if (dev == NULL || dev->data == NULL || signal_source == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;
    configuration = &(dev->data->desired_configuration);
    *signal_source = configuration->signal_source;

    return 0;
}


/**
 * @brief Get pyd1598 HPF Cut Off configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param hpf_cut_off Pointer to where the HPF Cut Off value should be stored
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_get_hpf_cut_off(const struct device *dev, enum pyd1598_hpf_cutoff *hpf_cut_off){
    if (dev == NULL || dev->data == NULL || hpf_cut_off == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;

    configuration = &(dev->data->desired_configuration);
    *hpf_cut_off = configuration->hpf_cut_off;

    return 0;
}


/**
 * @brief Get pyd1598 Count Mode configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param count_mode Pointer to where the count mode value should be stored
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_get_count_mode(const struct device *dev, enum pyd1598_count_mode *count_mode){
    if (dev == NULL || dev->data == NULL || hpf_cut_off == NULL) {
        return -EINVAL;
    }

    struct pyd1598_configuration *configuration;

    configuration = &(dev->data->desired_configuration);
    *count_mode = configuration->count_mode;

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
int pyd1598_set_default_config(const struct device *dev) {
    // access desired_configuration in pyd1598_data and set all values to default
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }
    
    pyd1598_set_threshold(dev, 31);
    pyd1598_set_blind_time(dev, 6);
    pyd1598_set_pulse_counter(dev, 0);
    pyd1598_set_window_time(dev, 0);
    pyd1598_set_operation_mode(dev, PYD1598_WAKE_UP);
    pyd1598_set_signal_source(dev, PYD1598_PIR_LPF);
    pyd1598_set_hpf_cut_off(dev, PYD1598_HPF_CUTOFF_0_4HZ);
    pyd1598_set_count_mode(dev, PYD1598_COUNT_ALL);
    pyd1598_set_reserved_bits(dev);

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
        .data_in_actual_configuration = false,                          \
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

