#ifndef DECORATIONS_H
#define DECORATIONS_H

#include "raylib.h"
#include "world/firstWorld.h"
#include "core/portal.h"
#include <vector>

typedef struct TreeDecoration {
    Vector3 position;
    float scale;
    Color trunkColor;
    Color foliageColor;
} TreeDecoration;

typedef struct RockDecoration {
    Vector3 position;
    float scale;
    Color color;
    int seed;  // Per variazione forma
} RockDecoration;

typedef struct DecorationSystem {
    std::vector<TreeDecoration> trees;
    std::vector<RockDecoration> rocks;
    Model treeModel;
    Model rockModel;
    bool hasModels;
} DecorationSystem;

// Funzioni principali
void InitDecorationSystem(DecorationSystem* ds);
void GenerateDecorationsForDimension(DecorationSystem* ds, World* world, DimensionType dimension);
void DrawDecorations(DecorationSystem* ds);
void CleanupDecorationSystem(DecorationSystem* ds);

// Funzioni helper per creare mesh procedurali
Mesh CreateTreeMesh();
Mesh CreateRockMesh(int seed);

#endif