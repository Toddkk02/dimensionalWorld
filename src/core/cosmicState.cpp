#include "cosmicState.h"
#include "raylib.h"
#include <string>
#include <vector>

CosmicState::CosmicState() : m_tension(0.0f),
                             m_timeInCurrentDimension(0.0f),
                             m_currentDimension("origin")
{

    m_events.push_back({"whispers", 15.0f, false});
    m_events.push_back({"firstwatcher", 25.0f, false});
    m_events.push_back({"realityflicker", 40.0f, false});
    m_events.push_back({"monumentreveal", 55.0f, false});
    m_events.push_back({"voidgazes", 70.0f, false});
    m_events.push_back({"cosmosobserves", 85.0f, false});
}

CosmicState &CosmicState::Get()
{
    static CosmicState instance;
    return instance;
}

// In cosmicState.cpp - aggiungi log nel metodo Update:
void CosmicState::Update(float deltaTime)
{
    m_timeInCurrentDimension += deltaTime;
    
    // ✅ DEBUG: Stampa ogni 5 secondi
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    if (debugTimer > 5.0f)
    {
        TraceLog(LOG_WARNING, "=== COSMIC STATE DEBUG ===");
        TraceLog(LOG_WARNING, "  Current Dimension: '%s'", m_currentDimension.c_str());
        TraceLog(LOG_WARNING, "  Is Dangerous: %s", DoesDimensionIncreaseTension() ? "YES" : "NO");
        TraceLog(LOG_WARNING, "  Tension: %.2f", m_tension);
        TraceLog(LOG_WARNING, "  Time in dim: %.0fs", m_timeInCurrentDimension);
        debugTimer = 0.0f;
    }
    
    // ✅ DECAY nelle dimensioni sicure, AUMENTO nelle dimensioni pericolose
    if (DoesDimensionIncreaseTension())
    {
        // Dimensioni pericolose: aumenta tension
        m_tension += deltaTime * 0.0005f;
    }
    else
    {
        // ✅ Dimensioni sicure: SEMPRE decay
        float decayRate = 0.001f; // Base decay
        
        // Decay più veloce se sei rimasto a lungo
        if (m_timeInCurrentDimension > 60.0f)
        {
            decayRate = 0.002f;
        }
        
        // Decay ancora più veloce dopo 2 minuti
        if (m_timeInCurrentDimension > 120.0f)
        {
            decayRate = 0.03f;
        }
        
        m_tension -= deltaTime * decayRate;
        
        // Log decay
        static float logTimer = 0.0f;
        logTimer += deltaTime;
        if (logTimer > 2.0f) // Ogni 2 secondi invece di 10
        {
            TraceLog(LOG_INFO, "🛡️ Safe zone decay: Tension %.1f (rate: %.4f/s)", 
                     m_tension, decayRate);
            logTimer = 0.0f;
        }
    }
    
    // ✅ Decay automatico se tension > 80 (emergency relief)
    if (m_tension > 80.0f)
    {
        m_tension -= deltaTime * 0.003f;
        TraceLog(LOG_INFO, "⚠️ Tension too high - auto decay active");
    }
    
    // Clamp
    if (m_tension < 0.0f) m_tension = 0.0f;
    if (m_tension > 100.0f) m_tension = 100.0f;
    
    CheckEvents();
}
bool CosmicState::DoesDimensionIncreaseTension() const
{
    if (m_currentDimension == "Forest Dimension")
        return false;
    if (m_currentDimension == "Desert Dimension")
        return false;
    if (m_currentDimension == "Ice Dimension")
        return false;

    return true;
}

void CosmicState::OnPortalCrossed()
{
    m_tension += 0.005f;
    CheckEvents();
}

void CosmicState::OnDimensionEntered(const std::string &dimensionID)
{
    m_currentDimension = dimensionID;
    m_timeInCurrentDimension = 0.0f;
        m_tension += 1.0f;

    if (m_tension > 100.0f)
        m_tension = 100.0f;

    CheckEvents();
}

void CosmicState::OnArtifactCollected(const std::string &artifactID)
{
    (void)artifactID;
    m_tension += 3.0f;
    CheckEvents();
}

void CosmicState::OnWatcherSeen()
{
    m_tension += 5.0f;
    TraceLog(LOG_WARNING, "👁️ WATCHER SEEN! Tension +5.0 (Current: %.1f)", m_tension);
    CheckEvents();
}

void CosmicState::OnMonumentActivated()
{
    m_tension += 4.0f;
    TraceLog(LOG_WARNING, "⚡ MONUMENT ACTIVATED! Tension +4.0 (Current: %.1f)", m_tension);
    CheckEvents();
}

void CosmicState::RemoveMadness()
{
    if (m_timeInCurrentDimension > 60.0f)
    {
        m_tension -= 0.0005f;
        if (m_tension < 0.0f)
            m_tension = 0.0f;
    }
}

float CosmicState::GetTension() const
{
    return m_tension;
}

float CosmicState::GetTimeInCurrentDimension() const
{
    return m_timeInCurrentDimension;
}

bool CosmicState::IsEventTriggered(const std::string &eventID) const
{
    for (const auto &e : m_events)
    {
        if (e.id == eventID)
            return e.triggered;
    }
    return false;
}

void CosmicState::CheckEvents()
{
    for (auto &e : m_events)
    {
        if (!e.triggered && m_tension >= e.triggerThreshold)
        {
            e.triggered = true;
            TraceLog(LOG_WARNING, "🌌 COSMIC EVENT TRIGGERED: %s (Tension: %.1f)",
                     e.id.c_str(), m_tension);
        }
    }
}