#include "ConfigLoader.h"
#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

std::vector<Satellite> ConfigLoader::LoadSatellites(const std::string& path) {
    std::vector<Satellite> satellites;
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << path << std::endl;
        return satellites;
    }

    json j;
    try {
        file >> j;
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return satellites;
    }

    if (j.contains("satellites") && j["satellites"].is_array()) {
        for (const auto& item : j["satellites"]) {
            Satellite s;
            s.id = item.value("id", 0);
            s.name = item.value("name", "Unknown");
            s.semiMajorAxis = item.value("semiMajorAxis", 7000.0f);
            s.eccentricity = item.value("eccentricity", 0.0f);
            s.inclination = glm::radians(item.value("inclination", 0.0f));
            s.raan = glm::radians(item.value("raan", 0.0f));
            s.argPeriapsis = glm::radians(item.value("argPeriapsis", 0.0f));
            s.meanAnomaly = glm::radians(item.value("meanAnomaly", 0.0f));
            
            // Default color based on altitude
            float alt = s.semiMajorAxis - 6371.0f;
            if (alt < 2000) s.color = glm::vec3(1.0, 0.2, 0.2);
            else if (alt < 35000) s.color = glm::vec3(0.2, 1.0, 0.2);
            else s.color = glm::vec3(0.2, 0.2, 1.0);

            if(item.contains("color")) {
                auto c = item["color"];
                if(c.is_array() && c.size() == 3) {
                    s.color = glm::vec3(c[0], c[1], c[2]);
                }
            }

            satellites.push_back(s);
        }
    }

    return satellites;
}

