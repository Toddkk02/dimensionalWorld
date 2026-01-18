#include "raylib.h"
#include "core/player.h"
#include "world/firstWorld.h"
#include "rendering/skybox.h"

int main(void) {
    InitWindow(800, 600, "Dimensional World");
    SetTargetFPS(60);
    
    PlayerSystem ps;
    InitPlayerSystem(&ps);
    
    Skybox skybox = LoadSkybox("assets/textures/skybox.jpg");
    
    while (!WindowShouldClose()) {
    UpdateCamera(&ps.camera, ps.cameraMode);
    
    BeginDrawing();
    ClearBackground(BLACK);
    
    DrawSkybox(skybox, ps.camera);  // ORA PRIMA di BeginMode3D!
    
    BeginMode3D(ps.camera);
    
    // DrawSkybox(skybox, ps.camera); // RIMUOVI da qui
    
    firstWorldGenerated();
    DrawGrid(10, 1.0f);
    
    for (int i = 0; i < MAX_COLUMNS; i++) {
        DrawCube(ps.playerPositions[i], 1.0f, ps.heights[i], 1.0f, ps.colors[i]);
    }
    
    EndMode3D();
    
    DrawFPS(10, 10);
    DrawText("ESC = libera mouse | WASD = movimento", 10, 30, 20, WHITE);
    
    EndDrawing();
}    
    UnloadSkybox(skybox);
    CloseWindow();
    return 0;
}
