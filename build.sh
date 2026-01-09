#!/bin/bash
set -e

# ล้าง build folder เก่า (ป้องกัน path เก่า)
if [ -d "build" ]; then
    echo "Removing old build folder..."
    rm -rf build
fi

echo "Configuring..."
cmake -S . -B build

echo "Building..."
cmake --build build

echo "Running..."
EXECUTABLE="./build/main"
if [ -f "$EXECUTABLE" ]; then
    # ให้สิทธิ์รันก่อน
    chmod +x "$EXECUTABLE"
    "$EXECUTABLE"
else
    echo "Error: Executable not found!"
    exit 1
fi
