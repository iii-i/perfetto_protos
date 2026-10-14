/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INCLUDE_PERFETTO_BASE_ENDIAN_H_
#define INCLUDE_PERFETTO_BASE_ENDIAN_H_

#include <stdint.h>
#include <stdlib.h>  // For MSVC
#include <string.h>
#include <type_traits>

#include "perfetto/base/build_config.h"
#include "perfetto/base/compiler.h"

namespace perfetto {
namespace base {

#if PERFETTO_BUILDFLAG(PERFETTO_COMPILER_MSVC)
inline uint16_t ByteSwap16(uint16_t x) {
  return _byteswap_ushort(x);
}
inline uint32_t ByteSwap32(uint32_t x) {
  return _byteswap_ulong(x);
}
inline uint64_t ByteSwap64(uint64_t x) {
  return _byteswap_uint64(x);
}
#else
inline uint16_t ByteSwap16(uint16_t x) {
  return __builtin_bswap16(x);
}
inline uint32_t ByteSwap32(uint32_t x) {
  return __builtin_bswap32(x);
}
inline uint64_t ByteSwap64(uint64_t x) {
  return __builtin_bswap64(x);
}
#endif

template <typename T>
inline T ByteSwap(T x) {
  static_assert(std::is_arithmetic_v<T>,
                "T must be an integer or floating-point type");
  if constexpr (sizeof(T) == 2) {
    uint16_t u;
    memcpy(&u, &x, sizeof(u));
    u = ByteSwap16(u);
    memcpy(&x, &u, sizeof(u));
  } else if constexpr (sizeof(T) == 4) {
    uint32_t u;
    memcpy(&u, &x, sizeof(u));
    u = ByteSwap32(u);
    memcpy(&x, &u, sizeof(u));
  } else if constexpr (sizeof(T) == 8) {
    uint64_t u;
    memcpy(&u, &x, sizeof(u));
    u = ByteSwap64(u);
    memcpy(&x, &u, sizeof(u));
  } else {
    static_assert(sizeof(T) == 1, "Unsupported size");
  }
  return x;
}

template <typename T>
inline T HostToLE(T x) {
  return PERFETTO_IS_LITTLE_ENDIAN() ? x : ByteSwap(x);
}

template <typename T>
inline T HostToBE(T x) {
  return PERFETTO_IS_LITTLE_ENDIAN() ? ByteSwap(x) : x;
}

template <typename T>
inline T LEToHost(T x) {
  return HostToLE(x);
}

template <typename T>
inline T BEToHost(T x) {
  return HostToBE(x);
}

template <typename T>
inline T LoadLE(const void* p) {
  T v;
  memcpy(&v, p, sizeof(v));
  return LEToHost(v);
}

template <typename T>
inline void StoreLE(void* p, T v) {
  v = HostToLE(v);
  memcpy(p, &v, sizeof(v));
}

template <typename T>
inline T LoadBE(const void* p) {
  T v;
  memcpy(&v, p, sizeof(v));
  return BEToHost(v);
}

template <typename T>
inline void StoreBE(void* p, T v) {
  v = HostToBE(v);
  memcpy(p, &v, sizeof(v));
}

}  // namespace base
}  // namespace perfetto

#endif  // INCLUDE_PERFETTO_BASE_ENDIAN_H_
