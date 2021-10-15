#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

// ==== Simple Hollow Trie ====

TEST(SimpleHollowTrie, IsPerfect) {
   using SimpleHollowTrie =
      exotic_hashing::SimpleHollowTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>;
   tests::common::run_test<std::uint64_t, SimpleHollowTrie, tests::common::TestIsPerfect>();
}

TEST(SimpleHollowTrie, IsMinimal) {
   using SimpleHollowTrie =
      exotic_hashing::SimpleHollowTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>;
   tests::common::run_test<std::uint64_t, SimpleHollowTrie, tests::common::TestIsMinimal>();
}

TEST(SimpleHollowTrie, IsMonotone) {
   using SimpleHollowTrie =
      exotic_hashing::SimpleHollowTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>;
   tests::common::run_test<std::uint64_t, SimpleHollowTrie, tests::common::TestIsMonotone>();
}

TEST(SimpleHollowTrie, IsMMPHF) {
   using SimpleHollowTrie =
      exotic_hashing::SimpleHollowTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>;
   tests::common::run_test<std::uint64_t, SimpleHollowTrie, tests::common::TestIsMMPHF>();
}

// ==== Hollow Trie ====

TEST(HollowTrie, IsPerfect) {
   using HollowTrie =
      exotic_hashing::HollowTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>;
   tests::common::run_test<std::uint64_t, HollowTrie, tests::common::TestIsPerfect>();
}

TEST(HollowTrie, IsMinimal) {
   using HollowTrie =
      exotic_hashing::HollowTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>;
   tests::common::run_test<std::uint64_t, HollowTrie, tests::common::TestIsMinimal>();
}

TEST(HollowTrie, IsMonotone) {
   using HollowTrie =
      exotic_hashing::HollowTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>;
   tests::common::run_test<std::uint64_t, HollowTrie, tests::common::TestIsMonotone>();
}

TEST(HollowTrie, IsMMPHF) {
   using HollowTrie =
      exotic_hashing::HollowTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>;
   tests::common::run_test<std::uint64_t, HollowTrie, tests::common::TestIsMMPHF>();
}

