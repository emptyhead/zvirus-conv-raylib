#include "terrain.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "constants.h"
#include "raylib.h"
#include "util.h"
#include "world.h"
#include "score_tag.h"
#include "my3d.h"
#include <rlgl.h>
#include <raymath.h>
#include "particle.h"

// Terrain mesh - generated when terrain loads
Mesh gTerrainMesh = {0};

// Initialize the scrolling grid mesh (mirrors MESHgrid in Source.bb)
void TerrainInitGrid(int sizePower) {
    // mirrors MESHgrid( size# )
    gGrid.cull = (1 << sizePower) * 6;
    gGrid.divs = gGrid.cull + 1;
    gGrid.divg = (gGrid.divs * 2) * 2 + 1;
    gGrid.view = gGrid.divs + 6;

    int vertCount = (gGrid.divg + 1) * (gGrid.divg + 1);
    int quadsPerDim = (gGrid.divg + 1) / 2;
    int triCount = quadsPerDim * quadsPerDim * 2;

    Mesh mesh = { 0 };
    mesh.vertexCount = vertCount;
    mesh.triangleCount = triCount;

    mesh.vertices = (float *)MemAlloc(vertCount * 3 * sizeof(float));
    mesh.colors = (unsigned char *)MemAlloc(vertCount * 4 * sizeof(unsigned char));
    mesh.texcoords = (float *)MemAlloc(vertCount * 2 * sizeof(float));
    mesh.indices = (unsigned short *)MemAlloc(triCount * 3 * sizeof(unsigned short));

    // Initialize vertices to 0
    for (int i = 0; i < vertCount; i++) {
        mesh.vertices[i * 3 + 0] = 0;
        mesh.vertices[i * 3 + 1] = 0;
        mesh.vertices[i * 3 + 2] = 0;
        
        // Default color (brownish from Source.bb 2063)
        mesh.colors[i * 4 + 0] = 80;
        mesh.colors[i * 4 + 1] = 50;
        mesh.colors[i * 4 + 2] = 30;
        mesh.colors[i * 4 + 3] = 255;
    }

    // Generate indices (mirrors MESHgrid Step 2 triangles)
    int triIdx = 0;
    int pitch = gGrid.divg + 1;
    for (int z = 0; z < gGrid.divg; z += 2) {
        for (int x = 0; x < gGrid.divg; x += 2) {
            unsigned short v0 = z * pitch + x;
            unsigned short v1 = v0 + 1;
            unsigned short v2 = (z + 1) * pitch + (x + 1);
            unsigned short v3 = (z + 1) * pitch + x;

            // CCW Winding (Up-facing):
            // Tri 1: Bottom-Left, Top-Right, Top-Left
            mesh.indices[triIdx++] = v0;
            mesh.indices[triIdx++] = v2;
            mesh.indices[triIdx++] = v1;
            // Tri 2: Bottom-Left, Bottom-Right, Top-Right
            mesh.indices[triIdx++] = v0;
            mesh.indices[triIdx++] = v3;
            mesh.indices[triIdx++] = v2;
        }
    }

    UploadMesh(&mesh, true); // true = dynamic
    gTerrainMesh = mesh;
}

// Update the grid vertices and colors based on player position (mirrors Source.bb loop in MAIN)
void TerrainUpdateGrid(const Ship *player) {
    if (!player || !gTerrainMesh.vertices) return;

    int ax = (int)floorf(player->x);
    int az = (int)floorf(player->z);
    float fx = player->x - (float)ax;
    float fz = player->z - (float)az;

    int pitch = gGrid.divg + 1;
    float cull = (float)gGrid.cull;

    // Clear color buffer to black/opaque (Source.bb 2063 default)
    for (int i = 0; i < gTerrainMesh.vertexCount; i++) {
        gTerrainMesh.colors[i * 4 + 0] = 0;
        gTerrainMesh.colors[i * 4 + 1] = 0;
        gTerrainMesh.colors[i * 4 + 2] = 0;
        gTerrainMesh.colors[i * 4 + 3] = 255;
    }

    for (int z = -gGrid.divs; z <= gGrid.divs + 1; z++) {
        for (int x = -gGrid.divs; x <= gGrid.divs + 1; x++) {
            int nx = wrapi(ax + x, SIZE, 0);
            int nz = wrapi(az + z, SIZE, 0);
            Terrain *t = &gTerrain[nx][nz];

            float ox = (float)x - fx;
            float oz = (float)z - fz;
            float vx = clampf(ox, -cull, cull);
            float vz = clampf(oz, -cull, cull);
            float vy = t->landHeight;

            if (gGrid.cull <= 12) {
                if (fabsf(vx) >= cull || fabsf(vz) >= cull) {
                    vy = TerrainGetHeight(player->x + vx, player->z + vz, 0);
                }
            }

            // Water waves logic (Source.bb 1709)
            if (fabsf(vy) < 0.25f) {
                vy += (0.25f - vy) * (1.0f + cosf((gRipple + t->waveFactor) * DEG2RAD));
            }

            if (x == -gGrid.divs || x == gGrid.divs + 1 || z == -gGrid.divs || z == gGrid.divs + 1) {
                vy = 0.0f;
            }

            // 1. Position Update (Source.bb 1717-1727)
            int x1_pos = (x + gGrid.divs) * 2 - (x > -gGrid.divs);
            int z1_pos = (z + gGrid.divs) * 2 - (z > -gGrid.divs);
            int xMax = (x > -gGrid.divs && x < gGrid.divs + 1) ? 1 : 0;
            int zMax = (z > -gGrid.divs && z < gGrid.divs + 1) ? 1 : 0;

            for (int iz = 0; iz <= zMax; iz++) {
                for (int ix = 0; ix <= xMax; ix++) {
                    int vIdx = (z1_pos + iz) * pitch + (x1_pos + ix);
                    if (vIdx >= 0 && vIdx < gTerrainMesh.vertexCount) {
                        gTerrainMesh.vertices[vIdx * 3 + 0] = vx;
                        gTerrainMesh.vertices[vIdx * 3 + 1] = vy;
                        gTerrainMesh.vertices[vIdx * 3 + 2] = (float)vz; 
                    }
                }
            }

            // 2. Color Update (Source.bb 1730-1741)
            if (x > -gGrid.divs && x < gGrid.divs && z > -gGrid.divs && z < gGrid.divs) {
                int x1_col = (x + gGrid.divs) * 2;
                int z1_col = (z + gGrid.divs) * 2;
                for (int iz = 0; iz <= 1; iz++) {
                    for (int ix = 0; ix <= 1; ix++) {
                        int vIdx = (z1_col + iz) * pitch + (x1_col + ix);
                        if (vIdx >= 0 && vIdx < gTerrainMesh.vertexCount) {
                            int colorIdx = (t->landInfected > 0) ? 1 : 0;
                            gTerrainMesh.colors[vIdx * 4 + 0] = t->r[colorIdx];
                            gTerrainMesh.colors[vIdx * 4 + 1] = t->g[colorIdx];
                            gTerrainMesh.colors[vIdx * 4 + 2] = t->b[colorIdx];
                            gTerrainMesh.colors[vIdx * 4 + 3] = 255;
                        }
                    }
                }
            }
        }
    }

    // Sync to GPU
    UpdateMeshBuffer(gTerrainMesh, 0, gTerrainMesh.vertices, gTerrainMesh.vertexCount * 3 * sizeof(float), 0);
    UpdateMeshBuffer(gTerrainMesh, 3, gTerrainMesh.colors, gTerrainMesh.vertexCount * 4 * sizeof(unsigned char), 0);
}

// Full-fidelity C port of TERRAINload( file$ ) from Source.bb.
bool TerrainLoad(const char *fileName) {
  if (!fileName || !fileName[0]) return false;

  char nameBuf[256];
  strncpy(nameBuf, fileName, sizeof(nameBuf) - 1);
  nameBuf[sizeof(nameBuf) - 1] = '\0';

  if (!strchr(nameBuf, '.')) {
    strncat(nameBuf, ".bmp", sizeof(nameBuf) - strlen(nameBuf) - 1);
  }

  char path[512];
  snprintf(path, sizeof(path), "LevelData/%s", nameBuf);

  Image img = LoadImage(path);
  if (img.data == NULL) {
    TraceLog(LOG_ERROR, "TerrainLoad: failed to load image: %s", path);
    return false;
  }

  gAreaTotal = SIZE * SIZE;
  gAreaInfected = 0;

  // --- Pass 1: heights, indices, base object data ---
  for (int z = 0; z < SIZE; ++z) {
    for (int x = 0; x < SIZE; ++x) {
      int sx = clampi(x, 0, img.width  - 1);
      int sz = clampi(z, 0, img.height - 1);
      Color c = GetImageColor(img, sx, sz);

      Terrain *t = &gTerrain[x][z];

      // t\LandHeight = ColorGreen() * .05
      t->landHeight  = (float)c.g * 0.05f;
      t->objectHeight = t->landHeight;

      // t\Landindex = ( ColorBlue() + 20 ) / 50
      t->landIndex = (c.b + 20) / 50;

      // t\ObjectIndex = ( ColorRed() + 10 ) / 20
      t->objectIndex = (c.r + 10) / 20;
      if (t->objectIndex > GROUND_OBJECTS) t->objectIndex = 0;

      // t\ObjectYaw = Rand(360)
      t->objectYaw    = GetRandomValue(0, 359);
      t->objectPitch  = 0.0f;
      t->objectSway   = 0.0f;
      t->objectStatus = 0;
      t->objectCycle  = (x + z) % 20;
      t->landInfected = 0;
      t->landHidden   = 0;
      t->waveFactor   = sqrtf(powf((float)x - 128.0f, 2) + powf((float)z - 128.0f, 2)) * 45.0f;

      // If t\ObjectIndex > 0  t\ObjectHeight = t\LandHeight + GroundObject(idx)\height
      if (t->objectIndex > 0) {
        t->objectHeight = t->landHeight + gGroundObjects[t->objectIndex].height;
      }

      // Trees (objectIndex < 5): random pitch & sway
      if (t->objectIndex > 0 && t->objectIndex < 5) {
        t->objectPitch = (float)GetRandomValue(-10, 10);
        // Original Blitz (30Hz) was 0.5. For 60Hz we use 0.25.
        t->objectSway  = (float)((GetRandomValue(0,1)*2 - 1)) * 0.25f;
      }
      // Rockets: fixed yaw
      if (t->objectIndex == 10) {
        t->objectYaw = 0;
      }
    }
  }

  UnloadImage(img);

  // --- Pass 2: per-cell biome colors (mirrors TERRAINload coloring block) ---
  for (int z = 0; z < SIZE; ++z) {
    for (int x = 0; x < SIZE; ++x) {
      Terrain *t = &gTerrain[x][z];
      float th = TerrainGetHeight((float)x + 0.5f, (float)z + 0.5f, 0) * 20.0f;

      switch (t->landIndex) {
        case 0: // sea
          t->r[0] = (uint8_t)clampi(60 + GetRandomValue(0,39), 0, 255);
          t->g[0] = (uint8_t)clampi(60 + GetRandomValue(0,39), 0, 255);
          t->b[0] = (uint8_t)clampi((int)(150 + th*10) + GetRandomValue(0,39), 0, 255);
          t->landInfected = 1;
          gAreaInfected++;
          break;
        case 1: // beach
          t->r[0] = (uint8_t)clampi((int)(th*10 + 60), 0, 255);
          t->g[0] = (uint8_t)clampi((int)(th*10 + 60), 0, 255);
          t->b[0] = (uint8_t)clampi((int)(200 - th*10) + GetRandomValue(0,39), 0, 255);
          break;
        case 2: // land
          t->r[0] = (uint8_t)clampi((int)th + GetRandomValue(0,59), 0, 255);
          t->g[0] = (uint8_t)clampi((int)(th + 60) + GetRandomValue(0,59), 0, 255);
          t->b[0] = (uint8_t)clampi((int)th + GetRandomValue(0,59), 0, 255);
          break;
        case 3: { // landing pad
          int detail = ((z > 125 && z < 131) && (x == 125 || x == 129)) ||
                       (x > 125 && x < 129 && z == 128);
          int col = (detail ? 155 : 0) + (detail ? 0 : GetRandomValue(0,99)) + 100;
          t->r[0] = (uint8_t)clampi(col, 0, 255);
          t->g[0] = (uint8_t)clampi(col, 0, 255);
          t->b[0] = (uint8_t)clampi(col, 0, 255);
        } break;
        case 4: // alien buildings
          t->r[0] = (uint8_t)clampi((int)th + 128, 0, 250);
          t->g[0] = (uint8_t)clampi((int)th + GetRandomValue(32,127), 0, 250);
          t->b[0] = (uint8_t)clampi((int)th + 128, 0, 250);
          t->landInfected = 1;
          break;
        default:
          t->r[0] = t->g[0] = t->b[0] = 128;
          break;
      }

      // Infected colour (dimmer, more purple)
      if (t->landIndex > 0) {
        t->r[1] = (uint8_t)clampi((int)((t->r[0] + 250) * 0.4f), 0, 255);
        t->g[1] = (uint8_t)clampi((int)((t->g[0] +  50) * 0.4f), 0, 255);
        t->b[1] = (uint8_t)clampi((int)((t->b[0] + 100) * 0.4f), 0, 255);
      } else {
        t->r[1] = t->r[0];
        t->g[1] = t->g[0];
        t->b[1] = t->b[0];
      }

      // Halfbright argb for minimap (mirrors the Blitz ARGB calculation)
      t->argb[0] = (0xFF000000u |
                    ((uint32_t)t->r[0] << 16) |
                    ((uint32_t)t->g[0] <<  8) |
                     (uint32_t)t->b[0]) >> 1;
      t->argb[1] = (0xFF000000u |
                    ((uint32_t)t->r[1] << 16) |
                    ((uint32_t)t->g[1] <<  8) |
                     (uint32_t)t->b[1]) >> 1;
    }
  }

  TraceLog(LOG_INFO, "TerrainLoad: loaded %s heights and colors", path);
  return true;
}


// Direct port of TERRAINgetheight# used throughout the game.
float TerrainGetHeight(float bx, float bz, int check) {
  // Robust Wrap into [0, SIZE).
  bx = wrapf(bx, (float)SIZE, 0.0f);
  bz = wrapf(bz, (float)SIZE, 0.0f);

  float tx = floorf(bx);
  float tz = floorf(bz);
  float ix = bx - tx;
  float iz = bz - tz;

  int itx = (int)tx;
  int itz = (int)tz;
  
  // Extra safety: Clamp indices to [0, SIZE-1] to prevent out-of-bounds due to precision
  itx = clampi(itx, 0, SIZE - 1);
  itz = clampi(itz, 0, SIZE - 1);

  int icx = (itx + 1) % SIZE;
  int icz = (itz + 1) % SIZE;

  float v1 = 0.0f;
  float v2 = 0.0f;

  if (check == 0) {
    v1 = gTerrain[itx][itz].landHeight +
         (gTerrain[icx][itz].landHeight - gTerrain[itx][itz].landHeight) * ix;
    v2 = gTerrain[itx][icz].landHeight +
         (gTerrain[icx][icz].landHeight - gTerrain[itx][icz].landHeight) * ix;
  } else {
    v1 = gTerrain[itx][itz].objectHeight +
         (gTerrain[icx][itz].objectHeight - gTerrain[itx][itz].objectHeight) * ix;
    v2 = gTerrain[itx][icz].objectHeight +
         (gTerrain[icx][icz].objectHeight - gTerrain[itx][icz].objectHeight) * ix;
  }

  return v1 + (v2 - v1) * iz;
}

// Basic terrain object loading system (placeholder for .my3d format)
// For now, creates simple placeholder meshes for testing
void TerrainInitObjects(void) {
  const char* names[] = {
      "", "Bush", "tree1", "tree2", "tree3", "house1", "house2", "House3", "Radar1", "Mill1", "Rocket", "Crate"
  };

  for (int i = 0; i <= GROUND_OBJECTS; i++) {
    gGroundObjects[i].height = 1.0f;  // Fallback height
    gGroundObjects[i].name = names[i];
    if (i > 0) {
        int shape = (i >= 5 && i <= 7) ? 3 : 2; // Houses squash, others implode
        gGroundObjects[i].meshState[0] = LoadMy3D(names[i]);
        gGroundObjects[i].meshState[1] = LoadMy3DState(names[i], 1);
        gGroundObjects[i].meshState[2] = LoadMy3DState(names[i], shape);
        gGroundObjects[i].meshState[3] = LoadMy3DState(names[i], 4); // Special code 4 for Infected-Destroyed
        
        // Calculate height from base mesh
        if (gGroundObjects[i].meshState[0].meshCount > 0) {
            BoundingBox box = GetModelBoundingBox(gGroundObjects[i].meshState[0]);
            gGroundObjects[i].height = (box.max.y - box.min.y) + 0.5f; 
            TraceLog(LOG_INFO, "TerrainInitObjects: '%s' height = %.2f", names[i], gGroundObjects[i].height);
        }
    }
  }

  // Composites
  const char* extras[] = { "Radar2", "Mill2" };
  int extraIndices[] = { 8, 9 };
  for (int j = 0; j < 2; j++) {
      int idx = extraIndices[j];
      const char* mName = extras[j];
      gGroundObjects[idx].meshExtra[0] = LoadMy3D(mName);
      gGroundObjects[idx].meshExtra[1] = LoadMy3DState(mName, 1);
      gGroundObjects[idx].meshExtra[2] = LoadMy3DState(mName, 2); // Implode
      gGroundObjects[idx].meshExtra[3] = LoadMy3DState(mName, 4); // Infected-implode
  }

  // Rocket and Crate usually have specific physics or are small
  // gGroundObjects[10].height = 1.5f; // Rocket
  // gGroundObjects[11].height = 1.0f; // Crate
}

// Render terrain objects near the player (simplified version of TERRAINupdate)
void TerrainRenderObjects(const Ship *player) {
    TerrainRenderObjectsEx(player, NULL);
}

void TerrainRenderObjectsEx(const Ship *player, Shader *overrideShader) {
  if (!player) return;

  int ax = (int)floorf(player->x);
  int az = (int)floorf(player->z);
  float fx = player->x - (float)ax;
  float fz = player->z - (float)az;

  // We check the area defined by gGrid.view
  int range = gGrid.view;

  for (int dz = -range; dz <= range; dz++) {
    for (int dx = -range; dx <= range; dx++) {
      int x = wrapi(ax + dx, SIZE, 0);
      int z = wrapi(az + dz, SIZE, 0);

      Terrain *t = &gTerrain[x][z];
      if (t->objectIndex > 0 && t->objectIndex <= GROUND_OBJECTS) {
        // Place object at position relative to player (ox, oz)
        float ox = (float)dx - fx;
        float oz = (float)dz - fz;
        float oy = t->landHeight;

        // Draw object if within grid bounds (Source.bb 1745)
        if (fabsf(ox) < (float)gGrid.cull + 0.5f && fabsf(oz) < (float)gGrid.cull + 0.5f) {
            float worldX = player->x + ox;
            float worldZ = player->z + oz; 

            // Render based on state (0=normal, 1=infected, 2=destroyed)
            int state = t->objectStatus;
            if (state < 0) state = 0;
            if (state > 2) state = 2;
            
            // Special Destroyed-Infected state
            if (state == 2 && t->landInfected > 0) state = 3;

            GroundObject *G = &gGroundObjects[t->objectIndex];
            Model model = G->meshState[state];
            
            // Adjust drawing for height and rotate animation
            if (model.meshCount > 0) {
                rlPushMatrix();
                // OSCALE is applied in loader but Blitz shifts heights slightly.
                rlTranslatef(worldX, oy, worldZ);
                
                // Rotations in Blitz: Pitch, then Yaw. And Z is flipped.
                float yaw = (float)t->objectYaw;
                
                // Radar/Mill/Crate animation
                if (t->objectStatus < 2) {
                    if (t->objectIndex == 11) yaw = gRotate * 2.0f; // Crate
                }
                
                
                rlRotatef(-yaw, 0, 1, 0);
                rlRotatef(-t->objectPitch, 1, 0, 0);
                
                if (overrideShader) {
                   for (int m = 0; m < model.meshCount; m++) {
                       Shader old = model.materials[model.meshMaterial[m]].shader;
                       model.materials[model.meshMaterial[m]].shader = *overrideShader;
                       DrawMesh(model.meshes[m], model.materials[model.meshMaterial[m]], MatrixIdentity());
                       model.materials[model.meshMaterial[m]].shader = old;
                   }
                } else {
                   DrawModel(model, (Vector3){0,0,0}, 1.0f, WHITE);
                }

                // Draw Composite Part
                Model extra = G->meshExtra[state];
                if (extra.meshCount > 0) {
                    if (t->objectIndex == 8) { // Radar dish
                        rlPushMatrix();
                        if (t->objectStatus < 2) rlRotatef(gRotate, 0, 1, 0); // Only spin if not destroyed
                        if (overrideShader) {
                            for (int m = 0; m < extra.meshCount; m++) {
                                Shader old = extra.materials[extra.meshMaterial[m]].shader;
                                extra.materials[extra.meshMaterial[m]].shader = *overrideShader;
                                DrawMesh(extra.meshes[m], extra.materials[extra.meshMaterial[m]], MatrixIdentity());
                                extra.materials[extra.meshMaterial[m]].shader = old;
                            }
                        } else {
                            DrawModel(extra, (Vector3){0,0,0}, 1.0f, WHITE);
                        }
                        rlPopMatrix();
                    } else if (t->objectIndex == 9) { // Mill blades
                        rlPushMatrix();
                        rlTranslatef(0, 1.25f, 0); 
                        if (t->objectStatus < 2) rlRotatef(gRotate * 5.0f, 0, 0, 1); // Only spin if not destroyed
                        if (overrideShader) {
                            for (int m = 0; m < extra.meshCount; m++) {
                                Shader old = extra.materials[extra.meshMaterial[m]].shader;
                                extra.materials[extra.meshMaterial[m]].shader = *overrideShader;
                                DrawMesh(extra.meshes[m], extra.materials[extra.meshMaterial[m]], MatrixIdentity());
                                extra.materials[extra.meshMaterial[m]].shader = old;
                            }
                        } else {
                            DrawModel(extra, (Vector3){0,0,0}, 1.0f, WHITE);
                        }
                        rlPopMatrix();
                    } else {
                        if (overrideShader) {
                            for (int m = 0; m < extra.meshCount; m++) {
                                Shader old = extra.materials[extra.meshMaterial[m]].shader;
                                extra.materials[extra.meshMaterial[m]].shader = *overrideShader;
                                DrawMesh(extra.meshes[m], extra.materials[extra.meshMaterial[m]], MatrixIdentity());
                                extra.materials[extra.meshMaterial[m]].shader = old;
                            }
                        } else {
                            DrawModel(extra, (Vector3){0,0,0}, 1.0f, WHITE);
                        }
                    }
                }

                rlPopMatrix();
            } else {
                float height = G->height;
                Color tint = WHITE;
                if (state == 1) tint = DARKPURPLE;
                if (state == 2) tint = DARKGRAY;
                DrawCube((Vector3){worldX, oy + height * 0.5f, worldZ}, 0.5f, height, 0.5f, tint);
            }
        }
      }
    }
  }
}

// Update tree/bush sway and emit particles over time (mirrors Source.bb loops)
void TerrainUpdateObjects(const Ship *player) {
    if (!player) return;

    int ax = (int)floorf(player->x);
    int az = (int)floorf(player->z);
    int range = gGrid.view;

    for (int dz = -range; dz <= range; dz++) {
        for (int dx = -range; dx <= range; dx++) {
            int x = wrapi(ax + dx, SIZE, 0);
            int z = wrapi(az + dz, SIZE, 0);

            Terrain *t = &gTerrain[x][z];
            if (t->objectIndex <= 0) continue;

            // Increment cycle (Source.bb 1747)
            // Original: wrap 20 at 30fps. 
            // At 60fps, we use 40 to maintain the original emission frequency.
            t->objectCycle = (t->objectCycle + 1) % 40;

            // Tree/Bush sway logic (Source.bb 1765)
            if (t->objectIndex < 5 && t->objectStatus < 2) {
                t->objectPitch += t->objectSway;
                if (fabsf(t->objectPitch) > 15.0f) {
                    t->objectSway = -t->objectSway;
                }
            }

            // Particle emission (Source.bb 1772-1777)
            // Safety: only spawn if we have spare capacity to avoid burying bullets
            if (t->objectCycle == 0 && gActiveParticles < (MAX_PARTICLES - 200)) {
                if (t->objectStatus == 2) {
                    // Destroyed smoke (Template 3)
                    float py = t->landHeight + gGroundObjects[t->objectIndex].height * 0.75f;
                    ParticleNew(3, (float)x, py, (float)z, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0);
                } else if (t->objectStatus == 1 && t->objectIndex < 5) {
                    // Infected tree infection spill (Template 10)
                    ParticleNew(10, (float)x, t->objectHeight, (float)z, 0, 0.25f, 0, 1, 0, 0, 0, 0, 0, 0);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// TerrainMapAdd — Port of Source.bb MAPadd. 
// Handles infection spread and radar-blocking map updates.
// ---------------------------------------------------------------------------
void TerrainMapAdd(int mode, float sx, float sz, int ss) {
    sx = wrapf(sx, (float)SIZE, 0.0f);
    sz = wrapf(sz, (float)SIZE, 0.0f);
    int isx = (int)floorf(sx);
    int isz = (int)floorf(sz);
    
    if (mode == 0) { // Infected
        for (int vx = -ss; vx <= ss; vx++) {
            int x = wrapi(isx + vx, SIZE, 0);
            for (int vz = -ss; vz <= ss; vz++) {
                int z = wrapi(isz + vz, SIZE, 0);
                Terrain *t = &gTerrain[x][z];
                if (t->landInfected == 0) {
                    t->landInfected = 1;
                    gAreaInfected++;
                    if (t->objectStatus == 0) t->objectStatus = 1;
                    
                    // Queue incremental map update (mirrors Source.bb 1594-1599)
                    if (t->landHidden == 0 && gMapCounter < 5000) {
                        gMapChange[0][gMapCounter] = (uint32_t)x;
                        gMapChange[1][gMapCounter] = (uint32_t)z;
                        gMapChange[2][gMapCounter] = t->argb[1];
                        gMapCounter++;
                    }
                }
            }
        }
    } else if (mode == 1) { // Hidden (Radar blocking)
        int base_sx = (int)floorf((sx + 16.0f) / 32.0f) * 32;
        int base_sz = (int)floorf((sz + 16.0f) / 32.0f) * 32;
        for (int vx = -16; vx <= 15; vx++) {
            int x = wrapi(base_sx + vx, SIZE, 0);
            for (int vz = -16; vz <= 15; vz++) {
                int z = wrapi(base_sz + vz, SIZE, 0);
                Terrain *t = &gTerrain[x][z];
                t->landHidden = 1;
                
                // Queue map black-out (mirrors Source.bb 1613-1616)
                if (gMapCounter < 5000) {
                    gMapChange[0][gMapCounter] = (uint32_t)x;
                    gMapChange[1][gMapCounter] = (uint32_t)z;
                    gMapChange[2][gMapCounter] = 0; // Black/Hidden
                    gMapCounter++;
                }
            }
        }
    }
}

void TerrainCollisionGround(float sx, float sy, float sz, int inView, int isShip, int index) {
    (void)sy; (void)inView; (void)isShip;
    sx = wrapf(sx, (float)SIZE, 0.0f);
    sz = wrapf(sz, (float)SIZE, 0.0f);
    int isx = (int)floorf(sx);
    int isz = (int)floorf(sz);

    for (int fx = 0; fx <= 1; fx++) {
        int cx = wrapi(isx + fx, SIZE, 0);
        for (int fz = 0; fz <= 1; fz++) {
            int cz = wrapi(isz + fz, SIZE, 0);
            Terrain *t = &gTerrain[cx][cz];
            if (t->objectIndex > 0 && t->objectStatus < 2) {
                if (t->objectStatus == 1)      ScoreTagAdd((float)cx, t->objectHeight, (float)cz, 1, 40);
                else if (t->objectStatus == 0) ScoreTagAdd((float)cx, t->objectHeight, (float)cz, 0, -40);
                
                t->objectHeight = t->landHeight;
                if (t->objectIndex == 8) TerrainMapAdd(1, (float)cx, (float)cz, 1);
                if (t->objectIndex == 11 && index == 0) gShips[0].spread++;
                t->objectStatus = 2; // Destroyed
            }
        }
    }
}

