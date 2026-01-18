#include "skybox.h"
#include "raylib.h"
#include <math.h>

Skybox LoadSkybox(const char *imagePath) {
    Skybox skybox = {};
    skybox.texture = LoadTexture(imagePath);
    skybox.mesh = GenMeshCube(1.0f, 1.0f, 1.0f); // non usato ma manteniamo la struttura
    return skybox;
}

void DrawSkybox(Skybox skybox, Camera3D camera) {
    // Disegna la texture come background prima di BeginMode3D
    // Questa funzione ora deve essere chiamata PRIMA di BeginMode3D
    (void)camera; // unused
    
    // Scala la texture per riempire lo schermo mantenendo proporzioni
    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();
    float scale = fmaxf(screenWidth / skybox.texture.width, 
                       screenHeight / skybox.texture.height);
    
    Rectangle source = {0, 0, (float)skybox.texture.width, (float)skybox.texture.height};
    Rectangle dest = {
        (screenWidth - skybox.texture.width * scale) / 2,
        (screenHeight - skybox.texture.height * scale) / 2,
        skybox.texture.width * scale,
        skybox.texture.height * scale
    };
    
    DrawTexturePro(skybox.texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
}

void UnloadSkybox(Skybox skybox) {
    UnloadTexture(skybox.texture);
    UnloadMesh(skybox.mesh);
}
