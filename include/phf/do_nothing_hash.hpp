#pragma once

#include <string>
#include <vector>

#include "../convenience/builtins.hpp"
#include "../convenience/tidy.hpp"

namespace exotic_hashing {
   /**
    * Most basic perfect hash function, i.e.,
    * identity mapping keys to their integer
    * value. Meant as a speed baseline
    * (can't get faster than this)
    */
   template<class Data>
   struct DoNothingHash {
      explicit DoNothingHash(const std::vector<Data>& d) {
         UNUSED(d);
      }

      static std::string name() {
         return "DoNothingHash";
      }

      constexpr forceinline size_t operator()(const Data& key) const {
         return key;
      }

      size_t byte_size() const {
         return 0;
      };
   };

} // namespace exotic_hashing
