#include "player.h"
#include <raylib.h>
#include <raymath.h>

// Funzione helper per ottenere l’altezza dell’acqua + terreno
static float GetWaterHeight(World* world, Vector3 pos)
{
    float terrainH = GetTerrainHeightAt(world, pos.x, pos.z);

    // Recupera il chunk
    int chunkX = (int)floor(pos.x / CHUNK_SIZE);
    int chunkZ = (int)floor(pos.z / CHUNK_SIZE);

    Chunk* c = NULL;
    for (int i = 0; i < world->chunkCount; i++) {
        if (world->chunks[i].chunkX == chunkX && world->chunks[i].chunkZ == chunkZ) {
            c = &world->chunks[i];
            break;
        }
    }

    if (!c || !c->generated) return terrainH; // nessuna acqua

    int lx = (int)(pos.x - chunkX * CHUNK_SIZE);
    int lz = (int)(pos.z - chunkZ * CHUNK_SIZE);

    if (lx < 0 || lx >= CHUNK_SIZE || lz < 0 || lz >= CHUNK_SIZE) return terrainH;

    float waterHeight = terrainH + c->liquidMap[lx][lz];
    return waterHeight;
}

void UpdatePlayerPhysics(PlayerSystem* ps, World* world, float deltaTime)
{
    // Gravità
    ps->velocity.y += ps->gravity * deltaTime;

    Vector3 newPos = ps->camera.position;
    newPos.y += ps->velocity.y * deltaTime;

    float terrainHeight = GetTerrainHeightAt(world, newPos.x, newPos.z);
    float waterHeight = GetWaterHeight(world, newPos);

    // Collisione con il terreno
    if (newPos.y <= terrainHeight + 1.8f)
    {
        newPos.y = terrainHeight + 1.8f;
        ps->velocity.y = 0.0f;
        ps->isGrounded = true;
    }
    else if (newPos.y <= waterHeight) // Se siamo nell’acqua
    {
        newPos.y = waterHeight;
        ps->velocity.y = 0.0f;
        ps->isGrounded = false;
        ps->inWater = true;
    }
    else
    {
        ps->isGrounded = false;
        ps->inWater = false;
    }

    // Salto
    if (ps->isGrounded && IsKeyPressed(KEY_SPACE))
    {
        ps->velocity.y = 10.0f;
    }

    // Nuoto se siamo in acqua
    if (ps->inWater && IsKeyDown(KEY_SPACE))
    {
        ps->velocity.y = 5.0f; // spinta verso l’alto
    }

    // Applica la nuova posizione
    float deltaY = newPos.y - ps->camera.position.y;
    ps->camera.position.y = newPos.y;
    ps->camera.target.y += deltaY;
}
