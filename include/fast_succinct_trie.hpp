#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <fst.hpp>

#include "include/convenience/builtins.hpp"

namespace exotic_hashing {
   /**
    * Lightweight wrapper around FST:
    * https://github.com/christophanneser/FST
    */
   template<class Key>
   struct FastSuccinctTrie {
      /**
       * Builds a new fast succinct trie from a dataset
       */
      explicit FastSuccinctTrie(const std::vector<Key>& dataset) {
         // find min and max elemnts. Assume dataset is not sorted!
         const auto min_it = std::min_element(dataset.begin(), dataset.end());
         if (min_it < dataset.end())
            _min_key = *min_it;
         const auto max_it = std::max_element(dataset.begin(), dataset.end());
         if (max_it < dataset.end())
            _max_key = *max_it;

         // convert keys to string. IMPORTANT: they must be sorted for fst to work
         converted_keys.reserve(dataset.size());
         for (const auto& key : dataset) {
            Key endian_swapped_word;
            if (std::is_same<Key, std::uint64_t>::value) {
               endian_swapped_word = __builtin_bswap64(key);
            } else if (std::is_same<Key, std::uint32_t>::value) {
               endian_swapped_word = __builtin_bswap32(key);
            } else {
               throw std::runtime_error(std::string("FST unsupported raw key type ") + typeid(Key).name());
            }

            converted_keys.emplace_back(std::string(reinterpret_cast<const char*>(&endian_swapped_word), sizeof(Key)));
         }
         std::sort(converted_keys.begin(), converted_keys.end());

         // construct fst. Note that values is never used during construction, hence it is simply left empty
         _fst = std::make_unique<fst::FST>(converted_keys);
      }

      forceinline size_t operator()(const Key& key) const {
         // deal with out of bounds keys here to prevent segfaults later
         if (key < _min_key || key > _max_key)
            return std::numeric_limits<size_t>::max();

         // convert key to string key
         std::string str_key;
         {
            Key endian_swapped_word;
            if (std::is_same<Key, std::uint64_t>::value) {
               endian_swapped_word = __builtin_bswap64(key);
            } else if (std::is_same<Key, std::uint32_t>::value) {
               endian_swapped_word = __builtin_bswap32(key);
            } else {
               throw std::runtime_error(std::string("FST unsupported raw key type ") + typeid(Key).name());
            }

            str_key = std::string(reinterpret_cast<const char*>(&endian_swapped_word), sizeof(Key));
         }

         // obtain index, i.e., rank, from FST
         const auto iter = _fst->moveToKeyGreaterThan(str_key, true);
         if (!iter.isValid())
            return std::numeric_limits<size_t>::max();
         return iter.getValue();
      }

      static std::string name() {
         return "FastSuccinctTrie";
      }

      size_t byte_size() const {
         size_t base_size = sizeof(FastSuccinctTrie<Key>) + _fst->getMemoryUsage();

         // ignore converted keys byte usage, since this additional overhead does not exist
         // in real world implementations.
         return base_size - sizeof(std::vector<std::string>);
      }

     private:
      std::unique_ptr<fst::FST> _fst;
      Key _min_key, _max_key;

      // FST internally stores a pointer to the string keys
      // which is required during lookup. Therefore, we must
      // keep the string keys!
      std::vector<std::string> converted_keys;
   };
}; // namespace exotic_hashing
