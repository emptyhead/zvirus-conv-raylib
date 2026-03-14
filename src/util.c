#include "util.h"
#include <math.h>

float clampf(float q, float lo, float hi) {
  if (q < lo) q = lo;
  if (q > hi) q = hi;
  return q;
}

int clampi(int q, int lo, int hi) {
  if (q < lo) q = lo;
  if (q > hi) q = hi;
  return q;
}

float wrapf(float q, float hi, float lo) {
  float range = hi - lo;
  if (range <= 0.0f) return lo;
  float res = q - lo;
  res = res - floorf(res / range) * range;
  return lo + res;
}

int wrapi(int q, int hi, int lo) {
  int range = hi - lo;
  if (range <= 0) return lo;
  int res = (q - lo) % range;
  if (res < 0) res += range;
  return lo + res;
}

void TFormVector(float lx, float ly, float lz, float pitchDeg, float yawDeg,
                        float *ox, float *oy, float *oz) {
    float pr = pitchDeg * (3.14159265f / 180.0f);
    float yr = yawDeg   * (3.14159265f / 180.0f);

    float cp = cosf(pr);
    float sp = sinf(pr);
    float cy = cosf(yr);
    float sy = sinf(yr);

    // Unified projection for +X=Right, -Z=Forward (World) 
    // from local point (lx, ly, lz) where lz=forward, ly=up.
    // Matches indicator line logic in main.c
    *ox =  lx * cy + (ly * sp + lz * cp) * sy;
    *oy =  ly * cp - lz * sp;
    *oz = -lx * sy + (ly * sp + lz * cp) * cy; 
}

bool UtilSelfTest(void) {
  // Basic clamp tests.
  if (clampf(-1.0f, 0.0f, 10.0f) != 0.0f) return false;
  if (clampf(11.0f, 0.0f, 10.0f) != 10.0f) return false;
  if (clampf(5.0f, 0.0f, 10.0f) != 5.0f) return false;

  if (clampi(-1, 0, 10) != 0) return false;
  if (clampi(11, 0, 10) != 10) return false;
  if (clampi(5, 0, 10) != 5) return false;

  // Basic wrap tests (inclusive lo, exclusive hi).
  if (wrapi(-1, 10, 0) != 9) return false;
  if (wrapi(10, 10, 0) != 0) return false;
  if (wrapi(3, 10, 0) != 3) return false;

  if (wrapf(-1.0f, 10.0f, 0.0f) != 9.0f) return false;
  if (wrapf(10.0f, 10.0f, 0.0f) != 0.0f) return false;

  return true;
}

