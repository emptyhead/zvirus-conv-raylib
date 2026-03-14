#include "hud.h"
#include <stdio.h>
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
}

void HudUpdateMap(void) {
    // Generate map image based on terrain landIndex and infection
    for (int z = 0; z < SIZE; z++) {
        for (int x = 0; x < SIZE; x++) {
            Terrain *t = &gTerrain[x][z];
            Color c;
            
            // Map colors (mirrors Blitz Map logic if we had it, but choosing standard biome colors)
            switch (t->landIndex) {
                case 0: c = (Color){20, 50, 200, 255}; break; // Sea
                case 1: c = (Color){200, 180, 120, 255}; break; // Beach
                case 2: c = (Color){50, 150, 50, 255}; break; // Land
                case 3: c = WHITE; break; // Landing Pad
                case 4: c = (Color){180, 50, 180, 255}; break; // Alien Buildings
                default: c = GRAY; break;
            }
            
            // If infected, mix with purple
            if (t->landInfected) {
                c.r = (unsigned char)(c.r * 0.5f + 128 * 0.5f);
                c.g = (unsigned char)(c.g * 0.5f);
                c.b = (unsigned char)(c.b * 0.5f + 128 * 0.5f);
            }
            
            ImageDrawPixel(&gMapImage, x, z, c); 
        }
    }
    
    UpdateTexture(gMapTexture, gMapImage.data);
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
    DrawCircle(mapX + (int)px, mapY + (int)pz, 2, WHITE);
    
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
