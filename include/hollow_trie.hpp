#pragma once

#include <cassert>
#include <iostream>
#include <limits>
#include <list>
#include <optional>
#include <sdsl/bit_vectors.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "compact_trie.hpp"
#include "convenience/builtins.hpp"
#include "convenience/tidy.hpp"

namespace exotic_hashing {
   template<class Key, class BitConverter>
   struct SimpleHollowTrie {
      /**
       * Builds a new hollow trie from a dataset by constructing a compacted
       * trie and converting it to the space efficient hollow trie
       * representation
       */
      SimpleHollowTrie(const std::vector<Key>& dataset) : SimpleHollowTrie(CompactTrie<Key, BitConverter>(dataset)) {}

      /**
       * Derives a hollow trie from a given compacted trie by converting it to
       * the space efficient hollow trie representation
       */
      SimpleHollowTrie(const CompactTrie<Key, BitConverter>& compact_trie) {
         // 1. Generate hollow trie representation (implemented as linked list
         // for representation construction algorithm efficiency)
         const auto encoding_list = convert(*compact_trie.root);

         // 2. Convert to final, minimal encoding
         nodes = std::vector<Node>(encoding_list.begin(), encoding_list.end());
      }

      forceinline size_t operator()(const Key& key) const {
         sdsl::bit_vector key_bits = converter(key);

         size_t left_leaf_cnt = 0, key_bits_ind = 0, leftmost_right = nodes.size();
         for (size_t i = 0; key_bits_ind < key_bits.size();) {
            const Node& n = nodes[i];

            key_bits_ind += n.bit_skip();

            // Right (if) or Left (else) traversal
            if (key_bits[key_bits_ind]) {
               // node_skip = left.leaf_cnt() == left_leaf_cnt of CompactTrie
               left_leaf_cnt += n.node_skip();

               // Right child is always at i + node_skip
               i = i + n.node_skip();

               // We encountered a right leaf
               if (i >= leftmost_right)
                  return left_leaf_cnt;
            } else {
               // We encountered a left leaf
               if (n.node_skip() == 1)
                  return left_leaf_cnt;

               // Keep track of this to be able to detect right leafs
               leftmost_right = i + n.node_skip();

               // Left child is always at i+1
               i = i + 1;
            }
         }

         return std::numeric_limits<size_t>::max();
      }

      static std::string name() {
         return "SimpleHollowTrie";
      }

      size_t byte_size() const {
         return sizeof(SimpleHollowTrie<Key, BitConverter>) + sizeof(Node) * nodes.size();
      };

      /**
       * Prints a latex tikz standalone document representing this
       * datastructuree.
       */
      template<class Stream>
      void print_tex(Stream& out) const {
         // Latex preamble
         out << "\\documentclass[tikz]{standalone}\n"
                "\\usepackage[utf8]{inputenc}\n"
                "\\usepackage{forest}\n"
                "\n"
                "\n"
                "\\begin{document}\n"
                "\n"
                " \\begin{forest}\n"
                "  for tree={\n"
                "   rectangle,\n"
                "   black,\n"
                "   draw,\n"
                "   fill=blue!30,\n"
                "  }"
             << std::endl;

         // Print actual trie
         print_subtrie_tikz(out, 0, nodes.size());

         // Latex closing tags
         out << " \\end{forest}\n"
                "\n"
                "\\end{document}"
             << std::endl;
      }

     private:
      struct Node {
         Node(const std::size_t& bit_skip, const std::size_t& node_skip) {
            // TODO: implement a real fix for this limitation
            if (unlikely(bit_skip > (((std::uint64_t) 0x1 << bit_skip_size) - 1)))
               throw std::runtime_error("Failed to construct HollowTrie: bit_skip exceeds " +
                                        std::to_string(bit_skip_size) + " bits");
            if (unlikely(node_skip > ((std::uint64_t) 0x1 << (sizeof(data) * 8 - bit_skip_size)) - 1))
               throw std::runtime_error("Failed to construct HollowTrie: node_skip exceeds " +
                                        std::to_string(sizeof(data) * 8 - bit_skip_size) + " bits");

            data = (bit_skip & (((std::uint64_t) 0x1 << bit_skip_size) - 1)) | (node_skip << bit_skip_size);
         }

         forceinline std::size_t bit_skip() const {
            return data & (((std::uint64_t) 0x1 << bit_skip_size) - 1);
         }

         forceinline std::size_t node_skip() const {
            return data >> bit_skip_size;
         }

        private:
         std::uint64_t data;
         static const size_t bit_skip_size = 8;
      };

      /**
       * Converts a given CompactTrie subtrie to the HollowTrie stream format, derived
       * from ideas from the theory paper & Jacobson's 89 work (mentioned in theory paper)
       *
       * parent encoding | left subtrie encoding | right subtrie encoding
       *
       * Due to this encoding, root node is at index = 0 and, for a node at index i, its left
       * child is always at index i+1 while its right child is always at index i+skip;
       *
       * Note that leaf nodes don't exist in the encoded representation
       *
       * This encoding saves space (only 64 bit per node) and should also
       * eliminate cache misses when accessing left children. Since about 50%
       * of edges along each path are expected to be left child accesses, this
       * should improve real world performance noticeably.
       */
      std::list<Node> convert(const typename CompactTrie<Key, BitConverter>::Node& subtrie) const {
         if (subtrie.is_leaf())
            return {};

         // TODO: leaf_count() is an O(log(N)) operation where N is the max
         // length of any Key's bitstream.  By also storing right_leaf_count in CompactTrie::Node,
         // we could make this constant time

         // Since inner_node_count = leaf_count - 1 for compact tries, this
         // call elegantly computes inner_node_count + 1, i.e., the required
         // node_skip
         const size_t node_skip = subtrie.left->leaf_count();

         // Encode parent | left subtrie | right subtrie
         std::list<Node> l;
         l.emplace_back(subtrie.prefix.size(), node_skip);
         l.splice(l.end(), convert(*subtrie.left));
         l.splice(l.end(), convert(*subtrie.right));
         return l;
      }

      /**
       * Prints a latex tikz forest representation of the subtrie
       * represented by this node
       *
       * @param out output stream to print to, e.g., std::cout
       * @param node_index index of the root of the considered subtrie
       * @param leftmost_right leftmost index of a right child encountered along the way.
       *  Necessary to detect right leafs. TODO: verify this claim
       * @param indent current indentation level. Defaults to 0 (root node)
       */
      template<class Stream>
      void print_subtrie_tikz(Stream& out,
                              const size_t node_index,
                              const size_t leftmost_right,
                              const size_t indent = 0) const {
         for (size_t i = 0; i < indent; i++)
            out << " ";

         // Current node
         const Node& n = nodes[node_index];
         out << "[{" << n.bit_skip() << ", " << n.node_skip() << "}" << std::endl;

         // Left child
         if (n.node_skip() == 1) {
            // ... is a leaf
            for (size_t i = 0; i < indent + 1; i++)
               out << " ";
            out << "[,phantom]" << std::endl;
         } else
            print_subtrie_tikz(out, node_index + 1, node_index + n.node_skip(), indent + 1);

         // Right child
         if (node_index + n.node_skip() >= leftmost_right) {
            // ... is a leaf
            for (size_t i = 0; i < indent + 1; i++)
               out << " ";
            out << "[,phantom]" << std::endl;
         } else
            print_subtrie_tikz(out, node_index + n.node_skip(), leftmost_right, indent + 1);

         for (size_t i = 0; i < indent; i++)
            out << " ";
         out << "]" << std::endl;
      }

      std::vector<Node> nodes;
      BitConverter converter;
   };
} // namespace exotic_hashing
