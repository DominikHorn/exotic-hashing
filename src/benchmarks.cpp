#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

#include <exotic_hashing.hpp>
#include <sdsl/suffix_arrays.hpp>
#include <benchmark/benchmark.h>

#include "../include/bitconverter.hpp"
#include "../include/convenience/builtins.hpp"
#include "../include/convenience/tidy.hpp"

#include "support/datasets.hpp"

using Data = std::uint64_t;
const std::vector<std::int64_t> dataset_sizes{100, 10000, 1000000, 10000000};
const std::vector<std::int64_t> datasets{dataset::ID::SEQUENTIAL, dataset::ID::UNIFORM, dataset::ID::FB,
                                         /*dataset::ID::OSM, dataset::ID::WIKI*/};

template<class Hashfn>
static void PresortedBuildTime(benchmark::State& state) {
   const auto dataset_size = state.range(0);
   const auto did = static_cast<dataset::ID>(state.range(1));
   auto dataset = dataset::load_cached(did, dataset_size);

   if (dataset.empty()) {
      // otherwise google benchmark produces an error ;(
      for (auto _ : state) {}
      return;
   }

   // assume dataset loading will sort the data for use
   assert(std::is_sorted(dataset.begin(), dataset.end()));

   for (auto _ : state) {
      const auto hashfn = Hashfn(dataset);
      benchmark::DoNotOptimize(hashfn);
   }

   // set counters (don't do this in inner loop to avoid tainting results)
   const Hashfn hashfn(dataset);
   state.SetItemsProcessed(static_cast<int64_t>(dataset.size()));
   state.counters["hashfn_bytes"] = hashfn.byte_size();
   state.counters["hashfn_bits_per_key"] = 8. * hashfn.byte_size() / dataset.size();
   state.counters["dataset_elem_count"] = dataset.size();
   state.counters["dataset_bytes"] = (sizeof(decltype(dataset)::value_type) * dataset.size());
   state.SetLabel(Hashfn::name() + ":" + dataset::name(did));
};

template<class Hashfn>
static void UnorderedBuildTime(benchmark::State& state) {
   const auto dataset_size = state.range(0);
   const auto did = static_cast<dataset::ID>(state.range(1));
   auto dataset = dataset::load_cached(did, dataset_size);

   if (dataset.empty()) {
      // otherwise google benchmark produces an error ;(
      for (auto _ : state) {}
      return;
   }

   std::random_device rd;
   std::default_random_engine rng(rd());
   auto shuffled_dataset = dataset;
   std::shuffle(shuffled_dataset.begin(), shuffled_dataset.end(), rng);

   for (auto _ : state) {
      const auto hashfn = Hashfn(shuffled_dataset);
      benchmark::DoNotOptimize(hashfn);
   }

   // set counters (don't do this in inner loop to avoid tainting results)
   const Hashfn hashfn(dataset);
   state.SetItemsProcessed(static_cast<int64_t>(shuffled_dataset.size()));
   state.counters["hashfn_bytes"] = hashfn.byte_size();
   state.counters["hashfn_bits_per_key"] = 8. * hashfn.byte_size() / dataset.size();
   state.counters["dataset_elem_count"] = dataset.size();
   state.counters["dataset_bytes"] = (sizeof(decltype(dataset)::value_type) * dataset.size());
   state.SetLabel(Hashfn::name() + ":" + dataset::name(did));
};

template<class Hashfn>
static void LookupTime(benchmark::State& state) {
   const auto dataset_size = state.range(0);
   const auto did = static_cast<dataset::ID>(state.range(1));
   auto dataset = dataset::load_cached(did, dataset_size);

   if (dataset.empty()) {
      // otherwise google benchmark produces an error ;(
      for (auto _ : state) {}
      return;
   }

   // assume dataset loading will sort the data for use
   assert(std::is_sorted(dataset.begin(), dataset.end()));

   // probe in random order to limit caching effects
   std::random_device rd;
   std::default_random_engine rng(rd());
   auto shuffled_dataset = dataset;
   std::shuffle(shuffled_dataset.begin(), shuffled_dataset.end(), rng);

   // build hashfn
   const auto hashfn = Hashfn(dataset);

   size_t i = 0;
   for (auto _ : state) {
      // get next lookup element
      while (unlikely(i >= shuffled_dataset.size()))
         i -= shuffled_dataset.size();
      const auto element = shuffled_dataset[i++];

      // hash element
      const auto hash = hashfn(element);
      benchmark::DoNotOptimize(hash);
   }

   // set counters (don't do this in inner loop to avoid tainting results)
   state.counters["hashfn_bytes"] = hashfn.byte_size();
   state.counters["hashfn_bits_per_key"] = 8. * hashfn.byte_size() / dataset.size();
   state.counters["dataset_elem_count"] = dataset.size();
   state.counters["dataset_bytes"] = (sizeof(decltype(dataset)::value_type) * dataset.size());
   state.SetLabel(Hashfn::name() + ":" + dataset::name(did));
};

#define BM(Hashfn)                                                                         \
   BENCHMARK_TEMPLATE(PresortedBuildTime, Hashfn)->ArgsProduct({dataset_sizes, datasets}); \
   BENCHMARK_TEMPLATE(UnorderedBuildTime, Hashfn)->ArgsProduct({dataset_sizes, datasets}); \
   BENCHMARK_TEMPLATE(LookupTime, Hashfn)->ArgsProduct({dataset_sizes, datasets});

using DoNothingHash = exotic_hashing::DoNothingHash<Data>;
BM(DoNothingHash);
using RankHash = exotic_hashing::RankHash<Data>;
BM(RankHash);
using RecSplit = exotic_hashing::RecSplit<Data>;
BM(RecSplit);
using CompactTrie = exotic_hashing::CompactTrie<Data, exotic_hashing::support::FixedBitConverter<Data>>;
BM(CompactTrie);
using SimpleHollowTrie = exotic_hashing::SimpleHollowTrie<Data, exotic_hashing::support::FixedBitConverter<Data>>;
BM(SimpleHollowTrie);
using HollowTrie = exotic_hashing::HollowTrie<Data, exotic_hashing::support::FixedBitConverter<Data>>;
BM(HollowTrie);
using MWHC = exotic_hashing::MWHC<Data>;
BM(MWHC);
using CompressedMWHC = exotic_hashing::CompressedMWHC<Data>;
BM(CompressedMWHC);
using CompactedMWHC = exotic_hashing::CompactedMWHC<Data>;
BM(CompactedMWHC);
using FST = exotic_hashing::FastSuccinctTrie<Data>;
BM(FST);

BENCHMARK_MAIN();
