#include "Earth.h"
#include "../util/TextureLoader.h"
#include <cmath>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

const float PI = 3.14159265359f;

Earth::Earth() : rotation(0.0f) {
    shader = new Shader("shaders/earth.vert", "shaders/earth.frag");
    
    // Load textures
    // Note: Ensure you have replaced these files with your new high-res texture!
    albedoMap = TextureLoader::LoadTexture("assets/textures/earth_albedo.jpg");
    normalMap = TextureLoader::LoadTexture("assets/textures/earth_normal.jpg");

    // Start with a nice angle (Africa/Europe visible)
    rotation.x = 180.0f; 

    generateSphere(1.0f, 64, 64);
}

Earth::~Earth() {
    delete vao;
    delete vbo;
    delete ebo;
    delete shader;
}

void Earth::Rotate(float xoffset, float yoffset) {
    rotation.x += xoffset;
    rotation.y += yoffset;
    
    // Clamp pitch
    if (rotation.y > 89.0f) rotation.y = 89.0f;
    if (rotation.y < -89.0f) rotation.y = -89.0f;
}

void Earth::Update(float deltaTime) {
    // Auto-rotate Earth (Day/Night cycle simulation)
    // Google Earth idle spin is usually slow to the left (Eastward rotation)
    // Earth rotates 360 deg in 24h. We want it faster for demo.
    // 2 degrees per second is a nice slow drift.
    rotation.x += rotationSpeed * deltaTime;
    if (rotation.x > 360.0f) rotation.x -= 360.0f;
}

glm::mat4 Earth::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    // Apply rotation (Yaw and Pitch)
    // Order: Rotate around Y (Spin), then X (Tilt if dragged)
    // Actually, for "Google Earth" feel, you usually want to spin around the POLE (Y axis in local space).
    
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw
    
    // Optional: Add Earth's axial tilt (23.5 deg)
    // model = glm::rotate(model, glm::radians(23.5f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    return model;
}

void Earth::generateSphere(float radius, unsigned int sectorCount, unsigned int stackCount) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for(unsigned int i = 0; i <= stackCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        for(unsigned int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            Vertex v;
            v.position[0] = x;
            v.position[1] = y;
            v.position[2] = z;

            v.normal[0] = nx;
            v.normal[1] = ny;
            v.normal[2] = nz;

            v.texCoords[0] = s; // standard mapping
            v.texCoords[1] = t;
            
            vertices.push_back(v);
        }
    }

    unsigned int k1, k2;
    for(unsigned int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if(i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if(i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    indexCount = indices.size();

    vao = new VAO();
    vbo = new VBO(vertices);
    ebo = new EBO(indices);

    vao->bind();
    ebo->bind();
    vao->linkAttrib(*vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
    vao->linkAttrib(*vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
    vao->linkAttrib(*vbo, 2, 2, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
    vao->unbind();
}

void Earth::Draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos, const glm::vec3& sunDir) {
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    
    glm::mat4 model = getModelMatrix();
    shader->setMat4("model", model);
    
    shader->setVec3("viewPos", cameraPos);
    shader->setVec3("lightDir", sunDir);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedoMap);
    shader->setInt("albedoMap", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    shader->setInt("normalMap", 1);

    vao->bind();
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    vao->unbind();
}
