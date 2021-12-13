#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

// ==== Simple Hollow Trie ====

TEST(LAVector, IsPerfect) {
   using LAVector = exotic_hashing::LAVector<std::uint64_t>;
   tests::common::run_test<std::uint64_t, LAVector, tests::common::TestIsPerfect>();
}

TEST(LAVector, IsMinimal) {
   using LAVector = exotic_hashing::LAVector<std::uint64_t>;
   tests::common::run_test<std::uint64_t, LAVector, tests::common::TestIsMinimal>();
}

TEST(LAVector, IsMonotone) {
   using LAVector = exotic_hashing::LAVector<std::uint64_t>;
   tests::common::run_test<std::uint64_t, LAVector, tests::common::TestIsMonotone>();
}

TEST(LAVector, IsMMPHF) {
   using LAVector = exotic_hashing::LAVector<std::uint64_t>;
   tests::common::run_test<std::uint64_t, LAVector, tests::common::TestIsMMPHF>();
}
