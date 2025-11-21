#pragma once
#include "../render/Buffers.h"
#include "../render/Shader.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Earth {
public:
    Earth();
    ~Earth();
    void Draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos, const glm::vec3& sunDir);
    
    // Rotation interaction
    void Rotate(float xoffset, float yoffset, const glm::vec3& camRight, const glm::vec3& camUp);
    void Update(float deltaTime); // Auto-rotation
    
    // Model transform
    glm::mat4 getModelMatrix() const;

private:
    VAO* vao;
    VBO* vbo;
    EBO* ebo;
    Shader* shader;
    unsigned int albedoMap;
    unsigned int normalMap;
    unsigned int indexCount;
    
    glm::quat orientation; 
    float rotationSpeed = 2.0f; // Degrees per second (Auto-rotation)

    void generateSphere(float radius, unsigned int sectorCount, unsigned int stackCount);
};
