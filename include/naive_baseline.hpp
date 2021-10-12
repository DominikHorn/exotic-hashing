#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "convenience/builtins.hpp"
#include "convenience/tidy.hpp"

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

   /**
    * Most basic minimal perfect hash function, i.e.,
    * mapping keys to their rank within the keyset.
    *
    * Meant as a space & speed baseline, i.e., slower than
    * this is only justified if space is also smaller.
    *
    * Using more space than this function is not desirable.
    */
   template<class Data>
   struct RankHash {
      explicit RankHash(const std::vector<Data>& d) : dataset(d) {
         // sort the dataset
         std::sort(dataset.begin(), dataset.end());

         // omit every second element, deleting junk and ensuring the final dataset
         // vector is minimal, i.e., does not waste any additional space
         for (size_t i = 1, j = 2; j < dataset.size(); i++, j += 2)
            dataset[i] = dataset[j];
         const size_t middle = (dataset.size() / 2) + (dataset.size() & 0x1);
         dataset.erase(dataset.begin() + middle, dataset.end());
         dataset.resize(dataset.size());
      }

      static std::string name() {
         return "RankHash";
      }

      constexpr forceinline size_t operator()(const Data& key) const {
         // primitively compute rank of key by:
         // 1. binary searching it in the sorted, compressed dataset
         const auto iter = std::lower_bound(dataset.begin(), dataset.end(), key);

         // 2. computing its rank based on the compressed keyset. If key does
         //    not match iter assume it was removed during compression and its
         //    index therefore is 2*iter_pos-1
         const size_t iter_pos = std::distance(dataset.begin(), iter);

         if (unlikely(iter == dataset.end()))
            return 2 * iter_pos - 1;
         return 2 * iter_pos - (*iter == key ? 0 : 1);
      }

      size_t byte_size() const {
         return dataset.size() * sizeof(Data) + sizeof(std::vector<Data>);
      };

     private:
      std::vector<Data> dataset;
   };

   /**
    * Naive order preserving mphf baseline using
    * as standard map container
    */
   template<class Data>
   struct MapOMPHF {
      explicit MapOMPHF(const std::vector<Data>& dataset) {
         for (size_t i = 0; i < dataset.size(); i++) {
            map[dataset[i]] = i;
         }
      }

      static std::string name() {
         return "MapOMPHF";
      }

      constexpr forceinline size_t operator()(const Data& key) const {
         return map.at(key);
      }

      size_t byte_size() const {
         size_t internal_map_size = 0;
         for (size_t i = 0; i < map.bucket_count(); i++)
            // This is a rough estimate and could vary from the actual size!
            internal_map_size += map.bucket_size(i) * (sizeof(Data) + sizeof(size_t)) + sizeof(void*);

         return sizeof(MapOMPHF<Data>) + internal_map_size;
      };

     private:
      std::unordered_map<Data, size_t> map;
   };
} // namespace exotic_hashing
