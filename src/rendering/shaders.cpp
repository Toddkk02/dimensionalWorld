#include "shaders.h"

Shader terrainShader;  // definizione reale della variabile globale

void LoadTerrainShader() {
    terrainShader = LoadShader("assets/shaders/terrain.vs", "assets/shaders/terrain.fs");

    // Imposta luce direzionale
    float dir[3] = {-0.5f, -1.0f, -0.5f};
    SetShaderValue(terrainShader, GetShaderLocation(terrainShader, "lightDir"), dir, SHADER_UNIFORM_VEC3);
}

void UnloadTerrainShader() {
    UnloadShader(terrainShader);
}
