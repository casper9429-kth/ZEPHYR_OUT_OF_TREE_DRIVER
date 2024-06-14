#ifndef ZEPHYR_INCLUDE_DRIVERS_SENSOR_PYD1598_H_
#define ZEPHYR_INCLUDE_DRIVERS_SENSOR_PYD1598_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>


// Enums
enum pyd1598_operation_mode {
    PYD1598_FORCED_READOUT = 0,
    PYD1598_WAKE_UP = 2
};

enum pyd1598_signal_source {
    PYD1598_PIR_BPF = 0,
    PYD1598_PIR_LPF = 1,
    PYD1598_TEMPERATURE_SENSOR = 3

};

enum pyd1598_hpf_cutoff {
    PYD1598_HPF_CUTOFF_0_4HZ = 0,
    PYD1598_HPF_CUTOFF_0_2HZ,
};

enum pyd1598_count_mode {
    PYD1598_COUNT_SIGN_CHANGE = 0,
    PYD1598_COUNT_ALL
};


// Functions
// push and fetch functions are used to push and fetch data from the sensor to internal buffer of the driver
int pyd1598_push(const struct device *dev);
int pyd1598_fetch(const struct device *dev);

// set and get functions used to set and get configuration parameters to and from the internal buffer of the driver
int pyd1598_set_reserved_bits(const struct device *dev);

int pyd1598_set_threshold(const struct device *dev,uint8_t threshold);
int pyd1598_get_threshold(const struct device *dev,uint8_t *threshold);

int pyd1598_set_blind_time(const struct device *dev,uint8_t blind_time);
int pyd1598_get_blind_time(const struct device *dev,uint8_t *blind_time);

int pyd1598_set_pulse_counter(const struct device *dev,uint8_t pulse_counter);
int pyd1598_get_pulse_counter(const struct device *dev,uint8_t *pulse_counter);

int pyd1598_set_window_time(const struct device *dev,uint8_t window_time);
int pyd1598_get_window_time(const struct device *dev,uint8_t *window_time);

int pyd1598_set_operation_mode(const struct device *dev,enum pyd1598_operation_mode mode);
int pyd1598_get_operation_mode(const struct device *dev,enum pyd1598_operation_mode *mode);

int pyd1598_set_signal_source(const struct device *dev,enum pyd1598_signal_source source);
int pyd1598_get_signal_source(const struct device *dev,enum pyd1598_signal_source *source);

int pyd1598_set_hpf_cutoff(const struct device *dev,enum pyd1598_hpf_cutoff cutoff);
int pyd1598_get_hpf_cutoff(const struct device *dev,enum pyd1598_hpf_cutoff *cutoff);

int pyd1598_set_count_mode(const struct device *dev,enum pyd1598_count_mode mode);
int pyd1598_get_count_mode(const struct device *dev,enum pyd1598_count_mode *mode);

int pyd1598_set_default_configuration(const struct device *dev);

// interface functions
int pyd1598_reset(const struct device *dev); 
int pyd1598_reset_and_fetch(const struct device *dev);
int pyd1598_poll_triggered(const struct device *dev,bool *triggered);
int pyd1598_get_temperature_readout(const struct device *dev, uint16_t *adc_counts, bool *out_of_range);
int pyd1598_get_bpf_readout(const struct device *dev, int16_t *adc_counts, bool *out_of_range);
int pyd1598_get_lpf_readout(const struct device *dev, uint16_t *adc_counts, bool *out_of_range);

// Fill in with functions when implemented

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_DRIVERS_SENSOR_PYD1598_H_ */