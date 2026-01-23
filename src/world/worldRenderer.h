#ifndef WORLD_RENDERER_H
#define WORLD_RENDERER_H

#include "raylib.h"
#include "blockTypes.h"
#include "firstWorld.h"
#include "dimensions.h"

typedef struct WorldRenderer {
    Material grassMat;
    Material dirtSideMat;
    Material dirtMat;
    Material waterMat;
    
    Shader fogShader;
    int fogDensityLoc;
    int fogColorLoc;
    int viewPosLoc;
    
    bool initialized;
    bool materialsLoaded;
} WorldRenderer;

void InitWorldRenderer(WorldRenderer* wr, DimensionConfig* dim);
void DrawWorld(WorldRenderer* wr, World* world, Camera3D camera, float fogDensity, Color fogColor);
void UnloadWorldRenderer(WorldRenderer* wr);

#endif