#include "CollisionWarning.h"
#include <GL/glew.h>
#include <cmath>
#include <iostream>

CollisionWarningRenderer::CollisionWarningRenderer() {
    warningShader = new Shader("shaders/warning.vert", "shaders/warning.frag");
    animTime = 0.0f;
    
    glGenVertexArrays(1, &trajectoryVAO);
    glGenBuffers(1, &trajectoryVBO);
    
    glGenVertexArrays(1, &impactVAO);
    glGenBuffers(1, &impactVBO);
    
    trajectoryVertexCount = 0;
    impactVertexCount = 0;
}

CollisionWarningRenderer::~CollisionWarningRenderer() {
    delete warningShader;
    glDeleteVertexArrays(1, &trajectoryVAO);
    glDeleteBuffers(1, &trajectoryVBO);
    glDeleteVertexArrays(1, &impactVAO);
    glDeleteBuffers(1, &impactVBO);
}

void CollisionWarningRenderer::update(const std::vector<CollisionPrediction>& predictions, float time) {
    animTime = time;
    trajectoryData.clear();
    impactMarkers.clear();
    
    if(predictions.empty()) {
        trajectoryVertexCount = 0;
        impactVertexCount = 0;
        return;
    }
    
    std::cout << "CollisionWarning: Updating " << predictions.size() << " predictions" << std::endl;
    
    for(const auto& pred : predictions) {
        if(!pred.isActive) continue;
        
        std::cout << "  -> Sat " << pred.satelliteId << " trajectory with " 
                  << pred.trajectoryPoints.size() << " points" << std::endl;
        
        // Generate trajectory line (BRIGHT red line from collision to impact)
        for(size_t i = 0; i < pred.trajectoryPoints.size(); ++i) {
            glm::vec3 pos = pred.trajectoryPoints[i] * (1.0f / 6371.0f); // Scale to render space
            
            // Animated pulse effect - VERY BRIGHT
            float pulseIntensity = 0.8f + 0.2f * sin(time * 3.0f - i * 0.1f);
            float red = 1.0f;
            float green = 0.1f; // Slight orange tint for visibility
            float blue = 0.0f;
            
            trajectoryData.push_back(pos.x);
            trajectoryData.push_back(pos.y);
            trajectoryData.push_back(pos.z);
            trajectoryData.push_back(red * pulseIntensity);
            trajectoryData.push_back(green * pulseIntensity);
            trajectoryData.push_back(blue);
        }
        
        // Generate impact marker (pulsing circle on Earth surface)
        generateImpactMarker(pred.impactPoint, impactMarkers);
    }
    
    // Upload trajectory data
    trajectoryVertexCount = trajectoryData.size() / 6;
    if(trajectoryVertexCount > 0) {
        glBindVertexArray(trajectoryVAO);
        glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
        glBufferData(GL_ARRAY_BUFFER, trajectoryData.size() * sizeof(float), trajectoryData.data(), GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        
        glBindVertexArray(0);
    }
    
    // Upload impact marker data
    impactVertexCount = impactMarkers.size() / 6;
    if(impactVertexCount > 0) {
        glBindVertexArray(impactVAO);
        glBindBuffer(GL_ARRAY_BUFFER, impactVBO);
        glBufferData(GL_ARRAY_BUFFER, impactMarkers.size() * sizeof(float), impactMarkers.data(), GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        
        glBindVertexArray(0);
    }
}

void CollisionWarningRenderer::generateImpactMarker(const glm::vec3& position, std::vector<float>& data) {
    // Create pulsing circle/cross marker at impact point
    glm::vec3 renderPos = position * (1.0f / 6371.0f);
    
    // Pulsing effect
    float pulse = 0.7f + 0.3f * sin(animTime * 4.0f);
    float size = 0.05f * pulse;
    
    // Create a circle with cross
    int segments = 16;
    for(int i = 0; i <= segments; ++i) {
        float angle = i * 2.0f * 3.14159f / segments;
        glm::vec3 offset = glm::vec3(cos(angle), sin(angle), 0.0f) * size;
        
        // Rotate offset to align with surface normal
        glm::vec3 normal = glm::normalize(position);
        glm::vec3 tangent = glm::normalize(glm::cross(normal, glm::vec3(0.0f, 1.0f, 0.0f)));
        if(glm::length(tangent) < 0.1f) {
            tangent = glm::normalize(glm::cross(normal, glm::vec3(1.0f, 0.0f, 0.0f)));
        }
        glm::vec3 bitangent = glm::cross(normal, tangent);
        
        glm::vec3 point = renderPos + tangent * offset.x + bitangent * offset.y + normal * 0.002f;
        
        data.push_back(point.x);
        data.push_back(point.y);
        data.push_back(point.z);
        data.push_back(1.0f); // Red
        data.push_back(0.3f * pulse); // Orange tint
        data.push_back(0.0f);
    }
    
    // Add cross lines
    glm::vec3 normal = glm::normalize(position);
    glm::vec3 tangent = glm::normalize(glm::cross(normal, glm::vec3(0.0f, 1.0f, 0.0f)));
    if(glm::length(tangent) < 0.1f) {
        tangent = glm::normalize(glm::cross(normal, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    glm::vec3 bitangent = glm::cross(normal, tangent);
    
    // Horizontal line
    data.push_back((renderPos - tangent * size).x);
    data.push_back((renderPos - tangent * size).y);
    data.push_back((renderPos - tangent * size).z);
    data.push_back(1.0f); data.push_back(0.5f * pulse); data.push_back(0.0f);
    
    data.push_back((renderPos + tangent * size).x);
    data.push_back((renderPos + tangent * size).y);
    data.push_back((renderPos + tangent * size).z);
    data.push_back(1.0f); data.push_back(0.5f * pulse); data.push_back(0.0f);
    
    // Vertical line
    data.push_back((renderPos - bitangent * size).x);
    data.push_back((renderPos - bitangent * size).y);
    data.push_back((renderPos - bitangent * size).z);
    data.push_back(1.0f); data.push_back(0.5f * pulse); data.push_back(0.0f);
    
    data.push_back((renderPos + bitangent * size).x);
    data.push_back((renderPos + bitangent * size).y);
    data.push_back((renderPos + bitangent * size).z);
    data.push_back(1.0f); data.push_back(0.5f * pulse); data.push_back(0.0f);
}

void CollisionWarningRenderer::draw(const glm::mat4& view, const glm::mat4& projection) {
    if(trajectoryVertexCount == 0 && impactVertexCount == 0) return;
    
    // DISABLE DEPTH TEST to draw ON TOP of everything (X-Ray vision)
    GLboolean depthEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);
    glDisable(GL_DEPTH_TEST);
    
    warningShader->use();
    warningShader->setMat4("view", view);
    warningShader->setMat4("projection", projection);
    warningShader->setMat4("model", glm::mat4(1.0f));
    
    // Draw trajectory lines - THICK and BRIGHT
    if(trajectoryVertexCount > 0) {
        // Debug first vertex to check coordinates
        if(trajectoryData.size() >= 3) {
            std::cout << "Draw Warning Line: Start Pos (" 
                      << trajectoryData[0] << ", " << trajectoryData[1] << ", " << trajectoryData[2] << ")" << std::endl;
        }

        glBindVertexArray(trajectoryVAO);
        glLineWidth(5.0f); 
        glDrawArrays(GL_LINE_STRIP, 0, trajectoryVertexCount);
        glBindVertexArray(0);
    }
    
    // Draw impact markers
    if(impactVertexCount > 0) {
        glBindVertexArray(impactVAO);
        glPointSize(15.0f); // Huge points
        glDrawArrays(GL_LINE_LOOP, 0, 17); // Circle
        glDrawArrays(GL_LINES, 17, 4); // Cross
        glBindVertexArray(0);
    }
    
    // Restore depth test
    if(depthEnabled) glEnable(GL_DEPTH_TEST);
}

