//
// Created by tyx on 2022/1/30.
// Inspired by Milo Yip's implementation
//
#pragma once
#ifndef TINYRPC_STRING_HPP
#define TINYRPC_STRING_HPP
#include <cassert>
#include <cmath>
#include <cstdint>
#include <string>
namespace common {
/*
 * fast itoa & ptr to hex conversion, 3x faster on 64bit integer conversion
 * nearly 9x faster on 32bit integer conversion than snprintf.
 */
template <typename T>
struct make_unsigned_mul_integer {
  typedef std::enable_if_t<std::is_unsigned_v<T>, uint64_t> type;
};

#if ULONG_MAX == ULONG_LONG_MAX
template <>
struct make_unsigned_mul_integer<unsigned long> {
  typedef __uint128_t type;
};
#else
template <>
struct make_unsigned_mul_integer<unsigned long> {
  typedef __uint64_t type;
};
#endif
template <>
struct make_unsigned_mul_integer<unsigned long long> {
  typedef __uint128_t type;
};

template <typename T>
using make_umi_t = typename make_unsigned_mul_integer<T>::type;
static const char digits[]{"9876543210123456789"};
static const char* zero = digits + 9;
static_assert(sizeof(digits) == 20, "wrong number of digits");

const char digitsHex[]{"0123456789abcdef"};
static_assert(sizeof digitsHex == 17, "wrong number of digitsHex");

// Efficient Integer to String Conversions, by Matthew Wilson.
// refer to muduo, modified to support 64bit-integer
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>, void>>
size_t itoa(char buf[], size_t maxlen, T value) {
  static_assert(std::is_integral<T>::value,
                "instantiated template param integerT requires integral type");
  using uiT = std::make_unsigned_t<T>;
  using umiT = make_umi_t<uiT>;
  static constexpr int shift_offset = sizeof(T) * 8 + 3;
  static constexpr auto base =
      static_cast<uiT>((sizeof(T) <= 4) ? 0xcccccccd : 0xcccccccccccccccd);

  uiT v = value > 0 ? value : -value;
  char* ptr = buf;
  int i;
  for (i = 0; v != 0 && 1 < maxlen - i; i++) {
    umiT q = (umiT)((umiT)v * base) >> shift_offset;
    *ptr++ = zero[static_cast<int>(v - q * 10)];
    v = q;
  }
  if (value < 0 && maxlen - i > 2) {
    *ptr++ = '-';
  }
  *ptr = '\0';
  std::reverse(buf, ptr);
  return ptr - buf;
}

template <typename T, typename = std::enable_if_t<std::is_pointer_v<T>, void>>
size_t ptrtoa(char buf[], size_t maxlen, T value) {
  auto v = reinterpret_cast<uintptr_t>(value);
  char* p = buf;
  int i;
  for (i = 0; v != 0 && 1 < maxlen - i; ++i) {
    auto q = v >> 4;
    *p++ = digitsHex[static_cast<int>(v - (q << 4))];
    v = q;
  }
  *p = '\0';
  std::reverse(buf, p);
  return p - buf;
}

/*
 * dtoa grisu2 algorithm implemention, refer to Milo Yip
 * almost 3x faster than snprintf.
 */
template <typename T1, typename T2>
static constexpr uint64_t make_uint64(T1 h, T2 l) {
  return ((static_cast<uint64_t>(h) << 32) | static_cast<uint64_t>(l));
}

struct DiyFp {
  DiyFp() = default;

  DiyFp(uint64_t f, int e) : f(f), e(e) {}

  explicit DiyFp(double d) {
    union {
      double d;
      uint64_t u64;
    } u = {d};

    int biased_e = (u.u64 & kDpExponentMask) >> kDpSignificandSize;
    uint64_t significand = (u.u64 & kDpSignificandMask);
    if (biased_e != 0) {
      f = significand + kDpHiddenBit;
      e = biased_e - kDpExponentBias;
    } else {
      f = significand;
      e = kDpMinExponent + 1;
    }
  }

  DiyFp operator-(const DiyFp& rhs) const {
    assert(e == rhs.e);
    assert(f >= rhs.f);
    return {f - rhs.f, e};
  }

  DiyFp operator*(const DiyFp& rhs) const {
    const uint64_t M32 = 0xFFFFFFFF;
    const uint64_t a = f >> 32;
    const uint64_t b = f & M32;
    const uint64_t c = rhs.f >> 32;
    const uint64_t d = rhs.f & M32;
    const uint64_t ac = a * c;
    const uint64_t bc = b * c;
    const uint64_t ad = a * d;
    const uint64_t bd = b * d;
    uint64_t tmp = (bd >> 32) + (ad & M32) + (bc & M32);
    tmp += 1U << 31;  /// mult_round
    return {ac + (ad >> 32) + (bc >> 32) + (tmp >> 32), e + rhs.e + 64};
  }

  [[nodiscard]] DiyFp Normalize() const {
    int s = __builtin_clzll(f);
    return {f << s, e - s};
  }

  [[nodiscard]] DiyFp NormalizeBoundary() const {
    DiyFp res = *this;
    while (!(res.f & (kDpHiddenBit << 1))) {
      res.f <<= 1;
      res.e--;
    }
    res.f <<= (kDiySignificandSize - kDpSignificandSize - 2);
    res.e = res.e - (kDiySignificandSize - kDpSignificandSize - 2);
    return res;
  }

  void NormalizedBoundaries(DiyFp* minus, DiyFp* plus) const {
    DiyFp pl = DiyFp((f << 1) + 1, e - 1).NormalizeBoundary();
    DiyFp mi = (f == kDpHiddenBit) ? DiyFp((f << 2) - 1, e - 2)
                                   : DiyFp((f << 1) - 1, e - 1);
    mi.f <<= mi.e - pl.e;
    mi.e = pl.e;
    *plus = pl;
    *minus = mi;
  }

  static const int kDiySignificandSize = 64;
  static const int kDpSignificandSize = 52;
  static const int kDpExponentBias = 0x3FF + kDpSignificandSize;
  static const int kDpMinExponent = -kDpExponentBias;
  static const uint64_t kDpExponentMask = make_uint64(0x7FF00000, 0x00000000);
  static const uint64_t kDpSignificandMask =
      make_uint64(0x000FFFFF, 0xFFFFFFFF);
  static const uint64_t kDpHiddenBit = make_uint64(0x00100000, 0x00000000);

  uint64_t f{};
  int e{};
};

inline DiyFp GetCachedPower(int e, int* K) {
  // 10^-348, 10^-340, ..., 10^340
  static const uint64_t kCachedPowers_F[] = {
      make_uint64(0xfa8fd5a0, 0x081c0288), make_uint64(0xbaaee17f, 0xa23ebf76),
      make_uint64(0x8b16fb20, 0x3055ac76), make_uint64(0xcf42894a, 0x5dce35ea),
      make_uint64(0x9a6bb0aa, 0x55653b2d), make_uint64(0xe61acf03, 0x3d1a45df),
      make_uint64(0xab70fe17, 0xc79ac6ca), make_uint64(0xff77b1fc, 0xbebcdc4f),
      make_uint64(0xbe5691ef, 0x416bd60c), make_uint64(0x8dd01fad, 0x907ffc3c),
      make_uint64(0xd3515c28, 0x31559a83), make_uint64(0x9d71ac8f, 0xada6c9b5),
      make_uint64(0xea9c2277, 0x23ee8bcb), make_uint64(0xaecc4991, 0x4078536d),
      make_uint64(0x823c1279, 0x5db6ce57), make_uint64(0xc2109436, 0x4dfb5637),
      make_uint64(0x9096ea6f, 0x3848984f), make_uint64(0xd77485cb, 0x25823ac7),
      make_uint64(0xa086cfcd, 0x97bf97f4), make_uint64(0xef340a98, 0x172aace5),
      make_uint64(0xb23867fb, 0x2a35b28e), make_uint64(0x84c8d4df, 0xd2c63f3b),
      make_uint64(0xc5dd4427, 0x1ad3cdba), make_uint64(0x936b9fce, 0xbb25c996),
      make_uint64(0xdbac6c24, 0x7d62a584), make_uint64(0xa3ab6658, 0x0d5fdaf6),
      make_uint64(0xf3e2f893, 0xdec3f126), make_uint64(0xb5b5ada8, 0xaaff80b8),
      make_uint64(0x87625f05, 0x6c7c4a8b), make_uint64(0xc9bcff60, 0x34c13053),
      make_uint64(0x964e858c, 0x91ba2655), make_uint64(0xdff97724, 0x70297ebd),
      make_uint64(0xa6dfbd9f, 0xb8e5b88f), make_uint64(0xf8a95fcf, 0x88747d94),
      make_uint64(0xb9447093, 0x8fa89bcf), make_uint64(0x8a08f0f8, 0xbf0f156b),
      make_uint64(0xcdb02555, 0x653131b6), make_uint64(0x993fe2c6, 0xd07b7fac),
      make_uint64(0xe45c10c4, 0x2a2b3b06), make_uint64(0xaa242499, 0x697392d3),
      make_uint64(0xfd87b5f2, 0x8300ca0e), make_uint64(0xbce50864, 0x92111aeb),
      make_uint64(0x8cbccc09, 0x6f5088cc), make_uint64(0xd1b71758, 0xe219652c),
      make_uint64(0x9c400000, 0x00000000), make_uint64(0xe8d4a510, 0x00000000),
      make_uint64(0xad78ebc5, 0xac620000), make_uint64(0x813f3978, 0xf8940984),
      make_uint64(0xc097ce7b, 0xc90715b3), make_uint64(0x8f7e32ce, 0x7bea5c70),
      make_uint64(0xd5d238a4, 0xabe98068), make_uint64(0x9f4f2726, 0x179a2245),
      make_uint64(0xed63a231, 0xd4c4fb27), make_uint64(0xb0de6538, 0x8cc8ada8),
      make_uint64(0x83c7088e, 0x1aab65db), make_uint64(0xc45d1df9, 0x42711d9a),
      make_uint64(0x924d692c, 0xa61be758), make_uint64(0xda01ee64, 0x1a708dea),
      make_uint64(0xa26da399, 0x9aef774a), make_uint64(0xf209787b, 0xb47d6b85),
      make_uint64(0xb454e4a1, 0x79dd1877), make_uint64(0x865b8692, 0x5b9bc5c2),
      make_uint64(0xc83553c5, 0xc8965d3d), make_uint64(0x952ab45c, 0xfa97a0b3),
      make_uint64(0xde469fbd, 0x99a05fe3), make_uint64(0xa59bc234, 0xdb398c25),
      make_uint64(0xf6c69a72, 0xa3989f5c), make_uint64(0xb7dcbf53, 0x54e9bece),
      make_uint64(0x88fcf317, 0xf22241e2), make_uint64(0xcc20ce9b, 0xd35c78a5),
      make_uint64(0x98165af3, 0x7b2153df), make_uint64(0xe2a0b5dc, 0x971f303a),
      make_uint64(0xa8d9d153, 0x5ce3b396), make_uint64(0xfb9b7cd9, 0xa4a7443c),
      make_uint64(0xbb764c4c, 0xa7a44410), make_uint64(0x8bab8eef, 0xb6409c1a),
      make_uint64(0xd01fef10, 0xa657842c), make_uint64(0x9b10a4e5, 0xe9913129),
      make_uint64(0xe7109bfb, 0xa19c0c9d), make_uint64(0xac2820d9, 0x623bf429),
      make_uint64(0x80444b5e, 0x7aa7cf85), make_uint64(0xbf21e440, 0x03acdd2d),
      make_uint64(0x8e679c2f, 0x5e44ff8f), make_uint64(0xd433179d, 0x9c8cb841),
      make_uint64(0x9e19db92, 0xb4e31ba9), make_uint64(0xeb96bf6e, 0xbadf77d9),
      make_uint64(0xaf87023b, 0x9bf0ee6b)};
  static const int16_t kCachedPowers_E[] = {
      -1220, -1193, -1166, -1140, -1113, -1087, -1060, -1034, -1007, -980, -954,
      -927,  -901,  -874,  -847,  -821,  -794,  -768,  -741,  -715,  -688, -661,
      -635,  -608,  -582,  -555,  -529,  -502,  -475,  -449,  -422,  -396, -369,
      -343,  -316,  -289,  -263,  -236,  -210,  -183,  -157,  -130,  -103, -77,
      -50,   -24,   3,     30,    56,    83,    109,   136,   162,   189,  216,
      242,   269,   295,   322,   348,   375,   402,   428,   455,   481,  508,
      534,   561,   588,   614,   641,   667,   694,   720,   747,   774,  800,
      827,   853,   880,   907,   933,   960,   986,   1013,  1039,  1066};

  // int k = static_cast<int>(ceil((-61 - e) * 0.30102999566398114)) + 374;
  double dk = (-61 - e) * 0.30102999566398114 +
              347;  // dk must be positive, so can do ceiling in positive
  int k = static_cast<int>(dk);
  if (dk - k > 0.0) k++;

  auto index = static_cast<unsigned>((k >> 3) + 1);
  *K = -(-348 + static_cast<int>(
                    index << 3));  // decimal exponent no need lookup table

  assert(index < sizeof(kCachedPowers_F) / sizeof(kCachedPowers_F[0]));
  return {kCachedPowers_F[index], kCachedPowers_E[index]};
}

inline void GrisuRound(char* buffer, int len, uint64_t delta, uint64_t rest,
                       uint64_t ten_kappa, uint64_t wp_w) {
  while (rest < wp_w && delta - rest >= ten_kappa &&
         (rest + ten_kappa < wp_w ||  /// closer
          wp_w - rest > rest + ten_kappa - wp_w)) {
    buffer[len - 1]--;
    rest += ten_kappa;
  }
}

inline unsigned CountDecimalDigit32(uint32_t n) {
  // Simple pure C++ implementation was faster than __builtin_clz version in
  // this situation.
  if (n < 10) return 1;
  if (n < 100) return 2;
  if (n < 1000) return 3;
  if (n < 10000) return 4;
  if (n < 100000) return 5;
  if (n < 1000000) return 6;
  if (n < 10000000) return 7;
  if (n < 100000000) return 8;
  if (n < 1000000000) return 9;
  return 10;
}

inline void DigitGen(const DiyFp& W, const DiyFp& Mp, uint64_t delta,
                     char* buffer, int* len, int* K) {
  static const uint32_t kPow10[] = {1,         10,        100,     1000,
                                    10000,     100000,    1000000, 10000000,
                                    100000000, 1000000000};
  const DiyFp one(uint64_t(1) << -Mp.e, Mp.e);
  const DiyFp wp_w = Mp - W;
  auto p1 = static_cast<uint32_t>(Mp.f >> -one.e);
  uint64_t p2 = Mp.f & (one.f - 1);
  int kappa = static_cast<int>(CountDecimalDigit32(p1));
  *len = 0;

  while (kappa > 0) {
    uint32_t d;
    switch (kappa) {
      case 10:
        d = p1 / 1000000000;
        p1 %= 1000000000;
        break;
      case 9:
        d = p1 / 100000000;
        p1 %= 100000000;
        break;
      case 8:
        d = p1 / 10000000;
        p1 %= 10000000;
        break;
      case 7:
        d = p1 / 1000000;
        p1 %= 1000000;
        break;
      case 6:
        d = p1 / 100000;
        p1 %= 100000;
        break;
      case 5:
        d = p1 / 10000;
        p1 %= 10000;
        break;
      case 4:
        d = p1 / 1000;
        p1 %= 1000;
        break;
      case 3:
        d = p1 / 100;
        p1 %= 100;
        break;
      case 2:
        d = p1 / 10;
        p1 %= 10;
        break;
      case 1:
        d = p1;
        p1 = 0;
        break;
      default:
        d = 0;
    }
    if (d || *len) buffer[(*len)++] = '0' + static_cast<char>(d);
    kappa--;
    uint64_t tmp = (static_cast<uint64_t>(p1) << -one.e) + p2;
    if (tmp <= delta) {
      *K += kappa;
      GrisuRound(buffer, *len, delta, tmp,
                 static_cast<uint64_t>(kPow10[kappa]) << -one.e, wp_w.f);
      return;
    }
  }

  // kappa = 0
  for (;;) {
    p2 *= 10;
    delta *= 10;
    char d = static_cast<char>(p2 >> -one.e);
    if (d || *len) buffer[(*len)++] = '0' + d;
    p2 &= one.f - 1;
    kappa--;
    if (p2 < delta) {
      *K += kappa;
      GrisuRound(buffer, *len, delta, p2, one.f, wp_w.f * kPow10[-kappa]);
      return;
    }
  }
}

inline void Grisu2(double value, char* buffer, int* length, int* K) {
  const DiyFp v(value);
  DiyFp w_m, w_p;
  v.NormalizedBoundaries(&w_m, &w_p);

  const DiyFp c_mk = GetCachedPower(w_p.e, K);
  const DiyFp W = v.Normalize() * c_mk;
  DiyFp Wp = w_p * c_mk;
  DiyFp Wm = w_m * c_mk;
  Wm.f++;
  Wp.f--;
  DigitGen(W, Wp, Wp.f - Wm.f, buffer, length, K);
}

inline const char* GetDigitsLut() {
  static const char cDigitsLut[200] = {
      '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0',
      '7', '0', '8', '0', '9', '1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
      '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2',
      '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
      '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3',
      '7', '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
      '4', '5', '4', '6', '4', '7', '4', '8', '4', '9', '5', '0', '5', '1', '5',
      '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
      '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6',
      '7', '6', '8', '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
      '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8', '0', '8', '1', '8',
      '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
      '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9',
      '7', '9', '8', '9', '9'};
  return cDigitsLut;
}

inline void WriteExponent(int K, char* buffer) {
  if (K < 0) {
    *buffer++ = '-';
    K = -K;
  }

  if (K >= 100) {
    *buffer++ = '0' + static_cast<char>(K / 100);
    K %= 100;
    const char* d = GetDigitsLut() + K * 2;
    *buffer++ = d[0];
    *buffer++ = d[1];
  } else if (K >= 10) {
    const char* d = GetDigitsLut() + K * 2;
    *buffer++ = d[0];
    *buffer++ = d[1];
  } else
    *buffer++ = '0' + static_cast<char>(K);

  *buffer = '\0';
}

inline void Prettify(char* buffer, int length, int k) {
  const int kk = length + k;  // 10^(kk-1) <= v < 10^kk

  if (length <= kk && kk <= 21) {
    // 1234e7 -> 12340000000
    for (int i = length; i < kk; i++) buffer[i] = '0';
    buffer[kk] = '.';
    buffer[kk + 1] = '0';
    buffer[kk + 2] = '\0';
  } else if (0 < kk && kk <= 21) {
    // 1234e-2 -> 12.34
    memmove(&buffer[kk + 1], &buffer[kk], length - kk);
    buffer[kk] = '.';
    buffer[length + 1] = '\0';
  } else if (-6 < kk && kk <= 0) {
    // 1234e-6 -> 0.001234
    const int offset = 2 - kk;
    memmove(&buffer[offset], &buffer[0], length);
    buffer[0] = '0';
    buffer[1] = '.';
    for (int i = 2; i < offset; i++) buffer[i] = '0';
    buffer[length + offset] = '\0';
  } else if (length == 1) {
    // 1e30
    buffer[1] = 'e';
    WriteExponent(kk - 1, &buffer[2]);
  } else {
    // 1234e30 -> 1.234e33
    memmove(&buffer[2], &buffer[1], length - 1);
    buffer[1] = '.';
    buffer[length + 1] = 'e';
    WriteExponent(kk - 1, &buffer[0 + length + 2]);
  }
}

inline unsigned long dtoa_grisu2(char* buffer, size_t maxlen, double value) {
  // Not handling NaN and inf
  assert(!std::isnan(value));
  assert(!std::isinf(value));

  if (value == 0) {
    buffer[0] = '0';
    buffer[1] = '.';
    buffer[2] = '0';
    buffer[3] = '\0';
    return 4;
  }
  int strlen = 0;
  if (value < 0) {
    *buffer++ = '-';
    value = -value;
    strlen++;
  }
  int length, K;
  Grisu2(value, buffer, &length, &K);
  Prettify(buffer, length, K);
  return strlen + length;
}
}  // namespace common
#endif  // TINYRPC_STRING_HPP
