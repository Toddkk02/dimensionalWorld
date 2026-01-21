#include "decorations.h"
#include <raymath.h>
#include <stdlib.h>
#include <time.h>

Mesh CreateTreeMesh()
{
    return GenMeshCylinder(0.3f, 4.0f, 8);
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
