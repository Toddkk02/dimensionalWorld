#pragma once

#include "raylib.h"
#include <vector>

struct Monument {
    Vector3 position;
    float height;
    float pulseIntensity;
    float activationRadius;
    float buriedDepth;
    bool discovered;
    bool activated;
    int id;
    Color glowColor;
    float rotationAngle;
    float particleTimer;
};

class MonumentSystem {
public:
    MonumentSystem();
    ~MonumentSystem();
    
    void Init();
    void GenerateMonuments(Vector3 centerPos, int count);
    void Update(Vector3 playerPos, float deltaTime);
    void Draw();
    void Cleanup();
    
    bool IsNearMonument(Vector3 pos, float* distance);
    int GetDiscoveredCount() const;
    int GetActivatedCount() const;

private:
    std::vector<Monument> m_monuments;
    Model m_obeliskModel;
    bool m_initialized;
    float m_spawnTimer;
    float m_activeWatchers;
    float m_modelBaseHeight;
    void CreateMonument(Vector3 pos);
    void DrawMonumentEffects(const Monument& mon);
};