# Sound Implementation Details

## Overview
The sound implementation in this port has been modified to accurately reflect the behavior of the original Blitz3D `Source.bb` project. This ensures parity in how the player perceives distance and direction, as well as maintaining the "arcade" feel of the 2D sound effects.

## Key Changes

### 1. Listener Reference Point
In the previous implementation of the port, all positional calculations (volume and panning) were performed relative to the active camera. This caused sounds to shift unexpectedly when toggling between Rear and Chase views.
- **New Behavior**: The listener position is now fixed to the player ship (`gShips[0]`).
- **Original Parity**: Matches `Source.bb`, where `PLAYERdistance` calculations for sound often used the player object as the reference.
- **C Implementation**: `src/ship.c:ShipPosition` now uses `gShips[0]` for `lx`, `lz`, and `d` calculations.

### 2. 3D Sound Falloff Formula
We have implemented the specific non-linear formula found in the original source code, using squared distance units.
- **Formula**: `volume# = GroupVolume * (1.0 / (d * 0.004 + 1.0))` where `d` is `dx*dx + dy*dy + dz*dz`.
- **C Implementation**: 
  ```c
  float vol = groupBaseVols[sgIdx] * (1.0f / (d * SND_DIST_SCALE + 1.0f));
  ```
- **Constants**: `SND_DIST_SCALE` (0.004f) is exposed in `src/constants.h`.

### 3. Panning Behavior
- **3D Sounds (Engine Drones)**: Use spatial panning relative to the camera's orientation. This preserves the 3D feel of the environment.
  - **Implementation**: Calculated in `src/ship.c:ShipPosition` using `atan2f` and `sinf(relAngle)`.
- **2D Event Sounds (Shots, Explosions, Hits)**: Played as mono (centered) to maintain an "arcade" feel, as requested.
  - **Implementation**: `SND_VOL_3D_CENTER` (0.5f) is used for `AudioPlay` calls in `ShipUpdateAll`, `particle.c`, and `terrain.c`.

### 4. 2D Event Sounds
Sounds triggered by events (shooting, explosions, bullet hits) now play at a fixed volume (modified only by a master multiplier) if the originating entity is "in view".
- **Condition**: `if (p->inView)`
- **Volume**: `SND_VOL_... * SND_VOL_MASTER`

## Code References

### Original `Source.bb`
- **Volume Logic**: Lines 900-909 (Manual volume calculation for Sound Groups).
- **Discrete Sounds**: 
  - `PlaySound(SOUNDshoot)` - Line 829.
  - `PlaySound(SOUNDexplode)` - Line 881.
  - `PlaySound(SOUNDsplat)` - Line 494.

### Raylib Port Implementation
- **Volume/Pan Logic**: `src/ship.c:ShipPosition`
- **Sound Triggering**:
  - **Shooting**: `src/ship.c:ShipUpdateAll` (around line 770)
  - **Explosions**: `src/ship.c:ShipUpdateAll` (around line 560)
  - **Bullet Hits**: `src/particle.c:CollisionBullet`
  - **Ground Impact**: `src/terrain.c:TerrainCollisionGround`
- **Constants**: `src/constants.h` (`SND_DIST_SCALE`, `SND_VOL_3D_CENTER`).

## Differences
- **Frame Rate**: The port runs at 60Hz. Sound updates are processed within the physics simulation steps to maintain timing consistency with the original 30Hz logic.
- **Coordinate System**: While the distance formula is identical, the port accounts for the inverted Z-axis by using squared distance sums which are coordinate-agnostic.