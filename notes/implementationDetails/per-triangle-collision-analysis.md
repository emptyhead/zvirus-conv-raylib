Viewed particle.c:1-319
Viewed bullet-collision-analysis.md:1-63
Viewed world.h:1-212
Viewed constants.h:1-346
Viewed util.c:1-71

### Implementation Status
The per-triangle ground collision for vegetation has been implemented in [particle.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c).

**Touched Line Ranges in `src/particle.c`:**
-   **L12**: Added `#include <raymath.h>` for matrix and ray operations.
-   **L146-148**: Captured particle's previous position (`ox, oy, oz`) before frame displacement.
-   **L167-233**: Replaced the previous grid-cell block check with mesh-accurate raycasting for object indices 1-4 (Bushes and Trees).

---

I have examined the [CollisionBullet](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c:20:0-52:1) function in [particle.c](file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c) and compared it with the existing [bullet-collision-analysis.md](file:///home/pes/dev/C/zvirus-conv-raylib/notes/implementationDetails/bullet-collision-analysis.md). Below is an estimation of the difficulty and performance impact of implementing per-triangle collision.


### Current Implementation Overview
The current collision mechanism is a simple **sphere-to-sphere** check:
- it calculates the wrapped distance between a particle and a ship's center.
- It compares this distance to a single `radius` value stored in `gFlyingObjects`.
- **Primary Issue**: Many hitboxes are currently hardcoded to `1.0f`, leaded to bullets passing through the wings of larger ships.

---

### Estimation: Per-Triangle Collision

#### 1. Implementation Difficulty: **Moderate**
Implementing this would require moving from a point-distance check to a **Ray-Mesh intersection** test.

*   **Raycasting**: You would need to cast a ray from the bullet's previous position to its current position to prevent "tunneling" (where a fast bullet skip over a thin mesh).
*   **Coordinate Transformations**: Raylib's `GetRayCollisionMesh` requires a `Matrix` transform. You would need to construct this matrix every frame for each ship-bullet pair being tested (Position × Rotation × Scale).
*   **Periodic Wrapping (The Hard Part)**: Because the world wraps at `SIZE (256)`, a bullet might be at `x=255.9` and the ship at `x=0.1`. You must detect the "closest" instance of the ship (of which there are technically 9 in a wrapped grid) before performing the raycast, or perform the raycast in a "local-relative" space that handles the wrap.
*   **Data Access**: All mesh data is already available via `gFlyingObjects[ship->ai].mesh`.

#### 2. Performance Impact: **Significant**
The performance cost would increase substantially without careful optimization.

*   **Complexity Scaling**: 
    *   **Current**: $O(B \times S)$, where $B$ is active bullets and $S$ is active ships. Each check is ~5 floating point operations.
    *   **Per-Triangle**: $O(B \times S \times T)$, where $T$ is the number of triangles in a ship's mesh (e.g., 200–2,000).
*   **Active Particles**: With `MAX_PARTICLES` set to `16,384`, even a small fraction of active bullets (e.g., 500) against 100 ships could result in millions of triangle tests per frame.
*   **Raylib Overhead**: `GetRayCollisionMesh` iterates through every triangle unless the mesh has a built-in BVH (Bounding Volume Hierarchy), which standard Raylib meshes do not unless manually generated.

---

### Comparison Summary

| Metric | Current (Sphere) | Per-Triangle |
| :--- | :--- | :--- |
| **Accuracy** | Low (Spherical approximation) | Very High (Pixel-perfect) |
| **Difficulty** | Trivial | Moderate (requires Matrix math & Raycasting) |
| **CPU Cost** | Negligible | High (Potential frame-time bottleneck) |
| **Logic** | `distance < radius` | `GetRayCollisionMesh(ray, mesh, transform)` |

### Recommendation
Instead of full per-triangle collision, which may be overkill for a fast-paced arcade game, I recommend a **Hybrid Approach**:

1.  **Keep the Sphere Check as a Broad-Phase**: Only perform mesh-level checks if the bullet is already within a "Bounding Sphere" (the current check).
2.  **Fix the Radii First**: As noted in your analysis, simply calculating the correct `radius` from the mesh's actual bounding box (using `G_SHIP_HITBOX_SCALE`) will fix 90% of the "going through" issues with very little performance cost.
3.  **Compound Hitboxes**: For very long or complex ships (like the Cruiser), use 2 or 3 spheres instead of one.

**Would you like me to implement a prototype of the "Hybrid" Mesh collision, or start by fixing the hardcoded sphere radii?**