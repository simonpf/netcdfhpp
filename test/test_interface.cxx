#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include <netcdf.hpp>
#include <cassert>

netcdf4::File create_test_file(std::string name) {
    auto file = netcdf4::File::create(name);
    file.add_dimension("dimension_1", 10);
    file.add_dimension("dimension_2", 20);
    file.add_dimension("dimension_unlimited");

    std::vector<std::string> dimensions = {
        "dimension_unlimited", "dimension_1", "dimension_2"};
    file.add_variable("int_variable", dimensions, netcdf4::Type::Int);
    file.add_variable("float_variable", dimensions, netcdf4::Type::Float);
    file.add_variable("int_variable_fixed", {"dimension_1", "dimension_2"}, netcdf4::Type::Int);
    file.add_variable("int_single_value", {}, netcdf4::Type::Int);
    return file;
}

netcdf4::File open_test_file(std::string name) {
    auto file = netcdf4::File::open(name);
    return file;
}

TEST_CASE( "create_and_open_file", "[netcdf]" ) {
    auto file = netcdf4::File::create("test_file.nc");
    file.close();
    file = netcdf4::File::open("test_file.nc");
}

TEST_CASE( "create_and_read_dimensions", "[netcdf]" ) {

    std::string name = "test_create_and_read.nc";
    auto file = create_test_file(name);

    //
    // Request dimensions.
    //

    auto dim = file.get_dimension("dimension_1");
    REQUIRE(dim.size == 10);
    REQUIRE(!dim.is_unlimited());

    dim = file.get_dimension("dimension_2");
    REQUIRE(dim.size == 20);
    REQUIRE(!dim.is_unlimited());

    dim = file.get_dimension("dimension_unlimited");
    REQUIRE(dim.is_unlimited());

    file.close();

    //
    // Read file and request dimensions.
    //
    file = open_test_file(name);

    dim = file.get_dimension("dimension_1");
    REQUIRE(dim.size == 10);
    REQUIRE(!dim.is_unlimited());

    dim = file.get_dimension("dimension_2");
    REQUIRE(dim.size == 20);
    REQUIRE(!dim.is_unlimited());

    dim = file.get_dimension("dimension_unlimited");
    REQUIRE(dim.is_unlimited());
}

TEST_CASE( "create_and_read_variable", "[netcdf]" ) {

    std::string name = "test_create_and_read_variable.nc";
    auto file = create_test_file(name);

    auto int_var = file.get_variable("int_variable");
    auto float_var = file.get_variable("float_variable");
    file.close();

    REQUIRE(int_var.get_dimensions().size() == 3);
    REQUIRE(float_var.get_dimensions().size() == 3);

    REQUIRE(file.has_variable("int_variable"));
    REQUIRE(file.has_variable("float_variable"));
    REQUIRE(!file.has_variable("iint_variable"));
    REQUIRE(!file.has_variable("ffloat_variable"));

    //
    // Read file and request dimensions.
    //
    file = open_test_file(name);

    int_var = file.get_variable("int_variable");
    REQUIRE(int_var.get_dimensions().size() == 3);

    float_var = file.get_variable("float_variable");
    REQUIRE(int_var.get_dimensions().size() == 3);
}

TEST_CASE( "create_and_parse_groups", "[netcdf]" ) {

    std::string name = "test_create_and_parse_groups.nc";
    auto file = create_test_file(name);

    auto group_1 = file.add_group("test_group_1");
    auto group_2 = group_1.add_group("test_group_2");
    REQUIRE(file.has_group("test_group_1"));
    REQUIRE(!file.has_group("test_group_2"));
    REQUIRE(group_1.has_group("test_group_2"));
    file.close();

    file = open_test_file(name);
    auto group_names_1 = file.get_group_names();
    REQUIRE(group_names_1.size() == 1);
    auto group_1_retrieved = file.get_group(group_names_1[0]);
    REQUIRE(group_1_retrieved.get_name() == group_1.get_name());
    REQUIRE(group_1_retrieved.get_group_names() == group_1.get_group_names());
    REQUIRE(file.has_group("test_group_1"));
    REQUIRE(!file.has_group("test_group_2"));
    REQUIRE(group_1.has_group("test_group_2"));

    auto group_names_2 = group_1_retrieved.get_group_names();
    REQUIRE(group_names_2.size() == 1);
    auto group_2_retrieved = group_1.get_group(group_names_2[0]);
    REQUIRE(group_2_retrieved.get_name() == group_2.get_name());
    REQUIRE(group_2_retrieved.get_group_names() == group_2.get_group_names());
}

TEST_CASE( "test_read_write_hyperslab", "[netcdf]" ) {

    std::string name = "test_write_variable.nc";
    auto file = create_test_file(name);

    auto int_var = file.get_variable("int_variable");
    size_t size = 8 * 8 * 10;
    auto data = std::make_unique<int[]>(size);
    for (size_t i = 0; i < size; ++i) {
        data[i] = i;
    }
    auto shape = int_var.shape();
    std::array<size_t, 3> starts = {1, 1, 10};
    std::array<size_t, 3> counts{8, 8, 10};

    int_var.write(starts, counts, data.get());
    file.close();

    file = open_test_file(name);
    int_var = file.get_variable("int_variable");
    auto data_read = std::make_unique<int[]>(size);

    shape = int_var.shape();

    int_var.read(starts, counts, data_read.get());


    for (size_t i = 0; i < size; ++i) {
        REQUIRE(data[i] == data_read[i]);
    }
}

TEST_CASE( "test_read_write_variable", "[netcdf]" ) {

    std::string name = "test_write_variable.nc";
    auto file = create_test_file(name);

    auto int_var = file.get_variable("int_variable_fixed");
    size_t size = 10 * 10 * 20;
    auto data = std::make_unique<int[]>(size);
    for (size_t i = 0; i < size; ++i) {
        data[i] = i;
    }
    int_var.write(data.get());
    file.close();

    file = open_test_file(name);
    int_var = file.get_variable("int_variable_fixed");
    auto data_read = std::make_unique<int[]>(int_var.size());
    int_var.read(data_read.get());

    for (size_t i = 0; i < int_var.size(); ++i) {
        REQUIRE(data[i] == data_read[i]);
    }
}

TEST_CASE( "test_read_write_single_value", "[netcdf]" ) {

    std::string name = "test_singe_value.nc";
    auto file = create_test_file(name);

    auto int_var = file.get_variable("int_single_value");
    int_var.write(99);
    file.close();

    file = open_test_file(name);
    int_var = file.get_variable("int_single_value");
    auto data_read = std::make_unique<int[]>(int_var.size());
    int value = int_var.read<int>();

    REQUIRE(value == 99);
}
