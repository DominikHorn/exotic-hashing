#pragma once

#include <algorithm>
#include <cstdint>
#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

// mwhc(d) = rank_D(d) (i.e., mwhc is monotone minimal perfect hash)
TEST(MWHC, IsOrderPreservingMinimalPerfect) {
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

      // random order (to test order preserving)
      std::shuffle(dataset.begin(), dataset.end(), rng_gen);

      // build mwhc on dataset
      exotic_hashing::MWHC<Data> mwhc(dataset);

      // order preserving minimal perfect test
      for (size_t i = 0; i < dataset.size(); i++)
         EXPECT_EQ(mwhc(dataset[i]), i);
   }
}
