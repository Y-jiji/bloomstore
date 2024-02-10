cmake_minimum_required(VERSION 3.18...3.22)

project(bloomstore)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -- Library implemented here

add_library(
    bloomstore
    SHARED
        lib/bloom_buffer.cpp
        lib/bloom_filter.cpp
        lib/bloom_store.cpp
        lib/partitioner.cpp
)

target_include_directories(
    bloomstore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# -- Testing stuff implemented here

add_executable(
    main
        bin/main.cpp
)

target_link_libraries(
    main
        bloomstore
)