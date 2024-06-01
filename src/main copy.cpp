// pyd1598_sensor_array.c

// Define <pyd1598_master> name in device tree

// Get the number of pyd1598 devices in <pyd1598_master> node that are status okay

// Get and save all pyd1598 devices with status okay in <pyd1598_master> node in a struct 

// Make functionality to get the number of pyd1598 devices outside this function

// Interface with pyd1598 devices by index
//  * Configure the device by index
//  * Get the number of devices
//  * Get the configuration of the device
//  * Read the device by index
//  * Reset the device by index
//  * Get the device name by index
//  * Check if any device has been triggerd
//  * Configure all devices


// https://devzone.nordicsemi.com/f/nordic-q-a/100815/out-of-tree-driver---zephyr-freestanding-app
// https://blog.golioth.io/adding-an-out-of-tree-sensor-driver-to-zephyr/
// https://mind.be/zephyr-tutorial-105-writing-a-simple-device-driver/
// https://docs.zephyrproject.org/latest/build/dts/howtos.html#get-devicetree-outputs



#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main_module, LOG_LEVEL_DBG); // Use a unique log module name


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
// Define which device tree node to use for this driver
/*
* E.g. if it exists pyd1598_master_0, pyd1598_master_1, pyd1598_master_2, ... pyd1598_master_n in the device tree
* and you want to use pyd1598_master_0, set the following line as:
* #define master_node pyd1598_master_0
*/
#define master_node pyd1598_0
// Define master node identifier
#define master_node_identifier DT_NODELABEL(master_node)

// Number of PYD1598 of #master_node
#define COUNT_CHILDREN(child) +1
#define NUM_PYD1598 (0 DT_FOREACH_CHILD(master_node_identifier, COUNT_CHILDREN))


// Macro to count the number of OKAY children of #master_node
#define COUNT_CHILDREN_OKAY(child) +1
#define NUM_PYD1598_OKAY (0 DT_FOREACH_CHILD_STATUS_OKAY(master_node_identifier, COUNT_CHILDREN_OKAY))



// Define a struct to hold the pyd1598 devices
struct master_data_struct
{    
    // Array of pyd1598 devices
    const struct device *pyd1598_devices[NUM_PYD1598_OKAY];
    // Number of pyd1598 devices
    int num_pyd1598_devices;
};


// Define an instance of the struct
// static master_data_struct master_data;
// static const struct device *array_or_childrens[NUM_PYD1598_OKAY] = {DT_FOREACH_CHILD_STATUS_OKAY_SEP(master_node_identifier,DEVICE_DT_GET,(,))};

int main() {
    // static const struct device *array_or_childrens[NUM_PYD1598_OKAY] = {DT_FOREACH_CHILD_STATUS_OKAY_SEP(master_node_identifier,DEVICE_DT_GET,(,))};

    // 
    const struct device *sensor = DEVICE_DT_GET(master_node_identifier);


    while (1)
    {
        /* code */



    }
    
    

    return 0;
}


