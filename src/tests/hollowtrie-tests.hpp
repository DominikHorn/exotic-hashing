#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// hollow_trie(d) = rank_D(d) (i.e., hollow_trie is monotone minimal perfect hash)
TEST(HollowTrie, IsMonotoneMinimalPerfect) {
   using Data = std::uint64_t;

   // we do want predictable random results, hence the fixed seeds
   size_t dataset_size = 50;
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

      exotic_hashing::SimpleHollowTrie<Data, exotic_hashing::FixedBitConverter<Data>> simple_hollow_trie(
         shuffled_dataset);
      exotic_hashing::HollowTrie<Data, exotic_hashing::FixedBitConverter<Data>> hollow_trie(shuffled_dataset);

      // Debug code (visualization)
      std::ofstream out;
      out.open("tmp/simple_hollow_trie_" + std::to_string(seed) + ".tex");
      simple_hollow_trie.print_tex(out);
      out.close();
      out.open("tmp/hollow_trie_" + std::to_string(seed) + ".tex");
      hollow_trie.print_tex(out);
      out.close();
      exotic_hashing::CompactTrie<Data, exotic_hashing::FixedBitConverter<Data>> compact_trie(shuffled_dataset);
      out.open("tmp/compact_trie_" + std::to_string(seed) + ".tex");
      compact_trie.print_tex(out);
      out.close();

      // Monotone minimal perfect is achieved iff trie exactly returns rank_D(d) for d \in D
      for (size_t i = 0; i < dataset.size(); i++) {
         EXPECT_EQ(simple_hollow_trie(dataset[i]), i);
         EXPECT_EQ(hollow_trie(dataset[i]), i);
      }
   }
}
