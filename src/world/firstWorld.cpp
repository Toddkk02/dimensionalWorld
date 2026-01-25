#include "firstWorld.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
#include "dimensions.h" 
#include <math.h>
#include <stdlib.h>
#include <raymath.h>
#include <string.h>
#include <vector>

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
    memset(&c->mesh, 0, sizeof(Mesh));
    return c;
}

// Aggiunta: seed per la dimensione
static int currentDimensionSeed = 0;

void SetWorldDimension(int dimension) {
    currentDimensionSeed = dimension * 1000;
}

static void GenerateChunk(Chunk* c) {
    float waterLevel = WATER_LEVEL;
    
    // Alloca oreMap dinamicamente
    c->oreMap = new int**[CHUNK_SIZE + 1];
    for (int x = 0; x <= CHUNK_SIZE; x++) {
        c->oreMap[x] = new int*[MAX_HEIGHT];
        for (int y = 0; y < MAX_HEIGHT; y++) {
            c->oreMap[x][y] = new int[CHUNK_SIZE + 1]();
        }
    }
    
    // Genera heightmap e liquidMap
    for (int x = 0; x <= CHUNK_SIZE; x++) {
        for (int z = 0; z <= CHUNK_SIZE; z++) {
            float wx = (float)(c->chunkX * CHUNK_SIZE + x);
            float wz = (float)(c->chunkZ * CHUNK_SIZE + z);
            
            float height = stb_perlin_noise3(wx * 0.02f, currentDimensionSeed, wz * 0.02f, 0, 0, 0) * 8.0f +
                          stb_perlin_noise3(wx * 0.05f, 10 + currentDimensionSeed, wz * 0.05f, 0, 0, 0) * 4.0f +
                          stb_perlin_noise3(wx * 0.1f, 20 + currentDimensionSeed, wz * 0.1f, 0, 0, 0) * 2.0f + 5.0f;
            
            c->heightMap[x][z] = height;
            
            if (height < waterLevel) {
                c->liquidMap[x][z] = waterLevel - height;
            } else {
                c->liquidMap[x][z] = 0.0f;
            }
        }
    }
    
    // Genera minerali sotterranei
    for (int x = 0; x <= CHUNK_SIZE; x++) {
        for (int z = 0; z <= CHUNK_SIZE; z++) {
            float wx = (float)(c->chunkX * CHUNK_SIZE + x);
            float wz = (float)(c->chunkZ * CHUNK_SIZE + z);
            int maxHeight = (int)c->heightMap[x][z];
            
            if (maxHeight > MAX_HEIGHT) maxHeight = MAX_HEIGHT;
            
            for (int y = 0; y <= maxHeight && y < MAX_HEIGHT; y++) {
                float wy = (float)y;
                
                // IRON ORE (comune, y < 40)
                if (y < 40) {
                    float ironNoise = stb_perlin_noise3(wx * 0.1f, wy * 0.1f, wz * 0.1f, 0, 0, 0);
                    if (ironNoise > 0.6f) {
                        c->oreMap[x][y][z] = (int)ItemType::IRON_ORE;
                        continue;
                    }
                }
                
                // GOLD ORE (raro, y < 25)
                if (y < 25) {
                    float goldNoise = stb_perlin_noise3(wx * 0.15f, wy * 0.15f, wz * 0.15f, 100, 0, 0);
                    if (goldNoise > 0.75f) {
                        c->oreMap[x][y][z] = (int)ItemType::GOLD_ORE;
                        continue;
                    }
                }
                
                // DIAMOND (molto raro, y < 15)
                if (y < 15) {
                    float diamondNoise = stb_perlin_noise3(wx * 0.2f, wy * 0.2f, wz * 0.2f, 200, 0, 0);
                    if (diamondNoise > 0.85f) {
                        c->oreMap[x][y][z] = (int)ItemType::DIAMOND;
                        continue;
                    }
                }
                
                // STONE (resto del sottosuolo, y < maxHeight - 3)
                if (y < maxHeight - 3) {
                    c->oreMap[x][y][z] = (int)ItemType::STONE;
                }
            }
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
        case 0: // TOP (Y+)
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

// Verifica se un blocco di terreno esiste in una posizione
static bool IsBlockAt(Chunk* c, int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) return false;
    if (y < 0) return false;
    int height = (int)c->heightMap[x][z];
    return (y <= height);
}

// Verifica se c'è acqua in una posizione
static bool IsWaterAt(Chunk* c, int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) return false;
    if (y < 0) return false;
    
    float terrainHeight = c->heightMap[x][z];
    float waterHeight = terrainHeight + c->liquidMap[x][z];
    
    return (y >= (int)terrainHeight && y < (int)waterHeight);
}

// Colori che cambiano in base alla dimensione corrente
static Color currentGrassTop = {153, 51, 255, 255};
static Color currentDirtSide = {51, 25, 0, 255};
static Color currentDirt = {51, 25, 0, 255};

void SetDimensionColors(Color grassTop, Color dirtSide, Color dirt) {
    currentGrassTop = grassTop;
    currentDirtSide = dirtSide;
    currentDirt = dirt;
}

static void GenerateChunkMesh(Chunk* c) {
    if (c->meshGenerated) {
        UnloadMesh(c->mesh);
    }
    
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<unsigned char> colors;
    
    Color grassTop = currentGrassTop;
    Color dirtSide = currentDirtSide;
    Color dirt = currentDirt;
    Color waterCol = {30, 100, 255, 180};  // acqua trasparente blu
    
    // FASE 1: Genera il terreno solido
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            int maxHeight = (int)c->heightMap[x][z];
            
            // Clamp maxHeight per evitare overflow
            if (maxHeight < 0) maxHeight = 0;
            if (maxHeight > 100) maxHeight = 100;
            
            // Genera i cubi del terreno
            for (int y = 0; y <= maxHeight; y++) {
                float wx = (float)(c->chunkX * CHUNK_SIZE + x);
                float wz = (float)(c->chunkZ * CHUNK_SIZE + z);
                
                bool isTopBlock = (y == maxHeight);
                Color topCol = isTopBlock ? grassTop : dirt;
                Color sideCol = dirtSide;
                
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
    
    // FASE 2: Genera l'acqua SOPRA il terreno
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            if (c->liquidMap[x][z] > 0.0f) {
                float wx = (float)(c->chunkX * CHUNK_SIZE + x);
                float wz = (float)(c->chunkZ * CHUNK_SIZE + z);
                float terrainHeight = c->heightMap[x][z];
                int waterDepth = (int)c->liquidMap[x][z];
                
                // Genera i cubi d'acqua dal terreno fino al livello dell'acqua
                for (int wy = 0; wy < waterDepth; wy++) {
                    int y = (int)terrainHeight + wy;
                    
                    // TOP - Mostra solo se è la superficie dell'acqua
                    if (wy == waterDepth - 1) {
                        AddCubeFace(vertices, normals, texcoords, colors, wx, y, wz, 0, waterCol);
                    }
                    
                    // BOTTOM - Non mostrare (è sul terreno o su altra acqua)
                    // Opzionale: se vuoi vedere il fondo dell'acqua, puoi abilitarlo
                    
                    // NORTH (Z+) - Solo se NON c'è acqua adiacente
                    if (!IsWaterAt(c, x, y, z+1)) {
                        AddCubeFace(vertices, normals, texcoords, colors, wx, y, wz, 2, waterCol);
                    }
                    
                    // SOUTH (Z-)
                    if (!IsWaterAt(c, x, y, z-1)) {
                        AddCubeFace(vertices, normals, texcoords, colors, wx, y, wz, 3, waterCol);
                    }
                    
                    // EAST (X+)
                    if (!IsWaterAt(c, x+1, y, z)) {
                        AddCubeFace(vertices, normals, texcoords, colors, wx, y, wz, 4, waterCol);
                    }
                    
                    // WEST (X-)
                    if (!IsWaterAt(c, x-1, y, z)) {
                        AddCubeFace(vertices, normals, texcoords, colors, wx, y, wz, 5, waterCol);
                    }
                }
            }
        }
    }
    
    if (vertices.empty()) {
        c->meshGenerated = false;
        return;
    }
    
    // Crea la mesh
    memset(&c->mesh, 0, sizeof(Mesh));
    c->mesh.vertexCount = vertices.size() / 3;
    c->mesh.triangleCount = c->mesh.vertexCount / 3;
    
    // Allocazione sicura della memoria
    c->mesh.vertices = (float*)malloc(vertices.size() * sizeof(float));
    c->mesh.normals = (float*)malloc(normals.size() * sizeof(float));
    c->mesh.texcoords = (float*)malloc(texcoords.size() * sizeof(float));
    c->mesh.colors = (unsigned char*)malloc(colors.size());
    
    if (!c->mesh.vertices || !c->mesh.normals || !c->mesh.texcoords || !c->mesh.colors) {
        // Gestione errore allocazione
        if (c->mesh.vertices) free(c->mesh.vertices);
        if (c->mesh.normals) free(c->mesh.normals);
        if (c->mesh.texcoords) free(c->mesh.texcoords);
        if (c->mesh.colors) free(c->mesh.colors);
        c->meshGenerated = false;
        return;
    }
    
    memcpy(c->mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
    memcpy(c->mesh.normals, normals.data(), normals.size() * sizeof(float));
    memcpy(c->mesh.texcoords, texcoords.data(), texcoords.size() * sizeof(float));
    memcpy(c->mesh.colors, colors.data(), colors.size());
    
    UploadMesh(&c->mesh, false);
    c->meshGenerated = true;
}

void WorldInit(World* world) {
    world->chunkCount = 0;
    for (int i = 0; i < MAX_CHUNKS; i++) {
        world->chunks[i].generated = false;
        world->chunks[i].meshGenerated = false;
        world->chunks[i].oreMap = nullptr;  // ← Inizializza a null
        memset(&world->chunks[i].mesh, 0, sizeof(Mesh));
    }
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
    // CRITICO: Non chiamare LoadMaterialDefault() ogni frame!

    static Material defaultMat = {};
    static bool materialLoaded = false;
    
    if (!materialLoaded) {
        defaultMat = LoadMaterialDefault();
        materialLoaded = true;
    }
    
    for (int i = 0; i < world->chunkCount; i++) {
        if (world->chunks[i].meshGenerated && world->chunks[i].mesh.vertexCount > 0) {
            DrawMesh(world->chunks[i].mesh, defaultMat, MatrixIdentity());
        }
    }
}

void WorldCleanup(World* world) {
    for (int i = 0; i < world->chunkCount; i++) {
        if (world->chunks[i].meshGenerated) {
            UnloadMesh(world->chunks[i].mesh);
            world->chunks[i].meshGenerated = false;
        }
        
        // ← AGGIUNGI: Dealloca oreMap
        if (world->chunks[i].oreMap) {
            for (int x = 0; x <= CHUNK_SIZE; x++) {
                for (int y = 0; y < MAX_HEIGHT; y++) {
                    delete[] world->chunks[i].oreMap[x][y];
                }
                delete[] world->chunks[i].oreMap[x];
            }
            delete[] world->chunks[i].oreMap;
            world->chunks[i].oreMap = nullptr;
        }
    }
}

void WorldLoadTextures(World* world, DimensionConfig* dim) {
    TraceLog(LOG_INFO, "Loading textures for %s", dim->name.c_str());
    
    // Carica solo se non già caricate E path non vuoto
    if (dim->grassTopTex.id == 0 && !dim->grassTopTexture.empty()) {
        if (FileExists(dim->grassTopTexture.c_str())) {
            dim->grassTopTex = LoadTexture(dim->grassTopTexture.c_str());
            dim->grassTopLoaded = true;
            TraceLog(LOG_INFO, "Loaded grassTop: %s", dim->grassTopTexture.c_str());
        } else {
            TraceLog(LOG_WARNING, "MISSING: %s", dim->grassTopTexture.c_str());
        }
    }
    
    if (dim->dirtSideTex.id == 0 && !dim->dirtSideTexture.empty()) {
        if (FileExists(dim->dirtSideTexture.c_str())) {
            dim->dirtSideTex = LoadTexture(dim->dirtSideTexture.c_str());
            dim->dirtSideLoaded = true;
            TraceLog(LOG_INFO, "Loaded dirtSide: %s", dim->dirtSideTexture.c_str());
        } else {
            TraceLog(LOG_WARNING, "MISSING: %s", dim->dirtSideTexture.c_str());
        }
    }
    
    if (dim->dirtTex.id == 0 && !dim->dirtTexture.empty()) {
        if (FileExists(dim->dirtTexture.c_str())) {
            dim->dirtTex = LoadTexture(dim->dirtTexture.c_str());
            dim->dirtLoaded = true;
            TraceLog(LOG_INFO, "Loaded dirt: %s", dim->dirtTexture.c_str());
        } else {
            TraceLog(LOG_WARNING, "MISSING: %s", dim->dirtTexture.c_str());
        }
    }
    
    if (dim->skyboxTex.id == 0 && !dim->skyboxTexture.empty()) {
        dim->skyboxTex = LoadTexture(dim->skyboxTexture.c_str());
    }
    
    world->useTextures = true;  // Forza uso texture
}

void WorldUnloadTextures(World* world) {
    if (world->useTextures) {
        //UnloadTexture(world->grassTopTexture);
        //UnloadTexture(world->dirtSideTexture);
        //UnloadTexture(world->dirtTexture);
        //UnloadTexture(world->waterTexture);
        //world->useTextures = false;
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

void RegenerateAllChunks(World* world) {
    // Marca tutti i chunk come non generati per forzare la rigenerazione
    for (int i = 0; i < world->chunkCount; i++) {
        if (world->chunks[i].meshGenerated) {
            UnloadMesh(world->chunks[i].mesh);
        }
        world->chunks[i].generated = false;
        world->chunks[i].meshGenerated = false;
    }
}

ItemType RemoveBlock(World* world, int x, int y, int z) {
    // Determina il chunk
    int chunkX = (int)floor((float)x / CHUNK_SIZE);
    int chunkZ = (int)floor((float)z / CHUNK_SIZE);
    
    Chunk* chunk = NULL;
    for (int i = 0; i < world->chunkCount; i++) {
        if (world->chunks[i].chunkX == chunkX && world->chunks[i].chunkZ == chunkZ) {
            chunk = &world->chunks[i];
            break;
        }
    }
    
    if (!chunk || !chunk->generated) return ItemType::NONE;
    
    // Coordinate locali nel chunk
    int lx = x - chunkX * CHUNK_SIZE;
    int lz = z - chunkZ * CHUNK_SIZE;
    
    // Clamp coordinate locali
    if (lx < 0) lx = 0;
    if (lx >= CHUNK_SIZE) lx = CHUNK_SIZE - 1;
    if (lz < 0) lz = 0;
    if (lz >= CHUNK_SIZE) lz = CHUNK_SIZE - 1;
    
    // Verifica se c'è un blocco qui
    float currentHeight = chunk->heightMap[lx][lz];
    
    // Se il blocco target è sopra il terreno, non c'è nulla da scavare
    if (y > (int)currentHeight || y < 0) {
        return ItemType::NONE;
    }
    
    // Determina il tipo di item droppato
    ItemType dropType = ItemType::DIRT;
    
    // Se è il blocco superiore, droppa grass
    if (y == (int)currentHeight) {
        dropType = ItemType::GRASS;
    } else {
        dropType = ItemType::DIRT;
    }
    
    // Rimuovi il blocco abbassando l'altezza di 1
    chunk->heightMap[lx][lz] = (float)(y - 1);
    
    // Se abbassando l'altezza andiamo sotto zero, clamp a 0
    if (chunk->heightMap[lx][lz] < 0.0f) {
        chunk->heightMap[lx][lz] = 0.0f;
    }
    
    // Rigenera la mesh del chunk
    if (chunk->meshGenerated) {
        UnloadMesh(chunk->mesh);
        chunk->meshGenerated = false;
    }
    GenerateChunkMesh(chunk);
    
    return dropType;
}

bool PlaceBlock(World* world, int x, int y, int z, ItemType blockType) {
    int chunkX = (int)floor((float)x / CHUNK_SIZE);
    int chunkZ = (int)floor((float)z / CHUNK_SIZE);
    
    Chunk* chunk = NULL;
    for (int i = 0; i < world->chunkCount; i++) {
        if (world->chunks[i].chunkX == chunkX && 
            world->chunks[i].chunkZ == chunkZ) {
            chunk = &world->chunks[i];
            break;
        }
    }
    
    if (!chunk || !chunk->generated) {
        TraceLog(LOG_WARNING, "PlaceBlock: Chunk not found or not generated");
        return false;
    }
    
    int lx = x - chunkX * CHUNK_SIZE;
    int lz = z - chunkZ * CHUNK_SIZE;
    
    // Clamp coordinate
    if (lx < 0) lx = 0;
    if (lx >= CHUNK_SIZE) lx = CHUNK_SIZE - 1;
    if (lz < 0) lz = 0;
    if (lz >= CHUNK_SIZE) lz = CHUNK_SIZE - 1;
    
    float currentHeight = chunk->heightMap[lx][lz];
    
    // Permetti di piazzare SOLO se y è esattamente currentHeight + 1
    if (y != (int)currentHeight + 1) {
        TraceLog(LOG_WARNING, "PlaceBlock: Invalid Y position (y=%d, terrain=%.0f, type=%d)", 
                 y, currentHeight, (int)blockType);
        return false;
    }
    
    // Limite massimo altezza
    if (y > 100) {
        TraceLog(LOG_WARNING, "PlaceBlock: Y too high (%d)", y);
        return false;
    }
    
    // Aggiorna l'altezza del terreno
    chunk->heightMap[lx][lz] = (float)y;
    
    TraceLog(LOG_INFO, "PlaceBlock: %s placed at (%d, %d, %d) | New height: %.0f", 
             GetItemName(blockType), x, y, z, chunk->heightMap[lx][lz]);
    
    // Rigenera la mesh
    if (chunk->meshGenerated) {
        UnloadMesh(chunk->mesh);
        chunk->meshGenerated = false;
    }
    GenerateChunkMesh(chunk);
    
    return true;
}
