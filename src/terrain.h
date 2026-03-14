#pragma once

#include <stdbool.h>

#include "raylib.h"

#include "world.h"

// Terrain mesh - generated when terrain loads, use with DrawMesh()
extern Mesh gTerrainMesh;

// Initialize the scrolling grid mesh (mirrors MESHgrid in Source.bb)
void TerrainInitGrid(int sizePower);

// Update the grid vertices and colors based on player position (mirrors Source.bb loop in MAIN)
void TerrainUpdateGrid(const Ship *player);

// C translation of TERRAINgetheight#( bx#, bz#, check=0 ) from Source.bb.
// check == 0: use landHeight, check == 1: use objectHeight.
float TerrainGetHeight(float bx, float bz, int check);

// Minimal C translation of TERRAINload(file$) that:
// - loads LevelData/<file> (optionally adding .bmp),
// - fills gTerrain heights and basic indices.
bool TerrainLoad(const char *fileName);

// Initialize terrain objects with placeholder models
void TerrainInitObjects(void);

// Render terrain objects near the player position
void TerrainRenderObjects(const Ship *player);
void TerrainUpdateObjects(const Ship *player);
void TerrainMapAdd(int mode, float sx, float sz, int ss);
void TerrainCollisionGround(float sx, float sy, float sz, int inView, int isShip, int index);
