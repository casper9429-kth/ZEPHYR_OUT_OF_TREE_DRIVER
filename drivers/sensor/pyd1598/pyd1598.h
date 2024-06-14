#ifndef ZEPHYR_INCLUDE_DRIVERS_SENSOR_PYD1598_H_
#define ZEPHYR_INCLUDE_DRIVERS_SENSOR_PYD1598_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>


// enum of pyd1598 operation modes
enum pyd1598_operation_mode {
    PYD1598_FORCED_READOUT = 0,
    PYD1598_WAKE_UP = 2
};

// enum of pyd1598 signal source
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


// Fill in with functions when implemented

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_DRIVERS_SENSOR_PYD1598_H_ */