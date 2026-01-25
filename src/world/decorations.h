#ifndef DECORATIONS_H
#define DECORATIONS_H

#include "raylib.h"
#include "../world/firstWorld.h"      // ← CAMBIA QUI
#include "../world/dimensions.h"
#include <vector>
#include "../core/player.h"

struct DecorationMiningState {
    bool mining;
    int targetType; // 0=tree, 1=rock, 2=crystal
    int targetIndex;
    float progress;
};

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
    int seed;
} RockDecoration;

typedef struct CrystalDecoration {
    Vector3 position;
    float scale;
    float rotation;
    float tiltAngle;
    Vector3 tiltAxis;
    Color color;
    int seed;
    bool glowing;
} CrystalDecoration;

typedef struct DecorationSystem {
    std::vector<TreeDecoration> trees;
    std::vector<RockDecoration> rocks;
    std::vector<CrystalDecoration> crystals;
    Model treeModel;
    Model rockModel;
    Model crystalModel;
    bool hasModels;
} DecorationSystem;

void InitDecorationSystem(DecorationSystem* ds);
void GenerateDecorationsForDimension(DecorationSystem* ds, World* world, DimensionConfig* dimension);
void DrawDecorations(DecorationSystem* ds);
void CleanupDecorationSystem(DecorationSystem* ds);
void CollisionWithDecoration(DecorationSystem* ds, PlayerSystem* player);
Mesh CreateTreeMesh();
Mesh CreateRockMesh(int seed);
Mesh CreateCrystalMesh(int seed);
bool MineDecoration(DecorationSystem* ds, DecorationMiningState* mining, Camera3D cam, float deltaTime);
#endif