#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aInstancePos;
layout (location = 3) in vec3 aInstanceColor;

uniform mat4 view;
uniform mat4 projection;

out vec3 Color;
out vec3 Normal;

void main()
{
    // Instance position is world space
    vec3 WorldPos = aPos + aInstancePos;
    gl_Position = projection * view * vec4(WorldPos, 1.0);
    
    Normal = aNormal;
    Color = aInstanceColor;
}

