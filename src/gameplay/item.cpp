#include "item.h"

const char* GetItemName(ItemType type) {
    switch (type) {
        case ItemType::NONE: return "None";
        case ItemType::DIRT: return "Dirt";
        case ItemType::GRASS: return "Grass";
        case ItemType::STONE: return "Stone";
        case ItemType::CRYSTAL: return "Crystal";
        case ItemType::SAND: return "Sand";
        case ItemType::ICE: return "Ice";
        case ItemType::WOOD: return "Wood";
        case ItemType::IRON_ORE: return "Iron Ore";
        case ItemType::GOLD_ORE: return "Gold Ore";
        case ItemType::DIAMOND: return "Diamond";
        default: return "Unknown";
    }
}

Color GetItemColor(ItemType type) {
    switch (type) {
        case ItemType::NONE: return WHITE;
        case ItemType::DIRT: return BROWN;
        case ItemType::GRASS: return GREEN;
        case ItemType::STONE: return GRAY;
        case ItemType::CRYSTAL: return SKYBLUE;
        case ItemType::SAND: return YELLOW;
        case ItemType::ICE: return LIGHTGRAY;
        case ItemType::WOOD: return (Color){139, 90, 43, 255};
        case ItemType::IRON_ORE: return (Color){192, 192, 192, 255};
        case ItemType::GOLD_ORE: return (Color){255, 215, 0, 255};
        case ItemType::DIAMOND: return (Color){0, 191, 255, 255};
        default: return MAGENTA;
    }
}