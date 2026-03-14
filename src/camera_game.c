#include "camera_game.h"

#include <math.h>

#include "raylib.h"
#include "util.h"
#include "world.h"

GameCameraMode gCameraMode = GCAM_CHASE;
int        gCam = 0;
float      gCameraYaw = 0.0f;
Camera3D   gCamera3D = {0};
Vector3    gChasePivot = {0};

void CameraGameInit(void) {
    gCamera3D.up         = (Vector3){0.0f, 1.0f, 0.0f};
    gCamera3D.fovy       = 60.0f;  // ~1.25x zoom compared to 45 deg
    gCamera3D.projection = CAMERA_PERSPECTIVE;
    // Position relative to player origin – will be set each frame
    gCamera3D.position = (Vector3){0.0f, 10.0f, -20.0f};
    gCamera3D.target   = (Vector3){0.0f, 0.0f, 0.0f};
    gChasePivot = (Vector3){0};
}

// Mirror of Source.bb CAMERAupdate( smooth#=.1 )
// In Blitz, camera/chase are children of the world; positions are in local
// (player-relative) space. In Raylib we work in world space but derive the
// same relative offset and add the player's world position.
void CameraGameUpdate(float smooth) {
    Ship *p = &gShips[gCam];

    // Distance and height should stay consistent regardless of terrain mesh size
    float z = 12.0f; // Default distance
    float h = 0.0f;  // Default height offset



    switch (gCameraMode) {

        case GCAM_CHASE:
        case GCAM_REAR:
        {
            // z# = LIMIT( 5 + Grid\Divs , 0, 32 )
            // dist# = (gCameraMode == GCAM_CHASE) ? z# : 12.0
            float dist = (gCameraMode == GCAM_CHASE) ? z : 12.0f;
            float yawRad = p->yaw * DEG2RAD;

            // Blitz PositionEntity chase,0,0,0 -> RotateEntity chase,0, p\yaw, 0 -> MoveEntity chase,0,0,-dist
            // Smoothing the *offset* relative to the player instead of absolute world coords.
            // This makes the camera immune to world-wrap 'teleports'.
            float idealX = -sinf(yawRad) * dist;
            float idealZ =  cosf(yawRad) * dist;

            // Use gChasePivot to store the smoothed relative offset
            // Initializing if it looks like absolute world coords (SIZE/2 check)
            if (fabsf(gChasePivot.x) > (float)SIZE/2.0f || (gChasePivot.x == 0 && gChasePivot.z == 0)) {
                gChasePivot.x = idealX;
                gChasePivot.z = idealZ;
            }

            gChasePivot.x += (idealX - gChasePivot.x) * smooth;
            gChasePivot.z += (idealZ - gChasePivot.z) * smooth;

            // Position camera at player + smoothed offset
            float cy = clampf(p->y, 6.0f, 2000.0f) + h;
            gCamera3D.position = (Vector3){ p->x + gChasePivot.x, cy, p->z + gChasePivot.z };

            // Look directly at the player
            gCamera3D.target = (Vector3){ p->x, p->y, p->z };
        }
        break;
    }

    // CameraYaw = 360 + EntityYaw(camera)
    {
        float dx = gCamera3D.target.x - gCamera3D.position.x;
        float dz = gCamera3D.target.z - gCamera3D.position.z;
        gCameraYaw = wrapf(360.0f + atan2f(dx, dz) * RAD2DEG, 360.0f, 0.0f);
    }
}
