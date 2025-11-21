#version 410 core
out vec4 FragColor;

in vec3 Normal;

uniform vec3 color;

void main()
{
    // Simple lighting
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(Normal), lightDir), 0.5);
    FragColor = vec4(color * diff, 1.0);
}

