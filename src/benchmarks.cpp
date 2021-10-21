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

#include "../include/convenience/builtins.hpp"
#include "../include/convenience/tidy.hpp"

#include "support/datasets.hpp"
#include "support/probing_set.hpp"

using Data = std::uint64_t;
const std::vector<std::int64_t> dataset_sizes{1000000};
const std::vector<std::int64_t> datasets{static_cast<std::underlying_type_t<dataset::ID>>(dataset::ID::NORMAL)};
const std::vector<std::int64_t> probe_distributions{
   static_cast<std::underlying_type_t<dataset::ProbingDistribution>>(dataset::ProbingDistribution::UNIFORM)};

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

int main() {
   using Data = std::uint64_t;

   for (size_t dataset_size : {100000000}) {
      const auto dataset = dataset::load_cached<Data>(dataset::ID::WIKI, dataset_size);
      // std::cout << "============ DATASET SIZE: " << dataset_size << " ==============" << std::endl;

      for (double bits_per_key : {4., 8., 16., 32., 64.})
         const auto hashfn = exotic_hashing::AdaptiveLearnedMMPHF<Data>(dataset, bits_per_key);
   }

   return 0;
}
