# Per-Triangle Ground Collision Analysis (Trees & Bushes)

This document outlines the implementation strategy for mesh-accurate collision detection for ground-based vegetation (Bushes and Trees) in the ZVirus Raylib port.

## Overview
Currently, ground collision is handled by a grid-height check or a "block" check. This results in bullets hitting invisible boundaries near tree trunks or passing through the gaps in branches. By implementing per-triangle collision for these specific objects, we achieve pixel-perfect hits.

## Technical Strategy

### 1. Spatial Culling
Ground objects are tied to the 256x256 terrain grid. We only perform mesh tests when a particle enters a cell containing an object with `objectIndex` between 1 and 4:
1. Bush
2. tree1
3. tree2
4. tree3

### 2. Transformation Matrix
For each candidate cell `(cx, cz)`, we construct a transformation matrix matching the `TerrainRenderObjects` logic:
- **Translation**: `(cx, landHeight, cz)`
- **Rotation Y**: `-objectYaw`
- **Rotation X**: `-objectPitch`
- **Scale**: Identity (1.0f)

The resulting Matrix is `M = Translate * RotateY * RotateX`.

### 3. Raycasting
We cast a ray from the particle's position at the start of the frame to its new position.
- **Ray Origin**: `(old_x, old_y, old_z)`
- **Ray Direction**: `(current_x - old_x, current_y - old_y, current_z - old_z)` normalized.
- **Ray Distance**: Length of the displacement vector.

### 4. Mesh Iteration
Raylib `Model` structures can contain multiple meshes. The implementation iterates through all meshes in the `model.meshState[status]` and tests each one using `GetRayCollisionMesh`.

## Performance Considerations
- **Culling**: Most cells are empty, incurring zero cost for mesh intersection.
- **Primitive Count**: Trees and bushes are low-poly (50-300 triangles).
- **Early Exit**: Once a hit is detected in one mesh of an object, we skip further tests for that particle in that frame.

## Implementation Details
The collision logic is integrated into `ParticleUpdateAll` in `src/particle.c`. It replaces the interpolated height check specifically for the vegetation object types.

Viewed particle.c:1-319
Edited per-triangle-collision-analysis.md

I have updated the analysis document with the implemented line ranges and coordinate considerations.

### Updated Implementation Summary in [particle.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c):
-   **Header**: Added `#include <raymath.h>` on **L12**.
-   **State Capture**: Added `ox, oy, oz` capture on **L146-148** to provide an accurate ray start point for 60Hz displacement math.
-   **Collision Logic**: Implemented the per-triangle raycasting block on **L167-233**, which handles:
    -   **Transformation Matrix**: Correctly ordered as $T \cdot RY \cdot RX$ to match the visual rendering.
    -   **Rotation**: Honors the project's CCW rotation direction (`-yaw` and `-pitch`).
    -   **Timing**: Uses the 60Hz-corrected velocity displacement for ray length.

The per-triangle implementation for vegetation is now complete and documented. Would you like to review any other collision systems?