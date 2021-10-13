#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sdsl/bit_vectors.hpp>
#include <stdexcept>

#include "bitconverter.hpp"
#include "support.hpp"

namespace exotic_hashing::support {
   /**
   * Elias Fano Lists are able to store lists of n *non decreasing*
   * integers out of the universe [0...m) in n * (2+ ceil(log(m/n))) bits,
   * while providing O(1) access to any element stored in the list
   */
   template<class T, class BitConverter = support::FixedBitConverter<T>>
   class EliasFanoList {
      sdsl::bit_vector upper{0, false};
      sdsl::bit_vector lower{0, false};
      decltype(upper)::select_1_type u_select{};
      size_t l, n;

     public:
      /**
       * Creates an Elias Fano list from an *already sorted* input range
       * [begin, end). Implemented according to instructions from
       *
       * https://www.antoniomallia.it/sorted-integers-compression-with-elias-fano-encoding.html
       */
      template<class ForwardIt>
      EliasFanoList(const ForwardIt& begin, const ForwardIt& end) {
         // Gather general parameters
         n = std::distance(begin, end);
         if (n == 0)
            return;

         const T m = 1 + *std::max_element(begin, end);
         constexpr const size_t t_bits = 8 * sizeof(T);
         const size_t log_m = t_bits - clz(m);
         const size_t u = t_bits - clz(n);
         l = log_m - u;

         // Initialize bitvectors
         // each element contributes a single 1 bit (n)
         // there are 2^u bucket, each of which contributes a single 0 bit (2^u == 0x1 << u)
         upper = decltype(upper)(n + (0x1 << u), 0);
         lower = decltype(lower)(n * l, 0);

         // Initialize support variables
         size_t upper_ind = 0, lower_ind = 0;
         auto last_upper_bucket = 0x0LLU;
         const BitConverter bc;

         // Set bitvector values
         for (auto it = begin; it < end; it++) {
            const auto& elem = *it;
            const auto bitstream = bc(elem);

            // lower l bits go into lower stream
            for (size_t i = t_bits - l; i < t_bits; i++) {
               assert(lower_ind < lower.size());
               lower[lower_ind++] = bitstream[i];
            }

            // upper u bits determine the upper bucket. Each bucket's element
            // count is unary encoded
            auto bucket_ind = 0x0LLU;
            for (size_t i = t_bits - log_m; i < t_bits - l; i++)
               bucket_ind = (bucket_ind << 1) | (bitstream[i] & 0x1);
            upper_ind += bucket_ind - last_upper_bucket;
            assert(upper_ind < upper.size());
            last_upper_bucket = bucket_ind;

            upper[upper_ind++] = 0x1;
         }

         // Initialize select support
         sdsl::util::init_support(u_select, &upper);
      }
      T operator[](const size_t i) const {
         assert(n > 0);

         // upper bits == bucket index. Therefore count the amount of 0
         // preceeding the i-th 1 bit
         T res = u_select(i + 1) - i;

         // append lower bits
         const auto base_l_ind = l * i;
         for (size_t j = 0; j < l; j++)
            res = (res << 0x1) | (lower[base_l_ind + j] & 0x1);

         return res;
      }

      size_t size() const {
         return upper.size() / 2;
      }

      size_t byte_size() const {
         return sdsl::size_in_bytes(upper) + sdsl::size_in_bytes(lower) + sdsl::size_in_bytes(u_select) +
            sizeof(decltype(l));
      }
   };
} // namespace exotic_hashing::support
