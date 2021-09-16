#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(LearnedLinear, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::LearnedLinear<std::uint64_t>, tests::common::TestIsPerfect>();
}

TEST(LearnedLinear, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::LearnedLinear<std::uint64_t>, tests::common::TestIsMinimal>();
}

TEST(LearnedLinear, IsMonotone) {
   tests::common::run_test<std::uint64_t, exotic_hashing::LearnedLinear<std::uint64_t>,
                           tests::common::TestIsMonotone>();
}

TEST(LearnedLinear, IsMMPHF) {
   tests::common::run_test<std::uint64_t, exotic_hashing::LearnedLinear<std::uint64_t>, tests::common::TestIsMMPHF>();
}
