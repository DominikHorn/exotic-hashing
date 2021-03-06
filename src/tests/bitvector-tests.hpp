#pragma once

#include <algorithm>
#include <cstdint>
#include <exotic_hashing.hpp>
#include <iostream>
#include <limits>
#include <random>

#include <gtest/gtest.h>

// ==== Bitvector ====
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

TEST(Bitvector, AppendSingle) {
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
         EXPECT_EQ(vec[i], bv[i]);
   }
}

TEST(Bitvector, AppendMultiple) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 200U, 256U, 100000U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) & 0x1;

      for (size_t cnt = 1; cnt < size && cnt <= sizeof(std::uint64_t) * 8; cnt++) {
         std::uint64_t data = 0x0;
         for (size_t i = 0; i < cnt; i++) {
            data <<= 1;
            data |= vec[cnt - i - 1] & 0x1;
         }

         Bitvector empty_bv;
         Bitvector bv(size * 3, false);

         empty_bv.append(data, cnt);
         bv.append(data, cnt);

         EXPECT_EQ(empty_bv.size(), cnt);
         EXPECT_EQ(bv.size(), cnt + size * 3);
         for (size_t i = 0; i < empty_bv.size(); i++) {
            EXPECT_EQ(empty_bv[i], vec[i]);
            EXPECT_EQ(bv[i + size * 3], vec[i]);
         }

         for (size_t i = 0; i < size * 3; i++)
            EXPECT_EQ(bv[i], false);
      }
   }
}

TEST(Bitvector, AppendOtherBitvector) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 200U, 256U, 100000U}) {
      Bitvector empty_bv;
      Bitvector bv(size * 3, false);

      Bitvector<> other(size, [&](const size_t& /*index*/) { return dist(rng) & 0x1; });

      empty_bv.append(other);
      bv.append(other);

      EXPECT_EQ(empty_bv.size(), other.size());
      EXPECT_EQ(bv.size(), other.size() + size * 3);

      for (size_t i = 0; i < empty_bv.size(); i++) {
         EXPECT_EQ(empty_bv[i], other[i]);
         EXPECT_EQ(bv[i + size * 3], other[i]);
      }

      for (size_t i = 0; i < size * 3; i++)
         EXPECT_EQ(bv[i], false);
   }
}

TEST(Bitvector, CountZeroes) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, 32);

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

TEST(Bitvector, Extract) {
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
            auto block = bv.extract(i, i + j);
            for (size_t k = i; k < i + j; k++) {
               EXPECT_EQ((block >> (k - i)) & 0x1, vec[k]);
            }
         }
      }
   }
}

TEST(Bitvector, Matches) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {0U, 1U, 7U, 8U, 63U, 64U, 128U, 200U, 256U, 10000U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) & 0x1;
      Bitvector bv(vec);

      for (size_t prefix_size = 0; prefix_size <= 137; prefix_size++) {
         const auto upper = bv.size() > 0 ? bv.size() - 1 : 0;
         std::uniform_int_distribution<size_t> dist(0, upper);
         size_t prefix_start = dist(rng);
         Bitvector prefix(prefix_size, false);
         for (size_t i = 0; i < prefix_size; i++)
            prefix[i] = i + prefix_start < vec.size() ? vec[i + prefix_start] & 0x1 : dist(rng) & 0x1;

         EXPECT_TRUE(bv.matches(prefix, prefix_start));

         if (prefix_size > 0) {
            auto wrong_prefix = prefix;
            for (size_t i = 0; i < prefix_size; i++)
               wrong_prefix[i] = !prefix[i];

            EXPECT_EQ(bv.matches(wrong_prefix, prefix_start), bv.size() == 0);
         }
      }
   }
}

// ==== FixedBitvector ====
TEST(FixedBitvector, FromGenerator) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 200U, 256U, 10000U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) & 0x1;

      // initialize bitvector using generator function
      FixedBitvector<10000 + 1> bv(vec.size(), [&](const size_t& index) { return vec[index]; });

      for (size_t i = 0; i < bv.size() && i < vec.size(); i++)
         EXPECT_EQ(vec[i], (bool) bv[i]);
   }
}

TEST(FixedBitvector, FromFixedValue) {
   using namespace exotic_hashing::support;

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 129U}) {
      for (const auto value : {true, false}) {
         FixedBitvector<129> bv(size, value);

         for (size_t i = 0; i < bv.size(); i++)
            EXPECT_EQ((bool) bv[i], value);
      }
   }
}

TEST(FixedBitvector, FromVectorBool) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 200U, 256U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) & 0x1;

      // initialize bitvector
      FixedBitvector<257> bv(vec);
      const FixedBitvector<257> bv2(vec);

      EXPECT_EQ(bv.size(), vec.size());
      EXPECT_EQ(bv2.size(), bv.size());
      for (size_t i = 0; i < bv.size() && i < vec.size(); i++) {
         EXPECT_EQ(vec[i], (bool) bv[i]);
         EXPECT_EQ(vec[i], (bool) bv2[i]);
      }
   }
}

TEST(FixedBitvector, CountZeroes) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, 32);

   for (const auto size : {8U, 63U, 64U, 65U, 125U, 128U, 200U, 256U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) == 0;

      // initialize bitvector
      FixedBitvector<256> bv(vec);

      EXPECT_EQ(bv.size(), vec.size());

      for (size_t i = 0; i < size; i++) {
         size_t lz = 0;
         while (lz + i < size && !vec[lz + i])
            lz++;

         EXPECT_EQ(lz, bv.count_zeroes(i));
      }
   }
}

TEST(FixedBitvector, Extract) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {125U, 128U, 200U, 256U, 10000U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) & 0x1;
      FixedBitvector<10000> bv(vec);

      for (size_t i = 0; i < size; i++) {
         for (size_t j = 1; j <= sizeof(std::uint64_t) * 8 && i + j < size; j++) {
            auto block = bv.extract(i, i + j);
            for (size_t k = i; k < i + j; k++) {
               EXPECT_EQ((block >> (k - i)) & 0x1, vec[k]);
            }
         }
      }
   }
}

TEST(FixedBitvector, Matches) {
   using namespace exotic_hashing::support;

   std::random_device rd;
   std::default_random_engine rng(rd());
   std::uniform_int_distribution<size_t> dist(0, std::numeric_limits<size_t>::max());

   for (const auto size : {0U, 1U, 7U, 8U, 63U, 64U, 128U, 200U, 256U, 10000U}) {
      std::vector<bool> vec(size, false);
      for (size_t i = 0; i < size; i++)
         vec[i] = dist(rng) & 0x1;
      FixedBitvector<10000> bv(vec);

      for (size_t prefix_size = 0; prefix_size <= 137; prefix_size++) {
         const auto upper = bv.size() > 0 ? bv.size() - 1 : 0;
         std::uniform_int_distribution<size_t> dist(0, upper);
         size_t prefix_start = dist(rng);
         FixedBitvector<10000> prefix(prefix_size, false);
         for (size_t i = 0; i < prefix_size; i++)
            prefix[i] = i + prefix_start < vec.size() ? vec[i + prefix_start] & 0x1 : dist(rng) & 0x1;

         EXPECT_TRUE(bv.matches(prefix, prefix_start));

         if (prefix_size > 0) {
            auto wrong_prefix = prefix;
            for (size_t i = 0; i < prefix_size; i++)
               wrong_prefix[i] = !prefix[i];

            EXPECT_EQ(bv.matches(wrong_prefix, prefix_start), bv.size() == 0);
         }
      }
   }
}
