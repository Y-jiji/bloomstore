#include<gtest/gtest.h>
#include<bloom_filter.hpp>
#include<math.h>
#include<unordered_set>
#include<iostream>
#include<random>
#include<cstring>
#include<span>
#include"./xorshift.hpp"

/// @brief verify no false positive rate is close to theoretically calculated ones. 
TEST(BloomFilter, FPRateCloseToTheory) {
    // k: # of functions
    // n: # of inserted elements
    // m: # of bits
    uint32_t k = 5, n = 200, m = 1000;
    auto fp = [](uint32_t x) {
        return static_cast<float>(x);
    };
    auto fp_rate_calculated = pow(1.0 - pow(1.0 - 1.0 / fp(m), fp(n * k)), fp(k));
    auto bloom_filter = bloomstore::BloomFilter(m, k);
    auto ground_truth = std::unordered_set<uint32_t>{};
    // randomly insert something
    auto random_number_generator = xorshift::XorShift32(5);
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t value = random_number_generator.Sample();
        ground_truth.insert(value);
        std::array<uint8_t, 4> array;
        memcpy(&array[0], &value, sizeof(value));
        bloom_filter.Insert(std::span{array});
    }
    // count false positive frequency
    uint32_t fp_count_empirical = 0;
    for (uint32_t i = 0; i < 20000; ++i) {
        uint32_t value = random_number_generator.Sample();
        std::array<uint8_t, 4> array;
        memcpy(&array[0], &value, sizeof(value));
        fp_count_empirical += 
            (bloom_filter.Test(std::span{array}) &&
            !ground_truth.contains(value));
    }
    // verify false positive rate is close to theoretical calculation
    auto fp_rate_empirical = fp(fp_count_empirical) / fp(20000);
    ASSERT_TRUE(std::abs(fp_rate_empirical - fp_rate_calculated) < 0.05);
}

/// @brief verify no false negative happens in bloom filter
TEST(BloomFilter, NoFalseNegative) {
    // k: # of functions
    // n: # of inserted elements
    // m: # of bits
    uint32_t k = 5, n = 200, m = 1000;
    auto ground_truth = std::unordered_set<uint32_t>{};
    auto bloom_filter = bloomstore::BloomFilter(m, k);
    // randomly insert something
    auto random_number_generator = xorshift::XorShift32(5);
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t value = random_number_generator.Sample();
        ground_truth.insert(value);
        std::array<uint8_t, 4> array;
        memcpy(&array[0], &value, sizeof(value));
        bloom_filter.Insert(std::span{array});
    }
    // verify no false negative
    for (auto value: ground_truth) {
        std::array<uint8_t, 4> array;
        memcpy(&array[0], &value, sizeof(value));
        ASSERT_TRUE(bloom_filter.Test(std::span{array}));
    }
}