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




#include <zephyr/logging/log.h> // Documentation: https://docs.zephyrproject.org/latest/services/logging/index.html
LOG_MODULE_REGISTER(pyd1598_driver, LOG_LEVEL_DBG); //This is the registration of the log module and should only be created ONCE.
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/pm/device.h>
#include <zephyr/sys/poweroff.h>
#include <soc.h>
#include <hal/nrf_gpio.h>
#include <hal/nrf_regulators.h>


#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>



// Define which device tree node to use for this driver
/*
* E.g. if it exists pyd1598_master_0, pyd1598_master_1, pyd1598_master_2, ... pyd1598_master_n in the device tree
* and you want to use pyd1598_master_0, set the following line as:
* #define master_node pyd1598_master_0
*/
#define master_node pyd1598_master

// Define master node identifier
#define master_node_identifier DT_NODELABEL(master_node)

// Number of PYD1598 of #master_node
#define COUNT_CHILDREN(child) +1
#define NUM_PYD1598 (0 DT_FOREACH_CHILD(master_node_identifier, COUNT_CHILDREN))


// Macro to count the number of OKAY children of #master_node
#define COUNT_CHILDREN_OKAY(child) +1
#define NUM_PYD1598_OKAY (0 DT_FOREACH_CHILD_STATUS_OKAY(master_node_identifier, COUNT_CHILDREN_OKAY))

// macro to print label of child nodes
#define PRINT_CHILD_NODE(node_id) LOG_INF("Child node: %s\n", DT_LABEL(node_id));



static struct master_data_struct {
    const struct device *sensors[NUM_PYD1598_OKAY];
    int count;
};


// Intialize the master data 
static struct master_data_struct master_data;

// Macro to add child node to master data, to be used in DT_FOREACH_CHILD_STATUS_OKAY must be a macro
static int add_child_node_iterator = 0;
#define ADD_CHILD_NODE_TO_MASTER_DATA(node_id) master_data.sensors[add_child_node_iterator++] = DEVICE_DT_GET(node_id);


// Function to initialize the master data
int master_data_init() {
    // Get the number of children of the master node that are status okay 
    master_data.count = NUM_PYD1598_OKAY;
    // Get the device of all sensors that are status okay and save them in the master data
    DT_FOREACH_CHILD_STATUS_OKAY(master_node_identifier, ADD_CHILD_NODE_TO_MASTER_DATA);

    while (1)
    {
        //print the number of children
        LOG_INF("Number of children: %d", master_data.count);

        for(int i = 0; i < master_data.count; i++) {
            LOG_INF("Sensor %d: %s", i, master_data.sensors[i]->name);
        }
        // Sleep for 1 second
        k_sleep(K_SECONDS(1));

    }
    



}


