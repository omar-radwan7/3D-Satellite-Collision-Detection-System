#pragma once
#include <vector>
#include "../scene/Satellite.h"

struct ConjunctionEvent {
    int sat1_id;
    int sat2_id;
    float time;
    float distance; // km
    float relativeVelocity; // km/s
    bool active;
};

class ConjunctionManager {
public:
    void update(const std::vector<Satellite>& satellites, float currentTime);
    const std::vector<ConjunctionEvent>& getEvents() const { return events; }
    void clearEvents() { events.clear(); }

private:
    std::vector<ConjunctionEvent> events;
    // Threshold in km. Real conjunctions are < 1km, but for visual demo we use larger.
    // Earth Radius ~6371km.
    // Let's use 200km for "warning" visualization.
    float warningThreshold = 200.0f; 
};

