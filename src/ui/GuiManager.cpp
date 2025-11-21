#include "GuiManager.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <string>

GuiManager::GuiManager(GLFWwindow* window) : window(window) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

GuiManager::~GuiManager() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GuiManager::NewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GuiManager::Render(const SimState& state, const SatelliteSystem& satSys, const ConjunctionManager& colMan) {
    ImGui::Begin("Simulation Controls");
    
    ImGui::Text("Simulation Time: %.2f", *state.simTime);
    ImGui::SliderFloat("Time Scale", state.timeScale, 0.0f, 100.0f);
    ImGui::Checkbox("Paused", state.paused);
    
    if (ImGui::Button("Reset Time")) {
        *state.simTime = 0.0f;
    }

    ImGui::Separator();
    ImGui::Text("Rendering");
    ImGui::Checkbox("Show Orbits", state.showOrbits);
    ImGui::Checkbox("Show Satellites", state.showSatellites);
    ImGui::Checkbox("Show Debris", state.showDebris);

    ImGui::Separator();
    ImGui::Text("Camera");
    ImGui::Checkbox("Follow Selected Satellite", state.cameraFollow);

    ImGui::End();

    ImGui::Begin("Satellite Inspector");
    
    const auto& satellites = ((SatelliteSystem&)satSys).getSatellites(); // Cast away const needed? No, getSatellites should return const ref if const object
    // My getSatellites was non-const in header. Let's fix cast or usage.
    // Actually, accessing by index is safe if vector doesn't change.
    
    // List box
    if (ImGui::BeginListBox("Satellites")) {
        for (const auto& sat : satellites) {
            bool is_selected = (*state.selectedSatId == sat.id);
            std::string label = "Sat " + std::to_string(sat.id);
            if (ImGui::Selectable(label.c_str(), is_selected)) {
                *state.selectedSatId = sat.id;
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();
    }

    if (*state.selectedSatId != -1) {
        // Find sat
        for (const auto& sat : satellites) {
            if (sat.id == *state.selectedSatId) {
                ImGui::Text("ID: %d", sat.id);
                ImGui::Text("Pos (km): %.1f, %.1f, %.1f", sat.position.x, sat.position.y, sat.position.z);
                ImGui::Text("SMA: %.1f km", sat.semiMajorAxis);
                ImGui::Text("Ecc: %.4f", sat.eccentricity);
                ImGui::Text("Inc: %.2f deg", glm::degrees(sat.inclination));
                break;
            }
        }
    }

    ImGui::End();

    ImGui::Begin("Collision Alerts");
    const auto& events = colMan.getEvents();
    ImGui::Text("Detected Conjunctions: %lu", events.size());
    
    if (ImGui::BeginTable("Events", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Sat 1");
        ImGui::TableSetupColumn("Sat 2");
        ImGui::TableSetupColumn("Dist (km)");
        ImGui::TableSetupColumn("Time");
        ImGui::TableHeadersRow();

        for (const auto& ev : events) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", ev.sat1_id);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", ev.sat2_id);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.1f", ev.distance);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.1f", ev.time);
        }
        ImGui::EndTable();
    }
    ImGui::End();

    ImGui::Render();
}

void GuiManager::RenderDrawData() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

