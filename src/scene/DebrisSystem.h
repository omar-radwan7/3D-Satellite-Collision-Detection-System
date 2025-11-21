#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../render/Shader.h"
#include "../render/Buffers.h"

struct Explosion {
    glm::vec3 position;
    float age;
    float lifetime;
    float maxScale;
};

class DebrisSystem {
public:
    DebrisSystem();
    ~DebrisSystem();
    void update(float deltaTime);
    void addExplosion(const glm::vec3& position);
    void draw(const glm::mat4& view, const glm::mat4& projection);

private:
    std::vector<Explosion> explosions;
    Shader* shader;
    unsigned int vao, vbo;
    int vertexCount;
    
    void initSpikeMesh();
};

