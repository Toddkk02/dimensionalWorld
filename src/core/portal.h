#ifndef PORTAL_H
#define PORTAL_H

#include "raylib.h"
#include "world/firstWorld.h"
#include <vector>

// Enum per le dimensioni
typedef enum {
    DIMENSION_PURPLE,    // Dimensione viola originale
    DIMENSION_GREEN,     // Dimensione verde con alberi
    DIMENSION_RED,       // Dimensione rossa aliena
    DIMENSION_COUNT
} DimensionType;

// Struttura per il portale
typedef struct Portal {
    Vector3 position;
    float animationTime;
    float maxRadius;
    bool active;
    DimensionType targetDimension;
    Color color;
} Portal;

// Struttura per la Portal Gun
typedef struct PortalGun {
    Model gunModel;
    Vector3 offset;  // Offset dalla camera
    float shootCooldown;
    bool hasModel;
} PortalGun;

// Sistema di gestione portali
typedef struct PortalSystem {
    std::vector<Portal> portals;
    PortalGun gun;
    DimensionType currentDimension;
    float portalCheckRadius;
} PortalSystem;

// Funzioni principali
void InitPortalSystem(PortalSystem* ps);
void UpdatePortalSystem(PortalSystem* ps, Camera3D camera, World* world, float deltaTime);
void DrawPortalGun(PortalSystem* ps, Camera3D camera);
void DrawPortals(PortalSystem* ps);
void CleanupPortalSystem(PortalSystem* ps);

// Funzione per ottenere i colori del bioma in base alla dimensione
void GetDimensionColors(DimensionType dim, Color* grassTop, Color* dirtSide, Color* dirt);

// Funzione per verificare se il player è vicino a un portale
Portal* CheckPlayerNearPortal(PortalSystem* ps, Vector3 playerPos);

#endif