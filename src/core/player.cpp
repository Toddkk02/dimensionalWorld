#include "player.h"
#include <raylib.h>

void InitPlayerSystem(PlayerSystem *ps) {
    // Camera setup
    ps->camera.position = (Vector3){0.0f, 2.0f, 4.0f};
    ps->camera.target   = (Vector3){0.0f, 2.0f, 0.0f};
    ps->camera.up       = (Vector3){0.0f, 1.0f, 0.0f};
    ps->camera.fovy     = 60.0f;
    ps->camera.projection = CAMERA_PERSPECTIVE;
    DisableCursor();
    ps->cameraMode = CAMERA_FIRST_PERSON;

    // Genera colonne random
    for (int i = 0; i < MAX_COLUMNS; i++) {
        ps->heights[i] = (float)GetRandomValue(1, 12);
        ps->playerPositions[i] = (Vector3){
            (float)GetRandomValue(-15, 15),
            ps->heights[i] / 2.0f,
            (float)GetRandomValue(-15, 15)
        };
        ps->colors[i] = (Color){
           (unsigned char)GetRandomValue(20, 255),
            (unsigned char)GetRandomValue(10, 55),
            30,
            255
        };
    }
}
