#include "worldRenderer.h"
#include <string.h>
#include <raymath.h>

static Material CreateTexturedMaterial(Texture2D tex, Color fallback, Shader fogShader) {
    Material mat = LoadMaterialDefault();
    
    // Applica fog shader al materiale
    if (fogShader.id > 0) {
        mat.shader = fogShader;
    }
    
    if (tex.id != 0) {
        mat.maps[MATERIAL_MAP_DIFFUSE].texture = tex;
        mat.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
        TraceLog(LOG_INFO, "Material created with texture ID: %d and fog shader", tex.id);
    } else {
        mat.maps[MATERIAL_MAP_DIFFUSE].color = fallback;
        TraceLog(LOG_WARNING, "Material created with fallback color and fog shader");
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
    
    // Carica fog shader
    wr->fogShader = LoadShader("assets/shaders/glsl330/fog_vertex.vs", 
                               "assets/shaders/glsl330/fog.fs");
    
    if (wr->fogShader.id > 0) {
        wr->fogDensityLoc = GetShaderLocation(wr->fogShader, "fogDensity");
        wr->fogColorLoc = GetShaderLocation(wr->fogShader, "fogColor");
        wr->viewPosLoc = GetShaderLocation(wr->fogShader, "viewPos");
        
        wr->fogShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(wr->fogShader, "mvp");
        wr->fogShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(wr->fogShader, "matModel");
        wr->fogShader.locs[SHADER_LOC_MATRIX_NORMAL] = GetShaderLocation(wr->fogShader, "matNormal");
        wr->fogShader.locs[SHADER_LOC_VECTOR_VIEW] = wr->viewPosLoc;
        
        TraceLog(LOG_INFO, "✓ Fog shader loaded into WorldRenderer (ID: %d)", wr->fogShader.id);
    } else {
        TraceLog(LOG_ERROR, "✗ Failed to load fog shader in WorldRenderer!");
    }
    
    // Crea materiali CON fog shader
    wr->grassMat = CreateTexturedMaterial(dim->grassTopTex, dim->grassTopColor, wr->fogShader);
    wr->dirtSideMat = CreateTexturedMaterial(dim->dirtSideTex, dim->dirtSideColor, wr->fogShader);
    wr->dirtMat = CreateTexturedMaterial(dim->dirtTex, dim->dirtColor, wr->fogShader);
    
    if (dim->waterTex.id != 0) {
        wr->waterMat = CreateTexturedMaterial(dim->waterTex, (Color){30, 100, 255, 180}, wr->fogShader);
    } else {
        Image img = GenImageColor(64, 64, (Color){30, 100, 255, 180});
        ImageDrawRectangleLines(&img, (Rectangle){0, 0, 64, 64}, 4, Fade(BLUE, 0.5f));
        Texture2D tex = LoadTextureFromImage(img);
        UnloadImage(img);
        wr->waterMat = CreateTexturedMaterial(tex, (Color){30, 100, 255, 180}, wr->fogShader);
    }

    wr->initialized = true;
    wr->materialsLoaded = true;
    
    TraceLog(LOG_INFO, "=== WORLD RENDERER INITIALIZED ===");
}

void DrawWorld(WorldRenderer* wr, World* world, Camera3D camera, float fogDensity, Color fogColor) {
    if (!wr || !wr->initialized || !world) {
        TraceLog(LOG_WARNING, "DrawWorld: Invalid parameters!");
        return;
    }

    // Update fog uniforms
    if (wr->fogShader.id > 0) {
        SetShaderValue(wr->fogShader, wr->fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);
        
        float fogColorNorm[4] = {
            fogColor.r / 255.0f,
            fogColor.g / 255.0f,
            fogColor.b / 255.0f,
            1.0f
        };
        SetShaderValue(wr->fogShader, wr->fogColorLoc, fogColorNorm, SHADER_UNIFORM_VEC4);
        
        float camPos[3] = {camera.position.x, camera.position.y, camera.position.z};
        SetShaderValue(wr->fogShader, wr->viewPosLoc, camPos, SHADER_UNIFORM_VEC3);
    }

    for (int i = 0; i < world->chunkCount; i++) {
        Chunk* c = &world->chunks[i];

        if (!c->meshGenerated || c->mesh.vertexCount == 0)
            continue;

        DrawMesh(c->mesh, wr->grassMat, MatrixIdentity());
    }
}

void UnloadWorldRenderer(WorldRenderer* wr) {
    if (!wr || !wr->initialized) return;

    TraceLog(LOG_INFO, "Unloading World Renderer...");

    if (wr->materialsLoaded) {
        // NON unloadare i materiali perché hanno reference allo shader
        // Unloada solo lo shader DOPO
        wr->materialsLoaded = false;
    }
    
    // Unload shader SOLO UNA VOLTA
    if (wr->fogShader.id > 0) {
        UnloadShader(wr->fogShader);
        wr->fogShader.id = 0;
        TraceLog(LOG_INFO, "✓ Fog shader unloaded");
    }

    wr->initialized = false;
}