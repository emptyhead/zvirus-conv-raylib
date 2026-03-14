# Entity Spawn Logic & Monster Implementation

This document describes the implementation of the entity spawning system, with a specific focus on the "Monster" (ID 17) and how it maintains parity with the original Blitz3D implementation.

## 1. General Spawn Logic ([`src/ship.c`](../src/ship.c))

Entities are spawned using the `ShipReset(id, aiType)` function. The system uses a ring-based randomization for all non-player entities:

- **Center Coordinate**: `(SIZE/2, SIZE/2)`
- **Spawn Radius**: Randomly chosen between `64.0` and `120.0` units.
- **Spawn Angle**: Randomly chosen between `0` and `360` degrees.

```c
int r = GetRandomValue(64, 120);
int a = GetRandomValue(0, 360);
p->x = (float)SIZE * 0.5f - 0.5f + (float)r * cosf((float)a * DEG2RAD);
p->z = (float)SIZE * 0.5f        + (float)r * sinf((float)a * DEG2RAD);
```

## 2. Monster-Specific Constraints (ID 17)

The "Monster" entity has unique spawning requirements to ensure it only appears in water regions (where the terrain is at base level).

### Water Validation Loop
To maintain parity with `Source.bb:599`, `ShipReset` includes a verification loop that rejects any spawn position where the terrain height is greater than a small epsilon (0.05 units).

```c
// Monster (AI 17) validation
float landH = TerrainGetHeight(p->x, p->z, 1);
if (aiType != 17 || landH <= 0.05f) { 
    ok = 1; // Valid spawn
}
```

### Key Differences from Standard Ships:
- **Vertical Offset**: Unlike standard flying ships (which have a `+50.0` height offset) or the player (`+0.5`), the Monster is positioned exactly at `landH`.
- **Radar Invisibility**: The Monster uses `index = 3`. The HUD map logic in `src/hud.c` is configured to only draw dots for `index == 1` (Enemies) and the player, making the Monster a hidden threat as in the original.
- **Spin Speed**: Replicated at `0.5f` (60Hz) from the original `1` (30Hz).

## 3. Parity Checklist

| Feature | Source.bb Implementation | C/RayLib Implementation | Status |
| :--- | :--- | :--- | :--- |
| **Water Only** | `ok = ( ai <> 17 Or terrain( p\x,p\z)\LandHeight = 0 )` | Validation loop in `ShipReset` | ✅ |
| **Ring Spawn** | `r = Rand(64,120) : a = Rand(0,360)` | `GetRandomValue(64, 120)` | ✅ |
| **Entity Alpha** | `EntityAlpha p\dot,(AI<>17)` | HUD filter `if (e->index == 1)` | ✅ |
| **Spin/Momentum** | `SpinSpeed 1` (30Hz) | `0.5f` (60Hz) | ✅ |

## 4. Coordination Systems Note
Recall that RayLib uses a **Right-Handed** coordinate system while Blitz3D uses **Left-Handed**. 
- In `ShipReset`, the conversion logic ensures that the circular spawn distribution correctly translates to the internal world representation.
- Inversion of the Z-axis is handled during integration in `ShipUpdateAll` (`p->z = wrapf(p->z - p->vz * 0.5f, ...)`).
