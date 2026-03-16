Viewed util.c:1-71
Searched for "p\\z = wrap( p\\z - p\\vz, size )"
Viewed Source.bb:1110-1150
Searched for "p\\x = wrap( p\\x + p\\vx, size )"
Searched for "p\\x = wrap( p\\x + p\\vx, size )"
Searched for "Function PLAYERupdate"
Viewed Source.bb:624-850
Viewed Source.bb:1110-1130
Viewed Source.bb:1570-1620
Searched for "Function TERRAINload"
Viewed Source.bb:1807-1860
Viewed particle.h:1-18
Viewed world.h:1-212
Searched for "Raycast"
Searched for "LinePick"

I have compared the bullet implementation in the Raylib project with the original [Source.bb](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb:0:0-0:0) logic. While the core physics and displacement math are fairly consistent (including the inverted Z-axis and 60Hz adjustments), I found several critical discrepancies that explain why bullets frequently "go through" ships and ground objects.

### Comparison Summary

| Feature | Original ([Source.bb](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb:0:0-0:0)) | Raylib Port | Impact |
| :--- | :--- | :--- | :--- |
| **Ship Collision Radius** | Dynamically calculated from mesh size: `Sqr(W^2 + D^2) * 0.4`. | Hardcoded to `1.0f` for all ships. | **Severe.** Large ships (Destroyers, Elites, Bosses) have tiny hitboxes relative to their models, letting bullets pass through wings/tails. |
| **Ground Hitbox** | Interpolated height check (`p\y < gh`). | Interpolated height check (`p->y < gh`). | **Moderate.** Using interpolated height creates a "pyramid" collision field. Visual corners of buildings are "ghosts" that bullets fly through unless hitting the exact cell center. |
| **Vertical Alignment** | Points measured between pivots. | Points measured between pivots. | **Minor.** Ships are drawn at `y+0.5` but hitboxes are centered at `y`. A 1.0 radius sphere mostly covers the bottom half. |
| **Z-Displacement** | `ShipZ - ShipVZ`, `BulletZ + BulletVZ`. | `ShipZ - ShipVZ * 0.5`, `BulletZ - BulletVZ * 0.5`. | **None.** Consistent with inverted Z-axis; both move in the same world direction. |

### Analysis of the "Going Through" Issue

#### 1. Underestimated Ship Radii
The primary cause for bullets missing ships is the hardcoded radius in [world.c](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/src/world.c:0:0-0:0):
```c
// world.c:172
gFlyingObjects[i].radius = 1.0f; 
```
In the original game, a large ship like the **Destroyer** (AI 7) would have a radius derived from its mesh body. If a ship is 10 units long, its original hitbox radius would be ~3.5 to 5.6. In our port, it stays at `1.0`. For a player, the bullet appears to hit the wing of a large ship but is technically "outside" the small central sphere.

#### 2. The Pyramid Collision Bug (Ground Objects)
Both versions use [TerrainGetHeight(p->x, p->z, 1)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/terrain.c:318:0-355:1) to check for ground object collisions. Because this function interpolates between grid cells, a single tall building at grid [(10, 10)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:479:0-787:1) has a hit-height of 2.0 only at that exact point. At [(10.5, 10.5)](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/main.c:479:0-787:1), if surrounding cells are empty, the collision height drops to `0.5`. A bullet at `y=1.0` will fly through the building's visual corner without triggering a hit.

#### 3. Vertical Pivot Mismatch
Ships are visually lifted in the draw call:
```c
// main.c:245
Vector3 drawPos = {p->x + dx, q->y + 0.5f, p->z + dz};
```
However, [CollisionBullet](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c:17:0-49:1) checks distance to `c->y`. If the bullet is at `y=1.5` (hitting the top of the cabin) and the ship is at `y=0.5` (base on ground), the vertical distance `dy` is 1.0. This is at the exact edge of our 1.0 radius sphere, making top-down or high-altitude shots likely to fail.

### Recommendation
1.  **Dynamic Radius Calculation**: Modify [FlyingObjectInit](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/world.c:38:0-183:1) to calculate `radius` from the `Model` bounding box, replicating the `Sqr(W^2 + D^2) * 0.4` logic from [Source.bb](cci:7://file:///home/pes/dev/C/zvirus-conv-raylib/Source.bb:0:0-0:0).
2.  **Max-Height Ground Check**: Change [ParticleUpdateAll](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c:132:0-182:1) to check the maximum height of the 4 adjacent cells for ground objects rather than the interpolated height. This will turn the "pyraid" hitboxes into "blocks" that match the visual meshes.
3.  **Centered Hitbox**: Adjust [CollisionBullet](cci:1://file:///home/pes/dev/C/zvirus-conv-raylib/src/particle.c:17:0-49:1) to use `c->y + 0.5f` as the vertical center of the ship's sphere.

### Implementation Checklist
I have created a task list to address these findings.

```task
[ ] Update world.c: calculate FlyingObject.radius from BoundingBox after loading meshes.
[ ] Update particle.c: in CollisionBullet, use `p->y - (c->y + 0.5f)` for vertical distance.
[ ] Update particle.c: in ParticleUpdateAll, replace TerrainGetHeight with a Max-Height check for ground objects.
```

Would you like me to proceed with implementing these fixes?