#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <queue>
#include <random>
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
   template<class Data>
   struct MWHC {
      explicit MWHC(const std::vector<Data>& dataset)
         : hasher(vertices_count(dataset.size())), mod_N(vertices_count(dataset.size())) {
         // 'unassigned vertex value' is marked using the value of N since N mod N = 0
         vertex_values.resize(mod_N.N);
         std::fill(vertex_values.begin(), vertex_values.end(), mod_N.N);

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
         return sizeof(decltype(vertex_values)) + sizeof(size_t) + vertex_values.size();
      }

     private:
      forceinline size_t vertices_count(const size_t& dataset_size, const long double& overalloc = 1.23) const {
         return std::ceil(overalloc * dataset_size);
      }

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

      class HyperGraph {
         struct Vertex {
            /// Each edge is represented as an index into the dataset
            std::vector<size_t> edges;

            forceinline size_t degree() const {
               return edges.size();
            }

            forceinline void remove(const size_t edge) {
               const auto it = std::find(edges.begin(), edges.end(), edge);
               if (likely(it != edges.end()))
                  edges.erase(it);
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
               const auto [h0, h1, h2] = hasher(dataset[i]);
               vertices[h0].edges.push_back(i);
               vertices[h1].edges.push_back(i);
               vertices[h2].edges.push_back(i);
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
            std::stack<size_t> res;

            // 1. Recursively peel all vertices if possible (without recursion to avoid stack overflows)
            for (size_t vertex_index = 0; vertex_index < vertices.size(); vertex_index++) {
               std::queue<size_t> next_vertices;
               next_vertices.push(vertex_index);

               while (!next_vertices.empty()) {
                  auto& vertex = vertices[next_vertices.front()];
                  next_vertices.pop();

                  // Only vertices with degree 1 are peelable
                  if (vertex.degree() != 1)
                     continue;

                  // Peel last edge from this vertex
                  const auto edge = vertex.edges[0];
                  const auto [h0, h1, h2] = hasher(dataset[edge]);
                  vertices[h0].remove(edge);
                  vertices[h1].remove(edge);
                  vertices[h2].remove(edge);

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

      Hasher hasher;
      hashing::reduction::FastModulo<std::uint64_t> mod_N;

      // TODO(dominik): many entries will be 0 -> compression opportunities?
      std::vector<size_t> vertex_values;
   };
} // namespace exotic_hashing
