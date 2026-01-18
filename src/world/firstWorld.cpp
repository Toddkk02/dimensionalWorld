#include "raylib.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

#define GRID_SIZE 20
#define scale 1.0f

void firstWorldGenerated(void) {  // cambio int -> void
    for (int x = 0; x < GRID_SIZE; x++){
        for (int z = 0; z < GRID_SIZE; z++){
            float y = stb_perlin_noise3(x*0.1f, 0.0f, z*0.1f, 0, 0, 0) * 2.0f; // CORRETTO
            DrawCube((Vector3){x*scale, y/2.0f, z*scale}, scale, y, scale, GREEN);
        }
    }
}

