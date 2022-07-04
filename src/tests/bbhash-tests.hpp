#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(BBHash, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::BBHash<std::uint64_t, std::ratio<1, 1>>,
                           tests::common::TestIsPerfect>();
   tests::common::run_test<std::uint64_t, exotic_hashing::BBHash<std::uint64_t, std::ratio<2, 1>>,
                           tests::common::TestIsPerfect>();
}

TEST(BBHash, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::BBHash<std::uint64_t, std::ratio<1, 1>>,
                           tests::common::TestIsMinimal>();
   tests::common::run_test<std::uint64_t, exotic_hashing::BBHash<std::uint64_t, std::ratio<2, 1>>,
                           tests::common::TestIsMinimal>();
}
