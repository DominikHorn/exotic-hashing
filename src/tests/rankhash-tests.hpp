#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

// ==== RankHash ====
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

// ==== CompressedRankHash ====
TEST(CompressedRankHash, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompressedRankHash<std::uint64_t>,
                           tests::common::TestIsPerfect>();
}

TEST(CompressedRankHash, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompressedRankHash<std::uint64_t>,
                           tests::common::TestIsMinimal>();
}

TEST(CompressedRankHash, IsMonotone) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompressedRankHash<std::uint64_t>,
                           tests::common::TestIsMonotone>();
}

TEST(CompressedRankHash, IsMMPHF) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompressedRankHash<std::uint64_t>,
                           tests::common::TestIsMMPHF>();
}
