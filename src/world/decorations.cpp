#include "decorations.h"
#include <raymath.h>
#include <stdlib.h>
#include <time.h>

Mesh CreateTreeMesh() {
    Mesh trunk = GenMeshCylinder(0.3f, 4.0f, 8);
    return trunk;
}

Mesh CreateRockMesh(int seed) {
    srand(seed);
    float radius = 0.5f + (rand() % 50) / 100.0f;
    int rings = 4 + rand() % 4;
    int slices = 4 + rand() % 4;
    Mesh rock = GenMeshSphere(radius, rings, slices);
    return rock;
}

Mesh CreateCrystalMesh(int seed) {
    srand(seed);
    float height = 2.0f + (rand() % 100) / 50.0f;
    float radius = 0.3f + (rand() % 50) / 100.0f;
    int sides = 6 + rand() % 3;
    Mesh crystal = GenMeshCylinder(radius, height, sides);
    return crystal;
}

void InitDecorationSystem(DecorationSystem* ds) {
    ds->trees.clear();
    ds->rocks.clear();
    ds->crystals.clear();
    ds->hasModels = false;
    
    if (FileExists("assets/models/tree.obj") || FileExists("assets/models/tree.glb")) {
        const char* path = FileExists("assets/models/tree.obj") ? 
                          "assets/models/tree.obj" : "assets/models/tree.glb";
        ds->treeModel = LoadModel(path);
    } else {
        Mesh treeMesh = CreateTreeMesh();
        ds->treeModel = LoadModelFromMesh(treeMesh);
    }
    
    Mesh rockMesh = CreateRockMesh(42);
    ds->rockModel = LoadModelFromMesh(rockMesh);
    
    Mesh crystalMesh = CreateCrystalMesh(123);
    ds->crystalModel = LoadModelFromMesh(crystalMesh);
    
    ds->hasModels = true;
}

Vector3 FindValidTerrainPosition(World* world, float minX, float maxX, float minZ, float maxZ) {
    float x = minX + (rand() % (int)(maxX - minX));
    float z = minZ + (rand() % (int)(maxZ - minZ));
    float y = GetTerrainHeightAt(world, x, z);
    return (Vector3){x, y, z};
}

void GenerateDecorationsForDimension(DecorationSystem* ds, World* world, DimensionConfig* dimension) {
    ds->trees.clear();
    ds->rocks.clear();
    ds->crystals.clear();
    
    srand((unsigned int)time(NULL) + dimension->id);
    
    // ALBERI
    for (int i = 0; i < dimension->treeCount; i++) {
        TreeDecoration tree;
        tree.position = FindValidTerrainPosition(world, -50, 50, -50, 50);
        tree.scale = 1.5f + (rand() % 100) / 100.0f;
        tree.trunkColor = (Color){101, 67, 33, 255};
        tree.foliageColor = dimension->treeColor;
        ds->trees.push_back(tree);
    }
    
    // ROCCE
    for (int i = 0; i < dimension->rockCount; i++) {
        RockDecoration rock;
        rock.position = FindValidTerrainPosition(world, -50, 50, -50, 50);
        rock.scale = 0.8f + (rand() % 120) / 100.0f;
        rock.color = dimension->rockColor;
        rock.seed = rand();
        ds->rocks.push_back(rock);
    }
    
    // CRISTALLI
    for (int i = 0; i < dimension->crystalCount; i++) {
        CrystalDecoration crystal;
        crystal.position = FindValidTerrainPosition(world, -50, 50, -50, 50);
        crystal.position.y += 0.5f;
        
        crystal.scale = 1.0f + (rand() % 150) / 100.0f;
        crystal.rotation = (float)(rand() % 360);
        
        crystal.tiltAngle = 15.0f + (rand() % 30);
        crystal.tiltAxis = (Vector3){
            (rand() % 100) / 100.0f - 0.5f,
            0.0f,
            (rand() % 100) / 100.0f - 0.5f
        };
        crystal.tiltAxis = Vector3Normalize(crystal.tiltAxis);
        
        crystal.color = dimension->crystalColor;
        crystal.seed = rand();
        crystal.glowing = true;
        
        ds->crystals.push_back(crystal);
    }
}

void DrawDecorations(DecorationSystem* ds) {
    // ALBERI
    for (size_t i = 0; i < ds->trees.size(); i++) {
        TreeDecoration* tree = &ds->trees[i];
        
        DrawModelEx(ds->treeModel, tree->position,
                   (Vector3){0, 1, 0}, 0,
                   (Vector3){tree->scale, tree->scale, tree->scale},
                   tree->trunkColor);
        
        Vector3 foliagePos = tree->position;
        foliagePos.y += 3.0f * tree->scale;
        
        for (int j = 0; j < 3; j++) {
            float offset = (j - 1) * 0.8f * tree->scale;
            Vector3 leafPos = foliagePos;
            leafPos.y += offset;
            
            float leafRadius = 1.5f * tree->scale * (1.0f - abs((float)j - 1) * 0.2f);
            DrawSphere(leafPos, leafRadius, tree->foliageColor);
        }
    }
    
    // ROCCE
    for (size_t i = 0; i < ds->rocks.size(); i++) {
        RockDecoration* rock = &ds->rocks[i];
        DrawModelEx(ds->rockModel, rock->position,
                   (Vector3){0, 1, 0}, (float)(rock->seed % 360),
                   (Vector3){rock->scale, rock->scale, rock->scale},
                   rock->color);
    }
    
    // CRISTALLI
    for (size_t i = 0; i < ds->crystals.size(); i++) {
        CrystalDecoration* crystal = &ds->crystals[i];
        
        Matrix matRotY = MatrixRotateY(crystal->rotation * DEG2RAD);
        Matrix matTilt = MatrixRotate(crystal->tiltAxis, crystal->tiltAngle * DEG2RAD);
        Matrix matScale = MatrixScale(crystal->scale, crystal->scale, crystal->scale);
        Matrix matTrans = MatrixTranslate(crystal->position.x, 
                                         crystal->position.y, 
                                         crystal->position.z);
        
        Matrix transform = MatrixMultiply(matScale, matTilt);
        transform = MatrixMultiply(transform, matRotY);
        transform = MatrixMultiply(transform, matTrans);
        
        if (crystal->glowing) {
            Color glowColor = crystal->color;
            glowColor.a = 80;
            
            Matrix glowTransform = MatrixScale(crystal->scale * 1.2f, 
                                              crystal->scale * 1.2f, 
                                              crystal->scale * 1.2f);
            glowTransform = MatrixMultiply(glowTransform, matTilt);
            glowTransform = MatrixMultiply(glowTransform, matRotY);
            glowTransform = MatrixMultiply(glowTransform, matTrans);
            
            DrawMesh(ds->crystalModel.meshes[0], 
                    ds->crystalModel.materials[0], 
                    glowTransform);
        }
        
        DrawMesh(ds->crystalModel.meshes[0], 
                ds->crystalModel.materials[0], 
                transform);
        
        if (crystal->glowing && (rand() % 10) < 3) {
            Vector3 particlePos = crystal->position;
            particlePos.x += (rand() % 100 - 50) / 100.0f;
            particlePos.y += (rand() % 200) / 100.0f;
            particlePos.z += (rand() % 100 - 50) / 100.0f;
            
            DrawSphere(particlePos, 0.05f, Fade(crystal->color, 0.6f));
        }
    }
}

void CleanupDecorationSystem(DecorationSystem* ds) {
    if (ds->hasModels) {
        UnloadModel(ds->treeModel);
        UnloadModel(ds->rockModel);
        UnloadModel(ds->crystalModel);
    }
    ds->trees.clear();
    ds->rocks.clear();
    ds->crystals.clear();
}