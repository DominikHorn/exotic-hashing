#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "include/convenience/builtins.hpp"

namespace exotic_hashing {
   template<class T>
   size_t clz(const T& x) {
      if (unlikely(x == 0))
         return sizeof(T) * 8;

      switch (sizeof(T)) {
         case sizeof(unsigned int):
            return __builtin_clz(x);
         case sizeof(unsigned long long):
            return __builtin_clzll(x);
         default:
            size_t i = 0;
            while (((x >> (sizeof(T) * 8 - i - 1)) & 0x1) == 0x0)
               i++;
            return i;
      }
   }

   /**
    * Elias Gamma Encoding and Decoding for positive
    * integers including 0.
    *
    * @tparam BitStream bitstream container. Must support `push_back()` and
    *   index access `[i]`. Defaults to std::vector<bool>
    * @tparam T encoded integer datatype, e.g., uint64_t. Note that T must only
    *   be able to represent your number, i.e., it is not necessary to downsize
    *   before encoding. For example `encode(uint8_t x1)` and `encode(uint64_t
    *   x2)` produce the same bitstream iff x1 == x2. defaults to uint64_t
    *
    */
   template<class BitStream = std::vector<bool>, class T = std::uint64_t>
   struct EliasGammaCoder {
      forceinline T decode(const BitStream& stream) const {
         // decode N = floor(log2(x)) (unary)
         size_t N = 0;
         while (N < stream.size() && stream[N] == 0x0)
            N++;

         if (N == 0)
            return 1;

         // number as 0x1 followed by the remaining bits
         T res = 0x1;
         for (size_t i = 0; i + N + 1 < stream.size(); i++) {
            res <<= 1;
            res |= stream[i + N + 1] & 0x1;
         }

         return res;
      }

      /**
       * Elias gamma encodes
       */
      forceinline BitStream encode(const T& x) const {
         assert(x != 0);

         // N = floor(log2(x))
         const size_t lz = clz(x);
         size_t N = (sizeof(T) * 8) - clz(x) - 1;
         if (unlikely(lz == 0))
            N = sizeof(T) * 8;
         assert(N == static_cast<size_t>(std::floor(std::log2(x))));

         // encode N in unary
         BitStream res;
         for (size_t i = 0; i < N; i++)
            res.push_back(0);
         res.push_back(1);

         // append the N-1 remaining binary digits of x
         for (size_t i = N - 1; N > i && i >= 0; i--)
            res.push_back((x >> i) & 0x1);

         return res;
      }
   };
} // namespace exotic_hashing
