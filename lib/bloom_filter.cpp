#include<bloom_filter.hpp>
#include<hashing.hpp>
#include<span>
#include<cstring>
#include<cassert>
#include<iostream>

namespace bloomstore
{

// --- Bloom Filter --- //

/// @brief initialize a bloom filter with nslots slots and nfunc hash functions
/// @param nslots   the number of slots
/// @param nfunc    the number of hash functions
BloomFilter::BloomFilter(size_t nslots, size_t nfunc):
    slots(nslots, false),
    nfunc(nfunc)
{}

/// @brief insert key into represented set
/// @param key the inserted key
void BloomFilter::Insert(std::span<uint8_t> key) {
    uint32_t hash_a = Hash(key, static_cast<uint32_t>('A'));
    uint32_t hash_b = Hash(key, static_cast<uint32_t>('B'));
    for (uint32_t i = 0; i < this->nfunc; ++i) {
        uint32_t hash_z = Mangle(i, hash_a, hash_b, this->slots.size());
        this->slots[hash_z] = true;
    }
}

/// @brief test if key is in the represented set. it may possibly return false positive results
/// @param key the tested key
/// @return true iff key is in the represented set. 
bool BloomFilter::Test(std::span<uint8_t> key) {
    uint32_t hash_a = Hash(key, static_cast<uint32_t>('A'));
    uint32_t hash_b = Hash(key, static_cast<uint32_t>('B'));
    bool collector = true;
    for (uint32_t i = 0; i < this->nfunc; ++i) {
        uint32_t hash_z = Mangle(i, hash_a, hash_b, this->slots.size());
        collector = collector && this->slots[hash_z];
    }
    return collector;
}

/// @brief remove all elements from the represented set. 
void BloomFilter::Clear() {
    std::fill(this->slots.begin(), this->slots.end(), false);
}

// --- Bloom Chain --- //

/// @brief test if key is in the represented set. it may possibly return false positive results
/// @param key the tested key
/// @return true iff key is in the represented set. 
BloomChain::BloomChain(size_t nslots, size_t nfunc, size_t align):
    nfunc(nfunc),
    space(((nslots + sizeof(size_t) * 8 + (align - 1)) / align * align + 7) / 8 * 8, 0),
    chain_length(0)
{
    this->matrix = std::span{&this->space[0], nslots};
    this->block_addresses = std::span{&this->space[nslots], 64};
}

/// @brief add new bloom filter to batch
/// @param filter the new bloom filter
/// @param block_address its block address
void BloomChain::Join(BloomFilter& filter, size_t block_address) {
    assert(this->chain_length < 64);
    for (int i = 0; i < filter.slots.size(); ++i) {
        if (filter.slots[i]) {
            this->matrix[i] = this->matrix[i] | (uint64_t{1} << this->chain_length);
        }
    }
    this->block_addresses[this->chain_length] = block_address;
    this->chain_length += 1;
}

/// @brief test if key exists in current chain
/// @param key the inquired key
/// @return a pointer iterator
PtrIterator BloomChain::Test(std::span<uint8_t> key) {
    uint32_t hash_a = Hash(key, static_cast<uint32_t>('A'));
    uint32_t hash_b = Hash(key, static_cast<uint32_t>('B'));
    uint64_t collector = ~uint64_t{0};
    for (uint32_t i = 0; i < this->nfunc; ++i) {
        uint32_t hash_z = Mangle(i, hash_a, hash_b, this->matrix.size());
        collector = collector & this->matrix[hash_z];
    }
    return PtrIterator{this->block_addresses, collector, 0};
}

/// @brief check if current chain is full
/// @return when chain length is 64, return true
bool BloomChain::IsFull() {
    return this->chain_length == 64;
}

/// @brief we cannot really know what do we want to do with the FileObject, so ... we just pass the loader
/// @param loader the loading routine
void BloomChain::Load(std::function<void(std::span<uint8_t>)> loader) {
    auto space = std::span{(uint8_t*)(&this->space[0]), sizeof(uint64_t) * this->space.size()};
    loader(space);
    this->chain_length = 64;
}

/// @brief dump current bloom chain into file and clear internal data
/// @param file the file to dump into
void BloomChain::Dump(FileObject& file) {
    assert(this->IsFull());
    auto space = std::span{(uint8_t*)(&this->space[0]), sizeof(uint64_t) * this->space.size()};
    file.Append(space);
    this->chain_length = 0;
    memset(&this->space[0], 0, sizeof(uint64_t) * this->space.size());
}

// --- PtrIterator --- //

/// @brief initialize a pointer iterator
/// @param block_addresses referred block addresses
/// @param bitmask a bitmask indicating whether current key presents in bloom chain
/// @param progress current iteration progress
PtrIterator::PtrIterator(
    std::span<size_t> block_addresses,
    uint64_t bitmask,
    uint8_t progress
):
    block_addresses{block_addresses},
    bitmask{bitmask},
    progress{progress}
{}

/// @brief get next address
/// @param address  the given address
/// @param depleted if current iterator is depleted
void PtrIterator::Next(size_t &address, bool& depleted) {
    if (this->progress == this->block_addresses.size()) {
        depleted = true;
        return;
    }
    auto j = this->block_addresses.size() - this->progress - 1;
    if ((this->bitmask) & (uint64_t{1} << j)) {
        address = this->block_addresses[j];
        depleted = false;
        this->progress += 1;
        return;
    }
    else {
        this->progress += 1;
        return Next(address, depleted);
    }
}

} // namespace bloomstore
