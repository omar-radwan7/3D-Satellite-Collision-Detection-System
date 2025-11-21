#include "GuiManager.h"
#include "../sim/conjunctions/ConjunctionAnalyzer.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <array>

GuiManager::GuiManager(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Futuristic HUD styling
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(12, 8);
    style.WindowPadding = ImVec2(14, 12);
    
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]        = ImVec4(0.05f, 0.06f, 0.09f, 0.94f);
    colors[ImGuiCol_TitleBg]         = ImVec4(0.08f, 0.10f, 0.16f, 0.00f);
    colors[ImGuiCol_TitleBgActive]   = ImVec4(0.12f, 0.15f, 0.22f, 0.00f);
    colors[ImGuiCol_FrameBg]         = ImVec4(0.12f, 0.16f, 0.22f, 0.80f);
    colors[ImGuiCol_FrameBgHovered]  = ImVec4(0.18f, 0.26f, 0.36f, 0.90f);
    colors[ImGuiCol_FrameBgActive]   = ImVec4(0.22f, 0.32f, 0.46f, 0.95f);
    colors[ImGuiCol_Button]          = ImVec4(0.18f, 0.25f, 0.35f, 0.90f);
    colors[ImGuiCol_ButtonHovered]   = ImVec4(0.26f, 0.37f, 0.52f, 0.95f);
    colors[ImGuiCol_ButtonActive]    = ImVec4(0.32f, 0.46f, 0.64f, 1.00f);
    colors[ImGuiCol_SliderGrab]      = ImVec4(0.40f, 0.75f, 0.95f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]= ImVec4(0.55f, 0.90f, 1.00f, 1.00f);
    colors[ImGuiCol_Header]          = ImVec4(0.16f, 0.24f, 0.35f, 0.80f);
    colors[ImGuiCol_HeaderHovered]   = ImVec4(0.24f, 0.36f, 0.50f, 0.90f);
    colors[ImGuiCol_HeaderActive]    = ImVec4(0.32f, 0.48f, 0.65f, 1.00f);
    colors[ImGuiCol_Text]            = ImVec4(0.88f, 0.92f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled]    = ImVec4(0.45f, 0.50f, 0.58f, 1.00f);
    colors[ImGuiCol_PlotLines]       = ImVec4(0.25f, 0.55f, 0.75f, 1.00f);
    colors[ImGuiCol_PlotHistogram]   = ImVec4(0.90f, 0.70f, 0.20f, 1.00f);
    
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

const char* getRiskLevelName(RiskLevel level) {
    switch(level) {
        case RiskLevel::CRITICAL: return "CRITICAL";
        case RiskLevel::HIGH: return "HIGH";
        case RiskLevel::MEDIUM: return "MEDIUM";
        case RiskLevel::LOW: return "LOW";
        default: return "SAFE";
    }
}

ImVec4 getRiskLevelColor(RiskLevel level) {
    switch(level) {
        case RiskLevel::CRITICAL: return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        case RiskLevel::HIGH: return ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
        case RiskLevel::MEDIUM: return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        case RiskLevel::LOW: return ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
        default: return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    }
}

void GuiManager::Render(const SimState& state, const SatelliteSystem& sats, const ConjunctionManager& collisions, const ConjunctionAnalyzer& conjAnalyzer) {
    ImGuiIO& io = ImGui::GetIO();
    
    // ===== MISSION CONTROL (Left Panel) =====
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(300, 400));
    ImGui::Begin("Mission Control", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    
    // Playback controls
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "⏱ TIME CONTROL");
    ImGui::Separator();
    ImGui::Text("Time: %.1f s", *state.simTime);
    
    if (*state.paused) {
        if (ImGui::Button("▶ PLAY", ImVec2(80, 30))) *state.paused = false;
    } else {
        if (ImGui::Button("⏸ PAUSE", ImVec2(80, 30))) *state.paused = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("⏹ STOP", ImVec2(80, 30))) {
        *state.simTime = 0.0f;
        *state.paused = true;
    }
    
    ImGui::SliderFloat("Speed", state.timeScale, 0.1f, 500.0f, "%.0fx");
    
    // Quick speed buttons
    ImGui::Text("Quick Speed:");
    if (ImGui::Button("1x", ImVec2(50, 25))) *state.timeScale = 1.0f;
    ImGui::SameLine();
    if (ImGui::Button("10x", ImVec2(50, 25))) *state.timeScale = 10.0f;
    ImGui::SameLine();
    if (ImGui::Button("50x", ImVec2(50, 25))) *state.timeScale = 50.0f;
    ImGui::SameLine();
    if (ImGui::Button("100x", ImVec2(50, 25))) *state.timeScale = 100.0f;
    ImGui::SameLine();
    if (ImGui::Button("500x", ImVec2(50, 25))) *state.timeScale = 500.0f;
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "📡 DISPLAY");
    ImGui::Separator();
    ImGui::Checkbox("Satellites", state.showSatellites);
    ImGui::Checkbox("Orbits", state.showOrbits);
    ImGui::Checkbox("Debris", state.showDebris);
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "ℹ SATELLITES");
    ImGui::Separator();
    int activeCount = 0;
    int totalCount = sats.getSatellites().size();
    for(const auto& s : sats.getSatellites()) {
        if(s.active) activeCount++;
    }
    ImGui::Text("Active: %d / %d", activeCount, totalCount);
    ImGui::Text("Destroyed: %d", totalCount - activeCount);
    
    ImGui::End();
    
    // ===== CONJUNCTION ANALYSIS (Right Top Panel) =====
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 350, 10));
    ImGui::SetNextWindowSize(ImVec2(340, 450));
    ImGui::Begin("Conjunction Analysis", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    
    const auto& conjEvents = conjAnalyzer.getEvents();
    const auto& criticalEvents = conjAnalyzer.getCriticalEvents();
    
    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "⚠ RISK ASSESSMENT");
    ImGui::Separator();
    ImGui::Text("Total Conjunctions: %lu", conjEvents.size());
    ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.0f, 1.0f), "Critical Events: %lu", criticalEvents.size());
    
    ImGui::Spacing();
    if(conjEvents.empty()) {
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "✓ NO NEAR-MISS EVENTS");
        ImGui::Text("All satellites are at safe distance.");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "⚡ ACTIVE CONJUNCTIONS:");
        
        ImGui::BeginChild("ConjunctionList", ImVec2(0, 320), true);
        
        for(size_t i = 0; i < conjEvents.size() && i < 10; ++i) {
            const auto& event = conjEvents[i];
            
            ImGui::PushID(i);
            ImGui::Separator();
            
            // Risk level indicator
            ImVec4 riskColor = getRiskLevelColor(event.risk_level);
            ImGui::TextColored(riskColor, "● %s RISK", getRiskLevelName(event.risk_level));
            
            ImGui::Text("Sat-%d ↔ Sat-%d", event.sat1_id, event.sat2_id);
            ImGui::Text("TCA: T+%.1fs", event.tca_time - *state.simTime);
            ImGui::Text("Miss Dist: %.2f km", event.min_distance);
            ImGui::Text("Rel Vel: %.2f km/s", event.relative_velocity);
            ImGui::Text("Risk Score: %.0f", event.risk_score);
            
            // Energy bar
            float energyNormalized = std::min(event.collision_energy / 1e8f, 1.0f);
            ImGui::ProgressBar(energyNormalized, ImVec2(-1, 0), "");
            ImGui::SameLine(0, 5);
            ImGui::Text("Energy");
            
            ImGui::PopID();
        }
        
        if(conjEvents.size() > 10) {
            ImGui::Text("... and %lu more", conjEvents.size() - 10);
        }
        
        ImGui::EndChild();
    }
    
    ImGui::End();
    
    // ===== COLLISION STATUS (Right Bottom Panel) =====
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 350, 470));
    ImGui::SetNextWindowSize(ImVec2(340, 240));
    ImGui::Begin("Collision Status", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    
    const auto& collisionEvents = collisions.getEvents();
    
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.0f, 1.0f), "💥 ACTIVE COLLISIONS");
    ImGui::Separator();
    
    if(collisionEvents.empty()) {
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "✓ NO ACTIVE COLLISIONS");
    } else {
        ImGui::Text("Collision Count: %lu", collisionEvents.size());
        ImGui::Spacing();
        
        ImGui::BeginChild("CollisionList", ImVec2(0, 150), true);
        for(const auto& ev : collisionEvents) {
            ImGui::Text("● Sat %d <-> Sat %d", ev.sat1_id, ev.sat2_id);
            if(ev.willFallToEarth) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.0f, 1.0f), "⚠ DEBRIS");
                ImGui::Text("  Impact ETA: %.1fs", ev.timeToImpact);
            }
            ImGui::Separator();
        }
        ImGui::EndChild();
    }
    
    ImGui::End();
}

void GuiManager::RenderDrawData() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
