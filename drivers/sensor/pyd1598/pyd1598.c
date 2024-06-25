/*
PYD1598 driver for Zephyr RTOS - out of tree driver

This driver should work with the Excelitas PYD1588 and PYD1598 sensors, but only the PYD1598 has been tested.

In this driver the Zephyr unified sensor API is used sparse,
the reason is that the api does not fit the sensor well. 
And the driver is implemented out of tree, modifying the sensor.h
is not an option.

Only wakeup mode and forced readout is supported.

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


// macron för konstater (ALLA)
// använd functioner för bit manipulation
// deklarera alla variabler högst i varje function
// get och set bredvid varande
// omdefinera gärna struct och undvik -> ->
// inte implicit cast, alltid explicit cast
// avoid memset, dangerous
// Only save the same information in one place, avoid redundancy


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

// Define macros for configuration
#define PYD1598_THRESHOLD_SHIFT 17
#define PYD1598_THRESHOLD_MASK ((uint32_t)0b11111111)

#define PYD1598_BLIND_TIME_SHIFT 13
#define PYD1598_BLIND_TIME_MASK ((uint32_t)0b1111)

#define PYD1598_PULSE_COUNTER_SHIFT 11
#define PYD1598_PULSE_COUNTER_MASK ((uint32_t)0b11)

#define PYD1598_WINDOW_TIME_SHIFT 9
#define PYD1598_WINDOW_TIME_MASK ((uint32_t)0b11)

#define PYD1598_OPERATION_MODE_SHIFT 7
#define PYD1598_OPERATION_MODE_MASK ((uint32_t)0b11)

#define PYD1598_SIGNAL_SOURCE_SHIFT 5
#define PYD1598_SIGNAL_SOURCE_MASK ((uint32_t)0b11)

#define PYD1598_RESERVED_2_SHIFT 3
#define PYD1598_RESERVED_2_MASK ((uint32_t)0b11)
#define PYD1598_RESERVED_2_DEC_VALUE ((uint32_t)2)


#define PYD1598_HPF_CUT_OFF_SHIFT 2
#define PYD1598_HPF_CUT_OFF_MASK ((uint32_t)0b1)

#define PYD1598_RESERVED_1_SHIFT 1
#define PYD1598_RESERVED_1_MASK ((uint32_t)0b1)
#define PYD1598_RESERVED_1_DEC_VALUE ((uint32_t)0)

#define PYD1598_COUNT_MODE_SHIFT 0
#define PYD1598_COUNT_MODE_MASK ((uint32_t)0b1)

// Define macros for measurement
#define PYD1598_OUT_OF_RANGE_MASK ((uint32_t)0b1)
#define PYD1598_OUT_OF_RANGE_SHIFT 14

#define PYD1598_ADC_COUNTS_MASK ((uint32_t)0b11111111111111)
#define PYD1598_ADC_COUNTS_SHIFT 0



struct pyd1598_data {
    uint32_t sensor_conf; // Desired configuration of the sensor
    uint32_t measurement; // Measurement data from the sensor
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
    // Declare variables
	const struct pyd1598_config *cfg; 
    struct pyd1598_data *data;
    uint32_t sensor_conf = 0;
    uint32_t measurement = 0;
    int ret = 0;

    // Check that the device is not null, and that the data and configuration is not null
    LOG_DBG("Initialising pyd1598");
    if (dev == NULL || dev->data == NULL || dev->config == NULL) {
        return -EINVAL;
    }

    // Define the variables
    cfg = dev->config;
    data = dev->data; 

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

    // Set reserved bits in desired configuration, to allow for user to not set them even if encouraged 
    sensor_conf = (sensor_conf & ~(PYD1598_RESERVED_2_MASK << PYD1598_RESERVED_2_SHIFT)) | (PYD1598_RESERVED_2_DEC_VALUE << PYD1598_RESERVED_2_SHIFT);
    sensor_conf = (sensor_conf & ~(PYD1598_RESERVED_1_MASK << PYD1598_RESERVED_1_SHIFT)) | (PYD1598_RESERVED_1_DEC_VALUE << PYD1598_RESERVED_1_SHIFT);

    // Set the sensor configuration and measurement data in ram
    data->sensor_conf = sensor_conf;
    data->measurement = measurement;

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
int pyd1598_push(const struct device *dev){
    // Variables
    const struct pyd1598_config *cfg; // Get the configuration
    struct pyd1598_data *data; // pyd1598_data
    uint32_t sensor_conf; // Raw bits of the configuration
    uint32_t reg_mask; // Reg mask 
    int key = 0; // Interupt key
    int bit = 0; // Each bit
    int ret = 0; // Return value

    // Check if the device is null
    LOG_DBG("pyd1598_push");
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;
    sensor_conf = data->sensor_conf;
    key = irq_lock();

    // beggining condition 
    // Set both direct link and serial in to output value 0
    ret = gpio_pin_configure_dt(&cfg->serial_in, GPIO_OUTPUT);
    if (ret != 0) {
        irq_unlock(key);
        LOG_ERR("Failed to configure serial in GPIO pin %d", cfg->serial_in.pin);
        return ret;
    }
    ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_OUTPUT);
    if (ret != 0) {
        irq_unlock(key);
        LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
        return ret;
    }
    gpio_pin_set_dt(&cfg->serial_in, 0);
    gpio_pin_set_dt(&cfg->direct_link, 0);

    // Sleep for 200 ns - 2000 ns
    k_busy_wait(1);

    // Loop through all bits (25)
    for (int i = 24; i >= 0; i--) {
        reg_mask = (uint32_t)(1) << i;
        bit = ((sensor_conf & reg_mask) != 0) ? 1 : 0;    
        gpio_pin_set_dt(&cfg->serial_in, 0);
        k_busy_wait(1);
        gpio_pin_set_dt(&cfg->serial_in, 1);
        k_busy_wait(1);
        gpio_pin_set_dt(&cfg->serial_in, bit);

        //sleep for atleast 80 us + 20%
        k_busy_wait(96);        
    } 
    // pull the pin low for 650 us + 20%
    gpio_pin_set_dt(&cfg->direct_link, 0);
    k_busy_wait(780);

    // after condition, set both direct link and serial in to input
    ret = gpio_pin_configure_dt(&cfg->serial_in, GPIO_INPUT);
    if (ret != 0) {
        irq_unlock(key);
        LOG_ERR("Failed to configure serial in GPIO pin %d", cfg->serial_in.pin);
        return ret;
    }
    ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_INPUT);
    if (ret != 0) {
        irq_unlock(key);
        LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
        return ret;
    }

    // Unlock irq
    irq_unlock(key);
    
    return 0;
}

/**
 * @brief Fetch out_of_range,measurement,config from sensor to internal buffer. 
 * 
 * @param dev Pointer to the sensor device
 *
 * @return 0 if successful, negative errno code if failure.
 */
int pyd1598_fetch(const struct device *dev){

    // Variables
    const struct pyd1598_config *cfg; // Get the configuration
    struct pyd1598_data *data; // pyd1598_data
    uint32_t bit = 0; // bit value, will endure bit shifts up to 25 bits
    uint32_t sensor_conf_desired = 0; // Raw bits of the configuration
    uint32_t sensor_conf = 0; // Raw bits of the configuration
    uint32_t measurement = 0; // Raw bits of the measurement
    int key = 0; // Interupt key
    int ret = 0; // return value

    // Check if the device is null
    LOG_DBG("pyd1598_fetch");
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config; // Get the configuration
    data = dev->data; // pyd1598_data
    sensor_conf_desired = data->sensor_conf; // Desired configuration
    key = irq_lock(); // Lock irq
    

    // low to high transition on direct link pin, high for at least 120 us
    ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_OUTPUT_LOW); // initalize to low
    if (ret != 0) {
        irq_unlock(key);
        LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
        return ret;
    }
    gpio_pin_set_dt(&cfg->direct_link, 1); 
    // set to high for at least 120 us + 20%
    k_busy_wait(168);


    // Readout the measurement data
    for (int i = 39; i >= 0; i--) {

        // force low for 200 ns - 2000ns
        ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_OUTPUT_LOW); // initalize to low
        if (ret != 0) {
            irq_unlock(key);
            LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
            return ret;
        }
        // // force low for 200 ns - 2000ns
        // Done by assembly
        
        gpio_pin_set_dt(&cfg->direct_link, 1);
        if (ret != 0) {
            irq_unlock(key);
            LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
            return ret;
        }
        // // force high for 200 ns - 2000ns
        // Done by assembly


        // release the pin, wait for less than 22 us => 5 us
        ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_INPUT); 
        if (ret != 0) {
            irq_unlock(key);
            LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
            return ret;
        }
        k_busy_wait(3);

        // read the bit, uint32_t because it might be shifted up to 25 bits, and code should not compile if not 32 bits
        bit = (uint32_t)(gpio_pin_get_dt(&cfg->direct_link));

        // save the bit in the measurement data
        if (i >= 25) {
            measurement = (measurement) | (bit << (i-25));
        }
        else { // bits 24-0 
            sensor_conf = (sensor_conf) | (bit << i);
        }
    }

    // Force direct link low for at least 1250 us + 20%
    ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_OUTPUT_LOW); // initalize to low
    if (ret != 0) {
        irq_unlock(key);
        LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
        return ret;
    }
    k_busy_wait(1500);
    
    // Release the direct link pin
    ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_INPUT); // initalize to low
    if (ret != 0) {
        irq_unlock(key);
        LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
        return ret;
    }

    // Unlock irq once for all
    irq_unlock(key);

    // for (int i = 24 ; i >= 0; i--) {
    //     // Plot sensor_conf and sensor_conf_desired
    //     LOG_INF("Bit %d| conf: %d| des: %d ", i, (sensor_conf & (1 << i)) >> i, (sensor_conf_desired & (1 << i)) >> i);
    // }
    LOG_INF("conf %d| des %d", sensor_conf, sensor_conf_desired);
    LOG_INF("\n");


    // Check if bits_configuration is the same as bits_configuration_desired
    if (sensor_conf != sensor_conf_desired) {
        LOG_ERR("Configuration read from the sensor does not match desired configuration");
        return -EIO;
    }

    // Save readout data to internal buffer
    data->sensor_conf = sensor_conf;
    data->measurement = measurement;

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
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the device is null
    LOG_DBG("pyd1598_set_reserved_bits");
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Cast the data to the correct type
    cfg = dev->config;
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Set reserved bits in desired configuration, to allow for user to not set them even if encouraged
    sensor_conf = (sensor_conf & ~(PYD1598_RESERVED_2_MASK << PYD1598_RESERVED_2_SHIFT)) | (PYD1598_RESERVED_2_DEC_VALUE << PYD1598_RESERVED_2_SHIFT);
    sensor_conf = (sensor_conf & ~(PYD1598_RESERVED_1_MASK << PYD1598_RESERVED_1_SHIFT)) | (PYD1598_RESERVED_1_DEC_VALUE << PYD1598_RESERVED_1_SHIFT);

    // Save the configuration to the internal buffer
    data->sensor_conf = sensor_conf;

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
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the threshold is out of range, or if the device is null
    LOG_DBG("pyd1598_set_threshold");
    if (threshold > 255 || dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Set raw bits in configuration at 24-17 to threshold, leave the rest of the bits as they are
    sensor_conf = (sensor_conf & ~(PYD1598_THRESHOLD_MASK << PYD1598_THRESHOLD_SHIFT)) | ((uint32_t)(threshold) << PYD1598_THRESHOLD_SHIFT);

    // Save the configuration to the internal buffer
    data->sensor_conf = sensor_conf;

    return 0;
}


/**
 * @brief Get pyd1598 threshold configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * 
 */
int pyd1598_get_threshold(const struct device *dev, uint8_t *threshold){
    // Variables
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the device is null
    LOG_DBG("pyd1598_get_threshold");
    if (dev == NULL || dev->data == NULL || threshold == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Get the threshold from the internal buffer
    *threshold = (sensor_conf & (PYD1598_THRESHOLD_MASK << PYD1598_THRESHOLD_SHIFT)) >> PYD1598_THRESHOLD_SHIFT;

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
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the blind time is out of range, or if the device is null
    LOG_DBG("pyd1598_set_blind_time");
    if (blind_time > 15 || dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Set raw bits in configuration at 16-13 to blind time, leave the rest of the bits as they are
    sensor_conf = (sensor_conf & ~(PYD1598_BLIND_TIME_MASK << PYD1598_BLIND_TIME_SHIFT)) | ((uint32_t)(blind_time) << PYD1598_BLIND_TIME_SHIFT);

    // Save the configuration to the internal buffer
    data->sensor_conf = sensor_conf;

    return 0;
}


/**
 * @brief Get pyd1598 blind time configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param blind_time Pointer to where the blind time value should be stored
 * 
 * @return 0 if successful, negative errno code if failure.
 * */
int pyd1598_get_blind_time(const struct device *dev, uint8_t *blind_time){
    // Variables
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the device is null
    LOG_DBG("pyd1598_get_blind_time");
    if (dev == NULL || dev->data == NULL || blind_time == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Get the blind time from the internal buffer
    *blind_time = (sensor_conf & (PYD1598_BLIND_TIME_MASK << PYD1598_BLIND_TIME_SHIFT)) >> PYD1598_BLIND_TIME_SHIFT;

    return 0;
}


/**
 * @brief Set pyd1598 pulse counter configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param pulse_counter Pulse counter value to set (range 0-3)
 * 
 * @return 0 if successful, negative errno code if failure.
 */
int pyd1598_set_pulse_counter(const struct device *dev, uint8_t pulse_counter){
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the pulse counter is out of range, or if the device is null
    LOG_DBG("pyd1598_set_pulse_counter");
    if (pulse_counter > 3 || dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Set raw bits in configuration at 12-11 to pulse counter, leave the rest of the bits as they are
    sensor_conf = (sensor_conf & ~(PYD1598_PULSE_COUNTER_MASK << PYD1598_PULSE_COUNTER_SHIFT)) | ((uint32_t)(pulse_counter) << PYD1598_PULSE_COUNTER_SHIFT);

    // Save the configuration to the internal buffer
    data->sensor_conf = sensor_conf;

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
    // Variables
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the device is null
    LOG_DBG("pyd1598_get_pulse_counter");
    if (dev == NULL || dev->data == NULL || pulse_counter == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Get the pulse counter from the internal buffer
    *pulse_counter = (sensor_conf & (PYD1598_PULSE_COUNTER_MASK << PYD1598_PULSE_COUNTER_SHIFT)) >> PYD1598_PULSE_COUNTER_SHIFT;

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
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the window time is out of range, or if the device is null
    LOG_DBG("pyd1598_set_window_time");
    if (window_time > 3 || dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Set raw bits in configuration at 10-9 to window time, leave the rest of the bits as they are
    sensor_conf = (sensor_conf & ~(PYD1598_WINDOW_TIME_MASK << PYD1598_WINDOW_TIME_SHIFT)) | ((uint32_t)(window_time) << PYD1598_WINDOW_TIME_SHIFT);

    // Save the configuration to the internal buffer
    data->sensor_conf = sensor_conf;

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
    // Variables
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the device is null
    LOG_DBG("pyd1598_get_window_time");
    if (dev == NULL || dev->data == NULL || window_time == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Get the window time from the internal buffer
    *window_time = (sensor_conf & (PYD1598_WINDOW_TIME_MASK << PYD1598_WINDOW_TIME_SHIFT)) >> PYD1598_WINDOW_TIME_SHIFT;

    return 0;
}


/**
 * @brief Set pyd1598 operation mode configuration to the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param operation_mode Operation mode (PYD1598_FORCED_READOUT, PYD1598_WAKE_UP)
 * 
 * @return 0 if successful, negative errno code if failure.
 */
int pyd1598_set_operation_mode(const struct device *dev, enum pyd1598_operation_mode operation_mode){
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the device is null
    LOG_DBG("pyd1598_set_operation_mode");
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Set raw bits in configuration at 8-7 to operation mode, leave the rest of the bits as they are
    sensor_conf = (sensor_conf & ~(PYD1598_OPERATION_MODE_MASK << PYD1598_OPERATION_MODE_SHIFT)) | ((uint32_t)(operation_mode) << PYD1598_OPERATION_MODE_SHIFT);
    // se

    // Save the configuration to the internal buffer
    data->sensor_conf = sensor_conf;

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
    // Variables
    struct pyd1598_data *data;
    uint32_t sensor_conf;
    uint32_t operation_mode_internal;

    // Check if the device is null
    LOG_DBG("pyd1598_get_operation_mode");
    if (dev == NULL || dev->data == NULL || operation_mode == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Get the operation mode from the internal buffer
    operation_mode_internal = (sensor_conf & (PYD1598_OPERATION_MODE_MASK << PYD1598_OPERATION_MODE_SHIFT)) >> PYD1598_OPERATION_MODE_SHIFT;
    if (operation_mode_internal == PYD1598_FORCED_READOUT) {
        *operation_mode = PYD1598_FORCED_READOUT;
    }
    else if (operation_mode_internal == PYD1598_WAKE_UP) {
        *operation_mode = PYD1598_WAKE_UP;
    }
    else {
        LOG_ERR("Operation mode read from internal buffer is not valid");
        return -EIO;
    }
    
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
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the device is null
    LOG_DBG("pyd1598_set_signal_source");
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Set raw bits in configuration at 6-5 to signal source, leave the rest of the bits as they are
    sensor_conf = (sensor_conf & ~(PYD1598_SIGNAL_SOURCE_MASK << PYD1598_SIGNAL_SOURCE_SHIFT)) | ((uint32_t)(signal_source) << PYD1598_SIGNAL_SOURCE_SHIFT);

    // Save the configuration to the internal buffer
    data->sensor_conf = sensor_conf;

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
    // Variables
    struct pyd1598_data *data;
    uint32_t sensor_conf;
    uint32_t signal_source_internal;

    // Check if the device is null
    LOG_DBG("pyd1598_get_signal_source");
    if (dev == NULL || dev->data == NULL || signal_source == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Get the signal source from the internal buffer
    signal_source_internal = (sensor_conf & (PYD1598_SIGNAL_SOURCE_MASK << PYD1598_SIGNAL_SOURCE_SHIFT)) >> PYD1598_SIGNAL_SOURCE_SHIFT;
    if (signal_source_internal == PYD1598_PIR_BPF) {
        *signal_source = PYD1598_PIR_BPF;
    }
    else if (signal_source_internal == PYD1598_PIR_LPF) {
        *signal_source = PYD1598_PIR_LPF;
    }
    else if (signal_source_internal == PYD1598_TEMPERATURE_SENSOR) {
        *signal_source = PYD1598_TEMPERATURE_SENSOR;
    }
    else {
        LOG_ERR("Signal source read from internal buffer is not valid");
        return -EIO;
    }

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
int pyd1598_set_hpf_cutoff(const struct device *dev, enum pyd1598_hpf_cutoff hpf_cut_off){
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the device is null
    LOG_DBG("pyd1598_set_hpf_cut_off");
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Set raw bits in configuration at 2 to hpf cut off, leave the rest of the bits as they are
    sensor_conf = (sensor_conf & ~(PYD1598_HPF_CUT_OFF_MASK << PYD1598_HPF_CUT_OFF_SHIFT)) | ((uint32_t)(hpf_cut_off) << PYD1598_HPF_CUT_OFF_SHIFT);

    // Save the configuration to the internal buffer
    data->sensor_conf = sensor_conf;

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
int pyd1598_get_hpf_cutoff(const struct device *dev, enum pyd1598_hpf_cutoff *hpf_cut_off){
    // Variables
    struct pyd1598_data *data;
    uint32_t sensor_conf;
    uint32_t hpf_cut_off_internal;

    // Check if the device is null
    LOG_DBG("pyd1598_get_hpf_cut_off");
    if (dev == NULL || dev->data == NULL || hpf_cut_off == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Get the HPF Cut Off from the internal buffer
    hpf_cut_off_internal = (sensor_conf & (PYD1598_HPF_CUT_OFF_MASK << PYD1598_HPF_CUT_OFF_SHIFT)) >> PYD1598_HPF_CUT_OFF_SHIFT;
    if (hpf_cut_off_internal == PYD1598_HPF_CUTOFF_0_4HZ) {
        *hpf_cut_off = PYD1598_HPF_CUTOFF_0_4HZ;
    }
    else if (hpf_cut_off_internal == PYD1598_HPF_CUTOFF_0_2HZ) {
        *hpf_cut_off = PYD1598_HPF_CUTOFF_0_2HZ;
    }
    else {
        LOG_ERR("HPF Cut Off read from internal buffer is not valid");
        return -EIO;
    }

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
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    uint32_t sensor_conf;

    // Check if the device is null
    LOG_DBG("pyd1598_set_count_mode");
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Set raw bits in configuration at 0 to count mode, leave the rest of the bits as they are
    sensor_conf = (sensor_conf & ~(PYD1598_COUNT_MODE_MASK << PYD1598_COUNT_MODE_SHIFT)) | ((uint32_t)(count_mode) << PYD1598_COUNT_MODE_SHIFT);

    // Save the configuration to the internal buffer
    data->sensor_conf = sensor_conf;

    return 0;
}


/**
 * @brief Get pyd1598 Count Mode configuration from the internal buffer.
 * 
 * @param dev Pointer to the sensor device
 * @param count_mode Pointer to where the Count Mode value should be stored
 * 
 * @return 0 if successful, negative errno code if failure.
 */
int pyd1598_get_count_mode(const struct device *dev, enum pyd1598_count_mode *count_mode){
    // Variables
    struct pyd1598_data *data;
    uint32_t sensor_conf;
    uint32_t count_mode_internal;

    // Check if the device is null
    LOG_DBG("pyd1598_get_count_mode");
    if (dev == NULL || dev->data == NULL || count_mode == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    data = dev->data;
    sensor_conf = data->sensor_conf;

    // Get the Count Mode from the internal buffer
    count_mode_internal = (sensor_conf & (PYD1598_COUNT_MODE_MASK << PYD1598_COUNT_MODE_SHIFT)) >> PYD1598_COUNT_MODE_SHIFT;
    if (count_mode_internal == PYD1598_COUNT_SIGN_CHANGE) {
        *count_mode = PYD1598_COUNT_SIGN_CHANGE;
    }
    else if (count_mode_internal == PYD1598_COUNT_ALL) {
        *count_mode = PYD1598_COUNT_ALL;
    }
    else {
        LOG_ERR("Count Mode read from internal buffer is not valid");
        return -EIO;
    }

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
    // access configuration in pyd1598_data and set all values to default
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }
    pyd1598_set_threshold(dev, 31);
    pyd1598_set_blind_time(dev, 6);
    pyd1598_set_pulse_counter(dev, 0);
    pyd1598_set_window_time(dev, 0);
    pyd1598_set_operation_mode(dev, PYD1598_WAKE_UP);
    pyd1598_set_signal_source(dev, PYD1598_PIR_LPF);
    pyd1598_set_hpf_cutoff(dev, PYD1598_HPF_CUTOFF_0_4HZ);
    pyd1598_set_count_mode(dev, PYD1598_COUNT_ALL);
    pyd1598_set_reserved_bits(dev);

    return 0;
}


/**
 * @brief Reset the sensor, only allowed in wake-up mode.
 * 
 * @param dev Pointer to the sensor device
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_reset(const struct device *dev) {
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    enum pyd1598_operation_mode operation_mode;
    int ret;

    // Check if the device is null
    LOG_DBG("pyd1598_reset");
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;

    // Check if the sensor is in wake-up mode
    pyd1598_get_operation_mode(dev, &operation_mode);
    if (operation_mode != PYD1598_WAKE_UP) {
        LOG_ERR("Sensor is not in wake-up mode, reset is only possible in wake-up mode");
        return -EIO;
    }

    // Configure the direct link pin to output and push direct link pin low for at least 160 us + 20%
    ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_OUTPUT_LOW);
    if (ret != 0) {
        LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
        return ret;
    }
    k_busy_wait(192);

    // Release the direct link pin
    ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
        return ret;
    }

    return 0;
}


/**
 * @brief Reset the sensor and fetch new data to the internal buffer, only allowed in wake-up mode.
 * 
 * @param dev Pointer to the sensor device
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_reset_and_fetch(const struct device *dev) {
    // Variables
    int ret;
    enum pyd1598_operation_mode operation_mode;
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;

    // Check if the device is null
    LOG_DBG("pyd1598_reset_and_fetch");
    if (dev == NULL || dev->data == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;

    // Check if the sensor is in wake-up mode
    pyd1598_get_operation_mode(dev, &operation_mode);
    if (operation_mode != PYD1598_WAKE_UP) {
        LOG_ERR("Sensor is not in wake-up mode, reset is only possible in wake-up mode");
        return -EIO;
    }

    // Configure the direct link pin to output and push direct link pin low for at least 160 us + 20%
    ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_OUTPUT_LOW);
    if (ret != 0) {
        LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
        return ret;
    }
    k_busy_wait(192);

    // Fetch the new data to the internal buffer
    ret = pyd1598_fetch(dev);
    if (ret != 0) {
        LOG_ERR("Failed to fetch new data after reset");
        return ret;
    }

    // Set the direct link pin to input, it might already be input
    ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
        return ret;
    }
    
    return 0;
}


/**
 * @brief Check if the sensor has triggered, only allowed in wake-up mode.
 * 
 * @param dev Pointer to the sensor device
 * 
 * @return 0 if successful, negative errno code if failure.
*/
int pyd1598_poll_triggered(const struct device *dev, bool *has_triggered){
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    enum pyd1598_operation_mode operation_mode;
    int ret = 0;

    // Check if the device is null
    LOG_DBG("pyd1598_has_triggerd");
    if (dev == NULL || dev->data == NULL || has_triggered == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;

    // Check if the sensor is in wake-up mode
    pyd1598_get_operation_mode(dev, &operation_mode);
    if (operation_mode != PYD1598_WAKE_UP) {
        LOG_ERR("Sensor is not in wake-up mode, polling triggered is only possible in wake-up mode");
        return -EIO;
    }
    
    // Set GPIO pin to input
    ret = gpio_pin_configure_dt(&cfg->direct_link, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("Failed to configure direct link GPIO pin %d", cfg->direct_link.pin);
        return ret;
    }

    // Read the direct link pin and save the value to has_triggered
    ret = gpio_pin_get_dt(&cfg->direct_link);
    if (ret < 0) {
        LOG_ERR("Failed to read direct link GPIO pin %d", cfg->direct_link.pin);
        return ret;
    }
    *has_triggered = (bool)ret;

    return 0;
}


/**
 * @brief Get the temperature readout from the sensor, sensor source must be set to temperature sensor.
 * 
 * @param dev Pointer to the sensor device
 * @param adc_counts Pointer to where the ADC counts value should be stored
 * @param out_of_range Pointer to where the out of range value should be stored
 * 
 * @return 0 if successful, negative errno code if failure.
 */
int pyd1598_get_temperature_readout(const struct device *dev, uint16_t *adc_counts, bool *out_of_range){
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    uint32_t measurement;
    enum pyd1598_signal_source signal_source;
    int ret;
    uint16_t adc_counts_internal;
    bool out_of_range_internal;


    // Check if the device is null
    LOG_DBG("get_temperature_readout");
    if (dev == NULL || dev->data == NULL || adc_counts == NULL || out_of_range == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;
    measurement = data->measurement;

    // Check that signal source is set to temperature sensor
    ret = pyd1598_get_signal_source(dev, &signal_source);
    if (ret != 0) {
        LOG_ERR("Failed to get signal source");
        return ret;
    }
    if (signal_source != PYD1598_TEMPERATURE_SENSOR) {
        LOG_ERR("Signal source is not set to temperature sensor");
        return -EIO;
    }

    // Get the measurement from the internal buffer
    adc_counts_internal = (uint16_t)((measurement & (PYD1598_ADC_COUNTS_MASK << PYD1598_ADC_COUNTS_SHIFT)) >> PYD1598_ADC_COUNTS_SHIFT);

    // Get the out of range from the internal buffer
    out_of_range_internal = (bool)((measurement & (PYD1598_OUT_OF_RANGE_MASK << PYD1598_OUT_OF_RANGE_SHIFT)) >> PYD1598_OUT_OF_RANGE_SHIFT);

    // Save the values to the pointers
    *adc_counts = adc_counts_internal;
    *out_of_range = out_of_range_internal;

    return 0;
}



/**
 * @brief Get the Band Pass Filterd (BPF) readout from the sensor, sensor source must be set to PIR BPF.
 * 
 * @param dev Pointer to the sensor device
 * @param adc_counts Pointer to where the ADC counts value should be stored
 * @param out_of_range Pointer to where the out of range value should be stored
 * 
 * @return 0 if successful, negative errno code if failure.
 */
int pyd1598_get_bpf_readout(const struct device *dev, int16_t *adc_counts, bool *out_of_range){
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    uint32_t measurement;
    enum pyd1598_signal_source signal_source;
    int ret;
    int16_t adc_counts_internal;
    bool out_of_range_internal;

    // Check if the device is null
    LOG_DBG("get_bpf_readout");
    if (dev == NULL || dev->data == NULL || adc_counts == NULL || out_of_range == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;
    measurement = data->measurement;

    // Check that signal source is set to PIR BPF
    ret = pyd1598_get_signal_source(dev, &signal_source);
    if (ret != 0) {
        LOG_ERR("Failed to get signal source");
        return ret;
    }
    if (signal_source != PYD1598_PIR_BPF) {
        LOG_ERR("Signal source is not set to PIR BPF");
        return -EIO;
    }

    // Get the measurement from the internal buffer
    adc_counts_internal = (int16_t)((measurement & (PYD1598_ADC_COUNTS_MASK << PYD1598_ADC_COUNTS_SHIFT)) >> PYD1598_ADC_COUNTS_SHIFT);

    // Get the out of range from the internal buffer
    out_of_range_internal = (bool)((measurement & (PYD1598_OUT_OF_RANGE_MASK << PYD1598_OUT_OF_RANGE_SHIFT)) >> PYD1598_OUT_OF_RANGE_SHIFT);

    // Save the values to the pointers
    *adc_counts = adc_counts_internal;
    *out_of_range = out_of_range_internal;

    return 0;
}



// get_lpf_readout: Get the Low Pass Filterd (LPF) readout from the sensor
/**
 * @brief Get the LPF readout from the sensor, sensor source must be set to PIR LPF.
 * 
 * @param dev Pointer to the sensor device
 * @param adc_counts Pointer to where the ADC counts value should be stored
 * @param out_of_range Pointer to where the out of range value should be stored
 * 
 * @return 0 if successful, negative errno code if failure.
 */
int pyd1598_get_lpf_readout(const struct device *dev, uint16_t *adc_counts, bool *out_of_range){
    // Variables
    const struct pyd1598_config *cfg;
    struct pyd1598_data *data;
    uint32_t measurement;
    enum pyd1598_signal_source signal_source;
    int ret;
    uint16_t adc_counts_internal;
    bool out_of_range_internal;

    // Check if the device is null
    LOG_DBG("get_lpf_readout");
    if (dev == NULL || dev->data == NULL || adc_counts == NULL || out_of_range == NULL) {
        return -EINVAL;
    }

    // Declare the variables
    cfg = dev->config;
    data = dev->data;
    measurement = data->measurement;

    // Check that signal source is set to PIR LPF
    ret = pyd1598_get_signal_source(dev, &signal_source);
    if (ret != 0) {
        LOG_ERR("Failed to get signal source");
        return ret;
    }
    if (signal_source != PYD1598_PIR_LPF) {
        LOG_ERR("Signal source is not set to PIR LPF");
        return -EIO;
    }

    // Get the measurement from the internal buffer
    adc_counts_internal = (uint16_t)((measurement & (PYD1598_ADC_COUNTS_MASK << PYD1598_ADC_COUNTS_SHIFT)) >> PYD1598_ADC_COUNTS_SHIFT);

    // Get the out of range from the internal buffer
    out_of_range_internal = (bool)((measurement & (PYD1598_OUT_OF_RANGE_MASK << PYD1598_OUT_OF_RANGE_SHIFT)) >> PYD1598_OUT_OF_RANGE_SHIFT);

    // Save the values to the pointers
    *adc_counts = adc_counts_internal;
    *out_of_range = out_of_range_internal;

    return 0;
}


#define pyd1598_INIT(index)                                                      \
	static struct pyd1598_data pyd1598_data_##index = {0};                        \
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

