#pragma once

#include <exotic_hashing.hpp>

#include <gtest/gtest.h>

/// Basic x = decode(encode(x)) test
TEST(EliasCoding, GammaIdempotency) {
   using namespace exotic_hashing::support;

   std::vector<std::uint64_t> test_data{1,    2,    3,    4,         5,
                                        8,    10,   16,   32,        64,
                                        100,  128,  256,  512,       1000,
                                        1024, 2048, 4096, 200000000, std::numeric_limits<std::uint64_t>::max()};

   for (std::uint64_t original : test_data) {
      auto enc = EliasGammaCoder::encode(original);
      auto [dec, bits] = EliasGammaCoder::decode(enc);

      EXPECT_EQ(dec, original);
      EXPECT_EQ(bits, enc.size());
   }
}

/// Test correct behaviour when gamma encoding is embedded into a bit stream, i.e.,
/// more irrelevant bits exist at the start or end of the stream
TEST(EliasCoding, GammaEmbedded) {
   using namespace exotic_hashing::support;

   std::vector<std::uint64_t> test_data{1,    2,    3,    4,         5,
                                        8,    10,   16,   32,        64,
                                        100,  128,  256,  512,       1000,
                                        1024, 2048, 4096, 200000000, std::numeric_limits<std::uint64_t>::max()};

   for (std::uint64_t original : test_data) {
      auto enc = EliasGammaCoder::encode(original);
      auto [dec, bits] = EliasGammaCoder::decode(enc);

      EXPECT_EQ(dec, original);
      EXPECT_EQ(bits, enc.size());

      for (size_t i = 0; i < 10; i++)
         enc.push_back(i & 0x1);

      auto [dec2, bits2] = EliasGammaCoder::decode(enc);

      EXPECT_EQ(dec2, original);
      EXPECT_EQ(bits2, bits);

      std::vector<bool> rep;
      const size_t start = 7;
      for (size_t i = 0; i < start; i++)
         rep.push_back(i & 0x1);
      for (const auto bit : enc)
         rep.push_back(bit);

      auto [dec3, bits3] = EliasGammaCoder::decode(rep, start);

      EXPECT_EQ(dec3, original);
      EXPECT_EQ(bits3, bits);
   }
}

/// Basic x = decode(encode(x)) test
TEST(EliasCoding, DeltaIdempotency) {
   using namespace exotic_hashing::support;

   std::vector<std::uint64_t> test_data{1,    2,    3,    4,         5,
                                        8,    10,   16,   32,        64,
                                        100,  128,  256,  512,       1000,
                                        1024, 2048, 4096, 200000000, std::numeric_limits<std::uint64_t>::max()};

   for (std::uint64_t original : test_data) {
      auto enc = EliasDeltaCoder::encode(original);
      auto [dec, bits] = EliasDeltaCoder::decode(enc);

      EXPECT_EQ(dec, original);
      EXPECT_EQ(bits, enc.size());
   }
}

/// Test correct behaviour when gamma encoding is embedded into a bit stream, i.e.,
/// more irrelevant bits exist at the beginning or end of the stream
TEST(EliasCoding, DeltaEmbedded) {
   using namespace exotic_hashing::support;

   std::vector<std::uint64_t> test_data{1,    2,    3,    4,         5,
                                        8,    10,   16,   32,        64,
                                        100,  128,  256,  512,       1000,
                                        1024, 2048, 4096, 200000000, std::numeric_limits<std::uint64_t>::max()};

   for (std::uint64_t original : test_data) {
      auto enc = EliasDeltaCoder::encode(original);
      auto [dec, bits] = EliasDeltaCoder::decode(enc);

      EXPECT_EQ(dec, original);
      EXPECT_EQ(bits, enc.size());

      for (size_t i = 0; i < 10; i++)
         enc.push_back(i & 0x1);

      auto [dec2, bits2] = EliasDeltaCoder::decode(enc);

      EXPECT_EQ(dec2, original);
      EXPECT_EQ(bits2, bits);

      std::vector<bool> rep;
      const size_t start = 7;
      for (size_t i = 0; i < start; i++)
         rep.push_back(i & 0x1);
      for (const auto bit : enc)
         rep.push_back(bit);

      auto [dec3, bits3] = EliasDeltaCoder::decode(rep, start);

      EXPECT_EQ(dec3, original);
      EXPECT_EQ(bits3, bits);
   }
}

