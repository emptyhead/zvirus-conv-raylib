#pragma once

#include <stdbool.h>

// Clamp a float between [lo, hi].
float clampf(float q, float lo, float hi);

// Clamp an int between [lo, hi].
int clampi(int q, int lo, int hi);

// Wrap a float into [lo, hi).
float wrapf(float q, float hi, float lo);

// Wrap an int into [lo, hi).
int wrapi(int q, int hi, int lo);

// Transform a local vector (lx, ly, lz) to world space based on pitch and yaw.
// Matches Blitz3D TFormVector logic (Pitch then Yaw).
void TFormVector(float lx, float ly, float lz, float pitchDeg, float yawDeg,
                    float *ox, float *oy, float *oz);

// Run a small self-test for the utilities.
bool UtilSelfTest(void);

