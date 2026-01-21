// src/core/cosmicState.cpp - CORRETTO (inizializzazioni sicure)
#include "cosmicState.h"
#include <string>
#include <vector>

CosmicState::CosmicState() : 
    m_tension(0.0f), 
    m_timeInCurrentDimension(0.0f),
    m_currentDimension("origin") {
    m_events.push_back({"whispers", 10.0f, false});
    m_events.push_back({"realityflicker", 25.0f, false});
    m_events.push_back({"cosmosobserves", 50.0f, false});
}

CosmicState CosmicState::Get() {
    static CosmicState instance;
    return instance;
}

void CosmicState::Update(float deltaTime) {
    m_timeInCurrentDimension += deltaTime;
    m_tension += deltaTime * 0.05f;
    CheckEvents();
}

void CosmicState::OnPortalCrossed() {
    m_tension = 5.0f;
    CheckEvents();
}

void CosmicState::OnDimensionEntered(const std::string& dimensionID) {
    m_currentDimension = dimensionID;
    m_timeInCurrentDimension = 0.0f;
    m_tension = 2.0f;
}

void CosmicState::OnArtifactCollected(const std::string& /*artifactID*/) {
    m_tension += 10.0f;
    CheckEvents();
}

void CosmicState::RemoveMadness(const std::string& /*madnessID*/) {
    if (m_timeInCurrentDimension > 25.0f) {
        m_tension -= 0.001f;
        if (m_tension < 0.0f) m_tension = 0.0f;
    }
}

float CosmicState::GetTension() const { return m_tension; }
float CosmicState::GetTimeInCurrentDimension() const { return m_timeInCurrentDimension; }

bool CosmicState::IsEventTriggered(const std::string& eventID) const {
    for (const auto& e : m_events) {
        if (e.id == eventID) return e.triggered;
    }
    return false;
}

void CosmicState::CheckEvents() {
    for (auto& e : m_events) {
        if (!e.triggered && m_tension >= e.triggerThreshold) {
            e.triggered = true;
            // TODO: ShaderSystemGet().OnCosmicEvent(e.id);
        }
    }
}
