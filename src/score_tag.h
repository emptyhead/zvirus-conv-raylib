#pragma once

#include "world.h"

// Initialize score tag system (loads textures)
void ScoreTagInit(void);

// Add a new floating score tag
// x, y, z: World position
// frame: Index into Scores.bmp (0-14)
// points: Score value to add
void ScoreTagAdd(float x, float y, float z, int frame, int points);

// Update all active score tags (movement, alpha)
void ScoreTagUpdateAll(float dt);

// Draw all active score tags as billboards
void ScoreTagDrawAll(void);
