#include"../testing/xorshift.hpp"
#include<partitioner.hpp>
#include<bloom_store.hpp>
#include<iostream>
#include<cstring>
#include<cassert>
#include<unistd.h>
#include<fcntl.h>
#include<stdint.h>
#include<errno.h>
#include<chrono>

#define TOTAL 20000000
#define DELTA 100000

void Truncate(std::string& path) {
    int fd = open(path.c_str(), O_CREAT|O_TRUNC, S_IRWXU);
    if (fd < 0) {
        std::cerr << "open: "   << path  << std::endl;
        std::cerr << "errno: "  << errno << std::endl;
        std::cerr << "fd: "     << fd    << std::endl;
    }
    assert(fd >= 0);
    int error_code = close(fd);
    assert(error_code == 0);
}

int main() {
    // mimicing the "linux" workload in the MSST article: https://ieeexplore.ieee.org/document/6232390
    auto bloom_store_replications = std::vector<bloomstore::BloomStore*>();
    for (char i = 'a'; i <= 'z'; ++i) {
        for (char j = 'a'; j <= 'z'; ++j) {
            auto path_kv = std::string{"./test-kv-"} + i + j;
            auto path_bf = std::string{"./test-bf-"} + i + j;
            Truncate(path_kv);
            Truncate(path_bf);
            bloom_store_replications.push_back(new bloomstore::BloomStore(
                path_kv, path_bf,
                8192, 11,    // bf_slots, bf_functions
                4   , 4,     // key_bytes, value_bytes
                512 , 1024   // ram_capacity, align
            ));
        }
    }
    auto partitioner = bloomstore::Partitioner(std::move(bloom_store_replications));
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
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < TOTAL; ++i) {
        auto action = (int)(random_number_generator.Sample() % 5 == 0);
        auto key    = to_arr(random_number_generator.Sample());
        auto value  = to_arr(random_number_generator.Sample());
        switch (action) {
            case 0: {
                partitioner.Put(std::span{key}, std::span{value});
                break;
            }
            case 1: {
                auto value = std::array<uint8_t, 4>();
                bool is_tombstone = true;
                bool is_found = true;
                partitioner.Get(std::span{key}, std::span{value}, is_tombstone, is_found);
                break;
            }
        }
        if (i % DELTA == 0) {
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " [ms]" << std::endl;
            std::cout << "Throughput = " << DELTA / (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() + 0.0001) * 1000 << " [TOTAL/sec]" << std::endl;
            begin = end;
        }
    }
}