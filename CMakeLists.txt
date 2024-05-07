cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 20)
project(dnsCache)
include_directories(dnsCache)

add_library(dnsCacheLib dnsCache/DnsCache.cpp)

# testing
enable_testing()
find_package(GTest REQUIRED)
find_package(fmt REQUIRED)

add_executable(dnsCacheTest test/dnsCacheTest.cpp)
target_link_libraries(dnsCacheTest GTest::gtest GTest::gtest_main fmt::fmt dnsCacheLib)
gtest_add_tests(TARGET dnsCacheTest SOURCES test/dnsCacheTest.cpp)