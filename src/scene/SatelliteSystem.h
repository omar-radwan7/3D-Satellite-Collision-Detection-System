#pragma once
#include <vector>
#include "Satellite.h"
#include "../render/Shader.h"
#include "../render/Buffers.h"
#include <glm/glm.hpp>

struct OrbitLine {
    unsigned int vao;
    unsigned int vbo;
    int vertexCount;
};

class SatelliteSystem {
public:
    SatelliteSystem();
    ~SatelliteSystem();

    void addSatellite(Satellite sat);
    void update(float currentTime);
    void drawSatellites(const glm::mat4& view, const glm::mat4& projection);
    void drawOrbits(const glm::mat4& view, const glm::mat4& projection);
    
    std::vector<Satellite>& getSatellites() { return satellites; }

private:
    std::vector<Satellite> satellites;
    std::vector<OrbitLine> orbitLines;
    
    Shader* satelliteShader;
    Shader* orbitShader;

    // For instanced rendering of satellites
    unsigned int satVAO, satVBO, satEBO;
    unsigned int instanceVBO; 
    int sphereIndexCount;
    
    // Data for instancing
    std::vector<glm::vec3> instancePositions;
    std::vector<glm::vec3> instanceColors;

    void initSphereMesh();
    void generateOrbitLine(const Satellite& sat);
};

