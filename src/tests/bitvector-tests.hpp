
#pragma once

#include <exotic_hashing.hpp>
#include <iostream>
#include <limits>
#include <random>

#include <gtest/gtest.h>

TEST(Bitvector, FromVectorBool) {
   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 200U, 256U, 100000U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) & 0x1;

      // initialize bitvector
      exotic_hashing::Bitvector bv(vec);
      const exotic_hashing::Bitvector bv2(vec);

      EXPECT_EQ(bv.size(), vec.size());
      EXPECT_EQ(bv2.size(), bv.size());
      for (size_t i = 0; i < bv.size(); i++) {
         EXPECT_EQ(vec[i], (bool) bv[i]);
         EXPECT_EQ(vec[i], (bool) bv2[i]);
      }
   }
}

TEST(Bitvector, CountZeroes) {
   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 200U, 256U, 10000U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) & 0x1;

      // initialize bitvector
      exotic_hashing::Bitvector bv(vec);

      EXPECT_EQ(bv.size(), vec.size());

      for (size_t i = 0; i < size; i++) {
         size_t lz = 0;
         while (lz + i < size && !vec[lz + i])
            lz++;

         EXPECT_EQ(lz, bv.count_zeroes(i));
      }
   }
}
