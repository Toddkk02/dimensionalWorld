#include "raylib.h"
#include "core/player.h"
#include "rendering/shaders.h"
#include "world/firstWorld.h"
#include "world/dimensions.h"
#include "rendering/skybox.h"
#include "core/portal.h"
#include "world/decorations.h"
#include "gameplay/dropped_item.h"
#include "gameplay/inventory.h"
#include <cstdio>
#include "rlgl.h"
#include <cmath>

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
        DrawCubeWires(blockCenter,
                      1.0f + offset,
                      1.0f + offset,
                      1.0f + offset,
                      Fade(YELLOW, 0.5f));
    }
}

int main(void)
{
    InitWindow(1600, 900, "Dimensional World - Infinite Dimensions");
    rlEnableDepthTest();
    SetTargetFPS(60);

    LoadTerrainShader();

    DimensionManager dimensionManager;
    dimensionManager.Initialize();

    DimensionConfig *currentDim = dimensionManager.GetCurrentDimension();

    Skybox skybox = LoadSkyboxFromDimension(currentDim);

    World world;
    WorldInit(&world);
    WorldLoadTextures(&world, currentDim);

    PortalSystem portalSystem;
    InitPortalSystem(&portalSystem);
    portalSystem.currentDimensionID = currentDim->id;

    DecorationSystem decorationSystem;
    InitDecorationSystem(&decorationSystem);

    Inventory playerInventory;
    bool inventoryOpen = false;

    // Riempimento inventario di test
    playerInventory.AddItem(ItemType::DIRT, 64);
    playerInventory.AddItem(ItemType::GRASS, 64);
    playerInventory.AddItem(ItemType::STONE, 64);
    TraceLog(LOG_INFO, "Test inventory filled with blocks");

    SetWorldDimension(currentDim->terrainSeed);
    SetDimensionColors(currentDim->grassTopColor,
                       currentDim->dirtSideColor,
                       currentDim->dirtColor);

    for (int x = -2; x <= 2; x++)
    {
        for (int z = -2; z <= 2; z++)
        {
            WorldUpdate(&world, (Vector3){(float)(x * 16), 10.0f, (float)(z * 16)});
        }
    }

    GenerateDecorationsForDimension(&decorationSystem, &world, currentDim);

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
    ps.mining.mining = false;
    ps.mining.progress = 0.0f;
    ps.mining.targetBlock = (Vector3){0, 0, 0};
    DisableCursor();

    bool isChangingDimension = false;
    float dimensionChangeTimer = 0.0f;
    int targetDimensionID = 0;

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        for (int i = 0; i < HOTBAR_SIZE; i++)
        {
            if (IsKeyPressed(KEY_ONE + i))
            {
                playerInventory.SelectSlot(i);
            }
        }

        if (IsKeyPressed(KEY_TAB))
        {
            inventoryOpen = !inventoryOpen;
        }

        if (isChangingDimension)
        {
            dimensionChangeTimer += deltaTime;

            if (dimensionChangeTimer > 1.5f)
            {
                portalSystem.currentDimensionID = targetDimensionID;
                currentDim = dimensionManager.GetDimension(targetDimensionID);

                UnloadSkybox(skybox);
                skybox = LoadSkyboxFromDimension(currentDim);

                SetWorldDimension(currentDim->terrainSeed);
                SetDimensionColors(currentDim->grassTopColor,
                                   currentDim->dirtSideColor,
                                   currentDim->dirtColor);

                WorldLoadTextures(&world, currentDim);
                RegenerateAllChunks(&world);
                GenerateDecorationsForDimension(&decorationSystem, &world, currentDim);

                isChangingDimension = false;
                dimensionChangeTimer = 0.0f;

                TraceLog(LOG_INFO, "Entered dimension: %s", currentDim->name.c_str());
            }
        }
        else
        {
            UpdateDroppedItems(&world, &playerInventory, ps.camera.position, deltaTime);
            WorldUpdate(&world, ps.camera.position);

            if (!inventoryOpen)
            {
                UpdatePlayerPhysics(&ps, &world, deltaTime);
                UpdateCamera(&ps.camera, ps.cameraMode);
                UpdateMining(ps.mining, ps.camera, &world, deltaTime);

                // MINING COMPLETATO
                if (ps.mining.progress >= 1.5f)
                {
                    ItemType drop = RemoveBlock(&world,
                                                (int)ps.mining.targetBlock.x,
                                                (int)ps.mining.targetBlock.y,
                                                (int)ps.mining.targetBlock.z);

                    if (drop != ItemType::NONE)
                    {
                        SpawnDroppedItem(drop, ps.mining.targetBlock);
                    }

                    ps.mining.progress = 0;
                }

                // PIAZZAMENTO BLOCCHI - FUORI DAL BLOCCO MINING!
                if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                {
                    TraceLog(LOG_INFO, "=== RIGHT CLICK DETECTED ===");

                    Item selectedItem = playerInventory.GetSelected();
                    TraceLog(LOG_INFO, "Selected item: %s (qty: %d)",
                             GetItemName(selectedItem.type), selectedItem.quantity);

                    if (selectedItem.type != ItemType::NONE && selectedItem.quantity > 0)
                    {
                        Vector3 placePos;

                        TraceLog(LOG_INFO, "Camera pos: (%.1f, %.1f, %.1f)",
                                 ps.camera.position.x, ps.camera.position.y, ps.camera.position.z);

                        if (RaycastPlaceBlock(ps.camera, &world, placePos))
                        {
                            TraceLog(LOG_INFO, "Raycast SUCCESS - Attempting to place at (%.0f, %.0f, %.0f)",
                                     placePos.x, placePos.y, placePos.z);

                            if (PlaceBlock(&world,
                                          (int)placePos.x,
                                          (int)placePos.y,
                                          (int)placePos.z,
                                          selectedItem.type))
                            {
                                playerInventory.RemoveSelected(1);
                                TraceLog(LOG_INFO, "✓ BLOCK PLACED SUCCESSFULLY!");
                            }
                            else
                            {
                                TraceLog(LOG_ERROR, "✗ PlaceBlock FAILED");
                            }
                        }
                        else
                        {
                            TraceLog(LOG_WARNING, "✗ Raycast failed - no valid position found");
                        }
                    }
                    else
                    {
                        TraceLog(LOG_WARNING, "No valid item in hand");
                    }

                    TraceLog(LOG_INFO, "=== END RIGHT CLICK ===\n");
                }

                UpdatePortalSystem(&portalSystem, ps.camera, &world, &dimensionManager, deltaTime);

                Portal *nearPortal = CheckPlayerNearPortal(&portalSystem, ps.camera.position);
                if (nearPortal && IsKeyPressed(KEY_E))
                {
                    isChangingDimension = true;
                    targetDimensionID = nearPortal->targetDimensionID;
                    nearPortal->active = false;
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (isChangingDimension)
        {
            float progress = dimensionChangeTimer / 1.5f;
            float flashIntensity = sinf(progress * 20.0f) * (1.0f - progress);
            ClearBackground(Fade(WHITE, flashIntensity * 0.9f));
        }

        DrawSkybox(skybox, ps.camera);

        BeginMode3D(ps.camera);

        BeginShaderMode(terrainShader);
        WorldDraw(&world);
        EndShaderMode();

        DrawDecorations(&decorationSystem);
        DrawPortals(&portalSystem);
        DrawPortalGun(&portalSystem, ps.camera);
        DrawMiningProgress(ps.mining);
        DrawDroppedItems();

        EndMode3D();

        DrawFPS(10, 10);

        char debugText[500];
        sprintf(debugText,
                "ESC = mouse | WASD = move | SPACE = jump | LEFT CLICK = mine | RIGHT CLICK = Place Block | E = Enter | TAB = Inventory\n"
                "Pos: %.1f, %.1f, %.1f | Dimension: %s (%d/%d)\n"
                "Portals: %zu | Trees: %zu | Rocks: %zu | Crystals: %zu | Items: %zu\n"
                "Mining: %s | Target: (%.0f, %.0f, %.0f) | Progress: %.1f/1.5s",
                ps.camera.position.x, ps.camera.position.y, ps.camera.position.z,
                currentDim->name.c_str(),
                currentDim->id + 1,
                dimensionManager.GetDimensionCount(),
                portalSystem.portals.size(),
                decorationSystem.trees.size(),
                decorationSystem.rocks.size(),
                decorationSystem.crystals.size(),
                g_droppedItems.size(),
                ps.mining.mining ? "YES" : "NO",
                ps.mining.targetBlock.x,
                ps.mining.targetBlock.y,
                ps.mining.targetBlock.z,
                ps.mining.progress);
        DrawText(debugText, 10, 30, 16, WHITE);

        if (ps.mining.mining)
        {
            float progress = fminf(ps.mining.progress / 1.5f, 1.0f);
            int barWidth = 300;
            int barHeight = 30;
            int barX = GetScreenWidth() / 2 - barWidth / 2;
            int barY = GetScreenHeight() - 150;

            DrawRectangle(barX, barY, barWidth, barHeight, Fade(BLACK, 0.7f));
            DrawRectangle(barX, barY, (int)(barWidth * progress), barHeight,
                          ColorLerp(RED, GREEN, progress));
            DrawRectangleLines(barX, barY, barWidth, barHeight, WHITE);

            char progressText[32];
            sprintf(progressText, "Mining: %.0f%%", progress * 100.0f);
            int textWidth = MeasureText(progressText, 20);
            DrawText(progressText, barX + barWidth / 2 - textWidth / 2, barY + 5, 20, WHITE);
        }

        Portal *nearPortal = CheckPlayerNearPortal(&portalSystem, ps.camera.position);
        if (nearPortal && !isChangingDimension)
        {
            DimensionConfig *targetDim = dimensionManager.GetDimension(nearPortal->targetDimensionID);
            char portalText[128];
            sprintf(portalText, "Press E to enter: %s", targetDim->name.c_str());
            int textWidth = MeasureText(portalText, 30);
            DrawText(portalText, GetScreenWidth() / 2 - textWidth / 2,
                     GetScreenHeight() - 100, 30, nearPortal->color);

            int descWidth = MeasureText(targetDim->description.c_str(), 20);
            DrawText(targetDim->description.c_str(),
                     GetScreenWidth() / 2 - descWidth / 2,
                     GetScreenHeight() - 60, 20, Fade(WHITE, 0.7f));
        }

        if (isChangingDimension)
        {
            float progress = dimensionChangeTimer / 1.5f;
            DimensionConfig *targetDim = dimensionManager.GetDimension(targetDimensionID);

            char travelText[128];
            sprintf(travelText, "Traveling to %s...", targetDim->name.c_str());
            int textWidth = MeasureText(travelText, 40);

            Color textColor = ColorLerp(WHITE, targetDim->grassTopColor, progress);
            DrawText(travelText, GetScreenWidth() / 2 - textWidth / 2,
                     GetScreenHeight() / 2, 40, textColor);

            DrawRectangle(GetScreenWidth() / 2 - 200, GetScreenHeight() / 2 + 60,
                          (int)(400 * progress), 10, targetDim->grassTopColor);
            DrawRectangleLines(GetScreenWidth() / 2 - 200, GetScreenHeight() / 2 + 60,
                               400, 10, WHITE);
        }

        DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 3, Fade(GREEN, 0.5f));
        DrawCircleLines(GetScreenWidth() / 2, GetScreenHeight() / 2, 10, Fade(GREEN, 0.3f));

        if (inventoryOpen)
        {
            playerInventory.DrawFullInventory();
        }
        else
        {
            playerInventory.DrawHotbar();
        }

        EndDrawing();
    }

    UnloadTerrainShader();
    UnloadSkybox(skybox);
    WorldUnloadTextures(&world);
    WorldCleanup(&world);
    CleanupPortalSystem(&portalSystem);
    CleanupDecorationSystem(&decorationSystem);
    CleanupDroppedItems();
    CloseWindow();
    return 0;
}