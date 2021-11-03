#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "include/convenience/builtins.hpp"
#include "src/support/fixed_datasets.hpp"
#include "src/support/string_datasets.hpp"

namespace dataset {
   template<class T>
   static void deduplicate_and_sort(std::vector<T>& vec) {
      std::sort(vec.begin(), vec.end());
      vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
      vec.shrink_to_fit();
   }

   enum class ID
   {
      SEQUENTIAL = 0,
      GAPPED_10 = 1,
      UNIFORM = 2,
      FB = 3,
      OSM = 4,
      WIKI = 5,
      NORMAL = 6,
      EMAILS = 7,
   };

   inline std::string name(ID id) {
      switch (id) {
         case ID::SEQUENTIAL:
            return "seq";
         case ID::GAPPED_10:
            return "gap_10";
         case ID::UNIFORM:
            return "uniform";
         case ID::NORMAL:
            return "normal";
         case ID::FB:
            return "fb";
         case ID::OSM:
            return "osm";
         case ID::WIKI:
            return "wiki";
         case ID::EMAILS:
            return "emails";
      }
      return "unnamed";
   };

   template<class Data = std::uint64_t>
   std::vector<Data> load_cached(ID id, size_t dataset_size) {
      static std::random_device rd;
      static std::default_random_engine rng(rd());

      // cache generated & sampled datasets to speed up repeated benchmarks
      static std::unordered_map<ID, std::unordered_map<size_t, std::vector<Data>>> datasets;

      // cache datasets loaded from file to avoid expensive reload operations
      static std::vector<Data> ds_fb, ds_osm, ds_wiki;
      static std::vector<std::string> ds_email;

      // return cached (if available)
      const auto id_it = datasets.find(id);
      if (id_it != datasets.end()) {
         const auto ds_it = id_it->second.find(dataset_size);
         if (ds_it != id_it->second.end())
            return ds_it->second;
      }

      // generate (or random sample) in appropriate size
      std::vector<Data> ds;
      ds.resize(dataset_size);

      if constexpr (std::is_integral<Data>::value) {
         switch (id) {
            case ID::SEQUENTIAL: {
               for (size_t i = 0; i < ds.size(); i++)
                  ds[i] = i + 20000;
               break;
            }
            case ID::GAPPED_10: {
               std::uniform_int_distribution<size_t> dist(0, 99999);
               for (size_t i = 0, num = 0; i < ds.size(); i++) {
                  do
                     num++;
                  while (dist(rng) < 10000);
                  ds[i] = num;
               }
               break;
            }
            case ID::UNIFORM: {
               std::uniform_int_distribution<Data> dist(0, std::numeric_limits<Data>::max() - 1);
               for (size_t i = 0; i < ds.size(); i++)
                  ds[i] = dist(rng);
               break;
            }
            case ID::NORMAL: {
               const auto mean = static_cast<double>(std::numeric_limits<Data>::max()) / 2.0;
               const auto variance = static_cast<double>(ds.size());
               std::normal_distribution<> dist(mean, variance);
               for (size_t i = 0; i < ds.size(); i++)
                  // cutoff after 3*variance
                  ds[i] = std::max(mean - 3 * variance, std::min(mean + 3 * variance, dist(rng)));
            }
            case ID::FB: {
               if (ds_fb.empty()) {
                  ds_fb = load_fixed_size<Data>("data/fb_200M_uint64");
                  std::shuffle(ds_fb.begin(), ds_fb.end(), rng);
               }

               // sampling this way is only valid since ds_fb is shuffled!
               for (size_t i = 0; i < ds_fb.size() && i < ds.size(); i++)
                  ds[i] = ds_fb[i];
               break;
            }
            case ID::OSM: {
               if (ds_osm.empty()) {
                  ds_osm = load_fixed_size<Data>("data/osm_cellids_200M_uint64");
                  std::shuffle(ds_osm.begin(), ds_osm.end(), rng);
               }

               // sampling this way is only valid since ds_osm is shuffled!
               for (size_t i = 0; i < ds_osm.size() && i < ds.size(); i++)
                  ds[i] = ds_osm[i];
               break;
            }
            case ID::WIKI: {
               if (ds_wiki.empty()) {
                  ds_wiki = load_fixed_size<Data>("data/wiki_ts_200M_uint64");
                  std::shuffle(ds_wiki.begin(), ds_wiki.end(), rng);
               }

               // sampling this way is only valid since ds_wiki is shuffled!
               for (size_t i = 0; i < ds_wiki.size() && i < ds.size(); i++)
                  ds[i] = ds_wiki[i];
               break;
            }
            default:
               throw std::runtime_error("invalid datastet id " +
                                        std::to_string(static_cast<std::underlying_type<ID>::type>(id)) +
                                        " or data type <Data>");
         }
      } else if constexpr (std::is_same<Data, std::string>::value) {
         switch (id) {
            case ID::EMAILS: {
               if (ds_email.empty()) {
                  std::cout << "(1)" << std::endl;
                  ds_email = load_strings("data/emails.txt");
                  std::cout << "(2)" << std::endl;
                  std::shuffle(ds_email.begin(), ds_email.end(), rng);
                  std::cout << "(3)" << std::endl;
               }

               std::cout << "(4) " << ds_email.size() << ", " << ds.size() << std::endl;
               // sampling this way is only valid since ds_email is shuffled!
               for (size_t i = 0; i < ds_email.size() && i < ds.size(); i++)
                  ds[i] = ds_email[i];
               std::cout << "(5)" << std::endl;
               break;
            }

            default:
               throw std::runtime_error("invalid datastet id " +
                                        std::to_string(static_cast<std::underlying_type<ID>::type>(id)) +
                                        " or data type <Data>");
         }
      }

      // deduplicate, sort before caching to avoid additional work in the future
      deduplicate_and_sort(ds);

      // cache dataset for future use
      const auto it = datasets.find(id);
      if (it == datasets.end()) {
         std::unordered_map<size_t, std::vector<Data>> map;
         map.insert({dataset_size, ds});
         datasets.insert({id, map});
      } else {
         it->second.insert({dataset_size, ds});
      }

      return ds;
   }
}; // namespace dataset
