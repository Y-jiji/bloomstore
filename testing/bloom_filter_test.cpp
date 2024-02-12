#include<gtest/gtest.h>
#include<bloom_filter.hpp>
#include<math.h>
#include<unordered_set>
#include<iostream>
#include<random>
#include<cstring>
#include<span>
#include"./xorshift.hpp"

#define ARR std::array<uint8_t, 4>

uint32_t to_uint32(std::array<uint8_t, 4>& value) {
    uint32_t xvalue = 0x77777777;
    memcpy(&xvalue, &value, sizeof(uint32_t));
    return xvalue;
}

std::array<uint8_t, 4> to_arr(uint32_t xvalue) {
    auto value = ARR();
    memcpy(&value, &xvalue, sizeof(uint32_t));
    return value;
}

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
    ASSERT_TRUE(std::abs(fp_rate_empirical - fp_rate_calculated) < 0.01);
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
    for (int i = 0; i < n; ++i) {
        uint32_t value = random_number_generator.Sample();
        ground_truth.insert(value);
        auto array = to_arr(value);
        bloom_filter.Insert(std::span{array});
    }
    // verify no false negative
    for (auto value: ground_truth) {
        auto array = to_arr(value);
        ASSERT_TRUE(bloom_filter.Test(std::span{array}));
    }
}

/// @brief verify if bloomchain behave the same as seperate bloom filters on join
TEST(BloomChain, JoinConsistency) {
    auto random_number_generator = xorshift::XorShift32(5);
    uint32_t k = 5, n = 200, m = 1000;
    auto bloom_chain = bloomstore::BloomChain(m, k, 1024);
    auto bloom_vector = std::vector<bloomstore::BloomFilter>();
    for (int i = 0; i < 64; ++i) {
        auto bloom_filter = bloomstore::BloomFilter(m, k);
        for (int j = 0; j < n; ++j) {
            auto key = to_arr(random_number_generator.Sample());
            bloom_filter.Insert(std::span{key});
        }
        bloom_vector.push_back(bloom_filter);
        bloom_chain.Join(std::move(bloom_filter), i);
    }
    for (int i = 0; i < 20000; ++i) {
        auto key = to_arr(random_number_generator.Sample());
        auto test_vector_positive = std::vector<int>();
        for (int j = 0; j < 64; ++j) {
            if (bloom_vector[j].Test(std::span{key})) {
                test_vector_positive.push_back(j);
            }
        }
        auto test_chain_positive = std::vector<int>();
        auto chain_iter = bloom_chain.Test(std::span{key});
        bool depleted = false;
        while (!depleted) {
            size_t address = 0x7777;
            chain_iter.Next(address, depleted);
            if (!depleted) { test_chain_positive.push_back(address); }
        }
        ASSERT_EQ(test_vector_positive, test_chain_positive);;
    }
}

TEST(BloomChain, DumpLoadConsistency) {
    auto random_number_generator = xorshift::XorShift32(5);
    uint32_t k = 5, n = 200, m = 1000;
    auto bloom_chain = bloomstore::BloomChain(m, k, 1024);
    auto bloom_vector = std::vector<bloomstore::BloomFilter>();
    auto path = std::string{"/tmp/bloomstore-test-file"};
    auto file = FileObject(path);
    for (int i = 0; i < 64; ++i) {
        auto bloom_filter = bloomstore::BloomFilter(m, k);
        for (int j = 0; j < n; ++j) {
            auto key = to_arr(random_number_generator.Sample());
            bloom_filter.Insert(std::span{key});
        }
        bloom_vector.push_back(bloom_filter);
        bloom_chain.Join(std::move(bloom_filter), i);
    }
    auto start = file.Size();
    bloom_chain.Dump(file);
    ASSERT_FALSE(bloom_chain.IsFull());
    bloom_chain.Load([&](std::span<uint8_t> span) {
        file.Read(start, span);
    });
    for (int i = 0; i < 20000; ++i) {
        auto key = to_arr(random_number_generator.Sample());
        auto test_vector_positive = std::vector<int>();
        for (int j = 0; j < 64; ++j) {
            if (bloom_vector[j].Test(std::span{key})) {
                test_vector_positive.push_back(j);
            }
        }
        auto test_chain_positive = std::vector<int>();
        auto chain_iter = bloom_chain.Test(std::span{key});
        bool depleted = false;
        while (!depleted) {
            size_t address = 0x7777;
            chain_iter.Next(address, depleted);
            if (!depleted) { test_chain_positive.push_back(address); }
        }
        ASSERT_EQ(test_vector_positive, test_chain_positive);
    }
}