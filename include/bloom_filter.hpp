#include<cstdint>
#include<vector>
#include<span>

namespace bloomstore {
class BloomChain;
class BloomFilter;
struct PtrIterator;

/// @brief bloom filter representing sets. 
class BloomFilter {

    private:
    std::vector<bool> slots;
    uint32_t nfunc;
    friend BloomChain;

    public:
    BloomFilter(uint32_t nslots, uint8_t nfunc);
    void Insert(std::span<uint8_t> key);
    bool Test(std::span<uint8_t> key);

};

/// @brief readonly bloom filters that can run in parallel
class BloomChain {

    private:
    std::vector<uint64_t> matrix;
    std::vector<size_t>   block_addresses;
    uint32_t nfunc;
    size_t   chain_length;

    public:
    BloomChain(uint32_t nslots, uint8_t nfunc);
    void Join(BloomFilter&& filter, size_t block_address);
    PtrIterator Test(std::span<uint8_t> key);

};

/// @brief pointer iterator for bloomchain
struct PtrIterator {

    private:
    std::vector<size_t> &block_addresses;
    uint64_t bitmask;
    uint8_t progress;
    friend BloomChain;

    public:
    PtrIterator(
        std::vector<size_t> &block_addresses,
        uint64_t bitmask,
        uint8_t progress
    );
    void Next(size_t& address, bool& depleted);

};

} // namespace bloomstore