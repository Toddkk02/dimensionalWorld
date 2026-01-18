#include "raylib.h"
#include "core/player.h"
#include "rendering/shaders.h"
#include "world/firstWorld.h"
#include "rendering/skybox.h"
#include <cstdio>
#include "rlgl.h"
int main(void) {
    InitWindow(800, 600, "Dimensional World");
    rlEnableDepthTest();
    SetTargetFPS(60);
    
    LoadTerrainShader();
    
    Skybox skybox = LoadSkybox("assets/textures/skybox.jpg");
    World world;
    WorldInit(&world);
    
    // Genera mondo intorno all'origine
    for (int x = -2; x <= 2; x++) {
        for (int z = -2; z <= 2; z++) {
            WorldUpdate(&world, (Vector3){(float)(x*16), 10.0f, (float)(z*16)});
        }
    }
    
    // Spawn sicuro dall'alto
    PlayerSystem ps;
    ps.camera.position = (Vector3){8.0f, 50.0f, 8.0f};
    ps.camera.target = (Vector3){8.0f, 49.0f, 9.0f};
    ps.camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    ps.camera.fovy = 60.0f;
    ps.camera.projection = CAMERA_PERSPECTIVE;
    ps.cameraMode = CAMERA_FIRST_PERSON;
    ps.velocity = (Vector3){0.0f, 0.0f, 0.0f};
    ps.isGrounded = false;
    ps.gravity = -30.0f;
    DisableCursor();
    
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        UpdatePlayerPhysics(&ps, &world, deltaTime);
        UpdateCamera(&ps.camera, ps.cameraMode);
        WorldUpdate(&world, ps.camera.position);

        BeginDrawing();
        ClearBackground(BLACK);
        DrawSkybox(skybox, ps.camera);
        
        BeginMode3D(ps.camera);
        BeginShaderMode(terrainShader);
        WorldDraw(&world);
        EndShaderMode();
        
        EndMode3D();
        
        DrawFPS(10, 10);
        char debugText[200];
        sprintf(debugText, "ESC = mouse | WASD = move | SPACE = jump\nPos: %.1f, %.1f, %.1f | Ground: %s", 
                ps.camera.position.x, ps.camera.position.y, ps.camera.position.z,
                ps.isGrounded ? "YES" : "NO");
        DrawText(debugText, 10, 30, 20, WHITE);
        EndDrawing();
    }    
    
    UnloadTerrainShader(); 
    UnloadSkybox(skybox);
    CloseWindow();
    return 0;
}