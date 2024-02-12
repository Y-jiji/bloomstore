#pragma once

#ifdef UNIX

#include<cstdint>
#include<string>
#include<span>

class FileObject {
    
    private:
    std::string path;
    int32_t fd;
    size_t size;
    size_t position;
    
    public:
    FileObject(std::string& path);
    ~FileObject();
    void Append(std::span<uint8_t> bytes);
    void Read(size_t position, std::span<uint8_t> bytes);
    void Seek(size_t position);
    bool ContinueRead(std::span<uint8_t> bytes);
    bool ContinueReadRev(std::span<uint8_t> bytes);
    size_t Size();
};

#endif