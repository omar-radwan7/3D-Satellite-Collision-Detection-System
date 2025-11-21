#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../scene/SatelliteSystem.h"
#include "../sim/CollisionDetect.h"

struct SimState {
    float* simTime;
    float* timeScale;
    bool* paused;
    int* selectedSatId;
    bool* showOrbits;
    bool* showSatellites;
    bool* showDebris;
    bool* cameraFollow;
};

class GuiManager {
public:
    GuiManager(GLFWwindow* window);
    ~GuiManager();

    void NewFrame();
    void Render(const SimState& state, const SatelliteSystem& satSys, const ConjunctionManager& colMan);
    void RenderDrawData();

private:
    GLFWwindow* window;
};

