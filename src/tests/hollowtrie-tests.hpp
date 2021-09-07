#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

// ==== Simple Hollow Trie ====

TEST(SimpleHollowTrie, IsPerfect) {
   using namespace exotic_hashing;
   using namespace exotic_hashing::support;

   tests::common::run_test<std::uint64_t, SimpleHollowTrie<std::uint64_t, FixedBitConverter<std::uint64_t>>,
                           tests::common::TestIsPerfect>();
}

TEST(SimpleHollowTrie, IsMinimal) {
   using namespace exotic_hashing;
   using namespace exotic_hashing::support;

   tests::common::run_test<std::uint64_t, SimpleHollowTrie<std::uint64_t, FixedBitConverter<std::uint64_t>>,
                           tests::common::TestIsMinimal>();
}

TEST(SimpleHollowTrie, IsMonotone) {
   using namespace exotic_hashing;
   using namespace exotic_hashing::support;

   tests::common::run_test<std::uint64_t, SimpleHollowTrie<std::uint64_t, FixedBitConverter<std::uint64_t>>,
                           tests::common::TestIsMonotone>();
}

TEST(SimpleHollowTrie, IsMMPHF) {
   using namespace exotic_hashing;
   using namespace exotic_hashing::support;

   tests::common::run_test<std::uint64_t, SimpleHollowTrie<std::uint64_t, FixedBitConverter<std::uint64_t>>,
                           tests::common::TestIsMMPHF>();
}

// ==== Hollow Trie ====

TEST(HollowTrie, IsPerfect) {
   using namespace exotic_hashing;
   using namespace exotic_hashing::support;

   tests::common::run_test<std::uint64_t, HollowTrie<std::uint64_t, FixedBitConverter<std::uint64_t>>,
                           tests::common::TestIsPerfect>();
}

TEST(HollowTrie, IsMinimal) {
   using namespace exotic_hashing;
   using namespace exotic_hashing::support;

   tests::common::run_test<std::uint64_t, HollowTrie<std::uint64_t, FixedBitConverter<std::uint64_t>>,
                           tests::common::TestIsMinimal>();
}

TEST(HollowTrie, IsMonotone) {
   using namespace exotic_hashing;
   using namespace exotic_hashing::support;

   tests::common::run_test<std::uint64_t, HollowTrie<std::uint64_t, FixedBitConverter<std::uint64_t>>,
                           tests::common::TestIsMonotone>();
}

TEST(HollowTrie, IsMMPHF) {
   using namespace exotic_hashing;
   using namespace exotic_hashing::support;

   tests::common::run_test<std::uint64_t, HollowTrie<std::uint64_t, FixedBitConverter<std::uint64_t>>,
                           tests::common::TestIsMMPHF>();
}

