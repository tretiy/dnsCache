#include "DnsCache.h"
#include <gtest/gtest.h>
#include <thread>
#include <fmt/core.h>

class dnsCacheTest : public testing::Test {
protected:
    dnsCacheTest() : cache(100)
    {}

    DNSCache cache;
};

TEST_F(dnsCacheTest, BasicCheck)
{
    EXPECT_EQ(cache.resolve("testItem").size(), 0);
    for (int i = 0; i < 1000; ++i)
        cache.update("localhost" + std::to_string(i), "127.0.0." + std::to_string(i));
    EXPECT_TRUE(cache.resolve("localhost999") == "127.0.0.999");
}
auto threadOperation = [](DNSCache &cacheInst, int len, const std::string &nameFmt, const std::string &ipFmt) {
    for (int i = 0; i < len; ++i) {
        cacheInst.update(fmt::format(fmt::runtime(nameFmt), i), fmt::format(fmt::runtime(ipFmt), i));
    }
};
TEST_F(dnsCacheTest, ThreadsCheck)
{
    std::thread thread1(threadOperation, std::ref(cache), 100, "localhost{}", "127.0.0.{}");
    std::thread thread2(threadOperation, std::ref(cache), 100, "localhost{}", "127.0.{}.0");
    thread1.join();
    thread2.join();
    EXPECT_TRUE(cache.resolve("localhost999") == "127.0.0.999");
}