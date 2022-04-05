#pragma once

#include <sdsl/bit_vector_il.hpp>
#include <sdsl/util.hpp>
#include <string>

#include "../omphf/mwhc.hpp"

/*
  // build rank bitvector
  sdsl::bit_vector bv(scale, 0);
  for (size_t i = 0; i < size; i++) {
     const auto key = *(begin + i);
     const auto ind = rank_index(key);

     assert(ind < scale);
     bv[ind] = true;
  }
  bitvec = bv;
  sdsl::util::init_support(rank, &bitvec);
*/

namespace exotic_hashing {
   template<class Data, class Hasher = support::Hasher<Data>, class HyperGraph = support::HyperGraph<Data, Hasher>>
   class RankedBitMWHC {
      BitMWHC<Data, Hasher, HyperGraph> mwhc;

      sdsl::bit_vector_il<> rank_bv;
      typename decltype(rank_bv)::rank_1_type rank{};

     public:
      RankedBitMWHC() noexcept {};

      template<class RandomIt>
      RankedBitMWHC(const RandomIt& begin, const RandomIt& end) {
         construct(begin, end);
      }

      template<class RandomIt>
      void construct(const RandomIt& begin, const RandomIt& end) {
         // construct mwhc
         mwhc.construct(begin, end);

         sdsl::bit_vector bv(std::distance(begin, end), 0);
         for (auto it = begin; it < end; it++) {
            const auto r = mwhc(*it);
            bv[r] = true;
         }
         rank_bv = bv;
         sdsl::util::init_support(rank, &rank_bv);
      }

      static std::string name() {
         return "RankedBitMWHC";
      }

      forceinline size_t operator()(const Data& key) const {
         const auto bit_i = mwhc(key);
         assert(bit_i < bitvec.size());
         return rank(bit_i);
      }

      size_t byte_size() const {
         return mwhc.byte_size() + sdsl::size_in_bytes(rank_bv) + sdsl::size_in_bytes(rank);
      }
   };
} // namespace exotic_hashing
