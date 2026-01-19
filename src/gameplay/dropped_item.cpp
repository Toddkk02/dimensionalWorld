#include "dropped_item.h"
#include "item.h"
#include <cmath>

// Sistema globale
std::vector<DroppedItem> g_droppedItems;

static Vector3 Vec3Add(Vector3 a, Vector3 b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

static Vector3 Vec3Sub(Vector3 a, Vector3 b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

static Vector3 Vec3Scale(Vector3 v, float s) {
    return { v.x * s, v.y * s, v.z * s };
}

static float Vec3Dist(Vector3 a, Vector3 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

static Vector3 Vec3Norm(Vector3 v) {
    float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (len <= 0.0001f) return {0,0,0};
    return { v.x / len, v.y / len, v.z / len };
}

void SpawnDroppedItem(ItemType type, Vector3 position) {
    DroppedItem item;
    item.type = type;
    item.position = position;
    item.position.y += 0.5f;  // Spawna sopra il blocco
    item.velocity = {0, 2.0f, 0};  // Piccolo salto iniziale
    item.lifetime = 0;
    item.rotation = 0;
    
    g_droppedItems.push_back(item);
}

void UpdateDroppedItems(Vector3 playerPos, float dt) {
    for (size_t i = 0; i < g_droppedItems.size(); ) {
        DroppedItem& item = g_droppedItems[i];
        
        item.lifetime += dt;
        item.rotation += dt * 90.0f;
        
        // Gravità
        item.velocity.y -= 9.8f * dt;
        
        // Movimento
        item.position = Vec3Add(item.position, Vec3Scale(item.velocity, dt));
        
        // Collisione base con terreno
        if (item.position.y < 1.0f) {
            item.position.y = 1.0f;
            item.velocity.y = 0;
        }
        
        // Magnete verso player
        float dist = Vec3Dist(item.position, playerPos);
        if (dist < 2.0f) {
            Vector3 dir = Vec3Norm(Vec3Sub(playerPos, item.position));
            item.position = Vec3Add(item.position, Vec3Scale(dir, dt * 5.0f));
            
            // Raccogli se molto vicino
            if (dist < 0.5f) {
                // TODO: Aggiungi all'inventario
                g_droppedItems.erase(g_droppedItems.begin() + i);
                continue;
            }
        }
        
        // Rimuovi dopo 5 minuti
        if (item.lifetime > 300.0f) {
            g_droppedItems.erase(g_droppedItems.begin() + i);
            continue;
        }
        
        i++;
    }
}

void DrawDroppedItems() {
    for (const DroppedItem& item : g_droppedItems) {
        DrawCube(item.position, 0.3f, 0.3f, 0.3f, GetItemColor(item.type));
        DrawCubeWires(item.position, 0.3f, 0.3f, 0.3f, BLACK);
    }
}

void CleanupDroppedItems() {
    g_droppedItems.clear();
}