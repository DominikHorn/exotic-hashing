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

      size_t bit_index = 0;
      auto dec = EliasGammaCoder::decode(enc, bit_index);

      EXPECT_EQ(dec, original);
      EXPECT_EQ(bit_index, enc.size());
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
      size_t bit_index = 0;
      auto dec = EliasGammaCoder::decode(enc, bit_index);

      EXPECT_EQ(dec, original);
      EXPECT_EQ(bit_index, enc.size());

      for (size_t i = 0; i < 10; i++)
         enc.append(i & 0x1);

      size_t bit_index2 = 0;
      auto dec2 = EliasGammaCoder::decode(enc, bit_index2);

      EXPECT_EQ(dec2, original);
      EXPECT_EQ(bit_index2, bit_index);

      Bitvector rep;
      const size_t prefix_size = 7;
      for (size_t i = 0; i < prefix_size; i++)
         rep.append(i & 0x1);
      for (size_t i = 0; i < enc.size(); i++)
         rep.append(enc[i]);

      size_t bit_index3 = prefix_size;
      auto dec3 = EliasGammaCoder::decode(rep, bit_index3);

      EXPECT_EQ(dec3, original);
      EXPECT_EQ(bit_index3, bit_index + prefix_size);
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
      size_t bit_index = 0;
      auto dec = EliasDeltaCoder::decode(enc, bit_index);

      EXPECT_EQ(dec, original);
      EXPECT_EQ(bit_index, enc.size());
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
      size_t bit_index = 0;
      auto dec = EliasDeltaCoder::decode(enc, bit_index);

      EXPECT_EQ(dec, original);
      EXPECT_EQ(bit_index, enc.size());

      for (size_t i = 0; i < 10; i++)
         enc.append(i & 0x1);

      size_t bit_index2 = 0;
      auto dec2 = EliasDeltaCoder::decode(enc, bit_index2);

      EXPECT_EQ(dec2, original);
      EXPECT_EQ(bit_index2, bit_index);

      Bitvector rep;
      const size_t prefix_size = 7;
      for (size_t i = 0; i < prefix_size; i++)
         rep.append(i & 0x1);
      for (size_t i = 0; i < enc.size(); i++)
         rep.append(enc[i]);

      size_t bit_index3 = prefix_size;
      auto dec3 = EliasDeltaCoder::decode(rep, bit_index3);

      EXPECT_EQ(dec3, original);
      EXPECT_EQ(bit_index3, bit_index + prefix_size);
   }
}

