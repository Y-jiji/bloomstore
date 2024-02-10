#include<cstdint>
#include<vector>
#include<span>

namespace bloomstore {

/// @brief bloom filter representing sets. 
class BloomFilter {

    private:
    std::vector<bool> slots;
    uint32_t nfunc;
    uint32_t Mangle(uint8_t n, uint32_t hash_a, uint32_t hash_b, uint32_t size);
    uint32_t Hash(std::span<uint8_t> key, uint32_t seed);

    public:
    BloomFilter(uint32_t nslots, uint8_t n_seeds);
    void Insert(std::span<uint8_t> key);
    bool Test(std::span<uint8_t> key);

};

} // namespace bloomstore