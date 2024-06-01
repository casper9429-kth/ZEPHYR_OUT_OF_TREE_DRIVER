# ZEPHYR_OUT_OF_TREE_DRIVER
This repo have the goal to learn and show how to add a out of tree driver in zephyr, and document common problems and how to solve them. For now it doesn't work.

# Primary Sources:
* [Adding an Out-of-Tree sensor driver to Zephyr](https://blog.golioth.io/adding-an-out-of-tree-sensor-driver-to-zephyr/)
* [ZEPHYR: Devicetree HOWTOs](https://docs.zephyrproject.org/latest/build/dts/howtos.html#get-devicetree-outputs)
* [ZEPHYR: Sensor Shell Example](https://github.com/zephyrproject-rtos/zephyr/tree/main/samples/sensor/sensor_shell)


# STATUS:
* [x] Make the device driver accesable in main.c
* [x] Make the full PYD1598 device driver with bitbanging
* [x] Test the full PYD1598 device driver 
* [x] Make a wrapper clustering multiple pyd1598 devices to act as a single device

# QUESTIONS:
* How to add a out of tree driver in zephyr, why doesn't the current way work?
* How to properly access the driver in main.c? Right now it is added in `drivers/sensor/pyd1598/CMakeLists.txt` with `zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})`. Is this the correct way?