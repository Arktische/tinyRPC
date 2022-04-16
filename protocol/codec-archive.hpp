#ifndef TINYRPC_CODEC_ARCHIVE_HPP
#define TINYRPC_CODEC_ARCHIVE_HPP
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

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

namespace endian {
class SwapByteBase {
 public:
  // endian test
  static bool ShouldSwap() {
    static const uint16_t swapTest = 1;
    return (*((char*)&swapTest) == 1);
  }
  // swap v1 v2
  static void SwapBytes(uint8_t& v1, uint8_t& v2) {
    uint8_t tmp = v1;
    v1 = v2;
    v2 = tmp;
  }
};

template <typename T, int S>
class SwapByte : public SwapByteBase {
 public:
  static T Swap(T v) {
    // code should never run to here
    assert(false);
    return v;
  }
};

template <typename T>
class SwapByte<T, 1> : public SwapByteBase {
 public:
  static T Swap(T v) { return v; }
};

template <typename T>
class SwapByte<T, 2> : public SwapByteBase {
 public:
  static T Swap(T v) {
    if (ShouldSwap()) return (v >> 8) | (v << 8);
    return v;
  }
};

template <typename T>
class SwapByte<T, 4> : public SwapByteBase {
 public:
  static T Swap(T v) {
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
  static T Swap(T v) {
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
  static float Swap(float v) {
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
  static double Swap(double v) {
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
template <typename STREAM_TYPE>
class Archive {
 public:
  explicit Archive(STREAM_TYPE& stream) : m_stream(stream) {}

 public:
  template <typename T>
  const Archive& operator<<(const T& v) const {
    *this& v;
    return *this;
  }

  template <typename T>
  Archive& operator>>(T& v) {
    *this& v;
    return *this;
  }

 public:
  template <typename T>
  Archive& operator&(T& v) {
    auto tp = detail::as_tuple<T>();
    std::apply([&](auto&&... args) { ((*this << args), ...); }, tp);
    return *this;
  }

  template <typename T>
  const Archive& operator&(const T& v) const {
    ((T&)v).Serialize(*this);
    return *this;
  }

  template <typename T, size_t N>
  Archive& operator&(T (&v)[N]) {
    uint32_t len;
    *this& len;
    for (size_t i = 0; i < N; ++i) *this& v[i];
    return *this;
  }

  template <typename T, size_t N>
  const Archive& operator&(const T (&v)[N]) const {
    uint32_t len = N;
    *this& len;
    for (size_t i = 0; i < N; ++i) *this& v[i];
    return *this;
  }

#define SERIALIZER_FOR_POD(type)                   \
  Archive& operator&(type& v) {                    \
    m_stream.read((char*)&v, sizeof(type));        \
    if (!m_stream) {                               \
      throw std::runtime_error("malformed data");  \
    }                                              \
    v = Swap(v);                                   \
    return *this;                                  \
  }                                                \
  const Archive& operator&(type v) const {         \
    v = Swap(v);                                   \
    m_stream.write((const char*)&v, sizeof(type)); \
    return *this;                                  \
  }

  SERIALIZER_FOR_POD(bool);
  SERIALIZER_FOR_POD(char);
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
  Archive& operator&(type<T>& v) {                                             \
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
  const Archive& operator&(const type<T>& v) const {                           \
    uint32_t len = v.size();                                                   \
    *this& len;                                                                \
    for (typename type<T>::const_iterator it = v.begin(); it != v.end(); ++it) \
      *this&* it;                                                              \
    return *this;                                                              \
  }

#define SERIALIZER_FOR_STL2(type)                                             \
  template <typename T1, typename T2>                                         \
  Archive& operator&(type<T1, T2>& v) {                                       \
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
  const Archive& operator&(const type<T1, T2>& v) const {                     \
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
  Archive& operator&(std::pair<T1, T2>& v) {
    *this& v.first& v.second;
    return *this;
  }

  template <typename T1, typename T2>
  const Archive& operator&(const std::pair<T1, T2>& v) const {
    *this& v.first& v.second;
    return *this;
  }

  Archive& operator&(std::string& v) {
    uint32_t len;
    *this& len;
    v.clear();
    char buffer[4096];
    uint32_t toRead = len;
    while (toRead != 0) {
      uint32_t l = std::min(toRead, (uint32_t)sizeof(buffer));
      m_stream.read(buffer, l);
      if (!m_stream) throw std::runtime_error("malformed data");
      v += std::string(buffer, l);
      toRead -= l;
    }
    return *this;
  }

  const Archive& operator&(const std::string& v) const {
    uint32_t len = v.length();
    *this& len;
    m_stream.write(v.c_str(), len);
    return *this;
  }

 private:
  template <typename T>
  T Swap(const T& v) const {
    return endian::SwapByte<T, sizeof(T)>::Swap(v);
  }

 public:
  STREAM_TYPE& m_stream;
};
}  // namespace codec
#endif  // TINYRPC_CODEC_ARCHIVE_HPP
