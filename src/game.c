#include <stdio.h>
#include "game.h"
#include "ship.h"
#include "terrain.h"
#include "util.h"
#include "world.h"
#include "raylib.h"
#include "audio.h"
#include "camera_game.h"
#include "particle.h"

void GameInit(GameContext *g) {
  (void)g;
  (void)UtilSelfTest();

  FlyingObjectInit();
  WaveDataInit();
  TerrainInitObjects();
  gGrid.cull = 0;
}

void GameStart(GameContext *g) {
  g->flags.quit = false;
  g->flags.gameOver = false;
  g->flags.waveComplete = false;

  g->wave = 0;
  gScore = 0;
  g->lives = 3;

  ShipInitAll();
}

void GameUpdate(GameContext *g) {
  // Update all active players/enemies

  // Update enemy AI happens inside ShipUpdateAll (which main.c calls)
  /*
  for (int i = 1; i < MAX_SHIPS; i++) {
    if (gShips[i].dead == 0 && gShips[i].ai > 0) {
      ShipAI(&gShips[i], dt);
    }
  }
  */

  // Check for wave completion
  if (gEnemies <= 0) {
    g->flags.waveComplete = true;
  }

  // --- Environment Particles (Rain, Stars, Clouds) ---
  Ship *camP = &gShips[gCam];
  if (camP->dead == 0) {
      if (camP->y > G_ENV_THRESHOLD) {
          // Space Dust / Cloud Layer (Templates 16/17)
          // Scale count by view distance to keep density consistent
          float density = (camP->y > MAX_HEIGHT) ? G_CLOUD_DENSITY : G_DUST_DENSITY;
          int count = (int)(gGrid.view * density); 
          if (count > 64) count = 64; 
          for (int l = 0; l < count; l++) {
              float rx = (float)GetRandomValue(-gGrid.view, gGrid.view) + camP->vx * 20.0f;
              float rz = (float)GetRandomValue(-gGrid.view, gGrid.view) - camP->vz * 20.0f;
              float ry = clampf(camP->y + (float)GetRandomValue(-gGrid.divg, gGrid.divg), G_ENV_THRESHOLD, 400.0f);
              // Template 16 (Star/Dust) or 17 (Too High/Cloud)
              ParticleNew(16 + (ry > MAX_HEIGHT), camP->x, 0.0f, camP->z, rx, ry, -rz, 1, 0, 0, 0, 0, 0.0f, 0.0f);
          }
      } else {
          // Rain (Template 9)
          if (gAreaTotal > 0) {
              // Blitz logic: No = ( 1+ GridSize * 2.0 ) * ( ( Rand( AreaTotal ) -50 ) < AreaInfected * 4.0 )
              bool shouldRain = ((float)(GetRandomValue(0, gAreaTotal) - G_RAIN_BIAS) < (float)gAreaInfected * G_RAIN_FREQ_SCALE);
              if (shouldRain) {
                  int count = (int)(gGrid.view * G_RAIN_DENSITY);
                  if (count > 48) count = 48;
                  for (int l = 0; l < count; l++) {
                      // Project rain ahead of player along the velocity vector
                      float leadFrames = G_RAIN_LEAD * FPS;
                      float rx = (float)GetRandomValue(-gGrid.divs, gGrid.divs) + camP->vx * leadFrames;
                      float ry = clampf(camP->y + (float)GetRandomValue(gGrid.divs, gGrid.divs + 10), 10.0f, G_ENV_THRESHOLD + 5.0f);
                      float rz = (float)GetRandomValue(-gGrid.divs, gGrid.divs) - camP->vz * leadFrames;
                      
                      // ParticleNew(id, base_x, base_y, base_z, offset_x, offset_y, offset_z, ...)
                      // We pass rx/ry/rz as offsets from the player's current position.
                      ParticleNew(9, camP->x, 0.0f, camP->z, rx, ry, -rz, 1, 0, 0, 0, 0, 0.0f, 0.0f);
                  }
              }
          }
      }
  }
}

void GameEnd(GameContext *g) {
  AudioPauseAmbient(true);
  g->state = STATE_GAME_END;
}

void WaveStart(GameContext *g) {
  g->wave += 1;
  g->flags.waveComplete = false;

  int waveIdx = g->wave - 1;
  if (waveIdx < 0) waveIdx = 0;
  if (waveIdx >= MAX_WAVES) waveIdx = MAX_WAVES - 1;

  WaveData *waveData = &gWaveData[waveIdx];

  // 1. Map Loading: If mapName is empty, it continues with current map.
  if (waveData->mapName[0] != '\0') {
    char filename[128];
    snprintf(filename, sizeof(filename), "%s.bmp", waveData->mapName);
    TerrainLoad(filename);
  }

  // 2. Reset player count and death statuses 
  for (int i = 0; i <= MAX_SHIPS; i++) {
    gShips[i].dead = -1; // -1 means truly dead/inactive
  }

  int playersCount = 0;
  gEnemies = 0;

  if (g->isTestLevel) {
      // 3a. Test Level Spawning
      
      // Spawn Ship (Index 0, AI 18 = Allie/Ship)
      ShipReset(0, 18);
      gShips[0].yaw = 0.0f; // Face exactly +Z
      playersCount++;

      // Spawn Test Enemy (Index 1)
      ShipReset(1, G_TEST_AI_TYPE);
      Ship *enemy = &gShips[1];
      enemy->x = gShips[0].x;
      enemy->z = gShips[0].z + G_TEST_SPAWN_DIST; // +Z is forward in this engine's math
      enemy->y = TerrainGetHeight(enemy->x, enemy->z, 1) + 
                 (float)gFlyingObjects[G_TEST_AI_TYPE].cruiseHeight;
      enemy->yaw = 180.0f; // Face the player
      enemy->chaseTimer = 0; // Trigger immediate evaluation
      if (enemy->index == 1) gEnemies++;
      playersCount++;
      
  } else {
      // 3b. Normal Spawning loop
      for (int ai = 0; ai < 19; ai++) {
        int number = waveData->unitCount[ai];

        for (int l = 0; l < number; l++) {
          if (playersCount >= MAX_SHIPS) break;

          ShipReset(playersCount, ai);
          if (gShips[playersCount].index == 1) {
            gEnemies++;
          }
          int parentIdx = playersCount;
          playersCount++;

          // 4. Child Spawning
          int childAI = 0;
          switch (ai) {
            case 0:  childAI = 20; break;
            case 10: childAI = 19; break;
            case 11: childAI = 4;  break;
            case 12: childAI = 5;  break;
            case 13: childAI = 6;  break;
            case 14: childAI = 7;  break;
            case 15: childAI = 16; break;
            case 16: childAI = 19; break;
          }

          if (childAI > 0) {
            for (int w = 0; w < 3; w++) {
              if (playersCount >= MAX_SHIPS) break;
              ShipReset(playersCount, childAI);
              gShips[playersCount].y = -1; 
              gShips[parentIdx].child[w] = playersCount;
              
              if (gShips[playersCount].index == 1) {
                gEnemies++;
              }
              playersCount++;
            }
          }
        }
      }
  }
  gMaxShips = (playersCount > 0) ? (playersCount - 1) : 0;
}

void WaveEnd(GameContext *g) {
  AudioPauseAmbient(true);
  
  // Real territory tracking
  int infected = 0;
  for (int z = 0; z < SIZE; z++) {
    for (int x = 0; x < SIZE; x++) {
      if (gTerrain[x][z].landInfected > 0) infected++;
    }
  }

  int total = SIZE * SIZE;
  // Source.bb 431: WAVEbonus = ( AreaTotal*.5 - AreaInfected ) / 16
  int bonus = (int)(((float)total * 0.5f - (float)infected) / 16.0f);
  if (bonus < 0) bonus = 0;
  gScore += bonus;

  // Update Wave End menu strings
  snprintf(gMenus[1].sub_name[1], 128, "%d", infected);
  snprintf(gMenus[1].sub_name[2], 128, "%d", total - infected);
  snprintf(gMenus[1].sub_name[3], 128, "%d", bonus);

  g->state = STATE_WAVE_END;
  g->flags.waveComplete = false; // Reset flag
}

