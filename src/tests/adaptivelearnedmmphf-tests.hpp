#pragma once

#include <exotic_hashing.hpp>
#include <gtest/gtest.h>

#include "common.hpp"

TEST(AdaptiveLearnedMMPHF, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::AdaptiveLearnedMMPHF<std::uint64_t, 10>,
                           tests::common::TestIsPerfect>();
}

TEST(AdaptiveLearnedMMPHF, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::AdaptiveLearnedMMPHF<std::uint64_t, 10>,
                           tests::common::TestIsMinimal>();
}

TEST(AdaptiveLearnedMMPHF, IsMonotone) {
   tests::common::run_test<std::uint64_t, exotic_hashing::AdaptiveLearnedMMPHF<std::uint64_t, 10>,
                           tests::common::TestIsMonotone>();
}

TEST(AdaptiveLearnedMMPHF, IsMMPHF) {
   tests::common::run_test<std::uint64_t, exotic_hashing::AdaptiveLearnedMMPHF<std::uint64_t, 10>,
                           tests::common::TestIsMMPHF>();
}
