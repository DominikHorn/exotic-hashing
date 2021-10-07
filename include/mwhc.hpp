#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
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

#include "include/convenience/builtins.hpp"
#include "include/reduction.hpp"

namespace exotic_hashing {
   namespace support {
      template<class Data, class Hasher>
      class HyperGraph {
         class Vertex {
            /// implemented using modified XOR-trick. Original from:
            /// https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=6824443
            std::uint64_t data = 0;

            static constexpr forceinline size_t edges_width() {
               return 48;
            }

            static constexpr forceinline size_t edges_mask() {
               return (0x1LLU << edges_width()) - 1;
            }

           public:
            /// degree mustn't be larger than 255
            forceinline size_t degree() const {
               return data >> edges_width();
            }

            /// edges are represented as 48 bit integers
            forceinline void add_edge(const size_t edge) {
               // increment degree & xor edge ontop of data (add it)
               data = ((degree() + 1) << edges_width()) | ((data ^ edge) & edges_mask());
            }

            /// edges are represented as 48 bit integers
            forceinline void remove_edge(const size_t edge) {
               // decrement degree & xor edge ontop of data (remove it)
               data = ((degree() - 1) << edges_width()) | ((data ^ edge) & edges_mask());
            }

            /// edges are represented as 48 bit integers
            forceinline size_t retrieve_last() const {
               assert(degree() == 1);
               return data & edges_mask();
            }
         };

         std::vector<Vertex> vertices;

         const std::vector<Data>& dataset;
         const Hasher& hasher;

        public:
         HyperGraph(const std::vector<Data>& dataset, const Hasher& hasher, const size_t& N)
            : vertices(N), dataset(dataset), hasher(hasher) {
            // Construct random hypergraph using hasher
            for (size_t i = 0; i < dataset.size(); i++) {
               // TODO(dominik): remove these random cache accesses
               const auto [h0, h1, h2] = hasher(dataset[i]);
               vertices[h0].add_edge(i);
               vertices[h1].add_edge(i);
               vertices[h2].add_edge(i);
            }
         }

         /**
          * Peels the hypergraph as described Majewski, Bohdan S., et al. in "A
          * family of perfect hashing methods." The Computer Journal 39.6
          * (1996): 547-554.
          *
          * @return stack containing the acyclicity check traversal order
          *   (reversed since stack is FILO), or an empty stack if the hypegraph
          *   contained a cycle
          */
         std::stack<size_t> peel() {
            // TODO(dominik): implement cache oblivious peeling, i.e., remove random access to vertices in
            // vertex struct!
            std::stack<size_t> res;

            // 1. Recursively peel all vertices if possible (without recursion to avoid stack overflows)
            for (size_t vertex_index = 0; vertex_index < vertices.size(); vertex_index++) {
               std::queue<size_t> next_vertices;
               next_vertices.push(vertex_index);

               while (!next_vertices.empty()) {
                  // TODO(dominik): eliminiate (if possible) this random cache access
                  auto& vertex = vertices[next_vertices.front()];
                  next_vertices.pop();

                  // Only vertices with degree 1 are peelable
                  if (vertex.degree() != 1)
                     continue;

                  // Peel last edge from this vertex
                  const auto edge = vertex.retrieve_last();
                  const auto [h0, h1, h2] = hasher(dataset[edge]);
                  // TODO(dominik): eliminiate (if ) these random cache accesses
                  vertices[h0].remove_edge(edge);
                  vertices[h1].remove_edge(edge);
                  vertices[h2].remove_edge(edge);

                  // Keep track of edge removal order
                  res.push(edge);

                  // Attempt to peel adjacent vertices next
                  next_vertices.push(h0);
                  next_vertices.push(h1);
                  next_vertices.push(h2);
               }
            }

            // 2. Check if there are any edges left, i.e., any vertex has degree > 0. If so, the
            //    acyclicity check has failed
            for (const auto& vertex : vertices)
               if (vertex.degree() > 0)
                  return {};

            return res;
         }
      };

      template<class Data>
      class Hasher {
         const hashing::AquaHash<Data> hashfn;
         hashing::reduction::FastModulo<Data> reducer;
         std::uint64_t s0, s1, s2;

         void seed_hashfns() {}

         forceinline __m128i make_seed(const std::uint64_t lower, const std::uint64_t upper = 0) const {
            return _mm_setr_epi8(static_cast<char>((upper >> 56) & 0xFF), static_cast<char>((upper >> 48) & 0xFF),
                                 static_cast<char>((upper >> 40) & 0xFF), static_cast<char>((upper >> 32) & 0xFF),
                                 static_cast<char>((upper >> 24) & 0xFF), static_cast<char>((upper >> 16) & 0xFF),
                                 static_cast<char>((upper >> 8) & 0xFF), static_cast<char>((upper >> 0) & 0xFF),

                                 static_cast<char>((lower >> 56) & 0xFF), static_cast<char>((lower >> 48) & 0xFF),
                                 static_cast<char>((lower >> 40) & 0xFF), static_cast<char>((lower >> 32) & 0xFF),
                                 static_cast<char>((lower >> 24) & 0xFF), static_cast<char>((lower >> 16) & 0xFF),
                                 static_cast<char>((lower >> 8) & 0xFF), static_cast<char>((lower >> 0) & 0xFF));
         }

        public:
         explicit Hasher(const size_t& N) : reducer(N) {
            // Randomly seed hash functions
            std::random_device r;
            std::default_random_engine rng(r());
            std::uniform_int_distribution<std::uint64_t> dist(std::numeric_limits<std::uint64_t>::min(),
                                                              std::numeric_limits<std::uint64_t>::max());
            s0 = dist(rng);
            do
               s1 = dist(rng);
            while (s1 == s0);
            do
               s2 = dist(rng);
            while (s2 == s0 || s2 == s1);
         }

         ~Hasher() = default;
         Hasher(const Hasher& other) : s0(other.s0), s1(other.s1), s2(other.s2), reducer(other.reducer.N) {}
         Hasher& operator=(const Hasher& other) {
            if (&other == this)
               return *this;

            s0 = other.s0;
            s1 = other.s1;
            s2 = other.s2;
            reducer = hashing::reduction::FastModulo<Data>(other.reducer.N);

            return *this;
         }

         Hasher(Hasher&& other) = delete;
         Hasher& operator=(Hasher&& other) noexcept {
            if (&other == this)
               return *this;

            s0 = other.s0;
            s1 = other.s1;
            s2 = other.s2;
            reducer = hashing::reduction::FastModulo<Data>(other.reducer.N);

            return *this;
         }

         forceinline std::tuple<size_t, size_t, size_t> operator()(const Data& d) const {
            return std::make_tuple(reducer(hashfn(d, make_seed(s0))), //
                                   reducer(hashfn(d, make_seed(s1))), //
                                   reducer(hashfn(d, make_seed(s2))));
         }
      };

   } // namespace support

   template<class Data, class Hasher = support::Hasher<Data>, class HyperGraph = support::HyperGraph<Data, Hasher>>
   struct MWHC;

   template<class Data, class Hasher = support::Hasher<Data>, class HyperGraph = support::HyperGraph<Data, Hasher>>
   class CompactedMWHC {
      using MWHC = MWHC<Data, Hasher, HyperGraph>;
      Hasher hasher;
      hashing::reduction::FastModulo<std::uint64_t> mod_N;

      sdsl::bit_vector_il<> bit_vec;
      decltype(bit_vec)::rank_1_type bit_vec_rank;
      sdsl::int_vector<> vertex_values;

     public:
      explicit CompactedMWHC(const std::vector<Data>& dataset)
         : hasher(MWHC::vertices_count(dataset.size())), mod_N(MWHC::vertices_count(dataset.size())) {
         const MWHC mwhc(dataset);

         // copy unchanged fields
         hasher = mwhc.hasher;
         mod_N = mwhc.mod_N;

         // helper to decide whether or not a value is set
         const auto is_set = [&](const auto& val) { return val != 0 && val != mod_N.N; };

         // build bitvector on top of vertex values (to eliminate zeroes)
         const auto n = mwhc.vertex_values.size();
         sdsl::bit_vector bv(n);
         for (size_t i = 0; i < n; i++)
            bv[i] = is_set(mwhc.vertex_values[i]);
         bit_vec = decltype(bit_vec)(bv);

         // initialize rank struct to speedup lookup
         sdsl::util::init_support(bit_vec_rank, &bit_vec);

         // copy vertex values into compacted layout.
         // add one trailing number to prevent trailing
         // unset values' rank from exceeding array ranges
         size_t set_n = 1;
         for (const auto& val : mwhc.vertex_values)
            set_n += is_set(val);

         sdsl::int_vector<> vec(set_n, 0);
         for (size_t i = 0; i < n; i++) {
            const auto val = mwhc.vertex_values[i];
            const auto rank = bit_vec_rank(i);

            assert(rank >= 0);
            assert(rank < vec.size());

            vec[rank] = val;
         }

         for (size_t i = 0; i < n; i++) {
            if (!is_set(mwhc.vertex_values[i]))
               continue;
            assert(mwhc.vertex_values[i] == vec[bit_vec_rank(i)]);
         }

         // compress compacted vertex values to eak out even more space
         sdsl::util::bit_compress(vec);
         vertex_values = vec;
      }

      forceinline size_t operator()(const Data& key) const {
         const auto [h0, h1, h2] = hasher(key);

         // 0 and mod_N.N (i.e. unset values) are compressed by a 0 bit in bit_vec
         const auto v0 = bit_vec[h0] * vertex_values[bit_vec_rank(h0)];
         const auto v1 = bit_vec[h1] * vertex_values[bit_vec_rank(h1)];
         const auto v2 = bit_vec[h2] * vertex_values[bit_vec_rank(h2)];

         size_t hash = v0;
         if (likely(h1 != h0))
            hash += v1;
         if (likely(h2 != h1 && h2 != h0))
            hash += v2;
         return mod_N(hash);
      }

      static std::string name() {
         return "CompactedMWHC";
      }

      size_t byte_size() const {
         return sizeof(hasher) + sizeof(mod_N) + sdsl::size_in_bytes(bit_vec) + sdsl::size_in_bytes(bit_vec_rank) +
            sdsl::size_in_bytes(vertex_values);
      }
   };

   template<class Data, class Hasher = support::Hasher<Data>, class HyperGraph = support::HyperGraph<Data, Hasher>>
   class CompressedMWHC {
      using MWHC = MWHC<Data, Hasher, HyperGraph>;

      Hasher hasher;
      hashing::reduction::FastModulo<std::uint64_t> mod_N;
      sdsl::int_vector<> vertex_values;

     public:
      explicit CompressedMWHC(const std::vector<Data>& dataset)
         : hasher(MWHC::vertices_count(dataset.size())), mod_N(MWHC::vertices_count(dataset.size())) {
         const MWHC mwhc(dataset);

         // copy unchanged fields
         hasher = mwhc.hasher;
         mod_N = mwhc.mod_N;

         // helper to decide whether or not a value is set
         const auto is_set = [&](const auto& val) { return val != 0 && val != mod_N.N; };

         // copy & compress vertex values. Bit compression seems to be most efficient
         // since vertex_values are more or less uniform random (flat entropy)
         sdsl::int_vector<> vec(mwhc.vertex_values.size(), 0);
         assert(vec.size() == mwhc.vertex_values.size());
         for (size_t i = 0; i < vec.size(); i++) {
            const auto val = mwhc.vertex_values[i];
            vec[i] = is_set(val) * val;
         }
         sdsl::util::bit_compress(vec);

         vertex_values = vec;
      }

      forceinline size_t operator()(const Data& key) const {
         const auto [h0, h1, h2] = hasher(key);
         size_t hash = vertex_values[h0];
         if (likely(h1 != h0))
            hash += vertex_values[h1];
         if (likely(h2 != h1 && h2 != h0))
            hash += vertex_values[h2];
         return mod_N(hash);
      }

      static std::string name() {
         return "CompressedMWHC";
      }

      size_t byte_size() const {
         return sizeof(hasher) + sizeof(mod_N) + sdsl::size_in_bytes(vertex_values);
      }
   };

   template<class Data, class Hasher, class HyperGraph>
   struct MWHC {
      explicit MWHC(const std::vector<Data>& dataset)
         : hasher(vertices_count(dataset.size())), mod_N(vertices_count(dataset.size())) {
         // 'unassigned vertex value' is marked using the value of N since N mod N = 0
         vertex_values.resize(mod_N.N);
         std::fill(vertex_values.begin(), vertex_values.end(), vertex_values.size());

         std::stack<size_t> edge_order;
         while (edge_order.empty()) {
            // 1. Generate random Hypergraph
            hasher = Hasher(mod_N.N);
            HyperGraph g(dataset, hasher, mod_N.N);

            // 2. Peel (i.e., check for acyclicity)
            edge_order = g.peel();
         }

         // 3. Assign values to vertices depending on edge_order
         while (!edge_order.empty()) {
            // get next edge
            const auto edge_ind = edge_order.top();
            edge_order.pop();

            // lookup vertices of this edge
            const auto [h0, h1, h2] = hasher(dataset[edge_ind]);

            // compute vertex assign value. Handle the case that there are collisions, i.e. same h value twice
            size_t current_val = vertex_values[h0];
            if (h1 != h0)
               current_val += vertex_values[h1];
            if (h2 != h0 && h2 != h1)
               current_val += vertex_values[h2];
            current_val = mod_N(current_val);
            size_t x = mod_N(mod_N.N + edge_ind - current_val);

            // assign vertex values
            bool assigned = false;
            if (vertex_values[h0] == static_cast<size_t>(mod_N.N)) {
               vertex_values[h0] = x;
               assigned = true;
            }

            if (vertex_values[h1] == static_cast<size_t>(mod_N.N)) {
               vertex_values[h1] = assigned ? 0 : x;
               assigned = true;
            }

            if (vertex_values[h2] == static_cast<size_t>(mod_N.N))
               vertex_values[h2] = assigned ? 0 : x;

            assert(this->operator()(dataset[edge_ind]) == edge_ind);
         }
      }

      static std::string name() {
         return "MWHC";
      }

      forceinline size_t operator()(const Data& key) const {
         const auto [h0, h1, h2] = hasher(key);
         size_t hash = vertex_values[h0];
         if (likely(h1 != h0))
            hash += vertex_values[h1];
         if (likely(h2 != h1 && h2 != h0))
            hash += vertex_values[h2];
         return mod_N(hash);
      }

      size_t byte_size() const {
         return sizeof(hasher) + sizeof(mod_N) + sizeof(decltype(vertex_values)) +
            sizeof(size_t) * vertex_values.size();
      }

     private:
      static forceinline size_t vertices_count(const size_t& dataset_size, const long double& overalloc = 1.23) {
         return std::ceil(overalloc * dataset_size);
      }

      Hasher hasher;
      hashing::reduction::FastModulo<std::uint64_t> mod_N;

      std::vector<size_t> vertex_values;

      friend CompressedMWHC<Data, Hasher, HyperGraph>;
      friend CompactedMWHC<Data, Hasher, HyperGraph>;
   };
} // namespace exotic_hashing
