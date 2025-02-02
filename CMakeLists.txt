cmake_minimum_required(VERSION 3.21)

project(Pyke)

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

# Custom target for Generate build
add_custom_target(generate
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Generate ${CMAKE_SOURCE_DIR} && make
  COMMENT "Building in Generate mode"
)

# Custom target for Release build
add_custom_target(release
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR} && make
  COMMENT "Building in Release mode"
)

add_custom_target(debug
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR} && make
	COMMENT "Building in Debug mode"
)

add_dependencies(generate main) 
add_custom_command(TARGET generate POST_BUILD
    COMMAND ./main
    COMMENT "Generating profile files for profile guided compilation."
)

add_dependencies(release generate)
