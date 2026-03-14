#include "score_tag.h"
#include <stddef.h>
#include <math.h>
#include "raylib.h"
#include "world.h"
#include "constants.h"
#include "camera_game.h"

static Texture2D gScoresTexture = {0};
static int gScoreTagNext = 0;

void ScoreTagInit(void) {
    Image img = LoadImage("MiscData/Scores.bmp");
    if (img.data != 0) {
        // Ensure image is in a format that supports alpha
        ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        
        Color *pixels = (Color *)img.data;
        int count = img.width * img.height;
        for (int i = 0; i < count; i++) {
            // Check for black pixels (mirrors Blitz3D mask logic)
            if (pixels[i].r == 0 && pixels[i].g == 0 && pixels[i].b == 0) {
                pixels[i] = BLANK;
            }
        }
        
        gScoresTexture = LoadTextureFromImage(img);
        UnloadImage(img);
    }
    
    // Clear all tags
    for (int i = 0; i <= MAX_SCORE_TAGS; i++) {
        gScoreTags[i].life = 0.0f;
    }
}

void ScoreTagAdd(float x, float y, float z, int frame, int points) {
    if (points != 0) {
        gScore += points;
        
        ScoreTag *st = &gScoreTags[gScoreTagNext];
        st->x = x;
        st->y = y + 1.0f; // Blitz: st\y = y + 1
        st->z = z;
        st->life = 1.0f;
        st->sprite = frame; // Reuse sprite field as frame index

        gScoreTagNext = (gScoreTagNext + 1) % (MAX_SCORE_TAGS + 1);
    }
}

void ScoreTagUpdateAll(float dt) {
    (void)dt;
    for (int i = 0; i <= MAX_SCORE_TAGS; i++) {
        ScoreTag *st = &gScoreTags[i];
        if (st->life > 0.0f) {
            // Blitz 30Hz: life -= 0.01, y += 0.01
            // RayLib 60Hz: life -= 0.005, y += 0.005
            // However, we want to match the "visual speed" of floating.
            // .01 per frame at 30fps is .3 per second.
            // So at 60fps it's .15 per frame? No, .005 per frame.
            st->life -= 0.005f;
            st->y += 0.005f;
            if (st->life < 0) st->life = 0;
        }
    }
}

void ScoreTagDrawAll(void) {
    Ship *pf = &gShips[gCam];
    
    // Blitz3D frames are 64x32.
    const float fw = 64.0f;
    const float fh = 32.0f;
    
    // Calculate how many frames fit in one row of the texture
    int framesPerRow = gScoresTexture.width / (int)fw;
    if (framesPerRow < 1) framesPerRow = 1;

    for (int i = 0; i <= MAX_SCORE_TAGS; i++) {
        ScoreTag *st = &gScoreTags[i];
        if (st->life <= 0.0f) continue;

        // Wrap-aware drawing
        float dx = st->x - pf->x;
        if (dx > (float)SIZE/2.0f) dx -= (float)SIZE;
        if (dx < -(float)SIZE/2.0f) dx += (float)SIZE;
        float dz = st->z - pf->z;
        if (dz > (float)SIZE/2.0f) dz -= (float)SIZE;
        if (dz < -(float)SIZE/2.0f) dz += (float)SIZE;

        // Clip to terrain size
        if (fabsf(dx) >= (float)gGrid.cull + 0.5f || fabsf(dz) >= (float)gGrid.cull + 0.5f) continue;

        Vector3 drawPos = { pf->x + dx, st->y, pf->z + dz };

        // Calculate source rectangle for the specific frame index
        // This handles horizontal, vertical, or grid strips.
        int col = st->sprite % framesPerRow;
        int row = st->sprite / framesPerRow;
        Rectangle srcRec = { (float)col * fw, (float)row * fh, fw, fh };
        
        Vector2 size = { 1.0f, 0.5f }; 
        Color colFade = Fade(WHITE, st->life);
        
        DrawBillboardRec(gCamera3D, gScoresTexture, srcRec, drawPos, size, colFade);
    }
}
