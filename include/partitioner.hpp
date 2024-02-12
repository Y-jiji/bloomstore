#include<vector>
#include<bloom_store.hpp>

namespace bloomstore {

class Partitioner {

    private:
    std::vector<BloomStore> instances;

    public:
    Partitioner(std::vector<BloomStore>);

};

} // namespace bloomstore