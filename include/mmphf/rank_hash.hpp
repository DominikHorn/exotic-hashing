#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "../convenience/builtins.hpp"

namespace exotic_hashing {
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
} // namespace exotic_hashing