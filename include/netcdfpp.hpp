/** An object oriented interface to NetCDF files.
 *
 * This single-include header provides a complete interface
 * to the NetCDF4 c-library.
 *
 * https://github.com/simonpf/netcdfhpp
 *
 * Published under MIT license.
 *
 * Copyright: Simon Pfreundschuh, 2020
 */
#ifndef __NETCDFPP__
#define __NETCDFPP__

#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "netcdf.h"

namespace netcdf4 {
namespace detail {

/** Handle NetCDF error
 *
 * Takes a NetCDF error code returned from a NetCDF library call and
 * throws a std::runtime_error with the error messages if code signals
 * an error.
 *
 * @param error_message Additional error message to display
 *        before the NetCDF error.
 * @param The NetCDF error code returned by the library call.
 */
void handle_error(std::string error_message, int error_code) {
  if (error_code != NC_NOERR) {
    std::stringstream error;
    error << error_message << "\n";
    error << nc_strerror(error_code) << std::endl;
    throw std::runtime_error(error.str());
  }
}

/** NetCDF file ID capsule.
 *
 * This wrapper struct manages the lifetime of a netcdf file.
 */
struct FileID {
  ~FileID() { close(); }

  void close() {
    if (open) {
      int error = nc_close(id);
      detail::handle_error("Error closing file: ", error);
      open = false;
    }
  }

  operator int() { return id; }

  int id = 0;
  bool open = true;
};

void assert_write_mode(int nc_id) {
  int error = nc_enddef(nc_id);
  if (error != NC_ENOTINDEFINE) {
    detail::handle_error("Error leaving define mode: ", error);
  }
}

void assert_define_mode(int nc_id) {
  int error = nc_redef(nc_id);
  if (error != NC_EINDEFINE) {
    detail::handle_error("Error (re)entering define mode: ", error);
  }
}

}  // namespace detail

enum class CreationMode { Clobber = NC_CLOBBER, NoClobber = NC_NOCLOBBER };

enum class OpenMode {
  Write = NC_WRITE,
  Share = NC_SHARE,
  WriteShare = NC_WRITE | NC_SHARE
};

enum class Type {
  NotAType = NC_NAT,
  Byte = NC_BYTE,
  Char = NC_CHAR,
  Short = NC_SHORT,
  Int = NC_INT,
  Long = NC_LONG,
  Float = NC_FLOAT,
  Double = NC_DOUBLE,
  UByte = NC_UBYTE,
  UShort = NC_USHORT,
  UInt = NC_UINT,
  Int64 = NC_INT64,
  UInt64 = NC_UINT64,
  String = NC_STRING,
  MaxAtomic = NC_MAX_ATOMIC_TYPE
};

std::ostream& operator<<(std::ostream& out, const Type& t) {
  std::string name = "";
  switch (t) {
    case Type::NotAType: {
      name = "not_a_type";
      break;
    }
    case Type::Byte: {
      name = "byte";
      break;
    }
    case Type::Char: {
      name = "char";
      break;
    }
    case Type::Short: {
      name = "short";
      break;
    }
    case Type::Int: {
      name = "int";
      break;
    }
    case Type::Float: {
      name = "float";
      break;
    }
    case Type::Double: {
      name = "double";
      break;
    }
    case Type::UByte: {
      name = "unsigned byte";
      break;
    }
    case Type::UShort: {
      name = "unsigned short";
      break;
    }
    case Type::UInt: {
      name = "unsigned int";
      break;
    }
    case Type::Int64: {
      name = "int64";
      break;
    }
    case Type::UInt64: {
      name = "unsigned int64";
      break;
    }
    case Type::String: {
      name = "string";
      break;
    }
  }
  out << name;
  return out;
}

template <typename T>
struct TypeMap;

template <>
struct TypeMap<int> {
  static constexpr Type value = Type::Int;
  static constexpr auto write = &nc_put_var_int;
  static constexpr auto write_array = &nc_put_vara_int;
  static constexpr auto write_value = &nc_put_var1_int;
  static constexpr auto write_strided = &nc_put_vars_int;
  static constexpr auto read = &nc_put_var_int;
  static constexpr auto read_array = &nc_get_vara_int;
  static constexpr auto read_value = &nc_get_var1_int;
  static constexpr auto read_strided = &nc_get_vars_int;
};

template <>
struct TypeMap<float> {
  static constexpr Type value = Type::Float;
  static constexpr auto write = &nc_put_var_float;
  static constexpr auto write_array = &nc_put_vara_float;
  static constexpr auto write_value = &nc_put_var1_float;
  static constexpr auto write_strided = &nc_put_vars_float;
  static constexpr auto read = &nc_put_var_float;
  static constexpr auto read_array = &nc_get_vara_float;
  static constexpr auto read_value = &nc_get_var1_float;
  static constexpr auto read_strided = &nc_get_vars_float;
};

template <>
struct TypeMap<double> {
  static constexpr Type value = Type::Double;
  static constexpr auto write = &nc_put_var_double;
  static constexpr auto write_array = &nc_put_vara_double;
  static constexpr auto write_value = &nc_put_var1_double;
  static constexpr auto write_strided = &nc_put_vars_double;
  static constexpr auto read = &nc_put_var_double;
  static constexpr auto read_array = &nc_get_vara_double;
  static constexpr auto read_value = &nc_get_var1_double;
  static constexpr auto read_strided = &nc_get_vars_double;
};

template <>
struct TypeMap<char> {
  static constexpr Type value = Type::Char;
  static constexpr auto write = &nc_put_var_schar;
  static constexpr auto write_array = &nc_put_vara_schar;
  static constexpr auto write_value = &nc_put_var1_schar;
  static constexpr auto write_strided = &nc_put_vars_schar;
  static constexpr auto read = &nc_put_var_schar;
  static constexpr auto read_array = &nc_get_vara_schar;
  static constexpr auto read_value = &nc_get_var1_schar;
  static constexpr auto read_strided = &nc_get_vars_schar;
};

struct Dimension {
  Dimension() {}

  Dimension(int id_) : id(id_) {}

  Dimension(std::string name_, int size_) {
    name_.copy(name, name_.size());
    name[name_.size()] = 0;
    size = size_;
  }

  bool is_unlimited() { return unlimited; }

  int id = 0;
  size_t size = 0;
  bool unlimited = false;
  char name[NC_MAX_NAME + 1] = {0};
};

class Variable {
 public:
  void parse_dimensions() {
    int n_dims = dimensions_.capacity();
    auto dim_ids = std::make_unique<int[]>(n_dims);
    int error = nc_inq_vardimid(parent_id_, id_, dim_ids.get());
    detail::handle_error("Error inquiring dimension IDs of variable:", error);

    for (int i = 0; i < n_dims; ++i) {
      int dim_id = dim_ids[i];
      Dimension dim{dim_id};
      error = nc_inq_dim(parent_id_, dim_id, dim.name, &dim.size);
      detail::handle_error("Error inquiring dimension:", error);
      dimensions_.push_back(dim);
    }
  }

  Variable() {}

  Variable(std::shared_ptr<detail::FileID> file_ptr, int parent_id, int id)
      : id_(id), parent_id_(parent_id), file_ptr_(file_ptr) {
    int n_dims, n_attrs, type;
    int error = nc_inq_var(parent_id_, id_, name_, &type, &n_dims, 0, &n_attrs);
    detail::handle_error("Error inquiring variable:", error);
    type_ = Type(type);
    dimensions_.reserve(n_dims);
    parse_dimensions();
  }

  template<typename T>
  void check_type() {
    if (TypeMap<T>::value != type_) {
        std::stringstream msg;
        msg << "Provided type " << TypeMap<T>::value << " is incompatible "
            << "with NetCDF type " << type_ << std::endl;
        throw std::runtime_error(msg.str());
    }
  }

  template <typename T, size_t N_DIMS>
  void write(std::array<size_t, N_DIMS> starts,
             std::array<size_t, N_DIMS> counts,
             const T* data) {
    using TypeTraits = TypeMap<T>;
    check_type<T>();
    detail::assert_write_mode(*file_ptr_);
    TypeTraits::write_array(
        parent_id_, id_, starts.data(), counts.data(), data);
  }

    template <typename T, size_t N_DIMS>
    void read(std::array<size_t, N_DIMS> starts,
               std::array<size_t, N_DIMS> counts,
               T* data) {
        using TypeTraits = TypeMap<T>;
        check_type<T>();
        detail::assert_write_mode(*file_ptr_);
        TypeTraits::read_array(
            parent_id_, id_, starts.data(), counts.data(), data);
    }

  const std::vector<Dimension>& get_dimensions() const { return dimensions_; }

  size_t size() {
    size_t result = 1;
    for (auto& d : dimensions_) {
      result *= d.size;
    }
    return result;
  }

  std::vector<size_t> shape() {
    std::vector<size_t> result(dimensions_.size());
    for (size_t i = 0; i < result.size(); ++i) {
      result[i] = dimensions_[i].size;
    }
    return result;
  }

  std::string get_name() const { return name_; }

 private:
  int id_, parent_id_;
  std::vector<Dimension> dimensions_;
  char name_[NC_MAX_NAME + 1] = {0};
  Type type_ = Type::NotAType;
  std::shared_ptr<detail::FileID> file_ptr_ = nullptr;
};

/** A NetCDF group.
  *
  * A NetCDF group contains dimensions, variables, attributes
  * and nested groups.
  *
  */
class Group {
 public:
  void parse_dimensions() {
    auto dim_ids = std::make_unique<int[]>(n_dims_);
    int error = nc_inq_dimids(id_, 0, dim_ids.get(), 0);
    detail::handle_error("Error inquiring dimension IDs:", error);
    for (int i = 0; i < n_dims_; ++i) {
      int dim_id = dim_ids[i];
      Dimension dim{dim_id};
      error = nc_inq_dim(id_, dim_id, dim.name, &dim.size);
      detail::handle_error("Error inquiring dimensions", error);
      dimensions_[dim.name] = dim;
    }

    dim_ids = std::make_unique<int[]>(n_unl_dims_);
    error = nc_inq_unlimdims(id_, 0, dim_ids.get());
    detail::handle_error("Error inquiring unlimited dimension IDs:", error);
    for (int i = 0; i < n_unl_dims_; ++i) {
      int dim_id = dim_ids[i];
      Dimension dim{dim_id};
      dim.unlimited = true;
      dim.size = -1;
      error = nc_inq_dim(id_, dim_id, dim.name, &dim.size);
      detail::handle_error("Error inquiring dimensions", error);
      dimensions_[dim.name] = dim;
    }
  }

  void parse_variables() {
    auto var_ids = std::make_unique<int[]>(n_dims_);
    int error = nc_inq_varids(id_, 0, var_ids.get());
    detail::handle_error("Error inquiring variable IDs:", error);
    for (int i = 0; i < n_vars_; ++i) {
      int var_id = var_ids[i];
      Variable var{file_ptr_, id_, var_id};
      variables_[var.get_name()] = var;
    }
  }

  Group(std::shared_ptr<detail::FileID> file_ptr, int id, std::string name)
      : file_ptr_(file_ptr), id_(id), name_(name) {
    //
    // Inquire number of dimensions, variables, attributes and groups.
    //
    int error = nc_inq_dimids(id_, &n_dims_, 0, 0);
    detail::handle_error("Error inquiring number of dimensions:", error);
    error = nc_inq_unlimdims(id_, &n_unl_dims_, 0);
    detail::handle_error("Error inquiring number of unlimited dimensions:",
                         error);
    error = nc_inq_varids(id_, &n_vars_, 0);
    detail::handle_error("Error inquiring number of variables:", error);
    error = nc_inq_natts(id_, &n_attrs_);
    detail::handle_error("Error inquiring number of attributes:", error);
    error = nc_inq_grps(id_, &n_groups_, 0);
    detail::handle_error("Error inquiring number of groups:", error);

    parse_dimensions();
    parse_variables();
    //parse_attributes();
    //parse_groups();
  }

  void assert_define_mode() { detail::assert_define_mode(id_); }

  void assert_write_mode() { detail::assert_write_mode(id_); }

  void sync() {
    assert_write_mode();
    int error = nc_sync(id_);
    detail::handle_error("Error entering define mode: ", error);
  }

  void add_dimension(std::string name, int size) {
    assert_define_mode();
    Dimension dim{name, size};
    int error = nc_def_dim(id_, name.c_str(), size, &dim.id);
    detail::handle_error("Error creating dimensions: ", error);
    dimensions_[name] = dim;
    sync();
  }

  void add_dimension(std::string name) {
    assert_define_mode();
    Dimension dim{name, -1};
    dim.unlimited = true;
    int error = nc_def_dim(id_, name.c_str(), NC_UNLIMITED, &dim.id);
    detail::handle_error("Error creating dimensions: ", error);
    sync();
    dimensions_[name] = dim;
  }

  Variable add_variable(std::string name,
                        std::vector<std::string> dimensions,
                        Type type) {
    assert_define_mode();
    int n_dims = dimensions.size();
    std::vector<int> dim_ids;
    dim_ids.reserve(n_dims);
    for (int i = 0; i < n_dims; ++i) {
      auto& d = dimensions[i];
      auto search = dimensions_.find(d);
      if (search == dimensions_.end()) {
        std::stringstream msg;
        msg << "Dimension " << d << " is not defined.";
        throw std::runtime_error(msg.str());
      }
      dim_ids[i] = search->second.id;
    }
    int var_id = 0;
    int error = nc_def_var(id_,
                           name.c_str(),
                           static_cast<int>(type),
                           n_dims,
                           dim_ids.data(),
                           &var_id);
    detail::handle_error("Error defining variable:", error);
    sync();
    variables_[name] = Variable(file_ptr_, id_, var_id);
    return variables_[name];
  }

  Dimension get_dimension(std::string name) {
    auto found = dimensions_.find(name);
    if (found != dimensions_.end()) {
      return found->second;
    }
    std::stringstream msg;
    msg << "Dimension " << name << " not found in dimensions.";
    throw std::runtime_error(msg.str());
  }

  Variable get_variable(std::string name) {
    auto found = variables_.find(name);
    if (found != variables_.end()) {
      return found->second;
    }
    std::stringstream msg;
    msg << "Variable " << name << " not found in variables.";
    throw std::runtime_error(msg.str());
  }

 protected:
  std::shared_ptr<detail::FileID> file_ptr_;
  int id_;
  std::string name_;
  int n_dims_, n_vars_, n_groups_, n_attrs_, n_unl_dims_;
  std::map<std::string, Dimension> dimensions_ = {};
  std::map<std::string, Variable> variables_ = {};
};

/** A NetCDF4 file.
 *
 * The File class represents a NetCDF4 file. It provides
 * an object-oriented interface to the NetCDF-c library.
 *
 */
class File : public Group {
 public:
  /** Create new NetCDF4 file.
     *
     * @param path The file's path
     * @param mode Creation mode defining whether or not to over-
     *        write an existing file.
     * @return File instance representing the newly created file.
     */
  static File create(std::string path,
                     CreationMode mode = CreationMode::Clobber) {
    auto file = std::make_shared<detail::FileID>();
    int error = nc_create(path.c_str(), static_cast<int>(mode), &file->id);
    detail::handle_error("Error creating file: " + path, error);
    return File(file);
  }

  /** Open NetCDF4 file.
     *
     * @param path The path to the file to open.
     * @param mode Opening mode defining whether write access
     *        is required.
     * @return File instance representing the opened file.
     */
  static File open(std::string path, OpenMode mode = OpenMode::Write) {
    auto file = std::make_shared<detail::FileID>();
    int error = nc_open(path.c_str(), static_cast<int>(mode), &file->id);
    detail::handle_error("Error opening file: " + path, error);
    return File(file);
  }

  File(std::shared_ptr<detail::FileID> file_ptr)
      : Group(file_ptr, *file_ptr, "") {}

  /// Close the file.
  void close() { file_ptr_->close(); }

 private:
};

}  // namespace netcdf4
#endif
