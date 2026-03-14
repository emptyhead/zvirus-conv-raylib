#pragma once

// Constants mirrored from Source.bb (keep names close to original).

enum {
  FPS = 60,

  MAX_HEIGHT = 150,

  MAX_PARTICLES = 16384,
  MAX_SCORE_TAGS = 10,

  SIZE = 256, // terrain width/height

  GROUND_OBJECTS = 11,
  FLYING_OBJECTS = 20,
  PARTICLE_TEMPLATES = 21,
  SOUND_GROUPS = 9,

  MAX_SHIPS = 100,
  MENU_COUNT = 4,

  MAX_WAVES = 22,
};

// Speed threshold for ground impact death (originally -0.1).
// Values further from zero (e.g. -0.5) make it harder to crash.
static const float CRASH_THRESHOLD = -0.5f;

static const float OSCALE = 0.0625f;

// --- Ship Physics & Timing (60Hz Parity) ---
// Formulae: Thrust/Turn = Blitz/2, Momentum = sqrt(Blitz), Rate = Blitz*2

// 0: Hoverplane (Player)
#define SHIP_0_THRUST 0.01f
#define SHIP_0_MOMENTUM 0.9962f
#define SHIP_0_TURN 2.5f
#define SHIP_0_FIRE_RATE 6
#define SHIP_0_CRUISE 5.0f

// 1: Seeder
#define SHIP_1_THRUST 0.0005f
#define SHIP_1_MOMENTUM 0.995f
#define SHIP_1_TURN 2.5f
#define SHIP_1_FIRE_RATE 6 * 2
#define SHIP_1_CRUISE 4.0f

// 2: Bomber
#define SHIP_2_THRUST 0.001f
#define SHIP_2_MOMENTUM 0.995f
#define SHIP_2_TURN 2.5f
#define SHIP_2_FIRE_RATE 40 * 2
#define SHIP_2_CRUISE 8.0f

// 3: Pest
#define SHIP_3_THRUST 0.02f
#define SHIP_3_MOMENTUM 0.9487f
#define SHIP_3_TURN 2.5f
#define SHIP_3_FIRE_RATE 4 * 2
#define SHIP_3_CRUISE 20.0f

// 4: Drone
#define SHIP_4_THRUST 0.0075f
#define SHIP_4_MOMENTUM 0.9899f
#define SHIP_4_TURN 1.0f
#define SHIP_4_FIRE_RATE 30 * 2
#define SHIP_4_CRUISE 20.0f
#define SHIP_4_RANGE 48.0f

// 5: Mutant
#define SHIP_5_THRUST 0.0075f
#define SHIP_5_MOMENTUM 0.9899f
#define SHIP_5_TURN 1.5f
#define SHIP_5_FIRE_RATE 24 * 2
#define SHIP_5_CRUISE 20.0f
#define SHIP_5_RANGE 48.0f

// 6: Fighter
#define SHIP_6_THRUST 0.0075f
#define SHIP_6_MOMENTUM 0.9899f
#define SHIP_6_TURN 2.0f
#define SHIP_6_FIRE_RATE 18 * 2
#define SHIP_6_CRUISE 20.0f
#define SHIP_6_RANGE 64.0f

// 7: Destroyer
#define SHIP_7_THRUST 0.00875f
#define SHIP_7_MOMENTUM 0.9899f
#define SHIP_7_TURN 2.5f
#define SHIP_7_FIRE_RATE 18 * 2
#define SHIP_7_CRUISE 20.0f
#define SHIP_7_RANGE 80.0f

// 8: Attractor
#define SHIP_8_THRUST 0.00075f
#define SHIP_8_MOMENTUM 0.995f
#define SHIP_8_TURN 2.5f
#define SHIP_8_FIRE_RATE 4 * 2
#define SHIP_8_CRUISE 5.0f

// 9: Repulsor
#define SHIP_9_THRUST 0.00125f
#define SHIP_9_MOMENTUM 0.995f
#define SHIP_9_TURN 2.5f
#define SHIP_9_FIRE_RATE 18 * 2
#define SHIP_9_CRUISE 7.5f

// 10: Mystery
#define SHIP_10_THRUST 0.00075f
#define SHIP_10_MOMENTUM 0.995f
#define SHIP_10_TURN 2.5f
#define SHIP_10_FIRE_RATE 4 * 2
#define SHIP_10_MISSILE_RATE 300 * 2
#define SHIP_10_CRUISE 25.0f

// 11-14: Generators
#define SHIP_GEN_THRUST 0.0000005f
#define SHIP_GEN_MOMENTUM 0.0f
#define SHIP_GEN_TURN 2.5f
#define SHIP_GEN_FIRE_RATE 6 * 2
#define SHIP_11_MISSILE_RATE 600
#define SHIP_GEN_MISSILE_RATE 600

// 15: Cruiser
#define SHIP_15_THRUST 0.002f
#define SHIP_15_MOMENTUM 0.995f
#define SHIP_15_TURN 10.0f
#define SHIP_15_FIRE_RATE 6 * 2
#define SHIP_15_MISSILE_RATE 300 * 2
#define SHIP_15_CRUISE 50.0f

// 16: Elite
#define SHIP_16_THRUST 0.01f
#define SHIP_16_MOMENTUM 0.9962f
#define SHIP_16_TURN 2.5f
#define SHIP_16_FIRE_RATE 12 * 2
#define SHIP_16_MISSILE_RATE 180 * 2
#define SHIP_16_CRUISE 40.0f

// 17: Monster
#define SHIP_17_THRUST 0.000005f
#define SHIP_17_MOMENTUM 0.0f
#define SHIP_17_TURN 1.5f
#define SHIP_17_FIRE_RATE 4 * 2

// 18: Allie
#define SHIP_18_THRUST 0.01f
#define SHIP_18_MOMENTUM 0.9962f
#define SHIP_18_TURN 2.5f
#define SHIP_18_FIRE_RATE 6 * 2
#define SHIP_18_MISSILE_RATE 30 * 2
#define SHIP_18_CRUISE 20.0f

// 19: Spore
#define SHIP_19_THRUST 0.015f
#define SHIP_19_MOMENTUM 0.9487f
#define SHIP_19_TURN 2.5f
#define SHIP_19_FIRE_RATE 4 * 2

// 20: Missile
#define SHIP_20_THRUST 0.02f
#define SHIP_20_MOMENTUM 0.9618f
#define SHIP_20_TURN 2.5f
#define SHIP_20_FIRE_RATE 4 * 2
#define SHIP_20_RANGE 1000

// --- Pitch Limits (UP is smaller, DOWN is larger in helicopter math) ---
#define PITCH_PLAYER_MIN -160
#define PITCH_PLAYER_MAX 160
#define PITCH_FLYER_MIN 0
#define PITCH_FLYER_MAX 90
#define PITCH_BOSSF_MIN 20
#define PITCH_BOSSF_MAX 110
#define PITCH_CLIMBER_MIN -75
#define PITCH_CLIMBER_MAX 75
#define PITCH_MISS_MIN -90
#define PITCH_MISS_MAX 90

// --- Global Tuning & Particles ---
static const float G_GRAVITY = 0.0025f; // (0.005 / 2)
static const float G_AVOIDANCE_ALT = 10.0f;

// --- AI Wandering Intervals (Frames at 60Hz) ---
static const int G_AI_WANDER_FLYER_MIN = 30; // Drones/Fighters
static const int G_AI_WANDER_FLYER_MAX = 90;
static const int G_AI_WANDER_HOVERER_MIN = 20; // Seeders/Bombers
static const int G_AI_WANDER_HOVERER_MAX = 60;
static const int G_AI_WANDER_SEEKER_MIN = 40; // Pests/Spores
static const int G_AI_WANDER_SEEKER_MAX = 120;

// --- Test Level ---
static const int G_TEST_AI_TYPE = 1; // Default: Drone
static const float G_TEST_SPAWN_DIST = -30.0f;

// --- Enemy Scaling (Global tweak) ---
static const float E_THRUST_SCALE = 1.0f;
static const float E_TURN_SCALE = 1.0f;
static const float E_RELOAD_SCALE = 1.0f;

// --- Particle Tuning ---
static const float P_FADE_NORM = 0.0125f;  // (0.025 / 2)
static const float P_FADE_SLOW = 0.005f;   // (0.01 / 2)
static const float P_FADE_XSLOW = 0.0005f; // (0.001 / 2)
static const float P_VX = 0.075f;          // Original Blitz X/Z spread unit
static const float P_VY = 0.25f;           // Original Blitz Y thrust unit
static const float P_VZ = 0.075f;          // Original Blitz X/Z spread unit
static const int G_PLAYER_THRUST_COUNT = 3;

// --- Environment Particle Tuning ---
static const float G_ENV_THRESHOLD = 25.0f;
static const float G_RAIN_BIAS = 50.0f;
static const float G_RAIN_FREQ_SCALE = 4.0f;
static const float G_RAIN_DENSITY = 1.5f;
static const float G_RAIN_LEAD = 1.0f;
static const float G_DUST_DENSITY = 2.0f;
static const float G_CLOUD_DENSITY = 2.0f;

// --- Mouse Controls ---
static const float MOUSE_YAW_SENSITIVITY = 0.5f;
static const float MOUSE_PITCH_SENSITIVITY = 0.5f;
static const int MOUSE_INVERT_PITCH = 1; // 0 = normal, 1 = inverted

// --- Keybinds ---
#define BIND_FIRE_KEY KEY_SPACE
#define BIND_FIRE_MOUSE MOUSE_BUTTON_RIGHT
#define BIND_THRUST_KEY KEY_X
#define BIND_THRUST_MOUSE MOUSE_BUTTON_LEFT
#define BIND_LAUNCH_KEY KEY_M
#define BIND_LAUNCH_MOUSE -1

#define BIND_YAW_LEFT_KEY KEY_A
#define BIND_YAW_RIGHT_KEY KEY_D
#define BIND_PITCH_UP_KEY KEY_W
#define BIND_PITCH_DOWN_KEY KEY_S

#define BIND_CAMERA_KEY KEY_C
#define BIND_PAUSE_KEY KEY_P
#define BIND_EXIT_KEY KEY_Q
#define BIND_NEXT_WAVE_KEY KEY_EQUAL

#define BIND_GRID_INC_KEY KEY_F10
#define BIND_GRID_DEC_KEY KEY_F9
#define BIND_TIME_INC_KEY KEY_KP_ADD
#define BIND_TIME_DEC_KEY KEY_KP_SUBTRACT

// --- Menu Keybinds ---
#define BIND_MENU_UP_KEY KEY_UP
#define BIND_MENU_DOWN_KEY KEY_DOWN
#define BIND_MENU_LEFT_KEY KEY_LEFT
#define BIND_MENU_RIGHT_KEY KEY_RIGHT
#define BIND_MENU_SELECT_1 KEY_ENTER
#define BIND_MENU_SELECT_2 KEY_SPACE
