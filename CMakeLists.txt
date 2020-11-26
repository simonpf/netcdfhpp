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

if (NOT CMAKE_VERSION VERSION_LESS 3.0)
  include (CMakePackageConfigHelpers)

  # Imported target support
  add_library (headers INTERFACE)
  add_library (netcdfhpp::headers ALIAS headers)
  target_include_directories (headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )

  install (TARGETS headers EXPORT netcdfhpp)

  configure_package_config_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/netcdfhppConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/netcdfhpp.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/netcdfhpp
    NO_CHECK_REQUIRED_COMPONENTS_MACRO # Eigen does not provide components
  )

  export (TARGETS headers NAMESPACE netcdfhpp:: FILE netcdfhppConfig.cmake)
  # Export Eigen3 package to CMake registry such that it can be easily found by
  # CMake even if it has not been installed to a standard directory.
  export (PACKAGE netcdfhpp)

  install (EXPORT netcdfhpp NAMESPACE netcdfhpp:: DESTINATION ${CMAKE_INSTALL_PREFIX}/netcdfhpp)
endif (NOT CMAKE_VERSION VERSION_LESS 3.0)
