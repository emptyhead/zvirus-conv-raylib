#include <math.h>
#include <stdlib.h>
#include "particle.h"
#include "constants.h"
#include "util.h"
#include "raylib.h"
#include "terrain.h"
#include "camera_game.h"
#include "world.h"
#include "score_tag.h"

int gActiveParticles = 0;
static int gParticleNext = 0;
static Texture2D gParticleTex;

void CollisionBullet(Particle *p) {
    for (int i = 0; i <= MAX_SHIPS; i++) {
        Ship *c = &gShips[i];
        if (c->dead == 0 && p->index != c->index) {
            float dx = p->x - c->x;
            if (dx > (float)SIZE / 2.0f) dx -= (float)SIZE;
            if (dx < -(float)SIZE / 2.0f) dx += (float)SIZE;
            float dz = p->z - c->z;
            if (dz > (float)SIZE / 2.0f) dz -= (float)SIZE;
            if (dz < -(float)SIZE / 2.0f) dz += (float)SIZE;
            float dy = p->y - c->y;
            float d = sqrtf(dx*dx + dy*dy + dz*dz) + 0.001f;

            float radius = gFlyingObjects[c->ai].radius;
            if (d < radius) {
                p->life = -1.0f;
                c->fuel -= gFlyingObjects[c->ai].damage;
                if (c->inView) {
                    ParticleNew(14, p->x, p->y, p->z, 0, 0, 0, 3, 0, 0, 0, p->index, 0.0f, 0.0f);
                }
                if (c->fuel <= 0) {
                    c->dead = 1;
                    if (p->index == 0) {
                        FlyingObject *F = &gFlyingObjects[c->ai];
                        ScoreTagAdd(c->x, c->y, c->z, F->tag, F->points);
                    }
                }
            }
        }
    }
}

void ParticleInit(void) {
    static const ParticleTemplate table[] = {
        {5, 0.75f, 250,250, 50,250,  0, 50, 1.0f, 2.0f, -0.075f, 0.075f, -0.3f, -0.2f, -0.075f, 0.075f, 1.0f, P_FADE_NORM}, // 0 Thrust
        {10, 1.0f, 255,255, 255,255, 255,255, 2.0f, 2.0f, 0,0, 0,0, 0.5f, 0.5f, 0.0f, P_FADE_NORM}, // 1 Bullet
        {5, 0.75f, 50,100, 50,100, 50,100, 1.0f, 2.0f, -0.075f, 0.075f, -0.075f, 0.075f, -0.15f, -0.05f, 0.5f, P_FADE_NORM}, // 2 Missile Trail
        {0, 0.75f, 50,100, 50,100, 50,100, 1.0f, 2.0f, -0.025f, 0.025f, 0.04f, 0.08f, -0.025f, 0.025f, 0.0f, P_FADE_NORM*0.2f}, // 3 Ground Object Smoke
        {0, 0.75f, 50,50, 125,250, 250,250, 1.0f, 2.0f, -0.07f, 0.07f, 0.1f, 0.125f, -0.07f, 0.07f, 0.5f, P_FADE_NORM}, // 4 Water Hit
        {0, 0.75f, 125,250, 125,250, 75,150, 1.0f, 2.0f, -0.06f, 0.06f, 0.075f, 0.1f, -0.06f, 0.06f, 0.5f, P_FADE_NORM}, // 5 Beach hit
        {0, 0.75f, 75,150, 125,250, 75,150, 1.0f, 2.0f, -0.07f, 0.07f, 0.075f, 0.1f, -0.07f, 0.07f, 0.5f, P_FADE_NORM}, // 6 Ground Hit
        {0, 0.75f, 125,250, 125,250, 125,250, 1.0f, 2.0f, -0.08f, 0.08f, 0.05f, 0.075f, -0.08f, 0.08f, 0.5f, P_FADE_NORM}, // 7 LandingPad Hit
        {0, 0.75f, 250,250, 0,250, 250,250, 1.0f, 2.0f, -0.08f, 0.08f, 0.05f, 0.075f, -0.08f, 0.08f, 0.5f, P_FADE_NORM}, // 8 AlienBuildings Hit
        {3, 0.75f, 25,25, 100,200, 200,200, 1.0f, 2.0f, -0.025f, 0.025f, -0.4f, -0.4f, -0.025f, 0.025f, 0.5f, P_FADE_SLOW}, // 9 rain
        {0, 0.75f, 250,250, 50,100, 0, 50, 1.5f, 3.0f, -0.15f, 0.15f, 0.1f, 0.2f, -0.15f, 0.15f, 2.0f, P_FADE_SLOW}, // 10 tree infection
        {0, 0.75f, 250,250, 50,100, 0, 50, 1.5f, 3.0f, -0.15f, 0.15f, 0.1f, 0.2f, -0.15f, 0.15f, 2.0f, P_FADE_SLOW}, // 11 infected
        {10, 1.0f, 255,255, 255,255, 255,255, 3.0f, 3.0f, 0,0, -0.2f, -0.2f, 0,0, 0.0f, P_FADE_SLOW}, // 12 bombs
        {5, 0.75f, 250,250, 0,250, 0,250, 1.5f, 3.0f, -0.15f, 0.15f, 0.25f, 0.4f, -0.15f, 0.15f, 4.0f, P_FADE_XSLOW}, // 13 explosion
        {0, 0.75f, 75,125, 75,125, 75,125, 1.5f, 3.0f, -0.1f, 0.1f, 0, 0.1f, -0.1f, 0.1f, 0.0f, P_FADE_SLOW}, // 14 spark
        {0, 0.75f, 250,250, 0,250, 250,250, 1.0f, 2.5f, -0.075f, 0.075f, -0.075f, 0.075f, -0.05f, -0.01f, 0.0f, P_FADE_NORM*2.0f}, // 15 tractor
        {0, 2.0f, 100,200, 100,200, 100,200, 2.0f, 2.0f, 0,0, 0,0, 0,0, 0.0f, P_FADE_NORM}, // 16 star
        {0, 2.0f, 150,250, 50,150, 150,250, 2.0f, 2.0f, 0,0, 0,0, 0,0, 0.0f, P_FADE_NORM}, // 17 too high
        {0, 0.75f, 250,250, 0,250, 250,250, 2.0f, 4.0f, -0.25f, 0.25f, -0.25f, 0.25f, -0.5f, -0.3f, 0.0f, P_FADE_NORM*2.0f}, // 18 cruiser thrust
        {0, 1.0f, 128,250, 128,250, 128,250, 1.5f, 1.5f, -0.1f, 0.1f, 0.3f, 0.5f, -0.1f, 0.1f, 3.0f, P_FADE_NORM*2.0f}, // 19 firework
        {3, 0.75f, 250,250, 0,250, 0,250, 1.5f, 3.0f, -0.1f, 0.1f, -0.1f, 0.1f, -0.1f, 0.1f, 1.0f, P_FADE_NORM}, // 20 firework explode
        {0, 0.75f, 0, 250, 250,250, 0, 250, 2.0f, 4.0f, -0.01f, 0.01f, -0.01f, 0.01f, 0.2f, 0.2f, 0.0f, P_FADE_NORM*2.0f}, // 21 radar
    };

    for (int i = 0; i <= PARTICLE_TEMPLATES; i++) {
        gParticleTemplates[i] = table[i];
    }

    for (int i = 0; i <= MAX_PARTICLES; i++) {
        gParticles[i].life = -1.0f;
    }

    // Generate a solid white square texture for particles (matches original square look)
    Image img = GenImageColor(8, 8, WHITE);
    gParticleTex = LoadTextureFromImage(img);
    UnloadImage(img);
}

void ParticleNew(int templateId, float x, float y, float z, float ox, float oy, float oz, int count, 
                 float vx, float vy, float vz, int index, float pitch, float yaw) {
    if (templateId < 0 || templateId > PARTICLE_TEMPLATES) return;
    ParticleTemplate *pt = &gParticleTemplates[templateId];

    for (int i = 0; i < count; i++) {
        Particle *p = &gParticles[gParticleNext];
        
        // Transform offset to world space
        float tx, ty, tz;
        TFormVector(ox, oy, oz, pitch, yaw, &tx, &ty, &tz);
        
        // Backtrack position by one frame's displacement to eliminate double-movement drift
        // Ship moves by (vx, vy, -vz) * 0.5 each frame. 
        p->x = x + tx - vx * 0.5f;
        p->y = y + ty - vy * 0.5f;
        p->z = z - tz + vz * 0.5f; 

        // Generate random velocity within template range and transform to world space
        float rvx = pt->vx1 + ((float)rand()/(float)RAND_MAX * (pt->vx2 - pt->vx1));
        float rvy = pt->vy1 + ((float)rand()/(float)RAND_MAX * (pt->vy2 - pt->vy1));
        float rvz = pt->vz1 + ((float)rand()/(float)RAND_MAX * (pt->vz2 - pt->vz1));
        
        float tvx, tvy, tvz;
        TFormVector(rvx, rvy, rvz, pitch, yaw, &tvx, &tvy, &tvz);
        
        p->vx = vx + tvx;
        p->vy = vy + tvy;
        p->vz = vz + tvz;

        p->r = (float)(pt->r1 + (rand() % (pt->r2 - pt->r1 + 1))) / 255.0f;
        p->g = (float)(pt->g1 + (rand() % (pt->g2 - pt->g1 + 1))) / 255.0f;
        p->b = (float)(pt->b1 + (rand() % (pt->b2 - pt->b1 + 1))) / 255.0f;
        p->size = pt->s1 + (float)rand()/(float)RAND_MAX * (pt->s2 - pt->s1);
        p->life = 1.0f;
        p->index = index;
        p->id = templateId;
        gParticleNext = (gParticleNext + 1) % (MAX_PARTICLES + 1);
    }
}

void ParticleUpdateAll(void) {
    gActiveParticles = 0;
    float gravity = gGravity;

    for (int i = 0; i <= MAX_PARTICLES; i++) {
        Particle *p = &gParticles[i];
        if (p->life < 0) continue;

        gActiveParticles++;
        ParticleTemplate *pt = &gParticleTemplates[p->id];

        p->life = clampf(p->life - pt->fade, 0.0f, 1.0f);
        p->vy -= gravity * pt->weight;
        p->x = wrapf(p->x + p->vx * 0.5f, (float)SIZE, 0.0f);
        p->y += p->vy * 0.5f;
        p->z = wrapf(p->z - p->vz * 0.5f, (float)SIZE, 0.0f); // Source.bb Z-alignment

        if (p->id == 1 || p->id == 12 || p->id == 18) {
            CollisionBullet(p);
        }

        float gh = TerrainGetHeight(p->x, p->z, 1) + (p->size * OSCALE);
        if (p->y < gh) {
            // Only Bullets (id 1) trigger ground item destruction (Source.bb 2313)
            if (p->id == 1) {
                TerrainCollisionGround(p->x, p->y, p->z, 1, 0, p->index);
            }
            
            // Infection spread when spores hit the ground (Source.bb 2314)
            if (p->id == 10 || p->id == 11) {
                TerrainMapAdd(0, p->x, p->z, p->id - 9);
            }

            p->y = gh;
            p->vx *= 0.5f;
            p->vy = -p->vy * 0.5f;
            p->vz *= 0.5f;

            if (pt->splash > 0) {
                p->life = -1.0f;
                int lx = wrapi((int)floorf(p->x), SIZE, 0);
                int lz = wrapi((int)floorf(p->z), SIZE, 0);
                int splashId = 4 + gTerrain[lx][lz].landIndex;
                if (p->id == 12) splashId = 11;
                ParticleNew(splashId, p->x, p->y, p->z, 0, 0, 0, (int)pt->splash, 0, 0, 0, p->index, 0.0f, 0.0f);
            }
        }

        if (p->life <= 0) p->life = -1.0f;
    }
}

void ParticleDrawAll(void) {
    Ship *pf = &gShips[gCam];
    for (int i = 0; i <= MAX_PARTICLES; i++) {
        Particle *p = &gParticles[i];
        if (p->life < 0) continue;

        // Wrap-aware drawing relative to focused player
        float dx = p->x - pf->x;
        if (dx > (float)SIZE/2.0f) dx -= (float)SIZE;
        if (dx < -(float)SIZE/2.0f) dx += (float)SIZE;
        float dz = p->z - pf->z;
        if (dz > (float)SIZE/2.0f) dz -= (float)SIZE;
        if (dz < -(float)SIZE/2.0f) dz += (float)SIZE;

        // Clip to terrain size
        if (fabsf(dx) >= (float)gGrid.cull + 0.5f || fabsf(dz) >= (float)gGrid.cull + 0.5f) continue;

        Vector3 drawPos = { pf->x + dx, p->y, pf->z + dz };

        ParticleTemplate *pt = &gParticleTemplates[p->id];
        float alpha = 1.0f;
        int alphaMode = (int)pt->alpha;
        switch (alphaMode) {
            case 0: alpha = pt->alpha * p->life; break;
            case 1: alpha = 1.0; break;
            case 2: alpha = 1.0f - fabsf(p->life - 0.5f) * 2.0f; break;
            default: alpha = pt->alpha * p->life; break;
        }

        Color col = { (unsigned char)(p->r * 255), (unsigned char)(p->g * 255), (unsigned char)(p->b * 255), (unsigned char)(alpha * 255) };
        
        // Draw billboarded particle
        DrawBillboard(gCamera3D, gParticleTex, drawPos, p->size * OSCALE, col);
    }
}
