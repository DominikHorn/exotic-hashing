#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>
#include <learned_hashing.hpp>

#include "common.hpp"

// ==== LearnedRankRMI ====
TEST(LearnedRankRMI, IsPerfect) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::LearnedRank<std::uint64_t, learned_hashing::MonotoneRMIHash<std::uint64_t, 1000000>>,
      tests::common::TestIsPerfect>();
}

TEST(LearnedRankRMI, IsMinimal) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::LearnedRank<std::uint64_t, learned_hashing::MonotoneRMIHash<std::uint64_t, 1000000>>,
      tests::common::TestIsMinimal>();
}

TEST(LearnedRankRMI, IsMonotone) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::LearnedRank<std::uint64_t, learned_hashing::MonotoneRMIHash<std::uint64_t, 1000000>>,
      tests::common::TestIsMonotone>();
}

TEST(LearnedRankRMI, IsMMPHF) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::LearnedRank<std::uint64_t, learned_hashing::MonotoneRMIHash<std::uint64_t, 1000000>>,
      tests::common::TestIsMMPHF>();
}

// ==== CompressedLearnedRankRMI ====
TEST(CompressedLearnedRankRMI, IsPerfect) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompressedLearnedRank<std::uint64_t, learned_hashing::MonotoneRMIHash<std::uint64_t, 1000000>>,
      tests::common::TestIsPerfect>();
}

TEST(CompressedLearnedRankRMI, IsMinimal) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompressedLearnedRank<std::uint64_t, learned_hashing::MonotoneRMIHash<std::uint64_t, 1000000>>,
      tests::common::TestIsMinimal>();
}

TEST(CompressedLearnedRankRMI, IsMonotone) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompressedLearnedRank<std::uint64_t, learned_hashing::MonotoneRMIHash<std::uint64_t, 1000000>>,
      tests::common::TestIsMonotone>();
}

TEST(CompressedLearnedRankRMI, IsMMPHF) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompressedLearnedRank<std::uint64_t, learned_hashing::MonotoneRMIHash<std::uint64_t, 1000000>>,
      tests::common::TestIsMMPHF>();
}

// ==== LearnedRankRadixSpline ====
TEST(LearnedRankRadixSpline, IsPerfect) {
   tests::common::run_test<std::uint64_t,
                           exotic_hashing::LearnedRank<std::uint64_t, learned_hashing::RadixSplineHash<std::uint64_t>>,
                           tests::common::TestIsPerfect>();
}

TEST(LearnedRankRadixSpline, IsMinimal) {
   tests::common::run_test<std::uint64_t,
                           exotic_hashing::LearnedRank<std::uint64_t, learned_hashing::RadixSplineHash<std::uint64_t>>,
                           tests::common::TestIsMinimal>();
}

TEST(LearnedRankRadixSpline, IsMonotone) {
   tests::common::run_test<std::uint64_t,
                           exotic_hashing::LearnedRank<std::uint64_t, learned_hashing::RadixSplineHash<std::uint64_t>>,
                           tests::common::TestIsMonotone>();
}

TEST(LearnedRankRadixSpline, IsMMPHF) {
   tests::common::run_test<std::uint64_t,
                           exotic_hashing::LearnedRank<std::uint64_t, learned_hashing::RadixSplineHash<std::uint64_t>>,
                           tests::common::TestIsMMPHF>();
}

// ==== CompressedLearnedRankRadixSpline ====
TEST(CompressedLearnedRankRadixSpline, IsPerfect) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompressedLearnedRank<std::uint64_t, learned_hashing::RadixSplineHash<std::uint64_t>>,
      tests::common::TestIsPerfect>();
}

TEST(CompressedLearnedRankRadixSpline, IsMinimal) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompressedLearnedRank<std::uint64_t, learned_hashing::RadixSplineHash<std::uint64_t>>,
      tests::common::TestIsMinimal>();
}

TEST(CompressedLearnedRankRadixSpline, IsMonotone) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompressedLearnedRank<std::uint64_t, learned_hashing::RadixSplineHash<std::uint64_t>>,
      tests::common::TestIsMonotone>();
}

TEST(CompressedLearnedRankRadixSpline, IsMMPHF) {
   tests::common::run_test<
      std::uint64_t,
      exotic_hashing::CompressedLearnedRank<std::uint64_t, learned_hashing::RadixSplineHash<std::uint64_t>>,
      tests::common::TestIsMMPHF>();
}
