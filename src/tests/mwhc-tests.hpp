#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

// mwhc(d) = rank_D(d) (i.e., mwhc is monotone minimal perfect hash)
TEST(MWHC, IsOrderPreservingMinimalPerfect) {
   using Data = std::uint64_t;

   // we do want predictable random results, hence the fixed seeds
   size_t dataset_size = 10000;
   for (const auto seed : {0, 1, 42, 1337}) {
      std::default_random_engine rng_gen(seed);

      // generate dataset
      std::uniform_int_distribution<size_t> dist(0, 100);
      std::vector<Data> dataset;
      for (Data d = 0; dataset.size() < dataset_size; d++)
         if (dist(rng_gen) < 10)
            dataset.push_back(d);

      // random order (to test order preserving)
      {
         std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

         for (size_t i = dataset.size() - 1; i > 0; i--)
            std::swap(dataset[i], dataset[dist(rng_gen) % (i + 1)]);
      }
      assert(dataset.size() == dataset_size);
      dataset_size += dataset_size - 1;

      exotic_hashing::MWHC<Data> mwhc(dataset);

      // Order preserving minimal perfect test
      for (size_t i = 0; i < dataset.size(); i++)
         EXPECT_EQ(mwhc(dataset[i]), i);
   }
}
