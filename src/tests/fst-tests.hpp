#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

TEST(FastSuccinctTrie, IsMonotoneMinimalPerfect) {
   using Data = std::uint64_t;

   // we do want predictable random results, hence the fixed seeds
   size_t dataset_size = 1000;
   for (const auto seed : {0, 1, 13, 42, 1337}) {
      std::default_random_engine rng_gen(seed);

      // generate dataset
      std::uniform_int_distribution<size_t> dist(0, 100);
      std::vector<Data> dataset;
      for (Data d = 0; dataset.size() < dataset_size; d++)
         if (dist(rng_gen) < 10)
            dataset.push_back(d);

      // increase dataset size for next iteration
      dataset_size += dataset_size - 1;

      // random insert order (algorithm must catch this!)
      std::vector<Data> shuffled_dataset = dataset;
      std::shuffle(shuffled_dataset.begin(), shuffled_dataset.end(), rng_gen);

      // build fst on shuffled data
      exotic_hashing::FastSuccinctTrie<Data> fst(shuffled_dataset);

      // Monotone minimal perfect is achieved iff trie exactly returns rank_D(d) for d \in D
      for (size_t i = 0; i < dataset.size(); i++)
         EXPECT_EQ(fst(dataset[i]), i);
   }
}
