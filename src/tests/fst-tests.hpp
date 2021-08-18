#pragma once

#include <cstdint>

#include <exotic_hashing.hpp>

#include "common.hpp"

TEST(FastSuccinctTrie, IsPerfect) {
   tests::common::run_test<std::uint64_t, exotic_hashing::FastSuccinctTrie<std::uint64_t>,
                           tests::common::TestIsPerfect>();
}

TEST(FastSuccinctTrie, IsMinimal) {
   tests::common::run_test<std::uint64_t, exotic_hashing::FastSuccinctTrie<std::uint64_t>,
                           tests::common::TestIsMinimal>();
}

TEST(FastSuccinctTrie, IsMonotone) {
   tests::common::run_test<std::uint64_t, exotic_hashing::FastSuccinctTrie<std::uint64_t>,
                           tests::common::TestIsMonotone>();
}

TEST(FastSuccinctTrie, IsMMPHF) {
   tests::common::run_test<std::uint64_t, exotic_hashing::FastSuccinctTrie<std::uint64_t>,
                           tests::common::TestIsMMPHF>();
}
