#version 410 core
out vec4 FragColor;

in vec3 Color;
in vec3 Normal;

void main()
{
    // Simple lighting
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(Normal), lightDir), 0.2);
    FragColor = vec4(Color * diff, 1.0);
}

