#pragma once

#include "world.h"

// Called once at game start: inits FlyingObjects and resets all ship slots.
void ShipInitAll(void);

// Reset a ship slot to a live entity of the given AI type.
// Mirrors Blitz PLAYERreset(ID, AI, dead=0).
void ShipReset(int id, int aiType, int dead);

// Squared-distance with wrapping + sets gNX/NY/NZ normals.
float ShipDistance(Ship *s1, Ship *s2);

// Find closest target of given index types within range (returns slot id or -1).
int ShipFindTarget(Ship *s, int index1, int index2, float range);

// Steer ship s toward target t; sets gJX, gJZ.
void ShipChase(Ship *s, Ship *target);

// Generic transformation: local vector (lx,ly,lz) to world space.
void TFormVector(float lx, float ly, float lz, float pitchDeg, float yawDeg,
                        float *ox, float *oy, float *oz);

void TFormForward(float pitchDeg, float yawDeg, int engine,
                         float *ox, float *oy, float *oz);

// Run AI decision for one ship.
void ShipAI(Ship *s, float dt);

// Full update pass covering all active ships.
void ShipUpdateAll(float dt);

// Called from main.c STATE_WAVE — delegates to ShipUpdateAll.
void ShipUpdateSimpleControls(float dt);

// Action globals (mirrors Blitz JX, JZ, Thrust, Shoot ... globals)
extern float gJX, gJZ, gDP, gDY;
extern float gThrust, gShoot, gLaunch;
extern float gInfect, gBomb, gAttract;
extern float gMouseX, gMouseY;
