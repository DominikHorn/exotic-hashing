#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include "common.hpp"

// ==== RMIRank ====
TEST(RMIRank, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::RMIRank<std::uint64_t>, tests::common::TestIsPerfect>();
}

TEST(RMIRank, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::RMIRank<std::uint64_t>, tests::common::TestIsMinimal>();
}

TEST(RMIRank, IsMonotone) {
   tests::common::run_test<std::uint64_t, exotic_hashing::RMIRank<std::uint64_t>, tests::common::TestIsMonotone>();
}

TEST(RMIRank, IsMMPHF) {
   tests::common::run_test<std::uint64_t, exotic_hashing::RMIRank<std::uint64_t>, tests::common::TestIsMMPHF>();
}

// ==== CompressedRMIRank ====
TEST(CompressedRMIRank, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompressedRMIRank<std::uint64_t>,
                           tests::common::TestIsPerfect>();
}

TEST(CompressedRMIRank, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompressedRMIRank<std::uint64_t>,
                           tests::common::TestIsMinimal>();
}

TEST(CompressedRMIRank, IsMonotone) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompressedRMIRank<std::uint64_t>,
                           tests::common::TestIsMonotone>();
}

TEST(CompressedRMIRank, IsMMPHF) {
   tests::common::run_test<std::uint64_t, exotic_hashing::CompressedRMIRank<std::uint64_t>,
                           tests::common::TestIsMMPHF>();
}
