#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

TEST(MapOMPHF, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::MapOMPHF<std::uint64_t>, tests::common::TestIsPerfect>();
}

TEST(MapOMPHF, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::MapOMPHF<std::uint64_t>, tests::common::TestIsMinimal>();
}

TEST(MapOMPHF, IsOrderPreserving) {
   tests::common::run_test<std::uint64_t, exotic_hashing::MapOMPHF<std::uint64_t>,
                           tests::common::TestIsOrderPreserving>();
}
