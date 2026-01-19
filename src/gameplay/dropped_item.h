#ifndef DROPPED_ITEM_H
#define DROPPED_ITEM_H

#include "raylib.h"
#include "item.h"
#include <vector>

struct DroppedItem {
    Vector3 position;
    Vector3 velocity;
    ItemType type;
    float lifetime;
    float rotation;
};

// Sistema globale dropped items
extern std::vector<DroppedItem> g_droppedItems;

void SpawnDroppedItem(ItemType type, Vector3 position);
void UpdateDroppedItems(Vector3 playerPos, float dt);
void DrawDroppedItems();
void CleanupDroppedItems();

#endif