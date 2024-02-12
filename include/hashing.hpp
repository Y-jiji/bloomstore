#include<cstdint>
#include<span>

namespace bloomstore
{

uint32_t Mangle(uint8_t n, uint32_t hash_a, uint32_t hash_b, uint32_t size);

uint32_t Hash(std::span<uint8_t> key, uint32_t seed);

} // namespace bloomstore