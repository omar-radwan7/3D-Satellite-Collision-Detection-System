#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../render/Shader.h"
#include "../render/Buffers.h"

struct Satellite {
    int id;
    std::string name;
    // Keplerian Elements
    float semiMajorAxis; // km
    float eccentricity;
    float inclination; // radians
    float raan; // radians
    float argPeriapsis; // radians
    float meanAnomaly; // radians at epoch
    
    // Calculated state
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;
    
    bool active = true; // For destroying satellites
};

class SatelliteSystem {
public:
    SatelliteSystem();
    ~SatelliteSystem();
    
    void addSatellite(const Satellite& sat);
    void initOrbits(); // Generate orbit paths
    
    void update(float time);
    void drawSatellites(const glm::mat4& view, const glm::mat4& projection);
    void drawOrbits(const glm::mat4& view, const glm::mat4& projection);
    
    void destroySatellite(int id); // Mark as inactive
    
    const std::vector<Satellite>& getSatellites() const { return satellites; }
    
private:
    std::vector<Satellite> satellites;
    Shader* satShader;
    Shader* orbitShader;
    
    unsigned int satVAO, satVBO, satEBO, satInstanceVBO;
    unsigned int orbitVAO, orbitVBO;
    int orbitVertexCount = 0;
    int indexCount = 0;
    
    void initRenderData();
};
