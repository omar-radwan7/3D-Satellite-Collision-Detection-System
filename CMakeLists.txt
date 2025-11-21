cmake_minimum_required(VERSION 3.16)
project(SatelliteCollisionSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Fetch nlohmann_json
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# Fetch ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# Fetch stb
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# Create a library for ImGui to handle the sources
add_library(imgui_lib
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_lib PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui_lib PUBLIC glfw GLEW::GLEW OpenGL::GL X11)

# Main executable
add_executable(SatelliteSim
    src/app/main.cpp
    src/render/Shader.cpp
    src/render/Buffers.cpp
    src/render/Camera.cpp
    src/scene/Earth.cpp
    src/scene/SatelliteSystem.cpp
    src/scene/DebrisSystem.cpp
    src/sim/OrbitPropagator.cpp
    src/sim/CollisionDetect.cpp
    src/ui/GuiManager.cpp
    src/util/TextureLoader.cpp
    src/util/ConfigLoader.cpp
)

target_include_directories(SatelliteSim PRIVATE
    src
    ${stb_SOURCE_DIR}
)

target_link_libraries(SatelliteSim PRIVATE
    glfw
    GLEW::GLEW
    OpenGL::GL
    glm::glm
    nlohmann_json::nlohmann_json
    imgui_lib
)

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
