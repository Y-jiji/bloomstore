#include<cstring>
#include<cassert>
#include<bloom_kvpairs.hpp>
#include<iostream>

namespace bloomstore {

#define K (this->key_bytes)
#define V (this->value_bytes)
#define C (this->capacity)

/// @brief initialize bitspan with a space
/// @param space the used space (must not overlap any others)
BitSpan::BitSpan(std::span<uint8_t> space):
    space{space}
{}

/// @brief get element at index i
/// @param i index
/// @return the elment at index i
const bool BitSpan::Get(size_t i) {
    return (this->space[i / 8] >> (i % 8)) & 1;
}

/// @brief set element at index to value
/// @param i index
/// @param v value
void BitSpan::Set(size_t i, bool v) {
    this->space[i / 8] = (this->space[i / 8] & ~(1<<(i % 8))) | (v << (i % 8));
}

/// @brief initialize an empty kv storage
KVPairs::KVPairs(size_t key_bytes, size_t value_bytes, size_t capacity, size_t align):
    size{0},
    key_bytes{key_bytes},
    value_bytes{value_bytes},
    capacity{capacity},
    space((capacity * key_bytes * value_bytes + (capacity + 7) / 8 + align - 1) / align * align, 0),
    tombstone(std::span{&this->space[0], (C+7)/8})
{
    this->tombstone = BitSpan(std::span{&this->space[0], (C+7)/8});
    this->pairs = std::span{&this->space[(C+7)/8], C*(K+V)};
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
    this->tombstone.Set(i, false);
    memcpy(&this->pairs[i * (K + V)], &key[0], K);
    memcpy(&this->pairs[i * (K + V) + K], &val[0], V);
    return;
}

/// @brief delete key by adding a tombstone
/// @param key the deleted key
void KVPairs::Del(std::span<uint8_t> key) {
    assert(key.size() == K);
    assert(this->size < this->capacity);
    auto i = this->size;
    this->size += 1;
    this->tombstone.Set(i, true);
    memcpy(&this->pairs[i * (K + V)], &key[0], K);
    return;
}

/// @brief get a key
/// @param key the inquired key
/// @return nullopt if not found, {nullopt} if deleted, {{value}} if entry exists
void KVPairs::Get(std::span<uint8_t> key, std::span<uint8_t> val, bool& is_tombstone, bool& is_found) {
    assert(key.size() == K);
    assert(val.size() == K);
    is_found = false;
    is_tombstone = false;
    for (int i = 0; i < this->size; ++i) {
        int j = this->size - i - 1;
        bool eq = 0 == memcmp(&this->pairs[j * (K + V)], &key[0], K);
        if (!eq) { continue; }
        is_found = true;
        is_tombstone = this->tombstone.Get(j);
        if (!is_tombstone)
            memcpy(&val[0], &this->pairs[j * (K + V) + K], V);
        return;
    }
}

/// @brief check if current kvpairs object is full
/// @return return true when it is full 
bool KVPairs::IsFull() {
    return this->size == this->capacity;
}

/// @brief dump current kvpairs to file and clear current object
/// @param file the file to dump into
void KVPairs::Dump(FileObject& file) {
    assert(this->IsFull());
    auto space = std::span{&this->space[0], this->space.size()};
    file.Append(space);
    this->size = 0;
}

/// @brief load kvpairs from file object
/// @param loader loader takes a buffer and loads data into this buffer
void KVPairs::Load(std::function<void(std::span<uint8_t>)> loader) {
    auto space = std::span{&this->space[0], this->space.size()};
    loader(space);
    this->size = this->capacity;
}

#undef K
#undef V

} // namespace bloomstore
