cmake_minimum_required(VERSION 3.19)
project("exotic-hashing" VERSION 1.0
        DESCRIPTION "A header only cmake/c++ library that exposes various state-of-the-art exotic hash functions"
        HOMEPAGE_URL "https://github.com/andreaskipf/hashing")

# Declare library & directories to include. See
# http://mariobadr.com/creating-a-header-only-library-with-cmake.html for more
# info/install instructions
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

# Make IDE friendly
target_sources(${PROJECT_NAME} INTERFACE exotic_hashing.hpp include/)

# Benchmark code
add_subdirectory(src)
