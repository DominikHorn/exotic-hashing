#pragma once

#include <cstddef>
#include <cstdint>

#include "include/convenience/builtins.hpp"

namespace exotic_hashing::support {
   template<class T>
   forceinline size_t ctz(const T& x) {
      switch (sizeof(T)) {
         case sizeof(std::uint32_t):
            return __builtin_ctz(x);
         case sizeof(std::uint64_t):
            return __builtin_ctzll(x);
         default:
            size_t i = 0;
            while (~((x >> i) & 0x1))
               i++;
            return i;
      }
   }

   template<class T>
   forceinline size_t clz(const T& x) {
      if (x == 0)
         return sizeof(T) * 8;

      switch (sizeof(T)) {
         case sizeof(std::uint32_t):
            return __builtin_clz(x);
         case sizeof(std::uint64_t):
            return __builtin_clzll(x);
         default:
            size_t i = 0;
            while (((x >> (sizeof(T) * 8 - i - 1)) & 0x1) == 0x0)
               i++;
            return i;
      }
   }
} // namespace exotic_hashing::support
