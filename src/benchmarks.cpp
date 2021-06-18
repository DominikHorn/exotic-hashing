#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <hashtable.hpp>
#include <benchmark/benchmark.h>

#include "../include/convenience/builtins.hpp"

template<class Data>
struct RankHash {
   RankHash(const std::vector<Data>& d) : dataset(d) {
      // sort the dataset
      std::sort(dataset.begin(), dataset.end());
   }

   static std::string name() {
      return "RankHash";
   }

   constexpr forceinline std::uint64_t operator()(const Data& key) const {
      // primitively compute rank of key by:
      // 1. binary searching it in the sorted dataset
      const auto iter = std::lower_bound(dataset.begin(), dataset.end(), key);
      // 2. returning the found index (computed based on returned iter)
      return (iter - dataset.begin());
   }

  private:
   std::vector<Data> dataset;
};

template<class Data>
struct Clamp {
   Clamp(const size_t& N) : N(N) {}

   static std::string name() {
      return "clamp";
   }

   constexpr forceinline size_t operator()(const Data& hash) const {
      if (unlikely(hash < 0))
         return 0;
      if (unlikely(hash >= N))
         return N - 1;
      return hash;
   }

  private:
   const size_t N;
};

template<class Hashfn, class Data>
auto __BM_chained = [](benchmark::State& state, const std::vector<Data>* dataset) {
   const size_t ht_size = dataset->size(); // load factor 1
   hashtable::Chained<std::uint64_t, std::uint64_t, 4, Hashfn, Clamp<std::uint64_t>> ht(ht_size, Hashfn(*dataset));
   std::cout << "ht initialized" << std::endl;

   for (const auto& key : *dataset) {
      ht.insert(key, key - 1);
   }
   std::cout << "ht filled with data" << std::endl;

   for (auto _ : state) {
      std::cout << "start of run" << std::endl;
      for (const auto& key : *dataset) {
         const auto payload = ht.lookup(key);
         benchmark::DoNotOptimize(payload);
      }
      std::cout << "end of run" << std::endl;
   }

   // TODO: collect excess space consumption statistics

   state.SetLabel(Hashfn::name());
   state.SetItemsProcessed(dataset->size());
   state.SetBytesProcessed(dataset->size() * sizeof(Data));
};

#define BM_SPACE_VS_PROBE(Hashfn, dataset) \
   benchmark::RegisterBenchmark("chained", __BM_chained<Hashfn, decltype(dataset)::value_type>, &dataset);

int main(int argc, char** argv) {
   using Data = std::uint64_t;

   // TODO: load dataset from disk instead
   // generate uniform random numbers dataset with 2 million elements
   const size_t dataset_size = 2000000;
   std::vector<Data> dataset;
   std::default_random_engine rng_gen;
   std::uniform_int_distribution<Data> dist(0, static_cast<size_t>(0x1) << 50);
   dataset.resize(dataset_size);
   for (size_t i = 0; i < dataset_size; i++)
      dataset[i] = dist(rng_gen);

   // Benchmark hash functions
   BM_SPACE_VS_PROBE(RankHash<Data>, dataset);

   benchmark::Initialize(&argc, argv);
   benchmark::RunSpecifiedBenchmarks();
   benchmark::Shutdown();
}
