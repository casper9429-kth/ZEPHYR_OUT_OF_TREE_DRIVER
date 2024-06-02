# ZEPHYR_OUT_OF_TREE_DRIVER
This repo have the goal to learn and show how to add a out of tree driver in zephyr, and document common problems and how to solve them. For now it will add an empty driver for the PYD1598 sensor and access it in main.c.

# Primary Sources:
* [Adding an Out-of-Tree sensor driver to Zephyr](https://blog.golioth.io/adding-an-out-of-tree-sensor-driver-to-zephyr/)
* [ZEPHYR: Devicetree HOWTOs](https://docs.zephyrproject.org/latest/build/dts/howtos.html#get-devicetree-outputs)
* [ZEPHYR: Sensor Shell Example](https://github.com/zephyrproject-rtos/zephyr/tree/main/samples/sensor/sensor_shell)
* [devzone.nordicsemi](https://devzone.nordicsemi.com/f/nordic-q-a/97106/difference-between-zephyr_library_sources-and-target_sources-in-cmakelists-txt)

# STATUS:
* [x] Make the device driver accesable in main.c
* [] Make the full PYD1598 device driver with bitbanging
* [] Test the full PYD1598 device driver 
* [] Make a wrapper clustering multiple pyd1598 devices to act as a single device

# QUESTIONS:
* How to add a out of tree driver in zephyr, why doesn't the current way work?
* How to properly access the driver in main.c? Right now it is added in `drivers/sensor/pyd1598/CMakeLists.txt` with `zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})`. Is this the correct way?

# PROBLEMS and SOLUTIONS:

## Problem 1: 
* **Problem:** When adding the driver in `drivers/sensor/pyd1598/CMakeLists.txt` the driver is compiled but does not get linked to device tree.

In file: `drivers/sensor/pyd1598/CMakeLists.txt`
```
zephyr_library()
zephyr_library_sources(pyd1598.c)
```
* **Solution:** Add the driver using cmake's `target_sources` in `CMakeLists.txt`.
In file: `drivers/sensor/pyd1598/CMakeLists.txt`
```
# Ensure the directory containing <driver.h> is included
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pyd1598.c)
```

## Problem 2:
When compiling the driver: `offset.h: error, this error automatically gets fixed by adding the driver in `CMakeLists.txt` as mentioned in Problem 1. But if the error still persists, you can add this to the driver's `CMakeLists.txt`:
```
# https://github.com/zephyrproject-rtos/zephyr/issues/67268
add_dependencies(${ZEPHYR_CURRENT_LIBRARY} offsets_h)
```


# Build:
1. Follow the [Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html) **use venv**
2. Clone this repo and place it anywhere in zephyr's directory
3. Make sure `source <where-you-placed-it-during-zephyr-getting-stated>/.venv/bin/activate` is activated 
3. `cd into this repo`
4. `west build -b nrf9160dk_nrf9160_ns --pristine`
5. `west flash`
