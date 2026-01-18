#ifndef FIRSTWORLD_H
#define FIRSTWORLD_H

#include <raylib.h>

#define CHUNK_SIZE 16
#define MAX_CHUNKS 256
#define MAX_HEIGHT 32
#define RENDER_DISTANCE 3

#define WATER_LEVEL 4.0f

typedef enum BlockType
{
    BLOCK_AIR,
    BLOCK_DIRT,
    BLOCK_WATER
} BlockType;

typedef struct Chunk
{
    int chunkX, chunkZ; // Coordinate del chunk
    bool generated;     // Generato?
    bool meshGenerated;

    float heightMap[CHUNK_SIZE + 1][CHUNK_SIZE + 1]; 
     float liquidMap[CHUNK_SIZE + 1][CHUNK_SIZE + 1]; 
    Mesh mesh;                                       
 } Chunk;

typedef struct World
{
    Chunk chunks[MAX_CHUNKS];
    int chunkCount;
} World;

// Funzioni del mondo
void WorldInit(World *world);
void WorldUpdate(World *world, Vector3 playerPos);
void WorldDraw(World *world);
float GetTerrainHeightAt(World *world, float x, float z);
void WorldCleanup(World* world);
BlockType GetBlockAt(World *world, int x, int y, int z);

#endif
