#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(CompactedMWHC, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompactedMWHC<std::uint64_t>, tests::common::TestIsPerfect>();
}

TEST(CompactedMWHC, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompactedMWHC<std::uint64_t>, tests::common::TestIsMinimal>();
}

TEST(CompactedMWHC, IsOrderPreserving) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompactedMWHC<std::uint64_t>,
                           tests::common::TestIsOrderPreserving>();
}
