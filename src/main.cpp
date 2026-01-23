#include "raylib.h"
#include "raymath.h"
#include "core/player.h"
#include "rendering/shaders.h"
#include "world/firstWorld.h"
#include "world/dimensions.h"
#include "world/blocks.h"
#include "rendering/skybox.h"
#include "core/portal.h"
#include "world/decorations.h"
#include "gameplay/dropped_item.h"
#include "gameplay/inventory.h"
#include "gameplay/mining.h"
#include "world/worldRenderer.h"
#include "core/cosmicState.h"
#include "horror/watchers.h"
#include "horror/audioManager.h"
#include "world/monuments.h"
#include <cstdio>
#include <cmath>
#include "rlgl.h"

void DrawMiningProgress(MiningState &mining)
{
    if (!mining.mining)
        return;
    Vector3 blockCenter = mining.targetBlock;
    blockCenter.x += 0.5f;
    blockCenter.y += 0.5f;
    blockCenter.z += 0.5f;
    float pulseScale = 1.0f + sinf(mining.progress * 10.0f) * 0.1f;
    DrawCubeWires(blockCenter, pulseScale, pulseScale, pulseScale, RED);

    int crackLevel = (int)(mining.progress / 0.5f);
    if (crackLevel > 3)
        crackLevel = 3;
    for (int i = 0; i <= crackLevel; i++)
    {
        float offset = 0.1f * i;
        DrawCubeWires(blockCenter, 1.0f + offset, 1.0f + offset, 1.0f + offset, Fade(YELLOW, 0.5f));
    }
}

int main()
{
    const int screenWidth = 1600, screenHeight = 900;
    InitWindow(screenWidth, screenHeight, "Dimensional World - Cosmic Horror Edition");
    SetTargetFPS(60);
    rlEnableDepthTest();

    TraceLog(LOG_INFO, "========================================");
    TraceLog(LOG_INFO, "   DIMENSIONAL WORLD - STARTING UP");
    TraceLog(LOG_INFO, "========================================");

    // ========== COSMIC STATE ==========
    CosmicState::Get();
    TraceLog(LOG_INFO, "✓ CosmicState initialized");

    // ========== DIMENSION MANAGER ==========
    DimensionManager dimensionManager;
    dimensionManager.Initialize();
    DimensionConfig *currentDim = dimensionManager.GetCurrentDimension();

    if (!currentDim)
    {
        TraceLog(LOG_ERROR, "✗ Failed to get current dimension!");
        CloseWindow();
        return -1;
    }

    TraceLog(LOG_INFO, "✓ Dimension Manager initialized with %d dimensions",
             dimensionManager.GetDimensionCount());

    // ========== LOAD DIMENSION TEXTURES ==========
    TraceLog(LOG_INFO, "Loading textures for dimension: %s", currentDim->name.c_str());
    dimensionManager.LoadDimensionTextures(currentDim);

    // ========== SHADER SYSTEM ==========
    LoadTerrainShader();

    // ========== LOAD CHROMATIC SHADER ==========
    Shader chromaticShader = {0};
    int aberrationLoc = -1;

    chromaticShader = LoadShader("assets/shaders/glsl330/chromatic_vertex.vs",
                                 "assets/shaders/glsl330/chromatic.fs");

    if (chromaticShader.id > 0)
    {
        aberrationLoc = GetShaderLocation(chromaticShader, "aberration");
        chromaticShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(chromaticShader, "mvp");
        TraceLog(LOG_INFO, "✓ Chromatic aberration shader loaded (ID: %d)", chromaticShader.id);
    }
    else
    {
        TraceLog(LOG_WARNING, "✗ Chromatic shader not loaded");
    }

    // ========== RENDER TEXTURE FOR POST-PROCESSING ==========
    RenderTexture2D screenTarget = LoadRenderTexture(screenWidth, screenHeight);
    TraceLog(LOG_INFO, "✓ Render target created");

    // ========== WORLD ==========
    World world;
    WorldInit(&world);
    SetWorldDimension(currentDim->terrainSeed);
    SetDimensionColors(currentDim->grassTopColor, currentDim->dirtSideColor, currentDim->dirtColor);
    WorldLoadTextures(&world, currentDim);
    TraceLog(LOG_INFO, "✓ World initialized");

    // ========== WORLD RENDERER (WITH INTEGRATED FOG) ==========
    WorldRenderer worldRenderer;
    InitWorldRenderer(&worldRenderer, currentDim);
    TraceLog(LOG_INFO, "✓ World Renderer initialized with fog shader");

    // ========== PLAYER ==========
    PlayerSystem ps = {};
    ps.camera.position = (Vector3){8.0f, 50.0f, 8.0f};
    ps.camera.target = (Vector3){8.0f, 49.0f, 9.0f};
    ps.camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    ps.camera.fovy = 60.0f;
    ps.camera.projection = CAMERA_PERSPECTIVE;
    ps.cameraMode = CAMERA_FIRST_PERSON;
    ps.velocity = (Vector3){0, 0, 0};
    ps.isGrounded = false;
    ps.gravity = -30.0f;
    ps.inWater = false;
    ps.mining.mining = false;
    ps.mining.progress = 0.0f;
    DisableCursor();
    TraceLog(LOG_INFO, "✓ Player system initialized");

    // ========== SKYBOX ==========
    Skybox skybox = LoadSkyboxFromDimension(currentDim);
    TraceLog(LOG_INFO, "✓ Skybox loaded");

    // ========== DECORATION SYSTEM ==========
    DecorationSystem decorationSystem;
    InitDecorationSystem(&decorationSystem);
    GenerateDecorationsForDimension(&decorationSystem, &world, currentDim);
    TraceLog(LOG_INFO, "✓ Decorations generated (Trees:%d Rocks:%d Crystals:%d)",
             (int)decorationSystem.trees.size(),
             (int)decorationSystem.rocks.size(),
             (int)decorationSystem.crystals.size());

    // ========== PORTAL SYSTEM ==========
    PortalSystem portalSystem;
    InitPortalSystem(&portalSystem);
    portalSystem.currentDimensionID = currentDim->id;
    TraceLog(LOG_INFO, "✓ Portal system initialized");

    // ========== INVENTORY ==========
    Inventory playerInventory;
    playerInventory.AddItem(ItemType::DIRT, 64);
    playerInventory.AddItem(ItemType::GRASS, 64);
    playerInventory.AddItem(ItemType::STONE, 64);
    TraceLog(LOG_INFO, "✓ Inventory initialized");

    // ========== HORROR SYSTEMS ==========
    TraceLog(LOG_INFO, "========================================");
    TraceLog(LOG_INFO, "   INITIALIZING HORROR SYSTEMS");
    TraceLog(LOG_INFO, "========================================");

    AudioManager::Get().Init();
    AudioManager::Get().SetMasterVolume(0.7f);

    WatcherSystem watcherSystem;
    watcherSystem.Init();

    MonumentSystem monumentSystem;
    monumentSystem.Init();
    monumentSystem.GenerateMonuments(ps.camera.position, 5);

    TraceLog(LOG_INFO, "✓ All horror systems initialized");

    // ========== GAME STATE ==========
    bool inventoryOpen = false;
    bool isChangingDimension = false;
    float dimensionChangeTimer = 0.0f;
    int targetDimensionID = 0;

    TraceLog(LOG_INFO, "========================================");
    TraceLog(LOG_INFO, "   ENTERING MAIN GAME LOOP");
    TraceLog(LOG_INFO, "========================================");

    // ========== MAIN LOOP ==========
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // ========== UPDATE COSMIC STATE ==========
        CosmicState::Get().Update(deltaTime);
        float tension = CosmicState::Get().GetTension();

        // ========== UPDATE HORROR SYSTEMS ==========
        AudioManager::Get().Update(tension, deltaTime);
        watcherSystem.Update(ps.camera, tension, deltaTime);
        monumentSystem.Update(ps.camera.position, deltaTime);

        // ========== CALCULATE FOG PARAMETERS ==========
        float fogDensity = 0.01f + (tension * 0.0024f);

        Color fogColor;
        if (tension < 20.0f)
        {
            float intensity = 200.0f - (tension * 2.0f);
            fogColor = (Color){
                (unsigned char)intensity,
                (unsigned char)intensity,
                (unsigned char)intensity,
                255};
        }
        else if (tension < 50.0f)
        {
            float t = (tension - 20.0f) / 30.0f;
            fogColor.r = (unsigned char)(160 - t * 90);
            fogColor.g = (unsigned char)(160 - t * 130);
            fogColor.b = (unsigned char)(160 + t * 75);
            fogColor.a = 255;
        }
        else if (tension < 80.0f)
        {
            float t = (tension - 50.0f) / 30.0f;
            fogColor.r = (unsigned char)(70 - t * 50);
            fogColor.g = (unsigned char)(30 - t * 20);
            fogColor.b = (unsigned char)(235 - t * 135);
            fogColor.a = 255;
        }
        else
        {
            float t = (tension - 80.0f) / 20.0f;
            if (t > 1.0f)
                t = 1.0f;
            fogColor.r = (unsigned char)(20 - t * 10);
            fogColor.g = (unsigned char)(10 - t * 5);
            fogColor.b = (unsigned char)(100 - t * 50);
            fogColor.a = 255;
        }

        // ========== CHROMATIC ABERRATION AMOUNT ==========
        // Per un effetto VERAMENTE nauseante:
        float chromaticAmount = 0.0f;

        // Base sempre presente
        chromaticAmount = 0.005f;

        // Scala esponenziale con la tensione
        chromaticAmount += (tension / 100.0f) * 0.15f; // Max +0.15 a tension 100

        // Pulsazione cardiaca (aumenta con tensione)
        float heartbeatSpeed = 2.0f + (tension / 50.0f) * 4.0f; // 2Hz -> 6Hz
        float heartbeat = sinf(GetTime() * heartbeatSpeed) * 0.5f + 0.5f;
        chromaticAmount += heartbeat * (tension / 100.0f) * 0.08f;

        // Glitch randomico ad alta tensione
        if (tension > 70.0f)
        {
            if ((int)(GetTime() * 10.0f) % 10 == 0)
            { // 10% del tempo
                chromaticAmount += 0.05f;
            }
        }

        // Cap massimo (opzionale, per evitare crash GPU)
        if (chromaticAmount > 0.25f)
            chromaticAmount = 0.25f;

        // ========== INPUT ==========
        if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_E))
            inventoryOpen = !inventoryOpen;

        for (int i = 0; i < HOTBAR_SIZE; i++)
            if (IsKeyPressed(KEY_ONE + i))
                playerInventory.SelectSlot(i);

        // ========== FOG DEBUG (PRESS F) ==========
        if (IsKeyPressed(KEY_F))
        {
            TraceLog(LOG_INFO, "========== FOG DEBUG ==========");
            TraceLog(LOG_INFO, "  Fog Density: %.4f", fogDensity);
            TraceLog(LOG_INFO, "  Fog Color: (%d,%d,%d)", fogColor.r, fogColor.g, fogColor.b);
            TraceLog(LOG_INFO, "  Shader ID: %d", worldRenderer.fogShader.id);
            TraceLog(LOG_INFO, "  fogDensityLoc: %d", worldRenderer.fogDensityLoc);
            TraceLog(LOG_INFO, "  fogColorLoc: %d", worldRenderer.fogColorLoc);
            TraceLog(LOG_INFO, "  viewPosLoc: %d", worldRenderer.viewPosLoc);
            TraceLog(LOG_INFO, "  Tension: %.1f", tension);
            TraceLog(LOG_INFO, "  Camera Pos: (%.1f, %.1f, %.1f)",
                     ps.camera.position.x, ps.camera.position.y, ps.camera.position.z);
            TraceLog(LOG_INFO, "===============================");
        }

        if (!inventoryOpen && !isChangingDimension)
        {
            // ========== WORLD UPDATE ==========
            WorldUpdate(&world, ps.camera.position);
            UpdatePlayerPhysics(&ps, &world, deltaTime);
            UpdateCamera(&ps.camera, ps.cameraMode);
            if (IsKeyPressed(KEY_K))
            {
                Vector3 forward = Vector3Subtract(ps.camera.target, ps.camera.position);
                forward = Vector3Normalize(forward);
                Vector3 spawnPos = Vector3Add(ps.camera.position, Vector3Scale(forward, 15.0f));
                spawnPos.y = ps.camera.position.y;

                watcherSystem.SpawnWatcher(spawnPos, 50.0f);
                TraceLog(LOG_INFO, "👁️ DEBUG: EyeTooth watcher spawned 15m ahead!");
            }
            // ========== MINING ==========
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                UpdateMining(ps.mining, ps.camera, &world, deltaTime);

                if (ps.mining.mining && ps.mining.progress >= 2.0f)
                {
                    int bx = (int)ps.mining.targetBlock.x;
                    int by = (int)ps.mining.targetBlock.y;
                    int bz = (int)ps.mining.targetBlock.z;

                    ItemType droppedItem = RemoveBlock(&world, bx, by, bz);

                    if (droppedItem != ItemType::NONE)
                    {
                        Vector3 dropPos = {
                            ps.mining.targetBlock.x + 0.5f,
                            ps.mining.targetBlock.y + 1.0f,
                            ps.mining.targetBlock.z + 0.5f};
                        SpawnDroppedItem(droppedItem, dropPos);
                    }

                    ps.mining.mining = false;
                    ps.mining.progress = 0.0f;
                }
            }
            else
            {
                ps.mining.mining = false;
                ps.mining.progress = 0.0f;
            }

            // ========== BLOCK PLACEMENT ==========
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
            {
                Item selectedItem = playerInventory.GetSelected();

                if (selectedItem.type != ItemType::NONE && selectedItem.quantity > 0)
                {
                    Vector3 placePos;
                    if (RaycastPlaceBlock(ps.camera, &world, placePos))
                    {
                        int px = (int)placePos.x;
                        int py = (int)placePos.y;
                        int pz = (int)placePos.z;

                        if (PlaceBlock(&world, px, py, pz, selectedItem.type))
                        {
                            playerInventory.RemoveSelected(1);
                        }
                    }
                }
            }

            // ========== PORTALS ==========
            UpdatePortalSystem(&portalSystem, ps.camera, &world, &dimensionManager, deltaTime);

            Portal *nearPortal = CheckPlayerNearPortal(&portalSystem, ps.camera.position);
            if (nearPortal && !isChangingDimension)
            {
                isChangingDimension = true;
                targetDimensionID = nearPortal->targetDimensionID;
                dimensionChangeTimer = 0.0f;
                CosmicState::Get().OnPortalCrossed();
                TraceLog(LOG_INFO, ">>> PORTAL ENTERED! Traveling to dimension %d <<<", targetDimensionID);
            }

            // ========== DROPPED ITEMS ==========
            UpdateDroppedItems(&world, &playerInventory, ps.camera.position, deltaTime);
        }

        // ========== DIMENSION CHANGE ==========
        if (isChangingDimension)
        {
            dimensionChangeTimer += deltaTime;

            if (dimensionChangeTimer > 0.5f && dimensionChangeTimer < 0.6f)
            {
                TraceLog(LOG_INFO, "========================================");
                TraceLog(LOG_INFO, "   DIMENSION CHANGE IN PROGRESS");
                TraceLog(LOG_INFO, "========================================");

                // Cleanup old dimension
                UnloadSkybox(skybox);
                CleanupDecorationSystem(&decorationSystem);
                UnloadWorldRenderer(&worldRenderer);
                WorldCleanup(&world);
                dimensionManager.UnloadDimensionTextures(currentDim);

                // Load new dimension
                currentDim = dimensionManager.GetDimension(targetDimensionID);

                if (!currentDim)
                {
                    TraceLog(LOG_ERROR, "✗ Failed to get dimension %d!", targetDimensionID);
                    isChangingDimension = false;
                    continue;
                }

                TraceLog(LOG_INFO, "→ Loading dimension: %s", currentDim->name.c_str());
                dimensionManager.LoadDimensionTextures(currentDim);

                WorldInit(&world);
                SetWorldDimension(currentDim->terrainSeed);
                SetDimensionColors(currentDim->grassTopColor, currentDim->dirtSideColor, currentDim->dirtColor);
                WorldLoadTextures(&world, currentDim);
                InitWorldRenderer(&worldRenderer, currentDim);
                skybox = LoadSkyboxFromDimension(currentDim);
                InitDecorationSystem(&decorationSystem);
                GenerateDecorationsForDimension(&decorationSystem, &world, currentDim);
                portalSystem.currentDimensionID = targetDimensionID;

                monumentSystem.GenerateMonuments(ps.camera.position, 5);

                CosmicState::Get().OnDimensionEntered(currentDim->name);

                TraceLog(LOG_INFO, "✓ Dimension change complete!");
            }

            if (dimensionChangeTimer > 1.5f)
            {
                isChangingDimension = false;
                dimensionChangeTimer = 0.0f;
            }
        }

        // ========== RENDERING TO TEXTURE ==========
        BeginTextureMode(screenTarget);
        ClearBackground(fogColor);

        // Skybox (no fog)
        DrawSkybox(skybox, ps.camera);

        BeginMode3D(ps.camera);

        // Draw world WITH integrated fog shader
        DrawWorld(&worldRenderer, &world, ps.camera, fogDensity, fogColor);
        DrawDecorations(&decorationSystem);

        // Draw elements WITHOUT fog
        monumentSystem.Draw();
        watcherSystem.Draw(ps.camera);
        DrawPortals(&portalSystem);
        DrawDroppedItems();
        DrawMiningProgress(ps.mining);

        EndMode3D();
        EndTextureMode();

        // ========== DRAW TO SCREEN WITH CHROMATIC ABERRATION ==========
        BeginDrawing();
        ClearBackground(BLACK);

        // Apply chromatic aberration if tension > 30
        if (chromaticShader.id > 0 && chromaticAmount > 0.001f)
        {
            BeginShaderMode(chromaticShader);
            SetShaderValue(chromaticShader, aberrationLoc, &chromaticAmount, SHADER_UNIFORM_FLOAT);
        }

        // Draw render texture to screen
        DrawTextureRec(
            screenTarget.texture,
            (Rectangle){0, 0, (float)screenTarget.texture.width, -(float)screenTarget.texture.height},
            (Vector2){0, 0},
            WHITE);

        if (chromaticShader.id > 0 && chromaticAmount > 0.001f)
        {
            EndShaderMode();
        }

        // ========== HUD ==========
        DrawPortalGun(&portalSystem, ps.camera);
        DrawFPS(10, 10);

        char debug[512];
        sprintf(debug,
                "DIM: %s | Tension: %.1f | Watchers: %d | Monuments: %d/%d\n"
                "Fog: %.3f | Chromatic: %.4f | Pos: (%.0f,%.0f,%.0f)\n"
                "Press F for fog debug",
                currentDim->name.c_str(),
                tension,
                watcherSystem.GetActiveWatcherCount(),
                monumentSystem.GetActivatedCount(),
                monumentSystem.GetDiscoveredCount(),
                fogDensity,
                chromaticAmount,
                ps.camera.position.x, ps.camera.position.y, ps.camera.position.z);
        DrawText(debug, 10, 30, 16, WHITE);

        DimensionConfig *selDim = dimensionManager.GetDimension(portalSystem.currentDimensionID);
        char portalInfo[128];
        sprintf(portalInfo, "Portal Target: %s", selDim->name.c_str());
        DrawText(portalInfo, 10, 90, 16, selDim->grassTopColor);

        // Warning messages based on tension
        if (tension > 25.0f && tension < 30.0f && CosmicState::Get().IsEventTriggered("firstwatcher"))
        {
            DrawText("Something is watching...",
                     screenWidth / 2 - 120, 120, 20,
                     Fade(YELLOW, sinf(GetTime() * 3.0f) * 0.5f + 0.5f));
        }
        else if (tension > 50.0f && tension < 70.0f)
        {
            DrawText("You feel watched...",
                     screenWidth / 2 - 100, 120, 20,
                     Fade(RED, sinf(GetTime() * 3.0f) * 0.5f + 0.5f));
        }
        else if (tension > 80.0f)
        {
            DrawText("REALITY IS BREAKING",
                     screenWidth / 2 - 150, 120, 24,
                     Fade(RED, sinf(GetTime() * 10.0f) * 0.5f + 0.5f));
        }

        DrawText("LMB: Mine | RMB: Place | MMB: Portal | TAB: Inventory | 1-9: Hotbar",
                 20, screenHeight - 30, 16, LIGHTGRAY);

        // Crosshair
        if (!inventoryOpen)
        {
            int centerX = screenWidth / 2;
            int centerY = screenHeight / 2;
            DrawLine(centerX - 10, centerY, centerX + 10, centerY, WHITE);
            DrawLine(centerX, centerY - 10, centerX, centerY + 10, WHITE);
        }

        // Inventory
        if (inventoryOpen)
        {
            playerInventory.DrawFullInventory();
        }
        else
        {
            playerInventory.DrawHotbar();
        }

        // Dimension change effect
        if (isChangingDimension)
        {
            float alpha = fminf(dimensionChangeTimer / 1.5f, 1.0f);
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, alpha * 0.8f));

            const char *text = "Traveling between dimensions...";
            int textWidth = MeasureText(text, 40);
            DrawText(text, screenWidth / 2 - textWidth / 2, screenHeight / 2 - 20, 40,
                     Fade(WHITE, sinf(dimensionChangeTimer * 5.0f) * 0.5f + 0.5f));
        }

        EndDrawing();
    }

    // ========== CLEANUP ==========
    TraceLog(LOG_INFO, "========================================");
    TraceLog(LOG_INFO, "   SHUTTING DOWN");
    TraceLog(LOG_INFO, "========================================");

    watcherSystem.Cleanup();
    monumentSystem.Cleanup();
    AudioManager::Get().Cleanup();

    UnloadRenderTexture(screenTarget);
    if (chromaticShader.id > 0)
        UnloadShader(chromaticShader);

    UnloadSkybox(skybox);
    UnloadWorldRenderer(&worldRenderer);
    dimensionManager.Cleanup();
    UnloadTerrainShader();

    WorldCleanup(&world);
    CleanupPortalSystem(&portalSystem);
    CleanupDecorationSystem(&decorationSystem);
    CleanupDroppedItems();

    TraceLog(LOG_INFO, "✓ Cleanup complete");
    CloseWindow();

    return 0;
}