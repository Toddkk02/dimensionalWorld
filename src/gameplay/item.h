#ifndef ITEM_H
#define ITEM_H

#include "raylib.h"

// Tipi di item droppabili
enum class ItemType {
    NONE = 0,
    DIRT,
    GRASS,
    STONE,
    CRYSTAL,
    SAND,
    ICE
};

// Item singolo
struct Item {
    ItemType type = ItemType::NONE;
    int quantity = 0;
};

// Helper visuali
const char* GetItemName(ItemType type);
Color GetItemColor(ItemType type);

#endif
