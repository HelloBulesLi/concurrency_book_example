cmake_minimum_required(VERSION 3.14)

project(parallel_accum)

SET(CMAKE_BUILD_TYPE "Debug")

# set( CMAKE_CXX_FLAGS "-std=c++14 -O3" )

# set(CMAKE_CXX_STANDARD 14)

set( CMAKE_CXX_FLAGS "-std=c++17 -O3" )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)


add_executable(${PROJECT_NAME} parallel_accum.cpp)
target_link_libraries(${PROJECT_NAME} Threads::Threads)
