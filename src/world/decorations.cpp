#include "decorations.h"
#include <raymath.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"   // Core Raylib
#include "raymath.h"
#include "../gameplay/dropped_item.h"
bool MineDecoration(DecorationSystem* ds, DecorationMiningState* mining, Camera3D cam, float deltaTime) {
    if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        mining->mining = false;
        mining->progress = 0.0f;
        return false;
    }
    
    // Raycast verso decorazione più vicina
    Vector3 dir = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
    float bestDist = 10.0f;
    int bestType = -1;
    int bestIndex = -1;
    
    // Check trees
    for (size_t i = 0; i < ds->trees.size(); i++) {
        float dist = Vector3Distance(cam.position, ds->trees[i].position);
        if (dist < bestDist) {
            Vector3 toTree = Vector3Subtract(ds->trees[i].position, cam.position);
            float dot = Vector3DotProduct(dir, Vector3Normalize(toTree));
            if (dot > 0.9f) { // 25° cone
                bestDist = dist;
                bestType = 0;
                bestIndex = i;
            }
        }
    }
    
    // Check rocks
    for (size_t i = 0; i < ds->rocks.size(); i++) {
        float dist = Vector3Distance(cam.position, ds->rocks[i].position);
        if (dist < bestDist) {
            Vector3 toRock = Vector3Subtract(ds->rocks[i].position, cam.position);
            float dot = Vector3DotProduct(dir, Vector3Normalize(toRock));
            if (dot > 0.9f) {
                bestDist = dist;
                bestType = 1;
                bestIndex = i;
            }
        }
    }
    
    if (bestType == -1) {
        mining->mining = false;
        mining->progress = 0.0f;
        return false;
    }
    
    // Mining in corso
    if (mining->mining && mining->targetType == bestType && mining->targetIndex == bestIndex) {
        mining->progress += deltaTime;
        
        if (mining->progress >= 3.0f) { // 3 secondi per minare
            Vector3 dropPos;
            ItemType dropType;
            
            if (bestType == 0) { // Tree
                dropPos = ds->trees[bestIndex].position;
                dropType = ItemType::WOOD;
                ds->trees.erase(ds->trees.begin() + bestIndex);
            } else { // Rock
                dropPos = ds->rocks[bestIndex].position;
                dropType = ItemType::STONE;
                ds->rocks.erase(ds->rocks.begin() + bestIndex);
            }
            
            SpawnDroppedItem(dropType, dropPos);
            mining->mining = false;
            mining->progress = 0.0f;
            return true;
        }
    } else {
        mining->mining = true;
        mining->targetType = bestType;
        mining->targetIndex = bestIndex;
        mining->progress = 0.0f;
    }
    
    return false;
}

Mesh CreateTreeMesh()
{
    return GenMeshCylinder(0.3f, 4.0f, 8);
}
inline bool CheckCollisionPointBoxCustom(Vector3 point, BoundingBox box)
{
    return (point.x >= box.min.x && point.x <= box.max.x) &&
           (point.y >= box.min.y && point.y <= box.max.y) &&
           (point.z >= box.min.z && point.z <= box.max.z);
}

Mesh CreateRockMesh(int seed)
{
    srand(seed);
    float radius = 0.5f + (rand() % 50) / 100.0f;
    int rings = 4 + rand() % 4;
    int slices = 4 + rand() % 4;
    return GenMeshSphere(radius, rings, slices);
}

Mesh CreateCrystalMesh(int seed)
{
    srand(seed);
    float height = 2.0f + (rand() % 100) / 50.0f;
    float radius = 0.3f + (rand() % 50) / 100.0f;
    int sides = 6 + rand() % 3;
    return GenMeshCylinder(radius, height, sides);
}

void CollisionWithDecoration(DecorationSystem* ds, PlayerSystem* player)
{
    if (!ds || !player) return;

    auto pushPlayerOut = [&](BoundingBox box, const char* name)
    {
        Vector3 pos = player->camera.position;

        if (CheckCollisionPointBoxCustom(pos, box))
        {
            Vector3 center = {
                (box.min.x + box.max.x) * 0.5f,
                (box.min.y + box.max.y) * 0.5f,
                (box.min.z + box.max.z) * 0.5f
            };

            float dx = pos.x - center.x;
            float dz = pos.z - center.z;

            float overlapX = (box.max.x - box.min.x) / 2 - fabsf(dx);
            float overlapZ = (box.max.z - box.min.z) / 2 - fabsf(dz);

            if (overlapX < overlapZ)
                pos.x += (dx > 0 ? overlapX : -overlapX);
            else
                pos.z += (dz > 0 ? overlapZ : -overlapZ);

            player->camera.position = pos;
        }
    };

    for (auto& tree : ds->trees)
    {
        float s = tree.scale;
        
        // TRONCO (cilindro alla base)
        BoundingBox trunk;
        trunk.min = { tree.position.x - s*0.3f, tree.position.y, tree.position.z - s*0.3f };
        trunk.max = { tree.position.x + s*0.3f, tree.position.y + 4.0f*s, tree.position.z + s*0.3f };
        pushPlayerOut(trunk, "Tree trunk");
        
        // CHIOMA (3 sfere sovrapposte)
        Vector3 foliagePos = tree.position;
        foliagePos.y += 3.0f * s;
        
        for (int j = 0; j < 3; j++)
        {
            float offset = (j - 1) * 0.8f * s;
            Vector3 leafPos = foliagePos;
            leafPos.y += offset;
            
            float leafRadius = 1.5f * s * (1.0f - abs((float)j - 1) * 0.2f);
            
            // Converti sfera in bounding box
            BoundingBox leafBox;
            leafBox.min = { leafPos.x - leafRadius, leafPos.y - leafRadius, leafPos.z - leafRadius };
            leafBox.max = { leafPos.x + leafRadius, leafPos.y + leafRadius, leafPos.z + leafRadius };
            pushPlayerOut(leafBox, "Tree foliage");
        }
    }

    for (auto& rock : ds->rocks)
    {
        BoundingBox box;
        float s = rock.scale;
        box.min = { rock.position.x - s/2, rock.position.y, rock.position.z - s/2 };
        box.max = { rock.position.x + s/2, rock.position.y + s, rock.position.z + s/2 };
        pushPlayerOut(box, "Rock");
    }

    for (auto& crystal : ds->crystals)
    {
        BoundingBox box;
        float s = crystal.scale;
        box.min = { crystal.position.x - s/2, crystal.position.y, crystal.position.z - s/2 };
        box.max = { crystal.position.x + s/2, crystal.position.y + s, crystal.position.z + s/2 };
        pushPlayerOut(box, "Crystal");
    }
}

void InitDecorationSystem(DecorationSystem *ds)
{
    ds->trees.clear();
    ds->rocks.clear();
    ds->crystals.clear();
    ds->hasModels = false;

    ds->treeModel = LoadModelFromMesh(CreateTreeMesh());
    ds->rockModel = LoadModelFromMesh(CreateRockMesh(42));
    ds->crystalModel = LoadModelFromMesh(CreateCrystalMesh(123));

    ds->hasModels = true;
}

Vector3 FindValidTerrainPosition(World *world, float minX, float maxX, float minZ, float maxZ)
{
    float x = minX + (rand() % (int)(maxX - minX));
    float z = minZ + (rand() % (int)(maxZ - minZ));
    float y = GetTerrainHeightAt(world, x, z);
    return {x, y, z};
}

void GenerateDecorationsForDimension(DecorationSystem *ds, World *world, DimensionConfig *dim)
{
    ds->trees.clear();
    ds->rocks.clear();
    ds->crystals.clear();

    srand((unsigned int)time(NULL) + dim->id);

    // ---------- ALBERI ----------
    if (dim->treeCount > 0)
    {
        for (int i = 0; i < dim->treeCount; i++)
        {
            TreeDecoration tree;
            tree.position = FindValidTerrainPosition(world, -50, 50, -50, 50);
            tree.scale = 1.5f + (rand() % 100) / 100.0f;
            tree.trunkColor = {101, 67, 33, 255};
            tree.foliageColor = dim->treeColor;
            ds->trees.push_back(tree);
        }
    }

    // ---------- ROCCE ----------
    if (dim->rockCount > 0)
    {
        for (int i = 0; i < dim->rockCount; i++)
        {
            RockDecoration rock;
            rock.position = FindValidTerrainPosition(world, -50, 50, -50, 50);
            rock.scale = 0.8f + (rand() % 120) / 100.0f;
            rock.color = dim->rockColor;
            rock.seed = rand();
            ds->rocks.push_back(rock);
        }
    }

    // ---------- CRISTALLI SOLO SE dim->crystalCount > 0 ----------
    if (dim->crystalCount > 0)
    {
        for (int i = 0; i < dim->crystalCount; i++)
        {
            CrystalDecoration crystal;
            crystal.position = FindValidTerrainPosition(world, -50, 50, -50, 50);
            crystal.position.y += 0.5f;
            crystal.scale = 1.0f + (rand() % 150) / 100.0f;
            crystal.rotation = (float)(rand() % 360);
            crystal.tiltAngle = 15.0f + (rand() % 30);
            crystal.tiltAxis = {(rand() % 100) / 100.0f - 0.5f, 0.0f, (rand() % 100) / 100.0f - 0.5f};
            crystal.tiltAxis = Vector3Normalize(crystal.tiltAxis);
            crystal.color = dim->crystalColor;
            crystal.seed = rand();
            crystal.glowing = true;
            ds->crystals.push_back(crystal);
        }
    }
}

    void DrawDecorations(DecorationSystem * ds)
    {
        // ---------- ALBERI ----------
        for (auto &tree : ds->trees)
        {
            // Tronco
            DrawModelEx(ds->treeModel, tree.position, {0, 1, 0}, 0, {tree.scale, tree.scale, tree.scale}, tree.trunkColor);

            // Chioma
            Vector3 foliagePos = tree.position;
            foliagePos.y += 3.0f * tree.scale; // posizionamento base chioma

            for (int j = 0; j < 3; j++)
            {
                float offset = (j - 1) * 0.8f * tree.scale;
                Vector3 leafPos = foliagePos;
                leafPos.y += offset;

                float leafRadius = 1.5f * tree.scale * (1.0f - abs((float)j - 1) * 0.2f);
                DrawSphere(leafPos, leafRadius, tree.foliageColor);
            }
        }
        for (auto &rock : ds->rocks)
        {
            DrawModelEx(ds->rockModel, rock.position, {0, 1, 0}, (float)(rock.seed % 360), {rock.scale, rock.scale, rock.scale}, rock.color);
        }

        for (auto &crystal : ds->crystals)
        {
            Matrix matRotY = MatrixRotateY(crystal.rotation * DEG2RAD);
            Matrix matTilt = MatrixRotate(crystal.tiltAxis, crystal.tiltAngle * DEG2RAD);
            Matrix matScale = MatrixScale(crystal.scale, crystal.scale, crystal.scale);
            Matrix matTrans = MatrixTranslate(crystal.position.x, crystal.position.y, crystal.position.z);

            Matrix transform = MatrixMultiply(matScale, matTilt);
            transform = MatrixMultiply(transform, matRotY);
            transform = MatrixMultiply(transform, matTrans);

            DrawMesh(ds->crystalModel.meshes[0], ds->crystalModel.materials[0], transform);
        }
    }



void CleanupDecorationSystem(DecorationSystem *ds)
{
    if (ds->hasModels)
    {
        UnloadModel(ds->treeModel);
        UnloadModel(ds->rockModel);
        UnloadModel(ds->crystalModel);
    }
    ds->trees.clear();
    ds->rocks.clear();
    ds->crystals.clear();
}
