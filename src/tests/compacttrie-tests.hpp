#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include "common.hpp"

TEST(CompactTrie, IsPerfect) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompactTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>,
      tests::common::TestIsPerfect>();
}

TEST(CompactTrie, IsMinimal) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompactTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>,
      tests::common::TestIsMinimal>();
}

TEST(CompactTrie, IsMonotone) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompactTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>,
      tests::common::TestIsMonotone>();
}

TEST(CompactTrie, IsMMPHF) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompactTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>,
      tests::common::TestIsMMPHF>();
}
