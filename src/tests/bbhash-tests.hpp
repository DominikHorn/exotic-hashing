#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(BBHash, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::BBHash<std::uint64_t>, tests::common::TestIsPerfect>();
}

TEST(BBHash, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::BBHash<std::uint64_t>, tests::common::TestIsMinimal>();
}
