add_definitions(-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc
    -fno-builtin-free)
include_directories(${TCMALLOC_HEADER})
file(GLOB_RECURSE SOURCE "*.cpp")

foreach(src ${SOURCE})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})
    target_link_libraries(${name}
        ${GOOGLE_BENCHMARK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
        ${TCMALLOC_LIB})
endforeach()
