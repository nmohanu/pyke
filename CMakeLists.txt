cmake_minimum_required(VERSION 3.21)

project(Pyke)

set(CMAKE_BUILD_TYPE Debug)

add_compile_options(
    -Ofast
	-funroll-loops
    -march=native
)
set (CMAKE_CXX_STANDARD 20)

add_executable(main main.cpp position.cpp board.cpp)

if(CMAKE_BUILD_TYPE STREQUAL "Generate")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-generate")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-generate")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-use")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-use")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
endif()
