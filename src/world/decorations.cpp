#include "decorations.h"
#include <raymath.h>
#include <stdlib.h>
#include <time.h>

Mesh CreateTreeMesh() {
    // Crea un albero stilizzato semplice
    // Tronco: cilindro
    Mesh trunk = GenMeshCylinder(0.3f, 4.0f, 8);
    return trunk;
}

Mesh CreateRockMesh(int seed) {
    // Usa il seed per variare la forma
    srand(seed);
    float radius = 0.5f + (rand() % 50) / 100.0f;
    int rings = 4 + rand() % 4;
    int slices = 4 + rand() % 4;
    
    Mesh rock = GenMeshSphere(radius, rings, slices);
    return rock;
}

void InitDecorationSystem(DecorationSystem* ds) {
    ds->trees.clear();
    ds->rocks.clear();
    ds->hasModels = false;
    

    // Prova a caricare modelli se esistono
    if (FileExists("assets/models/tree.obj") || FileExists("assets/models/tree.glb")) {
        const char* path = FileExists("assets/models/tree.obj") ? 
                          "assets/models/tree.obj" : "assets/models/tree.glb";
        ds->treeModel = LoadModel(path);
        ds->hasModels = true;
    } else {
        // Crea modello procedurale
        Mesh treeMesh = CreateTreeMesh();
        ds->treeModel = LoadModelFromMesh(treeMesh);
        ds->hasModels = true;
    }
    
    // Rock model procedurale
    Mesh rockMesh = CreateRockMesh(42);
    ds->rockModel = LoadModelFromMesh(rockMesh);
}

Vector3 FindValidTerrainPosition(World* world, float minX, float maxX, float minZ, float maxZ) {
    // Trova una posizione valida sul terreno
    float x = minX + (rand() % (int)(maxX - minX));
    float z = minZ + (rand() % (int)(maxZ - minZ));
    float y = GetTerrainHeightAt(world, x, z);
    
    return (Vector3){x, y, z};
}

void GenerateDecorationsForDimension(DecorationSystem* ds, World* world, DimensionType dimension) {
    ds->trees.clear();
    ds->rocks.clear();
    
    srand((unsigned int)time(NULL) + (int)dimension);
    
    switch(dimension) {
        case DIMENSION_PURPLE:
            // Dimensione viola: poche rocce viola/magenta
            for (int i = 0; i < 15; i++) {
                RockDecoration rock;
                rock.position = FindValidTerrainPosition(world, -50, 50, -50, 50);
                rock.scale = 1.0f + (rand() % 100) / 50.0f;
                rock.color = (Color){
                    (unsigned char)(150 + rand() % 105),
                    (unsigned char)(50 + rand() % 50),
                    (unsigned char)(200 + rand() % 55),
                    255
                };
                rock.seed = rand();
                ds->rocks.push_back(rock);
            }
            break;
            
        case DIMENSION_GREEN:
            // Dimensione verde: molti alberi verdi
            for (int i = 0; i < 40; i++) {
                TreeDecoration tree;
                tree.position = FindValidTerrainPosition(world, -50, 50, -50, 50);
                tree.scale = 1.5f + (rand() % 100) / 100.0f;
                tree.trunkColor = (Color){101, 67, 33, 255};  // Marrone
                tree.foliageColor = (Color){
                    (unsigned char)(30 + rand() % 50),
                    (unsigned char)(150 + rand() % 105),
                    (unsigned char)(30 + rand() % 50),
                    255
                };
                ds->trees.push_back(tree);
            }
            
            // Alcune rocce verdi
            for (int i = 0; i < 10; i++) {
                RockDecoration rock;
                rock.position = FindValidTerrainPosition(world, -50, 50, -50, 50);
                rock.scale = 0.8f + (rand() % 80) / 100.0f;
                rock.color = (Color){80, 120, 80, 255};
                rock.seed = rand();
                ds->rocks.push_back(rock);
            }
            break;
            
        case DIMENSION_RED:
            // Dimensione rossa: rocce rosse/arancioni alien
            for (int i = 0; i < 25; i++) {
                RockDecoration rock;
                rock.position = FindValidTerrainPosition(world, -50, 50, -50, 50);
                rock.scale = 1.2f + (rand() % 150) / 100.0f;
                rock.color = (Color){
                    (unsigned char)(200 + rand() % 55),
                    (unsigned char)(30 + rand() % 70),
                    (unsigned char)(10 + rand() % 40),
                    255
                };
                rock.seed = rand();
                ds->rocks.push_back(rock);
            }
            
            // "Alberi" rossi alien (cristalli)
            for (int i = 0; i < 15; i++) {
                TreeDecoration crystal;
                crystal.position = FindValidTerrainPosition(world, -50, 50, -50, 50);
                crystal.scale = 2.0f + (rand() % 100) / 50.0f;
                crystal.trunkColor = (Color){150, 30, 30, 255};
                crystal.foliageColor = (Color){255, 100, 50, 255};
                ds->trees.push_back(crystal);
            }
            break;
            
        default:
            break;
    }
}

void DrawDecorations(DecorationSystem* ds) {
    // Disegna gli alberi
    for (size_t i = 0; i < ds->trees.size(); i++) {
        TreeDecoration* tree = &ds->trees[i];
        
        // Tronco
        DrawModelEx(ds->treeModel, tree->position,
                   (Vector3){0, 1, 0}, 0,
                   (Vector3){tree->scale, tree->scale, tree->scale},
                   tree->trunkColor);
        
        // Fogliame (sfera o cono sopra il tronco)
        Vector3 foliagePos = tree->position;
        foliagePos.y += 3.0f * tree->scale;
        
        // Disegna fogliame come sfere
        for (int j = 0; j < 3; j++) {
            float offset = (j - 1) * 0.8f * tree->scale;
            Vector3 leafPos = foliagePos;
            leafPos.y += offset;
            
            float leafRadius = 1.5f * tree->scale * (1.0f - abs((float)j - 1) * 0.2f);
            DrawSphere(leafPos, leafRadius, tree->foliageColor);
        }
    }
    
    // Disegna le rocce
    for (size_t i = 0; i < ds->rocks.size(); i++) {
        RockDecoration* rock = &ds->rocks[i];
        
        // Usa mesh variata in base al seed
        DrawModelEx(ds->rockModel, rock->position,
                   (Vector3){0, 1, 0}, (float)(rock->seed % 360),
                   (Vector3){rock->scale, rock->scale, rock->scale},
                   rock->color);
    }
}

void CleanupDecorationSystem(DecorationSystem* ds) {
    if (ds->hasModels) {
        UnloadModel(ds->treeModel);
        UnloadModel(ds->rockModel);
    }
    ds->trees.clear();
    ds->rocks.clear();
}