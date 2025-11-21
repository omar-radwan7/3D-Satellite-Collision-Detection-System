#version 410 core
out vec4 FragColor;

in vec3 Color;

void main()
{
    // Use vertex color (debris/warning color)
    // Add slight glow/transparency logic if needed, but opaque is best for visibility
    FragColor = vec4(Color, 1.0); 
}

