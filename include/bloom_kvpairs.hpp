#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <array>
#include <span>
#include <functional>
#include <port.hpp>

namespace bloomstore {

class BitSpan {
  
    private:
    std::span<uint8_t> space;
  
    public:
    BitSpan(std::span<uint8_t> space);
    const bool Get(size_t i);
    void Set(size_t i, bool v);

};

class KVPairs {

    private:
    std::vector<uint8_t> space;
    BitSpan              tombstone;
    std::span<uint8_t>   pairs;
    size_t size;
    size_t key_bytes;
    size_t value_bytes;
    size_t capacity;

    public:
    KVPairs(size_t key_bytes, size_t value_bytes, size_t capacity, size_t align);
    void Put(std::span<uint8_t> key, std::span<uint8_t> value);
    void Del(std::span<uint8_t> key);
    void Get(std::span<uint8_t> key, std::span<uint8_t> value, bool& is_tombstone, bool& is_found);
    bool IsFull();
    void Dump(FileObject& file);
    void Load(std::function<void(std::span<uint8_t>)> loader);

};

} // namespace bloomstore