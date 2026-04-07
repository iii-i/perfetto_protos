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

#ifndef INCLUDE_PERFETTO_EXT_BASE_ENDIAN_H_
#define INCLUDE_PERFETTO_EXT_BASE_ENDIAN_H_

#include <stdint.h>
#include <stdlib.h>  // For MSVC
#include <string.h>

#include <type_traits>

#include "perfetto/base/build_config.h"
#include "perfetto/base/compiler.h"

namespace perfetto {
namespace base {

#if PERFETTO_IS_LITTLE_ENDIAN()
#if PERFETTO_BUILDFLAG(PERFETTO_COMPILER_MSVC)
inline uint16_t HostToBE16(uint16_t x) {
  return _byteswap_ushort(x);
}
inline uint32_t HostToBE32(uint32_t x) {
  return _byteswap_ulong(x);
}
inline uint64_t HostToBE64(uint64_t x) {
  return _byteswap_uint64(x);
}
inline uint16_t BE16ToHost(uint16_t x) {
  return _byteswap_ushort(x);
}
inline uint32_t BE32ToHost(uint32_t x) {
  return _byteswap_ulong(x);
}
inline uint64_t BE64ToHost(uint64_t x) {
  return _byteswap_uint64(x);
}
#else
inline uint16_t HostToBE16(uint16_t x) {
  return __builtin_bswap16(x);
}
inline uint32_t HostToBE32(uint32_t x) {
  return __builtin_bswap32(x);
}
inline uint64_t HostToBE64(uint64_t x) {
  return __builtin_bswap64(x);
}
inline uint16_t BE16ToHost(uint16_t x) {
  return __builtin_bswap16(x);
}
inline uint32_t BE32ToHost(uint32_t x) {
  return __builtin_bswap32(x);
}
inline uint64_t BE64ToHost(uint64_t x) {
  return __builtin_bswap64(x);
}
#endif
inline uint16_t HostToLE16(uint16_t x) {
  return x;
}
inline uint32_t HostToLE32(uint32_t x) {
  return x;
}
inline uint64_t HostToLE64(uint64_t x) {
  return x;
}
inline uint16_t LE16ToHost(uint16_t x) {
  return x;
}
inline uint32_t LE32ToHost(uint32_t x) {
  return x;
}
inline uint64_t LE64ToHost(uint64_t x) {
  return x;
}
#else
inline uint16_t HostToBE16(uint16_t x) {
  return x;
}
inline uint32_t HostToBE32(uint32_t x) {
  return x;
}
inline uint64_t HostToBE64(uint64_t x) {
  return x;
}
inline uint16_t BE16ToHost(uint16_t x) {
  return x;
}
inline uint32_t BE32ToHost(uint32_t x) {
  return x;
}
inline uint64_t BE64ToHost(uint64_t x) {
  return x;
}
inline uint16_t HostToLE16(uint16_t x) {
  return __builtin_bswap16(x);
}
inline uint32_t HostToLE32(uint32_t x) {
  return __builtin_bswap32(x);
}
inline uint64_t HostToLE64(uint64_t x) {
  return __builtin_bswap64(x);
}
inline uint16_t LE16ToHost(uint16_t x) {
  return __builtin_bswap16(x);
}
inline uint32_t LE32ToHost(uint32_t x) {
  return __builtin_bswap32(x);
}
inline uint64_t LE64ToHost(uint64_t x) {
  return __builtin_bswap64(x);
}
#endif

// Generic byte-order conversions for any arithmetic type (signed/unsigned
// integers, float, double). Dispatched by size; size 1 is a no-op. Handles
// types that don't match one of the sized primitives exactly (e.g. `long long`
// on platforms where `long` is already the 64-bit typedef).
template <typename T,
          typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T HostToBE(T x) {
  if constexpr (sizeof(T) == 2) {
    uint16_t i;
    memcpy(&i, &x, sizeof(i));
    i = HostToBE16(i);
    memcpy(&x, &i, sizeof(i));
  } else if constexpr (sizeof(T) == 4) {
    uint32_t i;
    memcpy(&i, &x, sizeof(i));
    i = HostToBE32(i);
    memcpy(&x, &i, sizeof(i));
  } else if constexpr (sizeof(T) == 8) {
    uint64_t i;
    memcpy(&i, &x, sizeof(i));
    i = HostToBE64(i);
    memcpy(&x, &i, sizeof(i));
  }
  return x;
}

template <typename T,
          typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T BEToHost(T x) {
  if constexpr (sizeof(T) == 2) {
    uint16_t i;
    memcpy(&i, &x, sizeof(i));
    i = BE16ToHost(i);
    memcpy(&x, &i, sizeof(i));
  } else if constexpr (sizeof(T) == 4) {
    uint32_t i;
    memcpy(&i, &x, sizeof(i));
    i = BE32ToHost(i);
    memcpy(&x, &i, sizeof(i));
  } else if constexpr (sizeof(T) == 8) {
    uint64_t i;
    memcpy(&i, &x, sizeof(i));
    i = BE64ToHost(i);
    memcpy(&x, &i, sizeof(i));
  }
  return x;
}

template <typename T,
          typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T HostToLE(T x) {
  if constexpr (sizeof(T) == 2) {
    uint16_t i;
    memcpy(&i, &x, sizeof(i));
    i = HostToLE16(i);
    memcpy(&x, &i, sizeof(i));
  } else if constexpr (sizeof(T) == 4) {
    uint32_t i;
    memcpy(&i, &x, sizeof(i));
    i = HostToLE32(i);
    memcpy(&x, &i, sizeof(i));
  } else if constexpr (sizeof(T) == 8) {
    uint64_t i;
    memcpy(&i, &x, sizeof(i));
    i = HostToLE64(i);
    memcpy(&x, &i, sizeof(i));
  }
  return x;
}

template <typename T,
          typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T LEToHost(T x) {
  if constexpr (sizeof(T) == 2) {
    uint16_t i;
    memcpy(&i, &x, sizeof(i));
    i = LE16ToHost(i);
    memcpy(&x, &i, sizeof(i));
  } else if constexpr (sizeof(T) == 4) {
    uint32_t i;
    memcpy(&i, &x, sizeof(i));
    i = LE32ToHost(i);
    memcpy(&x, &i, sizeof(i));
  } else if constexpr (sizeof(T) == 8) {
    uint64_t i;
    memcpy(&i, &x, sizeof(i));
    i = LE64ToHost(i);
    memcpy(&x, &i, sizeof(i));
  }
  return x;
}

}  // namespace base
}  // namespace perfetto

#endif  // INCLUDE_PERFETTO_EXT_BASE_ENDIAN_H_
