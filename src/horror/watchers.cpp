#include "watchers.h"
#include "../core/cosmicState.h"
#include <raymath.h>
#include <cmath>
#include <stdlib.h>

WatcherSystem::WatcherSystem() : 
    m_activeWatchers(0), 
    m_spawnTimer(0),
    m_initialized(false) {
}

WatcherSystem::~WatcherSystem() {
    if (m_initialized) {
        Cleanup();
    }
}

void WatcherSystem::Init() {
    m_watchers.clear();
    m_activeWatchers = 0;
    m_spawnTimer = 0;
    
    // PROVA A CARICARE MODELLO 3D CUSTOM
    const char* watcherModelPaths[] = {
    "assets/models/eye-tooth/EyeTooth_Print.glb",  // ← NOME ESATTO
    "assets/models/eye-tooth/scene.glb",
    "assets/models/eye-tooth/scene.gltf",
    "assets/models/watcher.obj"
};
    
    bool modelLoaded = false;
    for (const char* path : watcherModelPaths) {
        if (FileExists(path)) {
            m_watcherModel = LoadModel(path);
            if (m_watcherModel.meshCount > 0) {
                modelLoaded = true;
                TraceLog(LOG_INFO, "✓ Watcher model loaded: %s", path);
                break;
            }
        }
    }
    
    // FALLBACK A SFERA SE NON TROVA MODELLO
    if (!modelLoaded) {
        Mesh sphere = GenMeshSphere(1.0f, 16, 16);
        m_watcherModel = LoadModelFromMesh(sphere);
        TraceLog(LOG_INFO, "✓ Watcher using procedural sphere (no custom model found)");
    }
    
    m_initialized = true;
    TraceLog(LOG_INFO, "✓ Watcher System initialized");
}

void WatcherSystem::SpawnWatcher(Vector3 playerPos, float tension) {
    Watcher w;
    w.id = m_watchers.size();
    
    float angle = (rand() % 360) * DEG2RAD;
    float baseDistance = 35.0f;
    float variationDistance = 15.0f;
    float distance = baseDistance + (rand() % (int)variationDistance);
    
    w.position = (Vector3){
        playerPos.x + cosf(angle) * distance,
        playerPos.y + 2.0f,
        playerPos.z + sinf(angle) * distance
    };
    
    w.targetPosition = w.position;
    w.opacity = 0.0f;
    w.size = 1.5f + (tension / 100.0f) * 1.0f;
    w.moveTimer = 0.0f;
    w.stareIntensity = 0.0f;
    w.isVisible = false;
    w.playerLooking = false;
    w.distanceToPlayer = distance;
    
    m_watchers.push_back(w);
    TraceLog(LOG_WARNING, "👁️ Watcher #%d spawned at distance %.1f (Tension: %.1f)", 
             w.id, distance, tension);
}

void WatcherSystem::Update(Camera3D camera, float tension, float deltaTime) {
    m_spawnTimer += deltaTime;
    m_activeWatchers = 0;
    
    float spawnInterval = 120.0f;
    if (tension > 40.0f) spawnInterval = 60.0f;
    if (tension > 60.0f) spawnInterval = 30.0f;
    if (tension > 80.0f) spawnInterval = 15.0f;
    
    int maxWatchers = 1;
    if (tension > 40.0f) maxWatchers = 2;
    if (tension > 60.0f) maxWatchers = 3;
    if (tension > 80.0f) maxWatchers = 5;
    
    if (m_spawnTimer > spawnInterval && tension > 25.0f) {
        if ((int)m_watchers.size() < maxWatchers) {
            SpawnWatcher(camera.position, tension);
            m_spawnTimer = 0.0f;
        } else {
            m_spawnTimer = spawnInterval - 5.0f;
        }
    }
    
    for (auto it = m_watchers.begin(); it != m_watchers.end(); ) {
        UpdateWatcher(*it, camera, deltaTime);
        
        if (it->isVisible) {
            m_activeWatchers++;
        }
        
        float dist = Vector3Distance(it->position, camera.position);
        if (dist > 150.0f || (!it->isVisible && it->moveTimer > 45.0f)) {
            TraceLog(LOG_INFO, "Removing watcher #%d (dist: %.1f, timer: %.1f)", 
                     it->id, dist, it->moveTimer);
            it = m_watchers.erase(it);
        } else {
            ++it;
        }
    }
}

void WatcherSystem::UpdateWatcher(Watcher& watcher, Camera3D camera, float deltaTime) {
    watcher.moveTimer += deltaTime;
    
    watcher.distanceToPlayer = Vector3Distance(watcher.position, camera.position);
    
    watcher.playerLooking = IsInPlayerView(watcher.position, camera);
    
    if (watcher.playerLooking) {
        watcher.opacity += deltaTime * 2.0f;
        watcher.stareIntensity += deltaTime;
        
        if (watcher.opacity > 0.3f && watcher.stareIntensity > 1.5f) {
            CosmicState::Get().OnWatcherSeen();
            watcher.stareIntensity = 0.0f;
        }
    } else {
        watcher.opacity -= deltaTime * 0.5f;
        watcher.stareIntensity = 0.0f;
        
        if (watcher.distanceToPlayer > 15.0f) {
            Vector3 dirToPlayer = Vector3Subtract(camera.position, watcher.position);
            float dist = Vector3Length(dirToPlayer);
            
            if (dist > 0.1f) {
                dirToPlayer = Vector3Normalize(dirToPlayer);
                float moveSpeed = 2.0f;
                watcher.position = Vector3Add(
                    watcher.position, 
                    Vector3Scale(dirToPlayer, deltaTime * moveSpeed)
                );
            }
        } else {
            Vector3 right = Vector3CrossProduct(
                Vector3Subtract(camera.target, camera.position),
                camera.up
            );
            right = Vector3Normalize(right);
            
            float circleSpeed = 0.5f;
            watcher.position = Vector3Add(
                watcher.position,
                Vector3Scale(right, deltaTime * circleSpeed)
            );
        }
    }
    
    if (watcher.opacity < 0.0f) watcher.opacity = 0.0f;
    if (watcher.opacity > 0.8f) watcher.opacity = 0.8f;
    
    watcher.isVisible = watcher.opacity > 0.05f;
}

bool WatcherSystem::IsInPlayerView(Vector3 pos, Camera3D camera) {
    Vector3 toWatcher = Vector3Subtract(pos, camera.position);
    float dist = Vector3Length(toWatcher);
    
    if (dist < 0.1f) return false;
    
    toWatcher = Vector3Normalize(toWatcher);
    
    Vector3 forward = Vector3Subtract(camera.target, camera.position);
    forward = Vector3Normalize(forward);
    
    float dot = Vector3DotProduct(forward, toWatcher);
    
    return dot > 0.3f;
}

void WatcherSystem::Draw(Camera3D camera) {
    if (!m_initialized) return;
    
    for (const auto& watcher : m_watchers) {
        if (!watcher.isVisible || watcher.opacity < 0.05f) continue;
        
        // Disegna modello custom se disponibile
        if (m_watcherModel.meshCount > 0) {
            // Calcola rotazione verso player
            Vector3 toPlayer = Vector3Subtract(camera.position, watcher.position);
            float angle = atan2f(toPlayer.x, toPlayer.z) * RAD2DEG;
            
            // Disegna modello con fade
            Color tint = Fade(BLACK, watcher.opacity);
            DrawModelEx(
                m_watcherModel,
                watcher.position,
                (Vector3){0, 1, 0},
                angle,
                (Vector3){watcher.size, watcher.size, watcher.size},
                tint
            );
        } else {
            // FALLBACK: sfera nera
            Color bodyColor = Fade(BLACK, watcher.opacity);
            DrawSphere(watcher.position, watcher.size, bodyColor);
        }
        
        // OCCHI (sempre visibili)
        if (watcher.opacity > 0.2f) {
            Vector3 eyeLeft = watcher.position;
            eyeLeft.x -= 0.3f * watcher.size;
            eyeLeft.y += 0.2f * watcher.size;
            
            Vector3 eyeRight = watcher.position;
            eyeRight.x += 0.3f * watcher.size;
            eyeRight.y += 0.2f * watcher.size;
            
            float eyeGlow = watcher.playerLooking ? 1.0f : 0.5f;
            float eyeSize = 0.15f * watcher.size;
            
            DrawSphere(eyeLeft, eyeSize, Fade(WHITE, watcher.opacity * eyeGlow));
            DrawSphere(eyeRight, eyeSize, Fade(WHITE, watcher.opacity * eyeGlow));
            
            if (watcher.playerLooking && watcher.stareIntensity > 0.5f) {
                DrawSphere(eyeLeft, eyeSize * 1.3f, Fade(RED, watcher.opacity * 0.3f));
                DrawSphere(eyeRight, eyeSize * 1.3f, Fade(RED, watcher.opacity * 0.3f));
            }
        }
    }
}

void WatcherSystem::Cleanup() {
    if (!m_initialized) return;
    
    if (m_watcherModel.meshCount > 0) {
        UnloadModel(m_watcherModel);
    }
    
    m_watchers.clear();
    m_initialized = false;
    
    TraceLog(LOG_INFO, "✓ Watcher System cleaned up");
}

Vector3 WatcherSystem::GetBehindPlayerPosition(Camera3D camera, float distance) {
    Vector3 forward = Vector3Subtract(camera.target, camera.position);
    forward = Vector3Normalize(forward);
    
    Vector3 behind = Vector3Scale(forward, -distance);
    return Vector3Add(camera.position, behind);
}