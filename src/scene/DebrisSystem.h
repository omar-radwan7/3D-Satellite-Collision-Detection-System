#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../render/Shader.h"

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;
    float size;
    float life;
    float maxLife;
};

class DebrisSystem {
public:
    DebrisSystem();
    ~DebrisSystem();
    
    // Updated to accept separate velocities for accurate cloud simulation
    void addExplosion(glm::vec3 position, glm::vec3 v1, glm::vec3 v2, glm::vec3 color1, glm::vec3 color2);
    
    void update(float deltaTime);
    void draw(const glm::mat4& view, const glm::mat4& projection);
    
private:
    std::vector<Particle> particles;
    Shader* shader;
    unsigned int VAO, VBO;
};
