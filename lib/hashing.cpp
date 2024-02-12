#include<cstdint>
#include<span>
#include<cstring>

namespace bloomstore
{

// --- Hashing algorithm --- //

/// @brief double hash trick for generating more than 2 hash functions from only 2 full hash algorithm runs. 
/// @param n double hash counter
/// @param hash_a first hash
/// @param hash_b second hash
/// @param size the target domain size
/// @return mangled hash from hash_a and hash_b
uint32_t Mangle(uint8_t n, uint32_t hash_a, uint32_t hash_b, uint32_t size) {
    return (n * hash_a + hash_b) % size;
}

/// @brief a utility function for murmur hash implementation
/// @param k the scrambled key
/// @return scrambled k
inline uint32_t Scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

/// @brief standard murmur hash algorithm, stenographically copied from [wikipedia](https://en.wikipedia.org/wiki/MurmurHash)
/// @param key  the hashed key
/// @param seed seed for hashing
/// @return desired hash value
uint32_t Hash(std::span<uint8_t> key, uint32_t seed) {
    uint32_t h = seed;
    auto len = key.size();
    for (size_t i = 0; i + 3 < len; i += 4) {
        uint32_t k = 0;
        memcpy(&k, &key.subspan(i, 4)[0], sizeof(uint32_t));
        h ^= Scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    uint32_t k = 0;
    for (size_t i = len - len % 4; i < len; i += 1) {
        k <<= 8;
        k |= key[i];
    }
    h ^= Scramble(k);
    h ^= key.size();
    h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
    return h;
}

} // namespace bloomstore