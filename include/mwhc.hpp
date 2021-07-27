#pragma once

#include <cstddef>
#include <string>

#include <hashing.hpp>

#include "include/convenience/builtins.hpp"

namespace exotic_hashing {
   template<class Data>
   struct MWHC {
      explicit MWHC(const std::vector<Data>& d) {}

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
   };
} // namespace exotic_hashing
