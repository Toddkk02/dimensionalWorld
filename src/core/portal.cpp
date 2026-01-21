#include "portal.h"
#include <raymath.h>
#include <cmath>

void GetDimensionColors(DimensionConfig* dim, Color* grassTop, Color* dirtSide, Color* dirt) {
    *grassTop = dim->grassTopColor;
    *dirtSide = dim->dirtSideColor;
    *dirt = dim->dirtColor;
}

void InitPortalSystem(PortalSystem* ps) {
    ps->portals.clear();
    ps->currentDimensionID = 0;
    ps->portalCheckRadius = 3.0f;
    
    ps->gun.offset = (Vector3){0.5f, -0.3f, 0.8f};
    ps->gun.shootCooldown = 0.0f;
    ps->gun.hasModel = false;
    
    if (FileExists("assets/models/portalgun.obj") || FileExists("assets/models/portalgun.glb")) {
        const char* path = FileExists("assets/models/portalgun.obj") ? 
                          "assets/models/portalgun.obj" : "assets/models/portalgun.glb";
        ps->gun.gunModel = LoadModel(path);
        ps->gun.hasModel = true;
    } else {
        Mesh gunMesh = GenMeshCylinder(0.05f, 0.3f, 8);
        ps->gun.gunModel = LoadModelFromMesh(gunMesh);
        ps->gun.hasModel = true;
    }
}

void ShootPortal(PortalSystem* ps, Camera3D camera, World* world, DimensionManager* dimManager) {
    Ray ray;
    ray.position = camera.position;
    ray.direction = Vector3Subtract(camera.target, camera.position);
    ray.direction = Vector3Normalize(ray.direction);
    
    float maxDistance = 100.0f;
    float step = 0.5f;
    Vector3 hitPoint = {0.0f, 0.0f, 0.0f};
    bool foundHit = false;
    
    for (float dist = 0; dist < maxDistance; dist += step) {
        Vector3 testPoint = Vector3Add(ray.position, Vector3Scale(ray.direction, dist));
        float terrainHeight = GetTerrainHeightAt(world, testPoint.x, testPoint.z);
        
        if (testPoint.y <= terrainHeight) {
            hitPoint = testPoint;
            hitPoint.y = terrainHeight;
            foundHit = true;
            break;
        }
    }
    
    if (foundHit) {
        Portal portal;
        portal.position = hitPoint;
        portal.animationTime = 0.0f;
        portal.maxRadius = 2.0f;
        portal.active = true;
        
        int totalDimensions = dimManager->GetDimensionCount();
        portal.targetDimensionID = (ps->currentDimensionID + 1) % totalDimensions;
        
        DimensionConfig* targetDim = dimManager->GetDimension(portal.targetDimensionID);
        portal.color = targetDim->grassTopColor;
        
        ps->portals.push_back(portal);
        
        if (ps->portals.size() > 3) {
            ps->portals.erase(ps->portals.begin());
        }
    }
}

void UpdatePortalSystem(PortalSystem* ps, Camera3D camera, World* world, DimensionManager* dimManager, float deltaTime) {
    if (ps->gun.shootCooldown > 0) {
        ps->gun.shootCooldown -= deltaTime;
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) && ps->gun.shootCooldown <= 0) {
        ShootPortal(ps, camera, world, dimManager);
        ps->gun.shootCooldown = 0.5f;
    }
    
    for (size_t i = 0; i < ps->portals.size(); i++) {
        if (ps->portals[i].active) {
            ps->portals[i].animationTime += deltaTime * 3.0f;
            if (ps->portals[i].animationTime > 1.0f) {
                ps->portals[i].animationTime = 1.0f;
            }
        }
    }
}

void DrawPortalGun(PortalSystem* ps, Camera3D camera) {
    if (!ps->gun.hasModel) return;
    
    Vector3 forward = Vector3Subtract(camera.target, camera.position);
    forward = Vector3Normalize(forward);
    
    Vector3 right = Vector3CrossProduct(forward, camera.up);
    right = Vector3Normalize(right);
    
    Vector3 gunPos = camera.position;
    gunPos = Vector3Add(gunPos, Vector3Scale(right, ps->gun.offset.x));
    gunPos = Vector3Add(gunPos, Vector3Scale(camera.up, ps->gun.offset.y));
    gunPos = Vector3Add(gunPos, Vector3Scale(forward, ps->gun.offset.z));
    
    float angle = atan2f(forward.x, forward.z) * RAD2DEG;
    
    DrawModelEx(ps->gun.gunModel, gunPos, 
                (Vector3){0, 1, 0}, angle, 
                (Vector3){0.5f, 0.5f, 0.5f}, 
                (Color){100, 255, 100, 255});
}

void DrawPortals(PortalSystem* ps) {
    for (size_t i = 0; i < ps->portals.size(); i++) {
        Portal* p = &ps->portals[i];
        if (!p->active) continue;
        
        float currentRadius = p->maxRadius * p->animationTime;
        
        int rings = 5;
        for (int r = 0; r < rings; r++) {
            float ringRadius = currentRadius * (1.0f - (float)r / rings);
            float alpha = 255.0f * (1.0f - (float)r / rings);
            
            float rotation = p->animationTime * 360.0f + r * 72.0f;
            
            Color ringColor = p->color;
            ringColor.a = (unsigned char)alpha;
            
            DrawCircle3D(p->position, ringRadius, 
                        (Vector3){1, 0, 0}, 90.0f, ringColor);
            
            DrawCircle3D(p->position, ringRadius * 0.8f, 
                        (Vector3){0, 1, 0}, rotation, 
                        Fade(ringColor, 0.3f));
        }
        
        DrawSphere(p->position, 0.2f, Fade(p->color, 0.8f));
        
        int particleCount = 8;
        for (int j = 0; j < particleCount; j++) {
            float angle = (p->animationTime * 360.0f + j * 360.0f / particleCount) * DEG2RAD;
            float px = p->position.x + cosf(angle) * currentRadius * 0.7f;
            float pz = p->position.z + sinf(angle) * currentRadius * 0.7f;
            float py = p->position.y + sinf(p->animationTime * 10.0f + j) * 0.5f;
            
            DrawSphere((Vector3){px, py, pz}, 0.1f, Fade(p->color, 0.6f));
        }
    }
}

Portal* CheckPlayerNearPortal(PortalSystem* ps, Vector3 playerPos) {
    for (size_t i = 0; i < ps->portals.size(); i++) {
        Portal* p = &ps->portals[i];
        if (!p->active) continue;
        
        float distance = Vector3Distance(playerPos, p->position);
        if (distance < ps->portalCheckRadius) {
            return p;
        }
    }
    return NULL;
}

void CleanupPortalSystem(PortalSystem* ps) {
    if (ps->gun.hasModel) {
        UnloadModel(ps->gun.gunModel);
    }
    ps->portals.clear();
}