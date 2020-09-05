#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include <netcdfpp.h>

TEST_CASE( "create_and_open_file", "[factorial]" ) {
    auto file = netcdf4::File::create("test_file.nc");
    file.close();
    file = netcdf4::File::open("test_file.nc");
}
