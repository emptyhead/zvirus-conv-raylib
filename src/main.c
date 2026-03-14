#include <stdio.h>
#include <math.h>

#include "raylib.h"

#include "camera_game.h"
#include "game.h"
#include "ship.h"
#include "terrain.h"
#include "particle.h"
#include "hud.h"
#include "util.h"
#include "world.h"
#include "audio.h"
#include "menu.h"
#include "rlgl.h"
#include "score_tag.h"

static const char *GameStateName(GameState s) {
  switch (s) {
    case STATE_BOOT:
      return "BOOT";
    case STATE_MENU:
      return "MENU";
    case STATE_WAVE:
      return "WAVE";
    case STATE_WAVE_END:
      return "WAVE_END";
    case STATE_GAME_END:
      return "GAME_END";
    case STATE_PAUSE:
      return "PAUSE";
    default:
      return "UNKNOWN";
  }
}

static void DrawDebugUI(const GameContext *g) {
  const int pad = 10;
  DrawRectangle(pad, pad, 420, 210, Fade(BLACK, 0.55f));
  DrawRectangleLines(pad, pad, 420, 210, Fade(RAYWHITE, 0.35f));

  DrawText("zvirus-conv-raylib (port skeleton)", pad + 10, pad + 10, 18, RAYWHITE);
  DrawText(TextFormat("State: %s", GameStateName(g->state)), pad + 10, pad + 35, 16, RAYWHITE);
  DrawText(TextFormat("Wave: %d   Score: %d   Lives: %d   Enemies: %d", g->wave, gScore, g->lives, gEnemies),
           pad + 10, pad + 55, 16, RAYWHITE);
  
  static const char *camNames[] = {"CHASE", "REAR"};
  DrawText(TextFormat("Camera Mode: %s [C]", camNames[gCameraMode]), pad + 10, pad + 75, 16, LIME);
  DrawText(TextFormat("View Dist: %d [F9/F10]   Speed: %.1fx [Num+/-]", (int)g->gridSize + 1, g->timeScale), 
           pad + 10, pad + 95, 16, GOLD);

  DrawText("Keys: [Enter]=start/next  [Space]=complete wave  [Q]=quit", pad + 10, pad + 100, 14,
           RAYWHITE);

  // Simple 64x64 terrain height preview (top-down) in the debug panel.
  const int mapSize = 64;
  const int cell = SIZE / mapSize;
  const int mapX = pad + 10;
  const int mapY = pad + 110;
  for (int z = 0; z < mapSize; ++z) {
    for (int x = 0; x < mapSize; ++x) {
      int tx = clampi(x * cell, 0, SIZE - 1);
      int tz = clampi(z * cell, 0, SIZE - 1);
      float h = gTerrain[tx][tz].landHeight;  // 0..~?
      unsigned char shade = (unsigned char)clampf(h * 12.0f, 0.0f, 255.0f);
      DrawPixel(mapX + x, mapY + z, (Color){shade, shade, shade, 255});
    }
  }
}

static Model gTerrainModel = { 0 };

static void DrawFullFrame(const GameContext *g) {
    BeginDrawing();
    ClearBackground((Color){24, 24, 28, 255});

    Ship *p = &gShips[gCam];

    BeginMode3D(gCamera3D);

    // Terrain mesh (vertex-colored, centered on player)
    if (gTerrainModel.meshes != NULL) {
      // Position is relative to camera, but vertices are already offset 
      // relative to player in TerrainUpdateGrid.
      // So we draw at player's location to align vertices with world objects.
      DrawModel(gTerrainModel, (Vector3){p->x, 0, p->z}, 1.0f, WHITE);
    }

    // All live entities as colored cubes
    for (int i = 0; i < MAX_SHIPS; i++) {
      Ship *q = &gShips[i];
      if (q->dead != 0) continue;
      
      FlyingObject *F = &gFlyingObjects[q->ai];

      Color col = q->dead == 0 ? (Color){gFlyingObjects[q->ai].r, gFlyingObjects[q->ai].g, gFlyingObjects[q->ai].b, 255} : GRAY;
      if (i == 0) col = RED; // Keep player distinct

      // World wrapping awareness for entity rendering
      float dx = q->x - p->x;
      if (dx > (float)SIZE/2.0f) dx -= (float)SIZE;
      if (dx < -(float)SIZE/2.0f) dx += (float)SIZE;
      float dz = q->z - p->z;
      if (dz > (float)SIZE/2.0f) dz -= (float)SIZE;
      if (dz < -(float)SIZE/2.0f) dz += (float)SIZE;

      // Clip to terrain size
      if (fabsf(dx) >= (float)gGrid.cull + 0.5f || fabsf(dz) >= (float)gGrid.cull + 0.5f) continue;

      Vector3 drawPos = { p->x + dx, q->y + 0.5f, p->z + dz };

      // Draw rotated ship mesh
      rlPushMatrix();
      rlTranslatef(drawPos.x, drawPos.y, drawPos.z);
      
      float drawYaw = q->yaw;
      float drawPitch = q->pitch;
      
      // Apply spin animation
      if (F->spinSpeed > 0.0f) {
          drawYaw += q->spin;
          if (q->ai == 3 || q->ai == 19) { // Pest or Spore
              drawPitch += q->spin;
          }
      }
      
      // Negate yaw and pitch to match Raylib's right-handed CCW rotation with our CW physics
      rlRotatef(-drawYaw, 0, 1, 0); 
      rlRotatef(-drawPitch, 1, 0, 0);
      
      if (F->mesh.meshCount > 0) {
          if (q->dead != 0) {
              DrawModel(F->mesh, (Vector3){0, 0, 0}, 1.0f, GRAY);
          } else {
              DrawModel(F->mesh, (Vector3){0, 0, 0}, 1.0f, WHITE);
          }
      } else {
          // Fallback slightly rectangular cube
          DrawCube((Vector3){0}, 0.8f, 0.4f, 1.5f, col);
          DrawCubeWires((Vector3){0}, 0.8f, 0.4f, 1.5f, (i == 0) ? MAROON : DARKBLUE);
      }
      
      rlPopMatrix();

      // 1. Orientation Indicator (Facing) - White for player, Gray for others
      {
        float al = 6.0f; 
        float yr = q->yaw   * DEG2RAD;
        float pr = q->pitch * DEG2RAD;
        
        float fx =  sinf(yr) * cosf(pr);
        float fy = -sinf(pr);
        float fz =  cosf(yr) * cosf(pr);
        
        Vector3 start = { drawPos.x + fx * 0.6f, drawPos.y + fy * 0.6f, drawPos.z - fz * 0.6f };
        Vector3 end   = { drawPos.x + fx * al,   drawPos.y + fy * al,   drawPos.z - fz * al };
        
        Color facingCol = (i == 0) ? WHITE : LIGHTGRAY;
        DrawLine3D(start, end, facingCol);
        // Offset for thickness
        DrawLine3D((Vector3){start.x, start.y + 0.04f, start.z}, (Vector3){end.x, end.y + 0.04f, end.z}, facingCol);
      }

      // 2. Thrust Indicator (Movement Force) - Yellow for player
      if (q->thrustIntent > 0.0f) {
          float tx, ty, tz;
          if (F->engine == 0) {
              // Upward thrust for helicopters
              TFormVector(0, 1, 0, q->pitch, q->yaw, &tx, &ty, &tz);
          } else {
              // Forward thrust for jets
              TFormForward(q->pitch, q->yaw, F->engine, &tx, &ty, &tz);
          }
          
          float length = 5.0f;
          Vector3 start = drawPos; 
          Vector3 end   = { drawPos.x + tx * length, drawPos.y + ty * length, drawPos.z - tz * length };
          
          Color thrustCol = (i == 0) ? YELLOW : ((q->index == 0) ? DARKGREEN : ORANGE);
          // Offset slightly so it doesn't overlap perfectly with orientation
          Vector3 startOff = { start.x + 0.1f, start.y + 0.1f, start.z };
          Vector3 endOff   = { end.x + 0.1f,   end.y + 0.1f,   end.z };
          
          DrawLine3D(startOff, endOff, thrustCol);
          DrawLine3D((Vector3){startOff.x, startOff.y + 0.04f, startOff.z}, (Vector3){endOff.x, endOff.y + 0.04f, endOff.z}, thrustCol);
      }
    }

    TerrainRenderObjects(p);
    ParticleDrawAll();
    ScoreTagDrawAll();

    EndMode3D();
    
    // --- 2D Entity Labels ---
    for (int i = 1; i < MAX_SHIPS; i++) {
        Ship *q = &gShips[i];
        if (q->dead != 0) continue;
        
        // Re-calculate wrapped world position relative to camera
        float dx = q->x - p->x;
        if (dx > (float)SIZE/2.0f) dx -= (float)SIZE;
        if (dx < -(float)SIZE/2.0f) dx += (float)SIZE;
        float dz = q->z - p->z;
        if (dz > (float)SIZE/2.0f) dz -= (float)SIZE;
        if (dz < -(float)SIZE/2.0f) dz += (float)SIZE;
        
        // Clip to terrain size
        if (fabsf(dx) >= (float)gGrid.cull + 0.5f || fabsf(dz) >= (float)gGrid.cull + 0.5f) continue;
        
        Vector3 drawPos = { p->x + dx, q->y + 1.5f, p->z + dz };
        Vector2 screenPos = GetWorldToScreen(drawPos, gCamera3D);
        
        if (screenPos.x > 0 && screenPos.x < GetScreenWidth() && screenPos.y > 0 && screenPos.y < GetScreenHeight()) {
            const char *name = gFlyingObjects[q->ai].name;
            int fontSize = 10;
            int textWidth = MeasureText(name, fontSize);
            DrawText(name, (int)screenPos.x - textWidth / 2, (int)screenPos.y, fontSize, YELLOW);
        }
    }

    // HUD (2D Overlay)
    if (g->state == STATE_WAVE) {
        HudDraw(g);
        DrawDebugUI(g);
    } else if (g->state == STATE_MENU) {
        MenuDraw(0);
        HudDraw(g); // Always Draw HUD for the Fade Overlay
    } else if (g->state == STATE_WAVE_END) {
        MenuDraw(1);
        HudDraw(g);
    } else if (g->state == STATE_GAME_END) {
        MenuDraw((g->wave >= 21) ? 4 : 2);
        HudDraw(g);
    } else if (g->state == STATE_PAUSE) {
        MenuDraw(3);
        HudDraw(g);
    }

    EndDrawing();
}

void HudFade(GameContext *g, float target, float speed) {
    if (target == 1.0f) {
        // mirrors Blitz: one-off updates if target is 1.0 (In-game)
        ShipUpdateAll(0);
        GameUpdate(g);
        ParticleUpdateAll();
        CameraGameUpdate(1.0f);
        HudUpdateMap();
        TerrainUpdateGrid(&gShips[gCam]);
    }
    if (target == 2.0f) target = 1.0f;

    // Adjust speed for 60Hz. Original Blitz was 30Hz.
    float perFrame = speed * (30.0f / (float)g->fps);

    // Blocking loop as per Source.bb
    while (gFadeStatus != target && !WindowShouldClose() && !g->flags.quit) {
        gFadeStatus = clampf(gFadeStatus + perFrame, 0.0f, 1.0f);
        
        // In Blitz, it limits at Target too
        if (speed > 0 && gFadeStatus > target) gFadeStatus = target;
        if (speed < 0 && gFadeStatus < target) gFadeStatus = target;

        DrawFullFrame(g);
        
        if (BIND_EXIT_KEY > 0 && IsKeyPressed(BIND_EXIT_KEY)) g->flags.quit = true;
    }
}

int main(void) {
  GameContext g = {
      .state = STATE_BOOT,
      .fps = 60,
      .screenWidth = 1280,
      .screenHeight = 720,
      .flags = {.quit = false, .gameOver = false, .waveComplete = false},
      .wave = 0,
      .lives = 3,
      .timeScale = 1.0f,
      .gridSize = 3.0f,
  };

  InitWindow(g.screenWidth, g.screenHeight, "zvirus - raylib port");
  if (!IsWindowReady()) {
    fprintf(stderr, "Failed to initialize window/graphics context.\n");
    return 1;
  }
  SetTargetFPS(g.fps);

  InitAudioDevice();
  if (!IsAudioDeviceReady()) {
    fprintf(stderr, "Warning: audio device not available; continuing without audio.\n");
  }

  GameInit(&g);
  CameraGameInit();
  AudioInit();
  MenuInit();
  g.state = STATE_MENU;

  // Already called inside GameInit()
  ParticleInit();
  ScoreTagInit();
  HudInit();
  gFadeStatus = 0.0f; // Start black
  gGrid.cull = 0;
  TerrainInitGrid(3); // 3 power = Gridsize 3 (max view) mirrors Source.bb 2048 default (user requested size 4)

  if (gTerrainMesh.vertexCount > 0) {
      gTerrainModel = LoadModelFromMesh(gTerrainMesh);
  }

  // Fade in at start (mirrors Source.bb:410 FADE(1, .025))
  HudFade(&g, 1.0f, 0.025f);

  float lastTime = GetTime();

  while (!WindowShouldClose() && !g.flags.quit) {
    float now  = GetTime();
    float dt   = now - lastTime;
    if (dt > 0.1f) dt = 0.1f; // Cap at 100ms to prevent massive jumps after stalls
    lastTime   = now;
    if (BIND_EXIT_KEY > 0 && IsKeyPressed(BIND_EXIT_KEY)) g.flags.quit = true;

    // Manage cursor state based on game mode
    if (g.state == STATE_WAVE) {
        if (!IsCursorHidden()) DisableCursor();
    } else {
        if (IsCursorHidden()) EnableCursor();
    }

    // Cycle camera mode
    if (BIND_CAMERA_KEY > 0 && IsKeyPressed(BIND_CAMERA_KEY)) {
      gCameraMode = (GameCameraMode)((gCameraMode + 1) % 2);
      // Snap chase pivot to current player position on mode change
      gChasePivot.x = gShips[gCam].x;
      gChasePivot.z = gShips[gCam].z;
    }

    switch (g.state) {

      case STATE_MENU: {
        if (MenuUpdate(0)) {
          int selected = MenuGetSelected(0);
          g.isTestLevel = (selected == 5); // Option 5 is "Test Level"

          GameStart(&g);
          HudFade(&g, 0.0f, -0.05f); // Fade to black after menu selection
          
          WaveStart(&g);
          // Seed chase pivot at player start
          gChasePivot.x = gShips[0].x;
          gChasePivot.z = gShips[0].z;
          g.state = STATE_WAVE;
          AudioPauseAmbient(false);
          HudFade(&g, 1.0f, 0.025f);
          lastTime = GetTime(); // Reset timer after blocking fade
        }
      } break;

      case STATE_WAVE: {
        if (BIND_NEXT_WAVE_KEY > 0 && IsKeyPressed(BIND_NEXT_WAVE_KEY)) g.flags.waveComplete = true;
        if (BIND_PAUSE_KEY > 0 && IsKeyPressed(BIND_PAUSE_KEY)) {
            g.state = STATE_PAUSE; 
        }

        // Fixed-step simulation or dt-scaling
        static float accumulator = 0.0f;
        const float step = (1.0f / (float)FPS); // 1/30s
        
        accumulator += dt * g.timeScale;
        
        // Distribute mouse delta across expected simulation steps
        int stepCount = (int)(accumulator / step);
        if (stepCount > 0) {
            Vector2 mDelta = GetMouseDelta();
            gMouseX = mDelta.x / (float)stepCount;
            gMouseY = mDelta.y / (float)stepCount;
        } else {
            gMouseX = 0;
            gMouseY = 0;
        }

        while (accumulator >= step) {
             ShipUpdateAll(step); 
             GameUpdate(&g);
             ParticleUpdateAll();
             ScoreTagUpdateAll(step);
             TerrainUpdateObjects(&gShips[gCam]);
             gRipple = wrapf(gRipple + 15.0f * (step / (1.0f/30.0f)), 360.0f, 0.0f); // Match Blitz 30Hz speed
             gRotate = wrapf(gRotate + 2.0f * (step / (1.0f/30.0f)), 360.0f, 0.0f); // Animation global clock
             accumulator -= step;
        }

        // Update the scrolling grid centered on the camera's target player
        TerrainUpdateGrid(&gShips[gCam]);
        
        // Handle F9 / F10 Grid Size
        float lastGridSize = g.gridSize;
        if (BIND_GRID_INC_KEY > 0 && IsKeyPressed(BIND_GRID_INC_KEY)) g.gridSize = clampf(g.gridSize + 1.0f, 0.0f, 3.0f);
        if (BIND_GRID_DEC_KEY > 0 && IsKeyPressed(BIND_GRID_DEC_KEY))  g.gridSize = clampf(g.gridSize - 1.0f, 0.0f, 3.0f);
        
        if ( (int)g.gridSize != (int)lastGridSize ) {
            if (gTerrainModel.meshCount > 0) {
                UnloadModel(gTerrainModel);
                gTerrainModel = (Model){0};
                gTerrainMesh = (Mesh){0}; // Mark global mesh as unloaded
            }
            TerrainInitGrid((int)g.gridSize);
            gTerrainModel = LoadModelFromMesh(gTerrainMesh);
        }

        // Handle Time Scale (Slow Motion)
        if (BIND_TIME_INC_KEY > 0 && IsKeyPressed(BIND_TIME_INC_KEY)) g.timeScale = clampf(g.timeScale + 0.1f, 0.1f, 2.0f);
        if (BIND_TIME_DEC_KEY > 0 && IsKeyPressed(BIND_TIME_DEC_KEY)) g.timeScale = clampf(g.timeScale - 0.1f, 0.1f, 2.0f);

        static int mapUpdateCtr = 0;
        if (mapUpdateCtr++ % 60 == 0) HudUpdateMap();

        CameraGameUpdate(0.1f);   // smooth=0.1 mirrors Source.bb default
        
        // Handle Wave/Game End transitions
        if (g.flags.waveComplete) {
            // mirrors Blitz Source.bb:437
            WaveEnd(&g); // Sets state to STATE_WAVE_END
        }
        
        // Handle Ship Death / Lives
        if (gShips[0].dead == 1 && !g.flags.gameOver) {
            g.lives--;
            if (g.lives <= 0) {
                g.flags.gameOver = true;
                GameEnd(&g); // Sets state to STATE_WAVE_END (or similar)
            } else {
                // mirrors Blitz Source.bb:1523
                HudFade(&g, 0.0f, -0.025f);
                
                // Reset player logic would go here if not already handled inside ShipUpdateAll or similar
                // For now, mirroring the fade sequence.
                ShipReset(0, 0); 
                
                HudFade(&g, 1.0f, 0.025f);
                lastTime = GetTime(); // Reset timer after blocking fade
            }
        }

        if (g.flags.gameOver) g.state = STATE_GAME_END;

        AudioUpdate();
      } break;

      case STATE_WAVE_END: {
        if (MenuUpdate(1)) {
          HudFade(&g, 0.0f, -0.025f); // Fade to black AFTER menu selection
          WaveStart(&g);
          g.state = STATE_WAVE;
          AudioPauseAmbient(false);
          HudFade(&g, 1.0f, 0.025f);
          lastTime = GetTime(); // Reset timer after blocking fade
        }
      } break;

      case STATE_GAME_END: {
        int mid = (g.wave >= 21) ? 4 : 2;
        if (MenuUpdate(mid)) {
          g.state = STATE_MENU;
        }
      } break;
      
      case STATE_PAUSE: {
        if (MenuUpdate(3)) {
          if (gMenus[3].current == 2) g.flags.quit = true;
          else {
              g.state = STATE_WAVE;
              AudioPauseAmbient(false);
          }
        }
      } break;

      case STATE_BOOT:
      default:
        g.state = STATE_MENU;
        break;
    }

    DrawFullFrame(&g);
  }
 
  if (gTerrainModel.meshes != NULL) UnloadModel(gTerrainModel);
  CloseAudioDevice();
  CloseWindow();
  return 0;
}

