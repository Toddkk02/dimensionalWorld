#ifndef DIMENSIONS_H
#define DIMENSIONS_H

#include "raylib.h"
#include <string>
#include <vector>

// RIMUOVI completamente questa riga:
// struct Skybox;  ← CANCELLA QUESTA!

typedef struct DimensionConfig {
    int id;
    std::string name;
    std::string description;
    
    Color grassTopColor;
    Color dirtSideColor;
    Color dirtColor;
    
    std::string grassTopTexture;
    std::string dirtSideTexture;
    std::string dirtTexture;
    std::string waterTexture;
    
    std::string skyboxTexture;
    Color skyboxTint;
    
    int terrainSeed;
    float terrainScale;
    float terrainHeight;
    float waterLevel;
    
    int treeCount;
    int rockCount;
    int crystalCount;
    
    Color treeColor;
    Color rockColor;
    Color crystalColor;
    
    Color ambientLight;
    Color sunColor;
    Vector3 sunDirection;
    
    bool useFog;
    Color fogColor;
    float fogDensity;
    

} DimensionConfig;

class DimensionManager {
public:
    std::vector<DimensionConfig> dimensions;
    int currentDimensionID;
    
    void Initialize();
    void AddDimension(const DimensionConfig& config);
    DimensionConfig* GetDimension(int id);
    DimensionConfig* GetCurrentDimension();
    int GetDimensionCount();
    
    static DimensionConfig CreatePurpleDimension();
    static DimensionConfig CreateGreenDimension();
    static DimensionConfig CreateRedDimension();
    static DimensionConfig CreateDesertDimension();
    static DimensionConfig CreateIceDimension();
    static DimensionConfig CreateVolcanoDimension();
    static DimensionConfig CreateSpaceDimension();
};
#endif