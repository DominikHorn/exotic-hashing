#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <queue>
#include <random>
#include <sdsl/bit_vector_il.hpp>
#include <sdsl/io.hpp>
#include <sdsl/rrr_vector.hpp>
#include <sdsl/vectors.hpp>
#include <stack>
#include <string>
#include <tuple>
#include <vector>

#include <emmintrin.h>
#include <mmintrin.h>

#include <hashing.hpp>

#include "include/omphf/mwhc.hpp"
#include "include/support/bitvector.hpp"

// order is important
#include "../convenience/builtins.hpp"

namespace exotic_hashing {
   template<class Data, class Payload = std::uint64_t, class Hasher = support::Hasher<Data>,
            class HyperGraph = support::HyperGraph<Data, Hasher>>
   class CompressedSFMWHC;

   template<class Data, class Payload = std::uint64_t, class Hasher = support::Hasher<Data>,
            class HyperGraph = support::HyperGraph<Data, Hasher>>
   class SFMWHC {
      hashing::reduction::FastModulo<std::uint64_t> mod_N{1};
      Hasher hasher;

      std::vector<Payload> vertex_values;

      static forceinline size_t vertices_count(const size_t& dataset_size, const long double& overalloc = 1.23) {
         return std::ceil(overalloc * dataset_size);
      }

     public:
      SFMWHC() noexcept {};

      template<class KeyIt, class PayloadIt>
      SFMWHC(const KeyIt& keys_begin, const KeyIt& keys_end, const PayloadIt& payloads_begin) {
         construct(keys_begin, keys_end, payloads_begin);
      }

      explicit SFMWHC(const std::vector<Data>& keys, const std::vector<Payload>& payloads)
         : SFMWHC(keys.begin(), keys.end(), payloads.begin()) {}

      template<class KeyIt, class PayloadIt>
      void construct(const KeyIt& keys_begin, const KeyIt& keys_end, const PayloadIt& payloads_begin) {
         const auto n = vertices_count(std::distance(keys_begin, keys_end));
         mod_N = decltype(mod_N)(n);
         hasher = decltype(hasher)(n);
         vertex_values = decltype(vertex_values)(n, 0);

         // Find suitable peel order
         std::vector<size_t> peel_order;
         while (peel_order.empty()) {
            // 1. Generate random Hypergraph
            hasher = Hasher(mod_N.N);
            HyperGraph g(keys_begin, keys_end, hasher, mod_N.N);

            // 2. Peel (i.e., check for acyclicity)
            peel_order = g.peel();
         }

         // 3. Assign values to vertices depending on reverse peel order
         assert(peel_order.size() == static_cast<size_t>(std::distance(keys_begin, keys_end)));
         support::Bitvector<> settable(mod_N.N, true);
         for (size_t i = peel_order.size() - 1; i >= 0 && i < peel_order.size(); i--) {
            // get next edge
            const auto edge_ind = peel_order[i];

            // get key & payload for this edge
            const auto key = *(keys_begin + edge_ind);
            const auto payload = *(payloads_begin + edge_ind);

            // lookup vertices of this edge
            const auto [h0, h1, h2] = hasher(key);
            assert(settable[h0] || settable[h1] || settable[h2]);

            // compute vertex assign value. Handle the case that there are collisions, i.e. same h value twice
            const auto v0 = vertex_values[h0], v1 = vertex_values[h1], v2 = vertex_values[h2];
            size_t current_val = v0;
            if (h1 != h0)
               current_val ^= v1;
            if (h2 != h0 && h2 != h1)
               current_val ^= v2;
            const Payload x = payload ^ current_val;
            assert((current_val ^ x) == payload);

            // assign vertex values
            bool assigned = false;
            if (settable[h0]) {
               settable[h0] = false;
               vertex_values[h0] = x;
               assigned = true;
            }

            if (settable[h1]) {
               settable[h1] = false;
               vertex_values[h1] = assigned ? vertex_values[h1] : x;
               assigned = true;
            }

            if (settable[h2]) {
               settable[h2] = false;
               vertex_values[h2] = assigned ? vertex_values[h2] : x;
            }

            assert(!settable[h0]);
            assert(!settable[h1]);
            assert(!settable[h2]);

            assert(this->operator()(key) == payload);
         }
      }

      static std::string name() {
         return "SFMWHC";
      }

      forceinline size_t operator()(const Data& key) const {
         const auto [h0, h1, h2] = hasher(key);
         size_t hash = vertex_values[h0];
         if (likely(h1 != h0))
            hash ^= vertex_values[h1];
         if (likely(h2 != h1 && h2 != h0))
            hash ^= vertex_values[h2];
         return hash;
      }

      size_t byte_size() const {
         return sizeof(hasher) + sizeof(mod_N) + sizeof(decltype(vertex_values)) +
            sizeof(size_t) * vertex_values.size();
      }

      friend CompressedSFMWHC<Data, Payload, Hasher, HyperGraph>;
   };

   template<class Data, class Payload, class Hasher, class HyperGraph>
   class CompressedSFMWHC {
      using _SFMWHC = SFMWHC<Data, Payload, Hasher, HyperGraph>;

      hashing::reduction::FastModulo<std::uint64_t> mod_N{1};
      Hasher hasher;
      sdsl::int_vector<> vertex_values;

     public:
      CompressedSFMWHC() noexcept {};

      template<class KeyIt, class PayloadIt>
      CompressedSFMWHC(const KeyIt& keys_begin, const KeyIt& keys_end, const PayloadIt& payloads_begin) {
         construct(keys_begin, keys_end, payloads_begin);
      }

      explicit CompressedSFMWHC(const std::vector<Data>& dataset, const std::vector<Payload>& payloads)
         : CompressedSFMWHC(dataset.begin(), dataset.end(), payloads.begin()) {
         assert(dataset.size() == payloads.size());
      }

      template<class KeyIt, class PayloadIt>
      void construct(const KeyIt& keys_begin, const KeyIt& keys_end, const PayloadIt& payloads_begin) {
         // generate mwhc
         const _SFMWHC mwhc(keys_begin, keys_end, payloads_begin);

         // copy unchanged fields
         hasher = mwhc.hasher;
         mod_N = mwhc.mod_N;

         // helper to decide whether or not a value is set
         const auto is_set = [&](const auto& val) { return val != 0 && val != mod_N.N; };

         // copy & compress vertex values. Bit compression seems to be most efficient
         // since vertex_values are all < N
         sdsl::int_vector<> vec(mwhc.vertex_values.size(), 0);
         assert(vec.size() == mwhc.vertex_values.size());
         for (size_t i = 0; i < vec.size(); i++) {
            const auto val = mwhc.vertex_values[i];
            vec[i] = is_set(val) * val;
         }
         sdsl::util::bit_compress(vec);
         vertex_values = vec;
      }

      static std::string name() {
         return "CompressedSFMWHC";
      }

      forceinline size_t operator()(const Data& key) const {
         const auto [h0, h1, h2] = hasher(key);
         size_t hash = vertex_values[h0];
         if (likely(h1 != h0))
            hash ^= vertex_values[h1];
         if (likely(h2 != h1 && h2 != h0))
            hash ^= vertex_values[h2];
         return hash;
      }

      size_t byte_size() const {
         return sizeof(hasher) + sizeof(mod_N) + sdsl::size_in_bytes(vertex_values);
      }
   };
} // namespace exotic_hashing
