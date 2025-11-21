#version 410 core
out vec4 FragColor;

in vec3 Color;
in vec3 FragPos;
in vec3 LocalPos;
in float BeaconState;

void main()
{
    // Simple directional lighting from sun
    vec3 lightDir = normalize(vec3(1.0, 0.3, 0.8));
    vec3 normal = normalize(cross(dFdx(FragPos), dFdy(FragPos)));
    
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Check if this is the beacon light
    float distXY = length(LocalPos.xy);
    float absY = abs(LocalPos.y);
    bool isBeacon = (absY > 0.0055 && distXY < 0.003);
    
    vec3 finalColor;
    if (isBeacon) {
        // Beacon light: red glow when active
        if (BeaconState > 0.5) {
            // Bright red when on
            finalColor = vec3(1.0, 0.1, 0.1) * 1.5; // Bright red
        } else {
            // Dim red when off
            finalColor = vec3(0.2, 0.0, 0.0);
        }
    } else {
        // Normal satellite materials with lighting
        vec3 ambient = Color * 0.4;
        vec3 diffuse = Color * diff * 0.8;
        finalColor = ambient + diffuse;
    }
    
    FragColor = vec4(finalColor, 1.0);
}
