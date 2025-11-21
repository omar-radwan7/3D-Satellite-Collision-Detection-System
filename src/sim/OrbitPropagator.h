#pragma once
#include "../scene/Satellite.h"
#include <glm/glm.hpp>

class OrbitPropagator {
public:
    // Propagate satellite state to a given time (seconds from epoch)
    // Updates the satellite's position and velocity fields
    static void Propagate(Satellite& sat, float time);
    
    // Calculate position only (useful for orbit line drawing)
    static glm::vec3 CalculatePosition(const Satellite& sat, float time);
};

