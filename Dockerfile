FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libglfw3-dev \
    libglew-dev \
    libglm-dev \
    xorg-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source code
COPY . .

# Remove any existing build artifacts to ensure a fresh build
RUN rm -rf build

# Create build directory and build the project
RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j4

# Set the entrypoint to run the simulator
WORKDIR /app/build
CMD ["./SatelliteSim"]
