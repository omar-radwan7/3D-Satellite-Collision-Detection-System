#include "ConjunctionAnalyzer.h"
#include "../OrbitPropagator.h"
#include "../../scene/SatelliteSystem.h"
#include <cmath>
#include <algorithm>
#include <iostream>

ConjunctionAnalyzer::ConjunctionAnalyzer() 
    : minDistanceThreshold(10.0f)
    , riskScoreThreshold(50.0f)
    , predictionSteps(120) // 120 steps over prediction window
{
}

void ConjunctionAnalyzer::analyzeFutureConjunctions(
    const std::vector<Satellite>& satellites,
    float currentTime,
    float predictionWindow)
{
    events.clear();
    criticalEvents.clear();
    
    // Analyze all pairs
    for(size_t i = 0; i < satellites.size(); ++i) {
        if(!satellites[i].active) continue;
        
        for(size_t j = i + 1; j < satellites.size(); ++j) {
            if(!satellites[j].active) continue;
            
            // Find closest approach in prediction window
            auto approach = findClosestApproach(
                satellites[i],
                satellites[j],
                currentTime,
                currentTime + predictionWindow,
                predictionSteps
            );
            
            // Only record if within threshold
            if(approach.distance < minDistanceThreshold) {
                ConjunctionEvent event;
                event.sat1_id = satellites[i].id;
                event.sat2_id = satellites[j].id;
                event.tca_time = approach.time;
                event.tca_position = approach.position;
                event.min_distance = approach.distance;
                
                // Calculate relative velocity
                glm::vec3 relVel = approach.vel1 - approach.vel2;
                event.relative_velocity = glm::length(relVel);
                
                // Estimate collision energy (simplified: v^2 * proxy_mass)
                float proxyMass = 1000.0f; // kg (typical small satellite)
                event.collision_energy = estimateCollisionEnergy(
                    event.relative_velocity,
                    proxyMass,
                    proxyMass
                );
                
                // Calculate risk score
                float altitude = glm::length(event.tca_position);
                event.risk_score = calculateRiskScore(
                    event.min_distance,
                    event.relative_velocity,
                    altitude
                );
                
                event.risk_level = determineRiskLevel(event.min_distance);
                event.is_active = true;
                event.sat1_velocity_at_tca = approach.vel1;
                event.sat2_velocity_at_tca = approach.vel2;
                
                events.push_back(event);
                
                // Cache critical events
                if(event.risk_level >= RiskLevel::HIGH) {
                    criticalEvents.push_back(event);
                }
            }
        }
    }
    
    std::cout << "Conjunction Analysis: Found " << events.size() 
              << " conjunctions (" << criticalEvents.size() << " critical)" << std::endl;
}

ConjunctionAnalyzer::ClosestApproach ConjunctionAnalyzer::findClosestApproach(
    const Satellite& sat1,
    const Satellite& sat2,
    float startTime,
    float endTime,
    int steps)
{
    ClosestApproach result;
    result.distance = std::numeric_limits<float>::max();
    result.time = startTime;
    
    float dt = (endTime - startTime) / steps;
    
    for(int i = 0; i <= steps; ++i) {
        float t = startTime + i * dt;
        
        // Propagate both satellites to time t
        glm::vec3 pos1 = OrbitPropagator::CalculatePosition(sat1, t);
        glm::vec3 pos2 = OrbitPropagator::CalculatePosition(sat2, t);
        glm::vec3 vel1 = OrbitPropagator::CalculateVelocity(sat1, t);
        glm::vec3 vel2 = OrbitPropagator::CalculateVelocity(sat2, t);
        
        float dist = glm::distance(pos1, pos2);
        
        if(dist < result.distance) {
            result.distance = dist;
            result.time = t;
            result.position = (pos1 + pos2) * 0.5f;
            result.vel1 = vel1;
            result.vel2 = vel2;
        }
    }
    
    return result;
}

float ConjunctionAnalyzer::calculateRiskScore(float distance, float relVel, float altitude) {
    // Risk score formula (0-100):
    // - Distance: closer = higher risk
    // - Relative velocity: faster = higher risk
    // - Altitude: lower altitude = higher debris risk
    
    float distanceScore = 100.0f * (1.0f - std::min(distance / 10.0f, 1.0f)); // Max at 0km, 0 at 10km
    float velocityScore = std::min(relVel / 15.0f, 1.0f) * 30.0f; // Up to 30 points for high rel vel
    float altitudeScore = (altitude < 8000.0f) ? 20.0f : 0.0f; // Bonus risk for LEO
    
    return std::min(distanceScore + velocityScore + altitudeScore, 100.0f);
}

RiskLevel ConjunctionAnalyzer::determineRiskLevel(float distance) {
    if(distance < 1.0f) return RiskLevel::CRITICAL;
    if(distance < 2.0f) return RiskLevel::HIGH;
    if(distance < 5.0f) return RiskLevel::MEDIUM;
    if(distance < 10.0f) return RiskLevel::LOW;
    return RiskLevel::SAFE;
}

float ConjunctionAnalyzer::estimateCollisionEnergy(float relVel, float mass1, float mass2) {
    // Kinetic energy: 0.5 * m * v^2
    // Use reduced mass for collision: m1*m2/(m1+m2)
    float reducedMass = (mass1 * mass2) / (mass1 + mass2);
    float relVelMs = relVel * 1000.0f; // Convert km/s to m/s
    return 0.5f * reducedMass * relVelMs * relVelMs; // Joules
}

void ConjunctionAnalyzer::clearOldEvents(float currentTime) {
    // Remove events that have passed
    events.erase(
        std::remove_if(events.begin(), events.end(),
            [currentTime](const ConjunctionEvent& e) {
                return e.tca_time < currentTime - 60.0f; // Keep for 60s after TCA
            }),
        events.end()
    );
    
    criticalEvents.erase(
        std::remove_if(criticalEvents.begin(), criticalEvents.end(),
            [currentTime](const ConjunctionEvent& e) {
                return e.tca_time < currentTime - 60.0f;
            }),
        criticalEvents.end()
    );
}

ConjunctionEvent* ConjunctionAnalyzer::getEventById(int sat1, int sat2) {
    for(auto& event : events) {
        if((event.sat1_id == sat1 && event.sat2_id == sat2) ||
           (event.sat1_id == sat2 && event.sat2_id == sat1)) {
            return &event;
        }
    }
    return nullptr;
}

