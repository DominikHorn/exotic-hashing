#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// compact_trie(d) = rank_D(d) (i.e., hollow_trie is monotone minimal perfect hash)
TEST(CompactTrie, IsMonotoneMinimalPerfect) {
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

      // build compact trie on shuffled dataset
      exotic_hashing::CompactTrie<Data, exotic_hashing::FixedBitConverter<Data>> compact_trie(shuffled_dataset);

      // // Debug code (visualization)
      // std::ofstream out;
      // out.open("tmp/compact_trie_" + std::to_string(seed) + ".tex");
      // compact_trie.print_tex(out);
      // out.close();
      // exotic_hashing::CompactTrie<Data, exotic_hashing::FixedBitConverter<Data>> compact_trie(shuffled_dataset);
      // out.open("tmp/compact_trie_" + std::to_string(seed) + ".tex");
      // compact_trie.print_tex(out);
      // out.close();

      // Monotone minimal perfect is achieved iff trie exactly returns rank_D(d) for d \in D
      for (size_t i = 0; i < dataset.size(); i++)
         EXPECT_EQ(compact_trie(dataset[i]), i);
   }
}
