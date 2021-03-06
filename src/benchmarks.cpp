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

#include "support/datasets.hpp"
#include "support/probing_set.hpp"

// Order important
#include "../include/convenience/builtins.hpp"

using Data = std::uint64_t;
const std::vector<std::int64_t> dataset_sizes{1'000, 100'000, 1'000'000, 10'000'000, 200'000'000};
const std::vector<std::int64_t> datasets{static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::SEQUENTIAL),
                                         static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::GAPPED_10),
                                         static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::UNIFORM),
                                         static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::BOOKS),
                                         static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::FB),
                                         static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::NORMAL),
                                         static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::OSM),
                                         static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::WIKI)};
const std::vector<std::int64_t> probe_distributions{
   static_cast<std::underlying_type_t<dataset::ProbingDistribution>>(dataset::ProbingDistribution::UNIFORM),
   static_cast<std::underlying_type_t<dataset::ProbingDistribution>>(dataset::ProbingDistribution::EXPONENTIAL)};

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
   const auto probing_dist = static_cast<dataset::ProbingDistribution>(state.range(2));
   const auto probing_set = dataset::generate_probing_set(dataset, probing_dist);

   // build hashfn
   const auto hashfn = Hashfn(dataset);

   size_t i = 0;
   for (auto _ : state) {
      // get next lookup element
      while (unlikely(i >= probing_set.size()))
         i -= probing_set.size();
      const auto element = probing_set[i++];

      // hash element
      const auto hash = hashfn(element);
      benchmark::DoNotOptimize(hash);

      // prevent interleaved execution
      full_memory_barrier();
   }

   // set counters (don't do this in inner loop to avoid tainting results)
   state.counters["hashfn_bytes"] = hashfn.byte_size();
   state.counters["hashfn_bits_per_key"] = 8. * hashfn.byte_size() / dataset.size();
   state.counters["dataset_elem_count"] = dataset.size();
   state.counters["dataset_bytes"] = (sizeof(decltype(dataset)::value_type) * dataset.size());
   state.SetLabel(Hashfn::name() + ":" + dataset::name(did) + ":" + dataset::name(probing_dist));
};

#define BM(Hashfn)                                                                         \
   BENCHMARK_TEMPLATE(PresortedBuildTime, Hashfn)->ArgsProduct({dataset_sizes, datasets}); \
   BENCHMARK_TEMPLATE(UnorderedBuildTime, Hashfn)->ArgsProduct({dataset_sizes, datasets}); \
   BENCHMARK_TEMPLATE(LookupTime, Hashfn)                                                  \
      ->ArgsProduct({dataset_sizes, datasets, probe_distributions})                        \
      ->Iterations(50000000);

using DoNothingHash = exotic_hashing::DoNothingHash<Data>;
BM(DoNothingHash);
using RankHash = exotic_hashing::RankHash<Data>;
BM(RankHash);
using CompressedRankHash = exotic_hashing::CompressedRankHash<Data>;
BM(CompressedRankHash);
using MapOMPHF = exotic_hashing::MapOMPHF<Data>;
BM(MapOMPHF);

using RecSplit = exotic_hashing::RecSplit<Data>;
BM(RecSplit);
using BBHash1 = exotic_hashing::BBHash<Data, std::ratio<1, 1>>;
BM(BBHash1);
using BBHash2 = exotic_hashing::BBHash<Data, std::ratio<2, 1>>;
BM(BBHash2);

using CompactTrie = exotic_hashing::CompactTrie<Data, exotic_hashing::support::FixedBitConverter<Data>>;
BM(CompactTrie);
using CompactedCompactTrie =
   exotic_hashing::CompactedCompactTrie<Data, exotic_hashing::support::FixedBitConverter<Data>>;
BM(CompactedCompactTrie);
using SimpleHollowTrie = exotic_hashing::SimpleHollowTrie<Data, exotic_hashing::support::FixedBitConverter<Data>>;
BM(SimpleHollowTrie);
using HollowTrie = exotic_hashing::HollowTrie<Data, exotic_hashing::support::FixedBitConverter<Data>>;
BM(HollowTrie);
using FST = exotic_hashing::FastSuccinctTrie<Data>;
BM(FST);

using BitMWHC = exotic_hashing::BitMWHC<Data>;
BM(BitMWHC);
using MWHC = exotic_hashing::MWHC<Data>;
BM(MWHC);
using CompressedMWHC = exotic_hashing::CompressedMWHC<Data>;
BM(CompressedMWHC);
using CompactedMWHC = exotic_hashing::CompactedMWHC<Data>;
BM(CompactedMWHC);

// using LearnedRank_RMI = exotic_hashing::LearnedRank<Data, learned_hashing::MonotoneRMIHash<Data, 1000000>>;
// BM(LearnedRank_RMI);
// using LearnedRank_RadixSpline = exotic_hashing::LearnedRank<Data, learned_hashing::RadixSplineHash<Data>>;
// BM(LearnedRank_RadixSpline);
// using CompressedLearnedRank_RMI =
//    exotic_hashing::CompressedLearnedRank<Data, learned_hashing::MonotoneRMIHash<Data, 1000000>>;
// BM(CompressedLearnedRank_RMI);
// using CompressedLearnedRank_RadixSpline =
//    exotic_hashing::CompressedLearnedRank<Data, learned_hashing::RadixSplineHash<Data>>;
// BM(CompressedLearnedRank_RadixSpline);
//
// using LearnedLinear = exotic_hashing::LearnedLinear<Data>;
// BENCHMARK_TEMPLATE(LookupTime, LearnedLinear)
//    ->ArgsProduct({dataset_sizes,
//                   {static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::SEQUENTIAL),
//                    static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::GAPPED_10)},
//                   probe_distributions});
// BENCHMARK_TEMPLATE(PresortedBuildTime, LearnedLinear)
//    ->ArgsProduct({dataset_sizes,
//                   {static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::SEQUENTIAL),
//                    static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::GAPPED_10)}});
// BENCHMARK_TEMPLATE(UnorderedBuildTime, LearnedLinear)
//    ->ArgsProduct({dataset_sizes,
//                   {static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::SEQUENTIAL),
//                    static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::GAPPED_10)}});
// using AdaptiveLearnedMMPHF = exotic_hashing::AdaptiveLearnedMMPHF<Data>;
// BM(AdaptiveLearnedMMPHF);

BENCHMARK_MAIN();
