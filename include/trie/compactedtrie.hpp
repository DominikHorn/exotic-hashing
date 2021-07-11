#pragma once

#include <cassert>
#include <limits>
#include <optional>
#include <sdsl/bit_vectors.hpp>

#include "../convenience/builtins.hpp"
#include "../convenience/tidy.hpp"

namespace exotic_hashing {
   template<class Key, class BitConverter>
   struct CompactedTrieRank {
      CompactedTrieRank() {}

      ~CompactedTrieRank() {
         if (root != nullptr)
            delete root;
      }

      void insert(const Key& key) {
         sdsl::bit_vector key_bits = converter(key);

         if (unlikely(root == nullptr))
            root = new Node(key_bits, 0, key_bits.size());
         else
            root = root->insert(key_bits, 0);
      }

      forceinline size_t operator()(const Key& key) const {
         if (unlikely(root == nullptr))
            return 0;

         sdsl::bit_vector key_bits = converter(key);
         return root->rank(key_bits, 0, 0);
      }

      // TODO: function for converting to hollow trie that
      // acts as monotone hash function (index = amount of nodes left of exit node)

     private:
      struct Node {
         Node(const sdsl::bit_vector& key_bits, size_t start, size_t end, size_t left_child_cnt = 0)
            : left_child_cnt(left_child_cnt) {
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
               return right->rank(key_bits, start + prefix.size(), left_leaf_cnt + this->left_child_cnt);

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
                     // New key is inserted on the right (left child count stays unchanged)
                     parent->left_child_cnt = this->left_child_cnt;
                     parent->left = this;
                     parent->right = new Node(key_bits, i + start, key_bits.size());
                  } else {
                     // New key is inserted on the left (left child count of parent = 1)
                     parent->left_child_cnt = 1;
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
               left_child_cnt++;
               left = left->insert(key_bits, start + prefix.size());
            }

            return this;
         }

        private:
         sdsl::bit_vector prefix;
         size_t left_child_cnt = 0;

         Node* left = nullptr;
         Node* right = nullptr;

         forceinline bool is_leaf() const {
            // Either both children are set or both are not set (due to
            // construction)
            return left == nullptr;
         }
      };

      Node* root = nullptr;
      BitConverter converter;
   };

   template<class T>
   struct FixedBitConverter {
      forceinline sdsl::bit_vector operator()(const T& data) const {
         const size_t bit_size = sizeof(T) * 8;
         sdsl::bit_vector result(bit_size, 0);
         assert(result.size() == bit_size);
         for (size_t i = 0; i < bit_size; i++)
            result[i] = (data >> (bit_size - i - 1)) & 0x1;
#ifndef NDEBUG
         T reconstructed = 0;
         for (size_t i = 0; i < result.size(); i++) {
            const uint64_t bit = result[i];
            reconstructed |= bit << (bit_size - i - 1);
         }
         assert(reconstructed == data);
#endif
         return result;
      }
   };
} // namespace exotic_hashing
