#include "Earth.h"
#include "../util/TextureLoader.h"
#include <cmath>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

const float PI = 3.14159265359f;

Earth::Earth() {
    shader = new Shader("shaders/earth.vert", "shaders/earth.frag");
    
    // Load textures
    albedoMap = TextureLoader::LoadTexture("assets/textures/earth_albedo.jpg");
    normalMap = TextureLoader::LoadTexture("assets/textures/earth_normal.jpg");

    // Initialize with 0 rotation (facing Prime Meridian if texture is standard)
    orientation = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    generateSphere(1.0f, 64, 64);
}

Earth::~Earth() {
    delete vao;
    delete vbo;
    delete ebo;
    delete shader;
}

void Earth::Rotate(float xoffset, float yoffset, const glm::vec3& camRight, const glm::vec3& camUp) {
    float sensitivity = 0.5f; 
    float angleX = glm::radians(xoffset * sensitivity);
    float angleY = glm::radians(yoffset * sensitivity);

    // Dragging X (left/right) rotates around Camera Up
    // Dragging Y (up/down) rotates around Camera Right
    // We invert X dragging to make it feel like pulling the surface
    glm::quat qY = glm::angleAxis(-angleX, camUp);
    glm::quat qX = glm::angleAxis(angleY, camRight);

    orientation = qY * qX * orientation;
    orientation = glm::normalize(orientation);
}

void Earth::Update(float deltaTime) {
    // Auto-rotate around Earth's LOCAL Y axis (poles)
    glm::quat spin = glm::angleAxis(glm::radians(rotationSpeed * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
    orientation = orientation * spin;
    orientation = glm::normalize(orientation);
}

glm::mat4 Earth::getModelMatrix() const {
    return glm::mat4_cast(orientation);
}

void Earth::generateSphere(float radius, unsigned int sectorCount, unsigned int stackCount) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float x, y, z, xy;                              
    float nx, ny, nz, lengthInv = 1.0f / radius;    
    float s, t;                                     

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for(unsigned int i = 0; i <= stackCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             
        y = radius * sinf(stackAngle);              // Y is UP

        for(unsigned int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // 0 to 2pi

            x = xy * cosf(sectorAngle);             
            z = xy * sinf(sectorAngle);             
            
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            s = 1.0f - (float)j / sectorCount;
            t = 1.0f - (float)i / stackCount; // V=1 at top (North), V=0 at bottom

            Vertex v;
            v.position[0] = x;
            v.position[1] = y;
            v.position[2] = z;

            v.normal[0] = nx;
            v.normal[1] = ny;
            v.normal[2] = nz;

            v.texCoords[0] = s;
            v.texCoords[1] = t;
            
            vertices.push_back(v);
        }
    }

    unsigned int k1, k2;
    for(unsigned int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     
        k2 = k1 + sectorCount + 1;      

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
