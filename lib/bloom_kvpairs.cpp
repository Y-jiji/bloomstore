#include<cstring>
#include<bloom_kvpairs.hpp>
#include<iostream>
#include <cassert>

namespace bloomstore {

#define K (this->key_bytes)
#define V (this->value_bytes)

/// @brief delete key by adding a tombstone
/// @param key the deleted key
void KVPairs::Del(std::span<uint8_t> key) {
    assert(key.size() == K);
    assert(this->size < this->capacity);
    auto i = this->size;
    this->size += 1;
    this->tombstone[i] = true;
    memcpy(&this->pairs[i * (K + V)], &key[0], K);
    return;
}

/// @brief get a key
/// @param key the inquired key
/// @return nullopt if not found, {nullopt} if deleted, {{value}} if entry exists
void KVPairs::Get(std::span<uint8_t> key, std::span<uint8_t> val, bool& is_found, bool& is_tombstone) {
    assert(key.size() == K);
    assert(val.size() == K);
    is_found = false;
    is_tombstone = false;
    for (int i = 0; i < this->size; ++i) {
        int j = this->size - i - 1;
        bool eq = 0 == memcmp(&this->pairs[j * (K + V)], &key[0], K);
        if (!eq) { continue; }
        is_found = true;
        is_tombstone = this->tombstone[j];
        if (!is_tombstone)
            memcpy(&val[0], &this->pairs[j * (K + V) + K], V);
        return;
    }
}

/// @brief put a key into storage
/// @param key the inquired key
/// @param val the inserted value
void KVPairs::Put(std::span<uint8_t> key, std::span<uint8_t> val) {
    assert(key.size() == K);
    assert(val.size() == V);
    assert(this->size < this->capacity);
    auto i = this->size;
    this->size += 1;
    this->tombstone[i] = false;
    memcpy(&this->pairs[i * (K + V)], &key[0], K);
    memcpy(&this->pairs[i * (K + V) + K], &val[0], V);
    return;
}

/// @brief initialize an empty kv storage
/// @tparam CAPACITY of the storage
KVPairs::KVPairs(size_t key_bytes, size_t value_bytes, size_t capacity):
    size{0},
    key_bytes{key_bytes},
    value_bytes{value_bytes},
    capacity{capacity},
    tombstone(capacity, false),
    pairs(capacity * key_bytes * value_bytes, 0x77)
{}

#undef K
#undef V

} // namespace bloomstore
