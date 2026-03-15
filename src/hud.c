#include "hud.h"
#include <stdio.h>
#include <math.h>
#include "raylib.h"
#include "constants.h"
#include "camera_game.h"
#include "world.h"

static Texture2D gMapTexture = {0};
static Image gMapImage = {0};

void HudInit(void) {
    // Initialize map image/texture (256x256)
    gMapImage = GenImageColor(SIZE, SIZE, BLANK);
    gMapTexture = LoadTextureFromImage(gMapImage);
}

void HudUpdate(float dt) {
    (void)dt;
    HudUpdateMapIncremental();
}

void HudUpdateMap(void) {
    // Full generation (used on level load or periodic refresh)
    for (int z = 0; z < SIZE; z++) {
        for (int x = 0; x < SIZE; x++) {
            Terrain *t = &gTerrain[x][z];
            
            // Choose color based on state
            uint32_t argb = t->argb[0];
            if (t->landHidden) {
                argb = 0; // Blacked out by radar jammer
            } else if (t->landInfected) {
                argb = t->argb[1];
            }
            
            // Extract components from the shifted ARGB (mirrors Source.bb bitwise trick)
            Color c = {
                (unsigned char)((argb << 1) >> 16 & 0xFF),
                (unsigned char)((argb << 1) >> 8 & 0xFF),
                (unsigned char)((argb << 1) & 0xFF),
                255
            };
            
            ImageDrawPixel(&gMapImage, x, z, c); 
        }
    }
    
    UpdateTexture(gMapTexture, gMapImage.data);
}

void HudUpdateMapIncremental(void) {
    // Mirrors Source.bb 1628-1640
    if (gMapCounter == 0) return;

    for (int i = 0; i < gMapCounter && i < 5001; i++) {
        int x = (int)gMapChange[0][i];
        int z = (int)gMapChange[1][i];
        uint32_t argb = gMapChange[2][i];

        Color c = {
            (unsigned char)((argb << 1) >> 16 & 0xFF),
            (unsigned char)((argb << 1) >> 8 & 0xFF),
            (unsigned char)((argb << 1) & 0xFF),
            255
        };
        ImageDrawPixel(&gMapImage, x, z, c);
    }

    UpdateTexture(gMapTexture, gMapImage.data);
    gMapCounter = 0;
}

void HudDraw(const GameContext *g) {
    if (!g) return;
    Ship *p = &gShips[gCam];
    
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    
    // --- 1. Fuel and Height Bars (Bottom) ---
    float fuelPct = p->fuel / 500.0f;
    float heightPct = p->y / (float)MAX_HEIGHT;
    
    int barW = 200;
    int barH = 15;
    int margin = 20;
    
    // Fuel Bar (Left)
    DrawRectangle(margin, sh - margin - barH, barW, barH, (Color){40, 40, 40, 200});
    DrawRectangle(margin, sh - margin - barH, (int)(barW * fuelPct), barH, RED);
    DrawText("FUEL", margin, sh - margin - barH - 12, 10, WHITE);
    
    // Height Bar (Right)
    DrawRectangle(sw - margin - barW, sh - margin - barH, barW, barH, (Color){40, 40, 40, 200});
    DrawRectangle(sw - margin - barW, sh - margin - barH, (int)(barW * heightPct), barH, SKYBLUE);
    DrawText("ALT", sw - margin - barW, sh - margin - barH - 12, 10, WHITE);
    
    // --- 2. Score and Stats (Top) ---
    char info[256];
    snprintf(info, sizeof(info), "WAVE: %d   SCORE: %06d   LIVES: %d", g->wave, gScore, g->lives);
    DrawRectangle(0, 0, sw, 30, (Color){0, 0, 0, 180});
    DrawText(info, 20, 8, 16, YELLOW);
    
    // --- 3. Minimap (Top Right) ---
    int mapSize = 128;
    int mapX = sw - mapSize - margin;
    int mapY = 40;
    DrawTexturePro(gMapTexture, (Rectangle){0, 0, SIZE, SIZE}, (Rectangle){(float)mapX, (float)mapY, (float)mapSize, (float)mapSize}, (Vector2){0, 0}, 0.0f, WHITE);
    DrawRectangleLines(mapX, mapY, mapSize, mapSize, WHITE);
    
    // Ship dot on map
    float px = (p->x / (float)SIZE) * (float)mapSize;
    float pz = (p->z / (float)SIZE) * (float)mapSize;
    float mx = (float)mapX + px;
    float my = (float)mapY + pz;
    
    // Direction indicator
    float dirLen = 5.0f;
    float angleRad = p->yaw * DEG2RAD;
    DrawLineV((Vector2){mx, my}, 
              (Vector2){mx + sinf(angleRad) * dirLen, 
                        my - cosf(angleRad) * dirLen}, 
              WHITE);

    DrawCircle((int)mx, (int)my, 2, WHITE);
    
    // Enemy dots
    for (int i = 1; i <= MAX_SHIPS; i++) {
        Ship *e = &gShips[i];
        if (e->dead != 0) continue;
        if (e->index == 1) { // Enemy
            float ex = (e->x / (float)SIZE) * (float)mapSize;
            float ez = (e->z / (float)SIZE) * (float)mapSize;
            DrawCircle(mapX + (int)ex, mapY + (int)ez, 1, RED);
        }
    }
    
    // --- 4. Crosshair ---
    DrawLine(sw/2 - 10, sh/2, sw/2 + 10, sh/2, (Color){255, 255, 255, 150});
    DrawLine(sw/2, sh/2 - 10, sw/2, sh/2 + 10, (Color){255, 255, 255, 150});
    
    // --- 5. Screen Fade Overlay ---
    // gFadeStatus: 0.0 = Opaque Black, 1.0 = Transparent
    // mirrors Blitz EntityAlpha HUD\fade, (1.0 - FadeStatus)
    if (gFadeStatus < 1.0f) {
        float alpha = 1.0f - gFadeStatus;
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, alpha));
    }
}
