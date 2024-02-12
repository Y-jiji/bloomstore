#include<partitioner.hpp>
#include<gtest/gtest.h>
#include<unistd.h>
#include<fcntl.h>
#include"./xorshift.hpp"

namespace {

template<int const N>
struct KeyHasher {
    std::size_t operator()(const std::array<uint8_t, N>& a) const {
        std::size_t h = 0;
        for (auto e : a) {
            h ^= std::hash<int>{}(e)  + 0x9e3779b9 + (h << 6) + (h >> 2); 
        }
        return h;
    }   
};

void Truncate(std::string& path) {
    int fd = open(path.c_str(), O_CREAT|O_TRUNC, S_IRWXU);
    assert(fd >= 0);
    int error_code = close(fd);
    assert(error_code == 0);
}

TEST(Partitioner, Correctness) {
    auto bloom_store_replications = std::vector<bloomstore::BloomStore*>();
    for (char i = 'a'; i <= 'z'; ++i) {
        auto path_kv = std::string{"./test-kv-"} + i;
        auto path_bf = std::string{"./test-bf-"} + i;
        bloom_store_replications.push_back(new bloomstore::BloomStore(
            path_kv, path_bf,
            8192, 5,     // bf_slots, bf_functions
            4,    4,     // key_bytes, value_bytes
            1024, 4096   // ram_capacity, align
        ));
    }
    auto partitioner = bloomstore::Partitioner(std::move(bloom_store_replications));
    auto ground_truth = std::unordered_map<std::array<uint8_t, 4>, std::array<uint8_t, 4>, KeyHasher<4>>();
    auto random_number_generator = xorshift::XorShift32(5);
    auto to_arr = [](uint32_t xvalue) {
        auto value = std::array<uint8_t, 4>();
        memcpy(&value, &xvalue, sizeof(uint32_t));
        return value;
    };
    auto to_int = [](std::array<uint8_t, 4> value) {
        uint32_t xvalue = 0;
        memcpy(&xvalue, &value, sizeof(uint32_t));
        return xvalue;
    };
    for (int i = 0; i < 1000000; ++i) {
        auto action = random_number_generator.Sample() % 3;
        auto key    = to_arr(random_number_generator.Sample() % 64);
        auto value  = to_arr(random_number_generator.Sample());
        switch (action) {
            case 0: {
                ground_truth.erase(key);
                ground_truth.insert(std::make_pair(key, value));
                partitioner.Put(std::span{key}, std::span{value});
                break;
            }
            case 1: {
                ground_truth.erase(key);
                partitioner.Del(std::span{key});
                break;
            }
            case 2: {
                auto value = std::array<uint8_t, 4>();
                bool is_tombstone = true;
                bool is_found = true;
                partitioner.Get(std::span{key}, std::span{value}, is_tombstone, is_found);
                if (ground_truth.contains(key)) {
                    auto expected_value = ground_truth[key];
                    ASSERT_TRUE(is_found && !is_tombstone) << is_found << " " << is_tombstone << std::endl;
                    ASSERT_EQ(to_int(expected_value), to_int(value));
                }
                else {
                    ASSERT_TRUE(is_tombstone || !is_found);
                }
                break;
            }
        }
    }
}

}