#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aInstancePos;
layout (location = 3) in vec3 aInstanceColor;
layout (location = 4) in float aBeaconState;

uniform mat4 view;
uniform mat4 projection;

out vec3 Color;
out vec3 FragPos;
out vec3 LocalPos;
out float BeaconState;

void main()
{
    vec3 WorldPos = aPos + aInstancePos;
    gl_Position = projection * view * vec4(WorldPos, 1.0);
    
    LocalPos = aPos;
    FragPos = WorldPos;
    BeaconState = aBeaconState;
    
    // Material-based coloring (ISS-style)
    float distXY = length(aPos.xy);
    float absZ = abs(aPos.z);
    float absY = abs(aPos.y);
    
    // Beacon light (top of satellite, small sphere)
    if (absY > 0.0055 && distXY < 0.003) {
        // Red beacon light - brightness controlled by BeaconState
        Color = vec3(1.0, 0.0, 0.0);
    }
    // Main body module: White/Silver
    else if (distXY < 0.008 && absZ < 0.015) {
        Color = vec3(0.85, 0.87, 0.90);
    }
    // Solar panels (far from center in X/Y)
    else if (abs(aPos.x) > 0.01 || abs(aPos.y) > 0.008) {
        Color = vec3(0.05, 0.05, 0.15);
    }
    // Antenna dishes (front, circular)
    else if (aPos.z > 0.008) {
        Color = vec3(0.7, 0.7, 0.75);
    }
    else {
        Color = vec3(0.8, 0.75, 0.65);
    }
}
