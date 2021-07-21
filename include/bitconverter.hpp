#pragma once

#include <sdsl/suffix_arrays.hpp>

#include "convenience/builtins.hpp"

namespace exotic_hashing {
   template<class T>
   struct FixedBitConverter {
      forceinline sdsl::bit_vector operator()(const T& data) const {
         const size_t bit_size = sizeof(T) * 8;
         sdsl::bit_vector result(bit_size, 0);
         assert(result.size() == bit_size);
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
} // namespace exotic_hashing
