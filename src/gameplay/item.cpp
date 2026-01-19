#include "item.h"

// Nome visualizzato HUD
const char* GetItemName(ItemType type) {
    switch (type) {
        case ItemType::DIRT: return "Dirt";
        case ItemType::GRASS: return "Grass";
        case ItemType::STONE: return "Stone";
        case ItemType::CRYSTAL: return "Crystal";
        case ItemType::SAND: return "Sand";
        case ItemType::ICE: return "Ice";
        default: return "None";
    }
}

// Colore visuale item mondo + HUD
Color GetItemColor(ItemType type) {
    switch (type) {
        case ItemType::DIRT: return BROWN;
        case ItemType::GRASS: return GREEN;
        case ItemType::STONE: return GRAY;
        case ItemType::CRYSTAL: return SKYBLUE;
        case ItemType::SAND: return YELLOW;
        case ItemType::ICE: return LIGHTGRAY;
        default: return WHITE;
    }
}
