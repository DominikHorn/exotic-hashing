cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -march=native -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -static-libsan -fsanitize=address,leak,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_executable(benchmarks benchmarks.cpp)

# Add (thirdparty) libraries to project scope
add_subdirectory(hashtable)
target_link_libraries(benchmarks PRIVATE exotic-hashing hashtable ${GOOGLEBENCHMARK_LIBRARY} ${SDSL_LIBRARY})
