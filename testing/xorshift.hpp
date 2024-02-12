#include<stdint.h>
#include<span>

namespace xorshift {

class XorShift32 {
    
    private:
    uint32_t state;
    
    public:
    XorShift32(uint32_t seed):
        state(seed)
    { }
    uint32_t Sample() {
        uint32_t x = this->state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return this->state = x;
    }
    void Fill(std::span<uint8_t> buff) {
        uint32_t x = 0;
        for (int i = 0; i < buff.size(); ++i) {
            if (i % 4 == 0) { x = this->Sample(); }
            buff[i] = (x >> (i % 4)) & 0xff;
        }
    }
};

}