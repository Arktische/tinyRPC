//
// Created by tyx on 2022/1/30.
//
#pragma once
#ifndef TINYRPC_FASTMEMCPY_HPP
#define TINYRPC_FASTMEMCPY_HPP
#include <immintrin.h>
namespace common {
template <unsigned N>
struct avx_ldst32ubyte_unroll {
  __inline__ __attribute__((always_inline)) static void _call(void* dst,
                                                              const void* src) {
    __m256i m = _mm256_loadu_si256((const __m256i*)src + N - 1);
    _mm256_storeu_si256((__m256i*)dst + N - 1, m);
    avx_ldst32ubyte_unroll<N - 1>::_call(dst, src);
  }
};

template <>
struct avx_ldst32ubyte_unroll<0u> {
  __inline__ __attribute__((always_inline)) static void _call(void* dst,
                                                              const void* src) {
  }
};

template <unsigned N>
struct avx_ldst16ubyte_unroll {
  __inline__ __attribute__((always_inline)) static void _call(void* dst,
                                                              const void* src) {
    __m128i m0 = _mm_loadu_si128(((const __m128i*)src) + 0);
    _mm_storeu_si128(((__m128i*)dst) + 0, m0);
    avx_ldst16ubyte_unroll<N - 1>::_call(dst, src);
  }
};

template <>
struct avx_ldst16ubyte_unroll<0u> {
  __inline__ __attribute__((always_inline)) static void _call(void* dst,
                                                              const void* src) {
  }
};

inline void memcpy(void* dst, const void* src, size_t size) {
  unsigned char* dd = ((unsigned char*)dst) + size;
  const unsigned char* ss = ((const unsigned char*)src) + size;

  switch (size) {
    case 128:
      avx_ldst32ubyte_unroll<4>::_call(dd - 128, ss - 128);
    case 0:
      break;
    case 129:
      avx_ldst32ubyte_unroll<4>::_call(dd - 129, ss - 129);
    case 1:
      dd[-1] = ss[-1];
      break;
    case 130:
      avx_ldst32ubyte_unroll<4>::_call(dd - 130, ss - 130);
    case 2:
      *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2));
      break;
    case 131:
      avx_ldst32ubyte_unroll<4>::_call(dd - 131, ss - 131);
    case 3:
      *((uint16_t*)(dd - 3)) = *((uint16_t*)(ss - 3));
      dd[-1] = ss[-1];
      break;
    case 132:
      avx_ldst32ubyte_unroll<4>::_call(dd - 132, ss - 132);
    case 4:
      *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
      break;
    case 133:
      avx_ldst32ubyte_unroll<4>::_call(dd - 133, ss - 133);
    case 5:
      *((uint32_t*)(dd - 5)) = *((uint32_t*)(ss - 5));
      dd[-1] = ss[-1];
      break;
    case 134:
      avx_ldst32ubyte_unroll<4>::_call(dd - 134, ss - 134);
    case 6:
      *((uint32_t*)(dd - 6)) = *((uint32_t*)(ss - 6));
      *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2));
      break;
    case 135:
      avx_ldst32ubyte_unroll<4>::_call(dd - 135, ss - 135);
    case 7:
      *((uint32_t*)(dd - 7)) = *((uint32_t*)(ss - 7));
      *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
      break;
    case 136:
      avx_ldst32ubyte_unroll<4>::_call(dd - 136, ss - 136);
    case 8:
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 137:
      avx_ldst32ubyte_unroll<4>::_call(dd - 137, ss - 137);
    case 9:
      *((uint64_t*)(dd - 9)) = *((uint64_t*)(ss - 9));
      dd[-1] = ss[-1];
      break;
    case 138:
      avx_ldst32ubyte_unroll<4>::_call(dd - 138, ss - 138);
    case 10:
      *((uint64_t*)(dd - 10)) = *((uint64_t*)(ss - 10));
      *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2));
      break;
    case 139:
      avx_ldst32ubyte_unroll<4>::_call(dd - 139, ss - 139);
    case 11:
      *((uint64_t*)(dd - 11)) = *((uint64_t*)(ss - 11));
      *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
      break;
    case 140:
      avx_ldst32ubyte_unroll<4>::_call(dd - 140, ss - 140);
    case 12:
      *((uint64_t*)(dd - 12)) = *((uint64_t*)(ss - 12));
      *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
      break;
    case 141:
      avx_ldst32ubyte_unroll<4>::_call(dd - 141, ss - 141);
    case 13:
      *((uint64_t*)(dd - 13)) = *((uint64_t*)(ss - 13));
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 142:
      avx_ldst32ubyte_unroll<4>::_call(dd - 142, ss - 142);
    case 14:
      *((uint64_t*)(dd - 14)) = *((uint64_t*)(ss - 14));
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 143:
      avx_ldst32ubyte_unroll<4>::_call(dd - 143, ss - 143);
    case 15:
      *((uint64_t*)(dd - 15)) = *((uint64_t*)(ss - 15));
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 144:
      avx_ldst32ubyte_unroll<4>::_call(dd - 144, ss - 144);
    case 16:
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 145:
      avx_ldst32ubyte_unroll<4>::_call(dd - 145, ss - 145);
    case 17:
      avx_ldst16ubyte_unroll<1>::_call(dd - 17, ss - 17);
      dd[-1] = ss[-1];
      break;
    case 146:
      avx_ldst32ubyte_unroll<4>::_call(dd - 146, ss - 146);
    case 18:
      avx_ldst16ubyte_unroll<1>::_call(dd - 18, ss - 18);
      *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2));
      break;
    case 147:
      avx_ldst32ubyte_unroll<4>::_call(dd - 147, ss - 147);
    case 19:
      avx_ldst16ubyte_unroll<1>::_call(dd - 19, ss - 19);
      *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
      break;
    case 148:
      avx_ldst32ubyte_unroll<4>::_call(dd - 148, ss - 148);
    case 20:
      avx_ldst16ubyte_unroll<1>::_call(dd - 20, ss - 20);
      *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
      break;
    case 149:
      avx_ldst32ubyte_unroll<4>::_call(dd - 149, ss - 149);
    case 21:
      avx_ldst16ubyte_unroll<1>::_call(dd - 21, ss - 21);
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 150:
      avx_ldst32ubyte_unroll<4>::_call(dd - 150, ss - 150);
    case 22:
      avx_ldst16ubyte_unroll<1>::_call(dd - 22, ss - 22);
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 151:
      avx_ldst32ubyte_unroll<4>::_call(dd - 151, ss - 151);
    case 23:
      avx_ldst16ubyte_unroll<1>::_call(dd - 23, ss - 23);
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 152:
      avx_ldst32ubyte_unroll<4>::_call(dd - 152, ss - 152);
    case 24:
      avx_ldst16ubyte_unroll<1>::_call(dd - 24, ss - 24);
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 153:
      avx_ldst32ubyte_unroll<4>::_call(dd - 153, ss - 153);
    case 25:
      avx_ldst16ubyte_unroll<1>::_call(dd - 25, ss - 25);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 154:
      avx_ldst32ubyte_unroll<4>::_call(dd - 154, ss - 154);
    case 26:
      avx_ldst16ubyte_unroll<1>::_call(dd - 26, ss - 26);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 155:
      avx_ldst32ubyte_unroll<4>::_call(dd - 155, ss - 155);
    case 27:
      avx_ldst16ubyte_unroll<1>::_call(dd - 27, ss - 27);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 156:
      avx_ldst32ubyte_unroll<4>::_call(dd - 156, ss - 156);
    case 28:
      avx_ldst16ubyte_unroll<1>::_call(dd - 28, ss - 28);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 157:
      avx_ldst32ubyte_unroll<4>::_call(dd - 157, ss - 157);
    case 29:
      avx_ldst16ubyte_unroll<1>::_call(dd - 29, ss - 29);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 158:
      avx_ldst32ubyte_unroll<4>::_call(dd - 158, ss - 158);
    case 30:
      avx_ldst16ubyte_unroll<1>::_call(dd - 30, ss - 30);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 159:
      avx_ldst32ubyte_unroll<4>::_call(dd - 159, ss - 159);
    case 31:
      avx_ldst16ubyte_unroll<1>::_call(dd - 31, ss - 31);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 160:
      avx_ldst32ubyte_unroll<4>::_call(dd - 160, ss - 160);
    case 32:
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 161:
      avx_ldst32ubyte_unroll<4>::_call(dd - 161, ss - 161);
    case 33:
      avx_ldst32ubyte_unroll<1>::_call(dd - 33, ss - 33);
      dd[-1] = ss[-1];
      break;
    case 162:
      avx_ldst32ubyte_unroll<4>::_call(dd - 162, ss - 162);
    case 34:
      avx_ldst32ubyte_unroll<1>::_call(dd - 34, ss - 34);
      *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2));
      break;
    case 163:
      avx_ldst32ubyte_unroll<4>::_call(dd - 163, ss - 163);
    case 35:
      avx_ldst32ubyte_unroll<1>::_call(dd - 35, ss - 35);
      *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
      break;
    case 164:
      avx_ldst32ubyte_unroll<4>::_call(dd - 164, ss - 164);
    case 36:
      avx_ldst32ubyte_unroll<1>::_call(dd - 36, ss - 36);
      *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
      break;
    case 165:
      avx_ldst32ubyte_unroll<4>::_call(dd - 165, ss - 165);
    case 37:
      avx_ldst32ubyte_unroll<1>::_call(dd - 37, ss - 37);
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 166:
      avx_ldst32ubyte_unroll<4>::_call(dd - 166, ss - 166);
    case 38:
      avx_ldst32ubyte_unroll<1>::_call(dd - 38, ss - 38);
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 167:
      avx_ldst32ubyte_unroll<4>::_call(dd - 167, ss - 167);
    case 39:
      avx_ldst32ubyte_unroll<1>::_call(dd - 39, ss - 39);
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 168:
      avx_ldst32ubyte_unroll<4>::_call(dd - 168, ss - 168);
    case 40:
      avx_ldst32ubyte_unroll<1>::_call(dd - 40, ss - 40);
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 169:
      avx_ldst32ubyte_unroll<4>::_call(dd - 169, ss - 169);
    case 41:
      avx_ldst32ubyte_unroll<1>::_call(dd - 41, ss - 41);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 170:
      avx_ldst32ubyte_unroll<4>::_call(dd - 170, ss - 170);
    case 42:
      avx_ldst32ubyte_unroll<1>::_call(dd - 42, ss - 42);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 171:
      avx_ldst32ubyte_unroll<4>::_call(dd - 171, ss - 171);
    case 43:
      avx_ldst32ubyte_unroll<1>::_call(dd - 43, ss - 43);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 172:
      avx_ldst32ubyte_unroll<4>::_call(dd - 172, ss - 172);
    case 44:
      avx_ldst32ubyte_unroll<1>::_call(dd - 44, ss - 44);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 173:
      avx_ldst32ubyte_unroll<4>::_call(dd - 173, ss - 173);
    case 45:
      avx_ldst32ubyte_unroll<1>::_call(dd - 45, ss - 45);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 174:
      avx_ldst32ubyte_unroll<4>::_call(dd - 174, ss - 174);
    case 46:
      avx_ldst32ubyte_unroll<1>::_call(dd - 46, ss - 46);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 175:
      avx_ldst32ubyte_unroll<4>::_call(dd - 175, ss - 175);
    case 47:
      avx_ldst32ubyte_unroll<1>::_call(dd - 47, ss - 47);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 176:
      avx_ldst32ubyte_unroll<4>::_call(dd - 176, ss - 176);
    case 48:
      avx_ldst32ubyte_unroll<1>::_call(dd - 48, ss - 48);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 177:
      avx_ldst32ubyte_unroll<4>::_call(dd - 177, ss - 177);
    case 49:
      avx_ldst32ubyte_unroll<1>::_call(dd - 49, ss - 49);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 178:
      avx_ldst32ubyte_unroll<4>::_call(dd - 178, ss - 178);
    case 50:
      avx_ldst32ubyte_unroll<1>::_call(dd - 50, ss - 50);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 179:
      avx_ldst32ubyte_unroll<4>::_call(dd - 179, ss - 179);
    case 51:
      avx_ldst32ubyte_unroll<1>::_call(dd - 51, ss - 51);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 180:
      avx_ldst32ubyte_unroll<4>::_call(dd - 180, ss - 180);
    case 52:
      avx_ldst32ubyte_unroll<1>::_call(dd - 52, ss - 52);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 181:
      avx_ldst32ubyte_unroll<4>::_call(dd - 181, ss - 181);
    case 53:
      avx_ldst32ubyte_unroll<1>::_call(dd - 53, ss - 53);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 182:
      avx_ldst32ubyte_unroll<4>::_call(dd - 182, ss - 182);
    case 54:
      avx_ldst32ubyte_unroll<1>::_call(dd - 54, ss - 54);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 183:
      avx_ldst32ubyte_unroll<4>::_call(dd - 183, ss - 183);
    case 55:
      avx_ldst32ubyte_unroll<1>::_call(dd - 55, ss - 55);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 184:
      avx_ldst32ubyte_unroll<4>::_call(dd - 184, ss - 184);
    case 56:
      avx_ldst32ubyte_unroll<1>::_call(dd - 56, ss - 56);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 185:
      avx_ldst32ubyte_unroll<4>::_call(dd - 185, ss - 185);
    case 57:
      avx_ldst32ubyte_unroll<1>::_call(dd - 57, ss - 57);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 186:
      avx_ldst32ubyte_unroll<4>::_call(dd - 186, ss - 186);
    case 58:
      avx_ldst32ubyte_unroll<1>::_call(dd - 58, ss - 58);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 187:
      avx_ldst32ubyte_unroll<4>::_call(dd - 187, ss - 187);
    case 59:
      avx_ldst32ubyte_unroll<1>::_call(dd - 59, ss - 59);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 188:
      avx_ldst32ubyte_unroll<4>::_call(dd - 188, ss - 188);
    case 60:
      avx_ldst32ubyte_unroll<1>::_call(dd - 60, ss - 60);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 189:
      avx_ldst32ubyte_unroll<4>::_call(dd - 189, ss - 189);
    case 61:
      avx_ldst32ubyte_unroll<1>::_call(dd - 61, ss - 61);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 190:
      avx_ldst32ubyte_unroll<4>::_call(dd - 190, ss - 190);
    case 62:
      avx_ldst32ubyte_unroll<1>::_call(dd - 62, ss - 62);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 191:
      avx_ldst32ubyte_unroll<4>::_call(dd - 191, ss - 191);
    case 63:
      avx_ldst32ubyte_unroll<1>::_call(dd - 63, ss - 63);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 192:
      avx_ldst32ubyte_unroll<4>::_call(dd - 192, ss - 192);
    case 64:
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 193:
      avx_ldst32ubyte_unroll<4>::_call(dd - 193, ss - 193);
    case 65:
      avx_ldst32ubyte_unroll<2>::_call(dd - 65, ss - 65);
      dd[-1] = ss[-1];
      break;
    case 194:
      avx_ldst32ubyte_unroll<4>::_call(dd - 194, ss - 194);
    case 66:
      avx_ldst32ubyte_unroll<2>::_call(dd - 66, ss - 66);
      *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2));
      break;
    case 195:
      avx_ldst32ubyte_unroll<4>::_call(dd - 195, ss - 195);
    case 67:
      avx_ldst32ubyte_unroll<2>::_call(dd - 67, ss - 67);
      *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
      break;
    case 196:
      avx_ldst32ubyte_unroll<4>::_call(dd - 196, ss - 196);
    case 68:
      avx_ldst32ubyte_unroll<2>::_call(dd - 68, ss - 68);
      *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
      break;
    case 197:
      avx_ldst32ubyte_unroll<4>::_call(dd - 197, ss - 197);
    case 69:
      avx_ldst32ubyte_unroll<2>::_call(dd - 69, ss - 69);
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 198:
      avx_ldst32ubyte_unroll<4>::_call(dd - 198, ss - 198);
    case 70:
      avx_ldst32ubyte_unroll<2>::_call(dd - 70, ss - 70);
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 199:
      avx_ldst32ubyte_unroll<4>::_call(dd - 199, ss - 199);
    case 71:
      avx_ldst32ubyte_unroll<2>::_call(dd - 71, ss - 71);
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 200:
      avx_ldst32ubyte_unroll<4>::_call(dd - 200, ss - 200);
    case 72:
      avx_ldst32ubyte_unroll<2>::_call(dd - 72, ss - 72);
      *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
      break;
    case 201:
      avx_ldst32ubyte_unroll<4>::_call(dd - 201, ss - 201);
    case 73:
      avx_ldst32ubyte_unroll<2>::_call(dd - 73, ss - 73);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 202:
      avx_ldst32ubyte_unroll<4>::_call(dd - 202, ss - 202);
    case 74:
      avx_ldst32ubyte_unroll<2>::_call(dd - 74, ss - 74);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 203:
      avx_ldst32ubyte_unroll<4>::_call(dd - 203, ss - 203);
    case 75:
      avx_ldst32ubyte_unroll<2>::_call(dd - 75, ss - 75);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 204:
      avx_ldst32ubyte_unroll<4>::_call(dd - 204, ss - 204);
    case 76:
      avx_ldst32ubyte_unroll<2>::_call(dd - 76, ss - 76);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 205:
      avx_ldst32ubyte_unroll<4>::_call(dd - 205, ss - 205);
    case 77:
      avx_ldst32ubyte_unroll<2>::_call(dd - 77, ss - 77);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 206:
      avx_ldst32ubyte_unroll<4>::_call(dd - 206, ss - 206);
    case 78:
      avx_ldst32ubyte_unroll<2>::_call(dd - 78, ss - 78);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 207:
      avx_ldst32ubyte_unroll<4>::_call(dd - 207, ss - 207);
    case 79:
      avx_ldst32ubyte_unroll<2>::_call(dd - 79, ss - 79);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 208:
      avx_ldst32ubyte_unroll<4>::_call(dd - 208, ss - 208);
    case 80:
      avx_ldst32ubyte_unroll<2>::_call(dd - 80, ss - 80);
      avx_ldst16ubyte_unroll<1>::_call(dd - 16, ss - 16);
      break;
    case 209:
      avx_ldst32ubyte_unroll<4>::_call(dd - 209, ss - 209);
    case 81:
      avx_ldst32ubyte_unroll<2>::_call(dd - 81, ss - 81);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 210:
      avx_ldst32ubyte_unroll<4>::_call(dd - 210, ss - 210);
    case 82:
      avx_ldst32ubyte_unroll<2>::_call(dd - 82, ss - 82);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 211:
      avx_ldst32ubyte_unroll<4>::_call(dd - 211, ss - 211);
    case 83:
      avx_ldst32ubyte_unroll<2>::_call(dd - 83, ss - 83);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 212:
      avx_ldst32ubyte_unroll<4>::_call(dd - 212, ss - 212);
    case 84:
      avx_ldst32ubyte_unroll<2>::_call(dd - 84, ss - 84);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 213:
      avx_ldst32ubyte_unroll<4>::_call(dd - 213, ss - 213);
    case 85:
      avx_ldst32ubyte_unroll<2>::_call(dd - 85, ss - 85);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 214:
      avx_ldst32ubyte_unroll<4>::_call(dd - 214, ss - 214);
    case 86:
      avx_ldst32ubyte_unroll<2>::_call(dd - 86, ss - 86);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 215:
      avx_ldst32ubyte_unroll<4>::_call(dd - 215, ss - 215);
    case 87:
      avx_ldst32ubyte_unroll<2>::_call(dd - 87, ss - 87);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 216:
      avx_ldst32ubyte_unroll<4>::_call(dd - 216, ss - 216);
    case 88:
      avx_ldst32ubyte_unroll<2>::_call(dd - 88, ss - 88);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 217:
      avx_ldst32ubyte_unroll<4>::_call(dd - 217, ss - 217);
    case 89:
      avx_ldst32ubyte_unroll<2>::_call(dd - 89, ss - 89);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 218:
      avx_ldst32ubyte_unroll<4>::_call(dd - 218, ss - 218);
    case 90:
      avx_ldst32ubyte_unroll<2>::_call(dd - 90, ss - 90);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 219:
      avx_ldst32ubyte_unroll<4>::_call(dd - 219, ss - 219);
    case 91:
      avx_ldst32ubyte_unroll<2>::_call(dd - 91, ss - 91);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 220:
      avx_ldst32ubyte_unroll<4>::_call(dd - 220, ss - 220);
    case 92:
      avx_ldst32ubyte_unroll<2>::_call(dd - 92, ss - 92);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 221:
      avx_ldst32ubyte_unroll<4>::_call(dd - 221, ss - 221);
    case 93:
      avx_ldst32ubyte_unroll<2>::_call(dd - 93, ss - 93);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 222:
      avx_ldst32ubyte_unroll<4>::_call(dd - 222, ss - 222);
    case 94:
      avx_ldst32ubyte_unroll<2>::_call(dd - 94, ss - 94);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 223:
      avx_ldst32ubyte_unroll<4>::_call(dd - 223, ss - 223);
    case 95:
      avx_ldst32ubyte_unroll<2>::_call(dd - 95, ss - 95);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 224:
      avx_ldst32ubyte_unroll<4>::_call(dd - 224, ss - 224);
    case 96:
      avx_ldst32ubyte_unroll<2>::_call(dd - 96, ss - 96);
      avx_ldst32ubyte_unroll<1>::_call(dd - 32, ss - 32);
      break;
    case 225:
      avx_ldst32ubyte_unroll<4>::_call(dd - 225, ss - 225);
    case 97:
      avx_ldst32ubyte_unroll<2>::_call(dd - 97, ss - 97);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 226:
      avx_ldst32ubyte_unroll<4>::_call(dd - 226, ss - 226);
    case 98:
      avx_ldst32ubyte_unroll<2>::_call(dd - 98, ss - 98);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 227:
      avx_ldst32ubyte_unroll<4>::_call(dd - 227, ss - 227);
    case 99:
      avx_ldst32ubyte_unroll<2>::_call(dd - 99, ss - 99);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 228:
      avx_ldst32ubyte_unroll<4>::_call(dd - 228, ss - 228);
    case 100:
      avx_ldst32ubyte_unroll<2>::_call(dd - 100, ss - 100);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 229:
      avx_ldst32ubyte_unroll<4>::_call(dd - 229, ss - 229);
    case 101:
      avx_ldst32ubyte_unroll<2>::_call(dd - 101, ss - 101);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 230:
      avx_ldst32ubyte_unroll<4>::_call(dd - 230, ss - 230);
    case 102:
      avx_ldst32ubyte_unroll<2>::_call(dd - 102, ss - 102);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 231:
      avx_ldst32ubyte_unroll<4>::_call(dd - 231, ss - 231);
    case 103:
      avx_ldst32ubyte_unroll<2>::_call(dd - 103, ss - 103);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 232:
      avx_ldst32ubyte_unroll<4>::_call(dd - 232, ss - 232);
    case 104:
      avx_ldst32ubyte_unroll<2>::_call(dd - 104, ss - 104);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 233:
      avx_ldst32ubyte_unroll<4>::_call(dd - 233, ss - 233);
    case 105:
      avx_ldst32ubyte_unroll<2>::_call(dd - 105, ss - 105);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 234:
      avx_ldst32ubyte_unroll<4>::_call(dd - 234, ss - 234);
    case 106:
      avx_ldst32ubyte_unroll<2>::_call(dd - 106, ss - 106);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 235:
      avx_ldst32ubyte_unroll<4>::_call(dd - 235, ss - 235);
    case 107:
      avx_ldst32ubyte_unroll<2>::_call(dd - 107, ss - 107);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 236:
      avx_ldst32ubyte_unroll<4>::_call(dd - 236, ss - 236);
    case 108:
      avx_ldst32ubyte_unroll<2>::_call(dd - 108, ss - 108);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 237:
      avx_ldst32ubyte_unroll<4>::_call(dd - 237, ss - 237);
    case 109:
      avx_ldst32ubyte_unroll<2>::_call(dd - 109, ss - 109);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 238:
      avx_ldst32ubyte_unroll<4>::_call(dd - 238, ss - 238);
    case 110:
      avx_ldst32ubyte_unroll<2>::_call(dd - 110, ss - 110);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 239:
      avx_ldst32ubyte_unroll<4>::_call(dd - 239, ss - 239);
    case 111:
      avx_ldst32ubyte_unroll<2>::_call(dd - 111, ss - 111);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 240:
      avx_ldst32ubyte_unroll<4>::_call(dd - 240, ss - 240);
    case 112:
      avx_ldst32ubyte_unroll<2>::_call(dd - 112, ss - 112);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 241:
      avx_ldst32ubyte_unroll<4>::_call(dd - 241, ss - 241);
    case 113:
      avx_ldst32ubyte_unroll<2>::_call(dd - 113, ss - 113);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 242:
      avx_ldst32ubyte_unroll<4>::_call(dd - 242, ss - 242);
    case 114:
      avx_ldst32ubyte_unroll<2>::_call(dd - 114, ss - 114);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 243:
      avx_ldst32ubyte_unroll<4>::_call(dd - 243, ss - 243);
    case 115:
      avx_ldst32ubyte_unroll<2>::_call(dd - 115, ss - 115);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 244:
      avx_ldst32ubyte_unroll<4>::_call(dd - 244, ss - 244);
    case 116:
      avx_ldst32ubyte_unroll<2>::_call(dd - 116, ss - 116);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 245:
      avx_ldst32ubyte_unroll<4>::_call(dd - 245, ss - 245);
    case 117:
      avx_ldst32ubyte_unroll<2>::_call(dd - 117, ss - 117);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 246:
      avx_ldst32ubyte_unroll<4>::_call(dd - 246, ss - 246);
    case 118:
      avx_ldst32ubyte_unroll<2>::_call(dd - 118, ss - 118);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 247:
      avx_ldst32ubyte_unroll<4>::_call(dd - 247, ss - 247);
    case 119:
      avx_ldst32ubyte_unroll<2>::_call(dd - 119, ss - 119);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 248:
      avx_ldst32ubyte_unroll<4>::_call(dd - 248, ss - 248);
    case 120:
      avx_ldst32ubyte_unroll<2>::_call(dd - 120, ss - 120);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 249:
      avx_ldst32ubyte_unroll<4>::_call(dd - 249, ss - 249);
    case 121:
      avx_ldst32ubyte_unroll<2>::_call(dd - 121, ss - 121);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 250:
      avx_ldst32ubyte_unroll<4>::_call(dd - 250, ss - 250);
    case 122:
      avx_ldst32ubyte_unroll<2>::_call(dd - 122, ss - 122);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 251:
      avx_ldst32ubyte_unroll<4>::_call(dd - 251, ss - 251);
    case 123:
      avx_ldst32ubyte_unroll<2>::_call(dd - 123, ss - 123);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 252:
      avx_ldst32ubyte_unroll<4>::_call(dd - 252, ss - 252);
    case 124:
      avx_ldst32ubyte_unroll<2>::_call(dd - 124, ss - 124);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 253:
      avx_ldst32ubyte_unroll<4>::_call(dd - 253, ss - 253);
    case 125:
      avx_ldst32ubyte_unroll<2>::_call(dd - 125, ss - 125);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 254:
      avx_ldst32ubyte_unroll<4>::_call(dd - 254, ss - 254);
    case 126:
      avx_ldst32ubyte_unroll<2>::_call(dd - 126, ss - 126);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 255:
      avx_ldst32ubyte_unroll<4>::_call(dd - 255, ss - 255);
    case 127:
      avx_ldst32ubyte_unroll<2>::_call(dd - 127, ss - 127);
      avx_ldst32ubyte_unroll<2>::_call(dd - 64, ss - 64);
      break;
    case 256:
      avx_ldst32ubyte_unroll<8>::_call(dd - 256, ss - 256);
      break;
  }
}
}  // namespace common
#endif  // TINYRPC_FASTMEMCPY_HPP
