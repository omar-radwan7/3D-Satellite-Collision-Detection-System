#include "SatelliteSystem.h"
#include "../sim/OrbitPropagator.h"
#include <cmath>
#include <iostream>

const float EARTH_RADIUS_KM = 6371.0f;
const float RENDER_SCALE = 1.0f / EARTH_RADIUS_KM; 

SatelliteSystem::SatelliteSystem() {
    satelliteShader = new Shader("shaders/satellite.vert", "shaders/satellite.frag");
    orbitShader = new Shader("shaders/orbit.vert", "shaders/orbit.frag");

    initSphereMesh();
}

SatelliteSystem::~SatelliteSystem() {
    delete satelliteShader;
    delete orbitShader;
    glDeleteVertexArrays(1, &satVAO);
    glDeleteBuffers(1, &satVBO);
    glDeleteBuffers(1, &satEBO);
    glDeleteBuffers(1, &instanceVBO);
    
    for(auto& line : orbitLines) {
        glDeleteVertexArrays(1, &line.vao);
        glDeleteBuffers(1, &line.vbo);
    }
}

void SatelliteSystem::initSphereMesh() {
    // Generate a small sphere for satellites
    // Radius 0.02 units (approx 120km visual size? A bit large but visible)
    // Real satellites are tiny. We make them visible.
    float radius = 0.015f;
    unsigned int sectorCount = 12;
    unsigned int stackCount = 12;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    const float PI = 3.14159265359f;

    float x, y, z, xy;
    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for(unsigned int i = 0; i <= stackCount; ++i) {
        stackAngle = PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for(unsigned int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            
            // pos (3) + normal (3)
            vertices.push_back(x); vertices.push_back(z); vertices.push_back(-y); // GL coords
            vertices.push_back(x/radius); vertices.push_back(z/radius); vertices.push_back(-y/radius);
        }
    }

    unsigned int k1, k2;
    for(unsigned int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;
        for(unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if(i != 0) {
                indices.push_back(k1); indices.push_back(k2); indices.push_back(k1 + 1);
            }
            if(i != (stackCount - 1)) {
                indices.push_back(k1 + 1); indices.push_back(k2); indices.push_back(k2 + 1);
            }
        }
    }

    sphereIndexCount = indices.size();

    glGenVertexArrays(1, &satVAO);
    glGenBuffers(1, &satVBO);
    glGenBuffers(1, &satEBO);
    
    glBindVertexArray(satVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, satVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, satEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Instance buffer (vec3 position + vec3 color)
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    // We will buffer data later
    
    // Instance Position (loc 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribDivisor(2, 1);

    // Instance Color (loc 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
}

void SatelliteSystem::addSatellite(Satellite sat) {
    satellites.push_back(sat);
    generateOrbitLine(sat);
}

void SatelliteSystem::generateOrbitLine(const Satellite& sat) {
    std::vector<float> points;
    int segments = 100;
    
    // Calculate period T = 2*pi*sqrt(a^3/mu)
    // mu = 398600.4418
    float mu = 398600.4418f;
    float T = 2.0f * 3.14159f * std::sqrt(std::pow(sat.semiMajorAxis, 3.0f) / mu);
    
    for(int i = 0; i <= segments; ++i) {
        float t = (float)i / segments * T;
        glm::vec3 pos = OrbitPropagator::CalculatePosition(sat, t);
        // Apply Scale
        pos *= RENDER_SCALE;
        points.push_back(pos.x);
        points.push_back(pos.y);
        points.push_back(pos.z);
    }

    OrbitLine line;
    line.vertexCount = points.size() / 3;
    
    glGenVertexArrays(1, &line.vao);
    glGenBuffers(1, &line.vbo);
    
    glBindVertexArray(line.vao);
    glBindBuffer(GL_ARRAY_BUFFER, line.vbo);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    orbitLines.push_back(line);
}

void SatelliteSystem::update(float currentTime) {
    if(satellites.empty()) return;
    
    instancePositions.clear();
    instanceColors.clear();

    for(auto& sat : satellites) {
        OrbitPropagator::Propagate(sat, currentTime);
        
        // Convert to render scale
        glm::vec3 renderPos = sat.position * RENDER_SCALE;
        instancePositions.push_back(renderPos);
        instanceColors.clear();
    }

    // Interleave position and color data
    std::vector<float> instanceData;
    for(size_t i = 0; i < satellites.size(); ++i) {
        instanceData.push_back(instancePositions[i].x);
        instanceData.push_back(instancePositions[i].y);
        instanceData.push_back(instancePositions[i].z);
        instanceData.push_back(satellites[i].color.x);
        instanceData.push_back(satellites[i].color.y);
        instanceData.push_back(satellites[i].color.z);
    }

    // Update instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(float), instanceData.data(), GL_STREAM_DRAW);
}

void SatelliteSystem::drawSatellites(const glm::mat4& view, const glm::mat4& projection) {
    if(satellites.empty()) return;

    satelliteShader->use();
    satelliteShader->setMat4("view", view);
    satelliteShader->setMat4("projection", projection);

    glBindVertexArray(satVAO);
    glDrawElementsInstanced(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0, satellites.size());
    glBindVertexArray(0);
}

void SatelliteSystem::drawOrbits(const glm::mat4& view, const glm::mat4& projection) {
    orbitShader->use();
    orbitShader->setMat4("view", view);
    orbitShader->setMat4("projection", projection);
    
    // For better performance we could batch lines, but individual draw calls for <2000 is fine on desktop
    for(size_t i = 0; i < orbitLines.size(); ++i) {
        orbitShader->setVec3("color", satellites[i].color * 0.5f); // Dimmer orbit lines
        glBindVertexArray(orbitLines[i].vao);
        glDrawArrays(GL_LINE_STRIP, 0, orbitLines[i].vertexCount);
    }
    glBindVertexArray(0);
}

