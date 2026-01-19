#ifndef PORTAL_H
#define PORTAL_H

#include "raylib.h"
#include "../world/firstWorld.h"      // ← CAMBIA QUI
#include "../world/dimensions.h"
#include <vector>

typedef struct Portal {
    Vector3 position;
    float animationTime;
    float maxRadius;
    bool active;
    int targetDimensionID;
    Color color;
} Portal;

typedef struct PortalGun {
    Model gunModel;
    Vector3 offset;
    float shootCooldown;
    bool hasModel;
} PortalGun;

typedef struct PortalSystem {
    std::vector<Portal> portals;
    PortalGun gun;
    int currentDimensionID;
    float portalCheckRadius;
} PortalSystem;

void InitPortalSystem(PortalSystem* ps);
void UpdatePortalSystem(PortalSystem* ps, Camera3D camera, World* world, DimensionManager* dimManager, float deltaTime);
void DrawPortalGun(PortalSystem* ps, Camera3D camera);
void DrawPortals(PortalSystem* ps);
void CleanupPortalSystem(PortalSystem* ps);

void GetDimensionColors(DimensionConfig* dim, Color* grassTop, Color* dirtSide, Color* dirt);
Portal* CheckPlayerNearPortal(PortalSystem* ps, Vector3 playerPos);

#endif