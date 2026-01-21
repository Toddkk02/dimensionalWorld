#include "blocks.h"
#include <string.h>

// DEFINIZIONI REALI (una sola volta in tutto il progetto)
Texture2D blockTextures[BLOCK_COUNT];
Color blockFallbackColors[BLOCK_COUNT];

void InitBlockSystem() {
    // Inizializza texture a zero
    for (int i = 0; i < BLOCK_COUNT; i++) {
        blockTextures[i].id = 0;
        blockTextures[i].width = 0;
        blockTextures[i].height = 0;
        blockTextures[i].mipmaps = 0;
        blockTextures[i].format = 0;
    }
    
    // Colori fallback
    blockFallbackColors[BLOCK_AIR] = BLANK;
    blockFallbackColors[BLOCK_GRASS] = GREEN;
    blockFallbackColors[BLOCK_DIRT] = BROWN;
    blockFallbackColors[BLOCK_STONE] = GRAY;
    blockFallbackColors[BLOCK_SAND] = YELLOW;
    blockFallbackColors[BLOCK_WATER] = BLUE;
}

void CleanupBlockSystem() {
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (blockTextures[i].id != 0) {
            UnloadTexture(blockTextures[i]);
            blockTextures[i].id = 0;
        }
    }
}