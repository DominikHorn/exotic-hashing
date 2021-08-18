#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(SimpleHollowTrie, IsMonotoneMinimalPerfect) {
   tests::common::run_is_monotone_minimal_perfect<
      std::uint64_t,
      exotic_hashing::SimpleHollowTrie<std::uint64_t, exotic_hashing::FixedBitConverter<std::uint64_t>>>();
}

TEST(HollowTrie, IsMonotoneMinimalPerfect) {
   tests::common::run_is_monotone_minimal_perfect<
      std::uint64_t, exotic_hashing::HollowTrie<std::uint64_t, exotic_hashing::FixedBitConverter<std::uint64_t>>>();
}
