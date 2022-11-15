# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/teo/Desktop/espidf/esp-idf/components/bootloader/subproject"
  "/Users/teo/Desktop/SeniorDesignP2/SeniorProjectTestCurrentSensor/EPElectric_CurrentSensor/build/bootloader"
  "/Users/teo/Desktop/SeniorDesignP2/SeniorProjectTestCurrentSensor/EPElectric_CurrentSensor/build/bootloader-prefix"
  "/Users/teo/Desktop/SeniorDesignP2/SeniorProjectTestCurrentSensor/EPElectric_CurrentSensor/build/bootloader-prefix/tmp"
  "/Users/teo/Desktop/SeniorDesignP2/SeniorProjectTestCurrentSensor/EPElectric_CurrentSensor/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/teo/Desktop/SeniorDesignP2/SeniorProjectTestCurrentSensor/EPElectric_CurrentSensor/build/bootloader-prefix/src"
  "/Users/teo/Desktop/SeniorDesignP2/SeniorProjectTestCurrentSensor/EPElectric_CurrentSensor/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/teo/Desktop/SeniorDesignP2/SeniorProjectTestCurrentSensor/EPElectric_CurrentSensor/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/teo/Desktop/SeniorDesignP2/SeniorProjectTestCurrentSensor/EPElectric_CurrentSensor/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
