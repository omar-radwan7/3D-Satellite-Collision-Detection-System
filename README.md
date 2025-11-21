3D Satellite Collision Detection and Risk Assessment System

A real-time 3D visualization tool for satellite orbital mechanics and collision risk assessment. This space traffic control simulator models how satellites orbit Earth, predicts potential near-miss events hours in advance, and visualizes what happens when satellites collide.

The system uses real orbital mechanics to track satellites and analyze their future positions, identifying when and where they might get dangerously close. The visualization shows Earth from space with satellite orbits as colored lines, and highlights danger zones with visual markers when collision risks are detected.

![Main View](screenshots/![main-view](https://github.com/user-attachments/assets/501cb70f-44ea-4898-87d3-9aafa059d4a7)
main-view.png)

The main view is rendered using OpenGL 4.1 with GLFW for windowing. Earth uses texture mapping with albedo and normal maps for realistic surface detail. Satellite orbits are drawn as colored line segments using OpenGL line primitives. The scene uses GLM for coordinate transformations and perspective projection.

Key Features

The orbital risk prediction engine looks ahead up to one hour, analyzing every satellite pair to find conjunction events. It calculates risk scores based on proximity, relative velocity, and altitude, providing early warning of dangerous situations.

When a near-miss is predicted, pulsing geometric markers appear at the closest approach point and tubular danger corridors show the risk zone. Everything is color-coded from green for safe distances to red for critical threats.

If satellites actually collide, the system generates realistic debris field visualizations showing how collision fragments disperse in space. The debris spreads outward in two distinct clouds, one for each satellite, demonstrating how collision fragments would disperse in space. This helps visualize the cascading debris problem that makes space collisions so dangerous.
<img width="1117" height="749" alt="debris-explosion" src="https://github.com/user-attachments/assets/15cc2b52-0572-4ade-ae92-73163e45eadf" />

![Debris Explosion](screenshots/debris-explosion.png)

Debris visualization uses OpenGL line rendering with custom shaders. Each debris particle is drawn as a vector line segment representing velocity direction. The two-color system uses vertex color attributes to distinguish debris from each satellite. Particle positions are calculated using orbital mechanics and rendered with instanced drawing for performance.

The Earth model is fully interactive - you can rotate it by holding Shift and dragging, and it auto-rotates to show time passing.

The mission control interface provides playback controls, speed adjustment up to 500x normal speed, and multiple information panels showing satellite status, active collisions, and detailed conjunction analysis. Satellites are color-coded by altitude - cyan for low Earth orbit, green for medium Earth orbit, and red for geostationary orbit.

Satellite Technology and Orbital Mechanics

Satellites are modeled as 3D objects with central body modules, solar panel arrays, antenna dishes, and flashing beacon lights. The geometry is defined procedurally using OpenGL vertex buffers and rendered with instanced drawing for efficiency. GLM handles all vector and matrix operations for positioning and transformations.
![satellite-detail](https://github.com/user-attachments/assets/9c276978-ed01-4759-9d10-ff16e5bb1424)

![Satellite Detail](screenshots/satellite-detail.png)

Satellites use standard Keplerian orbital elements: semi-major axis, eccentricity, inclination, right ascension of ascending node, argument of periapsis, and mean anomaly. Movement calculations solve the two-body problem using Kepler's equation to determine position at any time. The orbital propagation engine calculates both position and velocity vectors by converting from orbital plane coordinates to Earth-Centered Inertial coordinates using rotation matrices. Velocity uses the vis-viva equation based on distance from Earth's center and semi-major axis. Positions update each frame based on simulation time, with orbital periods calculated from semi-major axis using Kepler's third law.

How to Run

Linux

Using Docker is the easiest method. Build the container with:

```
docker build -t sat-sim .
```

Allow X11 connections and run:

```
xhost +local:docker
docker run -it --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --device /dev/dri sat-sim
```

Inside the container, compile and run:

```
cd /app/build
cmake ..
make -j4
./SatelliteSim
```

For native Linux builds, install dependencies:

```
sudo apt-get install build-essential cmake libglfw3-dev libglew-dev libglm-dev libx11-dev
```

Then build:

```
mkdir build && cd build
cmake ..
make -j4
./SatelliteSim
```

macOS

Install dependencies with Homebrew:

```
brew install cmake glfw glew glm
```

Then build:

```
mkdir build && cd build
cmake ..
make -j4
./SatelliteSim
```

Requires OpenGL 4.1 support, available on most Macs from 2012 onwards.

Windows

Install dependencies using vcpkg:

```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install glfw3 glew glm
```

Configure and build with CMake:

```
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

Run the executable from the Release directory.
