cmake_minimum_required(VERSION 3.0.0)
project(LEO_satellite_robustness VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-lm -Wall -Wextra -Wpedantic -fopenmp -O3)

add_executable(break Region.cpp break.cpp Point.cpp Satellite.cpp utils.cpp Constellation.cpp)
add_executable(city Region.cpp city.cpp Point.cpp Satellite.cpp utils.cpp Constellation.cpp)

target_link_libraries(break m)
target_link_libraries(city m)

find_package(OpenMP REQUIRED)
target_link_libraries(break OpenMP::OpenMP_CXX)
target_link_libraries(city OpenMP::OpenMP_CXX)
