# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/teo/Desktop/espidf/esp-idf/components/bootloader/subproject"
  "/Users/teo/esp/esp-idf/examples/peripherals/i2c/MateoLCDCurrent/EPElectric_CurrentSensor/build/bootloader"
  "/Users/teo/esp/esp-idf/examples/peripherals/i2c/MateoLCDCurrent/EPElectric_CurrentSensor/build/bootloader-prefix"
  "/Users/teo/esp/esp-idf/examples/peripherals/i2c/MateoLCDCurrent/EPElectric_CurrentSensor/build/bootloader-prefix/tmp"
  "/Users/teo/esp/esp-idf/examples/peripherals/i2c/MateoLCDCurrent/EPElectric_CurrentSensor/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/teo/esp/esp-idf/examples/peripherals/i2c/MateoLCDCurrent/EPElectric_CurrentSensor/build/bootloader-prefix/src"
  "/Users/teo/esp/esp-idf/examples/peripherals/i2c/MateoLCDCurrent/EPElectric_CurrentSensor/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/teo/esp/esp-idf/examples/peripherals/i2c/MateoLCDCurrent/EPElectric_CurrentSensor/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/teo/esp/esp-idf/examples/peripherals/i2c/MateoLCDCurrent/EPElectric_CurrentSensor/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
