#ifndef FIRSTWORLD_H
#define FIRSTWORLD_H

#include "raylib.h"

#define CHUNK_SIZE 16  // Aumentato per terreno più fluido
#define MAX_CHUNKS 128
#define RENDER_DISTANCE 3

typedef enum {
    BLOCK_AIR = 0,
    BLOCK_DIRT
} BlockType;

typedef struct {
    int chunkX;
    int chunkZ;
    float heightMap[CHUNK_SIZE + 1][CHUNK_SIZE + 1];
    Mesh mesh;
    Material material;  // <- AGGIUNGI QUESTO
    bool generated;
    bool meshGenerated;
} Chunk;

typedef struct {
    Chunk chunks[MAX_CHUNKS];
    int chunkCount;
} World;

void WorldInit(World* world);
void WorldUpdate(World* world, Vector3 playerPos);
void WorldDraw(World* world);

BlockType GetBlockAt(World* world, int x, int y, int z);
float GetTerrainHeightAt(World* world, float x, float z);

#endif
