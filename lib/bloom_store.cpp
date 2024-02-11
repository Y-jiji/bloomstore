#include<bloom_store.hpp>

namespace bloomstore
{

BloomStore::BloomStore(
    std::string& path_kv,
    std::string& path_bf,
    size_t bloom_filter_nslots,
    size_t bloom_filter_nfuncs,
    size_t key_bytes,
    size_t value_bytes,
    size_t kv_ram_capacity,
    size_t align
):
    f_bloom_filters{path_bf},
    f_kv_pairs{path_kv},
    bloom_chain_collector{bloom_filter_nslots, bloom_filter_nfuncs, align},
    active_bloom_filter{bloom_filter_nslots, bloom_filter_nfuncs},
    active_kv_pairs{key_bytes, value_bytes, kv_ram_capacity, align},
    key_bytes{key_bytes},
    value_bytes{value_bytes},
    capacity{kv_ram_capacity},
    align{align}
{}

BloomStore::~BloomStore() {
}

void BloomStore::Get(
    std::span<uint8_t> key, 
    std::span<uint8_t> value, 
    bool& is_tombstone,
    bool& is_found
) {
    // try active kv pairs
    if (this->active_bloom_filter.Test(key)) {
        this->active_kv_pairs.Get(key, value, is_tombstone, is_found);
        if (is_found) { return; }
    }
    // try stacked bloom filters
    auto temp_kvpairs = KVPairs(this->key_bytes, this->value_bytes, this->capacity, this->align);
    // try bloom filters on disk
    // TODO
}

void BloomStore::Put(
    std::span<uint8_t> key,
    std::span<uint8_t> value
) {
    this->active_bloom_filter.Insert(key);
    this->active_kv_pairs.Put(key, value);
    this->TryFlush();
}

void BloomStore::Del(
    std::span<uint8_t> key
) {
    this->active_bloom_filter.Insert(key);
    this->active_kv_pairs.Del(key);
    this->TryFlush();
}

void BloomStore::TryFlush() {
    // TODO
    if (this->active_kv_pairs.IsFull()) {
        // dump to disk, dump to chain collector
    }
    if (this->bloom_chain_collector.IsFull()) {
        // dump to disk
    }
}

} // namespace bloomstore