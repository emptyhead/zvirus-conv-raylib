# Bullet Collision Improvements

This document details the changes made to improve bullet collision accuracy, preventing bullets from occasionally passing through ships and ground objects.

## Key Changes

### 1. Dynamic Ship Collision Radius
**Problem**: Hitboxes were hardcoded to `1.0f`, making large ships (Destroyers, Elites) easy to shoot through at the extremities.
**Original Source (`Source.bb:1385-1386`)**:
```blitzbasic
Radius# = Sqr( MeshWidth(F\Model)^2 + MeshDepth( F\Model)^2 )
F\Radius = LIMIT ( Radius*.4  , .625, 100 )
```
**Our Implementation (`src/world.c`)**:
```c
BoundingBox bb = GetModelBoundingBox(gFlyingObjects[i].mesh);
float w = bb.max.x - bb.min.x;
float d = bb.max.z - bb.min.z;
float meshR = sqrtf(w*w + d*d);
gFlyingObjects[i].radius = clampf(meshR * G_SHIP_HITBOX_SCALE, G_SHIP_RADIUS_MIN, 100.0f);
```
The constant `G_SHIP_HITBOX_SCALE` is exposed in `src/constants.h` (defaulting to `0.4f`).

### 2. Vertical Hitbox Re-Alignment
**Problem**: Ships are visually lifted by an offset (formerly `0.5f`), but collision was checked against their "base" position at `y`, causing bullets to pass over the hull.
**Our Implementation (`src/particle.c:CollisionBullet`)**:
```c
float dy = p->y - (c->y + G_SHIP_DRAW_OFFSET_Y);
```
Collision is now centered on the visual hull. `G_SHIP_DRAW_OFFSET_Y` is exposed in `src/constants.h`.

### 3. "Pyramid" Ground Collision Fix
**Problem**: `TerrainGetHeight` interpolates between grid cells, creating sloped hitboxes for boxy buildings. Bullets could pass through the upper corners of objects.
**Our Implementation (`src/particle.c:ParticleUpdateAll`)**:
If the standard interpolated check fails, bullets now check the 4 adjacent grid cells directly to see if they overlap a ground object's rectangular footprint. This prevents passthrough on visual corners.
```c
if (!groundHit && p->id == 1) {
    // ... loop 2x2 grid footprint ...
    if (t->objectIndex > 0 && p->y < (t->objectHeight + (p->size * OSCALE))) {
        groundHit = true;
    }
}
```

## Constants Exposed (`src/constants.h`)

| Constant | Value | Description |
| :--- | :--- | :--- |
| `G_SHIP_DRAW_OFFSET_Y` | `0.5f` | Vertical lift of ships above terrain height. |
| `G_SHIP_HITBOX_SCALE` | `0.4f` | Multiplier for mesh size to determine collision radius. |
| `G_SHIP_RADIUS_MIN` | `0.625f` | Minimum allowed radius for any ship. |

## Code Locations

| Project File | Original Function | Port Function | Line Range (C) | Original Line (BB) |
| :--- | :--- | :--- | :--- | :--- |
| `src/world.c` | `GAMEinit` | `FlyingObjectInit` | `173-183` | `1385-1386` |
| `src/particle.c` | `COLLISIONbullet` | `CollisionBullet` | `28` | `486` |
| `src/particle.c` | `PARTICLEupdate` | `ParticleUpdateAll` | `156-173` | `2305` |
| `src/main.c` | `MAIN` loop | `DrawFullFrame` | `142, 245, 404` | `N/A` |
| `src/ship.c` | `PLAYERupdate` | `ShipUpdateAll` | `115, 575, 694, 733` | `747-756` |
