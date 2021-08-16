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
   for (const auto seed : {0}) {
      std::default_random_engine rng_gen(seed);

      // generate dataset
      std::uniform_int_distribution<size_t> dist(0, 100);
      std::vector<Data> dataset;
      for (Data d = 0; dataset.size() < dataset_size; d++)
         if (dist(rng_gen) < 10)
            dataset.push_back(d);

      dataset_size += dataset_size - 1;

      // random insert order (algorithm must catch this!)
      std::vector<Data> shuffled_dataset = dataset;
      {
         std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

         for (size_t i = shuffled_dataset.size() - 1; i > 0; i--)
            std::swap(shuffled_dataset[i], shuffled_dataset[dist(rng_gen) % (i + 1)]);
      }
      assert(shuffled_dataset.size() == dataset.size());

      exotic_hashing::FastSuccinctTrie<Data> fst(shuffled_dataset);

      // Monotone minimal perfect is achieved iff trie exactly returns rank_D(d) for d \in D
      for (size_t i = 0; i < dataset.size(); i++)
         EXPECT_EQ(fst(dataset[i]), i);
   }
}
