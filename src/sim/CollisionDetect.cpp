#include "CollisionDetect.h"
#include <cmath>
#include <iostream>

void ConjunctionManager::update(const std::vector<Satellite>& satellites, float currentTime) {
    // Naive O(N^2) check. 
    // For N=2000, 4 million checks. Might be slow for 60FPS.
    // Optimization: Spatial partitioning (Grid) or just limit N for demo.
    // We will assume N is small enough or we accept the hit.
    
    // Clear old events that are no longer valid? 
    // Actually, we want to track new conjunctions. 
    // For simplicity, we rebuild the list every frame or update existing ones.
    // Let's rebuild.
    
    events.clear();

    size_t count = satellites.size();
    for(size_t i = 0; i < count; ++i) {
        for(size_t j = i + 1; j < count; ++j) {
            const Satellite& s1 = satellites[i];
            const Satellite& s2 = satellites[j];

            float dx = s1.position.x - s2.position.x;
            float dy = s1.position.y - s2.position.y;
            float dz = s1.position.z - s2.position.z;
            
            // Distance squared
            float distSq = dx*dx + dy*dy + dz*dz;
            
            if(distSq < (warningThreshold * warningThreshold)) {
                float dist = std::sqrt(distSq);
                
                // Calculate relative velocity (approximate if we don't have velocity vectors populated correctly)
                // Sat struct has velocity.
                // But OrbitPropagator might not have updated velocity if we only used simplified position logic.
                // Let's ensure OrbitPropagator updates velocity or we compute it?
                // The current OrbitPropagator::Propagate only updates position. 
                // I should verify this.
                
                ConjunctionEvent event;
                event.sat1_id = s1.id;
                event.sat2_id = s2.id;
                event.time = currentTime;
                event.distance = dist;
                event.active = true;
                
                // Placeholder for relative velocity
                event.relativeVelocity = 0.0f; 
                
                events.push_back(event);
            }
        }
    }
}

