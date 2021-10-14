#pragma once

#include <exotic_hashing.hpp>
#include <gtest/gtest.h>

#include "include/support/clustering.hpp"

TEST(Clustering, DensityGaugeAlwaysInRange) {
   using namespace exotic_hashing::support;
   using Data = std::uint64_t;

   const std::vector<Data> test_data{0, 1, 2, 3, 4, 5, 6, 10};
   DensityGauge density;

   for (auto begin_it = test_data.begin(); begin_it < test_data.end(); begin_it++) {
      for (auto end_it = begin_it + 1; end_it < test_data.end(); end_it++) {
         const auto res = density(begin_it, end_it);
         EXPECT_GE(res, 0.0);
         EXPECT_LE(res, 1.0);
      }
   }
}

TEST(Clustering, DensityGaugeSample) {
   using namespace exotic_hashing::support;
   using Data = std::uint64_t;

   const std::vector<Data> test_data{0, 1, 2, 3, 4, 5, 6, 10};
   DensityGauge density;

   // |X| / (max(X) - min(X))
   EXPECT_EQ(density(test_data.begin(), test_data.end()), 8. / (10. - 0. + 1));
   EXPECT_EQ(density(test_data.begin(), test_data.begin() + 5), 5. / (4. - 0. + 1));
   EXPECT_EQ(density(test_data.begin() + 6, test_data.end()), 2. / (10. - 6. + 1));
}

TEST(Clustering, EmptyVecZeroClusters) {
   using namespace exotic_hashing::support;
   using Data = std::uint64_t;

   const std::vector<Data> test_data{};

   auto clusters = cluster(test_data.begin(), test_data.end(), 0.0);
   EXPECT_TRUE(clusters.empty());
   clusters = cluster(test_data.begin(), test_data.end(), 1.0);
   EXPECT_TRUE(clusters.empty());
}

TEST(Clustering, SingleElemVecZeroClusters) {
   using namespace exotic_hashing::support;
   using Data = std::uint64_t;

   const std::vector<Data> test_data{0};

   auto clusters = cluster(test_data.begin(), test_data.end(), 0.0);
   EXPECT_TRUE(clusters.size() == 2);
   EXPECT_TRUE(clusters[0] == test_data.begin());
   EXPECT_TRUE(clusters[clusters.size() - 1] == test_data.end());

   clusters = cluster(test_data.begin(), test_data.end(), 1.0);
   EXPECT_TRUE(clusters.size() == 2);
   EXPECT_TRUE(clusters[0] == test_data.begin());
   EXPECT_TRUE(clusters[clusters.size() - 1] == test_data.end());
}

TEST(Clustering, DenseDataAllClustered) {
   using namespace exotic_hashing::support;
   using Data = std::uint64_t;

   std::vector<Data> test_data(100, 0);
   for (Data i = 0; i < test_data.size(); i++)
      test_data[i] = i;

   auto clusters = cluster(test_data.begin(), test_data.end(), 0.0);
   EXPECT_EQ(clusters.size(), 2);
   EXPECT_TRUE(clusters[0] == test_data.begin());
   EXPECT_TRUE(clusters[clusters.size() - 1] == test_data.end());

   clusters = cluster(test_data.begin(), test_data.end(), 0.5);
   EXPECT_EQ(clusters.size(), 2);
   EXPECT_TRUE(clusters[0] == test_data.begin());
   EXPECT_TRUE(clusters[clusters.size() - 1] == test_data.end());

   clusters = cluster(test_data.begin(), test_data.end(), 1.0);
   EXPECT_EQ(clusters.size(), 2);
   EXPECT_TRUE(clusters[0] == test_data.begin());
   EXPECT_TRUE(clusters[clusters.size() - 1] == test_data.end());
}

TEST(Clustering, ClustersObeyDensity) {
   using namespace exotic_hashing::support;
   using Data = std::uint64_t;

   std::unordered_map<size_t, size_t> params({{100, 12}, {101, 12}, {102, 13}, {103, 13}, {104, 13}});
   for (const auto& td : params) {
      std::vector<Data> test_data(td.first, 0);
      for (Data i = 0, offset = 0; i < test_data.size(); i++) {
         test_data[i] = i + offset;

         if (i % 10 == 0)
            offset += 100;
      }

      auto clusters = cluster(test_data.begin(), test_data.end(), 0.0);
      EXPECT_EQ(clusters.size(), 2);
      EXPECT_TRUE(clusters[0] == test_data.begin());
      EXPECT_TRUE(clusters[clusters.size() - 1] == test_data.end());

      clusters = cluster(test_data.begin(), test_data.end(), 0.08);
      EXPECT_EQ(clusters.size(), 2);
      EXPECT_TRUE(clusters[0] == test_data.begin());
      EXPECT_TRUE(clusters[clusters.size() - 1] == test_data.end());

      const auto threshold = 0.2;
      clusters = cluster(test_data.begin(), test_data.end(), threshold);
      EXPECT_EQ(clusters.size(), td.second);
      EXPECT_TRUE(clusters[0] == test_data.begin());
      EXPECT_TRUE(clusters[clusters.size() - 1] == test_data.end());
      DensityGauge density;
      for (size_t i = 1; i < clusters.size(); i++)
         EXPECT_GE(density(clusters[i - 1], clusters[i]), threshold);
   }
}
