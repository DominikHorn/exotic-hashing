#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include "common.hpp"

TEST(FastSuccinctTrie, IsMonotoneMinimalPerfect) {
   tests::common::run_is_monotone_minimal_perfect<std::uint64_t, exotic_hashing::FastSuccinctTrie<std::uint64_t>>();
}
