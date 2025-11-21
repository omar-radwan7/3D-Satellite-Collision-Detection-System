#version 410 core
out vec4 FragColor;

uniform vec3 color;

void main()
{
    FragColor = vec4(color, 0.6); // Alpha 0.6
}

