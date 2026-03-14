#pragma once

#include <stdbool.h>

typedef enum GameState {
  STATE_BOOT = 0,
  STATE_MENU,
  STATE_WAVE,
  STATE_WAVE_END,
  STATE_GAME_END,
  STATE_PAUSE,
} GameState;

typedef struct GameFlags {
  bool quit;
  bool gameOver;
  bool waveComplete;
} GameFlags;

typedef struct GameContext {
  GameState state;
  int fps;
  int screenWidth;
  int screenHeight;
  GameFlags flags;

  bool isTestLevel; // True if launched from Test Level menu option

  // Temporary debug info (replace as we port real globals).
  int wave;
  int lives;
  int enemies;

  float timeScale; // 1.0 = normal, 0.5 = half speed
  float gridSize;  // 0.0 to 3.0 view distance
} GameContext;

void GameInit(GameContext *g);
void GameStart(GameContext *g);
void GameUpdate(GameContext *g);
void GameEnd(GameContext *g);

void WaveStart(GameContext *g);
void WaveEnd(GameContext *g);

