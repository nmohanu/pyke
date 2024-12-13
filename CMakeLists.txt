cmake_minimum_required(VERSION 3.21)

project(Pyke)

add_compile_options(-O3 -march=native)
set (CMAKE_CXX_STANDARD 20)

add_executable(main main.cpp position.cpp)
