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
    int chunkX, chunkZ;
    bool generated;
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
void WorldCleanup(World* world);

// Funzioni di query
float GetTerrainHeightAt(World *world, float x, float z);
BlockType GetBlockAt(World *world, int x, int y, int z);

// Funzioni per gestire le dimensioni
void SetWorldDimension(int dimension);
void SetDimensionColors(Color grassTop, Color dirtSide, Color dirt);

// Funzione per rigenerare i chunk (quando cambi dimensione)
void RegenerateAllChunks(World* world);

#endif