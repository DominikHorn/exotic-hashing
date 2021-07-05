#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

#include <exotic_hashing.hpp>
#include <hashtable.hpp>
#include <sdsl/suffix_arrays.hpp>
#include <benchmark/benchmark.h>

#include "../include/convenience/builtins.hpp"
#include "../include/convenience/tidy.hpp"

template<class Hashfn, class Data>
auto __BM_build = [](benchmark::State& state, const std::vector<Data> dataset, const std::string dataset_name) {
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
auto __BM_throughput = [](benchmark::State& state, const std::vector<Data> dataset, const std::string dataset_name) {
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

template<class Hashfn, class Data>
auto __BM_chained = [](benchmark::State& state, const std::vector<Data> dataset, const std::string dataset_name) {
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

   const Hashfn hashfn(dataset);
   const size_t ht_size = dataset->size(); // load factor 1
   hashtable::Chained<Data, std::uint64_t, 4, Hashfn, Clamp> ht(ht_size, hashfn);

   for (const auto key : dataset) {
      ht.insert(key, key - 1);
   }

   for (auto _ : state) {
      for (const auto& key : dataset) {
         const auto payload = ht.lookup(key);
         benchmark::DoNotOptimize(payload);
      }
   }

   // TODO: collect excess space consumption statistics
   const auto lookup_stats = ht.lookup_statistics(*dataset);
   for (const auto stat : lookup_stats) {
      state.counters[stat.first] = std::stoi(stat.second);
   }
   state.counters["hashtable_slots"] = ht_size;
   state.counters["hashfn_bytes"] = hashfn.byte_size();
   state.counters["num_elements"] = dataset.size();

   state.SetLabel(Hashfn::name() + "@" + dataset_name);
   state.SetItemsProcessed(dataset.size() * state.iterations());
   state.SetBytesProcessed(dataset.size() * sizeof(Data) * state.iterations());
};

#define BM_SPACE_VS_PROBE(Hashfn, dataset, dataset_name)                                                            \
   benchmark::RegisterBenchmark("build", __BM_build<Hashfn, decltype(dataset)::value_type>, dataset, dataset_name); \
   benchmark::RegisterBenchmark("throughput", __BM_throughput<Hashfn, decltype(dataset)::value_type>, dataset,      \
                                dataset_name);                                                                      \
   //benchmark::RegisterBenchmark("chained", __BM_chained<Hashfn, decltype(dataset)::value_type>, dataset, dataset_name);

int main(int argc, char** argv) {
   using Data = std::uint64_t;

   sdsl::csa_wt<> fm_index;
   sdsl::construct_im(fm_index, "mississippi!", 1);
   std::cout << "'si' occurs " << sdsl::count(fm_index, "si") << " times.\n";
   sdsl::store_to_file(fm_index, "fm_index-file.sdsl");
   std::ofstream out("fm_index-file.sdsl.html");
   sdsl::write_structure<sdsl::HTML_FORMAT>(fm_index, out);

   // // Test different dataset sizes
   // for (const size_t dataset_size : {100000, 1000000, 10000000, 100000000, 200000000}) {
   //    std::vector<Data> dataset(dataset_size, 0);

   //    // uniform random numbers dataset
   //    {
   //       std::unordered_set<Data> seen;
   //       std::default_random_engine rng_gen;
   //       std::uniform_int_distribution<Data> dist(0, static_cast<size_t>(0x1) << 50);
   //       for (size_t i = 0; i < dataset_size; i++) {
   //          const Data rand_num = dist(rng_gen);
   //          if (seen.contains(rand_num))
   //             continue;

   //          dataset[i] = rand_num;
   //          seen.insert(rand_num);
   //       }
   //    }
   //    BM_SPACE_VS_PROBE(exotic_hashing::DoNothingHash<Data>, dataset, "uniform");
   //    BM_SPACE_VS_PROBE(exotic_hashing::RankHash<Data>, dataset, "uniform");
   //    BM_SPACE_VS_PROBE(exotic_hashing::RecSplit<Data>, dataset, "uniform");

   //    // sequential dataset
   //    {
   //       const size_t start_offset = 10000;
   //       for (size_t i = 0; i < dataset_size; i++) {
   //          dataset[i] = i + start_offset;
   //       }
   //    }
   //    BM_SPACE_VS_PROBE(exotic_hashing::DoNothingHash<Data>, dataset, "seqential");
   //    BM_SPACE_VS_PROBE(exotic_hashing::RankHash<Data>, dataset, "sequential");
   //    BM_SPACE_VS_PROBE(exotic_hashing::RecSplit<Data>, dataset, "sequential");
   // }

   // benchmark::Initialize(&argc, argv);
   // benchmark::RunSpecifiedBenchmarks();
   // benchmark::Shutdown();
}
