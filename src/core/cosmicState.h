#pragma once

#include <string>
#include <vector>

class CosmicState {
public:
    static CosmicState& Get();

    void Update(float deltaTime);
    void OnPortalCrossed();
    void OnDimensionEntered(const std::string& dimensionID);
    void OnArtifactCollected(const std::string& artifactID);
    void OnWatcherSeen();
    void OnMonumentActivated();
    void RemoveMadness();

    float GetTension() const;
    float GetTimeInCurrentDimension() const;
    bool IsEventTriggered(const std::string& eventID) const;

private:
    CosmicState();
    CosmicState(const CosmicState&) = delete;
    CosmicState& operator=(const CosmicState&) = delete;

    struct CosmicEvent {
        std::string id;
        float triggerThreshold;
        bool triggered;
    };

    float m_tension;
    float m_timeInCurrentDimension;
    std::string m_currentDimension;
    std::vector<CosmicEvent> m_events;

    void CheckEvents();
};