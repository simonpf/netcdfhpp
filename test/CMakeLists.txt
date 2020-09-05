include_directories(${CMAKE_SOURCE_DIR}/ext ${CMAKE_SOURCE_DIR}/include)

if (NETCDF_FOUND)
add_executable(test_create_open "test_create_and_open.cxx")
target_link_libraries(test_create_open ${NETCDF_LIBRARY})
endif (NETCDF_FOUND)
