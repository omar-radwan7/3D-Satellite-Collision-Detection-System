#include "DebrisSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>

const float EARTH_RADIUS_KM = 6371.0f;
const float RENDER_SCALE = 1.0f / EARTH_RADIUS_KM;

DebrisSystem::DebrisSystem() {
    shader = new Shader("shaders/debris.vert", "shaders/debris.frag");
    initSpikeMesh();
}

DebrisSystem::~DebrisSystem() {
    delete shader;
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void DebrisSystem::initSpikeMesh() {
    // Generate a star/spike shape
    // Simple: 3 crossing lines? Or a set of triangles forming a 3D star.
    // Let's do a set of 6 pyramids (spikes) pointing along axes.
    
    std::vector<float> vertices;
    // Format: Pos(3) + Normal(3)
    
    // Helper to add triangle
    auto addTri = [&](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
        glm::vec3 normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));
        vertices.push_back(p1.x); vertices.push_back(p1.y); vertices.push_back(p1.z);
        vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);
        
        vertices.push_back(p2.x); vertices.push_back(p2.y); vertices.push_back(p2.z);
        vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);
        
        vertices.push_back(p3.x); vertices.push_back(p3.y); vertices.push_back(p3.z);
        vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);
    };

    // Center
    glm::vec3 c(0.0f);
    float l = 0.5f; // Length of spike
    float w = 0.05f; // Width of base

    // Spikes along +/- X, Y, Z
    glm::vec3 axes[] = {
        {1,0,0}, {-1,0,0},
        {0,1,0}, {0,-1,0},
        {0,0,1}, {0,0,-1}
    };

    for(auto& axis : axes) {
        glm::vec3 tip = axis * l;
        // Base quad around axis? 
        // Simplified: Just 2 triangles or a pyramid from base.
        // Let's pick an arbitrary "up" for the base.
        glm::vec3 u = (std::abs(axis.y) > 0.9f) ? glm::vec3(1,0,0) : glm::vec3(0,1,0);
        glm::vec3 v = glm::normalize(glm::cross(axis, u));
        u = glm::normalize(glm::cross(v, axis));
        
        glm::vec3 b1 = u * w + v * w;
        glm::vec3 b2 = u * w - v * w;
        glm::vec3 b3 = -u * w - v * w;
        glm::vec3 b4 = -u * w + v * w;
        
        addTri(tip, b1, b2);
        addTri(tip, b2, b3);
        addTri(tip, b3, b4);
        addTri(tip, b4, b1);
    }

    vertexCount = vertices.size() / 6;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void DebrisSystem::addExplosion(const glm::vec3& position) {
    Explosion ex;
    ex.position = position; // Assumes already in render scale? No, passed in km, converted here?
    // Let's assume callers pass KM and we convert, or callers pass Render Units.
    // Consistent with SatelliteSystem, it keeps data in KM but render converts.
    // But here we store visual explosions. Let's assume `position` is in Render Units for simplicity in Draw.
    ex.position = position * RENDER_SCALE; 
    ex.age = 0.0f;
    ex.lifetime = 2.0f; // 2 seconds
    ex.maxScale = 0.05f; // Size
    explosions.push_back(ex);
}

void DebrisSystem::update(float deltaTime) {
    for(auto it = explosions.begin(); it != explosions.end(); ) {
        it->age += deltaTime;
        if(it->age > it->lifetime) {
            it = explosions.erase(it);
        } else {
            ++it;
        }
    }
}

void DebrisSystem::draw(const glm::mat4& view, const glm::mat4& projection) {
    if(explosions.empty()) return;

    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    glBindVertexArray(vao);
    
    // Could use instancing, but for small number of explosions loop is fine
    for(const auto& ex : explosions) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, ex.position);
        
        // Animate scale: Grow fast, then shrink
        float t = ex.age / ex.lifetime;
        float s = ex.maxScale * std::sin(t * 3.14159f);
        model = glm::scale(model, glm::vec3(s));
        
        // Rotate for effect?
        model = glm::rotate(model, ex.age * 5.0f, glm::vec3(0,1,0));

        shader->setMat4("model", model);
        
        // Pulse color from Yellow to Red to Black
        glm::vec3 color = glm::mix(glm::vec3(1,1,0), glm::vec3(1,0,0), t);
        color = glm::mix(color, glm::vec3(0.2, 0.0, 0.0), t * t);
        shader->setVec3("color", color);
        
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
    
    glBindVertexArray(0);
}

