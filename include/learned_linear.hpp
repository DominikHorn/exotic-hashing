#pragma once

#include <cstdint>
#include <sdsl/bit_vector_il.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/io.hpp>
#include <sdsl/util.hpp>
#include <string>

#include "include/convenience/builtins.hpp"

namespace exotic_hashing {
   template<class Data>
   class LearnedLinear {
      std::uint32_t neg_intercept = 0;
      sdsl::bit_vector_il<> bitvec;
      sdsl::bit_vector_il<>::rank_1_type rank;

      forceinline size_t rank_index(const Data& key) const {
         // slope is always 1 since we don't want to produce collisions
         return key - neg_intercept;
      }

     public:
      template<class It>
      LearnedLinear(It begin, It end) {
         const size_t size = std::distance(begin, end);

         // We have to sort to enable rank computation bellow
         std::sort(begin, end);

         // Compute dataset properties
         const auto min = *begin;
         const auto max = *(end - 1);
         const size_t scale = (max - min + 1);

         // Set intercept
         neg_intercept = min;

         // build rank bitvector
         sdsl::bit_vector bv(scale, 0);
         for (size_t i = 0; i < size; i++) {
            const auto key = *(begin + i);
            const auto ind = rank_index(key);

            bv[ind] = true;
         }
         bitvec = bv;
         sdsl::util::init_support(rank, &bitvec);
      }

      explicit LearnedLinear(std::vector<Data> dataset) : LearnedLinear(dataset.begin(), dataset.end()) {}

      forceinline size_t operator()(const Data& key) const {
         const size_t ind = rank_index(key);
         const size_t res = rank(ind);

         return res;
      }

      forceinline size_t byte_size() const {
         return sdsl::size_in_bytes(bitvec) + sdsl::size_in_bytes(rank) + sizeof(decltype(neg_intercept));
      }

      static std::string name() {
         return "LearnedLinear";
      }
   };
} // namespace exotic_hashing
