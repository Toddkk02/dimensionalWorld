#include "dimensions.h"
#include "blockTypes.h"
#include "blocks.h"  // ← AGGIUNGI QUESTO
#include <cstring>

// ========== COSTRUTTORE DIMENSIONCONFIG ==========

DimensionConfig::DimensionConfig() 
    : id(0),
      name(""),
      description(""),
      grassTopTexture(""),
      dirtSideTexture(""),
      dirtTexture(""),
      waterTexture(""),
      skyboxTexture(""),
      grassTopTex({0,0,0,0,0}),
      dirtSideTex({0,0,0,0,0}),
      dirtTex({0,0,0,0,0}),
      waterTex({0,0,0,0,0}),
      skyboxTex({0,0,0,0,0}),
      grassTopLoaded(false),
      dirtSideLoaded(false),
      dirtLoaded(false),
      waterLoaded(false),
      skyboxLoaded(false),
      terrainSeed(0),
      terrainScale(0.02f),
      terrainHeight(8.0f),
      waterLevel(4.0f),
      treeCount(0),
      rockCount(0),
      crystalCount(0),
      grassTopColor(GREEN),
      dirtSideColor(BROWN),
      dirtColor(DARKBROWN),
      treeColor(GREEN),
      rockColor(GRAY),
      crystalColor(BLUE),
      ambientLight(LIGHTGRAY),
      sunColor(WHITE),
      useFog(false),
      fogColor(GRAY),
      fogDensity(0.0f)
{
    for(int i = 0; i < BLOCK_COUNT; i++) {
        blockTexturePaths[i] = "";
    }
}

void DimensionConfig::UnloadTextures() {
    if (grassTopLoaded && grassTopTex.id != 0) {
        UnloadTexture(grassTopTex);
        grassTopTex.id = 0;
        grassTopLoaded = false;
    }
    if (dirtSideLoaded && dirtSideTex.id != 0) {
        UnloadTexture(dirtSideTex);
        dirtSideTex.id = 0;
        dirtSideLoaded = false;
    }
    if (dirtLoaded && dirtTex.id != 0) {
        UnloadTexture(dirtTex);
        dirtTex.id = 0;
        dirtLoaded = false;
    }
    if (skyboxLoaded && skyboxTex.id != 0) {
        UnloadTexture(skyboxTex);
        skyboxTex.id = 0;
        skyboxLoaded = false;
    }
    if (waterLoaded && waterTex.id != 0) {
        UnloadTexture(waterTex);
        waterTex.id = 0;
        waterLoaded = false;
    }
}

bool DimensionConfig::IsTextureLoaded(const Texture2D& tex) const {
    return tex.id != 0;
}

// ========== DIMENSION MANAGER ==========

void DimensionManager::Initialize() {
    dimensions.clear();
    currentDimensionID = 0;
    
    InitBlockSystem();
    
    AddDimension(CreatePurpleDimension());
    AddDimension(CreateGreenDimension());
    AddDimension(CreateRedDimension());
    AddDimension(CreateDesertDimension());
    AddDimension(CreateIceDimension());
    AddDimension(CreateVolcanoDimension());
    AddDimension(CreateSpaceDimension());
}

void DimensionManager::AddDimension(const DimensionConfig& config) {
    dimensions.push_back(config);
}

DimensionConfig* DimensionManager::GetDimension(int id) {
    for (size_t i = 0; i < dimensions.size(); i++) {
        if (dimensions[i].id == id) {
            return &dimensions[i];
        }
    }
    return nullptr;
}

DimensionConfig* DimensionManager::GetCurrentDimension() {
    return GetDimension(currentDimensionID);
}

int DimensionManager::GetDimensionCount() {
    return (int)dimensions.size();
}

void DimensionManager::LoadDimensionTextures(DimensionConfig* dim) {
    if (!dim) return;
    
    TraceLog(LOG_INFO, "=== LOADING TEXTURES FOR: %s ===", dim->name.c_str());
    
    // Grass Top
    if (!dim->grassTopLoaded && !dim->grassTopTexture.empty()) {
        if (FileExists(dim->grassTopTexture.c_str())) {
            dim->grassTopTex = LoadTexture(dim->grassTopTexture.c_str());
            dim->grassTopLoaded = (dim->grassTopTex.id != 0);
            TraceLog(LOG_INFO, "✓ Loaded grassTop: %s (ID: %d)", dim->grassTopTexture.c_str(), dim->grassTopTex.id);
        } else {
            TraceLog(LOG_WARNING, "✗ MISSING grassTop: %s", dim->grassTopTexture.c_str());
        }
    }
    
    // Dirt Side
    if (!dim->dirtSideLoaded && !dim->dirtSideTexture.empty()) {
        if (FileExists(dim->dirtSideTexture.c_str())) {
            dim->dirtSideTex = LoadTexture(dim->dirtSideTexture.c_str());
            dim->dirtSideLoaded = (dim->dirtSideTex.id != 0);
            TraceLog(LOG_INFO, "✓ Loaded dirtSide: %s (ID: %d)", dim->dirtSideTexture.c_str(), dim->dirtSideTex.id);
        } else {
            TraceLog(LOG_WARNING, "✗ MISSING dirtSide: %s", dim->dirtSideTexture.c_str());
        }
    }
    
    // Dirt
    if (!dim->dirtLoaded && !dim->dirtTexture.empty()) {
        if (FileExists(dim->dirtTexture.c_str())) {
            dim->dirtTex = LoadTexture(dim->dirtTexture.c_str());
            dim->dirtLoaded = (dim->dirtTex.id != 0);
            TraceLog(LOG_INFO, "✓ Loaded dirt: %s (ID: %d)", dim->dirtTexture.c_str(), dim->dirtTex.id);
        } else {
            TraceLog(LOG_WARNING, "✗ MISSING dirt: %s", dim->dirtTexture.c_str());
        }
    }
    
    // Skybox
    if (!dim->skyboxLoaded && !dim->skyboxTexture.empty()) {
        if (FileExists(dim->skyboxTexture.c_str())) {
            dim->skyboxTex = LoadTexture(dim->skyboxTexture.c_str());
            dim->skyboxLoaded = (dim->skyboxTex.id != 0);
            TraceLog(LOG_INFO, "✓ Loaded skybox: %s (ID: %d)", dim->skyboxTexture.c_str(), dim->skyboxTex.id);
        } else {
            TraceLog(LOG_WARNING, "✗ MISSING skybox: %s", dim->skyboxTexture.c_str());
        }
    }
    
    // Water
    if (!dim->waterLoaded && !dim->waterTexture.empty()) {
        if (FileExists(dim->waterTexture.c_str())) {
            dim->waterTex = LoadTexture(dim->waterTexture.c_str());
            dim->waterLoaded = (dim->waterTex.id != 0);
            TraceLog(LOG_INFO, "✓ Loaded water: %s (ID: %d)", dim->waterTexture.c_str(), dim->waterTex.id);
        } else {
            TraceLog(LOG_WARNING, "✗ MISSING water: %s", dim->waterTexture.c_str());
        }
    }
    
    TraceLog(LOG_INFO, "=== TEXTURE LOADING COMPLETE ===");
}

void DimensionManager::UnloadDimensionTextures(DimensionConfig* dim) {
    if (dim) {
        TraceLog(LOG_INFO, "Unloading textures for: %s", dim->name.c_str());
        dim->UnloadTextures();
    }
}

void DimensionManager::Cleanup() {
    for (auto& dim : dimensions) {
        dim.UnloadTextures();
    }
    CleanupBlockSystem();
}

// ========== FACTORY DIMENSIONI ==========

DimensionConfig DimensionManager::CreatePurpleDimension() {
    DimensionConfig config;
    config.id = 0;
    config.name = "Purple Dimension";
    config.description = "Il mondo viola originale di Rick";

    config.grassTopColor = {153, 51, 255, 255};
    config.dirtSideColor = {51, 25, 0, 255};
    config.dirtColor = {51, 25, 0, 255};

    // TEXTURE PATHS
    config.grassTopTexture = "assets/textures/purple_grass.png";
    config.dirtSideTexture = "assets/textures/purple_dirt_side.png";
    config.dirtTexture = "assets/textures/purple_dirt.png";
    config.waterTexture = "assets/textures/water.png";

    config.skyboxTexture = "assets/textures/skybox_purple.jpg";
    config.skyboxTint = {255, 200, 255, 255};

    config.terrainSeed = 0;
    config.terrainScale = 0.02f;
    config.terrainHeight = 8.0f;
    config.waterLevel = 4.0f;

    config.treeCount = 0;
    config.rockCount = 15;
    config.crystalCount = 0;
    config.rockColor = {153, 51, 255, 255};

    config.ambientLight = {76, 51, 102, 255};
    config.sunColor = {255, 204, 255, 255};
    config.sunDirection = {-0.5f, -1.0f, -0.5f};

    config.useFog = true;
    config.fogColor = {100, 50, 150, 255};
    config.fogDensity = 0.02f;

    return config;
}

DimensionConfig DimensionManager::CreateGreenDimension() {
    DimensionConfig config;
    config.id = 1;
    config.name = "Forest Dimension";
    config.description = "Dimensione verde con foreste rigogliose";

    config.grassTopColor = {50, 200, 50, 255};
    config.dirtSideColor = {101, 67, 33, 255};
    config.dirtColor = {139, 90, 43, 255};

    // TEXTURE PATHS
    config.grassTopTexture = "assets/textures/greengrass.png";
    config.dirtSideTexture = "assets/textures/greendirt.png";
    config.dirtTexture = "assets/textures/dirt.png";
    config.waterTexture = "assets/textures/water.png";

    config.skyboxTexture = "assets/textures/greenskybox.jpg";
    config.skyboxTint = {200, 255, 200, 255};

    config.terrainSeed = 1000;
    config.terrainScale = 0.02f;
    config.terrainHeight = 10.0f;
    config.waterLevel = 5.0f;

    config.treeCount = 40;
    config.rockCount = 10;
    config.crystalCount = 0;
    config.treeColor = {30, 150, 30, 255};
    config.rockColor = {80, 80, 70, 255};

    config.ambientLight = {102, 102, 76, 255};
    config.sunColor = {255, 242, 204, 255};
    config.sunDirection = {-0.5f, -1.0f, -0.5f};

    config.useFog = true;
    config.fogColor = {150, 200, 150, 255};
    config.fogDensity = 0.015f;

    return config;
}

DimensionConfig DimensionManager::CreateRedDimension() {
    DimensionConfig config;
    config.id = 2;
    config.name = "Alien Dimension";
    config.description = "Mondo rosso alieno con cristalli azzurri";

    config.grassTopColor = {255, 50, 50, 255};
    config.dirtSideColor = {150, 30, 30, 255};
    config.dirtColor = {100, 20, 20, 255};

    // TEXTURE PATHS
    config.grassTopTexture = "assets/textures/red_grass.png";
    config.dirtSideTexture = "assets/textures/red_dirt_side.png";
    config.dirtTexture = "assets/textures/red_dirt.png";
    config.waterTexture = "assets/textures/lava.png";

    config.skyboxTexture = "assets/textures/skybox_red.jpg";
    config.skyboxTint = {255, 150, 150, 255};

    config.terrainSeed = 2000;
    config.terrainScale = 0.03f;
    config.terrainHeight = 12.0f;
    config.waterLevel = 2.0f;

    config.treeCount = 0;
    config.rockCount = 25;
    config.crystalCount = 30;
    config.rockColor = {200, 50, 30, 255};
    config.crystalColor = {50, 200, 255, 255};

    config.ambientLight = {102, 51, 51, 255};
    config.sunColor = {255, 127, 102, 255};
    config.sunDirection = {-0.3f, -1.0f, -0.7f};

    config.useFog = true;
    config.fogColor = {150, 50, 50, 255};
    config.fogDensity = 0.025f;

    return config;
}

DimensionConfig DimensionManager::CreateDesertDimension() {
    DimensionConfig config;
    config.id = 3;
    config.name = "Desert Dimension";
    config.description = "Deserto infinito con dune e oasi";

    config.grassTopColor = {220, 180, 120, 255};
    config.dirtSideColor = {200, 160, 100, 255};
    config.dirtColor = {180, 140, 80, 255};

    // TEXTURE PATHS
    config.grassTopTexture = "assets/textures/sand.png";
    config.dirtSideTexture = "assets/textures/sandstone.png";
    config.dirtTexture = "assets/textures/sandstone.png";
    config.waterTexture = "assets/textures/water.png";

    config.skyboxTexture = "assets/textures/skybox_desert.jpg";
    config.skyboxTint = {255, 240, 200, 255};

    config.terrainSeed = 3000;
    config.terrainScale = 0.015f;
    config.terrainHeight = 6.0f;
    config.waterLevel = 1.0f;

    config.treeCount = 5;
    config.rockCount = 20;
    config.crystalCount = 0;
    config.treeColor = {100, 150, 50, 255};
    config.rockColor = {150, 120, 80, 255};

    config.ambientLight = {127, 114, 89, 255};
    config.sunColor = {255, 229, 178, 255};
    config.sunDirection = {-0.7f, -1.0f, -0.3f};

    config.useFog = true;
    config.fogColor = {220, 200, 160, 255};
    config.fogDensity = 0.01f;

    return config;
}

DimensionConfig DimensionManager::CreateIceDimension() {
    DimensionConfig config;
    config.id = 4;
    config.name = "Ice Dimension";
    config.description = "Tundra ghiacciata con aurora boreale";

    config.grassTopColor = {200, 220, 255, 255};
    config.dirtSideColor = {180, 200, 230, 255};
    config.dirtColor = {150, 170, 200, 255};

    // TEXTURE PATHS
    config.grassTopTexture = "assets/textures/ice.png";
    config.dirtSideTexture = "assets/textures/snow_side.png";
    config.dirtTexture = "assets/textures/snow.png";
    config.waterTexture = "assets/textures/ice.png";

    config.skyboxTexture = "assets/textures/skybox_ice.jpg";
    config.skyboxTint = {200, 220, 255, 255};

    config.terrainSeed = 4000;
    config.terrainScale = 0.025f;
    config.terrainHeight = 7.0f;
    config.waterLevel = 4.0f;

    config.treeCount = 10;
    config.rockCount = 30;
    config.crystalCount = 20;
    config.treeColor = {150, 200, 220, 255};
    config.rockColor = {180, 200, 220, 255};
    config.crystalColor = {150, 200, 255, 255};

    config.ambientLight = {102, 114, 127, 255};
    config.sunColor = {204, 229, 255, 255};
    config.sunDirection = {-0.2f, -1.0f, -0.8f};

    config.useFog = true;
    config.fogColor = {200, 220, 255, 255};
    config.fogDensity = 0.02f;

    return config;
}

DimensionConfig DimensionManager::CreateVolcanoDimension() {
    DimensionConfig config;
    config.id = 5;
    config.name = "Volcano Dimension";
    config.description = "Mondo vulcanico con fiumi di lava";

    config.grassTopColor = {60, 60, 60, 255};
    config.dirtSideColor = {40, 40, 40, 255};
    config.dirtColor = {30, 30, 30, 255};

    // TEXTURE PATHS
    config.grassTopTexture = "assets/textures/obsidian.png";
    config.dirtSideTexture = "assets/textures/stone.png";
    config.dirtTexture = "assets/textures/stone.png";
    config.waterTexture = "assets/textures/lava.png";

    config.skyboxTexture = "assets/textures/skybox_volcano.jpg";
    config.skyboxTint = {255, 150, 100, 255};

    config.terrainSeed = 5000;
    config.terrainScale = 0.035f;
    config.terrainHeight = 15.0f;
    config.waterLevel = 3.0f;

    config.treeCount = 0;
    config.rockCount = 40;
    config.crystalCount = 15;
    config.rockColor = {80, 50, 40, 255};
    config.crystalColor = {255, 100, 50, 255};

    config.ambientLight = {76, 51, 38, 255};
    config.sunColor = {255, 127, 76, 255};
    config.sunDirection = {-0.4f, -1.0f, -0.6f};

    config.useFog = true;
    config.fogColor = {100, 50, 40, 255};
    config.fogDensity = 0.03f;

    return config;
}

DimensionConfig DimensionManager::CreateSpaceDimension() {
    DimensionConfig config;
    config.id = 6;
    config.name = "Space Dimension";
    config.description = "Asteroidi fluttuanti nello spazio";

    config.grassTopColor = {80, 80, 100, 255};
    config.dirtSideColor = {60, 60, 80, 255};
    config.dirtColor = {40, 40, 60, 255};

    // TEXTURE PATHS
    config.grassTopTexture = "assets/textures/moon_rock.png";
    config.dirtSideTexture = "assets/textures/asteroid.png";
    config.dirtTexture = "assets/textures/asteroid.png";
    config.waterTexture = "assets/textures/void.png";

    config.skyboxTexture = "assets/textures/skybox_space.jpg";
    config.skyboxTint = {255, 255, 255, 255};

    config.terrainSeed = 6000;
    config.terrainScale = 0.04f;
    config.terrainHeight = 5.0f;
    config.waterLevel = 0.0f;

    config.treeCount = 0;
    config.rockCount = 50;
    config.crystalCount = 40;
    config.rockColor = {70, 70, 90, 255};
    config.crystalColor = {100, 200, 255, 255};

    config.ambientLight = {25, 25, 51, 255};
    config.sunColor = {204, 204, 255, 255};
    config.sunDirection = {-0.6f, -1.0f, -0.4f};

    config.useFog = false;
    config.fogColor = {0, 0, 0, 255};
    config.fogDensity = 0.0f;

    return config;
}