#!/bin/bash

# Build the Docker image
echo "Building Docker image..."
docker build -t sat-sim .

# Run the container with X11 forwarding
echo "Running Satellite Simulator..."
xhost +local:docker
docker run -it --rm \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    --device /dev/dri \
    sat-sim

