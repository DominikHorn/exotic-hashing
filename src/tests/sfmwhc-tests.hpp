#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>
#include <limits>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(SFMWHC, IsFunctionStorage) {
   using Key = std::uint64_t;
   using Payload = std::uint64_t;

   // we do want predictable random results, hence the fixed seeds
   size_t dataset_size = 100;
   for (const auto seed : {0, 1, 13, 42, 1337}) {
      // random source based on seed
      std::default_random_engine rng_gen(seed);

      // generate dataset
      std::uniform_int_distribution<Key> key_dist(0, std::numeric_limits<Key>::max());
      std::vector<Key> keys(dataset_size, 0);
      for (size_t i = 0; i < keys.size(); i++)
         keys[i] = key_dist(rng_gen);

      // generate payloads
      std::vector<std::uint64_t> payloads(dataset_size, 0);
      std::uniform_int_distribution<Payload> payload_dist(0, std::numeric_limits<Key>::max() - 1);
      for (size_t i = 0; i < keys.size(); i++)
         payloads[i] = payload_dist(rng_gen);

      // build hashfn
      exotic_hashing::SFMWHC<Key> h(keys, payloads);

      // test whether function was stored successfully
      for (size_t i = keys.size() - 1; i >= 0 && i < keys.size(); i--)
         EXPECT_TRUE(h(keys[i]) == payloads[i]);

      // increase dataset size for next iteration
      dataset_size += dataset_size - 1;
   }
}
