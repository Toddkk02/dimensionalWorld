#include "world.h"
#include "core/player.h"

void chunkCulling(void){
  float playerChunkX = ps.camera.position.x / CHUNK_SIZE;
  float playerChunkZ = ps.camera.position.z / CHUNK_SIZE;
  
  abs(playerChunkX) <= RENDER_RADIUS;
  abs(playerChunkZ) <= RENDER_RADIUS;
  
}
