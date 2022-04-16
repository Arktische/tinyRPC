//
// Created by tyx on 2/6/22.
//

#ifndef TINYRPC_CODEC_HPP
#define TINYRPC_CODEC_HPP
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include <iostream>
#include <string_view>
namespace codec {
template <typename messageT>
struct Codec {
  std::string encode(messageT&) {}
  messageT* decode(std::string_view) {}
};

template <>
struct Codec<google::protobuf::Message> {};
}  // namespace codec
#endif  // TINYRPC_CODEC_HPP
