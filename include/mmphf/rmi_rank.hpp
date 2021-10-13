#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <learned_hashing.hpp>

#include "../convenience/builtins.hpp"
#include "../convenience/tidy.hpp"

namespace exotic_hashing {
   template<class Key>
   struct SequentialRangeLookup {
      SequentialRangeLookup() = default;

      template<class Predictor>
      SequentialRangeLookup(const std::vector<Key>& dataset, const Predictor& predictor) {
         UNUSED(dataset);
         UNUSED(predictor);
      }

      forceinline size_t byte_size() const {
         return 0;
      }

      forceinline size_t operator()(size_t pred_ind, Key searched, const std::vector<Key>& dataset) const {
         const auto last_ind = dataset.size() - 1;
         size_t actual_ind = pred_ind;

         while (actual_ind > 0 && dataset[actual_ind] > searched)
            actual_ind--;
         while (actual_ind < last_ind && dataset[actual_ind] < searched)
            actual_ind++;

         assert(actual_ind >= 0);
         assert(actual_ind <= last_ind);

#if NDEBUG == 0
   #warning "Using additional counters in SequentialRangeLookup implementation for debugging"
         // tricking the compiler like this should be illegal...
         auto self = (std::remove_const_t<std::remove_pointer_t<decltype(this)>>*) (this);
         self->total_error += actual_ind > pred_ind ? actual_ind - pred_ind : pred_ind - actual_ind;
         self->queries++;
#endif

         return actual_ind;
      }

#if NDEBUG == 0
      double avg_error() const {
         return static_cast<double>(total_error) / static_cast<double>(queries);
      }

     private:
      size_t total_error = 0, queries = 0;
#endif
   };

   template<class Key>
   struct ExponentialRangeLookup {
      ExponentialRangeLookup() = default;

      template<class Predictor>
      ExponentialRangeLookup(const std::vector<Key>& dataset, const Predictor& predictor) {
         UNUSED(dataset);
         UNUSED(predictor);
      }

      forceinline size_t byte_size() const {
         return 0;
      }

      forceinline size_t operator()(size_t pred_ind, Key searched, const std::vector<Key>& dataset) const {
         typename std::vector<Key>::const_iterator interval_start, interval_end;
         const auto dataset_size = dataset.size();

         size_t next_err = 1;
         if (dataset[pred_ind] < searched) {
            size_t err = 0;
            while (err + pred_ind < dataset_size && dataset[err + pred_ind] < searched) {
               err = next_err;
               next_err *= 2;
            }
            err = std::min(err, dataset_size - pred_ind);

            interval_start = dataset.begin() + pred_ind;
            // +1 since lower_bound searches up to *excluding*
            interval_end = dataset.begin() + pred_ind + err;
         } else {
            size_t err = 0;
            while (err < pred_ind && dataset[pred_ind - err] < searched) {
               err = next_err;
               next_err *= 2;
            }
            err = std::min(err, pred_ind);

            interval_start = dataset.begin() + pred_ind - err;
            // +1 since lower_bound searches up to *excluding*
            interval_end = dataset.begin() + pred_ind;
         }

         assert(interval_start >= dataset.begin());
         assert(interval_end >= interval_start);
         assert(interval_end <= dataset.end());

         const size_t actual_ind =
            std::distance(dataset.begin(), std::lower_bound(interval_start, interval_end, searched));

#if NDEBUG == 0
   #warning "Using additional counters in ExponentialRangeLookup implementation for debugging"
         // tricking the compiler like this should be illegal...
         auto self = (std::remove_const_t<std::remove_pointer_t<decltype(this)>>*) (this);
         self->total_error += actual_ind > pred_ind ? actual_ind - pred_ind : pred_ind - actual_ind;
         self->queries++;
#endif

         return actual_ind;
      }

#if NDEBUG == 0
      double avg_error() const {
         return static_cast<double>(total_error) / static_cast<double>(queries);
      }

     private:
      size_t total_error = 0, queries = 0;
#endif
   };

   template<class Key>
   struct BinaryRangeLookup {
      size_t max_error = 0;

      BinaryRangeLookup() = default;

      template<class Predictor>
      BinaryRangeLookup(const std::vector<Key>& dataset, const Predictor& predictor) {
         for (size_t i = 0; i < dataset.size(); i++) {
            const size_t pred = predictor(dataset[i]);
            max_error = std::max(max_error, pred >= i ? pred - i : i - pred);
         }
      }

      forceinline size_t byte_size() const {
         return sizeof(decltype(max_error));
      }

      forceinline size_t operator()(size_t pred_ind, Key searched, const std::vector<Key>& dataset) const {
         // compute interval bounds
         const auto interval_start = dataset.begin() + (pred_ind > max_error) * (pred_ind - max_error);
         // +1 since std::lower_bound searches up to excluding upper bound
         const auto interval_end = dataset.begin() + std::min(pred_ind + max_error, dataset.size() - 1) + 1;

         assert(interval_start >= dataset.begin());
         assert(interval_end >= interval_start);
         assert(interval_end <= dataset.end());

         const size_t actual_ind =
            std::distance(dataset.begin(), std::lower_bound(interval_start, interval_end, searched));

#if NDEBUG == 0
   #warning "Using additional counters in BinaryRangeLookup implementation for debugging"
         // tricking the compiler like this should be illegal...
         auto self = (std::remove_const_t<std::remove_pointer_t<decltype(this)>>*) (this);
         self->total_error += actual_ind > pred_ind ? actual_ind - pred_ind : pred_ind - actual_ind;
         self->queries++;
#endif

         return actual_ind;
      }

#if NDEBUG == 0
      double avg_error() const {
         return static_cast<double>(total_error) / static_cast<double>(queries);
      }

     private:
      size_t total_error = 0, queries = 0;
#endif
   };

   template<class Data, size_t SecondLevelModelCount = 1000000, class LastLevelSearch = ExponentialRangeLookup<Data>>
   class RMIRank {
      std::vector<Data> dataset;
      learned_hashing::RMIHash<Data, SecondLevelModelCount> rmi{};
      LastLevelSearch lls;

     public:
      explicit RMIRank(const std::vector<Data>& d) : dataset(d) {
         // ensure dataset is sorted
         std::sort(dataset.begin(), dataset.end());

         // median of dataset
         const size_t median = (dataset.size() / 2) + (dataset.size() & 0x1);

         // build rmi on full dataset
         rmi = decltype(rmi)(dataset.begin(), dataset.end(), median);

         // omit every second element, deleting junk and ensuring the final dataset
         // vector is minimal, i.e., does not waste any additional space
         for (size_t i = 1, j = 2; j < dataset.size(); i++, j += 2)
            dataset[i] = dataset[j];
         dataset.erase(dataset.begin() + median, dataset.end());
         dataset.resize(dataset.size());
         assert(dataset.size() == median);

         // train lls using reduced dataset
         lls = LastLevelSearch(dataset, rmi);
      }

      static std::string name() {
         return "RMIRank";
      }

      forceinline size_t operator()(const Data& key) const {
         // predict using RMI
         const auto pred_ind = rmi(key);

         // Last level search to find actual index
         const auto actual_ind = lls(pred_ind, key, dataset);

         if (unlikely(actual_ind == dataset.size()))
            return 2 * actual_ind - 1;
         return 2 * actual_ind - (dataset[actual_ind] == key ? 0 : 1);
      }

      size_t byte_size() const {
         return dataset.size() * sizeof(Data) + sizeof(std::vector<Data>) + rmi.byte_size() + lls.byte_size();
      };

#if NDEBUG == 0
      /// average model prediction error experienced thus far
      size_t avg_lls_error() const {
         return lls.avg_error();
      }
#endif
   };
} // namespace exotic_hashing
