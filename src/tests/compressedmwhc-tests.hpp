#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(CompressedMWHC, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompressedMWHC<std::uint64_t>,
                           tests::common::TestIsPerfect>();
}

TEST(CompressedMWHC, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompressedMWHC<std::uint64_t>,
                           tests::common::TestIsMinimal>();
}

TEST(CompressedMWHC, IsOrderPreserving) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompressedMWHC<std::uint64_t>,
                           tests::common::TestIsOrderPreserving>();
}
