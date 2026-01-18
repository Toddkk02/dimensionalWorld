#ifndef SKYBOX_H
#define SKYBOX_H

#include "raylib.h"

typedef struct {
    Texture2D texture;
    Mesh mesh;
} Skybox;

Skybox LoadSkybox(const char *imagePath);
void DrawSkybox(Skybox skybox, Camera3D camera);
void UnloadSkybox(Skybox skybox);

#endif
