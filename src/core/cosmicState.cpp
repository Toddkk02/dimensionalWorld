#include "cosmicState.h"
#include "raylib.h"
#include <string>
#include <vector>

CosmicState::CosmicState() : 
    m_tension(0.0f), 
    m_timeInCurrentDimension(0.0f),
    m_currentDimension("origin") {
    
    m_events.push_back({"whispers", 15.0f, false});
    m_events.push_back({"firstwatcher", 25.0f, false});
    m_events.push_back({"realityflicker", 40.0f, false});
    m_events.push_back({"monumentreveal", 55.0f, false});
    m_events.push_back({"voidgazes", 70.0f, false});
    m_events.push_back({"cosmosobserves", 85.0f, false});
}

CosmicState& CosmicState::Get() {
    static CosmicState instance;
    return instance;   
}

void CosmicState::Update(float deltaTime) {
    m_timeInCurrentDimension += deltaTime;
    
    // Aumenta tension base più lentamente
    m_tension += deltaTime * 0.0005f; // era 0.001f
    
    // ✅ DECAY POTENZIATO dopo permanenza lunga
    if (m_timeInCurrentDimension > 120.0f) {
        float decayRate = 0.001f; // era 0.0005f
        
        // ✅ Decay accelerato dopo 5 minuti
        if (m_timeInCurrentDimension > 300.0f) {
            decayRate = 0.002f;
        }
        
        m_tension -= deltaTime * decayRate;
    }
    
    // ✅ NUOVO: Decay automatico se tension > 80 (troppo intenso)
    if (m_tension > 80.0f) {
        m_tension -= deltaTime * 0.003f;
        TraceLog(LOG_INFO, "⚠️ Tension too high - auto decay active");
    }
    
    if (m_tension < 0.0f) m_tension = 0.0f;
    if (m_tension > 100.0f) m_tension = 100.0f;
    
    CheckEvents();
}

void CosmicState::OnPortalCrossed() {
    m_tension += 0.5f;
    CheckEvents();
}

void CosmicState::OnDimensionEntered(const std::string& dimensionID) {
    m_currentDimension = dimensionID;
    m_timeInCurrentDimension = 0.0f;
    
    m_tension += 1.0f;
    
    if (m_tension > 100.0f)
        m_tension = 100.0f;
    
    CheckEvents();
}

void CosmicState::OnArtifactCollected(const std::string& artifactID) {
    (void)artifactID;
    m_tension += 3.0f;
    CheckEvents();
}

void CosmicState::OnWatcherSeen() {
    m_tension += 5.0f;
    TraceLog(LOG_WARNING, "👁️ WATCHER SEEN! Tension +5.0 (Current: %.1f)", m_tension);
    CheckEvents();
}

void CosmicState::OnMonumentActivated() {
    m_tension += 4.0f;
    TraceLog(LOG_WARNING, "⚡ MONUMENT ACTIVATED! Tension +4.0 (Current: %.1f)", m_tension);
    CheckEvents();
}

void CosmicState::RemoveMadness() {
    if (m_timeInCurrentDimension > 60.0f) {
        m_tension -= 0.0005f;
        if (m_tension < 0.0f) m_tension = 0.0f;
    }
}

float CosmicState::GetTension() const { 
    return m_tension; 
}

float CosmicState::GetTimeInCurrentDimension() const { 
    return m_timeInCurrentDimension; 
}

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
            TraceLog(LOG_WARNING, "🌌 COSMIC EVENT TRIGGERED: %s (Tension: %.1f)", 
                     e.id.c_str(), m_tension);
        }
    }
}