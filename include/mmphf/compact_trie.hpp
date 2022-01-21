#pragma once

#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>

#include "../support/bitvector.hpp"
#include "../support/elias.hpp"

// Order important
#include "../convenience/builtins.hpp"

namespace exotic_hashing {
   template<class Key, class BitConverter, class BitStream>
   struct SimpleHollowTrie;

   template<class Key, class BitConverter, class BitStream>
   struct HollowTrie;

   template<class Key, class BitConverter, bool estimate_non_key_rank, class BitStream>
   class CompactedCompactTrie;

   template<class Key, class BitConverter, bool estimate_non_key_rank = false,
            class BitStream = support::FixedBitvector<sizeof(Key) * 8, Key>>
   struct CompactTrie {
      CompactTrie() = default;

      CompactTrie(const CompactTrie<Key, BitConverter, estimate_non_key_rank, BitStream>& other)
         : root(new Node(other.root)) {}
      CompactTrie& operator=(const CompactTrie& node) = delete;
      CompactTrie(const CompactTrie&& other) = delete;
      CompactTrie& operator=(const CompactTrie&& node) = delete;

      explicit CompactTrie(const std::vector<Key>& keyset) {
         insert(keyset);
      }

      ~CompactTrie() {
         delete root;
      }

      static std::string name() {
         return "CompactTrie";
      }

      size_t byte_size() const {
         const size_t struct_size = sizeof(CompactTrie);
         const size_t nodes_size = root->byte_size();
         return struct_size + nodes_size;
      };

      template<class RandomIt>
      void construct(const RandomIt& begin, const RandomIt& end) {
         for (auto it = begin; it < end; it++)
            insert(*it);
      }

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
         // std::sort will not perform significant work
         // if keyset is already sorted. In all other cases,
         // presorting pays huge dividends during construction!
         // (~2000ns per key improvement on 10^8 keys).
         //
         // overall O(n log n) asymptotic complexity does not change when sorting
         auto keys = keyset;
         std::sort(keys.begin(), keys.end());

         // TODO(dominik): implement better 'bulk insert' algorithm exploiting the fact that
         //  keys are sorted
         for (const auto key : keys)
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
         const BitConverter converter;
         BitStream key_bits = converter(key);

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

         const BitConverter converter;
         BitStream key_bits = converter(key);
         return root->rank(key_bits, 0, 0);
      }

      /**
       * Prints a latex tikz standalone document representing this
       * datastructuree.
       *
       * Example usage:
       *
         ```c++
         std::ofstream out;
         out.open("compact_trie_" + std::to_string(seed) + ".tex");
         compact_trie.print_tex(out);
         out.close();
         ```
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
         Node(const BitStream& key_bits, size_t start, size_t end, size_t local_left_leaf_cnt = 0)
            : prefix(BitStream(end - start, [&](const size_t& i) { return key_bits[i + start]; })),
              local_left_leaf_cnt(local_left_leaf_cnt) {}

         /// Copy constructor
         Node(const Node& other)
            : prefix(other.prefix), local_left_leaf_cnt(other.local_left_leaft_cnt), left(new Node(other.left)),
              right(new Node(other.right)) {}
         Node& operator=(const Node& node) = delete;
         Node(const Node&& other) = delete;
         Node& operator=(const Node&& node) = delete;

         ~Node() {
            delete left;
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
         size_t rank(const BitStream& key_bits, size_t start, size_t left_leaf_cnt) const {
            const auto not_found_rank = [&] {
               if constexpr (estimate_non_key_rank)
                  return left_leaf_cnt;
               return std::numeric_limits<size_t>::max();
            };

            // Option 1: At least one bit missmatches between prefix and remaining key. This node
            //    would have been split during construction however if this were the case.
            //    Since it is not, we therefore immediately know that the key was not in the
            //    inserted keyset.
            //
            // Option 2: No bit missmatches, meaning current key is a prefix of another key
            //    in the keyset, violating the "prefix free code" assumption. Note that this
            //    can never happen for fixed length coding
            if (key_bits.size() - start < prefix.size())
               return not_found_rank();

            // If prefix does not match the key is not in this trie
            if (!key_bits.matches(prefix, start))
               return not_found_rank();

            // If this node is a leaf node...
            if (this->is_leaf()) {
               // ...and the key has no more bits to check, we have a match!
               if (key_bits.size() - start - prefix.size() == 0)
                  // rank starts at 0 (i.e., don't add 1 here)
                  return left_leaf_cnt;

               // ...otherwise the key is not in the keyset
               return not_found_rank();
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
         Node* insert(const BitStream& key_bits, size_t start) {
            // Find first index where prefix missmatches key if any and split node
            for (size_t i = 0; i < prefix.size(); i++) {
               // Key we're trying to insert is a prefix of another key that was previously inserted
               assert(i + start < key_bits.size());

               if (prefix[i] != key_bits[i + start]) {
                  Node* parent = new Node(key_bits, start, start + i);

                  // Shorten this node's prefix
                  BitStream new_prefix(prefix.size() - i, false);
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
            for (size_t i = 0; i < prefix.size(); i++)
               out << prefix[i];
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
            const size_t struct_size = sizeof(decltype(local_left_leaf_cnt)) + 2 * sizeof(decltype(left));
            const size_t prefix_size = prefix.byte_size();
            size_t size = struct_size + prefix_size;
            if (left != nullptr)
               size += left->byte_size();
            if (right != nullptr)
               size += right->byte_size();
            return size;
         };

        private:
         BitStream prefix;
         size_t local_left_leaf_cnt = 0;

         Node* left = nullptr;
         Node* right = nullptr;

         forceinline bool is_leaf() const {
            // Either both children are set or both are not set (due to
            // construction)
            return left == nullptr;
         }

         friend HollowTrie<Key, BitConverter, BitStream>;
         friend SimpleHollowTrie<Key, BitConverter, BitStream>;
         friend CompactedCompactTrie<Key, BitConverter, true, BitStream>;
         friend CompactedCompactTrie<Key, BitConverter, false, BitStream>;
      };

      Node* root = nullptr;

      friend HollowTrie<Key, BitConverter, BitStream>;
      friend SimpleHollowTrie<Key, BitConverter, BitStream>;
      friend CompactedCompactTrie<Key, BitConverter, true, BitStream>;
      friend CompactedCompactTrie<Key, BitConverter, false, BitStream>;
   };

   /**
    * Optimized CompactTrie, i.e., instead of utilizing reference based
    * nodes, CompactedCompactTrie utilizes a bitvector representation
    * parallel to HollowTrie
    */
   template<class Key, class BitConverter, bool estimate_non_key_rank = false,
            class BitStream = support::FixedBitvector<sizeof(Key) * 8, Key>>
   class CompactedCompactTrie {
      using IntEncoder = support::EliasDeltaCoder;
      support::Bitvector<> representation;

      support::Bitvector<>
      convert(const typename CompactTrie<Key, BitConverter, estimate_non_key_rank, BitStream>::Node& subtrie) const {
         // prune entire leaf level of original compact trie
         if (subtrie.is_leaf())
            return support::Bitvector<>();

         support::Bitvector<> rep;

         // Compute child encoding
         const auto left_bitrep = convert(*subtrie.left);
         const auto right_bitrep = convert(*subtrie.right);

         // Encode 'parent | left subtrie | right subtrie', where
         // each node is encoded as 'prefix_size | prefix | left_bit_size | left_leaf_cnt'
         rep.append(IntEncoder::encode(subtrie.prefix.size() + 1));
         rep.append(subtrie.prefix);

         rep.append(IntEncoder::encode(left_bitrep.size() + 1));

         // TODO(dominik): leaf_count() is an O(log(N)) operation where N is the max
         // length of any Key's bitstream. By also storing right_leaf_count in
         // CompactTrie::Node, we could make this constant time
         rep.append(IntEncoder::encode(subtrie.left->leaf_count()));

         rep.append(left_bitrep);
         rep.append(right_bitrep);

         return rep;
      }

      /// conceptual Node used during decoding
      struct Node {
         const BitStream prefix;

         const size_t left_bitsize;
         const size_t left_leaf_count;
      };

      Node read_node(const support::Bitvector<>& stream, size_t& bit_index) const {
         const auto prefix_len = IntEncoder::decode(stream, bit_index) - 1;

         // TODO(dominik): this likely produces suboptimal performance!
         BitStream prefix(prefix_len, false);
         for (size_t i = 0; i < prefix_len; i++)
            prefix[i] = stream[i + bit_index];

         // advance past prefix
         bit_index += prefix_len;

         const auto left_bitsize = IntEncoder::decode(stream, bit_index) - 1;
         const auto left_leaf_count = IntEncoder::decode(stream, bit_index);

         return {.prefix = prefix, .left_bitsize = left_bitsize, .left_leaf_count = left_leaf_count};
      }

     public:
      CompactedCompactTrie() = default;

      /**
       * Constructs from a keyset in any order
       */
      explicit CompactedCompactTrie(const std::vector<Key>& keyset) {
         // overall complexity O(n log n) will not change, but sorting
         // massively improves hidden constants during construction.
         auto ds = keyset;
         std::sort(ds.begin(), ds.end());

         // construct on sorted data
         construct(ds.begin(), ds.end());
      }

      /**
       * Constructs from a **sorted** key range [begin, end)
       */
      template<class ForwardIt>
      explicit CompactedCompactTrie(const ForwardIt& begin, const ForwardIt& end) {
         construct(begin, end);
      }

      /**
       * updates representation to only include keys of **sorted** key range [begin, end)
       */
      template<class ForwardIt>
      void construct(const ForwardIt& begin, const ForwardIt& end) {
         // for now, simply convert from existing compact trie. For efficiency, we
         // could definitely implement a better construction algorithm in the future
         CompactTrie<Key, BitConverter, estimate_non_key_rank, BitStream> t;
         t.construct(begin, end);

         representation = convert(*t.root);
      }

      forceinline size_t operator()(const Key& key) const {
         const BitConverter converter;
         const BitStream key_bits = converter(key);

         size_t left_leaf_cnt = 0, key_bits_ind = 0, leftmost_right = representation.size(), bit_ind = 0;
         const auto not_found_rank = [&] {
            if constexpr (estimate_non_key_rank)
               return left_leaf_cnt;
            return std::numeric_limits<size_t>::max();
         };
         while (key_bits_ind < key_bits.size()) {
            const auto node = read_node(representation, bit_ind);

            if (key_bits.size() - key_bits_ind < node.prefix.size())
               return not_found_rank();
            if (!key_bits.matches(node.prefix, key_bits_ind))
               return not_found_rank();

            key_bits_ind += node.prefix.size();

            // Right (if) or Left (else) traversal
            if (key_bits[key_bits_ind]) {
               left_leaf_cnt += node.left_leaf_count;

               // Right child is always at i + node_skip
               bit_ind += node.left_bitsize;

               // We encountered a right leaf
               if (bit_ind >= leftmost_right)
                  return left_leaf_cnt;
            } else {
               // We encountered a left leaf
               if (node.left_leaf_count == 1)
                  return left_leaf_cnt;

               // Keep track of this to be able to detect right leafs
               leftmost_right = bit_ind + node.left_bitsize;

               // bit_ind is already correctly set to left child start
            }
         }

         return not_found_rank();
      }

      static std::string name() {
         return "CompactedCompactTrie";
      }

      size_t byte_size() const {
         return sizeof(decltype(*this)) + static_cast<size_t>(std::ceil(representation.size() / 8.));
      }
   };
} // namespace exotic_hashing
