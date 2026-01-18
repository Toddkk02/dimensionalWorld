#include "raylib.h"
#include "core/player.h"
#include "rendering/shaders.h"
#include "world/firstWorld.h"
#include "rendering/skybox.h"
#include "core/portal.h"
#include "world/decorations.h"
#include <cstdio>
#include "rlgl.h"
#include <cmath>

int main(void)
{
    InitWindow(1600, 900, "Dimensional World - Rick & Morty Edition");
    rlEnableDepthTest();
    SetTargetFPS(60);

    LoadTerrainShader();

    Skybox skybox = LoadSkybox("assets/textures/skybox.jpg");
    World world;
    WorldInit(&world);
    
    // Inizializza il sistema portali
    PortalSystem portalSystem;
    InitPortalSystem(&portalSystem);

    // Inizializza il sistema decorazioni
    DecorationSystem decorationSystem;
    InitDecorationSystem(&decorationSystem);

    // Genera mondo intorno all'origine
    for (int x = -2; x <= 2; x++)
    {
        for (int z = -2; z <= 2; z++)
        {
            WorldUpdate(&world, (Vector3){(float)(x * 16), 10.0f, (float)(z * 16)});
        }
    }

    // Genera decorazioni iniziali per la dimensione viola
    GenerateDecorationsForDimension(&decorationSystem, &world, DIMENSION_PURPLE);

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
    ps.inWater = false;
    DisableCursor();

    // Variabile per gestire il cambio dimensione
    bool isChangingDimension = false;
    float dimensionChangeTimer = 0.0f;
    DimensionType targetDimension = DIMENSION_PURPLE;

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // Gestione cambio dimensione
        if (isChangingDimension) {
            dimensionChangeTimer += deltaTime;
            
            if (dimensionChangeTimer > 1.0f) {
                // Cambia effettivamente la dimensione
                portalSystem.currentDimension = targetDimension;
                SetWorldDimension((int)targetDimension);
                
                // Aggiorna i colori
                Color grassTop, dirtSide, dirt;
                GetDimensionColors(targetDimension, &grassTop, &dirtSide, &dirt);
                SetDimensionColors(grassTop, dirtSide, dirt);
                
                // Rigenera il mondo
                RegenerateAllChunks(&world);
                
                // Rigenera le decorazioni per la nuova dimensione
                GenerateDecorationsForDimension(&decorationSystem, &world, targetDimension);
                
                isChangingDimension = false;
                dimensionChangeTimer = 0.0f;
            }
        } else {
            // Update normale
            UpdatePlayerPhysics(&ps, &world, deltaTime);
            UpdateCamera(&ps.camera, ps.cameraMode);
            WorldUpdate(&world, ps.camera.position);
            UpdatePortalSystem(&portalSystem, ps.camera, &world, deltaTime);
            
            // Controlla se il player è vicino a un portale
            Portal* nearPortal = CheckPlayerNearPortal(&portalSystem, ps.camera.position);
            if (nearPortal && IsKeyPressed(KEY_E)) {
                // Inizia il cambio dimensione
                isChangingDimension = true;
                targetDimension = nearPortal->targetDimension;
                nearPortal->active = false;  // Disattiva il portale usato
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);
        
        // Effetto di cambio dimensione
        if (isChangingDimension) {
            // Schermo bianco lampeggiante durante il viaggio
            float flashIntensity = sinf(dimensionChangeTimer * 20.0f) * 0.5f + 0.5f;
            ClearBackground(Fade(WHITE, flashIntensity * 0.8f));
        }
        
        DrawSkybox(skybox, ps.camera);

        BeginMode3D(ps.camera);
        
        // Disegna il mondo
        BeginShaderMode(terrainShader);
        WorldDraw(&world);
        EndShaderMode();
        
        // Disegna le decorazioni (alberi, rocce, ecc.)
        DrawDecorations(&decorationSystem);
        
        // Disegna i portali
        DrawPortals(&portalSystem);
        
        // Disegna la portal gun (sempre visibile in prima persona)
        DrawPortalGun(&portalSystem, ps.camera);

        EndMode3D();

        // UI
        DrawFPS(10, 10);
        
        char debugText[300];
        const char* dimensionNames[] = {"PURPLE", "GREEN", "RED"};
        sprintf(debugText, 
                "ESC = mouse | WASD = move | SPACE = jump\n"
                "RIGHT CLICK = Shoot Portal | E = Use Portal\n"
                "Pos: %.1f, %.1f, %.1f | Ground: %s | Water: %s\n"
                "Dimension: %s | Portals: %zu | Decorations: %zu",
                ps.camera.position.x, ps.camera.position.y, ps.camera.position.z,
                ps.isGrounded ? "YES" : "NO",
                ps.inWater ? "YES" : "NO",
                dimensionNames[portalSystem.currentDimension],
                portalSystem.portals.size(),
                decorationSystem.trees.size() + decorationSystem.rocks.size());
        DrawText(debugText, 10, 30, 20, WHITE);
        
        // Mostra messaggio se vicino a un portale
        Portal* nearPortal = CheckPlayerNearPortal(&portalSystem, ps.camera.position);
        if (nearPortal && !isChangingDimension) {
            const char* targetDimName = dimensionNames[nearPortal->targetDimension];
            char portalText[128];
            sprintf(portalText, "Press E to enter %s dimension", targetDimName);
            int textWidth = MeasureText(portalText, 30);
            DrawText(portalText, GetScreenWidth()/2 - textWidth/2, GetScreenHeight() - 100, 30, nearPortal->color);
        }
        
        // Effetto di viaggio dimensionale
        if (isChangingDimension) {
            const char* travelText = "TRAVELING THROUGH DIMENSIONS...";
            int textWidth = MeasureText(travelText, 40);
            DrawText(travelText, GetScreenWidth()/2 - textWidth/2, GetScreenHeight()/2, 40, GREEN);
        }
        
        // Mirino al centro dello schermo
        DrawCircle(GetScreenWidth()/2, GetScreenHeight()/2, 3, Fade(GREEN, 0.5f));
        DrawCircleLines(GetScreenWidth()/2, GetScreenHeight()/2, 10, Fade(GREEN, 0.3f));
        
        EndDrawing();
    }

    UnloadTerrainShader();
    UnloadSkybox(skybox);
    WorldCleanup(&world);
    CleanupPortalSystem(&portalSystem);
    CleanupDecorationSystem(&decorationSystem);
    CloseWindow();
    return 0;
}