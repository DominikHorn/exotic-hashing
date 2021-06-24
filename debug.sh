
#!/bin/bash

# Setup script 
source .env
set -e
cd "$(dirname "$0")"

# Parse arguments
TARGET=${1:-"benchmarks"}
BUILD_TYPE=DEBUG
BUILD_DIR="cmake-build-$(echo "${BUILD_TYPE}" | awk '{print tolower($0)}')/"

# Build the target
./build.sh ${TARGET} ${BUILD_TYPE}

# Execute the target
lldb -o "run" -b ${BUILD_DIR}/src/${TARGET}
