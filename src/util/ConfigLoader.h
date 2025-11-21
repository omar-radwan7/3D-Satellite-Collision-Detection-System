#pragma once
#include <vector>
#include <string>
#include "../scene/SatelliteSystem.h"

class ConfigLoader {
public:
    static std::vector<Satellite> LoadSatellites(const std::string& filepath);
};
