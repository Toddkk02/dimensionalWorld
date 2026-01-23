#pragma once

#include "raylib.h"
#include <vector>

struct Watcher {
    Vector3 position;
    Vector3 targetPosition;
    float opacity;
    float size;
    float moveTimer;
    float stareIntensity;
    bool isVisible;
    bool playerLooking;
    int id;
    float distanceToPlayer;
};

class WatcherSystem {
public:
    WatcherSystem();
    ~WatcherSystem();
    
    void Init();
    void Update(Camera3D camera, float tension, float deltaTime);
    void Draw(Camera3D camera);
    void Cleanup();
    
    bool IsPlayerBeingWatched() const { return m_activeWatchers > 0; }
    int GetActiveWatcherCount() const { return m_activeWatchers; }
    
    void SpawnWatcher(Vector3 playerPos, float tension);  // ← AGGIUNGI

private:
    std::vector<Watcher> m_watchers;
    Model m_watcherModel;
    int m_activeWatchers;
    float m_spawnTimer;
    bool m_initialized;
    
    void UpdateWatcher(Watcher& watcher, Camera3D camera, float deltaTime);
    bool IsInPlayerView(Vector3 pos, Camera3D camera);
    Vector3 GetBehindPlayerPosition(Camera3D camera, float distance);
};