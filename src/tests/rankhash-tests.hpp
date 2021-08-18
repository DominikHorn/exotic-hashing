#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(RankHash, IsMonotoneMinimalPerfect) {
   tests::common::run_is_monotone_minimal_perfect<std::uint64_t, exotic_hashing::RankHash<std::uint64_t>>();
}
