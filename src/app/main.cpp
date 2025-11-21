#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <set>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../render/Shader.h"
#include "../render/Buffers.h"
#include "../render/Camera.h"
#include "../scene/Earth.h"
#include "../scene/SatelliteSystem.h"
#include "../scene/DebrisSystem.h"
#include "../scene/CollisionWarning.h"
#include "../sim/CollisionDetect.h"
#include "../sim/conjunctions/ConjunctionAnalyzer.h"
#include "../sim/ConjunctionVisualizer.h"
#include "../ui/GuiManager.h"
#include "../util/ConfigLoader.h"
#include "imgui.h"

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.5f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Systems
Earth* earth;
SatelliteSystem* satSystem;
DebrisSystem* debrisSystem;
ConjunctionManager* colMan;
CollisionWarningRenderer* warningRenderer;
ConjunctionAnalyzer* conjunctionAnalyzer;
ConjunctionVisualizer* conjunctionVis;
GuiManager* gui;

// State
float simTime = 0.0f;
float timeScale = 50.0f; // Start at 50x speed for faster observation 
bool paused = false;
int selectedSatId = -1;
bool showOrbits = true;
bool showSatellites = true;
bool showDebris = true;
bool cameraFollow = false;
bool showConjunctions = true;
float conjunctionUpdateTimer = 0.0f;
float conjunctionUpdateInterval = 1.0f; // Update every 1 second (simulation time) for faster updates

std::set<std::pair<int,int>> activeCollisions;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if(ImGui::GetIO().WantCaptureMouse) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    bool leftClick = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool rightClick = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    if (leftClick) {
        if (shiftPressed) {
            // Manually Rotate Earth 
            earth->Rotate(xoffset, yoffset, camera.Right, camera.Up);
        } 
        else if (camera.IsOrbiting) {
            camera.ProcessMouseMovement(xoffset, yoffset);
        }
    } 
    else if (rightClick && !camera.IsOrbiting) {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if(ImGui::GetIO().WantCaptureMouse) return;
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if(ImGui::GetIO().WantCaptureKeyboard) return;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
        
    static bool cPressed = false;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!cPressed) {
            camera.IsOrbiting = !camera.IsOrbiting;
            if (camera.IsOrbiting) 
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            cPressed = true;
        }
    } else {
        cPressed = false;
    }
}

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Satellite Collision Simulator", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (glewInit() != GLEW_OK) return -1;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    earth = new Earth();
    satSystem = new SatelliteSystem();
    debrisSystem = new DebrisSystem();
    colMan = new ConjunctionManager();
    warningRenderer = new CollisionWarningRenderer();
    conjunctionAnalyzer = new ConjunctionAnalyzer();
    conjunctionVis = new ConjunctionVisualizer();
    gui = new GuiManager(window);

    // Load satellites (Placeholder if file missing)
    std::vector<Satellite> loadedSats = ConfigLoader::LoadSatellites("assets/config/satellites.json");
    for(const auto& s : loadedSats) satSystem->addSatellite(s);

    // ===== FORCED COLLISION TEST: Satellites start at SAME position! =====
    // These satellites start at exactly the same point to trigger immediate collision warning
    {
        Satellite test1, test2;
        
        // Test satellite 1 - Very low LEO
        test1.id = 9001;
        test1.name = "COLLISION-SAT-A";
        test1.semiMajorAxis = 6900.0f; // 529 km altitude - LOW orbit
        test1.eccentricity = 0.0f;
        test1.inclination = glm::radians(30.0f);
        test1.raan = glm::radians(0.0f);
        test1.argPeriapsis = glm::radians(0.0f);
        test1.meanAnomaly = glm::radians(0.0f); // Starting at same position!
        test1.color = glm::vec3(0.0f, 1.0f, 1.0f); // CYAN (Iridium-33 style)
        
        // Test satellite 2 - EXACT SAME POSITION (guaranteed collision at t=0!)
        test2.id = 9002;
        test2.name = "COLLISION-SAT-B";
        test2.semiMajorAxis = 6900.0f; // Same altitude
        test2.eccentricity = 0.0f;
        test2.inclination = glm::radians(30.0f); // Same inclination
        test2.raan = glm::radians(0.0f); // Same RAAN
        test2.argPeriapsis = glm::radians(0.0f); // Same argument
        test2.meanAnomaly = glm::radians(0.5f); // Almost same position (within 50km)
        test2.color = glm::vec3(1.0f, 0.5f, 0.0f); // ORANGE (Cosmos-2251 style)
        
        satSystem->addSatellite(test1);
        satSystem->addSatellite(test2);
        
        std::cout << "\n=== COLLISION TEST SATELLITES ADDED ===" << std::endl;
        std::cout << "Sat 9001 (CYAN) and Sat 9002 (ORANGE) will collide!" << std::endl;
        std::cout << "Watch for Conjunction Assessment Vectors!\n" << std::endl;
    }

    srand(42); 
    int numSatellites = 50; // Reduced to better see collision test satellites
    for(int i = 0; i < numSatellites; ++i) {
        Satellite s;
        s.id = 1000 + i;
        
        // Distribution: Realistic altitudes to prevent clipping through Earth
        // 70% LEO (400-2000km altitude = 6771-8371km radius)
        // 20% MEO (2000-35000km altitude)
        // 10% GEO (35786km altitude)
        int type = rand() % 100;
        if (type < 70) {
            // LEO: 400-2000 km altitude
            s.semiMajorAxis = 6771.0f + (rand() % 1600);
        } else if (type < 90) {
            // MEO: 2000-35000 km altitude
            s.semiMajorAxis = 8371.0f + (rand() % 32629);
        } else {
            // GEO: ~35786 km altitude (42157 km radius)
            s.semiMajorAxis = 42157.0f + (rand() % 200);
        }

        s.eccentricity = (rand() % 50) / 1000.0f; // Very low eccentricity
        s.inclination = glm::radians((float)(rand() % 180));
        s.raan = glm::radians((float)(rand() % 360));
        s.argPeriapsis = glm::radians((float)(rand() % 360));
        s.meanAnomaly = glm::radians((float)(rand() % 360));
        
        // Color for orbit lines
        float alt = s.semiMajorAxis - 6371.0f;
        if (alt < 2000) s.color = glm::vec3(0.4, 0.8, 1.0); // Cyan for LEO
        else if (alt < 30000) s.color = glm::vec3(0.4, 1.0, 0.4); // Green for MEO
        else s.color = glm::vec3(1.0, 0.4, 0.4); // Red for GEO
        
        satSystem->addSatellite(s);
    }

    // Initialize orbit paths after adding all satellites
    satSystem->initOrbits();

    SimState state;
    state.simTime = &simTime;
    state.timeScale = &timeScale;
    state.paused = &paused;
    state.selectedSatId = &selectedSatId;
    state.showOrbits = &showOrbits;
    state.showSatellites = &showSatellites;
    state.showDebris = &showDebris;
    state.cameraFollow = &cameraFollow;

    camera.IsOrbiting = true;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        gui->NewFrame();

        earth->Update(deltaTime);

        if (!paused) {
            simTime += deltaTime * timeScale;
            satSystem->update(simTime);
            colMan->update(satSystem->getSatellites(), simTime);
            
            // Update collision warnings
            const auto& predictions = colMan->getPredictions();
            warningRenderer->update(predictions, currentFrame);
            
            // Conjunction analysis (periodic update for performance)
            conjunctionUpdateTimer += deltaTime * timeScale;
            if(conjunctionUpdateTimer >= conjunctionUpdateInterval) {
                conjunctionAnalyzer->analyzeFutureConjunctions(
                    satSystem->getSatellites(),
                    simTime,
                    3600.0f // Look ahead 1 hour
                );
                conjunctionUpdateTimer = 0.0f;
            }
            
            // Update conjunction visualization
            conjunctionVis->update(conjunctionAnalyzer->getEvents(), currentFrame);
            
            const auto& events = colMan->getEvents();
            std::set<std::pair<int,int>> currentCollisions;
            for(const auto& ev : events) {
                std::pair<int,int> key = {ev.sat1_id, ev.sat2_id};
                currentCollisions.insert(key);
                if(activeCollisions.find(key) == activeCollisions.end()) {
                    // New collision
                    const auto& satellites = satSystem->getSatellites();
                    glm::vec3 p1, p2, v1, v2, c1, c2;
                    bool f1=false, f2=false;
                    for(const auto& s : satellites) {
                        if(s.id == ev.sat1_id) { p1 = s.position; v1 = s.velocity; c1 = s.color; f1=true; }
                        if(s.id == ev.sat2_id) { p2 = s.position; v2 = s.velocity; c2 = s.color; f2=true; }
                    }
                    if(f1 && f2) {
                        glm::vec3 mid = (p1 + p2) * 0.5f;
                        // Pass separate velocities for correct "butterfly" cloud shape
                        debrisSystem->addExplosion(mid, v1, v2, c1, c2);
                        
                        // Destroy satellites (remove from map)
                        satSystem->destroySatellite(ev.sat1_id);
                        satSystem->destroySatellite(ev.sat2_id);
                    }
                }
            }
            activeCollisions = currentCollisions;
        }

        debrisSystem->update(deltaTime);

        if (cameraFollow && selectedSatId != -1) {
            const auto& satellites = satSystem->getSatellites();
            for(const auto& s : satellites) {
                if(s.id == selectedSatId) {
                    glm::vec3 targetPos = s.position * (1.0f / 6371.0f); // Scale down to visual earth
                    if(camera.IsOrbiting) camera.Target = targetPos;
                    else camera.Position = targetPos + glm::vec3(0.0f, 0.1f, 0.1f);
                    break;
                }
            }
        } else if (camera.IsOrbiting) {
            camera.Target = glm::vec3(0.0f);
        }

        processInput(window);

        glClearColor(0.0f, 0.0f, 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 sunDir = glm::normalize(glm::vec3(1.0f, 0.0f, 0.5f));
        
        earth->Draw(view, projection, camera.Position, sunDir);

        if (showOrbits) satSystem->drawOrbits(view, projection);
        if (showSatellites) satSystem->drawSatellites(view, projection);
        if (showDebris) debrisSystem->draw(view, projection);
        
        // Draw collision warnings (trajectory and impact markers)
        warningRenderer->draw(view, projection);
        
        // Draw conjunction risk visualization
        if (showConjunctions) conjunctionVis->draw(view, projection);

        gui->Render(state, *satSystem, *colMan, *conjunctionAnalyzer);
        gui->RenderDrawData();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete earth;
    delete satSystem;
    delete debrisSystem;
    delete colMan;
    delete warningRenderer;
    delete conjunctionAnalyzer;
    delete conjunctionVis;
    delete gui;
    glfwTerminate();
    return 0;
}
