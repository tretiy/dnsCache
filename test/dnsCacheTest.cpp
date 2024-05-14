#include "DnsCache.h"
#include <fmt/core.h>
#include <gtest/gtest.h>
#include <limits>
#include <string>
#include <thread>

class CacheSizeAsParamTest : public testing::TestWithParam<size_t> {
protected:
  CacheSizeAsParamTest() : cache(GetParam()) {}

  DNSCache cache;
};

TEST_P(CacheSizeAsParamTest, BasicCheck) {
  EXPECT_EQ(cache.resolve("testItem").size(), 0);
  auto paramValue = GetParam();
  for (int i = 0; i < paramValue; ++i) {
    cache.update("localhost" + std::to_string(i),
                 "127.0.0." + std::to_string(i));
  }
  --paramValue;
  EXPECT_TRUE(cache.resolve("localhost" + std::to_string(paramValue)) ==
              "127.0.0." + std::to_string(paramValue));
}
auto threadOperation = [](DNSCache &cacheInst, int len,
                          const std::string &nameFmt,
                          const std::string &ipFmt) {
  for (int i = 0; i < len; ++i) {
    cacheInst.update(fmt::format(fmt::runtime(nameFmt), i),
                     fmt::format(fmt::runtime(ipFmt), i));
  }
};
TEST_P(CacheSizeAsParamTest, FourThreadsUpdate) {
  auto paramValue = GetParam();
  std::thread thread1(threadOperation, std::ref(cache), paramValue,
                      "localhost{}", "127.0.{}.0");
  std::thread thread2(threadOperation, std::ref(cache), paramValue,
                      "localhost{}", "127.{}.1.0");
  std::thread thread3(threadOperation, std::ref(cache), paramValue,
                      "localhost{}", "127.{}.0.0");

  std::thread thread4(threadOperation, std::ref(cache), 10 * paramValue,
                      "localhost{}", "127.0.0.{}");
  thread1.join();
  thread2.join();
  thread3.join();
  thread4.join();
  EXPECT_TRUE(
      cache.resolve("localhost" + std::to_string(10 * paramValue - 1)) ==
      "127.0.0." + std::to_string(10 * paramValue - 1));
};

INSTANTIATE_TEST_SUITE_P(CacheSizesTests, CacheSizeAsParamTest,
                         ::testing::Values(1, 100, 1000, 10000, 100000,
                                           1000000));