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

   std::vector<std::vector<std::uint64_t>> test_data{{},
                                                     {0},
                                                     {0, 1},
                                                     {2, 3, 5, 7, 11, 13, 24},
                                                     {0,   1,   2,   3,   4,   5,   5,   5,   6,   7,
                                                      8,   9,   10,  10,  11,  12,  13,  200, 256, 256,
                                                      257, 258, 259, 260, 300, 511, 511, 512, 1024}};
   decltype(test_data)::value_type vec(100000, 0);
   for (size_t i = 0; i < vec.size(); i++)
      vec[i] = i;
   test_data.push_back(vec);

   for (const auto& vec : test_data) {
      EliasFanoList<std::uint64_t> efl(vec.begin(), vec.end());
      for (size_t i = 0; i < efl.size(); i++)
         EXPECT_EQ(vec[i], efl[i]);
   }
}

