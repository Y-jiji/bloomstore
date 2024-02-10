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

};

}