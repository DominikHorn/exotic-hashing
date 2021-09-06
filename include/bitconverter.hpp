#pragma once

#include <cassert>
#include <vector>

#include "convenience/builtins.hpp"

namespace exotic_hashing::support {
   template<class T, class BitStream = std::vector<bool>>
   struct FixedBitConverter {
      forceinline BitStream operator()(const T& data) const {
         const size_t bit_size = sizeof(T) * 8;
         BitStream result(bit_size, 0);
         assert(result.size() == bit_size);

         // TODO: use faster bitvector impl (constructor that takes generator lambda which yields bit at index!)
         for (size_t i = 0; i < bit_size; i++)
            result[i] = (data >> (bit_size - i - 1)) & 0x1;
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
