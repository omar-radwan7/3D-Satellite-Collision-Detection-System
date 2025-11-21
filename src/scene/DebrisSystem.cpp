#include "DebrisSystem.h"
#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>

DebrisSystem::DebrisSystem() {
    shader = new Shader("shaders/warning.vert", "shaders/warning.frag"); 
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

DebrisSystem::~DebrisSystem() {
    delete shader;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// Helper to generate a burst of debris for one object
void generateBurst(std::vector<Particle>& particles, glm::vec3 pos, glm::vec3 vel, glm::vec3 color, int count) {
    glm::vec3 forward = glm::normalize(vel);
    glm::vec3 up = glm::normalize(pos); // Radial
    glm::vec3 right = glm::cross(forward, up); // Cross-track
    
    // Make sure right vector is valid
    if(glm::length(right) < 0.01f) {
        right = glm::cross(forward, glm::vec3(0,1,0));
    }
    right = glm::normalize(right);
    
    for(int i=0; i<count; ++i) {
        Particle p;
        p.position = pos;
        
        // Scientific Conjunction Spread - COMPACT
        // Mostly In-Track (velocity variation), less Cross-Track/Radial
        float spreadInTrack = ((rand() % 1000) - 500) / 1000.0f; // +/- 0.5 (much tighter)
        float spreadCross = ((rand() % 300) - 150) / 1000.0f;     // +/- 0.15
        float spreadRadial = ((rand() % 300) - 150) / 1000.0f;    // +/- 0.15
        
        // Velocity Delta - SMALLER for realistic collision
        float intensity = 0.05f; // Small explosion magnitude
        glm::vec3 deltaV = (forward * spreadInTrack + right * spreadCross + up * spreadRadial) * intensity;
        
        p.velocity = vel + deltaV; 
        p.color = color;
        p.size = 1.0f; 
        p.life = 12.0f; // 12 seconds (not 7, but visible long enough)
        p.maxLife = 12.0f;
        
        particles.push_back(p);
    }
}

void DebrisSystem::addExplosion(glm::vec3 position, glm::vec3 v1, glm::vec3 v2, glm::vec3 color1, glm::vec3 color2) {
    // Scale inputs to render space
    glm::vec3 renderPos = position * (1.0f / 6371.0f);
    glm::vec3 renderV1 = v1 * (1.0f / 6371.0f); 
    glm::vec3 renderV2 = v2 * (1.0f / 6371.0f); 
    
    std::cout << "EXPLOSION GENERATED at (" << renderPos.x << ", " << renderPos.y << ", " << renderPos.z << ")" << std::endl;
    std::cout << "  V1: (" << renderV1.x << ", " << renderV1.y << ", " << renderV1.z << ")" << std::endl;
    std::cout << "  V2: (" << renderV2.x << ", " << renderV2.y << ", " << renderV2.z << ")" << std::endl;
    
    // Burst 1: Based on Sat 1 Velocity (e.g., Cyan) - FEWER particles
    generateBurst(particles, renderPos, renderV1, color1, 500);
    
    // Burst 2: Based on Sat 2 Velocity (e.g., Orange) - FEWER particles
    generateBurst(particles, renderPos, renderV2, color2, 500);
    
    std::cout << "  Total particles: " << particles.size() << std::endl;
}

void DebrisSystem::update(float deltaTime) {
    // DISABLE gravity - in space, debris continues in straight lines for visualization
    // Gravity makes them fall away too fast and leaves the viewport
    
    for(auto it = particles.begin(); it != particles.end(); ) {
        it->life -= deltaTime;
        if(it->life <= 0) {
            it = particles.erase(it);
        } else {
            // Just linear motion - NO gravity for better visibility
            it->position += it->velocity * deltaTime;
            ++it;
        }
    }
}

void DebrisSystem::draw(const glm::mat4& view, const glm::mat4& projection) {
    if(particles.empty()) return;
    
    // Disable depth test so the vectors are ALWAYS visible (X-ray vision)
    glDisable(GL_DEPTH_TEST);
    
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setMat4("model", glm::mat4(1.0f));
    
    std::vector<float> data;
    data.reserve(particles.size() * 12); 
    
    for(const auto& p : particles) {
        glm::vec3 start = p.position;
        // Shorter vectors for compact visualization
        float vectorScale = 3.0f; // Reduced for smaller explosion
        glm::vec3 end = p.position + p.velocity * vectorScale; 
        
        // Fade based on remaining life
        float alpha = p.life / p.maxLife;
        
        // Start (bright)
        data.push_back(start.x); data.push_back(start.y); data.push_back(start.z);
        data.push_back(p.color.r); data.push_back(p.color.g); data.push_back(p.color.b);
        
        // End (same brightness for solid look)
        data.push_back(end.x); data.push_back(end.y); data.push_back(end.z);
        data.push_back(p.color.r); data.push_back(p.color.g); data.push_back(p.color.b);
    }
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    // Medium thickness lines
    glLineWidth(2.0f); 
    glDrawArrays(GL_LINES, 0, particles.size() * 2);
    
    glBindVertexArray(0);
    
    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);
}
