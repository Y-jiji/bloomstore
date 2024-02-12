#ifdef UNIX

#include<cassert>
#include<cstdint>
#include<unistd.h>
#include<port.hpp>
#include<stdio.h>
#include<fcntl.h>
#include<string>
#include<span>
#include<sys/stat.h>
#include<iostream>

FileObject::FileObject(std::string& path):
    path{path}
{
    this->fd = open(this->path.c_str(), O_CREAT|O_RDWR|O_DIRECT|O_SYNC|O_APPEND, S_IRWXU);
    assert(this->fd >= 0);
    struct stat st;
    stat(this->path.c_str(), &st);
    this->size = st.st_size;
}

FileObject::~FileObject()
{
    int error_code = close(this->fd);
    assert(error_code == 0);
}

size_t FileObject::Size() {
    return this->size;
}

void FileObject::Append(std::span<uint8_t> bytes) {
    assert(bytes.size() % 1024 == 0);
    // we used O_APPEND, so no lseek is required
    this->Seek(this->Size());
    int32_t flag = write(this->fd, &bytes[0], bytes.size());
    assert(flag > 0);
    this->size += bytes.size();
}

void FileObject::Read(size_t position, std::span<uint8_t> bytes) {
    assert(bytes.size() % 1024 == 0);
    assert(position % 1024 == 0);
    lseek(this->fd, position, SEEK_SET);
    int32_t flag = read(this->fd, &bytes[0], bytes.size());
    this->position = position + bytes.size();
    assert(flag > 0);
}

void FileObject::Seek(size_t position) {
    this->position = position;
    int32_t result = lseek(this->fd, position, SEEK_SET);
    assert(result >= 0);
}

bool FileObject::ContinueRead(std::span<uint8_t> bytes) {
    assert(bytes.size() % 1024 == 0);
    if (this->size - this->position < bytes.size()) { return false; }
    int32_t flag = read(this->fd, &bytes[0], bytes.size());
    this->position += bytes.size();
    assert(flag > 0);
    return true;
}

bool FileObject::ContinueReadRev(std::span<uint8_t> bytes) {
    assert(bytes.size() % 1024 == 0);
    if (this->position < bytes.size()) { return false; }
    this->position -= bytes.size();
    lseek(this->fd, -bytes.size(), SEEK_CUR);
    int32_t flag = read(this->fd, &bytes[0], bytes.size());
    lseek(this->fd, -bytes.size(), SEEK_CUR);
    assert(flag > 0);
    return true;
}

#endif