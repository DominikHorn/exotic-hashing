#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(MWHC, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::MWHC<std::uint64_t>, tests::common::TestIsPerfect>();
}

TEST(MWHC, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::MWHC<std::uint64_t>, tests::common::TestIsMinimal>();
}

TEST(MWHC, IsOrderPreserving) {
   tests::common::run_test<std::uint64_t, exotic_hashing::MWHC<std::uint64_t>, tests::common::TestIsOrderPreserving>();
}
