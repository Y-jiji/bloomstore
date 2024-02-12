#include<vector>
#include<bloom_store.hpp>

namespace bloomstore {

class Partitioner {

    private:
    std::vector<BloomStore> instances;

    public:
    Partitioner(std::vector<BloomStore> instances);
    void Put(std::span<uint8_t> key, std::span<uint8_t> value);
    void Del(std::span<uint8_t> key);
    void Get(std::span<uint8_t> key, std::span<uint8_t> value, bool& is_tombstone, bool& is_found);

};

} // namespace bloomstore