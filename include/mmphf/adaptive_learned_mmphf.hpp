#pragma once

#include <algorithm>
#include <sdsl/vectors.hpp>
#include <string>
#include <vector>

#include "../convenience/builtins.hpp"
#include "../convenience/tidy.hpp"
#include "../support/bitconverter.hpp"
#include "../support/clustering.hpp"
#include "../support/elias_fano_list.hpp"
#include "../support/support.hpp"
#include "hollow_trie.hpp"
#include "learned_linear.hpp"

namespace exotic_hashing {
   template<class Data>
   class AdaptiveLearnedMMPHF {
      // // TODO(dominik): play with the idea of using different building blocks,
      // // e.g., merge highly non-dense regions and put them in MWHC instead to save space!
      // // Decide based on region's size, e.g., via threshold. Small region -> merge & put in MWHC (?)
      // enum BuildingBlock
      // {
      //    MWHC,
      //    LearnedLinear
      // };

      /// root sorts incoming keys into one of the ranges that is then served by a leaf model
      support::EliasFanoList<Data> delimiters{};

      /// each leaf is a mmphf over a region (e.g., a dense cluster) of the dataset
      std::vector<LearnedLinear<Data>> leafs{};

      /// to obtain a global mmphf over the entire data, it is necessary to
      /// store the min rank of each region.
      support::EliasFanoList<size_t> region_offsets{};

      /**
       * Computes the required density threshold to match a certain bits per
       * key for LearnedLinear models
       */
      constexpr forceinline double learned_linear_density_threshold(double bits_per_key) {
         // bits_per_key >= (8 * sizeof(LearnedLinear) / |X|) + 1/density(X)
         // -> neglecting the first term (assumption: large |X|), we obtain
         // density(X) >= 1/bits_per_key
         return 1.0 / bits_per_key;
      }

     public:
      explicit AdaptiveLearnedMMPHF(std::vector<Data> data) {
         // ensure data is sorted before we start
         std::sort(data.begin(), data.end());

         // 1. cluster based on density metric
         const auto threshold = learned_linear_density_threshold(5);
         const auto clusters = support::cluster(data.begin(), data.end(), threshold);
         const size_t clusters_count = clusters.size() - 1; // due to representation as iterators

         // 2. build one leaf model for each region
         std::vector<size_t> rs(clusters_count, 0);
         for (size_t i = 1; i < clusters.size(); i++) {
            const auto a = clusters[i - 1];
            const auto b = clusters[i];

            if (i < clusters_count)
               rs[i] = std::distance(a, b) + rs[i - 1];
            leafs.emplace_back(a, b);
         }
         region_offsets = decltype(region_offsets)(rs.begin(), rs.end());

         // 3. build hollow trie on cluster delimiters
         std::vector<Data> d(clusters_count - 1, 0);
         for (size_t i = 1; i < clusters.size() - 1; i++)
            d[i - 1] = *clusters[i];
         delimiters = decltype(delimiters)(d.begin(), d.end());
      }

      forceinline size_t operator()(const Data& key) const {
         const auto lb = support::lower_bound(0, delimiters.size(), key, delimiters);
         const size_t region_ind = lb + ((lb < delimiters.size() && delimiters[lb] == key) & 0x1);

         const size_t offset = region_offsets[region_ind];
         const size_t local_rank = leafs[region_ind](key);

         return local_rank + offset;
      }

      forceinline size_t byte_size() const {
         size_t res = region_offsets.byte_size() + delimiters.byte_size();
         for (const auto& leaf : leafs)
            res += leaf.byte_size();

         return res;
      }

      static std::string name() {
         return "AdaptiveLearnedMMPHF";
      }
   };
} // namespace exotic_hashing
