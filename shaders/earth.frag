#version 410 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform vec3 viewPos;
uniform vec3 lightDir; // Direction TO the light source

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDirection = normalize(lightDir);

    // Diffuse
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * texture(albedoMap, TexCoords).rgb;

    // Ambient - increased visibility for dark side
    vec3 ambient = 0.3 * texture(albedoMap, TexCoords).rgb;

    // Rim lighting (Atmosphere)
    float rim = 1.0 - max(dot(viewDir, norm), 0.0);
    rim = smoothstep(0.6, 1.0, rim);
    vec3 atmosphere = vec3(0.0, 0.4, 0.8) * rim * 0.5;

    // Specular (simple)
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = vec3(0.2) * spec; // Ocean specular roughly

    vec3 result = ambient + diffuse + atmosphere + specular;
    FragColor = vec4(result, 1.0);
}

