#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <exception>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <common/magic_get.hpp>
#include <common/trait.hpp>
#include <common/tuple_util.hpp>
namespace endian {
class SwapByteBase {
 public:
  // endian test
  static auto ShouldSwap() -> bool {
    static const uint16_t swapTest = 1;
    return (*((char*)&swapTest) == 1);
  }
  // swap v1 v2
  static auto SwapBytes(uint8_t& v1, uint8_t& v2) -> void {
    uint8_t tmp = v1;
    v1 = v2;
    v2 = tmp;
  }
};

template <typename T, int S>
class SwapByte : public SwapByteBase {
 public:
  static auto Swap(T v) -> T {
    // code should never run to here
    assert(false);
    return v;
  }
};

template <typename T>
class SwapByte<T, 1> : public SwapByteBase {
 public:
  static auto Swap(T v) -> T { return v; }
};

template <typename T>
class SwapByte<T, 2> : public SwapByteBase {
 public:
  static auto Swap(T v) -> T {
    if (ShouldSwap()) return (v >> 8) | (v << 8);
    return v;
  }
};

template <typename T>
class SwapByte<T, 4> : public SwapByteBase {
 public:
  static auto Swap(T v) -> T {
    if (ShouldSwap()) {
      return (SwapByte<uint16_t, 2>::Swap((uint32_t)v & 0xffff) << 16) |
             (SwapByte<uint16_t, 2>::Swap(((uint32_t)v & 0xffff0000) >> 16));
    }
    return v;
  }
};

template <typename T>
class SwapByte<T, 8> : public SwapByteBase {
 public:
  static auto Swap(T v) -> T {
    if (ShouldSwap())
      return (((uint64_t)SwapByte<uint32_t, 4>::Swap(
                  (uint32_t)(v & 0xffffffffull)))
              << 32) |
             (SwapByte<uint32_t, 4>::Swap((uint32_t)(v >> 32)));
    return v;
  }
};

template <>
class SwapByte<float, 4> : public SwapByteBase {
 public:
  static auto Swap(float v) -> float {
    union {
      float f;
      uint8_t c[4];
    };
    f = v;
    if (ShouldSwap()) {
      SwapBytes(c[0], c[3]);
      SwapBytes(c[1], c[2]);
    }
    return f;
  }
};

template <>
class SwapByte<double, 8> : public SwapByteBase {
 public:
  static auto Swap(double v) -> double {
    union {
      double f;
      uint8_t c[8];
    };
    f = v;
    if (ShouldSwap()) {
      SwapBytes(c[0], c[7]);
      SwapBytes(c[1], c[6]);
      SwapBytes(c[2], c[5]);
      SwapBytes(c[3], c[4]);
    }
    return f;
  }
};
}  // namespace endian
namespace codec {
template <typename streamT>
class Binary {
  using type = Binary;
  using ref_type = type&;
  using cref_type = const type&;

 public:
  explicit Binary(streamT& stream) : stream_(stream) {}

 public:
  template <typename T>
  auto operator<<(const T& v) const -> cref_type {
    *this& v;
    return *this;
  }

  template <typename T>
  auto operator>>(T& v) -> ref_type {
    *this& v;
    return *this;
  }

 public:
  template <typename T, size_t N>
  auto operator&(T (&v)[N]) -> ref_type {
    uint32_t len;
    *this& len;
    for (size_t i = 0; i < N; ++i) *this& v[i];
    return *this;
  }

  template <typename T, size_t N>
  auto operator&(const T (&v)[N]) const -> cref_type {
    uint32_t len = N;
    *this& len;
    for (size_t i = 0; i < N; ++i) *this& v[i];
    return *this;
  }

#define SERIALIZER_FOR_POD(type)                  \
  auto operator&(type& v)->ref_type {             \
    stream_.read((char*)&v, sizeof(type));        \
    if (!stream_) {                               \
      throw std::runtime_error("malformed data"); \
    }                                             \
    v = Swap(v);                                  \
    return *this;                                 \
  }                                               \
  auto operator&(type v) const->cref_type {       \
    v = Swap(v);                                  \
    stream_.write((const char*)&v, sizeof(type)); \
    return *this;                                 \
  }

  auto operator&(char& v) -> ref_type {
    stream_.read((char*)&v, sizeof(char));
    if (!stream_) {
      throw std::runtime_error("malformed data");
    }
    v = Swap(v);
    return *this;
  }
  auto operator&(char v) const -> cref_type {
    v = Swap(v);
    stream_.write((const char*)&v, sizeof(char));
    return *this;
  }

  SERIALIZER_FOR_POD(bool);
  // SERIALIZER_FOR_POD(char);
  SERIALIZER_FOR_POD(unsigned char);
  SERIALIZER_FOR_POD(short);
  SERIALIZER_FOR_POD(unsigned short);
  SERIALIZER_FOR_POD(int);
  SERIALIZER_FOR_POD(unsigned int);
  SERIALIZER_FOR_POD(long);
  SERIALIZER_FOR_POD(unsigned long);
  SERIALIZER_FOR_POD(long long);
  SERIALIZER_FOR_POD(unsigned long long);
  SERIALIZER_FOR_POD(float);
  SERIALIZER_FOR_POD(double);

#define SERIALIZER_FOR_STL(type)                                               \
  template <typename T>                                                        \
  auto operator&(type<T>& v)->ref_type {                                       \
    uint32_t len;                                                              \
    *this& len;                                                                \
    for (uint32_t i = 0; i < len; ++i) {                                       \
      T value;                                                                 \
      *this& value;                                                            \
      v.insert(v.end(), value);                                                \
    }                                                                          \
    return *this;                                                              \
  }                                                                            \
  template <typename T>                                                        \
  auto operator&(const type<T>& v) const->cref_type {                          \
    uint32_t len = v.size();                                                   \
    *this& len;                                                                \
    for (typename type<T>::const_iterator it = v.begin(); it != v.end(); ++it) \
      *this&* it;                                                              \
    return *this;                                                              \
  }

#define SERIALIZER_FOR_STL2(type)                                             \
  template <typename T1, typename T2>                                         \
  auto operator&(type<T1, T2>& v)->ref_type {                                 \
    uint32_t len;                                                             \
    *this& len;                                                               \
    for (uint32_t i = 0; i < len; ++i) {                                      \
      std::pair<T1, T2> value;                                                \
      *this& value;                                                           \
      v.insert(v.end(), value);                                               \
    }                                                                         \
    return *this;                                                             \
  }                                                                           \
  template <typename T1, typename T2>                                         \
  auto operator&(const type<T1, T2>& v) const->cref_type {                    \
    uint32_t len = v.size();                                                  \
    *this& len;                                                               \
    for (typename type<T1, T2>::const_iterator it = v.begin(); it != v.end(); \
         ++it)                                                                \
      *this&* it;                                                             \
    return *this;                                                             \
  }

  SERIALIZER_FOR_STL(std::vector);
  SERIALIZER_FOR_STL(std::deque);
  SERIALIZER_FOR_STL(std::list);
  SERIALIZER_FOR_STL(std::set);
  SERIALIZER_FOR_STL(std::multiset);
  SERIALIZER_FOR_STL2(std::map);
  SERIALIZER_FOR_STL2(std::multimap);

  template <typename T1, typename T2>
  auto operator&(std::pair<T1, T2>& v) -> ref_type {
    *this& v.first& v.second;
    return *this;
  }

  template <typename T1, typename T2>
  auto operator&(const std::pair<T1, T2>& v) const -> cref_type {
    *this& v.first& v.second;
    return *this;
  }

  auto operator&(std::string& v) -> ref_type {
    uint32_t len;
    *this& len;
    v.clear();
    char buffer[4096];
    uint32_t toRead = len;
    while (toRead != 0) {
      uint32_t l = std::min(toRead, (uint32_t)sizeof(buffer));
      stream_.read(buffer, l);
      if (!stream_) throw std::runtime_error("malformed data");
      v += std::string(buffer, l);
      toRead -= l;
    }
    return *this;
  }

  auto operator&(const std::string& v) const -> cref_type {
    uint32_t len = v.length();
    *this& len;
    stream_.write(v.c_str(), len);
    return *this;
  }

 private:
  template <typename T>
  auto Swap(const T& v) const -> T {
    return endian::SwapByte<T, sizeof(T)>::Swap(v);
  }

 public:
  streamT& stream_;
};
}  // namespace codec
