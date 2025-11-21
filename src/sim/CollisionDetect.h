#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../scene/SatelliteSystem.h"

struct CollisionEvent {
    int sat1_id;
    int sat2_id;
    float time;
    glm::vec3 collisionPoint;
    glm::vec3 impactPointOnEarth;
    float timeToImpact;
    bool willFallToEarth;
};

struct CollisionPrediction {
    int satelliteId;
    glm::vec3 currentPos;
    std::vector<glm::vec3> trajectoryPoints; // Path from current position to impact
    glm::vec3 impactPoint;
    float timeToImpact;
    bool isActive;
};

class ConjunctionManager {
public:
    void update(const std::vector<Satellite>& satellites, float time);
    const std::vector<CollisionEvent>& getEvents() const { return events; }
    const std::vector<CollisionPrediction>& getPredictions() const { return predictions; }
    
private:
    std::vector<CollisionEvent> events;
    std::vector<CollisionPrediction> predictions;
    
    void predictTrajectory(const Satellite& sat, float currentTime);
    glm::vec3 calculateImpactPoint(const glm::vec3& position, const glm::vec3& velocity);
};
