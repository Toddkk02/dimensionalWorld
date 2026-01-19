#include "skybox.h"
#include "../world/dimensions.h"  // ← CAMBIA QUI
#include "raylib.h"
#include <math.h>

Skybox LoadSkybox(const char *imagePath, Color tint) {
    Skybox skybox = {};
    
    if (FileExists(imagePath)) {
        skybox.texture = LoadTexture(imagePath);
    } else {
        Image img = GenImageColor(512, 512, tint);
        ImageDrawRectangleLines(&img, (Rectangle){0, 0, 512, 512}, 5, Fade(tint, 0.5f));
        skybox.texture = LoadTextureFromImage(img);
        UnloadImage(img);
    }
    
    skybox.tint = tint;
    skybox.mesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    return skybox;
}

Skybox LoadSkyboxFromDimension(DimensionConfig* dimension) {
    return LoadSkybox(dimension->skyboxTexture.c_str(), dimension->skyboxTint);
}

void DrawSkybox(Skybox skybox, Camera3D camera) {
    (void)camera;
    
    float screenWidth = (float)GetScreenWidth();
    float screenHeight = (float)GetScreenHeight();
    float scale = fmaxf(screenWidth / skybox.texture.width, 
                       screenHeight / skybox.texture.height);
    
    Rectangle source = {0, 0, (float)skybox.texture.width, (float)skybox.texture.height};
    Rectangle dest = {
        (screenWidth - skybox.texture.width * scale) / 2,
        (screenHeight - skybox.texture.height * scale) / 2,
        skybox.texture.width * scale,
        skybox.texture.height * scale
    };
    
    DrawTexturePro(skybox.texture, source, dest, (Vector2){0, 0}, 0.0f, skybox.tint);
}

void UnloadSkybox(Skybox skybox) {
    UnloadTexture(skybox.texture);
    UnloadMesh(skybox.mesh);
}