#include "CollisionDetect.h"
#include "../sim/OrbitPropagator.h"
#include <cmath>
#include <algorithm>
#include <iostream>

void ConjunctionManager::update(const std::vector<Satellite>& satellites, float time) {
    events.clear();
    predictions.clear();
    
    float threshold = 50.0f; // 50 km collision detection zone (increased for testing)
    
    for(size_t i=0; i<satellites.size(); ++i) {
        if(!satellites[i].active) continue; // Skip destroyed satellites

        for(size_t j=i+1; j<satellites.size(); ++j) {
            if(!satellites[j].active) continue; // Skip destroyed satellites

            float dist = glm::distance(satellites[i].position, satellites[j].position);
            if(dist < threshold) {
                // Debug output
                std::cout << "COLLISION DETECTED: Sat " << satellites[i].id << " <-> Sat " << satellites[j].id 
                          << " | Distance: " << dist << " km" << std::endl;
                
                CollisionEvent ev;
                ev.sat1_id = satellites[i].id;
                ev.sat2_id = satellites[j].id;
                ev.time = time;
                ev.collisionPoint = (satellites[i].position + satellites[j].position) * 0.5f;
                
                // Calculate if satellite will fall to Earth
                glm::vec3 collisionVelocity = (satellites[i].velocity + satellites[j].velocity) * 0.5f;
                float altitude = glm::length(ev.collisionPoint);
                float earthRadius = 6371.0f;
                
                // Check if debris orbit will decay (simplified: velocity too low for altitude)
                float orbitalSpeed = glm::length(collisionVelocity);
                float requiredSpeed = sqrt(398600.4418f / altitude); // Circular orbit speed
                
                ev.willFallToEarth = (orbitalSpeed < requiredSpeed * 0.9f); // If speed < 90% of orbital speed (more lenient)
                
                // ALWAYS show collision warnings (for visualization)
                // Calculate impact point on Earth
                ev.impactPointOnEarth = calculateImpactPoint(ev.collisionPoint, collisionVelocity);
                ev.timeToImpact = (altitude - earthRadius) / std::max(orbitalSpeed * 0.1f, 1.0f); // Time estimate
                
                // Create prediction visualization for both satellites
                CollisionPrediction pred1, pred2;
                pred1.satelliteId = satellites[i].id;
                pred1.currentPos = satellites[i].position;
                pred1.impactPoint = ev.impactPointOnEarth;
                pred1.timeToImpact = ev.timeToImpact;
                pred1.isActive = true;
                
                pred2.satelliteId = satellites[j].id;
                pred2.currentPos = satellites[j].position;
                pred2.impactPoint = ev.impactPointOnEarth;
                pred2.timeToImpact = ev.timeToImpact;
                pred2.isActive = true;
                
                // Generate trajectory points (collision point -> impact point)
                int steps = 30;
                for(int k = 0; k <= steps; ++k) {
                    float t = (float)k / steps;
                    glm::vec3 point = glm::mix(ev.collisionPoint, ev.impactPointOnEarth, t);
                    pred1.trajectoryPoints.push_back(point);
                    pred2.trajectoryPoints.push_back(point);
                }
                
                predictions.push_back(pred1);
                predictions.push_back(pred2);
                
                std::cout << "  -> Will fall to Earth: " << (ev.willFallToEarth ? "YES" : "NO") 
                          << " | Impact point: (" << ev.impactPointOnEarth.x << ", " 
                          << ev.impactPointOnEarth.y << ", " << ev.impactPointOnEarth.z << ")" << std::endl;
                
                events.push_back(ev);
            }
        }
    }
}

glm::vec3 ConjunctionManager::calculateImpactPoint(const glm::vec3& position, const glm::vec3& velocity) {
    // Simple ballistic trajectory calculation
    // Project position towards Earth along velocity direction
    glm::vec3 dir = glm::normalize(position + velocity * 100.0f); // Future position direction
    float earthRadius = 6371.0f;
    
    // Find where this direction intersects Earth surface
    return dir * earthRadius;
}

void ConjunctionManager::predictTrajectory(const Satellite& sat, float currentTime) {
    // This can be expanded for more sophisticated prediction
    // For now, we do prediction in the update() method
}
