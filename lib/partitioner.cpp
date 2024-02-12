#include<iostream>
#include<hashing.hpp>
#include<partitioner.hpp>

namespace bloomstore {

Partitioner::Partitioner(
    std::vector<BloomStore> instances
):
    instances{instances}
{}

void Partitioner::Put(std::span<uint8_t> key, std::span<uint8_t> value) {
    size_t index = Hash(key, 'Z') % this->instances.size();
    this->instances[index].Put(key, value);
}

void Partitioner::Del(std::span<uint8_t> key) {
    size_t index = Hash(key, 'Z') % this->instances.size();
    this->instances[index].Del(key);
}

void Partitioner::Get(std::span<uint8_t> key, std::span<uint8_t> value, bool& is_tombstone, bool& is_found) {
    size_t index = Hash(key, 'Z') % this->instances.size();
    this->instances[index].Get(key, value, is_tombstone, is_found);
}

} // namespace bloomstore
