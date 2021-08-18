#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include "common.hpp"

TEST(CompactTrie, IsMonotoneMinimalPerfect) {
   tests::common::run_is_monotone_minimal_perfect<
      std::uint64_t, exotic_hashing::CompactTrie<std::uint64_t, exotic_hashing::FixedBitConverter<std::uint64_t>>>();
}
