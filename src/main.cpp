#include "raylib.h"
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
#include <cstdio>
#include <cmath>
#include "rlgl.h"

void DrawMiningProgress(MiningState &mining) {
    if (!mining.mining) return;
    Vector3 blockCenter = mining.targetBlock;
    blockCenter.x += 0.5f; blockCenter.y += 0.5f; blockCenter.z += 0.5f;
    float pulseScale = 1.0f + sinf(mining.progress * 10.0f) * 0.1f;
    DrawCubeWires(blockCenter, pulseScale, pulseScale, pulseScale, RED);
    
    int crackLevel = (int)(mining.progress / 0.5f);
    if (crackLevel > 3) crackLevel = 3;
    for (int i = 0; i <= crackLevel; i++) {
        float offset = 0.1f * i;
        DrawCubeWires(blockCenter, 1.0f + offset, 1.0f + offset, 1.0f + offset, Fade(YELLOW, 0.5f));
    }
}

int main() {
    const int screenWidth = 1600, screenHeight = 900;
    InitWindow(screenWidth, screenHeight, "Dimensional World - Infinite Dimensions");
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
    
    if (!currentDim) { 
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
    TraceLog(LOG_INFO, "✓ Shaders loaded");

    // ========== WORLD ==========
    World world;
    WorldInit(&world);
    SetWorldDimension(currentDim->terrainSeed);
    SetDimensionColors(currentDim->grassTopColor, currentDim->dirtSideColor, currentDim->dirtColor);
    WorldLoadTextures(&world, currentDim);
    TraceLog(LOG_INFO, "✓ World initialized (Textures: %s)", world.useTextures ? "ENABLED" : "DISABLED");

    // ========== WORLD RENDERER ==========
    WorldRenderer worldRenderer;
    InitWorldRenderer(&worldRenderer, currentDim);
    TraceLog(LOG_INFO, "✓ World Renderer initialized");

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
    TraceLog(LOG_INFO, "✓ Inventory initialized with starter items");

    bool inventoryOpen = false;
    bool isChangingDimension = false;
    float dimensionChangeTimer = 0.0f;
    int targetDimensionID = 0;

    TraceLog(LOG_INFO, "========================================");
    TraceLog(LOG_INFO, "   ENTERING MAIN GAME LOOP");
    TraceLog(LOG_INFO, "========================================");

    // ========== MAIN LOOP ==========
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        CosmicState::Get().Update(deltaTime);

        // ========== INPUT ==========
        if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_E)) 
            inventoryOpen = !inventoryOpen;
        
        for (int i = 0; i < HOTBAR_SIZE; i++)
            if (IsKeyPressed(KEY_ONE + i)) 
                playerInventory.SelectSlot(i);

        if (!inventoryOpen && !isChangingDimension) {
            // ========== WORLD UPDATE ==========
            WorldUpdate(&world, ps.camera.position);
            UpdatePlayerPhysics(&ps, &world, deltaTime);
            UpdateCamera(&ps.camera, ps.cameraMode);
            
            // ========== MINING (SCAVARE) ==========
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                UpdateMining(ps.mining, ps.camera, &world, deltaTime);
                
                if (ps.mining.mining && ps.mining.progress >= 2.0f) {
                    int bx = (int)ps.mining.targetBlock.x;
                    int by = (int)ps.mining.targetBlock.y;
                    int bz = (int)ps.mining.targetBlock.z;
                    
                    ItemType droppedItem = RemoveBlock(&world, bx, by, bz);
                    
                    if (droppedItem != ItemType::NONE) {
                        Vector3 dropPos = {
                            ps.mining.targetBlock.x + 0.5f,
                            ps.mining.targetBlock.y + 1.0f,
                            ps.mining.targetBlock.z + 0.5f
                        };
                        SpawnDroppedItem(droppedItem, dropPos);
                    }
                    
                    ps.mining.mining = false;
                    ps.mining.progress = 0.0f;
                }
            } else {
                ps.mining.mining = false;
                ps.mining.progress = 0.0f;
            }
            
            // ========== PIAZZAMENTO BLOCCHI ==========
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                Item selectedItem = playerInventory.GetSelected();
                
                if (selectedItem.type != ItemType::NONE && selectedItem.quantity > 0) {
                    Vector3 placePos;
                    if (RaycastPlaceBlock(ps.camera, &world, placePos)) {
                        int px = (int)placePos.x;
                        int py = (int)placePos.y;
                        int pz = (int)placePos.z;
                        
                        if (PlaceBlock(&world, px, py, pz, selectedItem.type)) {
                            playerInventory.RemoveSelected(1);
                        }
                    }
                }
            }
            
            // ========== PORTALI ==========
            UpdatePortalSystem(&portalSystem, ps.camera, &world, &dimensionManager, deltaTime);
            
            Portal* nearPortal = CheckPlayerNearPortal(&portalSystem, ps.camera.position);
            if (nearPortal && !isChangingDimension) {
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
        if (isChangingDimension) {
            dimensionChangeTimer += deltaTime;
            
            if (dimensionChangeTimer > 0.5f && dimensionChangeTimer < 0.6f) {
                TraceLog(LOG_INFO, "========================================");
                TraceLog(LOG_INFO, "   DIMENSION CHANGE IN PROGRESS");
                TraceLog(LOG_INFO, "========================================");
                
                // 1. CLEANUP OLD DIMENSION
                TraceLog(LOG_INFO, "→ Unloading skybox...");
                UnloadSkybox(skybox);
                
                TraceLog(LOG_INFO, "→ Cleaning decorations...");
                CleanupDecorationSystem(&decorationSystem);
                
                TraceLog(LOG_INFO, "→ Unloading world renderer...");
                UnloadWorldRenderer(&worldRenderer);
                
                TraceLog(LOG_INFO, "→ Cleaning world...");
                WorldCleanup(&world);
                
                TraceLog(LOG_INFO, "→ Unloading old textures...");
                dimensionManager.UnloadDimensionTextures(currentDim);
                
                // 2. LOAD NEW DIMENSION
                currentDim = dimensionManager.GetDimension(targetDimensionID);
                
                if (!currentDim) {
                    TraceLog(LOG_ERROR, "✗ Failed to get dimension %d!", targetDimensionID);
                    isChangingDimension = false;
                    continue;
                }
                
                TraceLog(LOG_INFO, "→ Loading dimension: %s", currentDim->name.c_str());
                
                // 3. LOAD TEXTURES
                TraceLog(LOG_INFO, "→ Loading new textures...");
                dimensionManager.LoadDimensionTextures(currentDim);
                
                // 4. SETUP WORLD
                TraceLog(LOG_INFO, "→ Re-initializing world...");
                WorldInit(&world);
                SetWorldDimension(currentDim->terrainSeed);
                SetDimensionColors(currentDim->grassTopColor, currentDim->dirtSideColor, currentDim->dirtColor);
                WorldLoadTextures(&world, currentDim);
                
                // 5. INIT RENDERER
                TraceLog(LOG_INFO, "→ Re-initializing renderer...");
                InitWorldRenderer(&worldRenderer, currentDim);
                
                // 6. LOAD SKYBOX
                TraceLog(LOG_INFO, "→ Loading new skybox...");
                skybox = LoadSkyboxFromDimension(currentDim);
                
                // 7. GENERATE DECORATIONS
                TraceLog(LOG_INFO, "→ Generating decorations...");
                InitDecorationSystem(&decorationSystem);
                GenerateDecorationsForDimension(&decorationSystem, &world, currentDim);
                
                // 8. UPDATE PORTAL SYSTEM
                portalSystem.currentDimensionID = targetDimensionID;
                
                // 9. COSMIC EVENT
                CosmicState::Get().OnDimensionEntered(currentDim->name);
                
                TraceLog(LOG_INFO, "✓ Dimension change complete!");
                TraceLog(LOG_INFO, "========================================");
            }
            
            if (dimensionChangeTimer > 1.5f) {
                isChangingDimension = false;
                dimensionChangeTimer = 0.0f;
            }
        }

        // ========== RENDERING ==========
        BeginDrawing();
        ClearBackground(SKYBLUE);

        // Background
        DrawSkybox(skybox, ps.camera);

        BeginMode3D(ps.camera);
        
        // ✅ USA IL WORLD RENDERER CON TEXTURE
        DrawWorld(&worldRenderer, &world);
        
        DrawDecorations(&decorationSystem);
        DrawPortals(&portalSystem);
        DrawDroppedItems();
        DrawMiningProgress(ps.mining);
        
        EndMode3D();
        
        DrawPortalGun(&portalSystem, ps.camera);

        // ========== HUD ==========
        DrawFPS(10, 10);
        
        char debug[512];
        sprintf(debug, "DIM: %s | Tension: %.1f | Pos: (%.0f,%.0f,%.0f)\nTextures: %s | Renderer: %s", 
                currentDim->name.c_str(), 
                CosmicState::Get().GetTension(),
                ps.camera.position.x, ps.camera.position.y, ps.camera.position.z,
                world.useTextures ? "ON" : "OFF",
                worldRenderer.initialized ? "ACTIVE" : "INACTIVE");
        DrawText(debug, 10, 30, 16, WHITE);
        
        DrawText("LMB: Mine | RMB: Place | MMB: Portal | TAB: Inventory | 1-9: Hotbar", 
                 10, screenHeight - 30, 16, LIGHTGRAY);
        
        // Crosshair
        if (!inventoryOpen) {
            int centerX = screenWidth / 2;
            int centerY = screenHeight / 2;
            DrawLine(centerX - 10, centerY, centerX + 10, centerY, WHITE);
            DrawLine(centerX, centerY - 10, centerX, centerY + 10, WHITE);
        }

        // Inventory
        if (inventoryOpen) {
            playerInventory.DrawFullInventory();
        } else {
            playerInventory.DrawHotbar();
        }
        
        // Dimension change effect
        if (isChangingDimension) {
            float alpha = fminf(dimensionChangeTimer / 1.5f, 1.0f);
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, alpha * 0.8f));
            
            const char* text = "Traveling between dimensions...";
            int textWidth = MeasureText(text, 40);
            DrawText(text, screenWidth/2 - textWidth/2, screenHeight/2 - 20, 40, 
                     Fade(WHITE, sinf(dimensionChangeTimer * 5.0f) * 0.5f + 0.5f));
        }

        EndDrawing();
    }

    // ========== CLEANUP ==========
    TraceLog(LOG_INFO, "========================================");
    TraceLog(LOG_INFO, "   SHUTTING DOWN");
    TraceLog(LOG_INFO, "========================================");
    
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