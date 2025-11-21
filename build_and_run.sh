#!/bin/bash

cd "$(dirname "$0")/build"

echo "=== Running CMake ==="
cmake ..

echo ""
echo "=== Building Project ==="
make -j4

if [ $? -eq 0 ]; then
    echo ""
    echo "=== Build Successful! ==="
    echo "Running SatelliteSim..."
    echo ""
    ./SatelliteSim
else
    echo ""
    echo "=== Build Failed ==="
    exit 1
fi

