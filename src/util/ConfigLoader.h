#pragma once
#include <vector>
#include <string>
#include "../scene/Satellite.h"

class ConfigLoader {
public:
    static std::vector<Satellite> LoadSatellites(const std::string& path);
};

