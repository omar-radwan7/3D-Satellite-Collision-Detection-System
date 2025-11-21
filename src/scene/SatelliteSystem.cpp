#include "SatelliteSystem.h"
#include "../sim/OrbitPropagator.h"
#include <GL/glew.h>
#include <cmath>
#include <vector>
#include <iostream>

SatelliteSystem::SatelliteSystem() {
    satShader = new Shader("shaders/satellite.vert", "shaders/satellite.frag");
    orbitShader = new Shader("shaders/orbit.vert", "shaders/orbit.frag");
    initRenderData();
}

SatelliteSystem::~SatelliteSystem() {
    delete satShader;
    delete orbitShader;
    glDeleteVertexArrays(1, &satVAO);
    glDeleteBuffers(1, &satVBO);
    glDeleteBuffers(1, &satEBO);
    glDeleteBuffers(1, &satInstanceVBO);
    glDeleteVertexArrays(1, &orbitVAO);
    glDeleteBuffers(1, &orbitVBO);
}

void SatelliteSystem::addSatellite(const Satellite& sat) {
    satellites.push_back(sat);
}

void SatelliteSystem::destroySatellite(int id) {
    for(auto& sat : satellites) {
        if(sat.id == id) {
            sat.active = false;
            std::cout << "Satellite " << id << " destroyed and removed from map." << std::endl;
            break;
        }
    }
}

void SatelliteSystem::update(float time) {
    std::vector<float> instanceData;
    instanceData.reserve(satellites.size() * 7); // pos(3) + color(3) + beaconState(1)
    
    for(auto& sat : satellites) {
        OrbitPropagator::Propagate(sat, time);
        
        // Only add to instance buffer if active
        if(!sat.active) continue;

        glm::vec3 renderPos = sat.position * (1.0f / 6371.0f);
        
        instanceData.push_back(renderPos.x);
        instanceData.push_back(renderPos.y);
        instanceData.push_back(renderPos.z);
        instanceData.push_back(sat.color.r);
        instanceData.push_back(sat.color.g);
        instanceData.push_back(sat.color.b);
        
        // Beacon flash state (red light blinks)
        // Each satellite blinks at different rate based on ID
        float blinkSpeed = 2.0f + (sat.id % 10) * 0.3f; // Varied blink rates
        float beaconState = (sin(time * blinkSpeed) > 0.0f) ? 1.0f : 0.0f;
        instanceData.push_back(beaconState);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, satInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(float), instanceData.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SatelliteSystem::initRenderData() {
    // ISS-style satellite: Central body + 4 solar panel arrays + antenna dishes
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Made 3x smaller
    float bodyW = 0.007f;   // Body width
    float bodyH = 0.005f;   // Body height
    float bodyL = 0.013f;   // Body length
    
    float panelW = 0.04f;   // Large panels
    float panelH = 0.012f;
    float panelT = 0.0007f;
    
    float dishRad = 0.005f;
    int dishSegs = 12;
    
    // Add beacon light position at the top
    float beaconX = 0.0f;
    float beaconY = bodyH + 0.003f;
    float beaconZ = 0.0f;
    float beaconRad = 0.002f;
    
    // ====== CENTRAL MODULE (Box) ======
    float box[] = {
        // Front face
        -bodyW, -bodyH,  bodyL/2,
         bodyW, -bodyH,  bodyL/2,
         bodyW,  bodyH,  bodyL/2,
        -bodyW,  bodyH,  bodyL/2,
        // Back face
        -bodyW, -bodyH, -bodyL/2,
         bodyW, -bodyH, -bodyL/2,
         bodyW,  bodyH, -bodyL/2,
        -bodyW,  bodyH, -bodyL/2,
    };
    
    int baseIdx = 0;
    for(int i = 0; i < 24; i += 3) {
        vertices.push_back(box[i]);
        vertices.push_back(box[i+1]);
        vertices.push_back(box[i+2]);
    }
    
    // Box indices
    int b = baseIdx;
    int boxInds[] = {
        b+0,b+1,b+2, b+0,b+2,b+3, // Front
        b+4,b+6,b+5, b+4,b+7,b+6, // Back
        b+0,b+4,b+5, b+0,b+5,b+1, // Bottom
        b+2,b+6,b+7, b+2,b+7,b+3, // Top
        b+1,b+5,b+6, b+1,b+6,b+2, // Right
        b+0,b+3,b+7, b+0,b+7,b+4  // Left
    };
    for(int i : boxInds) indices.push_back(i);
    
    // ====== TOP SOLAR PANELS (2 arrays) ======
    baseIdx = vertices.size() / 3;
    float topY = bodyH + panelT;
    
    // Left top panel
    float tleft[] = {
        -bodyW - panelW, topY - panelT, -panelH,
        -bodyW,          topY - panelT, -panelH,
        -bodyW,          topY - panelT,  panelH,
        -bodyW - panelW, topY - panelT,  panelH,
        
        -bodyW - panelW, topY + panelT, -panelH,
        -bodyW,          topY + panelT, -panelH,
        -bodyW,          topY + panelT,  panelH,
        -bodyW - panelW, topY + panelT,  panelH,
    };
    for(int i=0; i<24; ++i) vertices.push_back(tleft[i]);
    
    // Indices for tleft
    int tl = baseIdx;
    // Same box indices pattern
    int tlInds[] = {
        tl+0,tl+1,tl+2, tl+0,tl+2,tl+3,
        tl+4,tl+6,tl+5, tl+4,tl+7,tl+6,
        tl+0,tl+4,tl+5, tl+0,tl+5,tl+1,
        tl+2,tl+6,tl+7, tl+2,tl+7,tl+3,
        tl+1,tl+5,tl+6, tl+1,tl+6,tl+2,
        tl+0,tl+3,tl+7, tl+0,tl+7,tl+4
    };
    for(int i : tlInds) indices.push_back(i);
    
    // Right top panel
    baseIdx = vertices.size() / 3;
    float tright[] = {
         bodyW,          topY - panelT, -panelH,
         bodyW + panelW, topY - panelT, -panelH,
         bodyW + panelW, topY - panelT,  panelH,
         bodyW,          topY - panelT,  panelH,
        
         bodyW,          topY + panelT, -panelH,
         bodyW + panelW, topY + panelT, -panelH,
         bodyW + panelW, topY + panelT,  panelH,
         bodyW,          topY + panelT,  panelH,
    };
    for(int i=0; i<24; ++i) vertices.push_back(tright[i]);
    
    int tr = baseIdx;
    int trInds[] = {
        tr+0,tr+1,tr+2, tr+0,tr+2,tr+3,
        tr+4,tr+6,tr+5, tr+4,tr+7,tr+6,
        tr+0,tr+4,tr+5, tr+0,tr+5,tr+1,
        tr+2,tr+6,tr+7, tr+2,tr+7,tr+3,
        tr+1,tr+5,tr+6, tr+1,tr+6,tr+2,
        tr+0,tr+3,tr+7, tr+0,tr+7,tr+4
    };
    for(int i : trInds) indices.push_back(i);

    // ====== BOTTOM SOLAR PANELS (2 arrays) ======
    float botY = -bodyH - panelT;
    
    // Left bottom panel
    baseIdx = vertices.size() / 3;
    float bleft[] = {
        -bodyW - panelW, botY - panelT, -panelH,
        -bodyW,          botY - panelT, -panelH,
        -bodyW,          botY - panelT,  panelH,
        -bodyW - panelW, botY - panelT,  panelH,
        
        -bodyW - panelW, botY + panelT, -panelH,
        -bodyW,          botY + panelT, -panelH,
        -bodyW,          botY + panelT,  panelH,
        -bodyW - panelW, botY + panelT,  panelH,
    };
    for(int i=0; i<24; ++i) vertices.push_back(bleft[i]);
    int bl = baseIdx;
    int blInds[] = {
        bl+0,bl+1,bl+2, bl+0,bl+2,bl+3,
        bl+4,bl+6,bl+5, bl+4,bl+7,bl+6,
        bl+0,bl+4,bl+5, bl+0,bl+5,bl+1,
        bl+2,bl+6,bl+7, bl+2,bl+7,bl+3,
        bl+1,bl+5,bl+6, bl+1,bl+6,bl+2,
        bl+0,bl+3,bl+7, bl+0,bl+7,bl+4
    };
    for(int i : blInds) indices.push_back(i);

    // Right bottom panel
    baseIdx = vertices.size() / 3;
    float bright[] = {
         bodyW,          botY - panelT, -panelH,
         bodyW + panelW, botY - panelT, -panelH,
         bodyW + panelW, botY - panelT,  panelH,
         bodyW,          botY - panelT,  panelH,
        
         bodyW,          botY + panelT, -panelH,
         bodyW + panelW, botY + panelT, -panelH,
         bodyW + panelW, botY + panelT,  panelH,
         bodyW,          botY + panelT,  panelH,
    };
    for(int i=0; i<24; ++i) vertices.push_back(bright[i]);
    int br = baseIdx;
    int brInds[] = {
        br+0,br+1,br+2, br+0,br+2,br+3,
        br+4,br+6,br+5, br+4,br+7,br+6,
        br+0,br+4,br+5, br+0,br+5,br+1,
        br+2,br+6,br+7, br+2,br+7,br+3,
        br+1,br+5,br+6, br+1,br+6,br+2,
        br+0,br+3,br+7, br+0,br+7,br+4
    };
    for(int i : brInds) indices.push_back(i);

    // ====== DISHES (Cylinders/Cones) ======
    // Front Dish
    baseIdx = vertices.size() / 3;
    float dishZ = bodyL/2 + 0.002f;
    float dish1X = -bodyW/2;
    
    vertices.push_back(dish1X);
    vertices.push_back(0.0f);
    vertices.push_back(dishZ);
    
    for(int i = 0; i <= dishSegs; ++i) {
        float angle = i * 2.0f * 3.14159f / dishSegs;
        vertices.push_back(dish1X + dishRad * cos(angle));
        vertices.push_back(dishRad * sin(angle));
        vertices.push_back(dishZ);
    }
    
    int centerIdx = baseIdx;
    for(int i = 0; i < dishSegs; ++i) {
        indices.push_back(centerIdx);
        indices.push_back(centerIdx + i + 1);
        indices.push_back(centerIdx + i + 2);
    }

    // Back/Second Dish
    baseIdx = vertices.size() / 3;
    float dish2X = bodyW/2;
    
    vertices.push_back(dish2X);
    vertices.push_back(0.0f);
    vertices.push_back(dishZ);
    
    for(int i = 0; i <= dishSegs; ++i) {
        float angle = i * 2.0f * 3.14159f / dishSegs;
        vertices.push_back(dish2X + dishRad * cos(angle));
        vertices.push_back(dishRad * sin(angle));
        vertices.push_back(dishZ);
    }
    
    centerIdx = baseIdx;
    for(int i = 0; i < dishSegs; ++i) {
        indices.push_back(centerIdx);
        indices.push_back(centerIdx + i + 1);
        indices.push_back(centerIdx + i + 2);
    }
    
    // ====== BEACON LIGHT (small sphere on top) ======
    baseIdx = vertices.size() / 3;
    int beaconSegs = 8;
    
    // Simple sphere for beacon
    vertices.push_back(beaconX);
    vertices.push_back(beaconY);
    vertices.push_back(beaconZ);
    
    for(int i = 0; i <= beaconSegs; ++i) {
        float angle = i * 2.0f * 3.14159f / beaconSegs;
        vertices.push_back(beaconX + beaconRad * cos(angle));
        vertices.push_back(beaconY);
        vertices.push_back(beaconZ + beaconRad * sin(angle));
    }
    
    centerIdx = baseIdx;
    for(int i = 0; i < beaconSegs; ++i) {
        indices.push_back(centerIdx);
        indices.push_back(centerIdx + i + 1);
        indices.push_back(centerIdx + i + 2);
    }
    
    // Setup OpenGL buffers
    glGenVertexArrays(1, &satVAO);
    glGenBuffers(1, &satVBO);
    glGenBuffers(1, &satEBO);
    glGenBuffers(1, &satInstanceVBO);

    glBindVertexArray(satVAO);

    glBindBuffer(GL_ARRAY_BUFFER, satVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, satEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, satInstanceVBO);
    
    // Instance position (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glVertexAttribDivisor(2, 1);
    
    // Instance color (location = 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribDivisor(3, 1);
    
    // Beacon state (location = 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);
    
    indexCount = indices.size();
    
    glGenVertexArrays(1, &orbitVAO);
    glGenBuffers(1, &orbitVBO);
}

void SatelliteSystem::initOrbits() {
    std::vector<float> orbitData;
    
    // Generate orbit path for each satellite
    for(const auto& sat : satellites) {
        int numPoints = 200;
        float timeStep = (2.0f * 3.14159f) / numPoints * sqrt(sat.semiMajorAxis * sat.semiMajorAxis * sat.semiMajorAxis / 398600.4418f);
        
        for(int i = 0; i <= numPoints; ++i) {
            float t = i * timeStep;
            glm::vec3 pos = OrbitPropagator::CalculatePosition(sat, t);
            glm::vec3 renderPos = pos * (1.0f / 6371.0f);
            
            orbitData.push_back(renderPos.x);
            orbitData.push_back(renderPos.y);
            orbitData.push_back(renderPos.z);
            orbitData.push_back(sat.color.r * 0.4f); // Dimmer orbit lines
            orbitData.push_back(sat.color.g * 0.4f);
            orbitData.push_back(sat.color.b * 0.4f);
        }
    }
    
    orbitVertexCount = orbitData.size() / 6;
    
    if(orbitVertexCount > 0) {
        glBindVertexArray(orbitVAO);
        glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
        glBufferData(GL_ARRAY_BUFFER, orbitData.size() * sizeof(float), orbitData.data(), GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        
        glBindVertexArray(0);
    }
}

void SatelliteSystem::drawSatellites(const glm::mat4& view, const glm::mat4& projection) {
    // Count active satellites for instanced drawing
    int activeCount = 0;
    for(const auto& s : satellites) {
        if(s.active) activeCount++;
    }
    
    if(activeCount == 0) return; // Don't draw if no active satellites
    
    satShader->use();
    satShader->setMat4("view", view);
    satShader->setMat4("projection", projection);
    // No uTime uniform needed for satellite shader
    
    glBindVertexArray(satVAO);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, activeCount);
    glBindVertexArray(0);
}

void SatelliteSystem::drawOrbits(const glm::mat4& view, const glm::mat4& projection) {
    if(orbitVertexCount == 0) return;
    
    orbitShader->use();
    orbitShader->setMat4("view", view);
    orbitShader->setMat4("projection", projection);
    orbitShader->setMat4("model", glm::mat4(1.0f));
    
    glBindVertexArray(orbitVAO);
    
    // Draw each orbit as a line loop
    int pointsPerOrbit = 201; // 200 segments + 1 to close
    for(size_t i = 0; i < satellites.size(); ++i) {
        // Optimization: Could skip inactive orbits here, but orbitVBO is static
        // So we draw all orbit lines even for destroyed satellites (as trails)
        glDrawArrays(GL_LINE_STRIP, i * pointsPerOrbit, pointsPerOrbit);
    }
    
    glBindVertexArray(0);
}
