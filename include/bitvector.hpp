#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

#include "convenience/builtins.hpp"

namespace exotic_hashing {
   template<class Storage>
   class Bitref {
      Storage& unit;
      const size_t n;

     public:
      Bitref(Storage& unit, const size_t n) : unit(unit), n(n) {}

      Bitref& operator=(bool const& rhs) {
         Storage new_bit = !!rhs;
         unit ^= (-new_bit ^ unit) & (1UL << n);
         return *this;
      }

      explicit operator bool() const {
         return (unit >> n) & 1U;
      }
   };

   template<class Storage = std::uint64_t>
   struct Bitvector {
      Bitvector() = default;
      explicit Bitvector(const std::vector<bool>& other) : bitcnt(other.size()) {
         const auto unit_cnt = (other.size() + unit_bits() - 1) / unit_bits();
         storage.resize(unit_cnt);

         for (size_t i = 0; i < other.size(); i++)
            this->operator[](i) = other[i];
      }

      /**
       * counts zeroes starting at index until the first 1 is encountered
       */
      forceinline size_t count_zeroes(size_t index = 0) {
         assert(index < bitcnt);

         const auto u_ind = unit_index(index);
         const auto l_ind = unit_local_index(index);

         const auto val = (storage[u_ind] >> l_ind);

         size_t cnt = 0;
         // ctz is undefined for val == 0
         if (val == 0) {
            // last storage unit
            if (u_ind + 1 >= storage.size())
               return bitcnt - (unit_bits() * u_ind + l_ind);

            cnt = unit_bits() - l_ind;
         } else
            cnt = __builtin_ctz(val);

         // special case: zero string wraps to next storage unit
         if (cnt + l_ind >= unit_bits() && cnt + l_ind < bitcnt)
            return cnt + count_zeroes(cnt + index);

         return cnt;
      }

      /**
       * provides read access to i-th bit
       */
      forceinline bool operator[](size_t index) const {
         return (storage[unit_index(index)] >> unit_local_index(index)) & 0x1;
      };

      /**
       * provides read/write access to i-th bit
       */
      forceinline Bitref<Storage> operator[](size_t index) {
         return Bitref(storage[unit_index(index)], unit_local_index(index));
      };

      /**
       * amount of bits stored in this bitvector
       */
      forceinline size_t size() const {
         return bitcnt;
      }

     private:
      std::vector<Storage> storage;
      size_t bitcnt = 0;

      static forceinline size_t unit_bits() {
         return sizeof(Storage) * 8;
      }

      forceinline size_t unit_index(const size_t& index) const {
         assert(index < bitcnt);
         return index >> __builtin_ctz(unit_bits());
      }

      forceinline size_t unit_local_index(const size_t& index) const {
         assert(index < bitcnt);
         return index & ((0x1 << __builtin_ctz(unit_bits())) - 1);
      }
   };
} // namespace exotic_hashing
