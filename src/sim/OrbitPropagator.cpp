#include "OrbitPropagator.h"
#include <cmath>

const float MU = 398600.4418f; // Earth's gravitational parameter (km^3/s^2)
const float PI = 3.14159265359f;
const float EPSILON = 1e-6f;

void OrbitPropagator::Propagate(Satellite& sat, float time) {
    // 1. Calculate Mean Anomaly at time t
    // n = sqrt(mu / a^3)
    float n = std::sqrt(MU / std::pow(sat.semiMajorAxis, 3.0f));
    float M = sat.meanAnomaly + n * time;

    // 2. Solve Kepler's Equation for Eccentric Anomaly E: M = E - e*sin(E)
    float E = M; // Initial guess
    for (int i = 0; i < 10; ++i) {
        float f = E - sat.eccentricity * std::sin(E) - M;
        float df = 1.0f - sat.eccentricity * std::cos(E);
        float delta = f / df;
        E -= delta;
        if (std::abs(delta) < EPSILON) break;
    }

    // 3. Calculate True Anomaly nu
    // tan(nu/2) = sqrt((1+e)/(1-e)) * tan(E/2)
    float sqrtTerm = std::sqrt((1.0f + sat.eccentricity) / (1.0f - sat.eccentricity));
    float tanNu2 = sqrtTerm * std::tan(E / 2.0f);
    float nu = 2.0f * std::atan(tanNu2);

    // 4. Calculate radius distance r
    float r = sat.semiMajorAxis * (1.0f - sat.eccentricity * std::cos(E));

    // 5. Position in orbital plane (PQW frame, but simplified)
    // P points to periapsis
    // Q is in orbital plane, 90 deg from P in direction of motion
    float x_orb = r * std::cos(nu);
    float y_orb = r * std::sin(nu);

    // 6. Rotate to ECI frame
    // We need to rotate by -omega (argPeriapsis), -i (inclination), -Omega (RAAN) in reverse order of intrinsic rotations?
    // Standard conversion:
    // X = r * (cos(Omega) * cos(omega+nu) - sin(Omega) * sin(omega+nu) * cos(i))
    // Y = r * (sin(Omega) * cos(omega+nu) + cos(Omega) * sin(omega+nu) * cos(i))
    // Z = r * (sin(omega+nu) * sin(i))
    
    float u = sat.argPeriapsis + nu; // Argument of latitude
    
    float cosU = std::cos(u);
    float sinU = std::sin(u);
    float cosOm = std::cos(sat.raan);
    float sinOm = std::sin(sat.raan);
    float cosI = std::cos(sat.inclination);
    float sinI = std::sin(sat.inclination);

    sat.position.x = r * (cosOm * cosU - sinOm * sinU * cosI);
    sat.position.y = r * (sinU * sinI); // Note: Y is up in our OpenGL, but Z is North in ECI usually.
    sat.position.z = r * (sinOm * cosU + cosOm * sinU * cosI); 
    
    // WAIT!
    // OpenGL: Y is Up.
    // ECI: Z is North (Up). X is Vernal Equinox. Y is 90 deg East.
    // So Mapping: ECI(X, Y, Z) -> OpenGL(x, z, -y)? Or just rotate the world.
    // Let's stick to ECI coordinates in `sat.position` (Z up)
    // And when rendering, we rotate or map them.
    // OpenGL Camera usually looks -Z.
    // Let's map ECI Z -> GL Y. ECI X -> GL X. ECI Y -> GL Z.
    // So (x, z, y) -> (x, y, z) swap.
    //
    // Let's recompute standard ECI (Z up)
    float X_eci = r * (cosOm * cosU - sinOm * sinU * cosI);
    float Y_eci = r * (sinOm * cosU + cosOm * sinU * cosI);
    float Z_eci = r * (sinU * sinI);

    // Map to GL (Y up)
    sat.position.x = X_eci;
    sat.position.y = Z_eci;
    sat.position.z = Y_eci; // Y_eci is usually "Right/East", in GL Z is "Backward".
                            // Let's assume Earth texture is aligned such that poles are Y.
}

glm::vec3 OrbitPropagator::CalculatePosition(const Satellite& sat, float time) {
    Satellite temp = sat;
    Propagate(temp, time);
    return temp.position;
}

