#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include "common.hpp"

TEST(CompactedCompactTrie, IsPerfect) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompactedCompactTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>,
      tests::common::TestIsPerfect>();
}

TEST(CompactedCompactTrie, IsMinimal) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompactedCompactTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>,
      tests::common::TestIsMinimal>();
}

TEST(CompactedCompactTrie, IsMonotone) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompactedCompactTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>,
      tests::common::TestIsMonotone>();
}

TEST(CompactedCompactTrie, IsMMPHF) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompactedCompactTrie<std::uint64_t, exotic_hashing::support::FixedBitConverter<std::uint64_t>>,
      tests::common::TestIsMMPHF>();
}
