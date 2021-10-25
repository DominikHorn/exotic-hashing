#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(BitMWHC, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::BitMWHC<std::uint64_t>, tests::common::TestIsPerfect>();
}
