# Attractor AI Implementation Details (AI Type 8)

This document details the implementation of the Attractor AI, comparing the ported C/Raylib version with the original BlitzBasic source while accounting for simulation rate and coordinate system variances.

## 1. Overview
The Attractor (AI Type 8) is a "Hoverer" engine enemy that actively pulls the player and allied ships towards its center while draining their fuel. Visually, it connects to its target with a jittering energy beam (lightning effect).

## 2. Code Comparison

| Component | Port File (Line Range) | Source.bb (Line Range) | Notes |
| :--- | :--- | :--- | :--- |
| **Execution Trigger** | `src/ship.c` (725-729) | 790 | Called after movement integration. |
| **Logic (Detection)** | `src/ship.c` (186-200) | 1168-1186 | `ShipFindTarget` vs `PLAYERfindtarget`. |
| **Logic (Attraction)** | `src/ship.c` (246-279) | 1039-1084 | `ShipAttract` vs `PLAYERattract`. |
| **Physics Application**| `src/ship.c` (263-266) | 1051-1054 | Force vector and fuel drain. |
| **Beam Rendering** | `src/main.c` (340-375) | 1064-1076 | Procedural lightning vs Vertex manipulation. |

## 3. Implementation Variances

### 3.1 Simulation Rate (60Hz vs 30Hz)
To maintain gameplay parity with the original 30Hz simulation, physics constants and temporal logic were adjusted for the 60Hz loop:
- **Attraction Force**: The base scale was halved from `0.075` to **`0.0375`**.
- **Fuel Drain**: The drain per frame was reduced from `2.0` to **`1.0`**.
- **Force Falloff**: The port implements a `1 / distance` magnitude falloff to more accurately model the original's `-dx / distance_squared` logic while remaining robust at low distances.

### 3.2 Coordinate System (Right-Handed vs Left-Handed)
The Raylib port uses a Right-Handed system with World Forward aligned to **negative Z**.
- **Z-Integration**: Both systems integrate using `z = z - vz`.
- **Force Polarity**: In the port, attraction along Z is applied as `t->vz += gNZ * force` (where `gNZ` is the normal pointing towards the target). This correctly pulls the target towards the attractor in the inverted Z space.
- **Rotations**: Raylib uses CCW rotations. The ship's internal `yaw` and `pitch` (following Blitz's CW convention) are negated during the rendering pass in `main.c` (`rlRotatef(-yaw, 0, 1, 0)`) to maintain visual parity.

### 3.3 Visual Effects (Tractor Beam)
The original used a specialized `Tractor` mesh and manipulated its vertices in real-time. The C port replaces this with a procedural rendering system:
- **Lightning Generation**: `main.c` iterates through 8 segments between the attractor and target, applying random jitter to each joint.
- **Thickness Simulation**: Multiple overlapping jittered lines are drawn for each segment to simulate the "thick energy discharge" look of the original.
- **Wrap-Awareness**: The beam re-projects the destination coordinates (`attractPos`) relative to the focused player to ensure the beam displays correctly even when spanning across the world's wrap-around seam.
