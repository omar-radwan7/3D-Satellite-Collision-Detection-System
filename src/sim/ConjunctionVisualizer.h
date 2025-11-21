#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../render/Shader.h"
#include "conjunctions/ConjunctionAnalyzer.h"

class ConjunctionVisualizer {
public:
    ConjunctionVisualizer();
    ~ConjunctionVisualizer();
    
    void update(const std::vector<ConjunctionEvent>& events, float currentTime);
    void draw(const glm::mat4& view, const glm::mat4& projection);
    
    void setShowDangerCorridors(bool show) { showCorridors = show; }
    void setShowTCAMarkers(bool show) { showTCAMarkers = show; }
    void setShowHypotheticalDebris(bool show) { showHypothetical = show; }
    
private:
    Shader* markerShader;
    Shader* corridorShader;
    
    unsigned int tcaVAO, tcaVBO;
    unsigned int corridorVAO, corridorVBO;
    
    std::vector<float> tcaMarkerData;
    std::vector<float> corridorData;
    
    int tcaVertexCount;
    int corridorVertexCount;
    
    bool showCorridors;
    bool showTCAMarkers;
    bool showHypothetical;
    
    float animTime;
    
    // Helper functions
    void generateTCAMarker(const ConjunctionEvent& event, std::vector<float>& data);
    void generateDangerCorridor(const ConjunctionEvent& event, std::vector<float>& data);
    glm::vec3 getRiskColor(RiskLevel level);
};

