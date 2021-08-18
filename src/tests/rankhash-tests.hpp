#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(RankHash, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::RankHash<std::uint64_t>, tests::common::TestIsPerfect>();
}

TEST(RankHash, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::RankHash<std::uint64_t>, tests::common::TestIsMinimal>();
}

TEST(RankHash, IsMonotone) {
   tests::common::run_test<std::uint64_t, exotic_hashing::RankHash<std::uint64_t>, tests::common::TestIsMonotone>();
}

TEST(RankHash, IsMMPHF) {
   tests::common::run_test<std::uint64_t, exotic_hashing::RankHash<std::uint64_t>, tests::common::TestIsMMPHF>();
}
