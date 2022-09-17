cmake_minimum_required(VERSION 3.11)

project(SharedMemoryExample VERSION 0.1.0)

add_library(SharedMemory SHARED sharemem.cpp sharemem.h mem_descs.h)
add_library(Memory STATIC imemory.h memory.cpp)
add_executable(Example tests.cpp tests.h main.cpp)

add_dependencies(Memory SharedMemory)
add_dependencies(Example Memory)

target_link_libraries(Memory PRIVATE SharedMemory)
target_link_libraries(Example PRIVATE Memory)