cmake_minimum_required(VERSION 3.19)
project("exotic-hashing" VERSION 1.0
        DESCRIPTION "A header only cmake/c++ library that exposes various state-of-the-art exotic hash functions"
        HOMEPAGE_URL "https://github.com/DominikHorn/exotic-hashing"
        LANGUAGES CXX)

# Declare library & directories to include. See
# http://mariobadr.com/creating-a-header-only-library-with-cmake.html for more
# info/install instructions
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# ==== Dependencies ====
include(${PROJECT_SOURCE_DIR}/thirdparty/sdsl-lite.cmake)
include(${PROJECT_SOURCE_DIR}/thirdparty/mmphf_fst.cmake)
include(${PROJECT_SOURCE_DIR}/thirdparty/hashing.cmake)
include(${PROJECT_SOURCE_DIR}/thirdparty/learned_hashing.cmake)

target_link_libraries(${PROJECT_NAME} INTERFACE ${SDSL_LIBRARY} ${HASHING_LIBRARY} ${LEARNED_HASHING_LIBRARY} ${MMPHF_FST_LIBRARY})

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-fconstexpr-steps=999999999 CONSTEXPR_RECURSION_DEPTH_CONFIGURABLE)
if (CONSTEXPR_RECURSION_DEPTH_CONFIGURABLE)
  target_compile_options(${PROJECT_NAME} INTERFACE -fconstexpr-steps=999999999)
endif()

# Make IDE friendly
target_sources(${PROJECT_NAME} INTERFACE exotic_hashing.hpp include/)

# Benchmark and test code
get_directory_property(hasParent PARENT_DIRECTORY)
if (NOT hasParent)
  add_subdirectory(src)
endif()
