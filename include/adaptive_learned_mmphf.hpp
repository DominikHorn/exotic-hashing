#pragma once

#include "include/convenience/builtins.hpp"
#include "include/convenience/tidy.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace exotic_hashing {
   template<class Data>
   class AdaptiveLearnedMMPHF {
      /**
       * At each level, a different building block is recursively chosen based
       * on the underlying characteristics of the dataset chunk
       */
      enum BuildingBlock
      {
         MWHC,
         LearnedLinear,
         DoNothing // Viable, e.g., on empty buckets
      };

      struct DecisionEngine {
         template<class It>
         BuildingBlock recommend(It begin, It end) {
            const size_t X = std::distance(begin, end);

            if (X == 0)
               // Empty dataset implies doing nothing works best
               return DoNothing;

            const Data max_x = std::max_element(begin, end);
            const Data min_x = std::min_element(begin, end);

            const auto density = static_cast<double>(X) / static_cast<double>(max_x - min_x);
            if (density > 0.7)
               return LearnedLinear;

            // TODO(dominik): we will probably need a different fallback, e.g.,
            // for cases where X is really large (trie??)
            return MWHC;
         }
      };

     public:
      explicit AdaptiveLearnedMMPHF(const std::vector<Data>& data) {
         // TODO(dominik): implement
         UNUSED(data);
      }

      static std::string name() {
         return "AdaptiveLearnedMMPHF";
      }
   };
} // namespace exotic_hashing
