cmake_minimum_required(VERSION 3.18...3.22)

project(bloomstore)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    gtest
    URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gtest)

# -- Platform Flags

if (UNIX)
    message(STATUS "Platform: UNIX")
    add_compile_definitions(UNIX)
endif()
if(MSVC OR MSYS OR MINGW)
    message(STATUS "Platform: WINDOWS")
    add_compile_definitions(WINDOWS)
endif()

add_compile_options(-Og -g)

# -- Library implemented here

add_library(
    bloomstore
    SHARED
        lib/hashing.cpp
        lib/bloom_kvpairs.cpp
        lib/bloom_filter.cpp
        lib/bloom_store.cpp
        lib/partitioner.cpp
        lib/port.cpp
)

target_include_directories(
    bloomstore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# -- Benchmarking stuff implemented here

add_executable(
    main
        bin/main.cpp
)

target_link_libraries(
    main
        bloomstore
)

# -- Testing stuff goes here

add_executable(
    test
        testing/bloom_filter_test.cpp
        testing/bloom_kvpairs_test.cpp
        testing/bloom_store_test.cpp
        testing/partitioner_test.cpp
)

target_link_libraries(test bloomstore gtest_main)