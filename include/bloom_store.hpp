#include<port.hpp>

namespace bloomstore
{

class BloomStore {

    private:
    FileObject f_bloom_filters;
    FileObject f_kv_pairs;
    BloomChain bloom_chain_collector;
    BloomFilter active_bloom_filter;
    KVPairs     active_kv_pairs;
    size_t size;
    size_t key_bytes;
    size_t value_bytes;
    size_t capacity;

    size_t stat_get_count;
    size_t stat_put_count;
    size_t stat_disk_read;

    public:
    BloomStore(std::string& path_kv, std::string& path_bf);
    ~BloomStore();
    void Put(std::span<uint8_t> key, std::span<uint8_t> value);
    void Del(std::span<uint8_t> key);
    void Get(std::span<uint8_t> key, std::span<uint8_t> value, bool& is_tombstone, bool& is_found);
};

}