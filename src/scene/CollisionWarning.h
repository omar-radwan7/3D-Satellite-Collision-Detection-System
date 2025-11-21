#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../render/Shader.h"
#include "../sim/CollisionDetect.h"

class CollisionWarningRenderer {
public:
    CollisionWarningRenderer();
    ~CollisionWarningRenderer();
    
    void update(const std::vector<CollisionPrediction>& predictions, float time);
    void draw(const glm::mat4& view, const glm::mat4& projection);
    
private:
    Shader* warningShader;
    unsigned int trajectoryVAO, trajectoryVBO;
    unsigned int impactVAO, impactVBO;
    
    std::vector<float> trajectoryData;
    std::vector<float> impactMarkers;
    int trajectoryVertexCount;
    int impactVertexCount;
    float animTime;
    
    void generateImpactMarker(const glm::vec3& position, std::vector<float>& data);
};

