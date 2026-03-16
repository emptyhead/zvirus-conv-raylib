# Attractor/Repulsor AI and Hit Sound Implementation Details

This document details the implementation of the Attractor/Repulsor AI and the bullet hit sound system, comparing the ported C/Raylib version with the original BlitzBasic source.

## 1. Bullet Hit Sound Implementation

Adds audible feedback when the player's bullets (or any ship's bullets) impact another entity.

| Feature | Port File (Line Range) | Source.bb (Line) | Notes |
| :--- | :--- | :--- | :--- |
| **Logic Trigger** | `src/particle.c` (35-50) | 493 | Fired within `CollisionBullet` / `COLLISIONbullet` |
| **Panned Audio** | `src/particle.c` (47-49) | N/A | Calculated using `atan2f` and `sinf` for 2D stereo panning |

### Variances
- **Panning**: The Raylib port calculates dynamic stereo panning based on the relative angle to the camera. The original Blitz code used `PlaySound` which in that specific context was mostly non-panned or handled by the engine's 3D sound system (which we have simplified to a 2D stereo model for this effect).
- **Master Volume**: All sounds are scaled by `SND_VOL_MASTER` (from `constants.h`).

---

## 2. Attractor AI (AI Type 8)

The Attractor (AI 8, Hoverer engine) pulls the player and allies toward itself and drains their fuel.

| Feature | Port File (Line Range) | Source.bb (Line Range) | Notes |
| :--- | :--- | :--- | :--- |
| **Main Logic** | `src/ship.c` (246-271) | 1039-1084 | `ShipAttract` vs `PLAYERattract` |
| **Force Application**| `src/ship.c` (258-261) | 1051-1054 | Velocity delta and fuel drain |
| **Landscape Destruction**| `src/ship.c` (267-268) | 1060-1061 | Occurs when no target is in range |

### Variances
- **60Hz Parity**: Movement forces and fuel drain were halved compared to the 30Hz source to maintain gameplay feel.
  - Attraction force (scale): `0.0375f` (Port) vs `0.075` (Source).
  - Fuel drain: `1.0f` (Port) vs `2.0` (Source).
- **Coordinate System**: The port uses `gNX/NY/NZ` which are derived from a Raylib-friendly Z-alignment.

---

## 3. Repulsor AI (AI Type 9)

The Repulsor (AI 9, Hoverer engine) snaps its rotation to point directly at the player before firing.

| Feature | Port File (Line Range) | Source.bb (Line Range) | Notes |
| :--- | :--- | :--- | :--- |
| **Main Logic** | `src/ship.c` (276-297) | 1090-1099 | `ShipShootAt` vs `PLAYERshootat` |
| **Aiming (Rotation)**| `src/ship.c` (282-293) | 1094-1095 | `PointEntity` equivalent |

### Variances
- **Orientation Snapping**: In the Raylib port, `ShipShootAt` is called after movement integration (`ShipUpdateAll` lines 717-721) but before particle spawning. This ensures the ship's model and firing vector are correctly aligned in the same frame the bullet is spawned.
- **Z-Inversion**: `atan2f(dx, -dz)` is used for yaw calculation to account for Raylib's world forward being `-Z`.

---

## 4. Execution Flow Changes

In the original Blitz code, special functions like `PLAYERattract` and `PLAYERshootat` were called from within `PLAYERupdate`. In the port, we have moved these to `ShipUpdateAll` to ensure they execute at the correct point in the frame lifecycle.

- **Port Line Range**: `src/ship.c` (717-721)
- **Source Line Range**: `Source.bb` (790-791)

---

## Summary of Parity Advesments

- **Simulation Rate**: All physics-related constants from `Source.bb` were evaluated for 60Hz. Where constants were "per-frame" in 30Hz, they have been halved or adjusted using the square root for momentum to maintain ballistic parity.
- **Rotation Directions**: Raylib's right-handed system with CCW rotations necessitated negating the yaw and pitch values during rendering (`main.c`) relative to the CW-positive physics values used in the simulation.
