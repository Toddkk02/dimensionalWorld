#ifndef WORLD_RENDERER_H
#define WORLD_RENDERER_H

#include "raylib.h"
#include "blockTypes.h"    // Include base types
#include "firstWorld.h"     // Include World struct
#include "dimensions.h"     // Include DimensionConfig

typedef struct WorldRenderer {
    Material grassMat;
    Material dirtSideMat;
    Material dirtMat;
    Material waterMat;
    bool initialized;
    bool materialsLoaded;
} WorldRenderer;

void InitWorldRenderer(WorldRenderer* wr, DimensionConfig* dim);
void DrawWorld(WorldRenderer* wr, World* world);
void UnloadWorldRenderer(WorldRenderer* wr);

#endif