#ifndef DROPPED_ITEM_H
#define DROPPED_ITEM_H

#include "raylib.h"
#include "item.h"
#include <vector>

// Forward declarations
struct World;
struct Inventory;

struct DroppedItem {
    ItemType type;
    Vector3 position;
    Vector3 velocity;
    float lifetime;
    float rotation;
};

// Sistema globale
extern std::vector<DroppedItem> g_droppedItems;

// Funzioni
void SpawnDroppedItem(ItemType type, Vector3 position);
void UpdateDroppedItems(World* world, Inventory* inventory, Vector3 playerPos, float dt);
void DrawDroppedItems();
void CleanupDroppedItems();

#endif