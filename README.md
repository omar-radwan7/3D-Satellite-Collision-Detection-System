<<<<<<< HEAD
# 3D-Satellite-Collision-Detection-System
=======
# Satellite Collision Simulator

A 3D visualization of satellite orbits and potential collisions using C++ and OpenGL 4.1.

## Features

- **Earth Rendering**: Textures (Albedo + Normal) and atmospheric rim shader.
- **Orbital Mechanics**: Keplerian orbit propagation (using Earth Centered Inertial frame).
- **Visualization**: 
  - Orbit paths (polylines).
  - Satellite markers (color-coded by altitude).
  - Debris explosions on collision events.
- **Collision Detection**: Real-time distance checking between satellites.
- **UI**: ImGui-based simulation controls and inspector.

## Visual Reference
Target visual style: `/mnt/data/a65372dd-a09b-4f1a-b9a8-6adb1ad6f54e.png`

## Requirements

- Docker (recommended for build environment)
- Or manual install:
  - C++20 compliant compiler (GCC/Clang)
  - CMake 3.16+
  - GLFW 3
  - GLEW
  - GLM

## Build & Run with Docker

1. Build the Docker image:
   ```bash
   docker build -t sat-sim .
   ```

2. Run the container (mounting X11 socket for GUI):
   ```bash
   # Allow local X connections (be careful with security)
   xhost +local:docker
   
   docker run -it --rm \
     -e DISPLAY=$DISPLAY \
     -v /tmp/.X11-unix:/tmp/.X11-unix \
     --device /dev/dri \
     sat-sim
   ```
   *Note: This requires an X11 server and GPU passthrough support on Linux.*

   **Inside the container:**
   ```bash
   # You are already in the /app/build directory
   cmake ..
   make -j4
   ./SatelliteSim
   ```

## Build Manually (Linux)

1. Install dependencies:
   ```bash
   sudo apt-get install build-essential cmake libglfw3-dev libglew-dev libglm-dev
   ```

2. Build:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

3. Run:
   ```bash
   ./SatelliteSim
   ```

## Controls

- **WASD**: Move camera
- **Mouse**: Look around (Right-click to capture mouse if UI is active)
- **Scroll**: Zoom
- **ESC**: Exit
- **UI**: Use the "Simulation Controls" panel to pause, speed up, or inspect satellites.

## Configuration

Edit `assets/config/satellites.json` to add custom satellites with Keplerian elements.

>>>>>>> 475ee3c (3d satellite collision detector system)
