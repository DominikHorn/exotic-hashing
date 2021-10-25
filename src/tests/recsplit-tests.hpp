#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(Recsplit, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::BitMWHC<std::uint64_t>, tests::common::TestIsPerfect>();
}

TEST(Recsplit, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::RecSplit<std::uint64_t>, tests::common::TestIsMinimal>();
}
