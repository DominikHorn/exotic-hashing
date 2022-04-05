#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(RankedBitMWHC, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::RankedBitMWHC<std::uint64_t>, tests::common::TestIsPerfect>();
}

TEST(RankedBitMWHC, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::RankedBitMWHC<std::uint64_t>, tests::common::TestIsMinimal>();
}
