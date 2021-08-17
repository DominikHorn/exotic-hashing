#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// hollow_trie(d) = rank_D(d) (i.e., hollow_trie is monotone minimal perfect hash)
TEST(HollowTrie, SimpleIsMonotoneMinimalPerfect) {
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

      // bigger dataset on next loop
      dataset_size += dataset_size - 1;

      // random insert order (algorithm must catch this!)
      std::vector<Data> shuffled_dataset = dataset;
      std::shuffle(shuffled_dataset.begin(), shuffled_dataset.end(), rng_gen);

      // build hollow trie
      exotic_hashing::SimpleHollowTrie<Data, exotic_hashing::FixedBitConverter<Data>> simple_hollow_trie(
         shuffled_dataset);

      // Debug code (visualization)
      // std::ofstream out;
      // out.open("tmp/simple_hollow_trie_" + std::to_string(seed) + ".tex");
      // simple_hollow_trie.print_tex(out);
      // out.close();

      // monotone minimal perfect is achieved iff trie exactly returns rank_D(d) for d \in D
      for (size_t i = 0; i < dataset.size(); i++)
         EXPECT_EQ(simple_hollow_trie(dataset[i]), i);
   }
}

TEST(HollowTrie, CompressedIsMonotoneMinimalPerfect) {
   using Data = std::uint64_t;

   // we do want predictable random results, hence the fixed seeds
   size_t dataset_size = 10000;
   for (const auto seed : {0, 1, 13, 42, 1337}) {
      std::default_random_engine rng_gen(seed);

      // generate dataset
      std::uniform_int_distribution<size_t> dist(0, 100);
      std::vector<Data> dataset;
      for (Data d = 0; dataset.size() < dataset_size; d++)
         if (dist(rng_gen) < 10)
            dataset.push_back(d);

      // bigger dataset on next loop
      dataset_size += dataset_size - 1;

      // random insert order (algorithm must catch this!)
      std::vector<Data> shuffled_dataset = dataset;
      std::shuffle(shuffled_dataset.begin(), shuffled_dataset.end(), rng_gen);

      // build hollow trie
      exotic_hashing::HollowTrie<Data, exotic_hashing::FixedBitConverter<Data>> hollow_trie(shuffled_dataset);

      // Debug code (visualization)
      // std::ofstream out;
      // out.open("tmp/hollow_trie_" + std::to_string(seed) + ".tex");
      // hollow_trie.print_tex(out);
      // out.close();

      // monotone minimal perfect is achieved iff trie exactly returns rank_D(d) for d \in D
      for (size_t i = 0; i < dataset.size(); i++)
         EXPECT_EQ(hollow_trie(dataset[i]), i);
   }
}
