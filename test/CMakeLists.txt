include_directories(${PROJECT_SOURCE_DIR}/ext ${PROJECT_SOURCE_DIR}/include)

if (NETCDF_FOUND)
add_executable(test_interface "test_interface.cxx")
target_link_libraries(test_interface ${NETCDF_LIBRARY})
endif (NETCDF_FOUND)
