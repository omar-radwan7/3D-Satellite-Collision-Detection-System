#include "ConjunctionVisualizer.h"
#include <GL/glew.h>
#include <cmath>
#include <iostream>

ConjunctionVisualizer::ConjunctionVisualizer()
    : showCorridors(true)
    , showTCAMarkers(true)
    , showHypothetical(false)
    , animTime(0.0f)
    , tcaVertexCount(0)
    , corridorVertexCount(0)
{
    markerShader = new Shader("shaders/warning.vert", "shaders/warning.frag");
    corridorShader = new Shader("shaders/warning.vert", "shaders/warning.frag");
    
    glGenVertexArrays(1, &tcaVAO);
    glGenBuffers(1, &tcaVBO);
    glGenVertexArrays(1, &corridorVAO);
    glGenBuffers(1, &corridorVBO);
}

ConjunctionVisualizer::~ConjunctionVisualizer() {
    delete markerShader;
    delete corridorShader;
    glDeleteVertexArrays(1, &tcaVAO);
    glDeleteBuffers(1, &tcaVBO);
    glDeleteVertexArrays(1, &corridorVAO);
    glDeleteBuffers(1, &corridorVBO);
}

glm::vec3 ConjunctionVisualizer::getRiskColor(RiskLevel level) {
    switch(level) {
        case RiskLevel::CRITICAL: return glm::vec3(1.0f, 0.0f, 0.0f); // Red
        case RiskLevel::HIGH:     return glm::vec3(1.0f, 0.5f, 0.0f); // Orange
        case RiskLevel::MEDIUM:   return glm::vec3(1.0f, 1.0f, 0.0f); // Yellow
        case RiskLevel::LOW:      return glm::vec3(0.5f, 1.0f, 0.5f); // Light Green
        default:                  return glm::vec3(0.0f, 1.0f, 0.0f); // Green
    }
}

void ConjunctionVisualizer::update(const std::vector<ConjunctionEvent>& events, float currentTime) {
    animTime = currentTime;
    tcaMarkerData.clear();
    corridorData.clear();
    
    for(const auto& event : events) {
        if(!event.is_active) continue;
        
        if(showTCAMarkers) {
            generateTCAMarker(event, tcaMarkerData);
        }
        
        if(showCorridors) {
            generateDangerCorridor(event, corridorData);
        }
    }
    
    // Upload TCA marker data
    tcaVertexCount = tcaMarkerData.size() / 6;
    if(tcaVertexCount > 0) {
        glBindVertexArray(tcaVAO);
        glBindBuffer(GL_ARRAY_BUFFER, tcaVBO);
        glBufferData(GL_ARRAY_BUFFER, tcaMarkerData.size() * sizeof(float), tcaMarkerData.data(), GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        
        glBindVertexArray(0);
    }
    
    // Upload corridor data
    corridorVertexCount = corridorData.size() / 6;
    if(corridorVertexCount > 0) {
        glBindVertexArray(corridorVAO);
        glBindBuffer(GL_ARRAY_BUFFER, corridorVBO);
        glBufferData(GL_ARRAY_BUFFER, corridorData.size() * sizeof(float), corridorData.data(), GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        
        glBindVertexArray(0);
    }
}

void ConjunctionVisualizer::generateTCAMarker(const ConjunctionEvent& event, std::vector<float>& data) {
    glm::vec3 pos = event.tca_position * (1.0f / 6371.0f); // Scale to render space
    glm::vec3 color = getRiskColor(event.risk_level);
    
    // Pulsing effect
    float pulse = 0.7f + 0.3f * sin(animTime * 5.0f);
    float size = 0.03f * pulse * (1.0f + event.risk_score / 100.0f); // Larger for higher risk
    
    // Create octahedron marker (8 triangular faces)
    glm::vec3 vertices[6] = {
        pos + glm::vec3(size, 0, 0),
        pos + glm::vec3(-size, 0, 0),
        pos + glm::vec3(0, size, 0),
        pos + glm::vec3(0, -size, 0),
        pos + glm::vec3(0, 0, size),
        pos + glm::vec3(0, 0, -size)
    };
    
    // Draw as lines forming the octahedron edges
    int edges[][2] = {
        {0,2}, {0,3}, {0,4}, {0,5},
        {1,2}, {1,3}, {1,4}, {1,5},
        {2,4}, {4,3}, {3,5}, {5,2}
    };
    
    for(auto edge : edges) {
        data.push_back(vertices[edge[0]].x);
        data.push_back(vertices[edge[0]].y);
        data.push_back(vertices[edge[0]].z);
        data.push_back(color.r * pulse);
        data.push_back(color.g * pulse);
        data.push_back(color.b * pulse);
        
        data.push_back(vertices[edge[1]].x);
        data.push_back(vertices[edge[1]].y);
        data.push_back(vertices[edge[1]].z);
        data.push_back(color.r * pulse);
        data.push_back(color.g * pulse);
        data.push_back(color.b * pulse);
    }
}

void ConjunctionVisualizer::generateDangerCorridor(const ConjunctionEvent& event, std::vector<float>& data) {
    // Draw a tube segment around the TCA position along the relative velocity vector
    glm::vec3 pos = event.tca_position * (1.0f / 6371.0f);
    glm::vec3 relVel = (event.sat1_velocity_at_tca - event.sat2_velocity_at_tca);
    
    if(glm::length(relVel) < 0.001f) return; // Skip if no relative motion
    
    glm::vec3 dir = glm::normalize(relVel * (1.0f / 6371.0f));
    glm::vec3 color = getRiskColor(event.risk_level);
    
    // Corridor length based on risk
    float corridorLen = 0.1f * (1.0f + event.risk_score / 100.0f);
    glm::vec3 start = pos - dir * corridorLen * 0.5f;
    glm::vec3 end = pos + dir * corridorLen * 0.5f;
    
    // Tube radius based on miss distance (larger miss = wider warning)
    float tubeRadius = std::max(0.005f, event.min_distance * 0.0001f);
    
    // Create tube with 8 segments
    int segments = 8;
    glm::vec3 perpVec = glm::cross(dir, glm::vec3(0,1,0));
    if(glm::length(perpVec) < 0.01f) {
        perpVec = glm::cross(dir, glm::vec3(1,0,0));
    }
    perpVec = glm::normalize(perpVec);
    glm::vec3 perpVec2 = glm::cross(dir, perpVec);
    
    for(int i = 0; i < segments; ++i) {
        float angle = i * 2.0f * 3.14159f / segments;
        float nextAngle = (i + 1) * 2.0f * 3.14159f / segments;
        
        glm::vec3 offset1 = (perpVec * cos(angle) + perpVec2 * sin(angle)) * tubeRadius;
        glm::vec3 offset2 = (perpVec * cos(nextAngle) + perpVec2 * sin(nextAngle)) * tubeRadius;
        
        // Line from start to end at this radial position
        data.push_back((start + offset1).x);
        data.push_back((start + offset1).y);
        data.push_back((start + offset1).z);
        data.push_back(color.r * 0.3f);
        data.push_back(color.g * 0.3f);
        data.push_back(color.b * 0.3f);
        
        data.push_back((end + offset1).x);
        data.push_back((end + offset1).y);
        data.push_back((end + offset1).z);
        data.push_back(color.r * 0.3f);
        data.push_back(color.g * 0.3f);
        data.push_back(color.b * 0.3f);
    }
}

void ConjunctionVisualizer::draw(const glm::mat4& view, const glm::mat4& projection) {
    // Disable depth test for X-ray vision
    glDisable(GL_DEPTH_TEST);
    
    if(showTCAMarkers && tcaVertexCount > 0) {
        markerShader->use();
        markerShader->setMat4("view", view);
        markerShader->setMat4("projection", projection);
        markerShader->setMat4("model", glm::mat4(1.0f));
        
        glBindVertexArray(tcaVAO);
        glLineWidth(2.5f);
        glDrawArrays(GL_LINES, 0, tcaVertexCount);
        glBindVertexArray(0);
    }
    
    if(showCorridors && corridorVertexCount > 0) {
        corridorShader->use();
        corridorShader->setMat4("view", view);
        corridorShader->setMat4("projection", projection);
        corridorShader->setMat4("model", glm::mat4(1.0f));
        
        glBindVertexArray(corridorVAO);
        glLineWidth(1.5f);
        glDrawArrays(GL_LINES, 0, corridorVertexCount);
        glBindVertexArray(0);
    }
    
    glEnable(GL_DEPTH_TEST);
}

