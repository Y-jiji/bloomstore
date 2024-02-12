#include<gtest/gtest.h>
#include<bloom_kvpairs.hpp>
#include<port.hpp>
#include<unordered_map>
#include<array>
#include"./xorshift.hpp"
#include<cstring>


namespace {

#define K 4
#define V 4
#define ARR std::array<uint8_t, 4>

struct KeyHasher {
    std::size_t operator()(const ARR& a) const {
        std::size_t h = 0;
        for (auto e : a) {
            h ^= std::hash<int>{}(e)  + 0x9e3779b9 + (h << 6) + (h >> 2); 
        }
        return h;
    }   
};

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

TEST(KVPairs, ReadWriteTest) {
    auto kvpairs = bloomstore::KVPairs(K, V, 4096, 1024);
    auto ground_truth = std::unordered_map<ARR, ARR, KeyHasher>();
    auto random_number_generator = xorshift::XorShift32(5);
    for (int i = 0; i < 4096; ++i) {
        auto action = random_number_generator.Sample() % 3;
        auto key    = to_arr(random_number_generator.Sample() % 64);
        auto value  = to_arr(random_number_generator.Sample());
        switch (action) {
            case 0: {
                ground_truth.erase(key);
                ground_truth.insert(std::make_pair(key, value));
                kvpairs.Put(std::span{key}, std::span{value});
                break;
            }
            case 1: {
                ground_truth.erase(key);
                kvpairs.Del(std::span{key});
                break;
            }
            case 2: {
                auto value = ARR();
                bool is_tombstone = true;
                bool is_found = true;
                kvpairs.Get(std::span{key}, std::span{value}, is_tombstone, is_found);
                if (ground_truth.contains(key)) {
                    auto expected_value = ground_truth[key];
                    ASSERT_EQ(expected_value, value);
                }
                else {
                    ASSERT_TRUE(is_tombstone || !is_found);
                }
                break;
            }
        }
    }
}

TEST(KVPairs, DumpLoadTest) {
    auto kvpairs = bloomstore::KVPairs(K, V, 4096, 1024);
    auto ground_truth = std::unordered_map<ARR, ARR, KeyHasher>();
    auto random_number_generator = xorshift::XorShift32(5);
    for (int i = 0; i < 4096; ++i) {
        auto action = random_number_generator.Sample() % 2;
        auto key    = to_arr(random_number_generator.Sample() % 64);
        auto value  = to_arr(random_number_generator.Sample());
        switch (action) {
            case 0: {
                ground_truth.erase(key);
                ground_truth.insert(std::make_pair(key, value));
                kvpairs.Put(std::span{key}, std::span{value});
                break;
            }
            case 1: {
                ground_truth.erase(key);
                kvpairs.Del(std::span{key});
                break;
            }
        }
    }
    auto path = std::string("/tmp/xxxxx");
    auto file = FileObject(path);
    auto start = file.Size();
    kvpairs.Dump(file);
    kvpairs.Load([&](std::span<uint8_t> span){
        file.Read(start, span);
    });
    for (int i = 0; i < 4096; ++i) {
        auto action = random_number_generator.Sample() % 2;
        auto key    = to_arr(random_number_generator.Sample() % 64);
        auto value = ARR();
        bool is_tombstone = true;
        bool is_found = true;
        kvpairs.Get(std::span{key}, std::span{value}, is_tombstone, is_found);
        if (ground_truth.contains(key)) {
            auto expected_value = ground_truth[key];
            ASSERT_EQ(expected_value, value);
        }
        else {
            ASSERT_TRUE(is_tombstone || !is_found);
        }
    }
}

#undef K
#undef V
#undef ARR

}