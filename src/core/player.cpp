#include "player.h"
#include <raylib.h>
#include <raymath.h>

void UpdatePlayerPhysics(PlayerSystem *ps, World* world, float deltaTime) {
    ps->velocity.y += ps->gravity * deltaTime;
    
    Vector3 newPos = ps->camera.position;
    newPos.y += ps->velocity.y * deltaTime;
    
    float terrainHeight = GetTerrainHeightAt(world, newPos.x, newPos.z);
    
    // Collisione con margine di 0.1
    if (newPos.y <= terrainHeight + 1.8f) {
        newPos.y = terrainHeight + 1.8f;
        ps->velocity.y = 0.0f;
        ps->isGrounded = true;
    } else {
        ps->isGrounded = false;
    }
    
    if (ps->isGrounded && IsKeyPressed(KEY_SPACE)) {
        ps->velocity.y = 10.0f;
    }
    
    float deltaY = newPos.y - ps->camera.position.y;
    ps->camera.position.y = newPos.y;
    ps->camera.target.y += deltaY;
}