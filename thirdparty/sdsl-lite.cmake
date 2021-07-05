include(ExternalProject)
find_package(Git REQUIRED)

# library name
set(SDSL_LIBRARY sdsl-lite)

ExternalProject_Add(
        ${SDSL_LIBRARY}_src
        PREFIX external/${SDSL_LIBRARY}
        GIT_REPOSITORY "https://github.com/simongog/sdsl-lite.git"
        GIT_TAG c32874c
        TIMEOUT 10
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/external/${SDSL_LIBRARY}
        -DCMAKE_INSTALL_LIBDIR=${PROJECT_BINARY_DIR}/external/${SDSL_LIBRARY}/lib
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DBENCHMARK_ENABLE_GTEST_TESTS=0
        UPDATE_COMMAND ""
        #BUILD_BYPRODUCTS <INSTALL_DIR>/lib/libbenchmark.a
)

# path to installed artifacts
ExternalProject_Get_Property(${SDSL_LIBRARY}_src install_dir)
set(SDSL_INCLUDE_DIR ${install_dir}/include)
set(SDSL_LIBRARY_PATH ${install_dir}/lib/libsdsl.a)

# build library from external project
file(MAKE_DIRECTORY ${SDSL_INCLUDE_DIR})
add_library(${SDSL_LIBRARY} STATIC IMPORTED)
set_property(TARGET ${SDSL_LIBRARY} PROPERTY IMPORTED_LOCATION ${SDSL_LIBRARY_PATH})
set_property(TARGET ${SDSL_LIBRARY} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${SDSL_INCLUDE_DIR})
add_dependencies(${SDSL_LIBRARY} ${SDSL_LIBRARY}_src)

message(STATUS "[SDSL] settings")
message(STATUS "    SDSL_LIBRARY = ${SDSL_LIBRARY}")
message(STATUS "    SDSL_INCLUDE_DIR = ${SDSL_INCLUDE_DIR}")
message(STATUS "    SDSL_LIBRARY_PATH = ${SDSL_LIBRARY_PATH}")
