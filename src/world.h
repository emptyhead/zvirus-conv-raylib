#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "constants.h"
#include <raylib.h>

typedef struct SoundGroup {
  int sound;
  int other;
  int channel;
  float distance;
  float volume;
  float pan;     // OUR convention: 0.0=left, 0.5=centre, 1.0=right (audio.c inverts for Raylib which uses 0.0=right)
  int id;
  int index;
} SoundGroup;

typedef struct Menu {
  int model;
  int texture;
  int id;
  int y_scale;
  int y_spacing;
  int x_spacing;
  int min_option;
  int max_option;
  int current;

  char name[10][64];
  int sub_min[10];
  int sub_max[10];
  int sub_current[10];
  char sub_name[10][128];
} Menu;

typedef struct ScoreTag {
  int sprite;
  float x, y, z;
  float life;
} ScoreTag;

typedef struct ParticleTemplate {
  int splash;
  float alpha;
  int r1, r2, g1, g2, b1, b2;
  float s1, s2;
  float vx1, vx2, vy1, vy2, vz1, vz2;
  float weight;
  float fade;
} ParticleTemplate;

typedef struct Particle {
  int id;       // Template ID
  int index;    // Faction index
  float x, y, z;
  float vx, vy, vz;
  float r, g, b;
  float size;
  float life;   // 1.0 down to 0.0
  int vertex;   // Base vertex index in mesh
} Particle;

typedef struct GroundObject {
  Model meshState[4];  // [0]=normal, [1]=infected, [2]=destroyed, [3]=destroyed_infected
  float height;
  const char* name;
  Model meshExtra[4];  // Composite parts for all 4 states
} GroundObject;

typedef struct FlyingObject {
  int model;
  float radius;
  char name[64];
  Model mesh;
  int points;
  int tag;
  float thrust;
  float momentum;
  int fireRate;
  int missileRate;
  int thrustRate;
  int damage;
  float spinSpeed;
  int engine;
  float pitchMin;
  float pitchMax;
  float turnSpeed;
  int range;
  int deadTimer;
  uint8_t r, g, b;
  float cruiseHeight;
  int soundGroup;
} FlyingObject;

typedef struct Grid {
  int divs;
  int divg;
  int view;
  int cull;
  int model;
  int s;
} Grid;

typedef struct Terrain {
  int landIndex;
  float landHeight;
  int objectIndex;
  float objectHeight;
  int objectStatus;
  int objectCycle;
  int objectYaw;
  float objectPitch;
  float objectSway;

  uint8_t r[2], g[2], b[2];
  uint32_t argb[2];

  int landInfected;
  int landHidden;
  float waveFactor;
} Terrain;

typedef struct Hud {
  int camera;
  int model;
  int pivot;
  int display;
  int map;
  int fuel;
  int height;
  int bonus;
  int title;
  int dot;
  int arrow;
  int crossHair;
  float onTarget;
  int fade;
} Hud;

typedef struct Ship {
  int model;
  int dot;
  int ai;
  int aiCounter;
  float x, y, z;
  float vx, vy, vz;
  int thrustCounter;
  int reload;
  float pitch;
  float yaw;
  float spin;
  float fuel;
  int inView;
  int dead;
  int target;
  int index;
  int id;
  int child[3];
  int counter;
  int spread;
  int gun;
  int chaseTimer;

  // Frame-persistent intent flags (replaces globals for AI)
  float thrustIntent;
  float shootIntent;
  float launchIntent;
  float infectIntent;
  float bombIntent;
  float attractIntent;
  float jxIntent;
  float jzIntent;
} Ship;

typedef struct WaveData {
  char mapName[64];
  int unitCount[19]; // Matches the 19 columns in Source.bb .WaveData
} WaveData;

extern float gGravity;
extern Grid gGrid;
extern Hud gHud;
extern SoundGroup gSoundGroups[SOUND_GROUPS + 1];
extern Terrain gTerrain[SIZE][SIZE];
extern GroundObject gGroundObjects[GROUND_OBJECTS + 1];
extern FlyingObject gFlyingObjects[FLYING_OBJECTS + 1];
extern Ship gShips[MAX_SHIPS + 1];
extern Particle gParticles[MAX_PARTICLES + 1];
extern ParticleTemplate gParticleTemplates[PARTICLE_TEMPLATES + 1];
extern ScoreTag gScoreTags[MAX_SCORE_TAGS + 1];
extern Menu gMenus[MENU_COUNT + 1];

extern float gDistanceLUP[MAX_SHIPS + 1][MAX_SHIPS + 1];
extern int   gMaxShips;
extern float gNX, gNY, gNZ;
extern int   gEnemies;
extern int   gScore;
extern float gFadeStatus;
extern int   gAreaTotal, gAreaInfected;
extern float gRipple;
extern float gRotate;

extern WaveData gWaveData[MAX_WAVES];

void FlyingObjectInit(void);
void WaveDataInit(void);
