#ifndef __NETCDFPP__
#define __NETCDFPP__

#include "netcdf.h"
#include <sstream>
#include <stdexcept>

namespace netcdf4 {
namespace detail {
void handle_error(std::string error_message, int error_code) {
  if (error_code != NC_NOERR) {
    std::stringstream error;
    error << error_message << "\n";
    error << nc_strerror(error_code) << std::endl;
    throw std::runtime_error(error.str());
  }
}

}  // namespace detail

enum class CreationMode { Clobber = NC_CLOBBER, NoClobber = NC_NOCLOBBER };

enum class OpenMode {
  Write = NC_WRITE, Share = NC_SHARE, WriteShare = NC_WRITE | NC_SHARE
      };

class File {
 public:

  static File create(std::string path, CreationMode mode = CreationMode::Clobber) {
    int id = 0;
    int error = nc_create(path.c_str(), static_cast<int>(mode), &id);
    detail::handle_error("Error creating file: " + path, error);
    return File(id);
  }

  static File open(std::string path, OpenMode mode = OpenMode::Write) {
      int id = 0;
      int error = nc_open(path.c_str(), static_cast<int>(mode), &id);
      detail::handle_error("Error opening file: " + path, error);
      return File(id);
  }

File(int id) : id_(id), open_(true) {
      int error = nc_inq(id_,
                         &n_dims_,
                         &n_vars_,
                         &n_attrs_,
                         &n_unl_dims_);
      detail::handle_error("Error constructing file object:",
                          error);
  }

  ~File () {
    nc_close(id_);
  }

  void close() {
    if (open_) {
      int error = nc_close(id_);
      detail::handle_error("Error closing file: ", error);
      open_ = false;
    }
  }

 private:
  int id_, n_dims_, n_vars_, n_groups_, n_attrs_,
      n_unl_dims_;
  bool open_;

};

}
#endif
