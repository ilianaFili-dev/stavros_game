#!/bin/sh
# Preflight the sheets, generate code, run tests and cross-build the mod DLL.
# usage: tools/build.sh [milestone]   (default M0)
set -e
cd "$(dirname "$0")/.."
M="${1:-M0}"
python3 tools/preflight.py --milestone "$M"
python3 tools/gen.py --milestone "$M"
mkdir -p build/test
g++ -std=c++17 -Wall -Wextra -o build/test/test_locator tests/test_locator.cpp src/clonedrone_locator.cpp
./build/test/test_locator
cmake -S . -B build/win -DCMAKE_TOOLCHAIN_FILE=cmake-mingw.cmake -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build build/win -j
ls -l build/win/dinput8.dll
