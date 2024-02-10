#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <array>
#include <span>

namespace bloomstore {

class KVPairs {
  private:
    std::vector<bool> tombstone;
    std::vector<uint8_t> pairs;
    size_t size;
    size_t key_bytes;
    size_t value_bytes;
    size_t capacity;

  public:
    KVPairs(size_t key_bytes, size_t value_bytes, size_t capacity);
    void Put(std::span<uint8_t> key, std::span<uint8_t> value);
    void Del(std::span<uint8_t> key);
    std::optional<std::optional<std::span<uint8_t>>> Get(std::span<uint8_t> key);
};

} // namespace bloomstore