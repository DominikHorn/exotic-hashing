#pragma once

#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

#include "include/support/elias_fano_list.hpp"

/// tests whether size function is correct
TEST(EliasFanoList, Size) {
   using namespace exotic_hashing::support;

   std::vector<std::vector<std::uint64_t>> test_data{{}, {0}, {0, 1}, {2, 3, 5, 7, 11, 13, 24}};
   for (const auto& vec : test_data) {
      EliasFanoList<std::uint64_t> efl(vec.begin(), vec.end());
      EXPECT_EQ(efl.size(), vec.size());
   }
}
/// tests whether x = elias_fano_list[indexOf(x)]
TEST(EliasFanoList, Access) {
   using namespace exotic_hashing::support;

   std::vector<std::vector<std::uint64_t>> test_data{{}, {0}, {0, 1}, {2, 3, 5, 7, 11, 13, 24}};
   for (const auto& vec : test_data) {
      EliasFanoList<std::uint64_t> efl(vec.begin(), vec.end());
      for (size_t i = 0; i < efl.size(); i++)
         EXPECT_EQ(vec[i], efl[i]);
   }
}

