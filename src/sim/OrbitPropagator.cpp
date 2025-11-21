#include "OrbitPropagator.h"
#include <cmath>

const float MU = 398600.4418f; // Earth gravitational parameter km^3/s^2

void OrbitPropagator::Propagate(Satellite& sat, float time) {
    sat.position = CalculatePosition(sat, time);
    sat.velocity = CalculateVelocity(sat, time);
}

glm::vec3 OrbitPropagator::CalculatePosition(const Satellite& sat, float time) {
    // Mean Motion
    float a = sat.semiMajorAxis;
    float n = sqrt(MU / (a * a * a));
    
    // Mean Anomaly at time t
    float M = sat.meanAnomaly + n * time;
    
    // Solve Kepler's Equation for Eccentric Anomaly E: M = E - e*sin(E)
    float E = M;
    for(int i=0; i<10; ++i) {
        E = M + sat.eccentricity * sin(E);
    }
    
    // True Anomaly v
    float v = 2 * atan(sqrt((1 + sat.eccentricity)/(1 - sat.eccentricity)) * tan(E/2));
    
    // Distance r
    float r = a * (1 - sat.eccentricity * cos(E));
    
    // Position in orbital plane
    float x_orb = r * cos(v);
    float y_orb = r * sin(v);
    
    // Rotate to ECI
    float O = sat.raan;
    float w = sat.argPeriapsis;
    float i = sat.inclination;
    
    float x = x_orb * (cos(O)*cos(w) - sin(O)*sin(w)*cos(i)) - y_orb * (cos(O)*sin(w) + sin(O)*cos(w)*cos(i));
    float y = x_orb * (sin(O)*cos(w) + cos(O)*sin(w)*cos(i)) - y_orb * (sin(O)*sin(w) - cos(O)*cos(w)*cos(i)); // Z-up?
    // Wait, standard conversion gives Z as axis of rotation for RAAN?
    // Standard ECI: Z is North.
    // x = ...
    // y = ...
    // z = ...
    
    // Let's use standard formulas for Z-up
    // X = r ( cos(O) cos(w+v) - sin(O) sin(w+v) cos(i) )
    // Y = r ( sin(O) cos(w+v) + cos(O) sin(w+v) cos(i) )
    // Z = r ( sin(w+v) sin(i) )
    
    float u = w + v; // Argument of latitude
    float X = r * (cos(O)*cos(u) - sin(O)*sin(u)*cos(i));
    float Y = r * (sin(O)*cos(u) + cos(O)*sin(u)*cos(i)); // This is Y in ECI (equatorial plane)
    float Z = r * (sin(u)*sin(i)); // This is Z (North)
    
    // BUT our OpenGL world is usually Y-up.
    // So we map ECI (X, Y, Z) -> OpenGL (X, Z, -Y)? Or (X, Y, Z) if we rotated Earth?
    // My Earth has poles on local Y.
    // So I should map ECI Z -> OpenGL Y.
    // ECI X -> OpenGL X
    // ECI Y -> OpenGL Z
    
    return glm::vec3(X, Z, Y); // Swap Y and Z to match Y-up world
}

glm::vec3 OrbitPropagator::CalculateVelocity(const Satellite& sat, float time) {
    // Calculate velocity using orbital mechanics
    // v = sqrt(μ * (2/r - 1/a))
    
    float a = sat.semiMajorAxis;
    float n = sqrt(MU / (a * a * a));
    float M = sat.meanAnomaly + n * time;
    
    // Solve for E
    float E = M;
    for(int i=0; i<10; ++i) {
        E = M + sat.eccentricity * sin(E);
    }
    
    float v = 2 * atan(sqrt((1 + sat.eccentricity)/(1 - sat.eccentricity)) * tan(E/2));
    float r = a * (1 - sat.eccentricity * cos(E));
    
    // Velocity magnitude
    float speed = sqrt(MU * (2.0f/r - 1.0f/a));
    
    // Velocity direction in orbital plane (perpendicular to position)
    // In the orbital plane, velocity is tangent to the orbit
    float vx_orb = -sin(v) * speed / sqrt(1.0f); // Simplified
    float vy_orb = (sat.eccentricity + cos(v)) * speed / sqrt(1.0f + 2.0f*sat.eccentricity*cos(v) + sat.eccentricity*sat.eccentricity);
    
    // More accurate: use specific angular momentum
    float h = sqrt(MU * a * (1 - sat.eccentricity * sat.eccentricity));
    vx_orb = -(MU / h) * sin(v);
    vy_orb = (MU / h) * (sat.eccentricity + cos(v));
    
    // Rotate to ECI
    float O = sat.raan;
    float w = sat.argPeriapsis;
    float i = sat.inclination;
    
    float cos_w = cos(w);
    float sin_w = sin(w);
    float cos_O = cos(O);
    float sin_O = sin(O);
    float cos_i = cos(i);
    float sin_i = sin(i);
    
    float vx = vx_orb * (cos_O*cos_w - sin_O*sin_w*cos_i) - vy_orb * (cos_O*sin_w + sin_O*cos_w*cos_i);
    float vy = vx_orb * (sin_O*cos_w + cos_O*sin_w*cos_i) - vy_orb * (sin_O*sin_w - cos_O*cos_w*cos_i);
    float vz = vx_orb * (sin_w*sin_i) + vy_orb * (cos_w*sin_i);
    
    // Map to OpenGL coordinates (ECI Z -> OpenGL Y)
    return glm::vec3(vx, vz, vy);
}
