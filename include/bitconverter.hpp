#pragma once

#include <cassert>
#include <vector>

#include "convenience/builtins.hpp"
#include "include/bitvector.hpp"

namespace exotic_hashing::support {
   template<class T, class BitStream = Bitvector<>>
   struct FixedBitConverter {
      forceinline BitStream operator()(const T& data) const {
         const size_t bit_size = sizeof(T) * 8;
         BitStream result(bit_size, [&](const size_t& index) { return (data >> (bit_size - index - 1)) & 0x1; });
         assert(result.size() == bit_size);
#ifndef NDEBUG
         T reconstructed = 0;
         for (size_t i = 0; i < result.size(); i++) {
            const uint64_t bit = result[i];
            reconstructed |= bit << (bit_size - i - 1);
         }
         assert(reconstructed == data);
#endif
         return result;
      }
   };
} // namespace exotic_hashing::support
