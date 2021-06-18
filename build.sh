#!/bin/bash

# Setup script 
source .env
set -e
cd "$(dirname "$0")"

# Parse arguments
TARGET=${1:-"benchmarks"}
BUILD_TYPE=${2:-"RELEASE"}
BUILD_DIR="cmake-build-$(echo "${BUILD_TYPE}" | awk '{print tolower($0)}')/"

# Generate cmake project files
cmake \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DCMAKE_C_COMPILER=${C_COMPILER}\
  -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
  -B ${BUILD_DIR} \
  .

# Build tests code
cmake \
  --build ${BUILD_DIR} \
  \ #--clean-first \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DCMAKE_C_COMPILER=${C_COMPILER}\
  -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
  --target ${TARGET} \
  -j
