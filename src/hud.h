#pragma once

#include "world.h"
#include "game.h"

void HudInit(void);
void HudUpdate(float dt);
void HudDraw(const GameContext *g);
void HudUpdateMap(void);

// Helper for map dots
void HudAddMapDot(float x, float z, int type);
void HudClearMap(void);
void HudFade(GameContext *g, float target, float speed);
