/*
 * This is a lightweight wrapper for recsplit, reexporting symbols
 * from suxcpp (https://github.com/vigna/sux.git) to make them:
 * 1) usable within this codebase
 * 2) lightweight in the sense that most unused functionality was plainly removed
 *
 * Please see LICENSE.md for more information
 */

#include "../../convenience/builtins.hpp"
#include "src/RecSplit.hpp"

#include <string>
#include <vector>

namespace exotic_hashing {
   template<class Data, size_t BucketSize = 9, size_t LeafSize = 12,
            sux::util::AllocType AllocType = sux::util::AllocType::MALLOC>
   class RecSplit {
      template<class ForwardIt>
      static std::vector<std::string> to_string(const ForwardIt& begin, const ForwardIt& end) {
         std::vector<std::string> res;
         for (auto it = begin; it < end; it++)
            res.emplace_back(reinterpret_to_string(*it));

         return res;
      }

      template<class T>
      static std::string reinterpret_to_string(const T& v) {
         return {reinterpret_cast<const char*>(&v), sizeof(T)};
      }

      sux::function::RecSplit<LeafSize, AllocType> rs_;

     public:
      RecSplit() noexcept = default;

      template<class ForwardIt>
      RecSplit(const ForwardIt& begin, const ForwardIt& end) {
         construct(begin, end);
      }

      explicit RecSplit(const std::vector<Data>& v) : RecSplit(v.begin(), v.end()) {}

      template<class ForwardIt>
      void construct(const ForwardIt& begin, const ForwardIt& end) {
         rs_ = decltype(rs_)(to_string(begin, end), BucketSize);
      }

      static std::string name() {
         return "RecSplit_leaf" + std::to_string(LeafSize) + "_bucket" + std::to_string(BucketSize);
      }

      forceinline size_t operator()(const Data& key) const {
         return rs_.operator()(reinterpret_to_string(key));
      }

      size_t byte_size() const {
         return rs_._totalBitSize / 8;
      };
   };
}; // namespace exotic_hashing
