#include "world.h"
#include "constants.h"

// Globals - Definitive storage for the whole game.
float gGravity = G_GRAVITY;
Grid gGrid = {0};
Hud gHud = {0};
SoundGroup gSoundGroups[SOUND_GROUPS + 1] = {0};
Terrain gTerrain[SIZE][SIZE] = {0};
GroundObject gGroundObjects[GROUND_OBJECTS + 1] = {0};
FlyingObject gFlyingObjects[FLYING_OBJECTS + 1] = {0};
Ship gShips[MAX_SHIPS + 1] = {0};
Particle gParticles[MAX_PARTICLES + 1] = {0};
ParticleTemplate gParticleTemplates[PARTICLE_TEMPLATES + 1] = {0};
ScoreTag gScoreTags[MAX_SCORE_TAGS + 1] = {0};
Menu gMenus[MENU_COUNT + 1] = {0};
float gDistanceLUP[MAX_SHIPS + 1][MAX_SHIPS + 1] = {0};

int   gMaxShips = 0;
float gNX = 0.0f, gNY = 0.0f, gNZ = 0.0f;
int   gEnemies = 0;
int   gScore = 0;
float gFadeStatus = 0.0f;
int   gAreaTotal = 0, gAreaInfected = 0;
float gRipple = 0.0f;
float gRotate = 0.0f;

WaveData gWaveData[MAX_WAVES] = {0};

// -----------------------------------------------------------------------
// FlyingObjectInit  –  Restore physics and data from Source.bb logic.
// -----------------------------------------------------------------------
#include "my3d.h"
#include <raylib.h>

void FlyingObjectInit(void) {
    // 60Hz Heavy Tuning
    // Pitch system (engine=0): 0=Up (Full lift), 90=Level (No lift), 180=Down (Nosedive)
    
    // Balancing for E_THRUST_SCALE = 1.0 (Previously 0.15)
    // Source base for most enemies is 0.015 at 30Hz -> 0.0075 at 60Hz.
    
    static const FlyingObject table[] = {
        // 0 Hoverplane (player)
        {.name="Hoverplane",.points=0,.tag=0,.thrust=SHIP_0_THRUST,.momentum=SHIP_0_MOMENTUM,
         .fireRate=SHIP_0_FIRE_RATE,.missileRate=30,.damage=100,.spinSpeed=0,.engine=0,
         .pitchMin=PITCH_PLAYER_MIN,.pitchMax=PITCH_PLAYER_MAX,.turnSpeed=SHIP_0_TURN,.range=128,
         .r=255,.g=255,.b=255,.cruiseHeight=SHIP_0_CRUISE,.soundGroup=0},
        
        // 1 Seeder
        {.name="Seeder",.points=100,.tag=2,.thrust=SHIP_1_THRUST*E_THRUST_SCALE,.momentum=SHIP_1_MOMENTUM,
         .fireRate=SHIP_1_FIRE_RATE*E_RELOAD_SCALE,.missileRate=0,.damage=500,.spinSpeed=2.5f,.engine=1,
         .pitchMin=0,.pitchMax=0,.turnSpeed=SHIP_1_TURN*E_TURN_SCALE,.range=0,
         .r=0,.g=100,.b=255,.cruiseHeight=SHIP_1_CRUISE,.soundGroup=1},
        
        // 2 Bomber
        {.name="Bomber",.points=800,.tag=9,.thrust=SHIP_2_THRUST*E_THRUST_SCALE,.momentum=SHIP_2_MOMENTUM,
         .fireRate=SHIP_2_FIRE_RATE*E_RELOAD_SCALE,.missileRate=0,.damage=500,.spinSpeed=0,.engine=1,
         .pitchMin=0,.pitchMax=0,.turnSpeed=SHIP_2_TURN*E_TURN_SCALE,.range=0,
         .r=96,.g=96,.b=255,.cruiseHeight=SHIP_2_CRUISE,.soundGroup=0},
        
        // 3 Pest
        {.name="Pest",.points=400,.tag=5,.thrust=SHIP_3_THRUST*E_THRUST_SCALE,.momentum=SHIP_3_MOMENTUM,
         .fireRate=SHIP_3_FIRE_RATE*E_RELOAD_SCALE,.missileRate=0,.damage=500,.spinSpeed=5,.engine=2,
         .pitchMin=PITCH_CLIMBER_MIN,.pitchMax=PITCH_CLIMBER_MAX,.turnSpeed=SHIP_3_TURN*E_TURN_SCALE,.range=0,
         .r=128,.g=128,.b=128,.cruiseHeight=SHIP_3_CRUISE,.soundGroup=2},
        
        // 4 Drone
        {.name="Drone",.points=300,.tag=4,.thrust=SHIP_4_THRUST*E_THRUST_SCALE,.momentum=SHIP_4_MOMENTUM,
         .fireRate=SHIP_4_FIRE_RATE*E_RELOAD_SCALE,.missileRate=0,.damage=500,.spinSpeed=0,.engine=0,
         .pitchMin=PITCH_FLYER_MIN,.pitchMax=PITCH_FLYER_MAX,.turnSpeed=SHIP_4_TURN*E_TURN_SCALE,.range=SHIP_4_RANGE,
         .r=160,.g=80,.b=40,.cruiseHeight=SHIP_4_CRUISE,.soundGroup=0},
        
        // 5 Mutant
        {.name="Mutant",.points=500,.tag=6,.thrust=SHIP_5_THRUST*E_THRUST_SCALE,.momentum=SHIP_5_MOMENTUM,
         .fireRate=SHIP_5_FIRE_RATE*E_RELOAD_SCALE,.missileRate=0,.damage=500,.spinSpeed=0,.engine=0,
         .pitchMin=PITCH_FLYER_MIN,.pitchMax=PITCH_FLYER_MAX,.turnSpeed=SHIP_5_TURN*E_TURN_SCALE,.range=SHIP_5_RANGE,
         .r=255,.g=0,.b=255,.cruiseHeight=SHIP_5_CRUISE,.soundGroup=0},
        
        // 6 Fighter
        {.name="Fighter",.points=750,.tag=7,.thrust=SHIP_6_THRUST*E_THRUST_SCALE,.momentum=SHIP_6_MOMENTUM,
         .fireRate=SHIP_6_FIRE_RATE*E_RELOAD_SCALE,.missileRate=0,.damage=250,.spinSpeed=0,.engine=0,
         .pitchMin=PITCH_FLYER_MIN,.pitchMax=PITCH_FLYER_MAX,.turnSpeed=SHIP_6_TURN*E_TURN_SCALE,.range=SHIP_6_RANGE,
         .r=255,.g=160,.b=0,.cruiseHeight=SHIP_6_CRUISE,.soundGroup=0},
        
        // 7 Destroyer
        {.name="Destroyer",.points=2000,.tag=11,.thrust=SHIP_7_THRUST*E_THRUST_SCALE,.momentum=SHIP_7_MOMENTUM,
         .fireRate=SHIP_7_FIRE_RATE*E_RELOAD_SCALE,.missileRate=0,.damage=50,.spinSpeed=0,.engine=0,
         .pitchMin=PITCH_FLYER_MIN,.pitchMax=PITCH_FLYER_MAX,.turnSpeed=SHIP_7_TURN*E_TURN_SCALE,.range=SHIP_7_RANGE,
         .r=255,.g=96,.b=96,.cruiseHeight=SHIP_7_CRUISE,.soundGroup=0},
        
        // 8 Attractor
        {.name="Attractor",.points=1000,.tag=10,.thrust=SHIP_8_THRUST*E_THRUST_SCALE,.momentum=SHIP_8_MOMENTUM,
         .fireRate=SHIP_8_FIRE_RATE*E_RELOAD_SCALE,.missileRate=0,.damage=100,.spinSpeed=2.5f,.engine=1,
         .pitchMin=0,.pitchMax=0,.turnSpeed=SHIP_8_TURN*E_TURN_SCALE,.range=12,
         .r=255,.g=255,.b=0,.cruiseHeight=SHIP_8_CRUISE,.soundGroup=3},
        
        // 9 Repulsor
        {.name="Repulsor",.points=1000,.tag=10,.thrust=SHIP_9_THRUST*E_THRUST_SCALE,.momentum=SHIP_9_MOMENTUM,
         .fireRate=SHIP_9_FIRE_RATE*E_RELOAD_SCALE,.missileRate=0,.damage=100,.spinSpeed=7.5f,.engine=1,
         .pitchMin=0,.pitchMax=0,.turnSpeed=SHIP_9_TURN*E_TURN_SCALE,.range=15,
         .r=96,.g=255,.b=96,.cruiseHeight=SHIP_9_CRUISE,.soundGroup=6},
        
        // 10 Mystery
        {.name="Mystery",.points=2000,.tag=11,.thrust=SHIP_10_THRUST*E_THRUST_SCALE,.momentum=SHIP_10_MOMENTUM,
         .fireRate=SHIP_10_FIRE_RATE*E_RELOAD_SCALE,.missileRate=SHIP_10_MISSILE_RATE*E_RELOAD_SCALE,.damage=20,.spinSpeed=0,.engine=1,
         .pitchMin=0,.pitchMax=0,.turnSpeed=SHIP_10_TURN*E_TURN_SCALE,.range=20,
         .r=0,.g=0,.b=0,.cruiseHeight=SHIP_10_CRUISE,.soundGroup=0},
        
        // 11-14: Generators
        {.name="Generator",.points=2000,.tag=11,.thrust=SHIP_GEN_THRUST*E_THRUST_SCALE,.momentum=SHIP_GEN_MOMENTUM,
         .fireRate=SHIP_GEN_FIRE_RATE*E_RELOAD_SCALE,.missileRate=SHIP_11_MISSILE_RATE*E_RELOAD_SCALE,.damage=10,.spinSpeed=1,.engine=1,
         .pitchMin=0,.pitchMax=0,.turnSpeed=SHIP_GEN_TURN*E_TURN_SCALE,.range=64,
         .r=160,.g=80,.b=40,.cruiseHeight=30,.soundGroup=5},
        {.name="Generator",.points=2000,.tag=11,.thrust=SHIP_GEN_THRUST*E_THRUST_SCALE,.momentum=SHIP_GEN_MOMENTUM,
         .fireRate=SHIP_GEN_FIRE_RATE*E_RELOAD_SCALE,.missileRate=SHIP_GEN_MISSILE_RATE*E_RELOAD_SCALE,.damage=10,.spinSpeed=1,.engine=1,
         .pitchMin=0,.pitchMax=0,.turnSpeed=SHIP_GEN_TURN*E_TURN_SCALE,.range=64,
         .r=255,.g=0,.b=255,.cruiseHeight=30,.soundGroup=5},
        {.name="Generator",.points=2000,.tag=11,.thrust=SHIP_GEN_THRUST*E_THRUST_SCALE,.momentum=SHIP_GEN_MOMENTUM,
         .fireRate=SHIP_GEN_FIRE_RATE*E_RELOAD_SCALE,.missileRate=SHIP_GEN_MISSILE_RATE*E_RELOAD_SCALE,.damage=10,.spinSpeed=1,.engine=1,
         .pitchMin=0,.pitchMax=0,.turnSpeed=SHIP_GEN_TURN*E_TURN_SCALE,.range=64,
         .r=255,.g=160,.b=0,.cruiseHeight=30,.soundGroup=5},
        {.name="Generator",.points=2000,.tag=11,.thrust=SHIP_GEN_THRUST*E_THRUST_SCALE,.momentum=SHIP_GEN_MOMENTUM,
         .fireRate=SHIP_GEN_FIRE_RATE*E_RELOAD_SCALE,.missileRate=SHIP_GEN_MISSILE_RATE*E_RELOAD_SCALE,.damage=10,.spinSpeed=1,.engine=1,
         .pitchMin=0,.pitchMax=0,.turnSpeed=SHIP_GEN_TURN*E_TURN_SCALE,.range=64,
         .r=255,.g=96,.b=96,.cruiseHeight=30,.soundGroup=5},
        
        // 15 Cruiser
        {.name="Cruiser",.points=5000,.tag=13,.thrust=SHIP_15_THRUST*E_THRUST_SCALE,.momentum=SHIP_15_MOMENTUM,
         .fireRate=SHIP_15_FIRE_RATE*E_RELOAD_SCALE,.missileRate=SHIP_15_MISSILE_RATE*E_RELOAD_SCALE,.damage=5,.spinSpeed=0,.engine=3,
         .pitchMin=-30,.pitchMax=30,.turnSpeed=SHIP_15_TURN*E_TURN_SCALE,.range=64,
         .r=192,.g=64,.b=255,.cruiseHeight=SHIP_15_CRUISE,.soundGroup=9},
        
        // 16 Elite
        {.name="Elite",.points=2500,.tag=12,.thrust=SHIP_16_THRUST*E_THRUST_SCALE,.momentum=SHIP_16_MOMENTUM,
         .fireRate=SHIP_16_FIRE_RATE*E_RELOAD_SCALE,.missileRate=SHIP_16_MISSILE_RATE*E_RELOAD_SCALE,.damage=25,.spinSpeed=0,.engine=0,
         .pitchMin=PITCH_BOSSF_MIN,.pitchMax=PITCH_BOSSF_MAX,.turnSpeed=SHIP_16_TURN*E_TURN_SCALE,.range=128,
         .r=192,.g=64,.b=255,.cruiseHeight=SHIP_16_CRUISE,.soundGroup=0},
        
        // 17 Monster
        {.name="Monster",.points=2000,.tag=11,.thrust=SHIP_17_THRUST*E_THRUST_SCALE,.momentum=SHIP_17_MOMENTUM,
         .fireRate=SHIP_17_FIRE_RATE*E_RELOAD_SCALE,.missileRate=0,.damage=500,.spinSpeed=0.5f,.engine=1,
         .pitchMin=0,.pitchMax=0,.turnSpeed=SHIP_17_TURN*E_TURN_SCALE,.range=32,
         .r=255,.g=255,.b=255,.cruiseHeight=0,.soundGroup=8},
        
        // 18 Allie
        {.name="Allie",.points=0,.tag=0,.thrust=SHIP_18_THRUST*E_THRUST_SCALE,.momentum=SHIP_18_MOMENTUM,
         .fireRate=SHIP_18_FIRE_RATE*E_RELOAD_SCALE,.missileRate=SHIP_18_MISSILE_RATE,.damage=25,.spinSpeed=0,.engine=0,
         .pitchMin=PITCH_PLAYER_MIN,.pitchMax=PITCH_PLAYER_MAX,.turnSpeed=SHIP_18_TURN*E_TURN_SCALE,.range=128,
         .r=255,.g=255,.b=255,.cruiseHeight=SHIP_18_CRUISE,.soundGroup=0},
        
        // 19 Spore
        {.name="Spore",.points=150,.tag=3,.thrust=SHIP_19_THRUST*E_THRUST_SCALE,.momentum=SHIP_19_MOMENTUM,
         .fireRate=SHIP_19_FIRE_RATE*E_RELOAD_SCALE,.missileRate=0,.damage=500,.spinSpeed=10,.engine=2,
         .pitchMin=PITCH_CLIMBER_MIN,.pitchMax=PITCH_CLIMBER_MAX,.turnSpeed=SHIP_19_TURN*E_TURN_SCALE,.range=0,
         .r=0,.g=0,.b=0,.cruiseHeight=20,.soundGroup=7},
        
        // 20 Missile
        {.name="Missile",.points=0,.tag=0,.thrust=SHIP_20_THRUST*E_THRUST_SCALE,.momentum=SHIP_20_MOMENTUM,
         .fireRate=SHIP_20_FIRE_RATE*E_RELOAD_SCALE,.missileRate=0,.damage=500,.spinSpeed=0,.engine=2,
         .pitchMin=PITCH_MISS_MIN,.pitchMax=PITCH_MISS_MAX,.turnSpeed=SHIP_20_TURN*E_TURN_SCALE,.range=SHIP_20_RANGE,
         .r=255,.g=255,.b=255,.cruiseHeight=30,.soundGroup=4},
    };
    int n = (int)(sizeof(table)/sizeof(table[0]));
    for (int i = 0; i < n && i <= FLYING_OBJECTS; i++) {
        gFlyingObjects[i] = table[i];
        
        gFlyingObjects[i].deadTimer = 200 - 195 * (i == 20);
        gFlyingObjects[i].radius = 1.0f; 
        
        // Load custom model format
        if (gFlyingObjects[i].name[0] != '\0') {
            gFlyingObjects[i].mesh = LoadMy3D(gFlyingObjects[i].name);
        }
        
        // ThrustRate = 5 * (l>0 And l<20 And l<>15) -> 10 for 60Hz
        gFlyingObjects[i].thrustRate = 10 * (i > 0 && i < 20 && i != 15);
    }
}

void WaveDataInit(void) {
    // Correct map names (Level1, Level2... are case sensitive on Linux)
    static const WaveData table[MAX_WAVES] = {
        {"Level1", {1, 2, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}},
        {"",       {1, 3, 1, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}},
        {"",       {1, 4, 2, 3, 2, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0}},
        {"",       {1, 5, 2, 4, 2, 2, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0}},
        {"Level2", {1, 5, 3, 5, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0}},
        {"",       {1, 7, 3, 6, 3, 3, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 0}},
        {"",       {1, 8, 4, 5, 4, 3, 3, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 1, 0}},
        {"",       {1, 9, 4, 4, 4, 4, 3, 2, 2, 3, 2, 0, 0, 0, 0, 0, 0, 1, 0}},
        {"Level3", {1, 8, 5, 3, 5, 4, 4, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 1, 0}},
        {"",       {1, 7, 5, 2, 5, 5, 4, 3, 3, 4, 3, 1, 0, 0, 0, 0, 0, 1, 0}},
        {"",       {1, 6, 4, 1, 0, 5, 5, 4, 4, 4, 3, 1, 0, 0, 0, 0, 0, 2, 0}},
        {"",       {1, 5, 3, 1, 0, 0, 5, 4, 4, 3, 4, 2, 1, 0, 0, 0, 0, 3, 0}},
        {"Level4", {1, 4, 2, 1, 0, 0, 0, 5, 3, 2, 4, 2, 1, 1, 0, 0, 1, 2, 2}},
        {"",       {1, 3, 1, 1, 0, 0, 0, 5, 2, 1, 3, 3, 2, 1, 0, 0, 2, 1, 0}},
        {"",       {1, 2, 1, 1, 0, 0, 0, 0, 1, 0, 3, 3, 2, 2, 1, 0, 3, 1, 0}},
        {"",       {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 2, 1, 3, 2, 1, 0, 4, 1, 0}},
        {"Level5", {1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 2, 1, 3, 3, 2, 1, 0, 1, 4}},
        {"",       {1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 1, 1, 1, 3, 2, 1, 0, 1, 0}},
        {"",       {1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 3, 2, 0, 1, 0}},
        {"",       {1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 2, 0, 1, 0}},
        {"Level6", {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 4, 0, 1, 8}},
        {"",       {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}},
    };
    for (int i = 0; i < MAX_WAVES; i++) {
        gWaveData[i] = table[i];
    }
}
