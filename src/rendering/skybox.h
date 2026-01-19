#ifndef SKYBOX_H
#define SKYBOX_H

#include "raylib.h"

// Forward declaration - NON includere dimensions.h qui!
struct DimensionConfig;

typedef struct {
    Texture2D texture;
    Mesh mesh;
    Color tint;
} Skybox;

Skybox LoadSkybox(const char *imagePath, Color tint);
Skybox LoadSkyboxFromDimension(struct DimensionConfig* dimension);  // ← Usa struct
void DrawSkybox(Skybox skybox, Camera3D camera);
void UnloadSkybox(Skybox skybox);

#endif