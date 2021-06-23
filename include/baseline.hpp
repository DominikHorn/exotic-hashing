#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <hashtable.hpp>
#include <benchmark/benchmark.h>

#include "convenience/builtins.hpp"
#include "convenience/tidy.hpp"

// import (therefore effectively export) recsplit code
#include "recsplit/recsplit.hpp"

namespace exotic_hashing {
   template<class Data>
   struct DoNothingHash {
      DoNothingHash(const std::vector<Data>& d) {
         UNUSED(d);
      }

      static std::string name() {
         return "DoNothingHash";
      }

      constexpr forceinline std::uint64_t operator()(const Data& key) const {
         return key;
      }

      size_t byte_size() const {
         return 0;
      };
   };

   template<class Data>
   struct RankHash {
      RankHash(const std::vector<Data>& d) : dataset(d) {
         // sort the dataset
         std::sort(dataset.begin(), dataset.end());
      }

      static std::string name() {
         return "RankHash";
      }

      constexpr forceinline std::uint64_t operator()(const Data& key) const {
         // primitively compute rank of key by:
         // 1. binary searching it in the sorted dataset
         const auto iter = std::lower_bound(dataset.begin(), dataset.end(), key);
         // 2. returning the found index (computed based on returned iter)
         return std::distance(dataset.begin(), iter);
      }

      size_t byte_size() const {
         return dataset.size() * sizeof(Data) + sizeof(std::vector<Data>);
      };

     private:
      std::vector<Data> dataset;
   };
} // namespace exotic_hashing
