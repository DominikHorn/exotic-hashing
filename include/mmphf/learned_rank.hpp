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
#include "../support/elias_fano_list.hpp"
#include "../support/support.hpp"

namespace exotic_hashing {
   template<class Key>
   struct SequentialRangeLookup {
      SequentialRangeLookup() = default;

      template<class Dataset, class Predictor>
      SequentialRangeLookup(const Dataset& dataset, const Predictor& predictor) {
         UNUSED(dataset);
         UNUSED(predictor);
      }

      forceinline size_t byte_size() const {
         return 0;
      }

      template<class Dataset>
      forceinline size_t operator()(size_t pred_ind, Key searched, const Dataset& dataset) const {
         const auto last_ind = dataset.size() - 1;
         size_t actual_ind = pred_ind;

         while (actual_ind > 0 && dataset[actual_ind] > searched)
            actual_ind--;
         while (actual_ind < last_ind && dataset[actual_ind] < searched)
            actual_ind++;

         assert(actual_ind >= 0);
         assert(actual_ind <= last_ind);

#if NDEBUG == 0
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

      template<class Dataset, class Predictor>
      ExponentialRangeLookup(const Dataset& dataset, const Predictor& predictor) {
         UNUSED(dataset);
         UNUSED(predictor);
      }

      forceinline size_t byte_size() const {
         return 0;
      }

      template<class Dataset>
      forceinline size_t operator()(size_t pred_ind, Key searched, const Dataset& dataset) const {
         // fast path correct predictions
         if (dataset[pred_ind] == searched)
            return pred_ind;

         const auto dataset_size = dataset.size();
         size_t interval_start = pred_ind, interval_end = pred_ind + 1;

         size_t err = 1;
         while (interval_start > 0 && dataset[interval_start] > searched) {
            interval_end = interval_start;
            interval_start -= std::min(err, interval_start);
            err *= 2;
         }
         err = 1;
         while (interval_end < dataset_size && dataset[interval_end] < searched) {
            interval_start = interval_end;
            interval_end += std::min(err, dataset_size - interval_end);
            err *= 2;
         }

         assert(interval_start >= 0);
         assert(interval_end >= interval_start);
         assert(interval_end <= dataset.size());

         const size_t actual_ind = support::lower_bound(interval_start, interval_end, searched, dataset);

#if NDEBUG == 0
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

      template<class Dataset, class Predictor>
      BinaryRangeLookup(const Dataset& dataset, const Predictor& predictor) {
         for (size_t i = 0; i < dataset.size(); i++) {
            const size_t pred = predictor(dataset[i]);
            max_error = std::max(max_error, pred >= i ? pred - i : i - pred);
         }
      }

      forceinline size_t byte_size() const {
         return sizeof(decltype(max_error));
      }

      template<class Dataset>
      forceinline size_t operator()(size_t pred_ind, Key searched, const Dataset& dataset) const {
         // compute interval bounds
         const auto interval_start = (pred_ind > max_error) * (pred_ind - max_error);
         // +1 since std::lower_bound searches up to excluding upper bound
         const auto interval_end = std::min(pred_ind + max_error, dataset.size() - 1) + 1;

         assert(interval_start >= 0);
         assert(interval_end >= interval_start);
         assert(interval_end <= dataset.size());

         const size_t actual_ind = support::lower_bound(interval_start, interval_end, searched, dataset);

#if NDEBUG == 0
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

   template<class Data, class Model = learned_hashing::RMIHash<Data, 1000000>,
            class LastLevelSearch = ExponentialRangeLookup<Data>>
   class LearnedRank {
      std::vector<Data> dataset;
      Model model{};
      LastLevelSearch lls;

     public:
      explicit LearnedRank(const std::vector<Data>& d) : dataset(d) {
         // nothing to do on empty data
         if (dataset.empty())
            return;

         // ensure dataset is sorted
         std::sort(dataset.begin(), dataset.end());

         // median of dataset (since is_sorted)
         const size_t half_size = dataset.size() / 2;

         // train on full data
         model.train(dataset.begin(), dataset.end(), half_size);

         // omit every second element, deleting junk and ensuring the final dataset
         // vector is minimal, i.e., does not waste any additional space
         size_t i = 0;
         for (size_t j = 1; j < dataset.size(); j += 2)
            dataset[i++] = dataset[j];
         assert(i == half_size);
         dataset.erase(dataset.begin() + half_size, dataset.end());
         dataset.resize(dataset.size());
         assert(dataset.size() == half_size);
         assert(std::is_sorted(dataset.begin(), dataset.end()));

         // train lls using reduced dataset
         lls = LastLevelSearch(dataset, model);
      }

      static std::string name() {
         return "LearnedRank<" + Model::name() + ">";
      }

      forceinline size_t operator()(const Data& key) const {
         // predict using RMI
         const auto pred_ind = model(key);

         // Last level search to find actual index
         const auto actual_ind = lls(pred_ind, key, dataset);

         assert(actual_ind == dataset.size() || dataset[actual_ind] >= key);
         assert(actual_ind == 0 || dataset[actual_ind - 1] < key);

         // edge case last element ('unlikely'?)
         if (unlikely(actual_ind == dataset.size()))
            return 2 * actual_ind;

         // all others
         return 2 * actual_ind + (dataset[actual_ind] == key) * 0x1;
      }

      size_t byte_size() const {
         return dataset.size() * sizeof(Data) + sizeof(std::vector<Data>) + model.byte_size() + lls.byte_size();
      };

#if NDEBUG == 0
      /// average model prediction error experienced thus far
      size_t avg_lls_error() const {
         return lls.avg_error();
      }
#endif
   };

   template<class Data, class Model = learned_hashing::RMIHash<Data, 1000000>,
            class LastLevelSearch = ExponentialRangeLookup<Data>>
   class CompressedLearnedRank {
      support::EliasFanoList<Data> efl{};
      Model model{};
      LastLevelSearch lls;

     public:
      explicit CompressedLearnedRank(std::vector<Data> dataset) {
         // nothing to do on empty data
         if (dataset.empty())
            return;

         // ensure dataset is sorted
         std::sort(dataset.begin(), dataset.end());

         // median of dataset (since is_sorted)
         const size_t half_size = dataset.size() / 2;

         // train on full data
         model.train(dataset.begin(), dataset.end(), half_size);

         // omit every second element, deleting junk and ensuring the final dataset
         // vector is minimal, i.e., does not waste any additional space
         size_t i = 0;
         for (size_t j = 1; j < dataset.size(); j += 2)
            dataset[i++] = dataset[j];
         assert(i == half_size);
         dataset.erase(dataset.begin() + half_size, dataset.end());
         dataset.resize(dataset.size());
         assert(dataset.size() == half_size);
         assert(std::is_sorted(dataset.begin(), dataset.end()));

         // train lls using reduced dataset
         lls = LastLevelSearch(dataset, model);

         // store dataset as elias fano monotone list
         efl = decltype(efl)(dataset.begin(), dataset.end());
      }

      static std::string name() {
         return "CompressedLearnedRank<" + Model::name() + ">";
      }

      forceinline size_t operator()(const Data& key) const {
         // predict using RMI
         const auto pred_ind = model(key);

         // Last level search to find actual index
         const auto actual_ind = lls(pred_ind, key, efl);

         assert(actual_ind == efl.size() || efl[actual_ind] >= key);
         assert(actual_ind == 0 || efl[actual_ind - 1] < key);

         // edge case last element ('unlikely'?)
         if (unlikely(actual_ind == efl.size()))
            return 2 * actual_ind;

         // all others
         return 2 * actual_ind + (efl[actual_ind] == key) * 0x1;
      }

      size_t byte_size() const {
         return efl.byte_size() + model.byte_size() + lls.byte_size();
      };

#if NDEBUG == 0
      /// average model prediction error experienced thus far
      size_t avg_lls_error() const {
         return lls.avg_error();
      }
#endif
   };
} // namespace exotic_hashing