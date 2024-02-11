#include<unistd.h>
#include<fcntl.h>
#include<cstdint>
#include<iostream>

int main() {
    int32_t fd = open("/tmp/test-file", O_RDWR|O_SYNC|O_DIRECT|O_APPEND|O_CREAT);
    auto s = std::string{""};
    for (int i = 0; i < 1024; ++i) {
        s += "y";
    }
    int32_t bytes = write(fd, s.c_str(), s.size());
    std::cerr << bytes << std::endl;
    close(fd);
    return 0;
}