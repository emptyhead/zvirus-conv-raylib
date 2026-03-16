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
    
    int barW = HUD_BAR_W;
    int barH = HUD_BAR_H;
    int margin = HUD_MARGIN;
    
    // Fuel Bar (Left)
    DrawRectangle(margin, sh - margin - barH, barW, barH, (Color){40, 40, 40, 200});
    DrawRectangle(margin, sh - margin - barH, (int)(barW * fuelPct), barH, RED);
    DrawText("FUEL", margin, sh - margin - barH - (12 * HUD_SCALE), (int)HUD_FONT_SIZE_LABELS, WHITE);
    
    // Height Bar (Right)
    DrawRectangle(sw - margin - barW, sh - margin - barH, barW, barH, (Color){40, 40, 40, 200});
    DrawRectangle(sw - margin - barW, sh - margin - barH, (int)(barW * heightPct), barH, SKYBLUE);
    DrawText("ALT", sw - margin - barW, sh - margin - barH - (12 * HUD_SCALE), (int)HUD_FONT_SIZE_LABELS, WHITE);
    
    // --- 2. Score and Stats (Top) ---
    char info[256];
    snprintf(info, sizeof(info), "WAVE: %d   SCORE: %06d   LIVES: %d", g->wave, gScore, g->lives);
    DrawRectangle(0, 0, sw, (int)(30 * HUD_SCALE), (Color){0, 0, 0, 180});
    DrawText(info, margin, (int)(8 * HUD_SCALE), (int)HUD_FONT_SIZE_STATS, YELLOW);
    
    // --- 3. Minimap (Top Right) ---
    int mapSize = MINIMAP_SIZE;
    int mapX = sw - mapSize - margin;
    int mapY = (int)(40 * HUD_SCALE);
    DrawTexturePro(gMapTexture, (Rectangle){0, 0, SIZE, SIZE}, (Rectangle){(float)mapX, (float)mapY, (float)mapSize, (float)mapSize}, (Vector2){0, 0}, 0.0f, WHITE);
    DrawRectangleLines(mapX, mapY, mapSize, mapSize, WHITE);
    
    // Ship indicators on map (Current Player/Camera)
    float px = (p->x / (float)SIZE) * (float)mapSize;
    float pz = (p->z / (float)SIZE) * (float)mapSize;
    float mx = (float)mapX + px;
    float my = (float)mapY + pz;
    
    float angleRad = p->yaw * DEG2RAD;
    float viewLen = MINIMAP_VIEW_SIZE;
    float wedgeAngle = 20.0f * DEG2RAD; // 40 degree total wedge
    
    Color viewCol = { 
        MINIMAP_VIEW_COLOR_R, 
        MINIMAP_VIEW_COLOR_G, 
        MINIMAP_VIEW_COLOR_B, 
        MINIMAP_VIEW_ALPHA 
    };

    // Calculate view wedge vertices (originates from dot center)
    // Ordered CCW (v1: origin, v2: +wedge, v3: -wedge) for visibility
    Vector2 v1 = { mx, my };
    Vector2 v2 = { mx + sinf(angleRad + wedgeAngle) * viewLen, my - cosf(angleRad + wedgeAngle) * viewLen };
    Vector2 v3 = { mx + sinf(angleRad - wedgeAngle) * viewLen, my - cosf(angleRad - wedgeAngle) * viewLen };
    
    DrawTriangle(v1, v2, v3, viewCol);

    // Player dot
    FlyingObject *pfo = &gFlyingObjects[p->ai];
    Color pCol = { pfo->r, pfo->g, pfo->b, 255 };
    DrawCircle((int)mx, (int)my, MINIMAP_DOT_PLAYER, pCol);
    
    // Entity dots (Enemies, Allies, etc.)
    for (int i = 0; i <= MAX_SHIPS; i++) {
        if (i == gCam) continue; 
        Ship *e = &gShips[i];
        if (e->dead != 0) continue;
        
        // Skip AI 17 (Monster) as its dot has 0 alpha in Source.bb
        if (e->ai == 17) continue;

        // Skip hidden enemies (Source.bb:847)
        if (e->index == 1 && gTerrain[(int)e->x][(int)e->z].landHidden) continue;

        FlyingObject *fo = &gFlyingObjects[e->ai];
        Color dotColor = { fo->r, fo->g, fo->b, 255 };
        
        float ex = (e->x / (float)SIZE) * (float)mapSize;
        float ez = (e->z / (float)SIZE) * (float)mapSize;
        DrawCircle(mapX + (int)ex, mapY + (int)ez, MINIMAP_DOT_ENTITY, dotColor);
    }
    
    // --- 4. Crosshair ---
    float chSize = HUD_CROSSHAIR_SIZE;
    float cx = sw / 2.0f;
    float cy = sh / 2.0f;
    DrawLine((int)(cx - chSize), (int)cy, (int)(cx + chSize), (int)cy, (Color){255, 255, 255, 150});
    DrawLine((int)cx, (int)(cy - chSize), (int)cx, (int)(cy + chSize), (Color){255, 255, 255, 150});
    
    // --- 5. Screen Fade Overlay ---
    // gFadeStatus: 0.0 = Opaque Black, 1.0 = Transparent
    // mirrors Blitz EntityAlpha HUD\fade, (1.0 - FadeStatus)
    if (gFadeStatus < 1.0f) {
        float alpha = 1.0f - gFadeStatus;
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, alpha));
    }
}
