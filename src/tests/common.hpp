#pragma once

#include <random>
#include <vector>

#include <gtest/gtest.h>

namespace tests::common {
   template<class T, size_t GapSize = 10, class RandomEngine>
   static std::vector<T> gapped_dataset(size_t size, RandomEngine& rng_gen) {
      std::uniform_int_distribution<size_t> dist(0, 100);
      std::vector<T> dataset;
      for (T d = 0; dataset.size() < size; d++)
         if (dist(rng_gen) < GapSize)
            dataset.push_back(d);

      return dataset;
   }

   template<class T, class MMPHF>
   static void run_is_monotone_minimal_perfect() {
      // we do want predictable random results, hence the fixed seeds
      size_t dataset_size = 1000;
      for (const auto seed : {0, 1, 13, 42, 1337}) {
         // random source based on seed
         std::default_random_engine rng_gen(seed);

         // load dataset
         auto dataset = gapped_dataset<T>(dataset_size, rng_gen);

         // increase dataset size for next iteration
         dataset_size += dataset_size - 1;

         // random insert order (algorithm must catch this!)
         std::vector<T> shuffled_dataset = dataset;
         std::shuffle(shuffled_dataset.begin(), shuffled_dataset.end(), rng_gen);

         // build compact trie on shuffled dataset
         MMPHF mmphf(shuffled_dataset);

         // Monotone minimal perfect is achieved iff mmphf exactly matches rank_D(d) for all d \in D
         for (size_t i = 0; i < dataset.size(); i++)
            EXPECT_EQ(mmphf(dataset[i]), i);
      }
   }
} // namespace tests::common
