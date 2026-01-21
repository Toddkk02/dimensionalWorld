#ifndef BLOCK_TYPES_H
#define BLOCK_TYPES_H

constexpr int BLOCK_COUNT = 16; // Numero totale di tipi blocco
// Definizione UNICA di BlockType per tutto il progetto
typedef enum BlockType {
    BLOCK_AIR = 0,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_SAND,
    BLOCK_WATER // Numero totale di tipi blocco
} BlockType;

#endif