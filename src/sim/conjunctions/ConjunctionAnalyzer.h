#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>

// Forward declaration
struct Satellite;

enum class RiskLevel {
    SAFE = 0,      // > 10 km
    LOW = 1,       // 5-10 km
    MEDIUM = 2,    // 2-5 km
    HIGH = 3,      // 1-2 km
    CRITICAL = 4   // < 1 km
};

struct ConjunctionEvent {
    int sat1_id;
    int sat2_id;
    float tca_time;              // Time of Closest Approach (seconds from now)
    glm::vec3 tca_position;      // Position at TCA
    float min_distance;          // km
    float relative_velocity;     // km/s
    float collision_energy;      // Joules (proxy: v_rel^2 * proxy_mass)
    float risk_score;            // 0-100
    RiskLevel risk_level;
    bool is_active;              // Still relevant
    
    // For visualization
    glm::vec3 sat1_velocity_at_tca;
    glm::vec3 sat2_velocity_at_tca;
};

class ConjunctionAnalyzer {
public:
    ConjunctionAnalyzer();
    
    // Main analysis function
    void analyzeFutureConjunctions(
        const std::vector<Satellite>& satellites,
        float currentTime,
        float predictionWindow = 3600.0f  // 1 hour default
    );
    
    // Getters
    const std::vector<ConjunctionEvent>& getEvents() const { return events; }
    const std::vector<ConjunctionEvent>& getCriticalEvents() const { return criticalEvents; }
    
    // Configuration
    void setMinDistanceThreshold(float km) { minDistanceThreshold = km; }
    void setRiskScoreThreshold(float score) { riskScoreThreshold = score; }
    void setPredictionSteps(int steps) { predictionSteps = steps; }
    
    // Event management
    void clearOldEvents(float currentTime);
    ConjunctionEvent* getEventById(int sat1, int sat2);
    
private:
    std::vector<ConjunctionEvent> events;
    std::vector<ConjunctionEvent> criticalEvents; // Cache for high-risk events
    
    // Thresholds
    float minDistanceThreshold;  // km
    float riskScoreThreshold;    // 0-100
    int predictionSteps;         // Number of future time steps to check
    
    // Helper functions
    float calculateRiskScore(float distance, float relVel, float altitude);
    RiskLevel determineRiskLevel(float distance);
    float estimateCollisionEnergy(float relVel, float sat1Mass, float sat2Mass);
    
    // Find closest approach between two orbital paths
    struct ClosestApproach {
        float time;
        float distance;
        glm::vec3 position;
        glm::vec3 vel1;
        glm::vec3 vel2;
    };
    ClosestApproach findClosestApproach(
        const Satellite& sat1,
        const Satellite& sat2,
        float startTime,
        float endTime,
        int steps
    );
};

