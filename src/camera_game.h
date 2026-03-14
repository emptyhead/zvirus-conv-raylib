#pragma once

#include "raylib.h"
#include "world.h"

// Camera mode constants
typedef enum GameCameraMode {
    GCAM_CHASE    = 0,
    GCAM_REAR     = 1,
} GameCameraMode;

// Global current camera mode
extern GameCameraMode gCameraMode;

// Index of the player the camera follows (cam variable in Source.bb)
extern int gCam;

// Yaw of the camera in degrees (used for particle facing)
extern float gCameraYaw;

// The raylib camera used for 3D rendering
extern Camera3D gCamera3D;

// A chase pivot – local-space position used for smooth camera chase
extern Vector3 gChasePivot;

// Initialize the camera system
void CameraGameInit(void);

// Update the camera each frame. smooth controls lag factor (0..1).
// Pass smooth=1.0 for a snap (e.g., on hard camera switch).
void CameraGameUpdate(float smooth);

