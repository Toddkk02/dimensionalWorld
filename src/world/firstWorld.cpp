#include "firstWorld.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
#include <math.h>
#include <stdlib.h>
#include <raymath.h>
#include <string.h>
#include <vector>

// Struttura per rappresentare un blocco 3D
struct Block {
    int x, y, z;
    bool active;
};

static Chunk* WorldGetChunk(World* world, int cx, int cz) {
    for (int i = 0; i < world->chunkCount; i++) {
        if (world->chunks[i].chunkX == cx && world->chunks[i].chunkZ == cz) {
            return &world->chunks[i];
        }
    }
    if (world->chunkCount >= MAX_CHUNKS) return NULL;
    
    Chunk* c = &world->chunks[world->chunkCount++];
    c->chunkX = cx;
    c->chunkZ = cz;
    c->generated = false;
    c->meshGenerated = false;
    return c;
}

static void GenerateChunk(Chunk* c) {
    for (int x = 0; x <= CHUNK_SIZE; x++) {
        for (int z = 0; z <= CHUNK_SIZE; z++) {
            float wx = (float)(c->chunkX * CHUNK_SIZE + x);
            float wz = (float)(c->chunkZ * CHUNK_SIZE + z);
            
            float height = stb_perlin_noise3(wx * 0.02f, 0, wz * 0.02f, 0, 0, 0) * 8.0f +
                          stb_perlin_noise3(wx * 0.05f, 10, wz * 0.05f, 0, 0, 0) * 4.0f +
                          stb_perlin_noise3(wx * 0.1f, 20, wz * 0.1f, 0, 0, 0) * 2.0f + 5.0f;
            
            c->heightMap[x][z] = height;
        }
    }
    c->generated = true;
}

// Helper per aggiungere un vertice con tutti i suoi attributi
static void AddVertex(std::vector<float>& verts, std::vector<float>& norms,
                     std::vector<float>& texc, std::vector<unsigned char>& cols,
                     float x, float y, float z, float nx, float ny, float nz,
                     float u, float v, Color col) {
    verts.push_back(x);
    verts.push_back(y);
    verts.push_back(z);
    
    norms.push_back(nx);
    norms.push_back(ny);
    norms.push_back(nz);
    
    texc.push_back(u);
    texc.push_back(v);
    
    cols.push_back(col.r);
    cols.push_back(col.g);
    cols.push_back(col.b);
    cols.push_back(col.a);
}

// Aggiunge una faccia cubica completa
static void AddCubeFace(std::vector<float>& verts, std::vector<float>& norms,
                       std::vector<float>& texc, std::vector<unsigned char>& cols,
                       float x, float y, float z, int face, Color color) {
    
    switch(face) {
        case 0: // TOP (Y+) - Guardando dall'alto verso il basso
            AddVertex(verts, norms, texc, cols, x,   y+1, z+1, 0, 1, 0, 0, 0, color);
            AddVertex(verts, norms, texc, cols, x+1, y+1, z+1, 0, 1, 0, 1, 0, color);
            AddVertex(verts, norms, texc, cols, x+1, y+1, z,   0, 1, 0, 1, 1, color);
            
            AddVertex(verts, norms, texc, cols, x,   y+1, z+1, 0, 1, 0, 0, 0, color);
            AddVertex(verts, norms, texc, cols, x+1, y+1, z,   0, 1, 0, 1, 1, color);
            AddVertex(verts, norms, texc, cols, x,   y+1, z,   0, 1, 0, 0, 1, color);
            break;
            
        case 1: // BOTTOM (Y-)
            AddVertex(verts, norms, texc, cols, x,   y, z,   0, -1, 0, 0, 0, color);
            AddVertex(verts, norms, texc, cols, x+1, y, z,   0, -1, 0, 1, 0, color);
            AddVertex(verts, norms, texc, cols, x+1, y, z+1, 0, -1, 0, 1, 1, color);
            
            AddVertex(verts, norms, texc, cols, x,   y, z,   0, -1, 0, 0, 0, color);
            AddVertex(verts, norms, texc, cols, x+1, y, z+1, 0, -1, 0, 1, 1, color);
            AddVertex(verts, norms, texc, cols, x,   y, z+1, 0, -1, 0, 0, 1, color);
            break;
            
        case 2: // NORTH (Z+)
            AddVertex(verts, norms, texc, cols, x,   y,   z+1, 0, 0, 1, 0, 0, color);
            AddVertex(verts, norms, texc, cols, x+1, y,   z+1, 0, 0, 1, 1, 0, color);
            AddVertex(verts, norms, texc, cols, x+1, y+1, z+1, 0, 0, 1, 1, 1, color);
            
            AddVertex(verts, norms, texc, cols, x,   y,   z+1, 0, 0, 1, 0, 0, color);
            AddVertex(verts, norms, texc, cols, x+1, y+1, z+1, 0, 0, 1, 1, 1, color);
            AddVertex(verts, norms, texc, cols, x,   y+1, z+1, 0, 0, 1, 0, 1, color);
            break;
            
        case 3: // SOUTH (Z-)
            AddVertex(verts, norms, texc, cols, x+1, y,   z, 0, 0, -1, 0, 0, color);
            AddVertex(verts, norms, texc, cols, x,   y,   z, 0, 0, -1, 1, 0, color);
            AddVertex(verts, norms, texc, cols, x,   y+1, z, 0, 0, -1, 1, 1, color);
            
            AddVertex(verts, norms, texc, cols, x+1, y,   z, 0, 0, -1, 0, 0, color);
            AddVertex(verts, norms, texc, cols, x,   y+1, z, 0, 0, -1, 1, 1, color);
            AddVertex(verts, norms, texc, cols, x+1, y+1, z, 0, 0, -1, 0, 1, color);
            break;
            
        case 4: // EAST (X+)
            AddVertex(verts, norms, texc, cols, x+1, y,   z+1, 1, 0, 0, 0, 0, color);
            AddVertex(verts, norms, texc, cols, x+1, y,   z,   1, 0, 0, 1, 0, color);
            AddVertex(verts, norms, texc, cols, x+1, y+1, z,   1, 0, 0, 1, 1, color);
            
            AddVertex(verts, norms, texc, cols, x+1, y,   z+1, 1, 0, 0, 0, 0, color);
            AddVertex(verts, norms, texc, cols, x+1, y+1, z,   1, 0, 0, 1, 1, color);
            AddVertex(verts, norms, texc, cols, x+1, y+1, z+1, 1, 0, 0, 0, 1, color);
            break;
            
        case 5: // WEST (X-)
            AddVertex(verts, norms, texc, cols, x, y,   z,   -1, 0, 0, 0, 0, color);
            AddVertex(verts, norms, texc, cols, x, y,   z+1, -1, 0, 0, 1, 0, color);
            AddVertex(verts, norms, texc, cols, x, y+1, z+1, -1, 0, 0, 1, 1, color);
            
            AddVertex(verts, norms, texc, cols, x, y,   z,   -1, 0, 0, 0, 0, color);
            AddVertex(verts, norms, texc, cols, x, y+1, z+1, -1, 0, 0, 1, 1, color);
            AddVertex(verts, norms, texc, cols, x, y+1, z,   -1, 0, 0, 0, 1, color);
            break;
    }
}

// Verifica se un blocco esiste in una posizione
static bool IsBlockAt(Chunk* c, int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) return false;
    if (y < 0) return false;
    int height = (int)c->heightMap[x][z];
    return (y <= height);
}

static void GenerateChunkMesh(Chunk* c) {
    if (c->meshGenerated) UnloadMesh(c->mesh);
    
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<unsigned char> colors;
    
    Color grassTop = {100, 180, 80, 255};
    Color grassSide = {80, 150, 60, 255};
    Color dirt = {139, 90, 43, 255};
    
    // Per ogni colonna nel chunk
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            int maxHeight = (int)c->heightMap[x][z];
            
            // Genera i cubi SOLO per i blocchi sulla superficie
            // Non generare tutto dall'alto al basso!
            for (int y = 0; y <= maxHeight; y++) {
                float wx = (float)(c->chunkX * CHUNK_SIZE + x);
                float wz = (float)(c->chunkZ * CHUNK_SIZE + z);
                
                bool isTopBlock = (y == maxHeight);
                Color topCol = isTopBlock ? grassTop : dirt;
                Color sideCol = grassSide;
                
                // Controlla ogni faccia e aggiungi solo quelle visibili
                
                // TOP - Solo se è il blocco più alto
                if (isTopBlock) {
                    AddCubeFace(vertices, normals, texcoords, colors, wx, y, wz, 0, topCol);
                }
                
                // BOTTOM - Solo se è y=0 o se sotto c'è aria
                if (y == 0 || !IsBlockAt(c, x, y-1, z)) {
                    AddCubeFace(vertices, normals, texcoords, colors, wx, y, wz, 1, dirt);
                }
                
                // NORTH (Z+)
                if (!IsBlockAt(c, x, y, z+1)) {
                    AddCubeFace(vertices, normals, texcoords, colors, wx, y, wz, 2, sideCol);
                }
                
                // SOUTH (Z-)
                if (!IsBlockAt(c, x, y, z-1)) {
                    AddCubeFace(vertices, normals, texcoords, colors, wx, y, wz, 3, sideCol);
                }
                
                // EAST (X+)
                if (!IsBlockAt(c, x+1, y, z)) {
                    AddCubeFace(vertices, normals, texcoords, colors, wx, y, wz, 4, sideCol);
                }
                
                // WEST (X-)
                if (!IsBlockAt(c, x-1, y, z)) {
                    AddCubeFace(vertices, normals, texcoords, colors, wx, y, wz, 5, sideCol);
                }
            }
        }
    }
    
    if (vertices.empty()) return;
    
    // Crea la mesh
    memset(&c->mesh, 0, sizeof(Mesh));
    c->mesh.vertexCount = vertices.size() / 3;
    c->mesh.triangleCount = c->mesh.vertexCount / 3;
    
    c->mesh.vertices = (float*)malloc(vertices.size() * sizeof(float));
    c->mesh.normals = (float*)malloc(normals.size() * sizeof(float));
    c->mesh.texcoords = (float*)malloc(texcoords.size() * sizeof(float));
    c->mesh.colors = (unsigned char*)malloc(colors.size());
    
    memcpy(c->mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
    memcpy(c->mesh.normals, normals.data(), normals.size() * sizeof(float));
    memcpy(c->mesh.texcoords, texcoords.data(), texcoords.size() * sizeof(float));
    memcpy(c->mesh.colors, colors.data(), colors.size());
    
    UploadMesh(&c->mesh, false);
    c->meshGenerated = true;
}

void WorldInit(World* world) {
    world->chunkCount = 0;
}

void WorldUpdate(World* world, Vector3 playerPos) {
    int playerChunkX = (int)floor(playerPos.x / CHUNK_SIZE);
    int playerChunkZ = (int)floor(playerPos.z / CHUNK_SIZE);
    
    for (int x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; x++) {
        for (int z = -RENDER_DISTANCE; z <= RENDER_DISTANCE; z++) {
            Chunk* c = WorldGetChunk(world, playerChunkX + x, playerChunkZ + z);
            if (c && !c->generated) GenerateChunk(c);
            if (c && c->generated && !c->meshGenerated) GenerateChunkMesh(c);
        }
    }
}

void WorldDraw(World* world) {
    Material mat = LoadMaterialDefault();
    
    for (int i = 0; i < world->chunkCount; i++) {
        if (world->chunks[i].meshGenerated) {
            DrawMesh(world->chunks[i].mesh, mat, MatrixIdentity());
        }
    }
}

float GetTerrainHeightAt(World* world, float x, float z) {
    int chunkX = (int)floor(x / CHUNK_SIZE);
    int chunkZ = (int)floor(z / CHUNK_SIZE);
    
    Chunk* chunk = NULL;
    for (int i = 0; i < world->chunkCount; i++) {
        if (world->chunks[i].chunkX == chunkX && world->chunks[i].chunkZ == chunkZ) {
            chunk = &world->chunks[i];
            break;
        }
    }
    
    if (!chunk || !chunk->generated) return 5.0f;
    
    float localX = x - chunkX * CHUNK_SIZE;
    float localZ = z - chunkZ * CHUNK_SIZE;
    int x0 = (int)floor(localX);
    int z0 = (int)floor(localZ);
    
    if (x0 < 0 || x0 >= CHUNK_SIZE || z0 < 0 || z0 >= CHUNK_SIZE) return 5.0f;
    
    return chunk->heightMap[x0][z0] + 1.0f;
}

BlockType GetBlockAt(World* world, int x, int y, int z) {
    float height = GetTerrainHeightAt(world, (float)x, (float)z);
    return (y < height) ? BLOCK_DIRT : BLOCK_AIR;
}