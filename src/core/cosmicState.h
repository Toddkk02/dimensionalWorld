#pragma once
#include <string>
#include <vector>
struct CosmicEvent
{
    std::string id;
    float triggerThreshold;
    bool triggered;
};

class CosmicState
{
public:
    // Accesso globale (engine-style)
    static CosmicState Get();

    // Update per frame
    void Update(float deltaTime);

    // Eventi di gioco
    void OnPortalCrossed();
    void OnDimensionEntered(const std::string& dimensionID);
    void OnArtifactCollected(const std::string& artifactID);
    void RemoveMadness(const std::string& madnessID);
    

    
    // Query per altri sistemi
    float GetTension() const;
    float GetTimeInCurrentDimension() const;
    bool IsEventTriggered(const std::string& eventID) const;

private:
    CosmicState();

    void CheckEvents();

    float m_tension;
    float m_timeInCurrentDimension;
    std::string m_currentDimension;

    std::vector<CosmicEvent> m_events;
};
