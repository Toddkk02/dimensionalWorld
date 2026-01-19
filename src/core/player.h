#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "../world/firstWorld.h"      // ← CAMBIA QUI
#include "../gameplay/mining.h"   // ← AGGIUNGI QUESTO

typedef struct PlayerSystem {
    Camera3D camera;
    int cameraMode;
    Vector3 velocity;
    bool isGrounded;
    float gravity;
    bool inWater;
    MiningState mining;  // ← AGGIUNGI QUESTO CAMPO
} PlayerSystem;

void UpdatePlayerPhysics(PlayerSystem *ps, World *world, float deltaTime);

#endif