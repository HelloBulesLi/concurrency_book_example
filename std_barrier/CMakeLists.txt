cmake_minimum_required(VERSION 3.14)

project(barrier)

SET(CMAKE_BUILD_TYPE "Debug")

set( CMAKE_CXX_FLAGS "-std=c++17 -O3" )

# set(CMAKE_CXX_STANDARD 14)

# set( CMAKE_CXX_FLAGS "-std=c++2a -O3" )

# set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(Boost 1.65.1 REQUIRED COMPONENTS thread)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} barrier.cpp)
target_link_libraries(${PROJECT_NAME} Threads::Threads ${Boost_LIBRARIES})
