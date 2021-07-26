#!/bin/bash

# setup script
set -e
cd "$(dirname "$0")"

# build and run tests
./build.sh tests RELEASE
cmake-build-release/src/tests
