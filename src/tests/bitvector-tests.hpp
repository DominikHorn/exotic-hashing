#pragma once

#include <cstdint>
#include <exotic_hashing.hpp>
#include <iostream>
#include <limits>
#include <random>

#include <gtest/gtest.h>

TEST(Bitvector, FromGenerator) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 200U, 256U, 100000U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) & 0x1;

      // initialize bitvector using generator function
      Bitvector bv(vec.size(), [&](const size_t& index) { return vec[index]; });

      for (size_t i = 0; i < bv.size(); i++)
         EXPECT_EQ(vec[i], (bool) bv[i]);
   }
}

TEST(Bitvector, FromFixedValue) {
   using namespace exotic_hashing::support;

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 200U, 256U, 100000U}) {
      for (const auto value : {true, false}) {
         Bitvector bv(size, value);

         for (size_t i = 0; i < bv.size(); i++)
            EXPECT_EQ((bool) bv[i], value);
      }
   }
}

TEST(Bitvector, FromVectorBool) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 200U, 256U, 100000U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) & 0x1;

      // initialize bitvector
      Bitvector bv(vec);
      const Bitvector bv2(vec);

      EXPECT_EQ(bv.size(), vec.size());
      EXPECT_EQ(bv2.size(), bv.size());
      for (size_t i = 0; i < bv.size(); i++) {
         EXPECT_EQ(vec[i], (bool) bv[i]);
         EXPECT_EQ(vec[i], (bool) bv2[i]);
      }
   }
}

TEST(Bitvector, Append) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 200U, 256U, 100000U}) {
      std::vector<bool> vec(size, false);
      Bitvector bv;
      for (size_t i = 0; i < size; i++) {
         vec[i] = dist(rng) & 0x1;
         bv.append(vec[i]);
      }

      EXPECT_EQ(bv.size(), vec.size());
      for (size_t i = 0; i < bv.size(); i++)
         EXPECT_EQ(vec[i], (bool) bv[i]);
   }
}

TEST(Bitvector, CountZeroes) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, 200);

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 200U, 256U, 10000U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) == 0;

      // initialize bitvector
      Bitvector bv(vec);

      EXPECT_EQ(bv.size(), vec.size());

      for (size_t i = 0; i < size; i++) {
         size_t lz = 0;
         while (lz + i < size && !vec[lz + i])
            lz++;

         EXPECT_EQ(lz, bv.count_zeroes(i));
      }
   }
}

TEST(Bitvector, ExtractBlock) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {125U, 128U, 200U, 256U, 10000U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) & 0x1;
      Bitvector bv(vec);

      for (size_t i = 0; i < size; i++) {
         for (size_t j = 1; j <= sizeof(std::uint64_t) * 8 && i + j < size; j++) {
            auto block = bv.extract_block(i, i + j);
            for (size_t k = i; k < i + j; k++) {
               EXPECT_EQ((block >> (k - i)) & 0x1, vec[k]);
            }
         }
      }
   }
}
