// player.c  —  Phase 3 port of PLAYERupdate, PLAYERreset, collision sweeps
// Logic mirrors Source.bb as closely as possible.

#include "ship.h"

#include <math.h>
#include <stddef.h>

#include "camera_game.h"  // gCam
#include "constants.h"
#include "terrain.h"
#include "util.h"
#include "world.h"
#include "particle.h"
#include "score_tag.h"
#include "raylib.h"
#include "audio.h"

// Computes the visual "Nose" direction of the ship.
void TFormForward(float pitchDeg, float yawDeg, int engine,
                         float *ox, float *oy, float *oz) {
    if (engine == 1) {
        // Hoverers always face strictly horizontally
        TFormVector(0, 0, 1, 0, yawDeg, ox, oy, oz);
    } else {
        TFormVector(0, 0, 1, pitchDeg, yawDeg, ox, oy, oz);
    }
}

// ---------------------------------------------------------------------------
// PLAYERdistance — port of the Blitz function.
// Sets gNX/NY/NZ normals used by COLLISIONplayer.
// Returns squared distance (matching Blitz which stores d before sqrt).
// ---------------------------------------------------------------------------
float ShipDistance(Ship *p, Ship *t) {
    if (!p || !t) return 999999.0f;
    float dx = p->x - t->x;
    if (fabsf(dx) > 128.0f) dx = ((float)SIZE - fabsf(dx)) * (dx < 0 ? 1.0f : -1.0f);
    float dz = p->z - t->z;
    if (fabsf(dz) > 128.0f) dz = ((float)SIZE - fabsf(dz)) * (dz < 0 ? 1.0f : -1.0f);
    float dy = p->y - t->y;
    float d  = (dx*dx + dy*dy + dz*dz) + 0.01f;
    float rd = sqrtf(d);
    gNX = -dx / rd;
    gNY = -dy / rd;
    gNZ = -dz / rd; // Normal points from target to player, Z negated for Raylib
    return d;
}

// ---------------------------------------------------------------------------
// FlyingObjectInit caller (called from GameInit)
// ---------------------------------------------------------------------------
void ShipInitAll(void) {
    FlyingObjectInit();

    // Reset all player slots to dead=-1. Mirror Blitz init.
    for (int i = 0; i <= MAX_SHIPS; i++) {
        gShips[i] = (Ship){0};
        gShips[i].dead = -1;
        gShips[i].target = -1;
        gShips[i].id = i;
    }

    // Slot 0 = the player craft (Hoverplane)
    ShipReset(0, 0, 0);
}

// ---------------------------------------------------------------------------
// PLAYERreset  (simplified — no 3D entity creation yet)
// ---------------------------------------------------------------------------
void ShipReset(int id, int aiType, int dead) {
    if (id < 0 || id > MAX_SHIPS) return;

    Ship *p = &gShips[id];
    FlyingObject *F = &gFlyingObjects[aiType];

    p->id   = id;
    p->ai   = aiType;
    p->dead = dead;

    // p\index = Sgn(F\points) + (AI=20)*2 + (AI=17)*2
    int pts = F->points;
    p->index = (pts > 0 ? 1 : (pts < 0 ? -1 : 0))
               + (aiType == 20 ? 2 : 0)
               + (aiType == 17 ? 2 : 0);

    p->spread = 0;
    p->yaw    = (float)GetRandomValue(0,7) * 45.0f
                + (float)(GetRandomValue(0,1)*2 - 1) * (float)GetRandomValue(5,20);
    p->pitch  = 0.0f; // Helicopter starting pitch (Hover/Level)
    p->spin   = 0.0f;
    p->vx = p->vy = p->vz = 0.0f;
    p->fuel = 500.0f + (aiType > 10 && aiType < 15 ? 500.0f : 0.0f)
              + (aiType == 15 ? 1500.0f : 0.0f);
    p->target     = -1;
    p->chaseTimer = GetRandomValue(0, 10 * FPS);
    p->jxIntent   = 0.0f;
    p->jzIntent   = 0.0f;
    p->aiCounter  = 0;
    p->thrustIntent = 0.0f;
    p->shootIntent = 0.0f;

    // Spawn position: Monsters (AI 17) only spawn in water (LandHeight == 0)
    int ok = 0;
    int safety = 0;
    do {
        int r = GetRandomValue(64, 120);
        int a = GetRandomValue(0, 360);
        p->x = (float)SIZE * 0.5f - 0.5f + (p->index != 0 ? (float)r * cosf((float)a * DEG2RAD) : 0.0f);
        p->z = (float)SIZE * 0.5f        + (p->index != 0 ? (float)r * sinf((float)a * DEG2RAD) : 0.0f);
        
        // p\y = TERRAINgetheight(x,z,1) + .5*(index=0) + 50*(index=1)
        // Note: For AI 17 (Monster), LandHeight must be 0 (water)
        float landH = TerrainGetHeight(p->x, p->z, 1);
        p->y = landH + (p->index == 0 ? 0.5f : 0.0f) + (p->index == 1 ? 50.0f : 0.0f);
        
        if (aiType != 17 || landH <= 0.05f) { // Allowing a small epsilon for water
            ok = 1;
        }
        
        safety++;
        if (safety > 1000) {
            ok = 1; // Safety break
        }
    } while (!ok);

    p->inView = 1;
}

// ---------------------------------------------------------------------------
// PLAYERposition — compute local offset from cam player, set inView
// ---------------------------------------------------------------------------
static void ShipPosition(Ship *c) {
    Ship *cam = &gShips[gCam];
    float lx = c->x - cam->x;
    if (fabsf(lx) > (float)SIZE/2.0f) lx = ((float)SIZE - fabsf(lx)) * (lx < 0 ? 1.0f : -1.0f);
    float lz = c->z - cam->z;
    if (fabsf(lz) > (float)SIZE/2.0f) lz = ((float)SIZE - fabsf(lz)) * (lz < 0 ? 1.0f : -1.0f);
    float view = (float)(gGrid.divs * 2);
    c->inView = (fabsf(lx) < view && fabsf(lz) < view) ? 1 : 0;

    // --- Sound Group Update ---
    if (c->inView) {
        FlyingObject *F = &gFlyingObjects[c->ai];
        int sgIdx = F->soundGroup;
        SoundGroup *sg = &gSoundGroups[sgIdx];
        float d = sqrtf(lx*lx + lz*lz + (c->y - cam->y)*(c->y - cam->y));
        if (d < sg->distance) {
            sg->distance = d;
            float vol = 1.0f - (d / SND_FALLOFF_DIST);
            if (vol < 0.0f) vol = 0.0f;
            
            // Group specific base volume
            static const float groupBaseVols[] = {
                SND_VOL_GROUP_0, SND_VOL_GROUP_1, SND_VOL_GROUP_2, SND_VOL_GROUP_3, SND_VOL_GROUP_4,
                SND_VOL_GROUP_5, SND_VOL_GROUP_6, SND_VOL_GROUP_7, SND_VOL_GROUP_8, SND_VOL_GROUP_9
            };
            vol *= groupBaseVols[sgIdx];

            if (sgIdx == 0) {
                if (c->index == gCam) {
                    float thrustFactor = (F->thrust > 0.000001f) ? (c->thrustIntent / F->thrust) : 0.0f;
                    vol *= (SND_VOL_PLAYER_THRUST_MIN + (SND_VOL_PLAYER_THRUST_MAX - SND_VOL_PLAYER_THRUST_MIN) * thrustFactor);
                }
            }
            if (vol > sg->volume) {
                sg->volume = vol * SND_VOL_MASTER;
                if (c->index == gCam) {
                    sg->pan = 0.5f; // Centre player engine
                } else {
                    float bearing = atan2f(lx, -lz); // Ship forward=-Z in world (Z-flipped from Blitz LH coords)
                    float camYawRad = cam->yaw * (3.14159265f / 180.0f);
                    float relAngle = bearing - camYawRad;
                    sg->pan = 0.5f + sinf(relAngle) * 0.45f;
                    if (sg->pan < 0.05f) sg->pan = 0.05f;
                    if (sg->pan > 0.95f) sg->pan = 0.95f;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// PLAYERfindtarget
// ---------------------------------------------------------------------------
int ShipFindTarget(Ship *p, int index1, int index2, float range) {
    if (!p) return -1;
    float rangeSq = range * range;
    p->target = -1;
    for (int i = 0; i <= gMaxShips; i++) {
        Ship *t = &gShips[i];
        if (t->dead != 0) continue;
        if (t->index == index1 || t->index == index2) {
            float d = ShipDistance(p, t);
            if (d < rangeSq) { rangeSq = d; p->target = t->id; }
        }
    }
    if (p->target > -1) p->chaseTimer = 0;
    return p->target;
}

// ---------------------------------------------------------------------------
// PLAYERchase  — mirrors DeltaYaw/DeltaPitch → JX/JZ
// ---------------------------------------------------------------------------
void ShipChase(Ship *p, Ship *t) {
    if (!p || !t) return;

    float lx = p->x - t->x;
    if (fabsf(lx) > 128.0f) lx = ((float)SIZE - fabsf(lx)) * (lx < 0 ? 1.0f : -1.0f);
    float lz = p->z - t->z;
    if (fabsf(lz) > 128.0f) lz = ((float)SIZE - fabsf(lz)) * (lz < 0 ? 1.0f : -1.0f);

    // Direction from p to t in local/world space
    float dx = -lx, dz = -lz, dy = t->y - p->y;
    float flatDist = sqrtf(dx*dx + dz*dz) + 0.001f;

    // DeltaYaw: angle from p's facing to target bearing
    // Forward in Raylib is -Z, so we negate dz for atan2 alignment (Target at -Z = 0 deg)
    float targetYaw   = atan2f(dx, -dz) * RAD2DEG;
    float dy_yaw      = targetYaw - p->yaw;
    while (dy_yaw >  180.0f) dy_yaw -= 360.0f;
    while (dy_yaw < -180.0f) dy_yaw += 360.0f;

    // DeltaPitch (Target pitch depends on engine system)
    // Target above player -> dy is positive -> targetPitch is negative (Up)
    float targetPitch = -atan2f(dy, flatDist) * RAD2DEG;
    float dp = targetPitch - p->pitch;

    // JX = Sgn(DeltaYaw), JZ = -Sgn(DeltaPitch)
    // CAPTION: In our Raylib system, increasing yaw turns RIGHT. 
    // In Blitz3D, decreasing yaw turns RIGHT. 
    // Since we use Source.bb's (yaw - jx*turn) we must swap jx's sign.
    extern float gJX, gJZ, gDP, gDY;
    gJX = (dy_yaw > 0.5f ? -1.0f : (dy_yaw < -0.5f ? 1.0f : 0.0f));
    gJZ = -(dp   > 0.5f ? 1.0f : (dp    < -0.5f ? -1.0f : 0.0f));
    gDY = dy_yaw;
    gDP = dp;

    p->chaseTimer += 1;
    if (t->dead != 0 || p->chaseTimer > (FPS * 10)) p->target = -1;
}

// ---------------------------------------------------------------------------
// PLAYERai  — engine-type dispatch matching Source.bb
// ---------------------------------------------------------------------------
void ShipAI(Ship *p, float dt) {
    (void)dt;
    if (!p || p->dead != 0 || p->ai == 0) return;

    FlyingObject *F = &gFlyingObjects[p->ai];
    
    // Reset combat intents (Movement intents persist for wandering/chase)
    p->thrustIntent = (float)F->thrust;
    p->shootIntent  = 0.0f;
    p->launchIntent = 0.0f;
    p->infectIntent = (float)(p->ai == 1);
    p->bombIntent   = (float)(p->ai == 2);
    p->attractIntent = (float)(p->ai == 8);

    float groundHeight = TerrainGetHeight(p->x, p->z, 0);
    float checkHeight  = groundHeight + (float)F->cruiseHeight;

    switch (F->engine) {
        case 0: {  // Standard flyer (Hoverplane, Drone, Fighters, etc.)
            float attackHeight = 5.0f; // Sourced from PLAYERai argument (5)
            float maxPitch = 0.0f;
            
            if (p->target > -1) {
                float rangeSq = (float)(F->range * F->range);
                float d = ShipDistance(p, &gShips[p->target]);
                ShipChase(p, &gShips[p->target]); 
                p->jxIntent = gJX; 
                p->jzIntent = gJZ;

                if (p->target > -1) {
                    if (fabsf(gDY) > 30.0f && p->index == 0) p->thrustIntent = 0.0f; // Source 964
                    p->shootIntent = (d < 225.0f && fabsf(gDP) < 90.0f) ? 1.0f : 0.0f;
                    if (d < rangeSq) checkHeight = gShips[p->target].y + attackHeight;
                    // Drone pitch: 10 + (45 + 45*(index=0)) * d / range
                    maxPitch = 10.0f + (45.0f + 45.0f * (p->index == 0 ? 1.0f : 0.0f)) * (d / rangeSq);
                    
                    if (gShips[p->target].dead != 0 || d > rangeSq) p->target = -1;
                    
                    // Boss logic (AI 16)
                    if (p->ai == 16 && d < 400.0f) p->launchIntent = 1.0f;
                }
            } else {
                // Wandering: Change direction based on decision timer (Flyer)
                if (p->aiCounter <= 0) {
                    p->jxIntent = (float)GetRandomValue(-25, 25) * 0.01f;
                    p->aiCounter = GetRandomValue(G_AI_WANDER_FLYER_MIN, G_AI_WANDER_FLYER_MAX);
                }
                maxPitch = (float)GetRandomValue(10, 45); 

                // Find opponent target (if p\index=1, find index 0)
                int opp = (p->index == 0) ? 1 : 0;
                ShipFindTarget(p, opp, opp, (float)F->range);
            }

            // Altitude control (Source.bb 980-990)
            if (p->shootIntent == 0.0f) {
                // targetPitch: Positive is DOWN/FORWARD for helicopter (0 is UP).
                float targetPitch = maxPitch;
                p->jzIntent = (targetPitch > p->pitch ? -1.0f : (targetPitch < p->pitch ? 1.0f : 0.0f));
            }
            
            if (p->y > (checkHeight + attackHeight * 0.5f)) p->thrustIntent = 0.0f; 
            
            if (p->y < checkHeight || p->vy < -0.25f) {
                if (p->y < MAX_HEIGHT) {
                    float climbAngle = (float)GetRandomValue(10, 45);
                    float targetPitch = climbAngle;
                    p->jzIntent = (targetPitch > p->pitch ? -1.0f : (targetPitch < p->pitch ? 1.0f : 0.0f));
                    p->thrustIntent = (float)F->thrust;
                }
            }
            
            float clear = p->y - groundHeight;
            if (clear <= G_AVOIDANCE_ALT) { // G_AVOIDANCE_ALT = 10
                float climbAngle = (float)GetRandomValue(5, (int)(4.5f * clear));
                float targetPitch = climbAngle;
                p->jzIntent = (targetPitch > p->pitch ? -1.0f : (targetPitch < p->pitch ? 1.0f : 0.0f));
                p->thrustIntent = (float)F->thrust;
            }
        } break;

        case 1:  // Hoverer (Seeder, Bomber, Attractor …)
        case 3:  // Cruiser
            p->y += (checkHeight - p->y) * 0.0125f; // Scaled for 60Hz (original 0.025)
            // Hoverer wandering jitter
            if (p->aiCounter <= 0) {
                p->jxIntent = (float)GetRandomValue(-25, 25) * 0.01f;
                p->aiCounter = GetRandomValue(G_AI_WANDER_HOVERER_MIN, G_AI_WANDER_HOVERER_MAX);
            }
            p->jzIntent = 0.0f;
            // Generators / Mystery / Cruiser
            if (p->ai > 9 && p->ai < 16) {
                ShipFindTarget(p, 0, 2, (float)F->range);
                if (p->target > -1) p->launchIntent = 1.0f;
            }
            break;

        case 2:  // Missile / Spore / Pest
            p->jxIntent = p->jzIntent = 0.0f;
            // Missile
            if (p->ai == 20) {
                p->thrustIntent *= (1.0f + (p->fuel == 500.0f ? 10.0f : 0.0f));
                p->fuel -= 1.0f;
                if (p->fuel < 0.0f) p->dead = 1;
                if (p->fuel < 490.0f) {
                    if (p->target > -1) ShipChase(p, &gShips[p->target]);
                    else                ShipFindTarget(p, 1, 1, 1000.0f);
                    p->jxIntent = gJX;
                    p->jzIntent = gJZ;
                }
            }
            // Pest / Spore
            if (p->ai == 3 || p->ai == 19) {
                ShipFindTarget(p, 0, 0, 1000.0f);
                if (p->target > -1) {
                    ShipChase(p, &gShips[p->target]);
                    p->jxIntent = gJX;
                    p->jzIntent = gJZ;
                } else {
                    if (p->aiCounter <= 0) {
                        p->jxIntent = (float)GetRandomValue(-25, 25) * 0.01f;
                        p->aiCounter = GetRandomValue(G_AI_WANDER_SEEKER_MIN, G_AI_WANDER_SEEKER_MAX);
                    }
                }
                if (p->y < checkHeight) p->jzIntent =  2.0f;
                if (p->y > MAX_HEIGHT)  p->jzIntent = -2.0f;
            }
            break;
    }
}

// ---------------------------------------------------------------------------
// PLAYERlaunch — Port of Source.bb function (spawns missile/child)
// ---------------------------------------------------------------------------
void ShipLaunch(Ship *p) {
    int found = -1;
    for (int m = 0; m < 3; m++) {
        int check = p->child[m];
        if (check >= 0 && gShips[check].dead == -1) {
            found = check;
            break;
        }
    }

    if (found > -1) {
        Ship *L = &gShips[found];
        FlyingObject *pF = &gFlyingObjects[p->ai];
        FlyingObject *lF = &gFlyingObjects[L->ai];

        L->fuel = 500.0f;
        float offset = pF->radius + lF->radius * 0.25f;
        
        // TFormNormal 0, .1, 1, p\model, 0 equivalent
        // Forward/Up vector from pitch/yaw
        float fx, fy, fz;
        TFormForward(p->pitch, p->yaw, 2, &fx, &fy, &fz); // use engine=2 for Z-forward offset

        L->vx = fx;
        L->vy = fy;
        L->vz = fz;
        L->x = p->x + fx * offset;
        L->y = p->y + fy * offset;
        L->z = p->z - fz * offset;
        L->yaw = p->yaw + p->spin;
        L->pitch = p->pitch;
        
        L->dead = 0;
        L->counter = lF->missileRate;
        p->counter = pF->missileRate;
        
        ShipPosition(L);
    }
}

// ---------------------------------------------------------------------------
// COLLISIONplayer  — bounce / damage between entities
// ---------------------------------------------------------------------------
static void CollisionShip(void) {
    for (int k = 0; k < gMaxShips; k++) {
        for (int m = k + 1; m <= gMaxShips; m++) {
            Ship *pp = &gShips[k];
            Ship *cp = &gShips[m];
            if (pp->dead != 0 || cp->dead != 0) continue;

            FlyingObject *Fp = &gFlyingObjects[pp->ai];
            FlyingObject *Fc = &gFlyingObjects[cp->ai];
            float rSum = Fp->radius + Fc->radius;
            float radiusSq = rSum * rSum;
            float d = ShipDistance(pp, cp);
            gDistanceLUP[pp->id][cp->id] = d;
            gDistanceLUP[cp->id][pp->id] = d;

            if (d < radiusSq) {
                float offset = sqrtf(radiusSq - d) * 0.5f;
                if (pp->index != cp->index) {
                    // Different faction: damage
                    cp->fuel -= 500.0f; 
                    if (cp->fuel <= 0.0f) {
                        cp->dead = 1;
                        ScoreTagAdd(cp->x, cp->y, cp->z, Fc->tag, Fc->points);
                    }
                    pp->fuel -= 500.0f; 
                    if (pp->fuel <= 0.0f) {
                        pp->dead = 1;
                        ScoreTagAdd(pp->x, pp->y, pp->z, Fp->tag, Fp->points);
                    }
                } else {
                    // Same faction: bounce
                    pp->x -= gNX * offset; pp->y -= gNY * offset; pp->z -= gNZ * offset;
                    pp->vx = pp->vx * 0.75f - gNX * offset;
                    pp->vy = pp->vy * 0.75f - gNY * offset;
                    pp->vz = pp->vz * 0.75f - gNZ * offset;
                    cp->x += gNX * offset; cp->y += gNY * offset; cp->z += gNZ * offset;
                    cp->vx = cp->vx * 0.75f + gNX * offset;
                    cp->vy = cp->vy * 0.75f + gNY * offset;
                    cp->vz = cp->vz * 0.75f + gNZ * offset;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Global JX/JZ/DP/DY/Thrust action variables (mirrors Blitz globals)
// ---------------------------------------------------------------------------
float gJX = 0.0f, gJZ = 0.0f, gDP = 0.0f, gDY = 0.0f;
float gThrust = 0.0f, gShoot = 0.0f, gLaunch = 0.0f;
float gInfect = 0.0f, gBomb = 0.0f, gAttract = 0.0f;
float gMouseX = 0.0f, gMouseY = 0.0f;

// ---------------------------------------------------------------------------
// ShipUpdateAll  — full PLAYERupdate port.
// Meant to be called once per game tick.
// For player 0, simple WASD mouse-style controls are applied.
// ---------------------------------------------------------------------------
void ShipUpdateAll(float dt) {
    (void)dt;
    for (int pid = 0; pid <= gMaxShips; pid++) {
        Ship *p = &gShips[pid];

        // --- Death Logic ---
        if (p->dead > 1) {
            p->dead++;
            if (p->dead >= gFlyingObjects[p->ai].deadTimer) {
                p->dead = -1;
                if (p->index == 1) gEnemies--;
            }
            continue;
        }

        if (p->dead == 1) {
            // Mirrors Source.bb 876-892 (Explosion trigger)
            if (p->inView) {
                // Compute pan from explosion position relative to camera heading
                Ship *cam = &gShips[gCam];
                float ex = p->x - cam->x;
                float ez = p->z - cam->z;
                if (fabsf(ex) > (float)SIZE/2.0f) ex = ((float)SIZE - fabsf(ex)) * (ex < 0 ? 1.0f : -1.0f);
                if (fabsf(ez) > (float)SIZE/2.0f) ez = ((float)SIZE - fabsf(ez)) * (ez < 0 ? 1.0f : -1.0f);
                float bearing = atan2f(ex, -ez); // Ship forward=-Z in world (Z-flipped from Blitz LH coords)
                float camYawRad = cam->yaw * (3.14159265f / 180.0f);
                float relAngle = bearing - camYawRad;
                float epan = 0.5f + sinf(relAngle) * 0.45f;
                if (epan < 0.0f) epan = 0.0f;
                if (epan > 1.0f) epan = 1.0f;
                AudioPlay(gSoundExplode, SND_VOL_EXPLODE * SND_VOL_MASTER, epan);
                ParticleNew(13, p->x, p->y, p->z, 0, 0.5f, 0, (int)(40 * gFlyingObjects[p->ai].radius), 0, 0, 0, p->index, 0.0f, 0.0f);
                ParticleNew(3, p->x, p->y, p->z, 0, 1.0f, 0, 10, 0, 0, 0, p->index, 0.0f, 0.0f);
            }
            p->dead = 2; // Transition to next state to avoid re-triggering (CRITICAL FIX)
            continue;
        }

        if (p->dead != 0) continue;

        FlyingObject *F = &gFlyingObjects[p->ai];

        // Counters
        if (p->reload   > 0) p->reload--;
        if (p->counter  > 0) p->counter--;
        
        // Force continuous thruster for the user craft (ID 0)
        int thrustRate = (p->id == 0) ? 0 : F->thrustRate;
        if (p->thrustCounter > 0) p->thrustCounter--;
        if (p->aiCounter > 0) p->aiCounter--;

        // --- Controls ---
        if (p->id == 0) {
            // Ship 0: keyboard / mouse
            gJX = gJZ = gThrust = gShoot = gLaunch = 0.0f;
            float yawSpeed   = (float)F->turnSpeed;
            float pitchSpeed = (float)F->turnSpeed;

            gJX = -gMouseX * MOUSE_YAW_SENSITIVITY / (float)F->turnSpeed;
            gJZ = -gMouseY * MOUSE_PITCH_SENSITIVITY / (float)F->turnSpeed;
            if (MOUSE_INVERT_PITCH) gJZ = -gJZ;

            // Keyboard steering (additive)
            if (BIND_YAW_LEFT_KEY > 0 && IsKeyDown(BIND_YAW_LEFT_KEY))   gJX += 1.0f;
            if (BIND_YAW_RIGHT_KEY > 0 && IsKeyDown(BIND_YAW_RIGHT_KEY))  gJX -= 1.0f;
            if (BIND_PITCH_UP_KEY > 0 && IsKeyDown(BIND_PITCH_UP_KEY))    gJZ += 1.0f;
            if (BIND_PITCH_DOWN_KEY > 0 && IsKeyDown(BIND_PITCH_DOWN_KEY))  gJZ -= 1.0f;

            // Thrust
            if ((BIND_THRUST_KEY > 0 && IsKeyDown(BIND_THRUST_KEY)) || 
                (BIND_THRUST_MOUSE >= 0 && IsMouseButtonDown(BIND_THRUST_MOUSE))) gThrust = (float)F->thrust;
            // Shoot
            if ((BIND_FIRE_KEY > 0 && IsKeyDown(BIND_FIRE_KEY)) || 
                (BIND_FIRE_MOUSE >= 0 && IsMouseButtonDown(BIND_FIRE_MOUSE)))  gShoot  = 1.0f;
            // Launch child/missile
            if ((BIND_LAUNCH_KEY > 0 && IsKeyDown(BIND_LAUNCH_KEY)) || 
                (BIND_LAUNCH_MOUSE >= 0 && IsMouseButtonDown(BIND_LAUNCH_MOUSE))) gLaunch = 1.0f;

            p->jxIntent = gJX;
            p->jzIntent = gJZ;
            p->thrustIntent = gThrust;
            p->shootIntent = gShoot;
            p->launchIntent = gLaunch;
            p->infectIntent = 0.0f;
            p->bombIntent = 0.0f;

            (void)yawSpeed; (void)pitchSpeed;
        } else {
            // AI
            ShipAI(p, dt);
        }

        // --- Execute Actions ---
        if (p->launchIntent > 0.0f && p->counter == 0) {
            ShipLaunch(p);
        }

        // No pitch during landing
        if (p->y <= 2.9f) p->jzIntent = 0.0f;

        // Fuel drain
        p->fuel = clampf(p->fuel - p->thrustIntent * 10.0f, 0.0f, 500.0f);

        // --- Thruster Particles moved to end of loop ---

        // --- AI Spawning (Spores / Bombs) moved to end of loop ---

        // --- Movement & Integration ---

        // Engine throttle cutoff
        if (F->engine == 0 && (p->y > (float)MAX_HEIGHT || p->fuel == 0.0f)) p->thrustIntent = 0.0f;

        // Spin
        p->spin  = wrapf(p->spin + (float)F->spinSpeed, 360.0f, 0.0f);

        // Pitch / Yaw (Unified polarity for player/AI)
        float jx = (p->index == 0) ? gJX : p->jxIntent;
        float jz = (p->index == 0) ? gJZ : p->jzIntent;

        p->pitch = clampf(p->pitch - jz * (float)F->turnSpeed,
                          (float)F->pitchMin, (float)F->pitchMax);
        p->yaw   = p->yaw - jx * (float)F->turnSpeed;

        // Thrust vector vs Forward vector
        float fx, fy, fz; // Forward (Nose)
        float tx, ty, tz; // Thrust
        
        TFormForward(p->pitch, p->yaw, F->engine, &fx, &fy, &fz);
        
        if (F->engine == 0) {
            // Helicopter-style: Thrust is UP relative to ship hull
            TFormVector(0, 1, 0, p->pitch, p->yaw, &tx, &ty, &tz);
        } else {
            // Jet/Missile-style: Thrust is FORWARD along ship nose
            tx = fx; ty = fy; tz = fz;
        }

        // Velocity update (Thrust applied along thrust vector)
        p->vx = p->vx * F->momentum + tx * p->thrustIntent;
        p->vy = p->vy * F->momentum + ty * p->thrustIntent
                - ((F->engine == 0 ? 1.0f : 0.0f) + (p->y > (float)MAX_HEIGHT ? 1.0f : 0.0f)) * gGravity;
        p->vz = p->vz * F->momentum + tz * p->thrustIntent;

        // Integrate (Blitz Source.bb logic: +vx, -vz)
        // Scaled by 0.5 for 60Hz simulation (original vx/vz were per 1/30s)
        p->x = wrapf(p->x + p->vx * 0.5f, (float)SIZE, 0.0f);
        p->z = wrapf(p->z - p->vz * 0.5f, (float)SIZE, 0.0f);
        p->y = p->y + p->vy * 0.5f;

        // Ground collision
        float gh2 = TerrainGetHeight(p->x, p->z, 1) + (p->ai != 17 ? 0.5f : 0.0f);
        if (p->y < gh2) {
            p->y = gh2;

            // Refuel on landing pad (index=0 + gentle descent + LandIndex=3)
            int canRefuel = (p->index == 0 && p->vy > CRASH_THRESHOLD && p->pitch < 6.0f
                             && gTerrain[(int)p->x][(int)p->z].landIndex == 3);
                             
            if (!canRefuel) {
                // Determine if impact is hard enough to kill
                if (p->vy < CRASH_THRESHOLD) {
                    p->dead = 1; // Death logic handles audio/particles in next frame
                } else {
                    // Gentle impact but not on a pad: bounce
                    if (p->vy < 0.0f) p->vy = -p->vy * 0.3f;
                    p->vx *= 0.8f;
                    p->vz *= 0.8f;
                }
            } else {
                p->pitch = 0.0f;
                p->fuel  = clampf(p->fuel + 1.5f, 0.0f, 500.0f);
                p->vy    = -p->vy * 0.5f;
                p->vx   *= 0.75f;
                p->vz   *= 0.75f;
            }
        }

        ShipPosition(p);

        // --- Particle Spawning (Moved here to eliminate 1-frame lag and align with visual pivot) ---
        if (p->inView) {
            float vy_vis = p->y + 0.5f;

            // Thrusters
            if (p->thrustIntent > 0.0f && p->thrustCounter == 0) {
                p->thrustCounter = thrustRate;
                if (F->engine == 0) {
                    ParticleNew(0, p->x, vy_vis, p->z, 0.0f, -0.5f, 0.0f, G_PLAYER_THRUST_COUNT, p->vx, p->vy, p->vz, p->index, p->pitch, p->yaw);
                } else if (F->engine == 2) {
                    ParticleNew(2, p->x, vy_vis, p->z, 0.0f, 0.0f, -1.0f, 1, p->vx, p->vy, p->vz, p->index, p->pitch, p->yaw);
                } else if (F->engine == 3) {
                    ParticleNew(18, p->x, vy_vis, p->z, 0.0f, 0.0f, -3.5f, 3, p->vx, p->vy, p->vz, p->index, p->pitch, p->yaw);
                }
            }

            // AI Spores / Bombs (Source.bb 810-811)
            if (p->reload == 0) {
                if (p->infectIntent > 0.0f) {
                    ParticleNew(11, p->x, vy_vis, p->z, 0.0f, 0.5f, 0.0f, 1, p->vx, p->vy, p->vz, p->index, p->pitch, p->yaw);
                    p->reload = F->fireRate;
                }
                if (p->bombIntent > 0.0f) {
                    ParticleNew(12, p->x, vy_vis, p->z, 0.0f, -0.5f, 0.0f, 1, p->vx, p->vy, p->vz, p->index, p->pitch, p->yaw);
                    p->reload = F->fireRate;
                }
            }

            // Damage / Low Fuel Smoke (Source.bb 795)
            // Original: Rand(0,3)==0 at 30Hz. At 60Hz we use Rand(0,7)==0 for roughly same density.
            if (F->engine != 2 && p->fuel <= 100.0f && GetRandomValue(0, 7) == 0) {
                ParticleNew(14, p->x, p->y, p->z, 0.0f, 0.0f, 0.0f, 2, 0, 0, 0, p->index, 0.0f, 0.0f);
            }

            // Shooting (Source.bb 826-844)
            if (p->shootIntent > 0.0f && p->reload == 0) {
                p->reload = F->fireRate;
                if (p->inView) {
                    // Audio
                    if (p->id == 0) {
                        AudioPlay(gSoundShoot, SND_VOL_SHOOT * SND_VOL_MASTER, 0.5f);
                    } else {
                        Ship *cam = &gShips[gCam];
                        float dx = p->x - cam->x, dz = p->z - cam->z;
                        if (fabsf(dx) > (float)SIZE/2.0f) dx = ((float)SIZE - fabsf(dx)) * (dx < 0 ? 1.0f : -1.0f);
                        if (fabsf(dz) > (float)SIZE/2.0f) dz = ((float)SIZE - fabsf(dz)) * (dz < 0 ? 1.0f : -1.0f);
                        float d = sqrtf(dx*dx + dz*dz);
                        float vol = (1.0f - (d / SND_FALLOFF_DIST)) * SND_VOL_SHOOT;
                        float bearing = atan2f(dx, -dz); // Ship forward=-Z in world (Z-flipped from Blitz LH coords)
                        float pan = 0.5f + sinf(bearing - cam->yaw * DEG2RAD) * 0.45f;
                        if (vol > 0.01f) AudioPlay(gSoundShoot, clampf(vol * SND_VOL_MASTER, 0, 1), clampf(pan, 0, 1));
                    }

                    // Particles (Bullets)
                    for (int ga = -p->spread; ga <= p->spread; ga++) {
                        float gunPitch = p->pitch;
                        float gunYaw = p->yaw - (float)ga * 3.0f;
                        ParticleNew(1, p->x, vy_vis, p->z, 0.0f, 0.0f, 0.25f, 1, p->vx, p->vy, p->vz, p->index, gunPitch, gunYaw);
                    }
                } else {
                    // Off-screen infection spread (Source.bb 816-820)
                    // Original fired every frame at 30Hz; throttle with fireRate to keep 30Hz parity at 60Hz.
                    if (p->ai == 1 || p->ai == 2) {
                        float an = (float)GetRandomValue(0, 360) * DEG2RAD;
                        float ra = (float)GetRandomValue(0, 6);
                        TerrainMapAdd(0, p->x + ra * cosf(an), p->z + ra * sinf(an), 1);
                        p->reload = F->fireRate;
                    }
                }
            }
        }
    }

    CollisionShip();
}

// ---------------------------------------------------------------------------
// ShipUpdateSimpleControls  — thin wrapper used by main.c in STATE_WAVE
// (Replaces the old WASD stub; actual work done in ShipUpdateAll)
// ---------------------------------------------------------------------------
void ShipUpdateSimpleControls(float dt) {
    (void)dt;
    ShipUpdateAll(dt);
}
