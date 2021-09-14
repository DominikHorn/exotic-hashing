#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

#include "include/convenience/builtins.hpp"
#include "include/support.hpp"

namespace exotic_hashing::support {
   template<class Storage = std::uint64_t>
   class Bitvector {
      class Bitref {
         Storage& unit;
         const size_t n;

        public:
         Bitref(Storage& unit, const size_t n) : unit(unit), n(n) {}

         Bitref& operator=(const bool& rhs) {
            Storage new_bit = !!rhs;
            unit ^= (-new_bit ^ unit) & (1UL << n);
            return *this;
         }

         Bitref& operator=(const Bitref& rhs) {
            if (&rhs != this) {
               Storage new_bit = !!rhs;
               unit ^= (-new_bit ^ unit) & (1UL << n);
            }

            return *this;
         }

         // this is a very rare case where implicit bool conversion is perfectly fine
         // since Bitref essentially represents a bool!
         operator bool() const { // NOLINT
            return (unit >> n) & 1U;
         }
      };

     public:
      /**
       * given a bitcnt and generator function, bulk load the bitvector
       * in the (hopefully) most efficient way.
       *
       * generator function is supposed to return each bit's value given an index.
       * Note that convenience initializers exist, e.g., for converting from std::vector<bool>
       */
      template<class Generator>
      Bitvector(const size_t& bitcnt, const Generator gen) : bitcnt(bitcnt) {
         const auto unit_cnt = (bitcnt + unit_bits() - 1) / unit_bits();
         storage.resize(unit_cnt);
         storage_size = unit_cnt;

         // attempt to minimize bit access overhead by 'bulk loading' storage
         for (size_t u_ind = 0; u_ind < unit_cnt; u_ind++) {
            const size_t g_ind = u_ind * unit_bits();
            const size_t msb = std::min(unit_bits(), bitcnt - g_ind) - 1;

            Storage unit = 0x0;
            for (size_t l_ind = 0; l_ind <= msb; l_ind++) {
               unit <<= 1;
               unit |= gen(g_ind + msb - l_ind) & 0x1;
            }

            storage[u_ind] = unit;
         }
      }

      /**
       * convenience initializer for loading integer datatypes
       * (bitorder is efficiently reversed)
       */
      template<class T>
      explicit Bitvector(const T& data);

      explicit Bitvector(const std::uint64_t& data) : bitcnt(sizeof(std::uint64_t) * 8) {
         // compatibility with other compilers
         // TODO(dominik): provide optimal bitreverse ourselfs (?)
#ifndef __has_builtin
   #define __has_builtin(x) 0
#endif
#if __has_builtin(__builtin_bitreverse64)
         storage.push_back(__builtin_bitreverse64(data));
#else
   #warning "using unoptimized bitreverse implementation in Bitvector(std::uint64_t)
         Storage val = 0x0;
         for (size_t i = 0; i < bitcnt; i++) {
            val <<= 1;
            val |= (data >> i) & 0x1;
         }
         storage.push_back(val);
         storage_size++;
#endif
      }

      /**
       * convenience initializer from std::vector<bool>
       */
      explicit Bitvector(const std::vector<bool>& other)
         : Bitvector(other.size(), [&](const size_t& index) { return other[index]; }) {}

      /**
       * convenience initialize with specific size & all values set to a single value
       */
      explicit Bitvector(const size_t& bitcnt, const bool& value)
         : Bitvector(bitcnt, [&](const size_t /*index*/) { return value; }) {}

      /**
       * zero parameter constructor
       */
      explicit Bitvector() : Bitvector(0, false) {}

      /**
       * provides read access to i-th bit
       */
      forceinline bool operator[](size_t index) const {
         return (storage[unit_index(index)] >> unit_local_index(index)) & 0x1;
      };

      /**
       * provides read/write access to i-th bit
       */
      forceinline Bitref operator[](size_t index) {
         return Bitref(storage[unit_index(index)], unit_local_index(index));
      };

      /**
       * amount of bits stored in this bitvector
       */
      forceinline size_t size() const {
         return bitcnt;
      }

      /**
       * append a single bit to this bitvector
       */
      forceinline void append(const bool& val) {
         const size_t index = bitcnt++;

         const auto u_ind = unit_index(index);
         if (u_ind >= storage_size) {
            storage.push_back(0x0);
            storage_size++;
         }

         this->operator[](index) = val;
      }

      /**
       * appends cnt bytes from val to this vector. Note that cnt must
       * be greater than 0
       */
      forceinline void append(const Storage& val, const size_t cnt, const size_t start = 0) {
         assert(cnt > 0);

         const auto mask = cnt >= sizeof(Storage) * 8 ? ~(0x0) : (0x1ULL << cnt) - 1;
         const Storage data = (val >> start) & mask;

         const auto u_ind = unit_index(bitcnt);
         const auto l_ind = unit_local_index(bitcnt);

         // catch, e.g., empty bitvector case
         if (u_ind >= storage_size) {
            storage.push_back(data);
            storage_size++;
         } else
            storage[u_ind] |= data << l_ind;

         // catch overflow into next (non existent) storage unit
         const auto lower_bitcnt = unit_bits() - l_ind;
         if (cnt > lower_bitcnt) {
            storage.push_back(data >> lower_bitcnt);
            storage_size++;
         }

         // don't forget to increase bitcnt
         bitcnt += cnt;
      }

      /**
       * appends all bits from other bitstream to this bitstream
       */
      forceinline void append(const Bitvector& other) {
         // TODO(dominik): optimize this operation
         for (size_t i = 0; i < other.size(); i++)
            append(other[i]);
      }

      /**
       * counts zeroes starting at index until the first 1 is encountered
       */
      forceinline size_t count_zeroes(size_t index = 0) const {
         assert(index < bitcnt);

         auto l_ind = unit_local_index(index);
         size_t first_set = 0;
         for (auto u_ind = unit_index(index); u_ind < storage_size; u_ind++) {
            const auto val = storage[u_ind] >> l_ind;
            if (val > 0) // search is done iff there is at least one set bit
               return first_set + ctz(val);
            else
               first_set += unit_bits() - l_ind;

            // next block has to be scanned starting at l_ind 0
            l_ind = 0;
         }

         // account for last block potentially not being full size
         return first_set - (unit_bits() * storage_size - bitcnt);
      }

      /**
       * extracts a block of bits [start, stop). Note that, in addition to
       * being in bounds, stop_index must be greater or equal to start_index
       * and stop_index - start_index must at most be sizeof(Storage)*8
       */
      forceinline Storage extract(size_t start_index, size_t stop_index) const {
         assert(start_index >= 0);
         assert(stop_index > start_index);
         assert(stop_index <= bitcnt);

         const auto size = stop_index - start_index;
         assert(size <= sizeof(Storage) * 8);

         const auto start_u_ind = unit_index(start_index);
         const auto start_l_ind = unit_local_index(start_index);
         const auto stop_u_ind = unit_index(stop_index);

         // all in the same block, take shortcut
         if (start_u_ind == stop_u_ind)
            return (storage[start_u_ind] >> start_l_ind) & ((0x1ULL << size) - 1);

         const auto stop_l_ind = unit_local_index(stop_index);

         const Storage upper_mask = (0x1ULL << stop_l_ind) - 1;
         const Storage upper = storage[stop_u_ind] & upper_mask;
         const Storage lower = storage[start_u_ind] >> start_l_ind;

         const auto shift = unit_bits() - start_l_ind;
         if (shift >= sizeof(Storage) * 8)
            return lower;

         return (upper << shift) | lower;
      }

      /**
       * Checks whether this bitvector matches the given prefix, beginning at start.
       * Test succeeds when all prefix bits or self bits are consumed (whichever one comes first).
       *
       * @param prefix
       * @param start optional offset for self to start checking from. Defaults to 0
       */
      forceinline bool matches(const Bitvector<Storage>& prefix, const size_t& start = 0) const {
         for (size_t i = 0; i + start < size() && i < prefix.size(); i++)
            if (operator[](i + start) != prefix[i])
               return false;
         return true;
      }

     private:
      std::vector<Storage> storage;
      size_t bitcnt = 0;
      size_t storage_size = 0;

      forceinline constexpr size_t unit_bits() const {
         return sizeof(Storage) * 8;
      }

      forceinline constexpr size_t unit_index(const size_t& index) const {
         return index >> ctz(unit_bits());
      }

      forceinline constexpr size_t unit_local_index(const size_t& index) const {
         return index & ((0x1ULL << ctz(unit_bits())) - 1);
      }
   };
} // namespace exotic_hashing::support
