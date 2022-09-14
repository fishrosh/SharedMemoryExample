cmake_minimum_required(VERSION 3.11)

project(SharedMemoryExample VERSION 0.1.0)

add_library(SharedMemory SHARED sharemem.cpp sharemem.h)
add_executable(Example tests.cpp tests.h main.cpp)
add_dependencies(Example SharedMemory)