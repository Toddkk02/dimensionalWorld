// ========== mining.cpp - VERSIONE CORRETTA DEFINITIVA ==========

#include "mining.h"
#include "raymath.h"
#include "../world/firstWorld.h"
#include <cmath>

// Helper per ottenere l'altezza RAW del terreno (senza +1)
static float GetRawTerrainHeight(World* world, float x, float z) {
    int chunkX = (int)floor(x / CHUNK_SIZE);
    int chunkZ = (int)floor(z / CHUNK_SIZE);
    
    Chunk* chunk = NULL;
    for (int i = 0; i < world->chunkCount; i++) {
        if (world->chunks[i].chunkX == chunkX && world->chunks[i].chunkZ == chunkZ) {
            chunk = &world->chunks[i];
            break;
        }
    }
    
    if (!chunk || !chunk->generated) return 0.0f;
    
    float localX = x - chunkX * CHUNK_SIZE;
    float localZ = z - chunkZ * CHUNK_SIZE;
    int x0 = (int)floor(localX);
    int z0 = (int)floor(localZ);
    
    if (x0 < 0 || x0 >= CHUNK_SIZE || z0 < 0 || z0 >= CHUNK_SIZE) return 0.0f;
    
    // Ritorna l'altezza RAW (senza +1)
    return chunk->heightMap[x0][z0];
}

// Raycast che cerca BLOCCHI SOLIDI
bool RaycastBlock(Camera3D cam, World* world, Vector3& hitPos) {
    Vector3 dir = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
    Vector3 pos = cam.position;
    
    float maxDist = 10.0f;
    float step = 0.1f;
    
    for (float dist = 0.5f; dist < maxDist; dist += step) { // Inizia da 0.5 per evitare auto-hit
        pos = Vector3Add(cam.position, Vector3Scale(dir, dist));
        
        // Ottieni l'altezza RAW del terreno
        float terrainHeight = GetRawTerrainHeight(world, pos.x, pos.z);
        
        // Se il punto del raggio è DENTRO un blocco solido
        if (pos.y <= terrainHeight + 1.0f) {
            // Abbiamo colpito!
            hitPos.x = floorf(pos.x);
            hitPos.z = floorf(pos.z);
            hitPos.y = floorf(terrainHeight);
            
            TraceLog(LOG_DEBUG, "RaycastBlock: Hit at (%.0f, %.0f, %.0f) | TerrainHeight: %.1f", 
                     hitPos.x, hitPos.y, hitPos.z, terrainHeight);
            
            return true;
        }
    }
    
    return false;
}

void UpdateMining(MiningState& mining, Camera3D cam, World* world, float dt) {
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector3 hit;
        if (RaycastBlock(cam, world, hit)) {
            // Verifica che il blocco sia valido
            float terrainHeight = GetRawTerrainHeight(world, hit.x, hit.z);
            
            if (hit.y > terrainHeight || hit.y < 0) {
                // Blocco non valido
                mining.mining = false;
                mining.progress = 0.0f;
                return;
            }
            
            // Se è lo stesso blocco, continua il mining
            if (mining.mining && 
                hit.x == mining.targetBlock.x && 
                hit.y == mining.targetBlock.y && 
                hit.z == mining.targetBlock.z) {
                mining.progress += dt;
            } else {
                // Nuovo blocco - reset progress
                mining.mining = true;
                mining.targetBlock = hit;
                mining.progress = 0.0f;
                
                TraceLog(LOG_INFO, "Mining started: Block(%.0f, %.0f, %.0f)", 
                         hit.x, hit.y, hit.z);
            }
        } else {
            // Non stiamo colpendo nessun blocco
            mining.mining = false;
            mining.progress = 0.0f;
        }
    } else {
        // Mouse rilasciato - reset
        mining.mining = false;
        mining.progress = 0.0f;
    }
}