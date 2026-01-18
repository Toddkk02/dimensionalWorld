#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

#define MAX_COLUMNS 20

typedef struct PlayerSystem {
    Camera3D camera;
    int cameraMode;
    Vector3 playerPositions[MAX_COLUMNS];
    float heights[MAX_COLUMNS];
    Color colors[MAX_COLUMNS];
} PlayerSystem;

// Inizializza player, camera e colonne
void InitPlayerSystem(PlayerSystem *ps);

#endif
