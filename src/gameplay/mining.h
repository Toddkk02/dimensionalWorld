#ifndef MINING_H
#define MINING_H

#include "raylib.h"

// Forward declaration
struct World;

struct MiningState {
    bool mining = false;
    float progress = 0.0f;
    Vector3 targetBlock = {0,0,0};
};

// Ritorna true se colpisce blocco
bool RaycastBlock(Camera3D cam, World* world, Vector3& hitPos);

// Aggiorna mining
void UpdateMining(MiningState& mining, Camera3D cam, World* world, float dt);
// Add this after RaycastBlock declaration in mining.h:

bool RaycastPlaceBlock(Camera3D cam, World* world, Vector3& placePos);
#endif