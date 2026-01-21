#pragma once
#include "raylib.h"
#include "blockTypes.h"
#include <string>
#include <array>
#include <vector>

struct DimensionConfig {
    int id{};
    std::string name;
    std::string description;

    Color grassTopColor{};
    Color dirtSideColor{};
    Color dirtColor{};

    std::string grassTopTexture;
    std::string dirtSideTexture;
    std::string dirtTexture;

    Texture2D grassTopTex{}; 
    Texture2D dirtSideTex{};
    Texture2D dirtTex{};
    
    bool grassTopLoaded{false};
    bool dirtSideLoaded{false};
    bool dirtLoaded{false};

    std::array<std::string, BLOCK_COUNT> blockTexturePaths;
    std::string waterTexture;
    Texture2D waterTex{};
    bool waterLoaded{false};

    std::string skyboxTexture;
    Texture2D skyboxTex{};
    Color skyboxTint{};
    bool skyboxLoaded{false};

    int terrainSeed{};
    float terrainScale{};
    float terrainHeight{};
    float waterLevel{};

    int treeCount{};
    int rockCount{};
    int crystalCount{};

    Color treeColor{};
    Color rockColor{};
    Color crystalColor{};

    Color ambientLight{};
    Color sunColor{};
    Vector3 sunDirection{};

    bool useFog{};
    Color fogColor{};
    float fogDensity{};

    DimensionConfig(); // dichiarazione solo
    void UnloadTextures();
    bool IsTextureLoaded(const Texture2D& tex) const;
};



class DimensionManager {
public:
    std::vector<DimensionConfig> dimensions;
    int currentDimensionID{0};

    void Initialize();
    void AddDimension(const DimensionConfig& config);
    DimensionConfig* GetDimension(int id);
    DimensionConfig* GetCurrentDimension();
    int GetDimensionCount();

    void LoadDimensionTextures(DimensionConfig* dim);
    void UnloadDimensionTextures(DimensionConfig* dim);
    void Cleanup();

    static DimensionConfig CreatePurpleDimension();
    static DimensionConfig CreateGreenDimension();
    static DimensionConfig CreateRedDimension();
    static DimensionConfig CreateDesertDimension();
    static DimensionConfig CreateIceDimension();
    static DimensionConfig CreateVolcanoDimension();
    static DimensionConfig CreateSpaceDimension();
};
