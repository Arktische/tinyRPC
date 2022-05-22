#pragma once
#include <liburing.h>
#include <async/generator.hpp>
namespace net2 {
struct io {
  io();
  ~io();



  io_uring* ring{};
};
}