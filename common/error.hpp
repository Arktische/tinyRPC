#pragma once
#include <sys/epoll.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>

#include <system_error>
typedef int OSError;

struct OSErrorCategory : std::error_category {
  const char* name() const noexcept override;
  std::string message(int ev) const override;
};

inline const char* OSErrorCategory::name() const noexcept {
  return "Operating System Error";
}
std::string OSErrorCategory::message(int ev) const { return strerror(ev); }

namespace std {
template <>
struct is_error_code_enum<OSError> : true_type {};
}  // namespace std

inline std::error_code make_error_code(OSError oerr) {
  return {static_cast<int>(oerr), OSErrorCategory()};
}