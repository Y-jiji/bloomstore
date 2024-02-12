#include<bloom_store.hpp>
#include<iostream>

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
    f_bloom_chains{path_bf},
    f_kv_pairs{path_kv},
    bloom_chain_collector{bloom_filter_nslots, bloom_filter_nfuncs, align},
    active_bloom_filter{bloom_filter_nslots, bloom_filter_nfuncs},
    active_kv_pairs{key_bytes, value_bytes, kv_ram_capacity, align},
    key_bytes{key_bytes},
    value_bytes{value_bytes},
    capacity{kv_ram_capacity},
    align{align},
    bloom_filter_nslots{bloom_filter_nslots},
    bloom_filter_nfuncs{bloom_filter_nfuncs}
{}

BloomStore::~BloomStore() {}

void BloomStore::Get(
    std::span<uint8_t> key, 
    std::span<uint8_t> value, 
    bool& is_tombstone,
    bool& is_found
) {
    is_tombstone = false;
    is_found = false; 
    this->stat_get_count += 1;
    // try active kv pairs
    if (this->active_bloom_filter.Test(key)) {
        this->active_kv_pairs.Get(key, value, is_tombstone, is_found);
        if (is_found) { return; }
    }
    // try things on disk
    auto temp_kvpairs = KVPairs(this->key_bytes, this->value_bytes, this->capacity, this->align);
    auto print_bloom_chain = [&](bloomstore::PtrIterator&& pointer_iter) {
        bool depleted = false;
        while (true) {
            size_t address;
            pointer_iter.Next(address, depleted);
            if (depleted) break;
        }
    };
    auto try_bloom_chain = [&](bloomstore::PtrIterator&& pointer_iter) {
        bool depleted = false;
        while (true) {
            size_t address;
            pointer_iter.Next(address, depleted);
            if (depleted) break;
            temp_kvpairs.Load([&](std::span<uint8_t> span) {
                this->stat_disk_read += 1;
                this->f_kv_pairs.Read(address, span);
            });
            temp_kvpairs.Get(key, value, is_tombstone, is_found);
            if (is_found) return;
        }
    };
    print_bloom_chain(std::move(this->bloom_chain_collector.Test(key)));
    try_bloom_chain(std::move(this->bloom_chain_collector.Test(key)));
    if (is_found) return;
    auto bloom_chain = bloomstore::BloomChain(
        this->bloom_filter_nslots, 
        this->bloom_filter_nfuncs, 
        this->align
    );
    this->f_bloom_chains.Seek(this->f_bloom_chains.Size());
    while (true) {
        bool is_read_successful = false;
        bloom_chain.Load([&](std::span<uint8_t> span) {
            this->stat_disk_read += 1;
            is_read_successful 
                = this->f_bloom_chains.ContinueReadRev(span);
        });
        if (!is_read_successful) return;
        print_bloom_chain(std::move(bloom_chain.Test(key)));
        try_bloom_chain(std::move(bloom_chain.Test(key)));
        if (is_found) return;
    }
}

void BloomStore::Put(
    std::span<uint8_t> key,
    std::span<uint8_t> value
) {
    this->stat_put_count += 1;
    this->active_bloom_filter.Insert(key);
    this->active_kv_pairs.Put(key, value);
    this->TryFlush();
}

void BloomStore::Del(
    std::span<uint8_t> key
) {
    this->stat_put_count += 1;
    this->active_bloom_filter.Insert(key);
    this->active_kv_pairs.Del(key);
    this->TryFlush();
}

void BloomStore::TryFlush() {
    if (this->active_kv_pairs.IsFull()) {
        auto address = this->f_kv_pairs.Size();
        this->active_kv_pairs.Dump(this->f_kv_pairs);
        this->bloom_chain_collector.Join(this->active_bloom_filter, address);
        this->active_bloom_filter.Clear();
    }
    if (this->bloom_chain_collector.IsFull()) {
        this->bloom_chain_collector.Dump(this->f_bloom_chains);
    }
}

} // namespace bloomstore