#!/usr/bin/env bash

# Runs clang-tidy on the codebase using compile_commands.json

BUILD_DIR="build_tidy"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Configuring CMake in $BUILD_DIR..."
    cmake -B "$BUILD_DIR" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

# Run clang tidy on all files
echo "Running clang-tidy on files..."
find src apps -name "*.cc" -o -name "*.cpp" | xargs clang-tidy -p "$BUILD_DIR"
