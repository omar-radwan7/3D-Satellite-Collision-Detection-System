#pragma once
#include <string>
#include <glm/glm.hpp>

struct Satellite {
    std::string name;
    int id;

    // Keplerian Elements (Units: km, radians)
    float semiMajorAxis; // a (km)
    float eccentricity;  // e
    float inclination;   // i (radians)
    float raan;          // Omega (Long. of Ascending Node) (radians)
    float argPeriapsis;  // omega (Argument of Periapsis) (radians)
    float meanAnomaly;   // M0 (radians) at epoch
    float meanMotion;    // n (radians/sec) - if not provided, computed from a

    // State
    glm::vec3 position;  // ECI position (km)
    glm::vec3 velocity;  // ECI velocity (km/s)
    
    // Visualization
    glm::vec3 color;
};

