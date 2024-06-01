# Install script for directory: /home/casper/golioth-ncs-workspace/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/home/casper/zephyr-sdk-0.16.4/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/zephyr/soc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/golioth-firmware-sdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/nrf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/wfa-qt-control-app/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/cjson/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/azure-sdk-for-c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/cirrus-logic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/memfault-firmware-sdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/canopennode/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/chre/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/hal_wurthelektronik/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/liblc3/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/lz4/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/nanopb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/picolibc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/tinycrypt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/TraceRecorder/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/uoscore-uedhoc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/zcbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/zscilib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/nrfxlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/modules/connectedhomeip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/casper/golioth-ncs-workspace/modules/lib/golioth-firmware-sdk/examples/zephyr/flox_fixed_com_mcu/applications/ZEPHYR_OUT_OF_TREE_DRIVER/build/zephyr/cmake/reports/cmake_install.cmake")
endif()

