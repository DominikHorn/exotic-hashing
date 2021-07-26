#pragma once

#include <cassert>
#include <iostream>
#include <limits>
#include <optional>
#include <sdsl/bit_vectors.hpp>

#include "convenience/builtins.hpp"
#include "convenience/tidy.hpp"

namespace exotic_hashing {
   template<class Key, class BitConverter>
   struct SimpleHollowTrie;

   template<class Key, class BitConverter>
   struct CompactTrie {
      CompactTrie() {}

      CompactTrie(const std::vector<Key>& keyset) {
         insert(keyset);
      }

      ~CompactTrie() {
         if (root != nullptr)
            delete root;
      }

      static std::string name() {
         return "CompactTrie";
      }

      size_t byte_size() const {
         return sizeof(CompactTrie) + root->byte_size();
      };

      /**
       * Inserts a set of keys into the trie. Each Key's binary encoding
       * mustn't be a prefix of any other previously inserted key's binary
       * encoding.
       *
       * Duplicate insertions will be ignored.
       *
       * @param keyset
       */
      void insert(const std::vector<Key>& keyset) {
         for (const auto key : keyset)
            insert(key);
      }

      /**
       * Inserts a key into the trie. Key's binary encoding mustn't be a
       * prefix of any other previously inserted key's binary encodings.
       *
       * Duplicate insertions will be ignored.
       *
       * @param key
       */
      void insert(const Key& key) {
         sdsl::bit_vector key_bits = converter(key);

         if (unlikely(root == nullptr))
            root = new Node(key_bits, 0, key_bits.size());
         else
            root = root->insert(key_bits, 0);
      }

      /**
       * Returns the rank of a given key relative to the sorted set of
       * previously inserted keys, or std::numeric_limits<size_t>::max() if
       * said key is not a member of the previously inserted keyset.
       *
       * @param key
       */
      forceinline size_t operator()(const Key& key) const {
         if (unlikely(root == nullptr))
            return 0;

         sdsl::bit_vector key_bits = converter(key);
         return root->rank(key_bits, 0, 0);
      }

      /**
       * Prints a latex tikz standalone document representing this
       * datastructuree.
       *
       * @param out output stream to print to, e.g., std::cout
       */
      template<class Stream>
      void print_tex(Stream& out) const {
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

         if (unlikely(root == nullptr))
            out << "  [,phantom]" << std::endl;
         else
            root->print_tikz(out, 2);

         out << " \\end{forest}\n"
                "\n"
                "\\end{document}"
             << std::endl;
      }

     private:
      struct Node {
         Node(const sdsl::bit_vector& key_bits, size_t start, size_t end, size_t local_left_leaf_cnt = 0)
            : local_left_leaf_cnt(local_left_leaf_cnt) {
            const size_t prefix_len = end - start;
            prefix.resize(prefix_len);
            for (size_t i = 0; i < prefix_len; i++)
               prefix[i] = key_bits[i + start];
         }

         ~Node() {
            if (left != nullptr)
               delete left;
            if (right != nullptr)
               delete right;
         }

         /**
          * Retrieves the rank associated with a given key. Returns
          * std::numeric_limits<size_t>::max() if the key was not inserted
          *
          * @param key_bits: bit representation of the key's value
          * @param start: start of the key_bits suffix under consideration for this node
          * @param left_leaf_cnt: amount of leafs to the left of this node
          */
         size_t rank(const sdsl::bit_vector& key_bits, size_t start, size_t left_leaf_cnt) const {
            const auto not_found_rank = std::numeric_limits<size_t>::max();

            // Option 1: At least one bit missmatches between prefix and remaining key. This node
            //    would have been split during construction however if this were the case.
            //    Since it is not, we therefore immediately know that the key was not in the
            //    inserted keyset.
            //
            // Option 2: No bit missmatches, meaning current key is a prefix of another key
            //    in the keyset, violating the "prefix free code" assumption. Note that this
            //    can never happen for fixed length coding
            if (key_bits.size() - start < prefix.size())
               return not_found_rank;

            // If prefix does not match the key is not in this trie
            for (size_t i = 0; i + start < key_bits.size() && i < prefix.size(); i++)
               if (key_bits[i + start] != prefix[i])
                  return not_found_rank;

            // If this node is a leaf node...
            if (this->is_leaf()) {
               // ...and the key has no more bits to check, we have a match!
               if (key_bits.size() - start - prefix.size() == 0)
                  // rank starts at 0 (i.e., don't add 1 here)
                  return left_leaf_cnt;

               // ...otherwise the key is not in the keyset
               return not_found_rank;
            }

            // This is not a leaf but key has no more bits to check, i.e.,
            // is a prefix of another key which violates the "prefix free code"
            // assumption
            assert(key_bits.size() - start - prefix.size() > 0);

            if (key_bits[start + prefix.size()])
               return right->rank(key_bits, start + prefix.size(), left_leaf_cnt + this->local_left_leaf_cnt);

            return left->rank(key_bits, start + prefix.size(), left_leaf_cnt);
         }

         /**
          * Inserts a key into this trie. Key's binary encoding mustn't be a prefix
          * of any other previously inserted key's binary encoding.
          *
          * @param key_bits: bit representation of the key's value
          * @param start: start of the key_bits suffix under consideration for this node
          */
         Node* insert(const sdsl::bit_vector& key_bits, size_t start) {
            // Find first index where prefix missmatches key if any and split node
            for (size_t i = 0; i < prefix.size(); i++) {
               // Key we're trying to insert is a prefix of another key that was previously inserted
               assert(i + start < key_bits.size());

               if (prefix[i] != key_bits[i + start]) {
                  Node* parent = new Node(key_bits, start, start + i);

                  // Shorten this node's prefix
                  sdsl::bit_vector new_prefix(prefix.size() - i, 0);
                  for (size_t j = i; j < prefix.size(); j++)
                     new_prefix[j - i] = prefix[j];
                  this->prefix = new_prefix;

                  if (key_bits[i + start]) {
                     // New key is inserted on the right
                     parent->local_left_leaf_cnt = this->leaf_count();
                     parent->left = this;
                     parent->right = new Node(key_bits, i + start, key_bits.size());
                  } else {
                     // New key is inserted on the left
                     parent->local_left_leaf_cnt = 1;
                     parent->left = new Node(key_bits, i + start, key_bits.size());
                     parent->right = this;
                  }

                  return parent;
               }
            }

            // Catch duplicate inserts but optimize for this not happening
            if (unlikely(key_bits.size() - start - prefix.size() == 0))
               return this;

            // Otherwise a previously inserted key (represented by this node)
            // is a prefix of the key we're trying to insert in violation of
            // the 'prefix free code' assumption.
            assert(!this->is_leaf());

            // Recursively insert on correct side, not touching this node
            if (key_bits[start + prefix.size()]) {
               right = right->insert(key_bits, start + prefix.size());
            } else {
               local_left_leaf_cnt++;
               left = left->insert(key_bits, start + prefix.size());
            }

            return this;
         }

         /**
          * Returns the amount of leaf nodes within the subtrie rooted in this
          * node
          */
         size_t leaf_count() const {
            if (is_leaf())
               return 1;

            assert(left != nullptr);
            assert(right != nullptr);

            return local_left_leaf_cnt + right->leaf_count();
         }

         /**
          * Prints a latex tikz forest representation of the subtrie
          * represented by this node
          *
          * @param out output stream to print to, e.g., std::cout
          * @param indent current indentation level. Defaults to 0 (root node)
          */
         template<class Stream>
         void print_tikz(Stream& out, const size_t indent = 0) const {
            for (size_t i = 0; i < indent; i++)
               out << " ";

            out << "[{";
            for (auto it = prefix.begin(); it < prefix.end(); it++)
               out << *it;
            out << ", " << local_left_leaf_cnt << "}" << std::endl;

            if (left == nullptr) {
               for (size_t i = 0; i < indent + 1; i++)
                  out << " ";
               out << "[,phantom]" << std::endl;
            } else
               left->print_tikz(out, indent + 1);

            if (right == nullptr) {
               for (size_t i = 0; i < indent + 1; i++)
                  out << " ";
               out << "[,phantom]" << std::endl;
            } else
               right->print_tikz(out, indent + 1);

            for (size_t i = 0; i < indent; i++)
               out << " ";
            out << "]" << std::endl;
         }

         size_t byte_size() const {
            size_t size = sizeof(Node) + size_in_bytes(prefix);
            if (left != nullptr)
               size += left->byte_size();
            if (right != nullptr)
               size += right->byte_size();
            return size;
         };

        private:
         sdsl::bit_vector prefix;
         size_t local_left_leaf_cnt = 0;

         Node* left = nullptr;
         Node* right = nullptr;

         forceinline bool is_leaf() const {
            // Either both children are set or both are not set (due to
            // construction)
            return left == nullptr;
         }

         friend SimpleHollowTrie<Key, BitConverter>;
      };

      Node* root = nullptr;
      BitConverter converter;

      friend SimpleHollowTrie<Key, BitConverter>;
   };
} // namespace exotic_hashing
