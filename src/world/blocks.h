// src/world/blocks.h - FIXED
#ifndef BLOCKS_H
#define BLOCKS_H

#include "raylib.h"
#include "blockTypes.h"  // ← INCLUDE blockTypes.h per avere BLOCK_COUNT

// Dichiarazioni EXTERN - definizioni in blocks.cpp
extern Texture2D blockTextures[BLOCK_COUNT];  // ← Usa BLOCK_COUNT invece di BLOCK_COUNT
extern Color blockFallbackColors[BLOCK_COUNT];

// Funzioni di gestione
void InitBlockSystem();
void CleanupBlockSystem();

#endif