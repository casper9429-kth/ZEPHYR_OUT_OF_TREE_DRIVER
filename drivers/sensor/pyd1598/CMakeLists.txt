# Ensure the directory containing <driver.h> is included
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Compile the source files into a library
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pyd1598.c)
#https://github.com/zephyrproject-rtos/zephyr/issues/67268
# add_dependencies(${ZEPHYR_CURRENT_LIBRARY} offsets_h)
