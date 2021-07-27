#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <emmintrin.h>
#include <mmintrin.h>

#include <hashing.hpp>

#include "include/convenience/builtins.hpp"

namespace exotic_hashing {
   template<class Data>
   struct MWHC {
      explicit MWHC(const std::vector<Data>& d) {
         hashing::AquaHash<Data> hashfn;
      }

      static std::string name() {
         return "MWHC";
      }

      forceinline size_t operator()(const Data& key) const {
         // TODO(dominik): implement
         return 0;
      }

      size_t byte_size() const {
         // TODO(dominik): implement
         return 0;
      }

     private:
      forceinline __m128i seed(const std::uint64_t lower, const std::uint64_t upper = 0) {
         return _mm_setr_epi8(static_cast<char>((upper >> 56) & 0xFF), static_cast<char>((upper >> 48) & 0xFF),
                              static_cast<char>((upper >> 40) & 0xFF), static_cast<char>((upper >> 32) & 0xFF),
                              static_cast<char>((upper >> 24) & 0xFF), static_cast<char>((upper >> 16) & 0xFF),
                              static_cast<char>((upper >> 8) & 0xFF), static_cast<char>((upper >> 0) & 0xFF),

                              static_cast<char>((lower >> 56) & 0xFF), static_cast<char>((lower >> 48) & 0xFF),
                              static_cast<char>((lower >> 40) & 0xFF), static_cast<char>((lower >> 32) & 0xFF),
                              static_cast<char>((lower >> 24) & 0xFF), static_cast<char>((lower >> 16) & 0xFF),
                              static_cast<char>((lower >> 8) & 0xFF), static_cast<char>((lower >> 0) & 0xFF));
      }
   };
} // namespace exotic_hashing
