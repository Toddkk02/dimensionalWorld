#include "worldRenderer.h"
#include <string.h>
#include <raymath.h>

static Material CreateTexturedMaterial(Texture2D tex, Color fallback) {
    Material mat = LoadMaterialDefault();
    
    if (tex.id != 0) {
        // USA LA TEXTURE
        mat.maps[MATERIAL_MAP_DIFFUSE].texture = tex;
        mat.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
        TraceLog(LOG_INFO, "Material created with texture ID: %d", tex.id);
    } else {
        // FALLBACK COLORE
        mat.maps[MATERIAL_MAP_DIFFUSE].color = fallback;
        TraceLog(LOG_WARNING, "Material created with fallback color");
    }
    
    return mat;
}

void InitWorldRenderer(WorldRenderer* wr, DimensionConfig* dim) {
    memset(wr, 0, sizeof(WorldRenderer));
    
    if (!dim) {
        TraceLog(LOG_ERROR, "InitWorldRenderer: NULL dimension!");
        wr->initialized = false;
        return;
    }

    TraceLog(LOG_INFO, "=== INIT WORLD RENDERER FOR: %s ===", dim->name.c_str());
    
    // Crea materiali con texture se disponibili
    wr->grassMat = CreateTexturedMaterial(dim->grassTopTex, dim->grassTopColor);
    wr->dirtSideMat = CreateTexturedMaterial(dim->dirtSideTex, dim->dirtSideColor);
    wr->dirtMat = CreateTexturedMaterial(dim->dirtTex, dim->dirtColor);
    
    // Water con trasparenza
    if (dim->waterTex.id != 0) {
        wr->waterMat = CreateTexturedMaterial(dim->waterTex, (Color){30, 100, 255, 180});
    } else {
        // Crea texture procedurale per acqua
        Image img = GenImageColor(64, 64, (Color){30, 100, 255, 180});
        ImageDrawRectangleLines(&img, (Rectangle){0, 0, 64, 64}, 4, Fade(BLUE, 0.5f));
        Texture2D tex = LoadTextureFromImage(img);
        UnloadImage(img);
        wr->waterMat = CreateTexturedMaterial(tex, (Color){30, 100, 255, 180});
    }

    wr->initialized = true;
    wr->materialsLoaded = true;
    
    TraceLog(LOG_INFO, "=== WORLD RENDERER INITIALIZED ===");
}

void DrawWorld(WorldRenderer* wr, World* world) {
    if (!wr || !wr->initialized || !world) {
        TraceLog(LOG_WARNING, "DrawWorld: Invalid parameters!");
        return;
    }

    for (int i = 0; i < world->chunkCount; i++) {
        Chunk* c = &world->chunks[i];

        if (!c->meshGenerated || c->mesh.vertexCount == 0)
            continue;

        // DISEGNA CON TEXTURE/MATERIALI
        DrawMesh(c->mesh, wr->grassMat, MatrixIdentity());
    }
}

void UnloadWorldRenderer(WorldRenderer* wr) {
    if (!wr || !wr->initialized) return;

    TraceLog(LOG_INFO, "Unloading World Renderer...");

    if (wr->materialsLoaded) {
        UnloadMaterial(wr->grassMat);
        UnloadMaterial(wr->dirtSideMat);
        UnloadMaterial(wr->dirtMat);
        UnloadMaterial(wr->waterMat);
        wr->materialsLoaded = false;
    }

    wr->initialized = false;
}