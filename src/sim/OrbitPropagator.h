#pragma once
#include "../scene/SatelliteSystem.h"

class OrbitPropagator {
public:
    static void Propagate(Satellite& sat, float time);
    static glm::vec3 CalculatePosition(const Satellite& sat, float time);
    static glm::vec3 CalculateVelocity(const Satellite& sat, float time);
};
