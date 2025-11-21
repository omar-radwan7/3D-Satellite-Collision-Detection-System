#!/bin/bash
cd "$(dirname "$0")/build"
cmake ..
make -j4
if [ $? -eq 0 ]; then
    ./SatelliteSim
else
    exit 1
fi

