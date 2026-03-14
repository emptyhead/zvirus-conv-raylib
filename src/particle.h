#pragma once

#include "world.h"
#include "ship.h"

// Forward declarations of gParticles and gParticleTemplates are in world.h
// via globals? Actually they are in world.c as globals.

void ParticleInit(void);
void ParticleNew(int templateId, float x, float y, float z, float ox, float oy, float oz, int count, 
                 float vx, float vy, float vz, int index, float pitch, float yaw);
void ParticleUpdateAll(void);
void ParticleDrawAll(void);

// Particle templates and max count are in constants.h
extern int gActiveParticles;
