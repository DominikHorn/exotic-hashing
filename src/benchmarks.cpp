#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "../include/convenience/builtins.hpp"
#include <benchmark/benchmark.h>

template<class Hashfn, class Data>
auto __BM_space_vs_probe = [](benchmark::State& state, const std::vector<Data>* dataset) {
   // Vector for tracking collisions
   const size_t ht_size = dataset->size(); // load factor 1

   const auto clamp = [](const auto& val) {
      if (unlikely(val < 0))
         return 0;
      if (unlikely(val >= ht_size))
         return ht_size - 1;
      return val;
   };

   for (const auto& key : *dataset) {
      const auto hash = hashfn(key);
      // TODO: insert into hashtable
   }

   for (auto _ : state) {
      for (const auto& key : *dataset) {
         const auto hash = hashfn(key);
         benchmark::DoNotOptimize(hash);
         // TODO: probe from hashtable
      }
   }

   // TODO: collect excess space consumption statistics

   state.SetLabel(Hashfn::name());
   state.SetItemsProcessed(dataset->size());
   state.SetBytesProcessed(dataset->size() * sizeof(Data));
};

#define BM_SPACE_VS_PROBE(Hashfn, dataset) \
   benchmark::RegisterBenchmark("space_vs_probe", __BM_space_vs_probe<Hashfn, decltype(dataset)::value_type>, &dataset);

int main(int argc, char** argv) {
   using Data = std::uint64_t;

   // TODO: load dataset from disk instead
   // generate uniform random numbers dataset with 200 million elements
   const size_t dataset_size = 200000000;
   std::vector<Data> dataset;
   std::default_random_engine rng_gen;
   std::uniform_int_distribution<Data> dist(0, static_cast<size_t>(0x1) << 50);
   dataset.resize(dataset_size);
   for (size_t i = 0; i < dataset_size; i++)
      dataset[i] = dist(rng_gen);

   // Benchmark hash functions
   //BM_SPACE_VS_PROBE(exotic_hashing::FstHash, dataset);

   benchmark::Initialize(&argc, argv);
   benchmark::RunSpecifiedBenchmarks();
   benchmark::Shutdown();
}
