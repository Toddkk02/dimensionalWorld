#include "monuments.h"
#include "../core/cosmicState.h"
#include <raymath.h>
#include <cmath>
#include <stdlib.h>
#include <cfloat>
MonumentSystem::MonumentSystem() : m_initialized(false)
{
}

MonumentSystem::~MonumentSystem()
{
    if (m_initialized)
    {
        Cleanup();
    }
}

void MonumentSystem::Init()
{
    m_monuments.clear();
    m_activeWatchers = 0;
    m_spawnTimer = 0;
    
    const char* monumentModelPaths[] = {
        "assets/models/obelisk/monolith.glb",
        "assets/models/obelisk.obj",
        "assets/models/alien_pillar.obj"
    };
    
    bool modelLoaded = false;
    for (const char* path : monumentModelPaths) {
        if (FileExists(path)) {
            m_obeliskModel = LoadModel(path);
            if (m_obeliskModel.meshCount > 0) {
                modelLoaded = true;
                TraceLog(LOG_INFO, "✓ Monument model loaded: %s", path);
                break;
            }
        }
    }
    
    if (!modelLoaded) {
        Mesh obelisk = GenMeshCylinder(0.5f, 8.0f, 6);
        m_obeliskModel = LoadModelFromMesh(obelisk);
        TraceLog(LOG_INFO, "✓ Monument using procedural cylinder (no custom model found)");
    }
    
    // ✅ FIX: Calcola altezza dopo il caricamento
    m_modelBaseHeight = 8.0f; // Default fallback
    
    if (m_obeliskModel.meshCount > 0) {
        BoundingBox box = GetMeshBoundingBox(m_obeliskModel.meshes[0]);
        float height = box.max.y - box.min.y;
        
        if (height > 0.1f && height < 1000.0f) { // Validazione
            m_modelBaseHeight = height;
            TraceLog(LOG_INFO, "✓ Model height calculated: %.2f units", m_modelBaseHeight);
        } else {
            TraceLog(LOG_WARNING, "⚠️ Invalid model height (%.2f), using fallback: %.2f", 
                     height, m_modelBaseHeight);
        }
    }
    
    m_initialized = true;
    TraceLog(LOG_INFO, "✓ Monument System initialized");
}
void MonumentSystem::GenerateMonuments(Vector3 centerPos, int count)
{
    if (!m_initialized)
    {
        TraceLog(LOG_ERROR, "MonumentSystem not initialized!");
        return;
    }

    for (int i = 0; i < count; i++)
    {
        float angle = (rand() % 360) * DEG2RAD;
        float distance = 50.0f + (rand() % 100);

        Vector3 pos = (Vector3){
            centerPos.x + cosf(angle) * distance,
            centerPos.y,
            centerPos.z + sinf(angle) * distance};

        CreateMonument(pos);
    }

    TraceLog(LOG_INFO, "Generated %d monuments around (%.0f, %.0f, %.0f)",
             count, centerPos.x, centerPos.y, centerPos.z);
}

void MonumentSystem::CreateMonument(Vector3 pos)
{
    Monument mon;
    mon.position = pos;
    mon.height = 6.0f + (rand() % 4);
    mon.buriedDepth = 0.3f + ((rand() % 20) / 100.0f);
    mon.pulseIntensity = 0.0f;
    mon.activationRadius = 5.0f;
    mon.discovered = false;
    mon.activated = false;
    mon.id = m_monuments.size();
    mon.glowColor = (Color){150, 50, 255, 255};
    mon.rotationAngle = 0.0f;
    mon.particleTimer = 0.0f;

    m_monuments.push_back(mon);
}

void MonumentSystem::Update(Vector3 playerPos, float deltaTime)
{
    if (!m_initialized)
        return;

    for (auto &mon : m_monuments)
    {
        float dist = Vector3Distance(playerPos, mon.position);

        if (dist < 30.0f && !mon.discovered)
        {
            mon.discovered = true;
            TraceLog(LOG_INFO, "🗿 Monument #%d discovered at distance %.1f", mon.id, dist);
        }

        if (dist < mon.activationRadius && !mon.activated && mon.discovered)
        {
            mon.activated = true;
            mon.pulseIntensity = 1.0f;
            CosmicState::Get().OnMonumentActivated();
            TraceLog(LOG_WARNING, "⚡ Monument #%d ACTIVATED! Reality trembles...", mon.id);
        }

        if (mon.discovered)
        {
            mon.pulseIntensity = 0.5f + sinf(GetTime() * 2.0f + mon.id) * 0.5f;
            mon.rotationAngle += deltaTime * 20.0f;
            if (mon.rotationAngle > 360.0f)
                mon.rotationAngle -= 360.0f;
        }

        if (mon.activated)
        {
            mon.particleTimer += deltaTime;
        }
    }
}

void MonumentSystem::Draw()
{
    if (!m_initialized)
        return;

    for (const auto &mon : m_monuments)
    {
        if (!mon.discovered)
            continue;

        Vector3 drawPos = mon.position;
        
        // Calcola lo scale corretto basato sull'altezza del modello
        float targetHeight = 8.0f; // Altezza desiderata in metri
        float scaleFactor = targetHeight / m_modelBaseHeight;

        // ✅ NOME CORRETTO: m_obeliskModel
        DrawModelEx(
            m_obeliskModel,  // <-- Era m_watcherModel, ora corretto
            drawPos,
            (Vector3){0, 1, 0},
            mon.rotationAngle,
            (Vector3){scaleFactor, scaleFactor, scaleFactor},
            WHITE);
            
        DrawMonumentEffects(mon);
    }
}

void MonumentSystem::DrawMonumentEffects(const Monument &mon)
{
    Color glowColor = Fade(mon.glowColor, mon.pulseIntensity * 0.6f);

    int particleCount = mon.activated ? 20 : 10;
    for (int i = 0; i < particleCount; i++)
    {
        float angle = (GetTime() * 50.0f + i * 360.0f / particleCount) * DEG2RAD;
        float radius = 2.0f + sinf(GetTime() * 2.0f + i) * 0.5f;

        Vector3 pPos = (Vector3){
            mon.position.x + cosf(angle) * radius,
            mon.position.y + mon.height * 0.5f + sinf(GetTime() * 3.0f + i) * 2.0f,
            mon.position.z + sinf(angle) * radius};

        DrawSphere(pPos, 0.1f, glowColor);
    }

    if (mon.activated)
    {
        for (int i = 0; i < 6; i++)
        {
            float y = mon.position.y + (i / 6.0f) * mon.height;
            DrawCircle3D(
                (Vector3){mon.position.x, y, mon.position.z},
                0.6f,
                (Vector3){1, 0, 0},
                90.0f,
                Fade(mon.glowColor, mon.pulseIntensity * 0.8f));
        }

        int beamCount = 8;
        for (int i = 0; i < beamCount; i++)
        {
            float angle = (GetTime() * 30.0f + i * 360.0f / beamCount) * DEG2RAD;
            float beamLength = 3.0f + sinf(GetTime() * 2.0f + i) * 1.0f;

            Vector3 start = mon.position;
            start.y += mon.height;

            Vector3 end = (Vector3){
                start.x + cosf(angle) * beamLength,
                start.y + 2.0f,
                start.z + sinf(angle) * beamLength};

            DrawLine3D(start, end, Fade(mon.glowColor, mon.pulseIntensity * 0.4f));
        }
    }

    Vector3 topPos = mon.position;
    topPos.y += mon.height + 0.5f;
    DrawSphere(topPos, 0.3f, Fade(mon.glowColor, mon.pulseIntensity));
}

bool MonumentSystem::IsNearMonument(Vector3 pos, float *distance)
{
    if (!m_initialized)
        return false;

    for (const auto &mon : m_monuments)
    {
        float dist = Vector3Distance(pos, mon.position);
        if (dist < 10.0f)
        {
            if (distance)
                *distance = dist;
            return true;
        }
    }
    return false;
}

int MonumentSystem::GetDiscoveredCount() const
{
    int count = 0;
    for (const auto &mon : m_monuments)
    {
        if (mon.discovered)
            count++;
    }
    return count;
}

int MonumentSystem::GetActivatedCount() const
{
    int count = 0;
    for (const auto &mon : m_monuments)
    {
        if (mon.activated)
            count++;
    }
    return count;
}

void MonumentSystem::Cleanup()
{
    if (!m_initialized)
        return;

    UnloadModel(m_obeliskModel);
    m_monuments.clear();
    m_initialized = false;

    TraceLog(LOG_INFO, "✓ Monument System cleaned up");
}