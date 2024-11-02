cmake_minimum_required(VERSION 3.21)

project(Pyke)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE "Release")

add_executable(main main.cpp do_moves.cpp move_generator.cpp position.cpp undo_moves.cpp)
