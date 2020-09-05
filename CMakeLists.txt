cmake_minimum_required (VERSION 3.10)
project (netcdfpp)

#
# C++20
#

set (CMAKE_CXX_STANDARD 20)

#
# Build flags
#

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -finline-functions")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

#
# Find required packages
#

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(NetCDF)

#
# External code
#

add_subdirectory(test)
