#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

// ==== Simple Hollow Trie ====

using SimpleHollowTrie = exotic_hashing::SimpleHollowTrie<
   std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t, exotic_hashing::support::Bitvector<>>>;

TEST(SimpleHollowTrie, IsPerfect) {
   tests::common::run_test<std::uint64_t, SimpleHollowTrie, tests::common::TestIsPerfect>();
}

TEST(SimpleHollowTrie, IsMinimal) {
   tests::common::run_test<std::uint64_t, SimpleHollowTrie, tests::common::TestIsMinimal>();
}

TEST(SimpleHollowTrie, IsMonotone) {
   tests::common::run_test<std::uint64_t, SimpleHollowTrie, tests::common::TestIsMonotone>();
}

TEST(SimpleHollowTrie, IsMMPHF) {
   tests::common::run_test<std::uint64_t, SimpleHollowTrie, tests::common::TestIsMMPHF>();
}

// ==== Hollow Trie ====

using HollowTrie = exotic_hashing::HollowTrie<
   std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t, exotic_hashing::support::Bitvector<>>>;

TEST(HollowTrie, IsPerfect) {
   tests::common::run_test<std::uint64_t, HollowTrie, tests::common::TestIsPerfect>();
}

TEST(HollowTrie, IsMinimal) {
   tests::common::run_test<std::uint64_t, HollowTrie, tests::common::TestIsMinimal>();
}

TEST(HollowTrie, IsMonotone) {
   tests::common::run_test<std::uint64_t, HollowTrie, tests::common::TestIsMonotone>();
}

TEST(HollowTrie, IsMMPHF) {
   tests::common::run_test<std::uint64_t, HollowTrie, tests::common::TestIsMMPHF>();
}

