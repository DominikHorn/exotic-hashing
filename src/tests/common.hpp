#pragma once

#include "include/convenience/tidy.hpp"
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>

namespace tests::common {
   template<class T, size_t GapSize = 10, class RandomEngine>
   static std::vector<T> gapped_dataset(size_t size, RandomEngine& rng_gen) {
      std::uniform_int_distribution<size_t> dist(0, 100);
      std::vector<T> dataset;
      for (T d = 0; dataset.size() < size; d++)
         if (dist(rng_gen) < GapSize)
            dataset.push_back(d);

      return dataset;
   }

   /// h(x) perfect iff h(x) != h(y)
   struct TestIsPerfect {
      template<class HashFn, class T>
      void operator()(const HashFn& h, const std::vector<T>& monotone_order,
                      const std::vector<T>& insertion_order) const {
         // insertion order mustn't matter since monotone ordering wrt. < is absolute
         UNUSED(insertion_order);

         std::unordered_set<size_t> seen;
         for (size_t i = 0; i < monotone_order.size(); i++) {
            const auto index = h(monotone_order[i]);
            EXPECT_FALSE(seen.contains(index));
            seen.insert(index);
         }
      }
   };

   /// h(x) minimal iff |D| == N --> h(x) : [0, N-1]
   struct TestIsMinimal {
      template<class HashFn, class T>
      void operator()(const HashFn& h, const std::vector<T>& monotone_order,
                      const std::vector<T>& insertion_order) const {
         // insertion order mustn't matter since monotone ordering wrt. < is absolute
         UNUSED(insertion_order);

         for (size_t i = 0; i < monotone_order.size(); i++) {
            const auto index = h(monotone_order[i]);
            EXPECT_GE(index, 0);
            EXPECT_LT(index, monotone_order.size());
         }
      }
   };

   /// h(x) monotone iff x <= y --> h(x) <= h(y)
   struct TestIsMonotone {
      template<class HashFn, class T>
      void operator()(const HashFn& h, const std::vector<T>& sorted_order,
                      const std::vector<T>& insertion_order) const {
         // insertion order mustn't matter since monotone ordering wrt. < is absolute
         UNUSED(insertion_order);

         // monotony check is implementable in O(n) since we have sorted, i.e., monotone ordered data
         // otherwise we would require an O(n^2) check, i.e., test each element against each other element
         for (size_t i = 0; i + 1 < sorted_order.size(); i++)
            EXPECT_TRUE(h(sorted_order[i]) <= h(sorted_order[i + 1]));
      }
   };

   /// combined integration test: h(x) = rank(d) \forall d \in D (potentially to weak?)
   struct TestIsMMPHF {
      template<class HashFn, class T>
      void operator()(const HashFn& h, const std::vector<T>& sorted_order,
                      const std::vector<T>& insertion_order) const {
         // insertion order mustn't matter since monotone ordering wrt. < is absolute
         UNUSED(insertion_order);

         for (size_t i = 0; i < sorted_order.size(); i++)
            EXPECT_EQ(h(sorted_order[i]), i);
      }
   };

   /// test whether order is preserved
   struct TestIsOrderPreserving {
      template<class HashFn, class T>
      void operator()(const HashFn& h, const std::vector<T>& sorted_order,
                      const std::vector<T>& insertion_order) const {
         std::unordered_map<T, size_t> order;
         for (size_t i = 0; i < insertion_order.size(); i++)
            order[insertion_order[i]] = i;

         for (size_t i = 0; i < sorted_order.size(); i++) {
            for (size_t j = 0; j < sorted_order.size(); j++) {
               const auto orig_i = order[sorted_order[i]];
               const auto orig_j = order[sorted_order[j]];

               // iff elements had an original order it is preserved after hashing
               if (orig_i < orig_j)
                  EXPECT_TRUE(h(sorted_order[i]) < h(sorted_order[j]));
               else if (orig_i > orig_j)
                  EXPECT_TRUE(h(sorted_order[i]) > h(sorted_order[j]));
            }
         }
      }
   };

   template<class T, class HashFn, class TestFun>
   static void run_test(const TestFun& test_fun = TestFun()) {
      // we do want predictable random results, hence the fixed seeds
      size_t dataset_size = 100;
      for (const auto seed : {0, 1, 13, 42, 1337}) {
         // random source based on seed
         std::default_random_engine rng_gen(seed);

         // generate gapped dataset
         auto sorted_dataset = gapped_dataset<T>(dataset_size, rng_gen);

         // increase dataset size for next iteration
         dataset_size += dataset_size - 1;

         // random insert order (respective algorithm must acknowledge this)
         auto shuffled_dataset = sorted_dataset;
         std::shuffle(shuffled_dataset.begin(), shuffled_dataset.end(), rng_gen);
         HashFn h(shuffled_dataset);

         // execute the designated test
         test_fun(h, sorted_dataset, shuffled_dataset);
      }
   }
} // namespace tests::common
