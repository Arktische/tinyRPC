//
// Created by tyx on 3/26/22.
//

#ifndef TINYRPC_ERROR_HPP
#define TINYRPC_ERROR_HPP
#include <system_error>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/epoll.h>
using std::error_code;
using std::make_error_code;
enum class OSError {
  IO_WOULDBLOCK = EWOULDBLOCK,
  IO_AGAIN = EAGAIN,
};

struct OSErrorCategory :std::error_category{
  const char* name() const noexcept override;
  std::string message(int ev) const override;
};

const char* OSErrorCategory::name() const noexcept
{
  return "Operating System Error";
}

namespace std {
template<>
struct is_error_code_enum<OSError>:true_type {};
}

std::error_code make_error_code(OSError oerr) {
  return {static_cast<int>(oerr),OSErrorCategory()};
}

#endif  // TINYRPC_ERROR_HPP
