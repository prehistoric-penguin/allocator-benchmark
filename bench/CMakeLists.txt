cmake_minimum_required(VERSION 2.8)
project(tcbench)

add_definitions (-std=c++11 -pthread)

find_package(Threads REQUIRED)

find_path(JEMALLOC_HEADER
    NAMES jemalloc
    PATHS ${CMAKE_SOURCE_DIR}/../jemalloc/include/
    NO_DEFAULT_PATH)
find_library(JEMALLOC_LIB
    NAMES libjemalloc.so
    PATHS ${CMAKE_SOURCE_DIR}/../jemalloc/lib/
    NO_DEFAULT_PATH)

find_path(TCMALLOC_HEADER
    NAMES tcmalloc.h
    PATHS ${CMAKE_SOURCE_DIR}/../gperftools/src/
    NO_DEFAULT_PATH)
find_library(TCMALLOC_LIB
    NAMES libtcmalloc.so
    PATHS ${CMAKE_SOURCE_DIR}/../gperftools/.libs/
    NO_DEFAULT_PATH)


message(STATUS "jemalloc header:" ${JEMALLOC_HEADER})
message(STATUS "jemalloc library:" ${JEMALLOC_LIB})

message(STATUS "tcmalloc header:" ${TCMALLOC_HEADER})
message(STATUS "tcmalloc library:" ${TCMALLOC_LIB})



find_path(GOOGLE_BENCHMARK_HEADER "benchmark/benchmark.h")
find_library(GOOGLE_BENCHMARK_LIBRARIES "benchmark")
message(STATUS "google benchmark header:" ${GOOGLE_BENCHMARK_HEADER})
message(STATUS "google benchmark library:" ${GOOGLE_BENCHMARK_LIBRARIES})

include_directories(${GOOGLE_BENCHMARK_HEADER})

add_subdirectory(jemalloc_bench)
add_subdirectory(tcmalloc_bench)
add_subdirectory(default_bench)
