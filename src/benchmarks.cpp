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

template<class Hashfn, class Data>
void BM_build(benchmark::State& state, std::vector<Data> dataset, const std::string& dataset_name) {
   for (auto _ : state) {
      const Hashfn hashfn(dataset);
      benchmark::DoNotOptimize(hashfn);
   }

   const Hashfn hashfn(dataset);
   state.counters["hashfn_bytes"] = hashfn.byte_size();
   state.counters["num_elements"] = dataset.size();

   state.SetLabel(Hashfn::name() + "@" + dataset_name);
   state.SetItemsProcessed(dataset.size() * state.iterations());
   state.SetBytesProcessed(dataset.size() * sizeof(Data) * state.iterations());
};

template<class Hashfn, class Data>
void BM_throughput(benchmark::State& state, const std::vector<Data> dataset, const std::string& dataset_name) {
   const Hashfn hashfn(dataset);
   for (auto _ : state) {
      for (const auto key : dataset) {
         const auto hash = hashfn(key);
         benchmark::DoNotOptimize(hash);
      }
   }

   // Avoid overhead in loop -> measure again
   state.counters["hashfn_bytes"] = hashfn.byte_size();
   state.counters["num_elements"] = dataset.size();

   state.SetLabel(Hashfn::name() + "@" + dataset_name);
   state.SetItemsProcessed(dataset.size() * state.iterations());
   state.SetBytesProcessed(dataset.size() * sizeof(Data) * state.iterations());
};

#define BM_SPACE_VS_PROBE(Hashfn, dataset, dataset_name)                                                          \
   benchmark::RegisterBenchmark("build", BM_build<Hashfn, decltype(dataset)::value_type>, dataset, dataset_name); \
   benchmark::RegisterBenchmark("throughput", BM_throughput<Hashfn, decltype(dataset)::value_type>, dataset,      \
                                dataset_name);                                                                    \
   //benchmark::RegisterBenchmark("chained", BM_chained<Hashfn, decltype(dataset)::value_type>, dataset, dataset_name);

int main(int argc, char** argv) {
   using Data = std::uint64_t;
   using DoNothingHash = exotic_hashing::DoNothingHash<Data>;
   using RankHash = exotic_hashing::RankHash<Data>;
   using RecSplit = exotic_hashing::RecSplit<Data>;
   using CompactTrie = exotic_hashing::CompactTrie<Data, exotic_hashing::FixedBitConverter<Data>>;
   using SimpleHollowTrie = exotic_hashing::SimpleHollowTrie<Data, exotic_hashing::FixedBitConverter<Data>>;
   using HollowTrie = exotic_hashing::HollowTrie<Data, exotic_hashing::FixedBitConverter<Data>>;
   using MWHC = exotic_hashing::MWHC<Data>;
   using CompressedMWHC = exotic_hashing::CompressedMWHC<Data>;
   using CompactedMWHC = exotic_hashing::CompactedMWHC<Data>;

   std::random_device r;
   std::default_random_engine rng_gen(r());

   // Benchmark for different dataset sizes
   for (const size_t dataset_size : {100000, 1000000, 10000000 /*, 100000000, 200000000*/}) {
      std::vector<Data> dataset(dataset_size, 0);

      // uniform random numbers dataset
      {
         std::unordered_set<Data> seen;
         std::uniform_int_distribution<Data> dist(0, static_cast<size_t>(0x1) << 50);
         for (size_t i = 0; i < dataset_size; i++) {
            const Data rand_num = dist(rng_gen);
            if (seen.contains(rand_num))
               continue;

            dataset[i] = rand_num;
            seen.insert(rand_num);
         }
      }
      BM_SPACE_VS_PROBE(DoNothingHash, dataset, "uniform")
      BM_SPACE_VS_PROBE(RankHash, dataset, "uniform")
      BM_SPACE_VS_PROBE(RecSplit, dataset, "uniform")
      BM_SPACE_VS_PROBE(CompactTrie, dataset, "uniform");
      BM_SPACE_VS_PROBE(SimpleHollowTrie, dataset, "uniform");
      BM_SPACE_VS_PROBE(HollowTrie, dataset, "uniform");
      BM_SPACE_VS_PROBE(MWHC, dataset, "uniform");
      BM_SPACE_VS_PROBE(CompressedMWHC, dataset, "uniform");
      BM_SPACE_VS_PROBE(CompactedMWHC, dataset, "uniform");

      // sequential dataset
      {
         const size_t start_offset = 10000;
         for (size_t i = 0; i < dataset_size; i++) {
            dataset[i] = i + start_offset;
         }
      }
      BM_SPACE_VS_PROBE(DoNothingHash, dataset, "seqential")
      BM_SPACE_VS_PROBE(RankHash, dataset, "sequential")
      BM_SPACE_VS_PROBE(RecSplit, dataset, "sequential")
      BM_SPACE_VS_PROBE(CompactTrie, dataset, "sequential")
      BM_SPACE_VS_PROBE(SimpleHollowTrie, dataset, "sequential")
      BM_SPACE_VS_PROBE(HollowTrie, dataset, "sequential");
      BM_SPACE_VS_PROBE(MWHC, dataset, "sequential");
      BM_SPACE_VS_PROBE(CompressedMWHC, dataset, "sequential");
      BM_SPACE_VS_PROBE(CompactedMWHC, dataset, "sequential");
   }

   benchmark::Initialize(&argc, argv);
   benchmark::RunSpecifiedBenchmarks();
   benchmark::Shutdown();
}
