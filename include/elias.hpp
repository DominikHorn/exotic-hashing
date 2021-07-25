#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <tuple>
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
    * Elias Gamma Encoding and Decoding for positive integers (excluding 0).
    */
   struct EliasGammaCoder {
      /**
       * Elias gamma encodes a given positive integer into a bitsream
       *
       * @tparam BitStream bitstream container. Must support `push_back()`.
       *   Defaults to std::vector<bool>
       * @tparam T integer datatype, e.g., uint64_t. Note that `sizeof(T)` does
       *   not influence the resulting bitstream, i.e., downcasting to a smaller
       *   type is not necessary before encoding. Defaults to std::uint64_t
       */
      template<class BitStream = std::vector<bool>, class T = std::uint64_t>
      static forceinline BitStream encode(const T& x) {
         assert(x > 0);

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

      /**
       * Decodes an elias gamma encoded bitstream
       *
       * @tparam T encoded integer datatype, e.g., uint64_t. Note that T must
       *   be large enough to represent your number. Defaults to uint64_t
       * @tparam BitStream bitstream container. Must support index access via
       *   `[i]`. Defaults to std::vector<bool>
       *
       * @return the decoded number as well as the amount of bits consumed
       */
      template<class T = std::uint64_t, class BitStream = std::vector<bool>>
      static forceinline std::tuple<T, size_t> decode(const BitStream& stream) {
         // decode N = floor(log2(x)) (unary)
         size_t N = 0;
         while (N < stream.size() && stream[N] == 0x0)
            N++;

         if (N == 0)
            return std::make_tuple(1, 1);

         // number as 0x1 followed by the remaining bits
         T res = 0x1;
         for (size_t i = 0; i < N && i + N + 1 < stream.size(); i++) {
            res <<= 1;
            res |= stream[i + N + 1] & 0x1;
         }

         return std::make_tuple(res, 2 * N + 1);
      }
   };

   /**
    * Elias Delta Encoding and Decoding for positive integers (excluding 0).
    */
   struct EliasDeltaCoder {
      /**
       * Elias delta encodes a given positive integer into a bitsream
       *
       * @tparam BitStream bitstream container. Must support `push_back()`.
       *   Defaults to std::vector<bool>
       * @tparam T integer datatype, e.g., uint64_t. Note that `sizeof(T)` does
       *   not influence the resulting bitstream, i.e., downcasting to a smaller
       *   type is not necessary before encoding. Defaults to std::uint64_t
       */
      template<class BitStream = std::vector<bool>, class T = std::uint64_t>
      static forceinline BitStream encode(const T& x) {
         assert(x > 0);

         // N = floor(log2(x))
         const size_t lz = clz(x);
         size_t N = (sizeof(T) * 8) - clz(x) - 1;
         if (unlikely(lz == 0))
            N = sizeof(T) * 8;
         assert(N == static_cast<size_t>(std::floor(std::log2(x))));

         // encode N+1 with elias gamma encoding
         BitStream res = EliasGammaCoder::encode(N + 1);

         // append the N remaining binary digits of x to this representation
         for (size_t i = N - 1; N > i && i >= 0; i--)
            res.push_back((x >> i) & 0x1);

         return res;
      }

      /**
       * Decodes an elias delta encoded bitstream
       *
       * @tparam T encoded integer datatype, e.g., uint64_t. Note that T must
       *   be large enough to represent your number. Defaults to uint64_t
       * @tparam BitStream bitstream container. Must support index access via
       *   `[i]`. Defaults to std::vector<bool>
       */
      template<class T = std::uint64_t, class BitStream = std::vector<bool>>
      static forceinline std::tuple<T, size_t> decode(const BitStream& stream) {
         // decode N (first bits encode N+1)
         const auto [N_Inc, bits] = EliasGammaCoder::decode(stream);
         const auto N = N_Inc - 1;

         if (N == 0)
            return std::make_tuple(1, bits);

         // number as 0x1 followed by the remaining bits
         T res = 0x1;
         for (size_t i = 0; i < N && i + bits < stream.size(); i++) {
            res <<= 1;
            res |= stream[i + bits] & 0x1;
         }

         return std::make_tuple(res, bits + N);
      }
   };
} // namespace exotic_hashing
