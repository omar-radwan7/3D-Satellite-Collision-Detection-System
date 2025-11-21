#include "ConfigLoader.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {
std::string ResolvePath(const std::string& requestedPath) {
    static const std::vector<std::string> prefixes = {
        "",
        "../",
        "../../"
    };
    for (const auto& prefix : prefixes) {
        std::ifstream f(prefix + requestedPath);
        if (f.good()) {
            return prefix + requestedPath;
        }
    }
    return requestedPath;
}
}

std::vector<Satellite> ConfigLoader::LoadSatellites(const std::string& filepath) {
    std::vector<Satellite> satellites;
    std::string resolved = ResolvePath(filepath);
    std::ifstream f(resolved);
    if(!f.is_open()) {
        std::cout << "Failed to open config: " << filepath << std::endl;
        return satellites;
    }
    
    try {
        json data = json::parse(f);
        for(const auto& item : data) {
            Satellite s;
            s.id = item["id"];
            s.name = item.value("name", "Unknown");
            s.semiMajorAxis = item["semiMajorAxis"];
            s.eccentricity = item["eccentricity"];
            s.inclination = glm::radians((float)item["inclination"]);
            s.raan = glm::radians((float)item["raan"]);
            s.argPeriapsis = glm::radians((float)item["argPeriapsis"]);
            s.meanAnomaly = glm::radians((float)item["meanAnomaly"]);
            s.color = glm::vec3(1.0f); // Default white
            satellites.push_back(s);
        }
    } catch(const std::exception& e) {
        std::cout << "JSON Parse Error: " << e.what() << std::endl;
    }
    return satellites;
}

