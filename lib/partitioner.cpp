#include<iostream>
#include<hashing.hpp>
#include<partitioner.hpp>

namespace bloomstore {

Partitioner::Partitioner(
    std::vector<BloomStore*>&& instances
):
    instances{instances}
{}

Partitioner::~Partitioner() {
    for (auto instance: instances) {
        delete instance;
    }
}

void Partitioner::Put(std::span<uint8_t> key, std::span<uint8_t> value) {
    size_t index = Hash(key, 'Z') % this->instances.size();
    this->instances[index]->Put(key, value);
}

void Partitioner::Del(std::span<uint8_t> key) {
    size_t index = Hash(key, 'Z') % this->instances.size();
    this->instances[index]->Del(key);
}

void Partitioner::Get(std::span<uint8_t> key, std::span<uint8_t> value, bool& is_tombstone, bool& is_found) {
    size_t index = Hash(key, 'Z') % this->instances.size();
    this->instances[index]->Get(key, value, is_tombstone, is_found);
}

size_t Partitioner::StatDiskReadCount() {
    size_t disk_read_count = 0;
    for (auto instance: this->instances) {
        disk_read_count += instance->stat_disk_read;
    }
    return disk_read_count;
}

size_t Partitioner::StatFalsePositive() {
    size_t false_positive = 0;
    for (auto instance: this->instances) {
        false_positive += instance->stat_false_positive;
    }
    return false_positive;
}

} // namespace bloomstore
