#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "include/bitvector.hpp"
#include "include/convenience/builtins.hpp"
#include "include/support.hpp"

namespace exotic_hashing::support {
   /**
    * Elias Gamma Encoding and Decoding for positive integers (excluding 0).
    */
   struct EliasGammaCoder {
      /**
       * Elias gamma encodes a given positive integer into a bitsream
       *
       * @tparam BitStream bitstream container. Must support `push_back()`.
       *   Defaults to support::Bitvector
       * @tparam T integer datatype, e.g., uint64_t. Note that `sizeof(T)` does
       *   not influence the resulting bitstream, i.e., downcasting to a smaller
       *   type is not necessary before encoding. Defaults to std::uint64_t
       */
      template<class BitStream = Bitvector<>, class T = std::uint64_t>
      static forceinline BitStream encode(const T& x) {
         assert(x > 0);

         // special case x == 1, i.e., N == 0
         if (x == 1)
            return BitStream(1, true);

         // N = floor(log2(x))
         const size_t lz = clz(x);
         const size_t N = unlikely(lz == 0) ? sizeof(T) * 8 : (sizeof(T) * 8 - lz - 1);
         assert(N == static_cast<size_t>(std::floor(std::log2(x))));

         // 1. encode N in unary
         BitStream res(N + 1, false);
         res[N] = true;

         // 2. append the N-1 remaining binary digits of x
         res.append(x, N);

         return res;
      }

      /**
       * Decodes an elias gamma encoded bitstream
       *
       * @tparam T encoded integer datatype, e.g., uint64_t. Note that T must
       *   be large enough to represent your number. Defaults to uint64_t
       * @tparam BitStream bitstream container. Must support index access via
       *   `[i]`. Defaults to Bitvector<>
       *
       * @param stream the bitstream to decode
       * @param start first index in bitstream to look at. Defaults to 0.
       *
       * @return the decoded number as well as the amount of bits consumed
       */
      template<class T = std::uint64_t, class BitStream = Bitvector<>>
      static forceinline std::pair<T, size_t> decode(const BitStream& stream, const size_t start = 0) {
         // decode N = floor(log2(x)) (unary)
         const size_t N = stream.count_zeroes(start);
         if (N == 0)
            return {1, 1U};

         // compute bitstream encoded size from N
         const auto total_length = 2 * N + 1;

         // decode remaining N bits
         const T tail = stream.extract(start + N + 1, start + total_length);

         // x is 0x1 followed by the remaining bits.
         // ternary guards against invalid shift exponent
         return {(N >= sizeof(T) * 8 ? 0x0 : (0x1 << N)) | tail, total_length};
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
       *   Defaults to Bitvector<>
       * @tparam T integer datatype, e.g., uint64_t. Note that `sizeof(T)` does
       *   not influence the resulting bitstream, i.e., downcasting to a smaller
       *   type is not necessary before encoding. Defaults to std::uint64_t
       */
      template<class BitStream = Bitvector<>, class T = std::uint64_t>
      static forceinline BitStream encode(const T& x) {
         // TODO(dominik): optimize
         assert(x > 0);

         // N = floor(log2(x))
         const size_t lz = clz(x);
         const size_t N = unlikely(lz == 0) ? sizeof(T) * 8 : (sizeof(T) * 8) - lz - 1;
         assert(N == static_cast<size_t>(std::floor(std::log2(x))));

         // 1. encode N+1 with elias gamma encoding
         BitStream res = EliasGammaCoder::encode(N + 1);

         // 2. append the N remaining binary digits of x to this representation
         if (N > 0)
            res.append(x, N);

         return res;
      }

      /**
       * Decodes an elias delta encoded bitstream
       *
       * @tparam T encoded integer datatype, e.g., uint64_t. Note that T must
       *   be large enough to represent your number. Defaults to uint64_t
       * @tparam BitStream bitstream container. Must support index access via
       *   `[i]`. Defaults to Bitvector<>
       *
       * @param stream the bitstream to decode
       * @param start first index in bitstream to look at. Defaults to 0. Must be within bounds!
       *
       */
      template<class T = std::uint64_t, class BitStream = Bitvector<>>
      static forceinline std::pair<T, size_t> decode(const BitStream& stream, const size_t start = 0) {
         // decode N (first bits encode N+1)
         const auto [N_Inc, bits] = EliasGammaCoder::decode(stream, start);
         const auto N = N_Inc - 1;

         if (N == 0)
            return {1, bits};

         // number as 0x1 followed by the remaining bits
         // decode remaining N bits
         const T tail = stream.extract(start + bits, start + bits + N);

         return {(N >= sizeof(T) * 8 ? 0x0 : (0x1 << N)) | tail, bits + N};
      }
   };
} // namespace exotic_hashing::support
